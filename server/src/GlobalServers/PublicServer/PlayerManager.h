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
	time_t echo;  // 最后一次响应时间
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

	// 通过角色id和模块id查找处理器. 返回0 表示失败
	UI32 FindProcessor( UI32 RoleId, int mod);

	//查询各连接池ID,名称,数量
	bool QueryPoolList( int mod, int* szIdPool, int nMaxSize = 0 );

	// 添加处理器
	bool AddProcessor(int id, const char* szProcessorName, size_t nThreadCount, const char* szADOConnStr);

	// 查找处理机
	CAsyncProcesser* FindProcessor(int nProcessorID);

	// 查找处理机
	CAsyncProcesser* FindProcessor(const char* szProcessorName);

	// 注册工作
	bool Call(int nProcessorId, LPASYNCPROCFUNC func, CMsgTyped* pParamList);

	// 注册工作
	bool Call(const char* szProcessorName, LPASYNCPROCFUNC func, CMsgTyped* pParamList);

	// 删除处理机
	bool RemoveProcessor(int nProcessorID);

	// 删除处理机
	bool RemoveProcessor(const char* szProcessorName);

private:
	vector<ServerInfo*> m_ServerList;
};

