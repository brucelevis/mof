/*-------------------------------------------------------------------------
	created:	2010/08/23  14:42
	filename: 	e:\Project_SVN\Server\Utils\MessageQueue\MsgQueue.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#pragma once
#include "MsgBuf.h"
#include "../ThreadPool/ThreadPool.h"

#ifdef LINUX_
#include <semaphore.h>
#endif
#include <queue>

class CMsgQueue
{
public:
	CMsgQueue();
	~CMsgQueue();

	void PushMessage(CMsgTyped* pmsg, bool bSetAsReadBuf = true);
	void PushMessageNoWait(CMsgTyped* pmsg, bool bSetAsReadBuf = true);
	void MsgWait();
	CMsgTyped* GetMessage();
	void Clear();
	size_t MsgCount();

private:
	std::queue<CMsgTyped*> m_msgQueue;
	#ifdef LINUX_
	pthread_cond_t m_hEventGo;
	pthread_mutex_t mutex_;
	#else
	HANDLE m_hEventGo;
	#endif
};

class CSafeQueue : public CriticalObject
{
public:
	CSafeQueue();
	~CSafeQueue();

	void PushMessage(CMsgTyped* pmsg, bool bSetAsReadBuf = true);
	void PushMessageNoWait(CMsgTyped* pmsg, bool bSetAsReadBuf = true);
	void MsgWait();
	CMsgTyped* GetMessage();
	void Clear();
	size_t MsgCount();

private:
	std::queue<CMsgTyped*> m_msgQueue;
	#ifdef LINUX_
	pthread_cond_t m_hEventGo;
	pthread_mutex_t mutex_;
	#else
	HANDLE m_hEventGo;
	#endif
};

