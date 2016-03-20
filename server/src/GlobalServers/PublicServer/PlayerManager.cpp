/*-------------------------------------------------------------------------
	created:	2010/09/28  14:18
	filename: 	e:\Project_SVN\Server\GlobalServer\PlayerManager.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#include "Game.h"
#include "PlayerManager.h"


// class CServerList
// --------------------------------------------------------------

CServerList::CServerList()
{

}

CServerList::~CServerList()
{
	Lock();

	// 清理
	for (size_t i=0; i < m_ServerList.size(); i++)
	{
		delete m_ServerList[i];
	}
	m_ServerList.clear();

	Unlock();
}

ServerInfo* CServerList::FindServer(int nServerId, int mod_id)
{
	Lock();

	for (size_t i=0; i < m_ServerList.size(); i++)
	{
		if (m_ServerList[i]->serverid == nServerId && m_ServerList[i]->mod_id == mod_id)
		{
			Unlock();

			return m_ServerList[i];
		}
	}

	Unlock();

	return NULL;
}

ServerInfo* CServerList::FindServer(PersistID& id)
{
	Lock();


	for (size_t i=0; i < m_ServerList.size(); i++)
	{
		if (m_ServerList[i]->conn == id)
		{
			Unlock();

			return m_ServerList[i];
		}
	}

	Unlock();

	return NULL;
}

ServerInfo* CServerList::AddServer(int nServerId, const char* szServerName,
								   const char* ip, PersistID id, int mod_id)
{
	ServerInfo* pServer = new ServerInfo;
	pServer->serverid = nServerId;
	pServer->ip = ip ? ip : "";
	pServer->conn = id;
	pServer->servername = szServerName ? szServerName : "";
	pServer->mod_id = mod_id;

	Lock();

	m_ServerList.push_back(pServer);

	Unlock();

	return pServer;
}

bool CServerList::RemoveServer(PersistID& id)
{
	Lock();

	for (size_t i=0; i < m_ServerList.size(); i++)
	{
		if (m_ServerList[i]->conn == id)
		{
			// 断开连接
			WShellAPI::Disconnect(m_ServerList[i]->conn);

			Log(LOG_NORMAL, "[ %s ] UnRegistered. sid:[%d], ip:[%s]", m_ServerList[i]->servername.c_str(),
				m_ServerList[i]->serverid, m_ServerList[i]->ip.c_str());

			delete m_ServerList[i];
			m_ServerList.erase(m_ServerList.begin() + i);

			break;
		}
	}

	Unlock();

	return true;
}

// 通过角色id和模块id查找处理器. 返回0 表示失败
UI32 CServerList::FindProcessor( UI32 RoleId, int mod )
{
	return _CONFIG->FindProcessor(RoleId, mod);
}

//查询各连接池数量,名称,ID
bool CServerList::QueryPoolList( int mod, int* szIdPool, int nMaxSize )
{
	return _CONFIG->GetIdPool( mod, szIdPool, nMaxSize );
}

bool CServerList::SendMessage(PersistID& id, CMsgTyped* pmsg)
{
	if (!pmsg)
	{
		return false;
	}

	ServerInfo* pServer = FindServer(id);

	if (!pServer)
	{
		return false;
	}

	return (WShellAPI::SendData((const PersistID &)pServer->conn,
		(char*)pmsg->GetData(), pmsg->GetLength()) == true);
}

bool CServerList::SendMessage(int serverid, int modid, CMsgTyped* pmsg)
{
    if (!pmsg)
	{
		return false;
	}

	ServerInfo* pServer = FindServer(serverid, modid);

	if (!pServer)
	{
		return false;
	}

	return (WShellAPI::SendData((const PersistID &)pServer->conn,
		(char*)pmsg->GetData(), pmsg->GetLength()) == true);
}

bool CServerList::Echo(PersistID& id)
{
	Lock();

	ServerInfo* pServer = FindServer(id);

	if (!pServer)
	{
		Unlock();

		return false;
	}

	::time(&pServer->echo);

	Unlock();

	return true;
}

void CServerList::OnHeartBeat(int nBeat)
{
	Lock();

	time_t now;
	::time(&now);

	for (size_t i=0; i < m_ServerList.size(); i++)
	{
		// 超过60秒未响应, 注销服务器
		if (now - m_ServerList[i]->echo > 60)
		{
			// 断开连接
			WShellAPI::Disconnect(m_ServerList[i]->conn);

			Log(LOG_NORMAL, "[ %s ] UnRegistered. sid:[%d], ip:[%s]", m_ServerList[i]->servername.c_str(),
				m_ServerList[i]->serverid, m_ServerList[i]->ip.c_str());

			delete m_ServerList[i];
			m_ServerList.erase(m_ServerList.begin() + i);

			break;
		}
	}

	Unlock();
}

void CServerList::ServerDisconnected(PersistID& connId)
{
	Lock();

	for (size_t i=0; i < m_ServerList.size(); i++)
	{
		if (m_ServerList[i]->conn == connId)
		{
			Log(LOG_ERROR, "[ %s ] disconnected. sid:[%d], ip:[%s]", m_ServerList[i]->servername.c_str(),
				m_ServerList[i]->serverid, m_ServerList[i]->ip.c_str());

			//m_ServerList[i]->conn.Invalid();

			break;
		}
	}

	Unlock();
}

void CServerList::RegisterCallBack(UI32 msgId, IGlobalMessageProc* handle)
{
	Lock();
	if (handle)
	{
		_GAME->RegisterIOMessageProc(false, msgId, handle);\
	}
	Unlock();
}

bool CServerList::AddProcessor(int id, const char* szProcessorName, size_t nThreadCount, const char* szADOConnStr)
{
	return _ASYNCPROC->AddProcessor(id, szProcessorName, nThreadCount, szADOConnStr);
}

CAsyncProcesser* CServerList::FindProcessor(int nProcessorID)
{
	return _ASYNCPROC->FindProcessor(nProcessorID);
}

CAsyncProcesser* CServerList::FindProcessor(const char* szProcessorName)
{
	return _ASYNCPROC->FindProcessor(szProcessorName);
}

bool CServerList::Call(int nProcessorId, LPASYNCPROCFUNC func, CMsgTyped* pParamList)
{
	return _ASYNCPROC->Call(nProcessorId, func, pParamList);
}

bool CServerList::Call(const char* szProcessorName, LPASYNCPROCFUNC func, CMsgTyped* pParamList)
{
	return _ASYNCPROC->Call(szProcessorName, func, pParamList);
}

bool CServerList::RemoveProcessor(int nProcessorID)
{
	return _ASYNCPROC->RemoveProcessor(nProcessorID);
}

bool CServerList::RemoveProcessor(const char* szProcessorName)
{
	return _ASYNCPROC->RemoveProcessor(szProcessorName);
}


