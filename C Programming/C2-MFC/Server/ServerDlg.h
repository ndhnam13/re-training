
// ServerDlg.h : header file
//

#pragma once
#include <vector>
#include "ServerSocket.h"

// CServerDlg dialog
class CServerDlg : public CDialogEx
{
// Construction
public:
	CServerDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButtonListen();
	afx_msg void OnBnClickedButtonStopListening();
	afx_msg void OnLbnSelchangeList1();
	void AddClientToList(CString clientInfo, CServerSocket* pClient);
	void RemoveClientFromList(CServerSocket* pClient);
	void NewWindow(CServerSocket* pClient);
private:
	std::vector<CAsyncSocket*> m_ClientsList;
	//ServerSocket
	CServerSocket m_ServerSocket;
	CListBox m_ClientBox;
	BOOL listenState = FALSE;
};
