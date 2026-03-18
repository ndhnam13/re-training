**Đề bài:** C2 server bằng MFC có tính năng chạy lệnh, Download, Upload file từ/lên client

## Tạo project MFC Form-based

Dưới type chọn Form-based và style là native

Tạo project xong sẽ cho ta các file chính sau

- `Server.cpp`: App entry
- `ServerDlg.h`: Khai báo class Dialog. Các biến toàn cục sẽ lưu ở đây
- `ServerDlg.cpp`: Xử lí logic. các đoạn code về xử lí tính năng của Server sẽ ở dây
- `IDD_SERVER_DIALOG`: Đây là một file resource cho ta chỉnh sửa giao diện của App. Để thêm nút, ta thêm từ ToolBox, đổi tên,... trong phần properties
- Khi tạo xong nút, hoặc các thành phần khác chọn `Add Event Handler`, chọn loại,... để visual studio tự tạo hàm để xử lí logic của nút

# C2 Server

## 1. Server socket: listen

Tạo file header `ServerSocket.h` và `ServerSocket.cpp` để tạo socket listen, lưu trữ thông tin của các client

Đối với server sử dụng MFC socket cho tiện

### Xử lí  khi nhận kết nối từ client

- Sau khi khởi tạo socket bằng `AfxSocketInit()`, gọi `CAsyncSocket::Create()` để tạo một socket trên port muốn listen rồi `CAsyncSocket::Listen()`

- Khi socket đang listen, ``CAsyncSocket::Accept()` và `CAsyncSocket::OnAccept()` để chấp nhận kết nối từ server và lưu vào một array list các client (`std::vector<CAsyncSocket*> m_ClientsList;`)

- Sau khi có client vừa kết nối ta sẽ phải thêm client đó vào một `CListBox`. Để làm việc này ta sẽ phải truyền một pointer tới class `CServerDlg` 
  ```c++
  CServerDlg::CServerDlg(CWnd* pParent /*=nullptr*/)
  	: CDialogEx(IDD_SERVER_DIALOG, pParent),
  	m_ServerSocket(this) // Pass the CServerDlg when initializing server socket 
  {
  	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
  }
  ```

- Để dialog có thể thực hiện truyền dữ liệu giữa `CListBox` và GUI ta phải thêm hàm `DDX_Control` vào trong method `CServerDlg::DoDataExchange()` tại file `ServerDlg.cpp`
  ```c++
  void CServerDlg::DoDataExchange(CDataExchange* pDX)
  {
  	CDialogEx::DoDataExchange(pDX);
  	DDX_Control(pDX, IDC_LIST1, m_ClientList); // Link control to variable
  }
  ```

  - Khi có client kết nối dến thì ta sẽ dùng pointer đó để gọi một method trung gian gọi `CListBox::AddString`. Dưới đây là phần code áp dụng vào trong hàm `OnBnClickedButtonListen()` để xử lí khi nút được bấm 
  
  ```c++
  //ServerSocket.cpp
  CServerSocket::CServerSocket(CServerDlg* pDlg)
  {
  	m_pDlg = pDlg;
  }
  
  CServerSocket::~CServerSocket()
  {
  }
  
  BOOL CServerSocket::StartListen(UINT port) {
  	if (Create(port, SOCK_STREAM, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE, NULL) == 0)
  		return FALSE;
  	Listen();
  	return TRUE;
  }
  void CServerSocket::OnAccept(int nErrorCode) {
  	SOCKADDR_IN clientAddr;
  	int clientAddrLen = sizeof(clientAddr);
  	CServerSocket* pClient = new CServerSocket(m_pDlg);
  
  	if (Accept(*pClient, (SOCKADDR*)&clientAddr, &clientAddrLen)!=0) {
  
  		CStringA ip = inet_ntoa(clientAddr.sin_addr);
  		int port = ntohs(clientAddr.sin_port);
  
  		CString clientConnect;
  		clientConnect.Format(L"Client connected: %S:%d", ip, port);
  		CString clientInfo;
  		clientInfo.Format(L"%S:%d", ip, port);
  
  		if (m_pDlg) {
  			m_pDlg->AddClientToList(clientInfo, pClient);
  		}
  
  		AfxMessageBox(clientConnect);
  		
  	}
  	else {
  		delete pClient;
  	}
  
  	CAsyncSocket::OnAccept(nErrorCode);
  }
  
  //ServerDlg.cpp
  void CServerDlg::AddClientToList(CString clientInfo, CServerSocket* pClient) {
  	m_ClientsList.push_back(pClient); // Add the client socket address to m_Clients arraylist
  	int nIndex =  m_ClientBox.AddString(clientInfo);
  	m_ClientBox.SetItemDataPtr(nIndex, pClient);
  }
  ```


### Xử lí khi client ngắt kết nối

Trong `ServerSocket.h` tạo thêm một virtual method override `OnClose()` để nếu nhận được tín hiệu closesocket từ Client sẽ thông báo qua MessageBox IP:PORT của client đó. Sau đó loại bỏ Client khỏi arraylist và CListBox

```c++
// Called when client closed connection to server
void CServerSocket::OnClose(int nErrorCode) {
	CString peerName;
	UINT peerPort;
	CAsyncSocket::GetPeerName(peerName, peerPort);
	CString dissMsg;
	dissMsg.Format(L"A client dissconnected: %s:%u", peerName, peerPort);
	if (m_pDlg) {
		m_pDlg->RemoveClientFromList(this);
	}
	AfxMessageBox(dissMsg);

	CAsyncSocket::OnClose(nErrorCode);
}

void CServerDlg::RemoveClientFromList(CServerSocket* pClient) {
	// Delete from CListBox
	for (int i = 0; i < m_ClientBox.GetCount(); i++)
	{
		if (m_ClientBox.GetItemDataPtr(i) == pClient)
		{
			m_ClientBox.DeleteString(i);
			break;
		}
	}
	// Delete from vector
	for (auto i = m_ClientsList.begin(); i != m_ClientsList.end(); i++) {
		if (*i == pClient) {
			delete *i;
			m_ClientsList.erase(i);
			break;
		}
	}
}
```



## 2. ClientDlg + Client: Gửi lệnh, xử lí lệnh, gửi và nhận output

Sẽ tạo thêm 2 file ` ClientDlg.h` và `ClientDlg.cpp` để xử lí các message đến/từ client

Trước đó đã tạo một list box và một hàm để xử lí khi nhấn vào 1 thành phần trong listbox đó. Bây giờ sẽ cập nhật để khi nhấn vào 1 client thì sẽ mở ra một cửa sổ mới để thực hiện các tính năng

- Cập nhật `CServerDlg::OnLbnSelchangeList1()` để hiện một cửa sổ mới khi nhấn vào client

```c++
void CServerDlg::OnLbnSelchangeList1() {
	/* TODO: When clicked on an index, show message box
	* Use the choosen client IP:PORT
	* Chose TYPE of CMD to send to client
	*/ 
	int nIndex = m_ClientBox.GetCurSel(); // Get the current selected item
	if (nIndex != LB_ERR) {
		CServerSocket* pClient = (CServerSocket*)m_ClientBox.GetItemDataPtr(nIndex);
		if (pClient != NULL) {
			CString peerName;
			UINT peerPort;
			if (pClient->GetPeerName(peerName, peerPort)) {
				CString msg;
				msg.Format(L"%s:%u", peerName, peerPort);
				int choice = MessageBoxW(msg, L"Server", MB_YESNO);
				if (choice == IDYES)
					NewWindow(pClient);
			}
		}
	}
}
void CServerDlg::NewWindow(CServerSocket* pClient) {
	// TODO: Create a new Window with buttons to control selected client
}
```

- Hiện tại chưa có cửa sổ nên ta sẽ phải tạo resource cửa sổ mới: 
  - `Server.rc` -> Dialog: Tạo `IDD_CLIENT_DIALOG` -> ADD Class, đặt tên `CClientDlg` thì Visual Studio sẽ tự tạo 2 file .h và .cpp
  - `ClientDlg.h`: Chỉnh sửa lại phần constructor thêm tham số `CServerSocket* pClient` rồi copy một số hàm khởi tạo bên `ServerDlg.h`
  - `ClientDlg.cpp`: Tương tự

```c++
void CServerDlg::NewWindow(CServerSocket* pClient) {
	CClientDlg* pDlg = new CClientDlg(pClient, this);
	// Create the window using its Resource ID
	CString windowText;
	char clientAddr[INET_ADDRSTRLEN];
	int clientPort;
	struct sockaddr_in sa;
	int saSize = sizeof(sa);
	getpeername(pClient->m_hSocket, (sockaddr*) & sa, &saSize);
	inet_ntop(AF_INET, &sa.sin_addr, (PSTR)clientAddr, sizeof(clientAddr));
	clientPort = ntohs(sa.sin_port);
	windowText.Format(L"%S:%d", clientAddr, clientPort);
	if (pDlg->Create(IDD_CLIENT_DIALOG, this)) {
		pDlg->SetWindowTextW(windowText);
		pDlg->ShowWindow(SW_SHOW);
	}
}
```

### Loại lệnh

```c++
#define RUNCMD 1
#define DOWNLOAD 2
#define UPLOAD 3
#define CLOSE 4
```

### Khi recv từ client

Nếu client mất nhiều thời gian để chạy một lệnh trước khi trả kết quả về server (Ví dụ như chạy lệnh `ping` hoặc `tasklist` `systeminfo`) thì khi vừa gọi `CAsyncSocket::Receive() hoặc recv()` sẽ trả về `-1` với mã lỗi [WSAEWOULDBLOCK](https://learn.microsoft.com/en-us/windows/win32/winsock/windows-sockets-error-codes-2)(Resource temporarily unavailable) khi gặp mã lỗi đó thì chỉ cần `Sleep()` thêm một chút rồi `CAsyncSocket::Receive() hoặc recv()` lại, nếu gặp các mã lỗi khác thì thông báo cho người dùng

Dưới đây là cách xử lí

```c++
while (recved < sizeof(int)) {
	int ret = m_pClient->Receive((char*)&expectedSize + recved, sizeof(int) - recved);
	if (ret == SOCKET_ERROR) {
		if (GetLastError() == WSAEWOULDBLOCK) {
			Sleep(10);
			continue;
		}
		break;
	}
	if (ret == 0) break;
	recved += ret;
}
```

### Xử lí đa luồng

Để có thể điều khiển nhiều client cùng một lúc thì mỗi khi nhấn một nút điều khiển (RunCmd, Download, Upload) cần tạo ra một thread mới để thực hiện nó. Để tạo một thread mới ta sẽ sử dụng [CreateThread()](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createthread)

```c++
HANDLE CreateThread(
  [in, optional]  LPSECURITY_ATTRIBUTES   lpThreadAttributes,
  [in]            SIZE_T                  dwStackSize,
  [in]            LPTHREAD_START_ROUTINE  lpStartAddress,
  [in, optional]  __drv_aliasesMem LPVOID lpParameter,
  [in]            DWORD                   dwCreationFlags,
  [out, optional] LPDWORD                 lpThreadId
);
```

`lpStartAddress` sẽ là hàm để xử lí lệnh gửi đi, nhận về. `lpParameter` sẽ là một struct chứa các tham số mà hàm `lpStartAddress` cần sử dụng

Để quản lý việc giao tiếp, mỗi thread sẽ sử dụng một mutex để thực hiện lock và unlock. Việc này đảm bảo tại một thời điểm chỉ có duy nhất 1 tthread được phép truy cập và sử dụng socket để send/recv

Cơ chế này chặn việc bị nhiễu dữ liệu khi Server gửi liên tiếp nhiều yêu cầu (ví dụ: vừa RunCmd vừa Download). Các thread được tạo sau sẽ vào trạng thái chờ cho đến khi thread trước đó xử lí xong và unlock mutex, response của client luôn được nhận đầy đủ và chính xác vào đúng thread tương ứng

```c++
struct runCmdParams {
	HWND hDlg;
	HWND hBtn;
	SOCKET hSocket;
	CString cmdLine;
	std::mutex* pMutex;
};

void CClientDlg::OnBnDownload()
{
	HWND hBtn = ::GetDlgItem(this->m_hWnd, IDC_BUTTON_DOWNLOAD);
	::EnableWindow(hBtn, FALSE);
	char filePath[MAX_PATH] = { 0 };
	char fileName[MAX_PATH] = { 0 };
	if (GetDlgItemTextA(this->m_hWnd, IDC_EDIT_FILEDOWNLOAD, filePath, MAX_PATH) > 0 && GetDlgItemTextA(this->m_hWnd, IDC_EDIT_SAVENAME, fileName, MAX_PATH) > 0) {
		downloadParams* lpParameter = new downloadParams;
		lpParameter->hDlg = this->m_hWnd;
		lpParameter->hSocket = m_pClient->m_hSocket;
		lpParameter->hBtn = hBtn;
		strcpy_s(lpParameter->filePath, filePath);
		strcpy_s(lpParameter->fileName, fileName);
		lpParameter->pMutex = &m_socketMutex;
		CreateThread(NULL, 0, downloadThread, lpParameter, 0, NULL);
	}
	else {
		AfxMessageBox(L"Type Smth");
		::EnableWindow(hBtn, TRUE);
	}
}

DWORD runCmdThread(LPVOID lpParams) {
	runCmdParams* pParams = (runCmdParams*)lpParams;
	HWND hDlg = pParams->hDlg;
	HWND hBtn = pParams->hBtn;
	SOCKET hSocket = pParams->hSocket;
	CString cmdLine = pParams->cmdLine;
	std::mutex* pMutex = pParams->pMutex;

	pMutex->lock();
    //.........//
    pMutex->unlock();
    EnableWindow(hBtn, TRUE);
	delete pParams;
    return 0;
}
```



### a.) RUNCMD

Khi có cửa sổ mới ta sẽ tạo thêm một số button, edit control box trong `IDC_CLIENT_DIALOG`

- `IDC_EDIT_CMDINPUT`: Cho nhập lệnh để gửi đến client
- `IDC_EDIT_CMDOUTPUT`: Ghi lại output client gửi về. Có các thuộc tính sau: `Readonly`, `Multiline`
- `IDC_BUTTON_RUNCMD`: Khi được nhấn vào sẽ lấy lệnh trong `IDC_EDIT_CMDINPUT` rồi gửi client và nhận lại kết quả

Packet format:

- **Server -> Client: [4b opCode] [4b Size] [cmd]**
- **Client -> Server: [4b opCode] [4b Size] [output]**

Đối với Server: Để gửi thì khá đơn giản, ta chỉ cần dùng hàm `Send()` lấy buffer của input và size. Nhưng đối với hàm `Recv()` do chưa biết được size của output là như nào, ta sẽ phải làm cách khác:

- Ở client, output gửi về sẽ prepend thêm 4 byte opcode và 4 byte size

- Ở server: Recv 4 byte đầu, nếu đúng opCode mới đi tiếp, recv 4 byte size, rồi allocate 1 buffer với size đó. Sau đó tiếp tục recv cho đến khi nhận đủ size vào buffer, print buffer vào edit control box

Đối với Client: recv 8 byte và lưu lại opCode, expectedSize. Sau đó tuỳ vào opCode sẽ xử lí

- Để lưu lại lệnh từ server: malloc với size, rồi gọi `recv()` cho đến khi đủ expectedSize  
- Đối với RUNCMD: Để chạy và lưu output ta sẽ kết hợp sử dụng [CreatePipe()](https://learn.microsoft.com/en-us/windows/win32/api/namedpipeapi/nf-namedpipeapi-createpipe) và `CreateProcess()`
- Theo tài liệu của MSDN thì `CreatePipe()` sẽ tạo ra 2 handle để chương trình có thể ghi vào (hFile) hoặc đọc (hFile) 

```c++
BOOL CreatePipe(
  [out]          PHANDLE               hReadPipe,
  [out]          PHANDLE               hWritePipe,
  [in, optional] LPSECURITY_ATTRIBUTES lpPipeAttributes,
  [in]           DWORD                 nSize
);

typedef struct _SECURITY_ATTRIBUTES {
  DWORD  nLength;
  LPVOID lpSecurityDescriptor;
  BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;
```

- `bInheritHandle` trong struct `_SECURITY_ATTRIBUTES` sẽ để là `TRUE` để các chương trình khác inherit được handle
- Tiếp theo prepend `cmd.exe /c ` vào trước lệnh để tạo thành commandLine khi gọi `CreateProcess()`
- `CreateProcess()` sẽ được tạo với `bInheritHandles` là TRUE và trong struct `STARTUPINFO` sẽ đặt `STARTF_USESTDHANDLES` để sử dụng handle được người dùng đặt, hStdOutput và hStdError sẽ có giá trị của hWrite (Từ `CreatePipe()`) để ghi output hoặc error của lệnh
- Để đọc output/error ta sẽ dùng `ReadFile()` với parameter handle là `hRead` từ `CreatePipe()`. Nhưng do chưa biết được độ lớn của output/error nên ta sẽ đọc từng block (1024 byte) với `realloc()` để allocate size mới cho buffer mỗi khi parameter `lpNumberOfBytesRead` của `ReadFile()` > 0

```c++
BOOL ReadFile(
  [in]                HANDLE       hFile,
  [out]               LPVOID       lpBuffer,
  [in]                DWORD        nNumberOfBytesToRead,
  [out, optional]     LPDWORD      lpNumberOfBytesRead,
  [in, out, optional] LPOVERLAPPED lpOverlapped
);
```

```c++
// Read from hRead and expand finalResult using realloc until ReadFile read all data
while (ReadFile(hRead, tempBuf, sizeof(tempBuf), &bytesRead, NULL) && bytesRead > 0) {
    char* newResult = (char*)realloc(finalResult, totalResultSize + bytesRead);
    if (newResult) {
        finalResult = newResult;
        memcpy(finalResult + totalResultSize, tempBuf, bytesRead);
        totalResultSize += bytesRead;
    }
}
```

- Sau khi có bufferSize và buffer ta sẽ gửi output/error về Server theo packet format

### b) DOWNLOAD

Các dialog control sử dụng

- `IDC_EDIT_SAVENAME`: Cho người dùng nhập tên file sẽ lưu khi tải từ client
- `IDC_EDIT_FILEDOWNLOAD`: Đường dẫn đến file trên client
- `IDC_BUTTON_DOWNLOAD`: Khi bấm sẽ thực hiện chức năng tải file

Packet format:

- **Server -> Client: [4b opCode] [4b Size] [filePath]**
- **Client -> Server: [4b opCode] [4b Size] [fileData]**

Đối với Server:

- Sau ấn nút để gửi lệnh DOWNLOAD: Trước khi send sẽ tạo một buffer với format **[4b opCode] [4b Size] [filePath]** rồi gửi lên server
- Khi nhận lại size và data của file thành công thì gọi `CreateDirectoryA` để tạo thư mục `DownloadedFiles` lưu các file tải về
- Sau đó tạo 1 file trong chế độ viết (`CreateFileA()`) với tên trong `IDC_EDIT_SAVENAME` rồi lưu dữ liệu nhận được từ client vào file đó

Đôi với Client:

- Nếu nhận được opCode `DOWNLOAD`, lưu lại độ dài và đường dẫn đến file cần tạo 
- Gọi `CreateFileA()` để mở file trong chế độ đọc
- Nếu `CreateFile` thành công thì sẽ thực hiện lấy file size (`GetFileSizeEx()`) và `ReadFile()` vào 1 buffer. Nếu không thành công thì nội dung của file sẽ có nội dung thông báo lỗi, mã lỗi gửi về cho Server

### c) UPLOAD

Các dialog control sử dụng:

- `IDC_BUTTON_CHOOSEFILE`: Khi ấn nút này sẽ tạo cửa sổ chọn file và lưu đường dẫn vào `IDC_EDIT_FILELOC`
- `IDC_EDIT_FILELOC`: Chứa đường dẫn đến file mà người dùng muốn upload lên Client
- `IDC_EDIT_UPLOADLOC`: Chứa đường dẫn đến file mà người dùng muốn lưu ở trên Client
- `IDC_BUTTON_UPLOAD`: Khi thực hiện bấm nút sẽ thực hiện chức năng upload file

Packet format:

- **Server -> Client: [4b opCode] [4b uploadPathSize] [4b uploadPath] [4b fileSize] [4b fileData]**
- **Client -> Server: [4b opCode] [4b size] [4b result]**

Đối với Server:

- Khi ấn nút `IDC_BUTTON_CHOOSEFILE`, gọi class `CFileDialog` để tạo một cửa sổ file explorer cho người dùng chọn file. Sau khi chọn thành công thì sẽ lấy đường dẫn đến file đó và lưu vào `IDC_EDIT_FILELOC`
- Sau ấn nút để gửi lệnh UPLOAD: Lấy đường dẫn đến file muốn lưu trên client từ `IDC_EDIT_UPLOADLOC` và size. Lấy đường dẫn đến file muốn upload từ `IDC_EDIT_FILELOC`, `GetFileSizeEx()` để lấy size rồi `CreateFileA()` và `ReadFile()` vào 1 buffer. Tất cả các thông tin trên sẽ được lưu vào 1 buffer khác có format **[4b opCode] [4b uploadPathSize] [4b uploadPath] [4b fileSize] [4b fileData]**
- Chờ Client gửi lại kết quả, nếu là 1 tức là đã upload thành công. Nếu khác 1 thì tức là client đang gửi về mã lỗi, dùng MessageBox để thông báo mã lỗi đó

Đối với Client:

- Sau khi nhận được opCode và uploadPathSize, lưu lại uploadPath sau đó tiếp tục recv fileSize và fileData
- Tạo và ghi dữ liệu sử dụng `CreateFileA()` `WriteFile()` nếu bất kỳ 1 trong 2 hàm trên không thành công thì sẽ gửi về mã lỗi. Nếu không gửi về giá trị 1

### d) CLOSE

Các dialog control sử dụng:

- `IDC_BUTTON_CLOSE`

Packet format:

- **Server -> Client: [4b opCode] [4b 0]**

Đối với Server:

- Khi ấn nút `IDC_BUTTON_CLOSE` tạo một buffer với packet format và gửi tới Client
- Nếu thành công gọi `EndDialog()` để đóng cửa sổ điều khiển client

Đối với Client:

- Gọi `closesocket()` để đóng kết nối