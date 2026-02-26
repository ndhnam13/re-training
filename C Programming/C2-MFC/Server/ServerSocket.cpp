#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ServerSocket.h"
#include "ServerDlg.h"

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