#pragma once
#include <vector>
#include "afxdialogex.h"
#include "ServerSocket.h"

// CClientDlg dialog

class CClientDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CClientDlg)

public:
	CClientDlg(CServerSocket* pClient, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CClientDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	//afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	afx_msg void OnBnClickedButtonRuncmd();
	afx_msg void OnBnDownload();
	afx_msg void OnBnUpload();
private:
	MSG* pMsg;
	CServerSocket* m_pClient;	
};
