//
//  RankList.cpp
//  CenterServer
//
//  Created by Cheung Zirpon on 15-6-23.
//
//

#include "RankList.h"
#include "serverGroupMgr.h"
#include "serverList.h"
#include "configManager.h"

const char* getKeyByEnum(int type)
{
    switch (type) {
        case eSortBat:
            return "battle";
        case eSortConsume:
            return "consume";
        case eSortRecharge:
            return "recharge";
        case eSortPet:
            return "pet";
        default:
            break;
    }
    return NULL;
}

void CPaihangList::init(int serverid, SortType type, vector<CmpFun> funtor)
{
    mList     = set<PaihangInfo*, CSort>(CSort(funtor));
    mServerId = serverid;
    mSortType = type;
    mKey      = getKeyByEnum(type);
    
    loadDB();
    sort();

    CServerGroupMgr* groupMgr = getRankListGroupMgrByType(mSortType);
    if (groupMgr == NULL) {
        return;
    }
    CServerGroup* tmp = groupMgr->getServerGroupByServerId(mServerId);
    if (tmp == NULL) {
        //写日志
        return;
    }

    vector<PaihangInfo*> info;
    getList(info);
    tmp->freshRankList(info);

    RedisResult result(redisCmd("hget GameServer:%d:paihang:nextFreshTime", mServerId, mKey.c_str()));
    
    int nexttime = result.readInt();
    
    if (nexttime) {
        mNextFreshTime = nexttime;
    }
    else {
        setNextFreshTime();
    }
}


void CPaihangList::onRecvData(int total, vector<PaihangInfo*> &data)
{
    if (loadcheck()) {
        freshDB(total, data);
        
        if (mRecvDataCache.size() != total) {
            return;
        }
        else {
            mRecvDataCache.clear();
        }
        
    } else {
        loadDB();
    }
    
    sort();
    
    CServerGroupMgr* groupMgr = getRankListGroupMgrByType(mSortType);
    if (groupMgr == NULL) {
        return;
    }
    CServerGroup* tmp = groupMgr->getServerGroupByServerId(mServerId);
    if (tmp == NULL) {
        //写日志
        return;
    }
    
    vector<PaihangInfo*> info;
    getList(info);
    tmp->freshRankList(info);
}

bool
CPaihangList::loadcheck()
{
    RedisResult paihangDataCount(redisCmd("hlen GameServer:%d:paihang:%s", mServerId, mKey.c_str()));
    
    int count = paihangDataCount.readInt();
    
    if ( 0 < count) {
        time_t nowtime = time(NULL);
        RedisResult result(redisCmd("hget GameServer:%d:paihang:nextFreshTime %s", mServerId, mKey.c_str()));
        
        int nexttime = result.readInt();
        
        if (nexttime) {
            mNextFreshTime = nexttime;
            if (nowtime < mNextFreshTime) {
                return false;
            }
        }
    }
    
    return true;
}

void
CPaihangList::freshDB(int total, vector<PaihangInfo*> &data)
{
    if (0 == data.size()) {
        return;
    }
    for (int i = 0; i < data.size(); i++) {
        
        int objid = data[i]->getSortValueByType(eSortIndex);
        PaihangInfo* tmp = NULL;

        if (mSortType != eSortPet) {
            tmp = new SortInfo;
            tmp->load(data[i]->str());
        } else if (mSortType == eSortPet) {
            tmp = new SortPetInfo;
            tmp->load(data[i]->str());
        }
        mRecvDataCache.insert(make_pair(objid, tmp));
    }
    if (mRecvDataCache.size() != total) {
        return;
    }
    
    clear();

    for (Iterator itr = mRecvDataCache.begin(); itr != mRecvDataCache.end(); itr++) {
        
        int objid  = itr->second->getSortValueByType(eSortIndex);
        
        doRedisCmd("hset GameServer:%d:paihang:%s %d %s", mServerId, mKey.c_str(), objid, itr->second->str().c_str());
        
        mData.insert(make_pair(objid, itr->second));
    }
    
    
    setNextFreshTime();
}

void
CPaihangList::loadDB()
{
    clear();
    
    RedisResult reply (redisCmd("hgetall GameServer:%d:paihang:%s", mServerId, mKey.c_str()));

    for (int i = 0; i <reply.getHashElements(); i += 2) {
        int    objId    = reply.readHash(i, 0);
        string sInfo    = reply.readHash(i+1).c_str();
        
        if (objId <= 0 || sInfo.empty()) {
            continue;
        }
        
        PaihangInfo* tmp = NULL;
        
        if (mSortType != eSortPet) {
            tmp = new SortInfo;
        } else if (mSortType == eSortPet) {
            tmp = new SortPetInfo;
        }
        if (tmp->load(sInfo)) {
            mData.insert(make_pair(objId, tmp));
        }
    }
}

void CPaihangList::clear()
{
    CServerGroupMgr* groupMgr = getRankListGroupMgrByType(mSortType);
    if (groupMgr == NULL) {
//        printf("can't find ServerGroupMgr by Serverid: %d RankListType:%d\n", mServerId, mSortType);
        return;
    }
    CServerGroup* tmp = groupMgr->getServerGroupByServerId(mServerId);
    if (tmp == NULL) {
        //写日志
//        printf("can't find ServerGroup by Serverid: %d RankListType:%d\n", mServerId, mSortType);
        return;
    }
    
    tmp->onCallRemoveDataByServerid(mServerId);
    
    mList.clear();
    
    for (Iterator iter = mData.begin(); iter != mData.end(); iter++) {
        PaihangInfo* info = iter->second;
        delete info;
        info = NULL;
    }

    mData.clear();
}

void
CPaihangList::sort()
{
    if (0 == mData.size()) {
        return;
    }

//    log_info("roleinfo:");
    for (Iterator iter = mData.begin(); iter != mData.end(); iter++) {
        PaihangInfo* info = iter->second;
//        printf("%d roleid: %d, %s: %d, \n", mServerId, info->getSortValueByType(eSortIndex), mKey.c_str(), info->getSortValueByType(mSortType));
        mList.insert(info);
    }
    
//    log_info("sortinfo:");
//    set<PaihangInfo*, CSort>::iterator iter = mList.begin();
    
//    for (; iter != mList.end(); iter++) {
//        PaihangInfo* tmp = *iter;
//        printf("%d index: %d, %s: %d, \n", mServerId, tmp->getSortValueByType(eSortIndex), mKey.c_str(), tmp->getSortValueByType(mSortType));
//    }
}

void
CPaihangList::getList(vector<PaihangInfo*>& data)
{
    set<PaihangInfo*, CSort>::iterator iter = mList.begin();
    for (; iter != mList.end(); iter++) {
        data.push_back(*iter);
    }
}

PaihangInfo*
CPaihangList::getSortDataById(int objId)
{
    Iterator iter = mData.find(objId);
    
    if (iter != mData.end()) {
        return iter->second;
    }
    return NULL;
}

void
CPaihangList::setNextFreshTime()
{
    int nowtime = time(NULL);
    int freshPeriod;
    ServerGroupCfg* cfg = g_ConfigManager.getRankListGroupCfg(mSortType);
    if (cfg == NULL) {
        freshPeriod = 3600;
    } else {
        freshPeriod = cfg->getGroupFreshPeriod(mServerId);
    }
    
    nowtime = nowtime - nowtime % freshPeriod;
    
    mNextFreshTime = nowtime + freshPeriod;
    
    doRedisCmd("hset GameServer:%d:paihang:nextFreshTime %s %d", mServerId, mKey.c_str(), mNextFreshTime);
}

void
CPaihangList::onHeartBeat()
{
    time_t nowtime = time(NULL);
    
//    printf("serverid: %d, ranklisttype: %d, nextfreshtime: %d\n", mServerId, mSortType, mNextFreshTime);
    if (nowtime > mNextFreshTime) {
        ServerEntity* server = getServerList()->getServer(mServerId);
        if (server == NULL) {
            return;
        }
        if (server->getState() == ServerEntity::kDisconnect) {
            loadDB();
            sort();
            
            CServerGroupMgr* groupMgr = getRankListGroupMgrByType(mSortType);
            if (groupMgr == NULL) {
                return;
            }
            CServerGroup* tmp = groupMgr->getServerGroupByServerId(mServerId);
            if (tmp == NULL) {
                //写日志
                return;
            }
            
            vector<PaihangInfo*> info;
            getList(info);
            tmp->freshRankList(info);
            return;
        }
        else if (server->getState() == ServerEntity::kConnect) {
            
            int sessionid = server->getSession();
            req_cs_loadPaihangData req;
            req.type = mSortType;
            sendNetPacket(sessionid, &req);
        }
    }
}






