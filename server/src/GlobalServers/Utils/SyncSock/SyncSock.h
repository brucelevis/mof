/*-------------------------------------------------------------------------
	created:	2010/08/16  16:10
	filename: 	e:\Project_SVN\Server\Utils\SyncSock\SyncSock.h
	author:		Deeple

	purpose:	����socket ��ʵ��
---------------------------------------------------------------------------*/
#pragma once

#include <sys/socket.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <queue>
#include "../WShellAPI/eWshell.h"
#include "../MessageQueue/MsgQueue.h"

using namespace std;

// Socket API
bool InitSocket();
SOCKET ConnectServer(const char* addr, u_short uPort);
void CloseConn(SOCKET& s);
bool SendData(SOCKET& s, void* pData, int nLen);
bool RecvData(SOCKET& s, void* pData, int nLen);
bool SendMsg(SOCKET& s, void* pData, int nLen);
bool RecvMsg(SOCKET& s, char* pRecvBuf, size_t& nBufLen);

// ����ʽsocket
class CSyncSocket : public CriticalObject
{
public:
	CSyncSocket();
	virtual ~CSyncSocket();

	bool Open(const char* ServerIP, u_short uPort);
	void Close();
	bool InUse(){return m_bInuse;}

	// ͬ��������Ϣ�� ���ص�CMsgTyped ����һ��Ҫ��ʱɾ��
	CMsgTyped* SyncSendMessage(CMsgTyped* pmsg);

protected:
	SOCKET m_connSock;
	string m_szIP;
	u_short m_uPort;

	char m_recvBuf[0xFFFF];

	bool m_bInuse;
};

// ����socket ������
class CSyncSocketGroup
{
public:
	CSyncSocketGroup();
	virtual ~CSyncSocketGroup();

	// ������
	bool Open(int count, const char* ServerIP, u_short uPort);
	void DisconnectAll();
	void Close();

	// ͬ��������Ϣ�� ���ص�CMsgTyped ����һ��Ҫ��ʱɾ��
	CMsgTyped* SyncSendMessage(CMsgTyped* pmsg);

protected:
	vector<CSyncSocket*> m_connSet;
};

// �����߳�
struct SyncSockThread
{
	pthread_t m_hWorkThread;
	CSafeQueue m_MessageQueue;
	WEvent m_exitEvent;
};

// ʵ���̳߳�
class CAsyncSocket
{
public:
	CAsyncSocket();
	virtual ~CAsyncSocket();

public:
	// ��
	bool Open(const char* ServerIP, u_short uPort);

	// �ر�
	void ShutDown();

	// �����첽��Ϣ
	bool SendMessage(CMsgTyped* pMsg, bool bFailIFBadSock = false);

	// ������Ϣ
	bool ProcessMessages();

	// ������Ч
	bool IsConnected(){return m_hConnSock != INVALID_SOCKET;}

protected:
	// �����Ӧ��Ϣ
	virtual void OnReceive(CMsgTyped* pmsg);

	// �ڽ����߳��е��ã����ע���̰߳�ȫ
	virtual void OnConnected(){};
	virtual void OnDisconnected(){};
	virtual void OnConnectFailed(size_t nRetry){};

protected:
	static void* SendProc(void * lpParam);
	static void* RecvProc(void * lpParam);

protected:
	// �����߳�
	SyncSockThread m_SendThread;
	SyncSockThread m_RecvThread;

	// �Ƿ�������
	bool m_bRunning;

	// ��������
	SOCKET m_hConnSock;
	string m_szIP;
	u_short m_uPort;

	// ���ջ���
    char* m_pRecvBuf;
};

