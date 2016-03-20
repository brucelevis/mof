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

#include "MailListCache.h"

CMailCache g_MailCache;

// ----------------------------------------------
void CPlayerMailList::AddItem(MailListItem& item, bool back)
{
    if (m_mailIds.find(item.mailid) != m_mailIds.end()) {
        return;
    }

    m_mailIds.insert(item.mailid);
    MailListItem* newItem = new MailListItem;
    *newItem = item;
    if (back)
    {
        m_mailList.push_back( newItem );
    }
    else
    {
        m_mailList.insert(m_mailList.begin(), newItem);
    }
}

void CPlayerMailList::DelItem(const char* guid)
{
    if (!guid) return;

    m_mailIds.erase(guid);
    for (size_t i=0; i < m_mailList.size(); i++)
    {
        if (strcmp(m_mailList[i]->mailid.c_str(), guid) == 0)
        {
            delete m_mailList[i];
            m_mailList.erase( m_mailList.begin() + i);

            return;
        }
    }
}

MailListItem* CPlayerMailList::FindItem(const char* guid)
{
    if (!guid) return NULL;

    for (size_t i=0; i < m_mailList.size(); i++)
    {
        if (strcmp(m_mailList[i]->mailid.c_str(), guid) == 0)
        {
            return m_mailList[i];
        }
    }

    return NULL;
}

// ----------------------------------------------
CMailCache::CMailCache()
{

}

CMailCache::~CMailCache()
{
    SMART_LOCK(this);

    std::map<string, CPlayerMailList* >::iterator it;

    for (it = m_MailCache.begin(); it != m_MailCache.end(); it++)
    {
        delete it->second;
    }
    m_MailCache.clear();
}

void CMailCache::CreateCache(const char* playerName, vector<MailListItem*>& mails)
{
    if (!playerName) return;

    SMART_LOCK(this);

    // 查找是否已经存在缓存
    std::map<string, CPlayerMailList* >::iterator it = m_MailCache.find( playerName );
    if (it != m_MailCache.end() )
    {
        SAFE_DELETE( it->second );
    }

    // 创建新的
    CPlayerMailList* newCache = new CPlayerMailList;

    // 添加元素
    for (size_t i=0; i < mails.size(); i++)
    {
        newCache->AddItem(*mails[i]);
    }

    //添加到容器
    m_MailCache[playerName] = newCache;
}

void CMailCache::AddToCache(const char* playerName, MailListItem* mailItem)
{
    if (!playerName || !mailItem) return;

    SMART_LOCK(this);

    CPlayerMailList* pCache = FindCache( playerName );

    if (pCache)
    {
 	// 将新邮件插入到列表头部，
        pCache->AddItem(*mailItem, false);
    }
}

void CMailCache::RemoveFromCache(const char* playerName, const char* mailGuid)
{
    if (!playerName || !mailGuid) return;

    SMART_LOCK(this);

    CPlayerMailList* pCache = FindCache( playerName );
    if (pCache)
    {
        pCache->DelItem( mailGuid );
    }
}

void CMailCache::SetReaded(const char* playerName, const char* mailGuid)
{
    if (!playerName || !mailGuid) return;

    SMART_LOCK(this);

    CPlayerMailList* pCache = FindCache( playerName );
    if (pCache)
    {
        MailListItem* item = pCache->FindItem( mailGuid );
        if (item)
        {
            item->isReaded = 1;
        }
    }
}

void CMailCache::SetAttachNone(const char* playerName, const char* mailGuid)
{
    if (!playerName || !mailGuid) return;

    SMART_LOCK(this);

    CPlayerMailList* pCache = FindCache( playerName );
    if (pCache)
    {
        MailListItem* item = pCache->FindItem( mailGuid );
        if (item)
        {
            item->hasAttach = 0;
        }
    }
}

bool CMailCache::FindCache(const char* playerName, vector<MailListItem*>& mails)
{
    if (!playerName) return false;

    SMART_LOCK(this);

    std::map<string, CPlayerMailList* >::iterator it = m_MailCache.find( playerName );
    if (it == m_MailCache.end() || !it->second) return false;

    // 填充容器
    mails.clear();

    CPlayerMailList& plist = *it->second;

    for( size_t i=0; i < plist.m_mailList.size(); i++)
    {
        MailListItem* pnewi = new MailListItem;
        *pnewi = *plist.m_mailList[i];
        mails.push_back( pnewi );
    }

    return true;
}

CPlayerMailList* CMailCache::FindCache(const char* playerName)
{
    if (!playerName) return NULL;

    SMART_LOCK(this);

    std::map<string, CPlayerMailList* >::iterator it = m_MailCache.find( playerName );
    if (it == m_MailCache.end() ) return NULL;

    return it->second;
}

void CMailCache::ProcessOutDates()
{
    SMART_LOCK(this);

    time_t now = time( NULL );

    std::map<string, CPlayerMailList* >::iterator it;

    for (it = m_MailCache.begin(); it != m_MailCache.end(); it++)
    {
        // 只能缓存10分钟
        if (it->second && now - it->second->create_tm > 600)
        {
            //缓存过期
            delete it->second;
            it->second = NULL;
        }
    }
}

void CMailCache::Rename(const char* playerName, const char* newName)
{
    if (!playerName ||!newName) return;

    std::map<string, CPlayerMailList* >::iterator it, it2;

    Lock();
    it = m_MailCache.find(playerName);
    it2 = m_MailCache.find(newName);

    // 此名字已经存在，新名字未被使用，则替换
    if (it != m_MailCache.end() && it2 == m_MailCache.end())
    {
        // 进行更名
        m_MailCache[newName] = it->second;
        m_MailCache.erase(it);
    }
    Unlock();
}

