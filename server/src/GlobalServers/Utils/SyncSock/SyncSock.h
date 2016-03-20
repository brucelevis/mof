/*-------------------------------------------------------------------------
	created:	2010/08/16  16:10
	filename: 	e:\Project_SVN\Server\Utils\SyncSock\SyncSock.h
	author:		Deeple

	purpose:	阻塞socket 的实现
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

// 阻塞式socket
class CSyncSocket : public CriticalObject
{
public:
	CSyncSocket();
	virtual ~CSyncSocket();

	bool Open(const char* ServerIP, u_short uPort);
	void Close();
	bool InUse(){return m_bInuse;}

	// 同步发送消息， 返回的CMsgTyped 对象一定要及时删除
	CMsgTyped* SyncSendMessage(CMsgTyped* pmsg);

protected:
	SOCKET m_connSock;
	string m_szIP;
	u_short m_uPort;

	char m_recvBuf[0xFFFF];

	bool m_bInuse;
};

// 堵塞socket 连接组
class CSyncSocketGroup
{
public:
	CSyncSocketGroup();
	virtual ~CSyncSocketGroup();

	// 打开连接
	bool Open(int count, const char* ServerIP, u_short uPort);
	void DisconnectAll();
	void Close();

	// 同步发送消息， 返回的CMsgTyped 对象一定要及时删除
	CMsgTyped* SyncSendMessage(CMsgTyped* pmsg);

protected:
	vector<CSyncSocket*> m_connSet;
};

// 工作线程
struct SyncSockThread
{
	pthread_t m_hWorkThread;
	CSafeQueue m_MessageQueue;
	WEvent m_exitEvent;
};

// 实现线程池
class CAsyncSocket
{
public:
	CAsyncSocket();
	virtual ~CAsyncSocket();

public:
	// 打开
	bool Open(const char* ServerIP, u_short uPort);

	// 关闭
	void ShutDown();

	// 发送异步消息
	bool SendMessage(CMsgTyped* pMsg, bool bFailIFBadSock = false);

	// 处理消息
	bool ProcessMessages();

	// 连接有效
	bool IsConnected(){return m_hConnSock != INVALID_SOCKET;}

protected:
	// 处理回应消息
	virtual void OnReceive(CMsgTyped* pmsg);

	// 在接收线程中调用，务必注意线程安全
	virtual void OnConnected(){};
	virtual void OnDisconnected(){};
	virtual void OnConnectFailed(size_t nRetry){};

protected:
	static void* SendProc(void * lpParam);
	static void* RecvProc(void * lpParam);

protected:
	// 工作线程
	SyncSockThread m_SendThread;
	SyncSockThread m_RecvThread;

	// 是否在运行
	bool m_bRunning;

	// 网络连接
	SOCKET m_hConnSock;
	string m_szIP;
	u_short m_uPort;

	// 接收缓冲
    char* m_pRecvBuf;
};

