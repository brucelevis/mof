/********************************************************************
	created:	2010/11/15
	created:	15:11:2010   15:12
	filename: 	e:\SVN\服务器端\Source\ModLogin\async_func.cpp
	file path:	e:\SVN\服务器端\Source\ModLogin
	file base:	async_func
	file ext:	cpp
	author:

	purpose:
*********************************************************************/
#pragma  once
#include <algorithm>
#include <functional>
#include "async_func.h"
#include "Notify.h"
#include "GmTaskManager.h"
#include "BanRole.h"
#include "BanChat.h"

void AsyncProc_SaveNotify(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pmsg)
	{
		return;
	}

	if (!pConn)
	{
	    return;
	}

    int serverid = pmsg->IntVal();
    int notifyid = pmsg->IntVal();
    string content = pmsg->StringVal();
    string starttime = Time_tToString(pmsg->IntVal());
    string endtime = Time_tToString(pmsg->IntVal());
    int times = pmsg->IntVal();
    int interval = pmsg->IntVal();

    char sql[512];
    snprintf(sql, 512, "INSERT INTO tb_gm_notify(server_id, notify_id, content, start_time, end_time, total_times, send_interval)"
             "values (%d, %d, '%s', '%s', '%s', %d, %d);",
             serverid, notifyid, content.c_str(), starttime.c_str(), endtime.c_str(), times, interval);

    try
    {
        pConn->executeUpdate(sql);
    }
	catch(sql::SQLException &e)
	{
	    Log(LOG_ERROR, "save notify error");
	}
}

void AsyncProc_DeleteNotify(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pmsg)
	{
		return;
	}

	if (!pConn)
	{
	    return;
	}


    int serverid = pmsg->IntVal();
    int notifyid = pmsg->IntVal();

    char sql[512];
    snprintf(sql, 512, "DELETE FROM tb_gm_notify where server_id = %d and notify_id = %d", serverid, notifyid);

    try
    {
        pConn->executeUpdate(sql);
    }
	catch(sql::SQLException &e)
	{
	    Log(LOG_ERROR, "save notify error");
	}
}

void AsyncProc_SaveBanRole(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pmsg)
	{
		return;
	}

	if (!pConn)
	{
	    return;
	}

    int serverid = pmsg->IntVal();
    int roleid = pmsg->IntVal();
    string starttime = Time_tToString(pmsg->IntVal());
    string endtime = Time_tToString(pmsg->IntVal());

    char sqlstr[512];
    snprintf(sqlstr, 512, "INSERT INTO tb_gm_banrole(server_id, role_id, start_time, end_time) values (%d, %d, '%s','%s')",
             serverid, roleid, starttime.c_str(), endtime.c_str());

    try
    {
        pConn->executeUpdate(sqlstr);
    }
	catch(sql::SQLException &e)
	{
	    Log(LOG_ERROR, "save notify error");
	}
}

void AsyncProc_DeleteBanRole(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pmsg)
	{
		return;
	}

	if (!pConn)
	{
	    return;
	}

    int serverid = pmsg->IntVal();
    int roleid = pmsg->IntVal();

    char sql[512];
    snprintf(sql, 512, "DELETE FROM tb_gm_banrole where server_id = %d and role_id = %d", serverid, roleid);

    try
    {
        pConn->executeUpdate(sql);
    }
	catch(sql::SQLException &e)
	{
	    Log(LOG_ERROR, "save notify error");
	}
}

void AsyncProc_DeleteBanChat(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pmsg)
	{
		return;
	}

	if (!pConn)
	{
	    return;
	}

    int serverid = pmsg->IntVal();
    int roleid = pmsg->IntVal();

    char sql[512];
    snprintf(sql, 512, "DELETE FROM tb_gm_banchat where server_id = %d and role_id = %d", serverid, roleid);

    try
    {
        pConn->executeUpdate(sql);
    }
	catch(sql::SQLException &e)
	{
	    Log(LOG_ERROR, "save notify error");
	}
}

void AsyncProc_SaveBanChat(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pmsg)
	{
		return;
	}

	if (!pConn)
	{
	    return;
	}


    int serverid = pmsg->IntVal();
    int roleid = pmsg->IntVal();
    string starttime = Time_tToString(pmsg->IntVal());
    string endtime = Time_tToString(pmsg->IntVal());

    char sql[512];
    snprintf(sql, 512, "INSERT INTO tb_gm_banchat(server_id, role_id, start_time, end_time) values (%d, %d, '%s','%s')",
             serverid, roleid, starttime.c_str(), endtime.c_str());

    try
    {
        pConn->executeUpdate(sql);
    }
	catch(sql::SQLException &e)
	{
	    Log(LOG_ERROR, "save banchat error");
	}
}

void AsyncProp_LoadGmTasks(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pConn)
	{
	    return;
	}

    try
    {
        MysqlResult notifies;
        notifies.dataSet = pConn->executeQuery("SELECT * FROM tb_gm_notify");
        while (notifies.dataSet->next())
        {
            int serverid = notifies.dataSet->getInt("server_id");
            int notifyid = notifies.dataSet->getInt("notify_id");
            string content = notifies.dataSet->getString("content");
            string starttime = notifies.dataSet->getString("start_time");
            string endtime = notifies.dataSet->getString("end_time");
            int interval = notifies.dataSet->getInt("send_interval");
            int times = notifies.dataSet->getInt("total_times");


            Notify* notify = new Notify;
            notify->mServerId = serverid;
            notify->mNotifyId = notifyid;
            notify->mContent = content;
            notify->mStartTime = sscanftime(starttime.c_str());
            notify->mEndTime = sscanftime(endtime.c_str());;
            notify->mInterval = interval;
            notify->mTimes = times;
            g_GmTaskManager.addGmTask(notify);
        }

        MysqlResult banroles;
        banroles.dataSet = pConn->executeQuery("SELECT * FROM tb_gm_banrole");
        while (banroles.dataSet->next())
        {
            int serverid = banroles.dataSet->getInt("server_id");
            int roleid = banroles.dataSet->getInt("role_id");
            string starttime = banroles.dataSet->getString("start_time");
            string endtime = banroles.dataSet->getString("end_time");

            BanRole* banrole = new BanRole;
            banrole->mServerId = serverid;
            banrole->mRoleId = roleid;
            banrole->mStartTime = sscanftime(starttime.c_str());
            banrole->mEndTime = sscanftime(endtime.c_str());
            banrole->mState = 0;

            g_GmTaskManager.addGmTask(banrole);
        }

        MysqlResult banchats;
        banchats.dataSet = pConn->executeQuery("SELECT * FROM tb_gm_banchat");
        while (banchats.dataSet->next())
        {
            int serverid = banchats.dataSet->getInt("server_id");
            int roleid = banchats.dataSet->getInt("role_id");
            string starttime = banchats.dataSet->getString("start_time");
            string endtime = banchats.dataSet->getString("end_time");

            BanChat* banchat = new BanChat;
            banchat->mServerId = serverid;
            banchat->mRoleId = roleid;
            banchat->mStartTime = sscanftime(starttime.c_str());
            banchat->mEndTime = sscanftime(endtime.c_str());
            banchat->mState = 0;

            g_GmTaskManager.addGmTask(banchat);
        }

    }
	catch(sql::SQLException &e)
	{
	    Log(LOG_ERROR, "load gm task error");
	}

    g_GmTaskManager.loadGmTaskComplete();
}

void AsyncProc_HandleGmTask(CMsgTyped* pmsg, sql::Statement* pConn)
{
	if (!pConn)
	{
	    return;
	}

    char sqlstr[512];
    sprintf(sqlstr, "call Proc_GetGmTaskList()");

    try
    {
        MysqlResult result;
        result.dataSet = pConn->executeQuery(sqlstr);
        if (result.dataSet == NULL)
        {
            throw "execute sqlstr error";
        }

        while (result.dataSet->next())
        {
            int serverid = result.dataSet->getInt("_serverid");
            string cmdstr = result.dataSet->getString("_cmdstr");
            g_GmTaskManager.sendGmTaskCmd(serverid, cmdstr.c_str());
        }
        pConn->getMoreResults();

    }
	catch(sql::SQLException &e)
	{
        _Debug("error: %s when executing %s.\n", e.what(), sqlstr);
	}
	catch(...)
	{
        _Debug("unknown exception when executing %s.\n", sqlstr);
	}

}
