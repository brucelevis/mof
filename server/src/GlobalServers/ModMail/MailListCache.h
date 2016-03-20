
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

#include "async_func.h"
#include <set>
using namespace std;

class CPlayerMailList
{
public:
    CPlayerMailList(){ create_tm = time( NULL ); }
    ~CPlayerMailList()
    {
        for (size_t i=0; i < m_mailList.size(); i++){
            delete m_mailList[i];
        }
    }

    void AddItem(MailListItem& item, bool back = true);
    void DelItem(const char* guid);
    MailListItem* FindItem(const char* guid);

public:
    // 数据
    vector<MailListItem*> m_mailList;
    set<string> m_mailIds;

    // cache建立时间
    time_t create_tm;
};

class CMailCache : public CriticalObject
{
public:
    CMailCache();
    ~CMailCache();

public:
    void CreateCache(const char* playerName, vector<MailListItem*>& mails);
    void AddToCache(const char* playerName, MailListItem* mailItem);
    void SetReaded(const char* playerName, const char* mailGuid);
    void SetAttachNone(const char* playerName, const char* mailGuid);
    void RemoveFromCache(const char* playerName, const char* mailGuid);
    bool FindCache(const char* playerName, vector<MailListItem*>& mails);
    CPlayerMailList* FindCache(const char* playerName);

    // 重名
	void Rename(const char* playerName, const char* newName);

    // 处理过期缓存
    void ProcessOutDates();

private:
    std::map<string, CPlayerMailList* > m_MailCache;
};

extern CMailCache g_MailCache;



