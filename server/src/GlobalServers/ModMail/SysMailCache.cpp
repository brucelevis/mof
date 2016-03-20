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

#include "SysMailCache.h"
#include "RoleRegister.h"
#include <mysql_connection.h>

using namespace sql;
using namespace sql::mysql;

SysMailMgr g_sysMails;

// 清除缓存
void ClearCache(std::map<string, SysMail*>& cache)
{
    std::map<string, SysMail*>::iterator it;
    for (it = cache.begin(); it != cache.end(); it++)
    {
        if (it->second)
        {
            delete it->second;
        }
    }
    cache.clear();
}

SysMailMgr::SysMailMgr()
{
    m_bLoadOk = false;
}

SysMailMgr::~SysMailMgr()
{
    ClearAll();
}

void SysMailMgr::AsyncProcLoadSysMail(CMsgTyped* pmsg, sql::Statement* pConn)
{
    SMART_LOCK(&g_sysMails);

    // 加载邮件列表
    MysqlResult res;
    try
    {
        res.dataSet = pConn->executeQuery("select * from SysMailTable");
        if (!res.dataSet) return;

        while (res.dataSet->next())
        {
            /*`MailID` CHAR(40) NOT NULL DEFAULT '' COLLATE 'utf8_general_ci',
            `Title` VARCHAR(256) NULL DEFAULT NULL COLLATE 'utf8_general_ci',
            `SendTime` DATETIME NULL DEFAULT NULL,
            `Attachment` VARCHAR(512) NOT NULL COLLATE 'utf8_general_ci',
            `MailContent` VARCHAR(4000) NOT NULL COLLATE 'utf8_general_ci',
            */

            SysMail* pcache = new SysMail;
            pcache->mailId = res.dataSet->getString("MailID").c_str();
            pcache->sender = res.dataSet->getString("Sender").c_str();
            pcache->mailId = "sys_" + pcache->mailId;
            pcache->title = res.dataSet->getString("Title").c_str();
            pcache->ssendTime = res.dataSet->getString("SendTime");
            pcache->sendTime = sscanftime(pcache->ssendTime.c_str());
            pcache->attach = res.dataSet->getString("Attachment");
            pcache->content = res.dataSet->getString("MailContent");

            g_sysMails.m_mail_list[pcache->mailId] = pcache;
        }
    }
 	catch(sql::SQLException & e)
	{
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";

        g_sysMails.ClearAll();

        return;
	}

	// 加载状态列表
    MysqlResult res2;
    try
    {
        res2.dataSet = pConn->executeQuery("select MailID,Owner,HasAttachment,IsDelete from SysMailStat");
        if (!res2.dataSet) return;

        while (res2.dataSet->next())
        {
            /*
            `MailID` CHAR(40) NULL DEFAULT NULL COLLATE 'utf8_general_ci',
            `Owner` VARCHAR(50) NULL DEFAULT NULL COLLATE 'utf8_general_ci',
            `HasAttachment` INT(10) NULL DEFAULT NULL,
            `IsDelete` INT(10) NULL DEFAULT NULL,
            `IsRead` INT(10) NULL DEFAULT NULL,*/

            string MailID = res2.dataSet->getString("MailID").c_str();
            MailID = "sys_" + MailID;
            string Owner = res2.dataSet->getString("Owner").c_str();

            // 查找
            std::map<string, SysMail*>::iterator it = g_sysMails.m_mail_list.find(MailID);
            if (it == g_sysMails.m_mail_list.end())
            {
                continue;
            }

            SysMail* pcache = it->second;
            if (!pcache)
            {
                continue;
            }

            // 创建状态
            SysMailStat stat;
            stat.hasAttach = res2.dataSet->getInt("HasAttachment");
            stat.isdelete = res2.dataSet->getInt("IsDelete");

            // 记录状态
            pcache->stat_map[Owner] = stat;
        }
    }
    catch(sql::SQLException & e)
	{
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";

        g_sysMails.ClearAll();

        return;
	}

	g_sysMails.m_bLoadOk = true;
}

void SysMailMgr::Load()
{
    // 从数据库中加载
    CMsgTyped param;
    g_pGame->Call(g_poolId , AsyncProcLoadSysMail, &param);
}

void SysMailMgr::AsyncProcReloadSysMail(CMsgTyped* pmsg, sql::Statement* pConn)
{
    // 重新载入系统邮件列表， 与现有列表做比较， 删除已经过期的， 并添加新邮件
    std::map<string, SysMail*> new_mails;

    // 加载邮件列表
    MysqlResult res;
    try
    {
        res.dataSet = pConn->executeQuery("select * from SysMailTable");
        if (!res.dataSet) return;

        while (res.dataSet->next())
        {
            SysMail* pcache = new SysMail;
            pcache->mailId = res.dataSet->getString("MailID").c_str();
            pcache->sender = res.dataSet->getString("Sender").c_str();
            pcache->mailId = "sys_" + pcache->mailId;
            pcache->title = res.dataSet->getString("Title").c_str();
            pcache->ssendTime = res.dataSet->getString("SendTime");
            pcache->sendTime = sscanftime(pcache->ssendTime.c_str());
            pcache->attach = res.dataSet->getString("Attachment");
            pcache->content = res.dataSet->getString("MailContent");

            new_mails[pcache->mailId] = pcache;
        }
    }
 	catch(sql::SQLException & e)
	{
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";

        ClearCache(new_mails);

        return;
	}

	// 开始比较新旧邮件
	SMART_LOCK(&g_sysMails);

	// 开始比较
	// 新旧交换
	g_sysMails.m_mail_list.swap( new_mails );

	// 遍历map, 如果在旧map存在同样的记录， 拷贝其缓存
    std::map<string, SysMail*>::iterator it, ittmp;
    for (it = g_sysMails.m_mail_list.begin(); it != g_sysMails.m_mail_list.end(); it++)
    {
        if (it->second)
        {
            // 旧集合中存在相同记录, 拷贝其
            ittmp = new_mails.find(it->first);
            if (ittmp != new_mails.end())
            {
                // 交换
                SysMail* now = it->second;
                SysMail* old = ittmp->second;

                now->stat_map.swap(old->stat_map);

                //it->second = old;
                //ittmp->second = now;
            }
        }
    }

    // 删除垃圾缓存
    ClearCache(new_mails);
}

void SysMailMgr::ReLoad()
{
    // 从数据库中加载
    CMsgTyped param;
    g_pGame->Call(g_poolId , AsyncProcReloadSysMail, &param);
}

void SysMailMgr::ClearAll()
{
    SMART_LOCK(this);

    ClearCache(m_mail_list);
}

bool CreateStat(sql::Statement* pConn, const char* sysmailId, const char* owner,
                const char* sendTime, SysMailStat& state)
{
    const char* realMailId = sysmailId + 4;

    /*CREATE TABLE `SysMailStat` (
	`MailID` CHAR(40) NULL DEFAULT NULL COLLATE 'utf8_general_ci',
	`Owner` VARCHAR(50) NULL DEFAULT NULL COLLATE 'utf8_general_ci',
	`SendTime` DATETIME NULL DEFAULT NULL,
	`HasAttachment` INT(10) NULL DEFAULT NULL,
	`IsDelete` INT(10) NULL DEFAULT NULL,
	`IsRead` INT(10) NULL DEFAULT NULL,
	INDEX `MailID` (`MailID`),
	INDEX `Owner` (`Owner`)*/

    try
	{

        MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
        string sOwner = con->escapeString(owner);

		char sql[1024];
		sprintf(sql, "insert into SysMailStat values('%s','%s','%s','%d','%d')",
          realMailId, sOwner.c_str(), sendTime, state.hasAttach, state.isdelete);

		pConn->executeUpdate(sql);

		return true;
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";

        return false;
    }
}

bool UpdateStat(sql::Statement* pConn, const char* sysmailId, const char* owner, SysMailStat& state)
{
    const char* realMailId = sysmailId + 4;

    try
	{

        MySQL_Connection* con = (MySQL_Connection*)pConn->getConnection();
        string sOwner = con->escapeString(owner);

		char sql[1024];
		sprintf(sql, "update SysMailStat set HasAttachment=%d, IsDelete=%d where MailID='%s' AND Owner = '%s'",
          state.hasAttach, state.isdelete, realMailId, sOwner.c_str());

		pConn->executeUpdate(sql);
		return true;
	}
	catch(sql::SQLException & e)
    {
        cout << __LINE__ << "\n";
        cout << "# ERR: " << e.what() << "\n";
        return false;
    }
}

void SysMailMgr::SysDeleteMail(sql::Statement* pConn, const char* player, const char* mail_id)
{
    Lock();

    // 检测邮件是否存在
    SysMail* cache = FindCache(mail_id);
    if (!cache)
    {
        Unlock();

        return;
    }

    std::map<string, SysMailStat>::iterator it = cache->stat_map.find(player);
    if (it == cache->stat_map.end())
    {
        // 缓存不存在, 建立缓存
        SysMailStat stat;
        stat.isdelete = 1; // 状态为已经删除
        cache->stat_map[player] = stat;

        Unlock();

        CreateStat(pConn, mail_id, player, cache->ssendTime.c_str(), stat);
    }
    else
    {
        // 更新状态为已经删除
        if (it->second.isdelete == 1)
        {
            Unlock();
            return;
        }

        // 内存修改
        it->second.isdelete = 1;

        Unlock();

        // 修改状态
        UpdateStat(pConn, mail_id, player, it->second);
    }
}

bool SysMailMgr::SysMailList(const char* PlayerName, vector<MailListItem*>& maillist)
{
    // 记录之前的记录数量
    size_t oldsz = maillist.size();

    SMART_LOCK(this);

    // 遍历邮件缓存
    std::map<string, SysMail*>::iterator it;
    std::map<string, SysMailStat>::iterator statit;
    for (it = m_mail_list.begin(); it != m_mail_list.end(); it++)
    {
        if (!it->second) continue;

        // 查找是否存在玩家的缓存
        statit = it->second->stat_map.find(PlayerName);
        if (statit == it->second->stat_map.end())
        {
            // 没有状态， 可以使用
            MailListItem &mi = *(new MailListItem);

            mi.mailid = it->second->mailId;
            mi.title = it->second->title;
            mi.sender = it->second->sender;
            mi.receiver = PlayerName;
            mi.isReaded = 0;
            mi.send_time = it->second->ssendTime;
            mi.hasAttach = it->second->attach.size() > 0;

            mi.tm_send_time = it->second->sendTime;

            // 加入列表
            maillist.push_back(&mi);
        }
        else
        {
            // 查看邮件是否已经被删除
            SysMailStat & st = statit->second;
            if (st.isdelete == 1)
            {
                continue;
            }

            // 创建邮件项
            MailListItem &mi = *(new MailListItem);

            mi.mailid = it->second->mailId;
            mi.title = it->second->title;
            mi.sender = it->second->sender;
            mi.receiver = PlayerName;
            mi.isReaded = 1;
            mi.send_time = it->second->ssendTime;
            mi.hasAttach = ( st.hasAttach == 1 && it->second->attach.size() > 0 );

            mi.tm_send_time = it->second->sendTime;

            // 加入列表
            maillist.push_back(&mi);
        }
    }

    // 记录数不同则返回true
    return oldsz != maillist.size();
}

bool SysMailMgr::SysReadMail(sql::Statement* pConn, const char* PlayerName,
                             const char* mail_guid, string& content, string& attach)
{
    Lock();

    // 检测邮件是否存在
    SysMail* cache = FindCache(mail_guid);
    if (!cache)
    {
        Unlock();

        return false;
    }

    std::map<string, SysMailStat>::iterator it = cache->stat_map.find(PlayerName);
    if (it == cache->stat_map.end())
    {
        // 缓存不存在, 建立缓存
        SysMailStat stat;
        cache->stat_map[PlayerName] = stat;

        content = cache->content;
        attach = stat.hasAttach == 1 ? cache->attach : "";

        Unlock();

        CreateStat(pConn, mail_guid, PlayerName, cache->ssendTime.c_str(), stat);
    }
    else
    {
        // 状态为已经删除
        if (it->second.isdelete == 1)
        {
            Unlock();
            return false;
        }

        // 返回值
        content = cache->content;
        attach = it->second.hasAttach == 1 ? cache->attach : "";

        Unlock();

        // 修改状态
        UpdateStat(pConn, mail_guid, PlayerName, it->second);
    }

    return true;
}

bool SysMailMgr::SysGetAttach(sql::Statement* pConn, const char* owner, const char* mail_guid, string& attach)
{
    Lock();

    // 检测邮件是否存在
    SysMail* cache = FindCache(mail_guid);
    if (!cache)
    {
        Unlock();

        return false;
    }

    // 没有附件
    if (cache->attach.size() == 0)
    {
        Unlock();

        return false;
    }

    std::map<string, SysMailStat>::iterator it = cache->stat_map.find(owner);
    if (it == cache->stat_map.end())
    {
        // 缓存不存在, 建立缓存
        SysMailStat stat;
        stat.hasAttach = 0; // 标记为附件已经领取
        cache->stat_map[owner] = stat;

        attach = cache->attach;

        Unlock();

        if (!CreateStat(pConn, mail_guid, owner, cache->ssendTime.c_str(), stat))
        {
            attach = "";
            return false;
        }
    }
    else
    {
        // 状态为已经删除
        if (it->second.isdelete == 1)
        {
            Unlock();
            return false;
        }

        // 状态为附件已经被领取
        if (it->second.hasAttach == 0)
        {
            Unlock();
            return false;
        }

        // 内存修改
        it->second.hasAttach = 0;
        attach = cache->attach;

        Unlock();

        // 修改状态
        if (!UpdateStat(pConn, mail_guid, owner, it->second))
        {
            attach = "";
            return false;
        }
    }

    return true;
}

SysMail* SysMailMgr::FindCache(const char* mail_id)
{
    if (!mail_id) return NULL;

    std::map<string, SysMail*>::iterator it = m_mail_list.find(mail_id);
    if (it != m_mail_list.end())
    {
        return it->second;
    }

    return NULL;
}

SysMailStat* SysMailMgr::FindStat(const char* mail_id, const char* owner)
{
    if (!owner) return NULL;

    SysMail* cache = FindCache(mail_id);
    if (!cache) return NULL;

    std::map<string, SysMailStat>::iterator it = cache->stat_map.find(owner);
    if (it != cache->stat_map.end())
    {
        return &it->second;
    }

    return NULL;
}

void SysMailMgr::Rename(const char* playerName, const char* newName)
{
    if (!playerName ||!newName) return;

    std::map<string, SysMail*>::iterator it;
    std::map<string, SysMailStat>::iterator statit,statit2;

    Lock();

    for (it = m_mail_list.begin(); it != m_mail_list.end(); it++)
    {
        // 遍历每一个邮件
        if (!it->second) continue;

        SysMail& stat = *it->second;

        statit = stat.stat_map.find(playerName);
        statit2 = stat.stat_map.find(newName);

        // 查看能否替换
        if (statit != stat.stat_map.end()&& statit2 == stat.stat_map.end())
        {
            stat.stat_map[newName] = statit->second;
            stat.stat_map.erase(statit);
        }
    }

    Unlock();
}

