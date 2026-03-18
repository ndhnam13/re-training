// CClientDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Server.h"
#include "afxdialogex.h"
#include "ClientDlg.h"
#include "resource.h"

// CClientDlg dialog

IMPLEMENT_DYNAMIC(CClientDlg, CDialogEx)

CClientDlg::CClientDlg(CServerSocket* pClient, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
{
	m_pClient = pClient;
}

CClientDlg::~CClientDlg()
{
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD, &CClientDlg::OnBnDownload)
	ON_BN_CLICKED(IDC_BUTTON_UPLOAD, &CClientDlg::OnBnUpload)
	ON_BN_CLICKED(IDC_BUTTON_RUNCMD, &CClientDlg::OnBnClickedButtonRuncmd)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CClientDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_CHOOSEFILE, &CClientDlg::OnBnClickedButtonChoosefile)
END_MESSAGE_MAP()


// CClientDlg message handlers
BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;
}

//void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
//{
//	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
//	{
//		CAboutDlg dlgAbout;
//		dlgAbout.DoModal();
//	}
//	else
//	{
//		CDialogEx::OnSysCommand(nID, lParam);
//	}
//}

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Avoid closing the window when pressing Enter or Escape
BOOL CClientDlg::PreTranslateMessage(MSG* pMsg) {
	//Only get WM_KEYDOWN message at the Dialog window
	if (pMsg->hwnd == this->m_hWnd && pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;                // Do not process further
		}
	}
	return CWnd::PreTranslateMessage(pMsg);
}

// Callback functions parameters
struct runCmdParams {
	HWND hDlg;
	HWND hBtn;
	SOCKET hSocket;
	CString cmdLine;
	std::mutex* pMutex;
};
struct downloadParams {
	HWND hDlg;
	HWND hBtn;
	SOCKET hSocket;
	char filePath[MAX_PATH];
	char fileName[MAX_PATH];
	std::mutex* pMutex;
};
struct uploadParams {
	HWND hDlg;
	HWND hBtn;
	SOCKET hSocket;
	char filePath[MAX_PATH];
	char uploadPath[MAX_PATH];
	std::mutex* pMutex;
};

// Callback functions
DWORD runCmdThread(LPVOID lpParams) {
	runCmdParams* pParams = (runCmdParams*)lpParams;
	HWND hDlg = pParams->hDlg;
	HWND hBtn = pParams->hBtn;
	SOCKET hSocket = pParams->hSocket;
	CString cmdLine = pParams->cmdLine;
	std::mutex* pMutex = pParams->pMutex;

	pMutex->lock();
	//Prepending cmdLine with 4 bytes opCode and 4 bytes of size
	int cmdLineLen = (cmdLine.GetLength() + 1) * sizeof(wchar_t);
	int opCode = RUNCMD;
	std::vector<byte> sendBuffer(sizeof(int) * 2 + cmdLineLen);
	memcpy(sendBuffer.data(), &opCode, sizeof(int));
	memcpy(sendBuffer.data() + sizeof(int), &cmdLineLen, sizeof(int));
	memcpy(sendBuffer.data() + sizeof(int) * 2, cmdLine, cmdLineLen);
	// Send to client
	if (send(hSocket, (char*)sendBuffer.data(), sendBuffer.size(), 0) != SOCKET_ERROR) {
		int expectedSize = 0;
		int recved = 0;
		// Recv from client
		// Get first 4 bytes (opCode)
		int recvOpCode = 0;
		while (recved < sizeof(int)) {
			int ret = recv(hSocket, (char*)&recvOpCode + recved, sizeof(int) - recved, 0);
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
		// Only continue if returned opCode is RUNCMD
		if (recvOpCode == opCode) {
			// Get 4 bytes (size)
			// If Receive() returns error WSAEWOULDBLOCK, then Sleep() until we get enough data
			expectedSize = 0;
			recved = 0;
			while (recved < 4) {
				int ret = recv(hSocket, (char*)&expectedSize + recved, sizeof(int) - recved, 0);
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
			if (recved == 4) {
				// Allocate buffer 
				std::vector<char> outputBuffer(expectedSize + 1, 0);
				// Get the recv bytes
				int received = 0;
				while (received < expectedSize) {
					int ret = recv(hSocket, outputBuffer.data() + received, expectedSize - received, 0);
					if (ret == SOCKET_ERROR) {
						if (GetLastError() == WSAEWOULDBLOCK) {
							Sleep(10);
							continue;
						}
						break;
					}
					if (ret == 0) break;
					received += ret;
				}
				SetDlgItemTextA(hDlg, IDC_EDIT_CMDOUTPUT, outputBuffer.data());
				SetDlgItemTextW(hDlg, IDC_EDIT_CMDINPUT, L"");
			}
			else SetDlgItemTextW(hDlg, IDC_EDIT_CMDOUTPUT, L"No resp from client");
		}
	}
	else {
		char msg[50];
		sprintf_s(msg, "Send to client failed\nError: %d", GetLastError());
		MessageBoxA(NULL, msg, "Server", MB_OK | MB_ICONERROR);
	}

	pMutex->unlock();
	EnableWindow(hBtn, TRUE);
	delete pParams;
	return 0;
}

DWORD downloadThread(LPVOID lpParams) {
	downloadParams* pParams = (downloadParams*)lpParams;
	HWND hDlg = pParams->hDlg;
	HWND hBtn = pParams->hBtn;
	SOCKET hSocket = pParams->hSocket;
	char* filePath = pParams->filePath;
	char* fileName = pParams->fileName;
	std::mutex* pMutex = pParams->pMutex;

	pMutex->lock();
	//Prepend with opCode and size
	int filePathLen = strlen(filePath) + 1;
	int opCode = DOWNLOAD;
	std::vector<byte> sendBuffer(sizeof(int) * 2 + filePathLen);
	memcpy(sendBuffer.data(), &opCode, sizeof(int));
	memcpy(sendBuffer.data() + sizeof(int), &filePathLen, sizeof(int));
	memcpy(sendBuffer.data() + sizeof(int) * 2, filePath, filePathLen);
	//Send to client
	if (send(hSocket, (char*)sendBuffer.data(), sendBuffer.size(), 0) != SOCKET_ERROR) {
		unsigned int expectedSize = 0;
		int recved = 0;
		//Recv from client
		// Get 4 bytes opCode
		int recvOpCode = 0;
		while (recved < 4) {
			int ret = recv(hSocket, (char*)&recvOpCode + recved, sizeof(int) - recved, 0);
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
		// Only continue if returned opCode is RUNCMD
		if (recvOpCode == opCode) {
			// Get 4 bytes size
			// If Receive() returns error WSAEWOULDBLOCK, then Sleep() until we get enough data
			expectedSize = 0;
			recved = 0;
			while (recved < 4) {
				int ret = recv(hSocket, (char*)&expectedSize + recved, sizeof(int) - recved, 0);
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
			if (recved == 4) {
				//Allocate buffer
				std::vector<byte> fileBuffer(expectedSize);
				int recieved = 0;
				//Recv file bytes into fileBuffer
				while (recieved < expectedSize) {
					int ret = recv(hSocket, (char*)fileBuffer.data() + recieved, expectedSize - recieved, 0);
					if (ret == SOCKET_ERROR) {
						if (GetLastError() == WSAEWOULDBLOCK) {
							Sleep(10);
							continue;
						}
						break;
					}
					if (ret == 0) break;
					recieved += ret;
				}
				//Create folder to save downloaded files
				CreateDirectoryA(".\\DownloadedFiles\\", NULL);
				//CreateFile + WriteFile to save file
				char saveLocation[MAX_PATH];
				sprintf_s(saveLocation, ".\\DownloadedFiles\\%s", fileName);
				HANDLE hFile = CreateFileA(saveLocation, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, 0, NULL);
				if (hFile == INVALID_HANDLE_VALUE) {
					int err = GetLastError();
					if (err == ERROR_FILE_EXISTS)
						AfxMessageBox(L"File already exists, choose diff name");
				}
				else {
					if (!WriteFile(hFile, fileBuffer.data(), fileBuffer.size(), NULL, NULL)) {
						char msg[50];
						sprintf_s(msg, "Cant write file\nError: %d", GetLastError());
						MessageBoxA(NULL, msg, "Server", MB_OK | MB_ICONERROR);
					}
					else {
						char msg[MAX_PATH];
						sprintf_s(msg, "File saved to %s", saveLocation);
						MessageBoxA(NULL, msg, "Server", MB_OK | MB_ICONINFORMATION);
					}
				}
				CloseHandle(hFile);
			}
		}
	}
	else AfxMessageBox(L"File download failed");

	pMutex->unlock();
	EnableWindow(hBtn, TRUE);
	delete pParams;
	return 0;
}

DWORD uploadThread(LPVOID lpParams) {
	uploadParams* pParams = (uploadParams*)lpParams;
	HWND hDlg = pParams->hDlg;
	HWND hBtn = pParams->hBtn;
	SOCKET hSocket = pParams->hSocket;
	char* filePath = pParams->filePath;
	char* uploadPath = pParams->uploadPath;
	std::mutex* pMutex = pParams->pMutex;

	pMutex->lock();
	int uploadPathSize = strlen(uploadPath);
	int opCode = UPLOAD;
	HANDLE hFile = CreateFileA(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBoxA(hDlg, "Failed to open local file", "Error", MB_OK);
		return 1;
	}
	LARGE_INTEGER largeInt;
	GetFileSizeEx(hFile, &largeInt);
	unsigned int fileSize = largeInt.QuadPart;
	std::vector<char> fileBuffer(fileSize);
	DWORD bytesRead = 0;
	ReadFile(hFile, fileBuffer.data(), fileSize, &bytesRead, NULL);
	CloseHandle(hFile);

	std::vector <byte> sendBuffer(sizeof(int) * 2 + uploadPathSize + sizeof(int) + fileSize);
	memcpy(sendBuffer.data(), &opCode, sizeof(int));
	memcpy(sendBuffer.data() + sizeof(int), &uploadPathSize, sizeof(int));
	memcpy(sendBuffer.data() + sizeof(int) * 2, uploadPath, uploadPathSize);
	memcpy(sendBuffer.data() + sizeof(int) * 2 + uploadPathSize, &fileSize, sizeof(int));
	memcpy(sendBuffer.data() + sizeof(int) * 2 + uploadPathSize + sizeof(int), fileBuffer.data(), fileSize);

	if (send(hSocket, (const char*)sendBuffer.data(), sendBuffer.size(), 0) != SOCKET_ERROR) {
		int expectedSize = 0;
		int recved = 0;
		while (recved < 4) {
			int ret = recv(hSocket, (char*)&expectedSize + recved, sizeof(int) - recved, 0);
			if (ret == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAEWOULDBLOCK) {
					Sleep(10);
					continue;
				}
				break;
			}
			if (ret <= 0) break;
			recved += ret;
		}

		int uploadResult = 0;
		int received = 0;
		while (received < expectedSize) {
			int ret = recv(hSocket, (char*)&uploadResult + received, expectedSize - received, 0);
			if (ret == SOCKET_ERROR) {
				if (WSAGetLastError() == WSAEWOULDBLOCK) {
					Sleep(10);
					continue;
				}
				break;
			}
			if (ret <= 0) break;
			received += ret;
		}

		if (uploadResult == 1)
			MessageBoxW(hDlg, L"File upload succeded", L"Success", MB_OK);
		else {
			char msg[50];
			sprintf_s(msg, "File upload failed\nError: %d", uploadResult);
			MessageBoxA(hDlg, msg, "Server", MB_OK | MB_ICONERROR);
		}
	}
	else {
		char msg[50];
		sprintf_s(msg, "Send to client failed\nError: %d", WSAGetLastError());
		MessageBoxA(hDlg, msg, "Server", MB_OK | MB_ICONERROR);
	}

	pMutex->unlock();
	EnableWindow(hBtn, TRUE);
	delete pParams;
	return 0;
}

//Button and control handler
void CClientDlg::OnBnClickedButtonRuncmd()
{
	HWND hBtn = ::GetDlgItem(this->m_hWnd, IDC_BUTTON_RUNCMD);
	::EnableWindow(hBtn, FALSE);
	/* Get input from input edit control box
	* Prepend input with opCode and size
	* Send/Recv
	* Print output into output edit control box
	*/
	CString cmdLine;
	// Get input from edit control box
	if (GetDlgItemText(IDC_EDIT_CMDINPUT, cmdLine)) {
		SetDlgItemText(IDC_EDIT_CMDOUTPUT, L"Running CMD on client...");

		HWND hDlg = this->m_hWnd;
		SOCKET hSocket = m_pClient->m_hSocket;
		runCmdParams* lpParameter = new runCmdParams{hDlg, hBtn, hSocket, cmdLine, &m_socketMutex};
		CreateThread(NULL, 0, runCmdThread, lpParameter, 0, NULL);
	}
	else {
		AfxMessageBox(L"Type Smth");
		::EnableWindow(hBtn, TRUE);
	}
}

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

void CClientDlg::OnBnClickedButtonChoosefile()
{
	// Open a file dialog then get the path
	CFileDialog fileDlg(TRUE, NULL, NULL, 0, L"All Files (*.*)|*.*||", this);

	if (fileDlg.DoModal() == IDOK) {
		CStringA filePath;
		filePath = fileDlg.GetPathName();
		SetDlgItemTextA(this->m_hWnd, IDC_EDIT_FILELOC, filePath);
	}
}

void CClientDlg::OnBnUpload()
{
	HWND hBtn = ::GetDlgItem(this->m_hWnd, IDC_BUTTON_UPLOAD);
	::EnableWindow(hBtn, FALSE);
	/* Get the path from IDC_EDIT_FILELOC, get save location from IDC_EDIT_UPLOADLOC
	* Get uploadPath and size
	* Get file size, Open+Read the path into a buffer
	* BUILD UPLOAD PACKET
	[4b opCode] [4b uploadPath size] [uploadPath] [4b fileSize] [file bytes]
	* Send to client
	* Recv from client, if 1 => success, if 0 => failed
	*/
	char filePath[MAX_PATH] = { 0 };
	char uploadPath[MAX_PATH] = { 0 };

	if (GetDlgItemTextA(this->m_hWnd, IDC_EDIT_FILELOC, filePath, MAX_PATH) > 0 &&
		GetDlgItemTextA(this->m_hWnd, IDC_EDIT_UPLOADLOC, uploadPath, MAX_PATH) > 0) {

		uploadParams* lpParameter = new uploadParams;
		lpParameter->hDlg = this->m_hWnd;
		lpParameter->hBtn = hBtn;
		lpParameter->hSocket = m_pClient->m_hSocket;
		strcpy_s(lpParameter->filePath, filePath);
		strcpy_s(lpParameter->uploadPath, uploadPath);
		lpParameter->pMutex = &m_socketMutex;

		CreateThread(NULL, 0, uploadThread, lpParameter, 0, NULL);
	}
	else {
		AfxMessageBox(L"Type Smth");
		::EnableWindow(hBtn, TRUE);
	}
}

void CClientDlg::OnBnClickedButtonClose()
{
	int opCode = CLOSE;
	int msgLen = 0;
	std::vector <byte> sendBuffer(sizeof(int) * 2);
	memcpy(sendBuffer.data(), &opCode, sizeof(int));
	memcpy(sendBuffer.data() + sizeof(int), &msgLen, sizeof(int));
	//Send Close message to client then close the window
	if (m_pClient->Send(sendBuffer.data(), sendBuffer.size(), 0) != SOCKET_ERROR) {
		AfxMessageBox(L"Closed connection to client");
		EndDialog(0);
	}
	else {
		char msg[50];
		sprintf_s(msg, "Send to client failed\nError: %d", GetLastError());
		MessageBoxA(NULL, msg, "Server", MB_OK | MB_ICONERROR);
	}
}