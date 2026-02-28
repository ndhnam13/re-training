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
	ON_BN_CLICKED(IDC_BUTTON2, &CClientDlg::OnBnDownload)
	ON_BN_CLICKED(IDC_BUTTON3, &CClientDlg::OnBnUpload)
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
		if (m_pClient->Send(sendBuffer.data(), (int)sendBuffer.size(), 0) != SOCKET_ERROR) {
			// Recv from client
			int expectedSize = 0;
			// Get first 4 bytes (size)
			if (m_pClient->Receive(&expectedSize, sizeof(int)) > 0) {
				// Allocate buffer 
				std::vector<wchar_t> outputBuffer(expectedSize / sizeof(wchar_t) + 1);
				// Get the recv bytes
				int received = 0;
				while (received < expectedSize) {
					received += m_pClient->Receive(outputBuffer.data() + (received / sizeof(wchar_t)), expectedSize - received);
				}
				SetDlgItemText(IDC_EDIT_CMDOUTPUT, outputBuffer.data());
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
	// TODO: Add your control notification handler code here
}

void CClientDlg::OnBnUpload()
{
	// TODO: Add your control notification handler code here
}


