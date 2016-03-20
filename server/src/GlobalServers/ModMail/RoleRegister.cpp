
#include "RoleRegister.h"

#include <mysql_connection.h>

using namespace sql;
using namespace sql::mysql;

RoleList g_RoleList;
int g_poolId = -1;

// 数据库函数
void AsyncProc_CreateRole(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pConn)
	{
		return;
	}

	string roleName = pmsg->StringVal(); //wstring roleNmae = pmsg->WideStrVal();

    MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
    string PlayerName = con->escapeString(roleName);
	try
	{
		char sql[1024];
		sprintf(sql, "insert into MailIndex values('%s')",PlayerName.c_str()); // WideStrAsUtf8(roleNmae).c_str()

		pConn->executeUpdate(sql);

		g_RoleList.AddRole(roleName.c_str());
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
    }
}

void AsyncProc_DeleteRole(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pConn)
	{
		return;
	}

	string roleName = pmsg->StringVal();

    MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
    string PlayerName = con->escapeString(roleName);
	try
	{
		char sql[1024];
		sprintf(sql, "delete from MailIndex where player = '%s'", PlayerName.c_str()); //WideStrAsUtf8

		pConn->executeUpdate(sql);
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
    }
}

void AsyncProc_DeleteRubish(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pConn)
	{
		return;
	}

	try
	{
		pConn->executeUpdate("delete from MailTable where DATEDIFF(now(), SendTime) > 30"); //天 15 30
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
    }

    // 删除系统邮件垃圾
    try
	{
		pConn->executeUpdate("delete from SysMailStat where DATEDIFF(now(), SendTime) > 30"); //天 15 30
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
    }

    try
	{
		pConn->executeUpdate("delete from SysMailTable where DATEDIFF(now(), SendTime) > 30");  //天 15 30
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
    }
}

void AsyncProc_LoadRoles(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pConn )
	{
		return;
	}

    MysqlResult res;

	try
	{
		res.dataSet = pConn->executeQuery("select * from MailIndex");

		if (!res.dataSet) return;

        while (res.dataSet->next())
        {
            string player = res.dataSet->getString("player").c_str(); //wstring Utf8AsWideStr

            g_RoleList.AddRole(player.c_str());
        }
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
    }
}

RoleList::RoleList()
{

}

RoleList::~RoleList()
{

}

void RoleList::CreateRole(const char* playerName)
{
	if (!playerName) return;

	Lock();

	std::map<string, int, MapStringComp>::iterator it = m_roleMap.find(playerName); //wstring //MapWStringComp
	if (it == m_roleMap.end())
	{
        // 创建这个location index
        CMsgTyped param;
        param.SetString(playerName); //SetWideStr
        g_pGame->Call(g_poolId, AsyncProc_CreateRole, &param);
	}

	Unlock();
}

bool RoleList::FindRole(const char* playerName)
{
	if (!playerName) return false;

	SMART_LOCK(this);

	std::map<string, int, MapStringComp>::iterator it = m_roleMap.find(playerName);
	return (bool)(it != m_roleMap.end());
}

int RoleList::DeleteRole(const char* playerName)
{
	if (!playerName) return 0;

	Lock();

	m_roleMap.erase( playerName );

	Unlock();

    // 删除数据库中记录
	CMsgTyped param;
    param.SetString(playerName); //SetWideStr
    g_pGame->Call(g_poolId, AsyncProc_DeleteRole, &param);

	return 0;
}

int RoleList::CheckDeleteRubish()
{
    // 投递请求
    CMsgTyped param;
    g_pGame->Call(g_poolId, AsyncProc_DeleteRubish, &param);

	return 0;
}

void RoleList::LoadRoleList(IGlobal* pGame)
{
	int poolCount[256];
	int count = 256;
	g_pGame->QueryPoolList(2, poolCount, count);

    // 得到数据库连接id
	g_poolId = poolCount[0];

    // 投递请求
    CMsgTyped param;
    g_pGame->Call(g_poolId, AsyncProc_LoadRoles, &param);
}

void RoleList::AddRole(const char* playerName)
{
	if (!playerName) return;

	Lock();
	m_roleMap[playerName] = 0;
	Unlock();
}

// 重更名index
void AsyncProc_RenameIndex(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pConn)
	{
		return;
	}


    MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
    string oname = con->escapeString(pmsg->StringVal());
    string nname = con->escapeString(pmsg->StringVal());
	//const char* oname = pmsg->StringVal();
	//const char* nname = pmsg->StringVal();

    // 更新index
	try
	{
	    char sqlbuf[1024];
	    sprintf(sqlbuf, "update MailIndex set player = '%s' where player = '%s'", nname.c_str(), oname.c_str());
		pConn->executeUpdate(sqlbuf); //天 15 30
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
    }

    // 更新MailTable
	try
	{
	    char sqlbuf[1024];
	    sprintf(sqlbuf, "update MailTable set Owner = '%s', Receiver = '%s' where Owner = '%s' and Receiver = '%s'",
            nname.c_str(), nname.c_str(), oname.c_str(), oname.c_str());
		pConn->executeUpdate(sqlbuf); //天 15 30
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
    }

    // 更新SYS Mail
	try
	{
	    char sqlbuf[1024];
	    sprintf(sqlbuf, "update SysMailStat set Owner = '%s' where Owner = '%s'", nname.c_str(), oname.c_str());
		pConn->executeUpdate(sqlbuf); //天 15 30
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
    }
}

bool RoleList::Rename(const char* playerName, const char* newName)
{
    if (!playerName ||!newName) return false;

    bool bret = false;

    std::map<string, int, MapStringComp>::iterator it,it2;

    Lock();
    it = m_roleMap.find(playerName);
    it2 = m_roleMap.find(newName);

    // 此名字已经存在，新名字未被使用，则替换
    if (it != m_roleMap.end() && it2 == m_roleMap.end())
    {
        // 进行更名
        m_roleMap[newName] = 1;
        m_roleMap.erase(it);

        bret = true;

        // 发起异步调用
        // 投递请求
        CMsgTyped param;
        param.SetString(playerName);
        param.SetString(newName);
        g_pGame->Call(g_poolId, AsyncProc_RenameIndex, &param);
    }
    Unlock();

    return bret;
}


