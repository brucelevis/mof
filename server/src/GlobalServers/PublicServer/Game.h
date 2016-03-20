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
	// ��ʼ��
	bool Initialize(const char* config);

	// ����
	void Run();

	// �ͷ�
	void UnInitalize();

public:
	// ���Ϳͻ�����Ϣ
	bool SendClientMessage(PersistID& id, CMsgTyped* pmsg);

	// �Ͽ��ͻ���
	bool Disconnect(PersistID& id);

	// �����˳���Ϣ
	void SendQuitMessage();

	// ע�ᴦ����
	void RegisterIOMessageProc(bool system, UI32 msgId, IGlobalMessageProc* proc);

protected:
	// ��ʼ��������
	void OnRegisterIoMsgProc();

	// �������߳�
	//static DWORD  WaitCmdLine(void* lpParam); windows
	static void* WaitCmdLine( void* lpParam ); //linux

protected:
	// ���ڵ��ö�ջDUMP
	void OnIOMessageDump(CMsgTyped* pmsg);
	void OnCmdLineDump(StringArray& sa);
	void OnComponentMessageDump(CMsgTyped* pmsg);
	void OnHeartBeatDump(UI32 nBeat);

	void OnIOMessage(CMsgTyped* pmsg);
	void OnCmdLine(StringArray& sa);
	void OnComponentMessage(CMsgTyped* pmsg);
	void OnHeartBeat(UI32 nBeat);

protected:
	// ϵͳ��Ϣ1
	void OnConnect(PersistID& id);
	void OnDisconnect(PersistID& id);

protected:
	// ϵͳ����

	// �߼�����

protected:
	// �����Ϣ
	void OnQuitMessage(CMsgTyped* pmsg);

	// �߼���Ӧ


private:
	// ���ڹ��������߳�
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

	// ��Ϣ���������
	vector<IGlobalMessageProc*> m_sysIoProc;
	vector<IGlobalMessageProc*> m_customIoProc;
};

extern CGame* _GAME;
extern CGameConfig* _CONFIG;
extern CAsyncProcMgr* _ASYNCPROC;
extern CServerList* _SERVERLIST;
extern CInterfaceManager* _IFMGR;

