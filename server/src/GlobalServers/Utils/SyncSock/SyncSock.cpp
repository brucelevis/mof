/*-------------------------------------------------------------------------
	created:	2010/08/16  16:10
	filename: 	e:\Project_SVN\Server\Utils\SyncSock\SyncSock.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#include "SyncSock.h"
#include <netinet/in.h>
#include <arpa/inet.h>

bool InitSocket()
{
	return true;
}

SOCKET ConnectServer(const char* addr, u_short uPort)
{
	SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ConnectSocket == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = addr ? inet_addr(addr) : 0;
	clientService.sin_port = ::htons(uPort);

	if (connect(ConnectSocket, (struct sockaddr *)&clientService, sizeof(clientService)) != 0)
	{
		close(ConnectSocket);
		return INVALID_SOCKET;
	}

	int nodelay = 1;
	setsockopt(ConnectSocket,IPPROTO_TCP, TCP_NODELAY,(char*)&nodelay, sizeof(int));

	return ConnectSocket;
}

void CloseConn(SOCKET& s)
{
	close(s);
	s = INVALID_SOCKET;
}

bool SendData(SOCKET& s, void* pData, int nLen)
{
	if (!pData)
		return false;

	char * chData = (char*) pData;

	int nSent = 0;

	while (nSent < nLen)
	{
		int n = ::send(s, chData + nSent, nLen - nSent, 0);
		if (n < 1)
		{
			CloseConn(s);
			return false;
		}
		nSent += n;
	}

	return true;
}

bool RecvData(SOCKET& s, void* pData, int nLen)
{
	if (!pData)
		return false;

	char * chData = (char*) pData;

	int nRecvd = 0;

	while (nRecvd < nLen)
	{
		int n = ::recv(s, chData + nRecvd, nLen - nRecvd, 0);
		if (n < 1)
		{
			CloseConn(s);
			return false;
		}
		nRecvd += n;
	}

	return true;
}

bool SendMsg(SOCKET& s, void* pData, int nLen)
{
	if (SendData(s, &nLen, sizeof(int))
		&& SendData(s, pData, nLen))
	{
		return true;
	}
	return false;
}

bool RecvMsg(SOCKET& s, char* pRecvBuf, size_t& nBufLen)
{
	size_t nMsgLen = 0;

	if (!RecvData(s, &nMsgLen, sizeof(int)))
	{
		return false;
	}

	if (nMsgLen > nBufLen)
	{
		CloseConn(s);
		return false;
	}

	if (!RecvData(s, pRecvBuf, (int)nMsgLen))
	{
		return false;
	}

	nBufLen = nMsgLen;

	return true;
}


// class CSyncSocket
// ----------------------------------------------------------------------
CSyncSocket::CSyncSocket()
{
	m_bInuse = false;
	m_connSock = INVALID_SOCKET;
}

CSyncSocket::~CSyncSocket()
{
	Close();
}

bool CSyncSocket::Open(const char* ServerIP, u_short uPort)
{
	m_connSock = ConnectServer(ServerIP, uPort);

	if (ServerIP)
	{
		m_szIP = ServerIP;
	}

	m_uPort = uPort;

	return (m_connSock != INVALID_SOCKET);
}

void CSyncSocket::Close()
{
	CloseConn(m_connSock);
}

CMsgTyped* CSyncSocket::SyncSendMessage(CMsgTyped* pmsg)
{
	SMART_LOCK_THIS();

	m_bInuse = true;

	if (!pmsg)
	{
		m_bInuse = false;

		return NULL;
	}

	// 发送数据并接受回应
	// 处理请求
	bool bSend = SendMsg(m_connSock, (void*)pmsg->GetData(), (int)pmsg->GetLength());
	if (!bSend)
	{
		// 如果发送失败，重连
		::CloseConn(m_connSock);
		m_connSock = ConnectServer(m_szIP.c_str(), m_uPort);

		if (m_connSock != INVALID_SOCKET)
		{
			bSend = SendMsg(m_connSock, (void*)pmsg->GetData(), (int)pmsg->GetLength());
		}
	}

	if (!bSend)
	{
		m_bInuse = false;

		return NULL;
	}

	CMsgTyped* pRet = NULL;

	// 接收回调
	size_t recv_buf_len = sizeof(m_recvBuf);

	if (RecvMsg(m_connSock, m_recvBuf, recv_buf_len))
	{
		pRet = new CMsgTyped(recv_buf_len, m_recvBuf);
	}

	m_bInuse = false;

	return pRet;
}

// class CSyncSocketGroup
// -------------------------------------------------------------
CSyncSocketGroup::CSyncSocketGroup()
{

}

CSyncSocketGroup::~CSyncSocketGroup()
{
	Close();
}

bool CSyncSocketGroup::Open(int count, const char* ServerIP, u_short uPort)
{
	Close();

	CheckMaxValue(count, 32);
	CheckMinValue(count, 1);

	m_connSet.resize(count);

	bool bConnected = true;
	for (size_t i=0; i < m_connSet.size(); i++)
	{
		m_connSet[i] = new CSyncSocket;
		if (!m_connSet[i]->Open(ServerIP, uPort))
		{
			bConnected = false;
			break;
		}
	}

	if (!bConnected)
	{
		Close();
		return false;
	}

	return true;
}

void CSyncSocketGroup::DisconnectAll()
{
	for (size_t i=0; i < m_connSet.size(); i++)
	{
		if (m_connSet[i])
		{
			m_connSet[i]->Close();
		}
	}
}

void CSyncSocketGroup::Close()
{
	for (size_t i=0; i < m_connSet.size(); i++)
	{
		if (m_connSet[i])
		{
			m_connSet[i]->Close();
			delete m_connSet[i];
		}
	}
	m_connSet.clear();
}

CMsgTyped* CSyncSocketGroup::SyncSendMessage(CMsgTyped* pmsg)
{
	if (m_connSet.size() == 0)
	{
		return NULL;
	}

	for (size_t i=0; i < m_connSet.size(); i++)
	{
		if (!m_connSet[i]->InUse())
		{
			return m_connSet[i]->SyncSendMessage(pmsg);
		}
	}

	return m_connSet[0]->SyncSendMessage(pmsg);
}

// class SyncSock
//-------------------------------------------------------------
#define AUX_RECV_BUF_SIZE (1024 * 1024)

CAsyncSocket::CAsyncSocket()
{
	m_hConnSock = INVALID_SOCKET;

	m_SendThread.m_hWorkThread = NULL;
	m_RecvThread.m_hWorkThread = NULL;

	m_bRunning = false;
}

CAsyncSocket::~CAsyncSocket()
{
	ShutDown();
}

bool CAsyncSocket::Open(const char* ServerIP, u_short uPort)
{
	if (m_bRunning)
	{
		ShutDown();
	}

	if (!ServerIP)
	{
		return false;
	}

	m_szIP = ServerIP;
	m_uPort = uPort;

	m_bRunning = true;

    pthread_create(&m_RecvThread.m_hWorkThread, NULL, RecvProc, (void*)this);
    pthread_create(&m_SendThread.m_hWorkThread, NULL, SendProc, (void*)this);

	return true;
}

void CAsyncSocket::ShutDown()
{
	if (!m_bRunning)
	{
		return;
	}

	m_bRunning = false;

	// 关闭网络连接
	CloseConn(m_hConnSock);

	// 等待0.5秒
	m_SendThread.m_exitEvent.WaitEvent(500);
	m_RecvThread.m_exitEvent.WaitEvent(500);

	// 关闭句柄
	pthread_cancel(m_SendThread.m_hWorkThread);
	pthread_cancel(m_RecvThread.m_hWorkThread);

	close(m_SendThread.m_hWorkThread);
	close(m_RecvThread.m_hWorkThread);

	// 清理消息队列
	m_RecvThread.m_MessageQueue.Clear();
	m_SendThread.m_MessageQueue.Clear();
}

bool CAsyncSocket::SendMessage(CMsgTyped* pMsg, bool bFailIFBadSock)
{
	if (bFailIFBadSock)
	{
		if (m_hConnSock == INVALID_SOCKET)
		{
			return false;
		}
	}

	// 堆积超过10万条消息的时, 就停止
	if (m_SendThread.m_MessageQueue.MsgCount() > 204800)
	{
		m_SendThread.m_MessageQueue.Clear();
		return false;
	}

	m_SendThread.m_MessageQueue.PushMessageNoWait(pMsg, false);

	return true;
}

bool CAsyncSocket::ProcessMessages()
{
	bool bRet = false;
	CMsgTyped* pmsg = NULL;
	while ((pmsg = m_RecvThread.m_MessageQueue.GetMessage()) != NULL)
	{
		bRet = true;
		try
		{
			OnReceive(pmsg);
		}
		catch (...)
		{
		}
		delete pmsg;
	}

	return bRet;
}

void CAsyncSocket::OnReceive(CMsgTyped* pmsg)
{
	// 处理回调
}

void* CAsyncSocket::SendProc(void* lpParam)
{
	CAsyncSocket* pThis = (CAsyncSocket*)lpParam;

	if (!pThis)
	{
		return 0;
	}

	CMsgTyped* pSendMsg = NULL;

	SOCKET& s = pThis->m_hConnSock;

	while (pThis->m_bRunning)
	{
		// 如果网络连接无效, 等待
		if (s == INVALID_SOCKET)
		{
			usleep(1000000);
			continue;
		}

		// 获取消息
		pSendMsg = pThis->m_SendThread.m_MessageQueue.GetMessage();
		if (!pSendMsg)
		{
			usleep(10000);
			continue;
		}

		// 发送数据
		// 处理请求
		SendMsg(s, (void*)pSendMsg->GetData(), (int)pSendMsg->GetLength());

		delete pSendMsg;
	}

	pThis->m_SendThread.m_exitEvent.SetEvent();

	return 0;
}

void* CAsyncSocket::RecvProc(void* lpParam)
{
	CAsyncSocket* pThis = (CAsyncSocket*)lpParam;

	if (!pThis)
	{
		return 0;
	}

	char* recv_data = new char[AUX_RECV_BUF_SIZE];
	size_t recv_buf_len;

	SOCKET& s = pThis->m_hConnSock;

	// 连接失败次数
	int nRetryConnect = 1;

	while (pThis->m_bRunning)
	{
		// 维护socket连接
		if (s == INVALID_SOCKET)
		{
			s = ConnectServer(pThis->m_szIP.c_str(), pThis->m_uPort);
			if (s == INVALID_SOCKET)
			{
				// 连接断开
				pThis->OnConnectFailed(nRetryConnect ++);

				usleep(1000000);
				continue;
			}

			// 重新连接, 通知逻辑
			pThis->OnConnected();
		}

		// 接收数据
		recv_buf_len = AUX_RECV_BUF_SIZE;
		if (RecvMsg(s, recv_data, recv_buf_len))
		{
			CMsgTyped response(recv_buf_len, recv_data);
			pThis->m_RecvThread.m_MessageQueue.PushMessage(&response);
		}
		else
		{
			// 连接断开
			pThis->OnDisconnected();
		}
	}

	delete [] recv_data;

	pThis->m_RecvThread.m_exitEvent.SetEvent();

	return 0;
}

