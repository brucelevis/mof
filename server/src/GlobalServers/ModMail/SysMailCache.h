
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
#pragma once

#include "MailListCache.h"

// 系统邮件状态
struct SysMailStat
{
public:
    SysMailStat()
    {
        hasAttach = 1;
        isdelete = 0;
    }
    char hasAttach;
    char isdelete;
    char rosolv;
};

// 系统邮件
struct SysMail
{
    string mailId;
    string sender;
    string title;
    time_t sendTime;
    string ssendTime;
    string attach;
    string content;
    std::map<string, SysMailStat> stat_map;
};

// 系统邮件管理器
class SysMailMgr : CriticalObject
{
public:
    SysMailMgr();
    ~SysMailMgr();

    void Load();
    void ReLoad();// 重新加载邮件列表， 可以载入新邮件和删除从外部删除的邮件
    void ClearAll();
    void SysDeleteMail(sql::Statement* pConn, const char* player, const char* mail_id);
    bool SysMailList(const char* PlayerName, vector<MailListItem*>& maillist);

    bool SysReadMail(sql::Statement* pConn, const char* PlayerName,
                 const char* mail_guid, string& content, string& attach);

    bool SysGetAttach(sql::Statement* pConn, const char* owner, const char* mail_guid, string& attach);

    // 检测是否系统邮件
    static bool IsSysMail(const char* mail_guid)
        { return (strstr(mail_guid, "sys_") == mail_guid); }

    //重名
    void Rename(const char* playerName, const char* newName);

private:
    SysMail* FindCache(const char* mail_id);
    SysMailStat* FindStat(const char* mail_id, const char* owner);

protected:
    static void AsyncProcLoadSysMail(CMsgTyped* pmsg, sql::Statement* pConn);
    static void AsyncProcReloadSysMail(CMsgTyped* pmsg, sql::Statement* pConn);

private:
    bool m_bLoadOk;
    std::map<string, SysMail*> m_mail_list;
};

extern SysMailMgr g_sysMails;
