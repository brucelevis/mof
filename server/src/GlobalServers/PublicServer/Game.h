/*-------------------------------------------------------------------------
	created:	2010/08/24  18:31
	filename: 	e:\Project_SVN\Server\ServerDemo\Game.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/


#pragma once

#include "MessageEnum.h"
#include "GameConfig.h"
#include "PlayerManager.h"
#include "ErrorCode.h"
#include "ThreadPool/ThreadPool.h"
#include "RegisterIOProc.h"
#include "InterfaceManager.h"

class IHaveSafeQueue
{
public:
	IHaveSafeQueue(){}
	virtual ~IHaveSafeQueue(){}

public:
	void SendMessage(CMsgTyped* pmsg)
	{
		m_msgQueue.PushMessage(pmsg);
	}

	void SendMessageNoWait(CMsgTyped* pmsg)
	{
		m_msgQueue.PushMessageNoWait(pmsg);
	}

	void MsgWait()
	{
		m_msgQueue.MsgWait();
	}

	CMsgTyped* GetMessage()
	{
		return m_msgQueue.GetMessage();
	}

protected:
	CSafeQueue m_msgQueue;
};

class CGame : public CriticalObject, public IHaveSafeQueue
{
public:
	CGame(void);
	~CGame(void);

public:
	// 初始化
	bool Initialize(const char* config);

	// 运行
	void Run();

	// 释放
	void UnInitalize();

public:
	// 发送客户端消息
	bool SendClientMessage(PersistID& id, CMsgTyped* pmsg);

	// 断开客户端
	bool Disconnect(PersistID& id);

	// 发送退出消息
	void SendQuitMessage();

	// 注册处理函数
	void RegisterIOMessageProc(bool system, UI32 msgId, IGlobalMessageProc* proc);

protected:
	// 初始化处理函数
	void OnRegisterIoMsgProc();

	// 命令行线程
	//static DWORD  WaitCmdLine(void* lpParam); windows
	static void* WaitCmdLine( void* lpParam ); //linux

protected:
	// 用于调用堆栈DUMP
	void OnIOMessageDump(CMsgTyped* pmsg);
	void OnCmdLineDump(StringArray& sa);
	void OnComponentMessageDump(CMsgTyped* pmsg);
	void OnHeartBeatDump(UI32 nBeat);

	void OnIOMessage(CMsgTyped* pmsg);
	void OnCmdLine(StringArray& sa);
	void OnComponentMessage(CMsgTyped* pmsg);
	void OnHeartBeat(UI32 nBeat);

protected:
	// 系统消息1
	void OnConnect(PersistID& id);
	void OnDisconnect(PersistID& id);

protected:
	// 系统请求

	// 逻辑请求

protected:
	// 组件消息
	void OnQuitMessage(CMsgTyped* pmsg);

	// 逻辑回应


private:
	// 用于管理输入线程
	//HANDLE m_hInputThread; //windows
	//WEvent m_hInputThread;
	pthread_t m_hInputThread; //linux

	//HANDLE m_hInputThreadExit; //windows
	bool m_bRunning;

private:
	CGameConfig m_gameConfig;
	CAsyncProcMgr m_AsyncProcMgr;
	CServerList m_ServerList;
	CInterfaceManager m_IFMgr;

	// 消息处理管理器
	vector<IGlobalMessageProc*> m_sysIoProc;
	vector<IGlobalMessageProc*> m_customIoProc;
};

extern CGame* _GAME;
extern CGameConfig* _CONFIG;
extern CAsyncProcMgr* _ASYNCPROC;
extern CServerList* _SERVERLIST;
extern CInterfaceManager* _IFMGR;

