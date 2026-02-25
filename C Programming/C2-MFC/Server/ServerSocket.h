#pragma once
#include <afxsock.h>
#include <afxdialogex.h>
#include <vector>

class CServerDlg;

class CServerSocket : public CAsyncSocket {
	public: 
		CServerSocket(CServerDlg* pDlg);
		virtual ~CServerSocket();

		BOOL StartListen(UINT port);
	protected:
		virtual void OnAccept(int nErrorCode);
		virtual void OnClose(int nErrorCode);

	private:
		CServerDlg* m_pDlg;
};