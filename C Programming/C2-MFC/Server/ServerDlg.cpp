
// ServerDlg.cpp : implementation file
// Run CMD, List Process, Down/up file to 

#include "pch.h"
#include "framework.h"
#include "Server.h"
#include "ServerDlg.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CServerDlg dialog
CServerDlg::CServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVER_DIALOG, pParent),
	m_ServerSocket(this) // Pass the CServerDlg pointer when initializing server socket
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ClientBox); // Link control to variable
}

BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CServerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CServerDlg::OnBnClickedButtonListen)
	ON_LBN_SELCHANGE(IDC_LIST1, &CServerDlg::OnLbnSelchangeList1)
	ON_BN_CLICKED(IDC_BUTTON3, &CServerDlg::OnBnClickedButtonStopListening)
END_MESSAGE_MAP()


// CServerDlg message handlers
BOOL CServerDlg::OnInitDialog()
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

void CServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CServerDlg::OnPaint()
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

HCURSOR CServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// Avoid closing the window when pressing Enter or Escape
BOOL CServerDlg::PreTranslateMessage(MSG* pMsg) {
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

// Handling UI Logic
void CServerDlg::OnBnClickedButton1() {
	MessageBoxA(NULL, "123", "456", MB_YESNO);
}

void CServerDlg::AddClientToList(CString clientInfo, CServerSocket* pClient) {
	m_ClientsList.push_back(pClient); // Add the client socket address to m_Clients arraylist
	int nIndex =  m_ClientBox.AddString(clientInfo);
	m_ClientBox.SetItemDataPtr(nIndex, pClient);
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

void CServerDlg::OnBnClickedButtonListen() {
	/* TODO: Use winsock to create a server socket waiting for connection 
	* List total clients connected at the moment
	* On connect CreateMessageBox alert, inc clients
	*/
	//192.168.8.128:7676
	if (!listenState) {
		listenState = TRUE;
		AfxSocketInit();
		m_ServerSocket.StartListen(7676);
		MessageBoxA(NULL, "Listening", "Server", MB_OK);
	}
	else {
		MessageBoxA(NULL, "Already Listening", "Server", MB_OK);
	}
	
}

void CServerDlg::OnBnClickedButtonStopListening()
{
	if (listenState) {
		listenState = FALSE;
		m_ServerSocket.Close();
		MessageBoxA(NULL, "Stopped Listening", "Server", MB_OK);
	}
	else {
		MessageBoxA(NULL, "Already stopped listening", "Server", MB_OK);
	}
}

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
	
	CClientDlg cDlg(pClient, this);
	cDlg.DoModal();
}
