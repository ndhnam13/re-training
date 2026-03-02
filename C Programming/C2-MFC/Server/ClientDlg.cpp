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

//Button and control handler
void CClientDlg::OnBnClickedButtonRuncmd()
{
	/* TODO: Get input from input edit control box
	* Prepend input with opCode and size
	* Send/Recv
	* Print output into output edit control box
	*/
	CString cmdLine;
	// Get input from edit control box
	if (GetDlgItemText(IDC_EDIT_CMDINPUT, cmdLine)) {
		SetDlgItemText(IDC_EDIT_CMDOUTPUT, L"");

		//Prepending cmdLine with 4 bytes opCode and 4 bytes of size
		int cmdLineLen = (cmdLine.GetLength() + 1) * sizeof(wchar_t);
		int opCode = RUNCMD;
		std::vector<byte> sendBuffer(sizeof(int)*2 + cmdLineLen);
		memcpy(sendBuffer.data(), &opCode, sizeof(int));
		memcpy(sendBuffer.data() + sizeof(int), &cmdLineLen, sizeof(int));
		memcpy(sendBuffer.data() + sizeof(int) * 2, cmdLine, cmdLineLen);
		// Send to client
		if (m_pClient->Send(sendBuffer.data(), sendBuffer.size(), 0) != SOCKET_ERROR) {
			int expectedSize = 0;
			int recved = 0;
			// Recv from client
			// Get first 4 bytes (size)
			// If Receive() returns error WSAEWOULDBLOCK, then Sleep() until we get enough data
			while (recved < 4) {
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
			if (recved == 4) {
				// Allocate buffer 
				std::vector<char> outputBuffer(expectedSize + 1, 0);
				// Get the recv bytes
				int received = 0;
				while (received < expectedSize) {
					received += m_pClient->Receive(outputBuffer.data() + received, expectedSize - received);
				}
				SetDlgItemTextA(this->m_hWnd, IDC_EDIT_CMDOUTPUT, outputBuffer.data());
				SetDlgItemText(IDC_EDIT_CMDINPUT, L"");
			} 
			else SetDlgItemText(IDC_EDIT_CMDOUTPUT, L"No resp from client");
		}
		else AfxMessageBox(L"Send Failed");
	}
	else AfxMessageBox(L"Type Smth");
}

void CClientDlg::OnBnDownload()
{
	char filePath[MAX_PATH];
	char fileName[MAX_PATH];
	if (GetDlgItemTextA(this->m_hWnd, IDC_EDIT_FILEDOWNLOAD, filePath, MAX_PATH) > 0 && GetDlgItemTextA(this->m_hWnd, IDC_EDIT_SAVENAME, fileName, MAX_PATH) > 0) {
		//Prepend with opCode and size
		int filePathLen = strlen(filePath) + 1;
		int opCode = DOWNLOAD;
		std::vector<byte> sendBuffer(sizeof(int)*2 + filePathLen);
		memcpy(sendBuffer.data(), &opCode, sizeof(int));
		memcpy(sendBuffer.data() + sizeof(int), &filePathLen, sizeof(int));
		memcpy(sendBuffer.data() + sizeof(int)*2, filePath, filePathLen);
		//Send to client
		if (m_pClient->Send(sendBuffer.data(), sendBuffer.size(), 0) != SOCKET_ERROR) {
			unsigned int expectedSize = 0;
			int recved = 0;
			//Recv from client
			// Get 4 bytes size
			// If Receive() returns error WSAEWOULDBLOCK, then Sleep() until we get enough data
			while (recved < 4) {
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
			if ( recved == 4) {
				//Allocate buffer
				std::vector<byte> fileBuffer(expectedSize);
				int recieved = 0;
				//Recv file bytes into fileBuffer
				while (recieved < expectedSize) {
					recieved += m_pClient->Receive(fileBuffer.data() + recieved, expectedSize - recieved);
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
						AfxMessageBox(L"Cant write file");
					}
				}
				CloseHandle(hFile);
			}
		}
		else AfxMessageBox(L"File download failed");
	}
	else AfxMessageBox(L"Type smth");
}

void CClientDlg::OnBnUpload()
{
	// TODO: Add your control notification handler code here
}


