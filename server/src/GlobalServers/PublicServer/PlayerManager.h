/*-------------------------------------------------------------------------
	created:	2010/09/28  14:18
	filename: 	e:\Project_SVN\Server\GlobalServer\PlayerManager.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/



#pragma once

#include "../Interface/GameType.h"
#include "../Interface/ShellHandle.h"

struct ServerInfo
{
public:
	ServerInfo()
	{
		serverid = 0;
		::time(&echo);
	}

	bool Valid(){return conn.Valid();}

	string servername;
	int serverid;
	PersistID conn;
	time_t echo;  // ���һ����Ӧʱ��
	string ip;
	int mod_id;
};

class CServerList : public CriticalObject, public IGlobal
{
public:
	CServerList();
	virtual ~CServerList();

public:
	ServerInfo* FindServer(int nServerId, int mod_id);
	ServerInfo* FindServer(PersistID& id);
	ServerInfo* AddServer(int nServerId, const char* szServerName, const char* ip, PersistID id, int mod_id);
	bool RemoveServer(PersistID& id);

	bool Echo(PersistID& id);
	void OnHeartBeat(int nBeat);
	void ServerDisconnected(PersistID& connId);

public:
	void RegisterCallBack(UI32 msgId, IGlobalMessageProc* handle);
	bool SendMessage(PersistID& id, CMsgTyped* pmsg);

	bool SendMessage(int serverid, int modid, CMsgTyped* pmsg);

	// ͨ����ɫid��ģ��id���Ҵ�����. ����0 ��ʾʧ��
	UI32 FindProcessor( UI32 RoleId, int mod);

	//��ѯ�����ӳ�ID,����,����
	bool QueryPoolList( int mod, int* szIdPool, int nMaxSize = 0 );

	// ��Ӵ�����
	bool AddProcessor(int id, const char* szProcessorName, size_t nThreadCount, const char* szADOConnStr);

	// ���Ҵ����
	CAsyncProcesser* FindProcessor(int nProcessorID);

	// ���Ҵ����
	CAsyncProcesser* FindProcessor(const char* szProcessorName);

	// ע�Ṥ��
	bool Call(int nProcessorId, LPASYNCPROCFUNC func, CMsgTyped* pParamList);

	// ע�Ṥ��
	bool Call(const char* szProcessorName, LPASYNCPROCFUNC func, CMsgTyped* pParamList);

	// ɾ�������
	bool RemoveProcessor(int nProcessorID);

	// ɾ�������
	bool RemoveProcessor(const char* szProcessorName);

private:
	vector<ServerInfo*> m_ServerList;
};

