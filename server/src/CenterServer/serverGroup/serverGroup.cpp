//
//  serverGroup.cpp
//  CenterServer
//
//  Created by Cheung Zirpon on 15-6-24.
//
//

#include "serverGroup.h"
#include "configManager.h"
#include "paihangSortInfo.h"
#include "serverList.h"
#include "centerLog.h"

extern CenterLog createCrossServiceBattleRankListLog();
extern CenterLog createCrossServiceRechargeRankListLog();
extern CenterLog createCrossServiceConsumeRankListLog();
extern CenterLog createCrossServicePetRankListLog();

extern CenterLog createCrossServiceBattleRankListAwardHistoryLog();
extern CenterLog createCrossServiceRechargeRankListAwardHistoryLog();
extern CenterLog createCrossServiceConsumeRankListAwardHistoryLog();
extern CenterLog createCrossServicePetRankListAwardHistoryLog();


CServerGroup*
CServerGroup::createGroup(ServerGroupCfgDef* groupcfg)
{
    if (groupcfg == NULL) {
        return NULL;
    }

    CServerGroup* tmp = new CServerGroup;
    tmp->mGroupId       = groupcfg->mGroupId;
    tmp->mSortType      = groupcfg->mRankListType;
    tmp->mAwardSendTime = groupcfg->mSendAwardTime;

    tmp->init();
    return tmp;
}

bool
CServerGroup::init() {
    return true;
}

void
CServerGroup::load() {
}

bool
CServerGroup::addServerEntity(ServerEntity* server)
{
    if (server == NULL) {
        return false;
    }
    if (!checkServerInclude(server->mServerId)) {
        return false;
    }
    
    mServerMap.insert(make_pair(server->mServerId, server));
    
    return true;
}

bool
CServerGroup::checkServerInclude(int serverid)
{
    ServerGroupCfg* cfg = g_ConfigManager.getRankListGroupCfg(mSortType);
    if (cfg == NULL) {
        return false;
    }
    int groupid = cfg->getServerGroupId(serverid);
    
    if (mGroupId == groupid) {
        return true;
    }
    return false;
}

ServerEntity*
CServerGroup::getServerEntityById(int serverid)
{
    map<int, ServerEntity*>::iterator iter =mServerMap.find(serverid);
    if (iter != mServerMap.end()) {
        return iter->second;
    }
    return NULL;
}



void
CServerGroup::freshRankList(vector<PaihangInfo*> &data)
{
    if (data.size() == 0) {
        return;
    }
    string key = getKeyByEnum(mSortType);

    vector<PaihangInfo*> tmpBatList;
    int k = 0;
    
    vector<PaihangInfo*> rankList = mRankList;
//    log_info("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
//    for (int i = 0; i < rankList.size(); i++) {
//        printf("%d index: %d, %s: %d, rank: %d\n",  rankList[i]->getSortValueByType(eSortServerid),
//                                                    rankList[i]->getSortValueByType(eSortIndex),
//                                                    key.c_str(),
//                                                    rankList[i]->getSortValueByType(mSortType),
//                                                    rankList[i]->getSortRankByType(mSortType));
//    }
//    log_info("------------------------------------------------------");

    if (rankList.size() == 0) {
        for (int i = 0; i < data.size(); i++) {
            tmpBatList.push_back(data[i]);
        }
    } else {
        vector<PaihangInfo*>::iterator listIter  = rankList.begin();
        vector<PaihangInfo*>::iterator dataIter  = data.begin();
        while (listIter != rankList.end() && dataIter != data.end())
        {
            int first  = (*listIter)->getSortValueByType(mSortType);
            int second = (*dataIter)->getSortValueByType(mSortType);
            
            if (first < second)
            {
                tmpBatList.push_back(*dataIter);
                dataIter++;
                k++;
            }
            else {
                tmpBatList.push_back(*listIter);
                listIter++;
                k++;
            }
        }
        
        while (listIter != rankList.end()) {
            if (k >= CROSSSERVICE_PAIHANG_LOAD_COUNT) {
                break;
            }
            
            tmpBatList.push_back(*listIter);
            listIter++;
            k++;
        }
        
        while (dataIter != data.end()) {
            if (k >= CROSSSERVICE_PAIHANG_LOAD_COUNT) {
                break;
            }
            
            tmpBatList.push_back(*dataIter);
            dataIter++;
            k++;
        }
    }
    if (tmpBatList.size() == 0) {
        return;
    }
    mRankList.clear();
    mRankList = tmpBatList;
    
    rankListSort();
    
    addRankListLog();
    
    noticeServerEntityOnRefresh();
}


void
CServerGroup::rankListSort()
{
    string key = getKeyByEnum(mSortType);

    for (int i = 0; i < mRankList.size(); i++) {
        if (i == 0) {
            mRankList[i]->setSortRankByType(mSortType, i + 1);
        } else {
            
            int first  = mRankList[i]->getSortValueByType(mSortType);
            int second = mRankList[i-1]->getSortValueByType(mSortType);
            
            int firstRank = mRankList[i-1]->getSortRankByType(mSortType);
            if (first == second) {
                mRankList[i]->setSortRankByType(mSortType, firstRank);
            } else {
                mRankList[i]->setSortRankByType(mSortType, firstRank + 1);
            }
            
        }

    }
}

void
CServerGroup::rankListShotCutOnDB()
{
    string key = getKeyByEnum(mSortType);

    doRedisCmd("zremrangebyrank ServerGroup:%d:ranklist:%s 0 -1", mGroupId, key.c_str());

    for (int i = 0; i < mRankList.size(); i++) {
        int value = mRankList[i]->getSortValueByType(mSortType);
        
        doRedisCmd("zadd ServerGroup:%d:ranklist:%s %d %d", mGroupId, key.c_str(), value, mRankList[i]->getSortValueByType(eSortIndex));
    }
}


void
CServerGroup::onCallRemoveDataByServerid(int serverid)
{
    vector<PaihangInfo*> tmp;
    for (int i = 0; i < mRankList.size(); i++) {
        if (serverid != mRankList[i]->getSortValueByType(eSortServerid)) {
            tmp.push_back(mRankList[i]);
        }
    }
    
    mRankList.clear();
    mRankList = tmp;
    
    rankListSort();
}

void
CServerGroup::getRankList(vector<PaihangInfo*> &ret)
{
    if (mRankList.size()) {
        ret = mRankList;
    }
}

int
CServerGroup::getSortRankByObjId(int objId)
{
    for (int i = 0; i < mRankList.size(); i++) {
        PaihangInfo* tmp = mRankList[i];
        if (objId == tmp->getSortValueByType(eSortIndex)) {
            return tmp->getSortRankByType(mSortType);
        }
    }
    
    return -1;
}

void
CServerGroup::onHeartBeat()
{
    time_t nowtime = time(NULL);

    sendAwards(nowtime);
}


//以后优化
void
CServerGroup::sendAwards(int tick)
{
    if (tick < mAwardSendTime) {
        return;
    }
    
    ServerGroupCfg* cfg = g_ConfigManager.getRankListGroupCfg(mSortType);
    if (cfg == NULL) {
        return;
    }
    ServerGroupCfgDef* def = cfg->getServerGroupCfgDefByGroupid(mGroupId);
    if (def == NULL) {
        return;
    }
    
    const char* key = getKeyByEnum(mSortType);
    if (key == NULL) {
        return;
    }
    
    time_t awardSendTime = mAwardSendTime;
    
    tm tmAwardtime;
    localtime_r(&awardSendTime, &tmAwardtime);
    char awardSendTimeBuf[128];
    strftime ( awardSendTimeBuf,80,"%Y-%m-%d Time: %H:%M:%S\n", &tmAwardtime);
    
//    printf("groupid: %d, sorttype: %d, sendawardtime: %s\n", mGroupId, mSortType, awardSendTimeBuf);
    
	RedisResult result(redisCmd("exists %s:ServerGroup:%d:%d", key, mGroupId, mAwardSendTime));

    map<int, string> sendAwardsHistory;
    if (result.readInt() == 1) {
        RedisResult sendtime(redisCmd("hgetall %s:ServerGroup:%d:%d", key, mGroupId, mAwardSendTime));
        for (int i = 0; i < sendtime.getHashElements(); i += 2) {
            int    rank      = sendtime.readHash(i, 0);
            string awardInfo = sendtime.readHash(i+1, "");
            
            sendAwardsHistory.insert(make_pair(rank, awardInfo));
        }
    }
    
    if (sendAwardsHistory.size() == 0 ) {
        
        if (mRankList.size() == 0) {
            return;
        }
        
        int j = 0;
        bool noMoreAwards = false;
        for (int i = 0; i < mRankList.size(); i++) {
            
            int rank = mRankList[i]->getSortRankByType(mSortType);
            
            while(rank != (j+1)) {
                if (j < def->mAwards.size() - 1) {
                    j++;
                    continue;
                }
                noMoreAwards = true;
                break;
            }

            if (true == noMoreAwards) {
                break;
            }
            
            string awards  = def->mAwards[j];
            int sendResult = -1;
            
            if (eSortPet != mSortType) {
                
                sendResult = 0;

                SortInfo* tmp   = (SortInfo*)mRankList[i];
                int    roleid   = tmp->roleid;
                string rolename = tmp->rolename;
                int    serverid = tmp->serverid;
                
                ServerEntity* server = getServerList()->getServer(serverid);
                if (server == NULL) {
                    continue;
                }
                
                if (server->getState() == ServerEntity::kConnect) {
                    
                    req_cs_sendRankListAwards req;
                    req.rolename        = rolename;
                    req.roleid          = roleid;
                    req.awards          = awards;
                    req.rank            = rank;
                    req.rankListType    = mSortType;
                    
                    sendNetPacket(server->getSession(), &req);
                    
                    sendResult = 1;
                }

                Json::Value value;
                value["rolename"]       = rolename;
                value["roleid"]         = roleid;
                value["sendResult"]     = sendResult;
                value["awards"]         = awards;
                value["serverid"]       = serverid;
                value["rank"]           = rank;
                value["rankListType"]   = mSortType;

                string  key         = getKeyByEnum(mSortType);
                string  roleinfo    = xyJsonWrite(value);
                doRedisCmd("hset %s:ServerGroup:%d:%d %d %s", key.c_str(), mGroupId, mAwardSendTime, roleid, roleinfo.c_str());
                
                switch (mSortType) {
                    case eSortBat:
                    {
                        CenterLog log = createCrossServiceBattleRankListAwardHistoryLog();
                        log.setField("serverGroupId", mGroupId);
                        log.setField("roleid", roleid);
                        log.setField("awardSendTime", awardSendTimeBuf);
                        log.setField("roleinfo", roleinfo.c_str());
                    }
                        break;
                    case eSortConsume:
                    {
                        CenterLog log = createCrossServiceConsumeRankListAwardHistoryLog();
                        log.setField("serverGroupId", mGroupId);
                        log.setField("roleid", roleid);
                        log.setField("awardSendTime", awardSendTimeBuf);
                        log.setField("roleinfo", roleinfo.c_str());
                    }
                        break;
                    case eSortRecharge:
                    {
                        CenterLog log = createCrossServiceRechargeRankListAwardHistoryLog();
                        log.setField("serverGroupId", mGroupId);
                        log.setField("roleid", roleid);
                        log.setField("awardSendTime", awardSendTimeBuf);
                        log.setField("roleinfo", roleinfo.c_str());
                    }
                        break;
                    default:
                        break;
                }
            
            }
            else {
                
                sendResult = 0;

                SortPetInfo* tmp   = (SortPetInfo*)mRankList[i];
                int    roleid   = tmp->masterid;
                string rolename = tmp->mastername;
                int    serverid = tmp->serverid;
                
                ServerEntity* server = getServerList()->getServer(serverid);
                if (server == NULL) {
                    continue;
                }
                if (server->getState() == ServerEntity::kConnect) {
                    
                    req_cs_sendRankListAwards req;
                    req.rolename        = rolename;
                    req.roleid          = roleid;
                    req.awards          = awards;
                    req.rank            = rank;
                    req.rankListType    = mSortType;
                    
                    sendNetPacket(server->getSession(), &req);
                    
                    sendResult = 1;
                }
                
                Json::Value value;
                value["rolename"]       = rolename;
                value["roleid"]         = roleid;
                value["sendResult"]     = sendResult;
                value["awards"]         = awards;
                value["serverid"]       = serverid;
                value["rank"]           = rank;
                value["rankListType"]   = mSortType;

                string  key         = getKeyByEnum(mSortType);
                string  roleinfo    = xyJsonWrite(value);
                doRedisCmd("hset %s:ServerGroup:%d:%d %d %s", key.c_str(), mGroupId, mAwardSendTime, i+1,  roleinfo.c_str());
                
                CenterLog log = createCrossServicePetRankListAwardHistoryLog();
                log.setField("serverGroupId", mGroupId);
                log.setField("roleid", roleid);
                log.setField("awardSendTime", awardSendTimeBuf);
                log.setField("roleinfo", roleinfo.c_str());
            }
        }
    }
    else
    {
        //有发送历史记录
        map<int, string>::iterator iter = sendAwardsHistory.begin();
        
        for (; iter != sendAwardsHistory.end(); iter++) {
            string sendInfo = iter->second;
            
            int serverid     = -1;
            int sendResult   = -1;
            int roleid       = -1;
            int rank         = -1;
            int rankListType = -1;
            string rolename  = "";
            string awards    = "";
            
            try
            {
                Json::Value value;
                Json::Reader reader;
                
                if (!reader.parse(sendInfo, value) || value.type() != Json::objectValue)
                {
                    continue;
                }
                
                roleid          = value["roleid"].asInt();
                rolename        = value["rolename"].asString();
                serverid        = value["serverid"].asInt();
                sendResult      = value["sendResult"].asInt();
                awards          = value["awards"].asString();
                rank            = value["rank"].asInt();
                rankListType    = value["rankListType"].asInt();
                
                if (roleid <= 0     || rolename.empty() || serverid <= 0        ||
                    awards.empty()  || rank <= 0        || rankListType != mSortType)
                {
                    continue;
                }
                
            } catch (...)
            {
                return;
            }

            //发送历史记录不成功的重发
            if (sendResult == 0) {
                ServerEntity* server = getServerList()->getServer(serverid);
                if (server == NULL) {
                    continue;
                }
                if (server->getState() == ServerEntity::kConnect) {
                    
                    req_cs_sendRankListAwards req;
                    req.rolename     = rolename;
                    req.roleid       = roleid;
                    req.awards       = awards;
                    req.rank         = rank;
                    req.rankListType = rankListType;
                    
                    sendNetPacket(server->getSession(), &req);
                    
                    sendResult = 1;
                    Json::Value value;
                    
                    value["rolename"]       = rolename;
                    value["roleid"]         = roleid;
                    value["sendResult"]     = sendResult;
                    value["awards"]         = awards;
                    value["serverid"]       = serverid;
                    value["rank"]           = rank;
                    value["rankListType"]   = rankListType;
                    
                    string  key = getKeyByEnum(rankListType);
                    string  roleinfo    = xyJsonWrite(value);
                    doRedisCmd("hset %s:ServerGroup:%d:%d %d %s", key.c_str(), mGroupId, mAwardSendTime, roleid,  xyJsonWrite(value).c_str());
                    
                    switch (mSortType) {
                            case eSortBat:
                            {
                                CenterLog log = createCrossServiceBattleRankListAwardHistoryLog();
                                log.setField("serverGroupId", mGroupId);
                                log.setField("roleid", roleid);
                                log.setField("awardSendTime", awardSendTimeBuf);
                                log.setField("roleinfo", roleinfo.c_str());
                            }
                                break;
                            case eSortConsume:
                            {
                                CenterLog log = createCrossServiceConsumeRankListAwardHistoryLog();
                                log.setField("serverGroupId", mGroupId);
                                log.setField("roleid", roleid);
                                log.setField("awardSendTime", awardSendTimeBuf);
                                log.setField("roleinfo", roleinfo.c_str());
                            }
                                break;
                            case eSortRecharge:
                            {
                                CenterLog log = createCrossServiceRechargeRankListAwardHistoryLog();
                                log.setField("serverGroupId", mGroupId);
                                log.setField("roleid", roleid);
                                log.setField("awardSendTime", awardSendTimeBuf);
                                log.setField("roleinfo", roleinfo.c_str());
                            }
                                break;
                            case eSortPet:
                            {
                                CenterLog log = createCrossServicePetRankListAwardHistoryLog();
                                log.setField("serverGroupId", mGroupId);
                                log.setField("roleid", roleid);
                                log.setField("awardSendTime", awardSendTimeBuf);
                                log.setField("roleinfo", roleinfo.c_str());
                            }
                                break;
                            default:
                                break;
                    }
                }
            }
        }
    }
}



bool
CServerGroup::isOpened()
{
    ServerGroupCfg* cfg = g_ConfigManager.getRankListGroupCfg(mSortType);
    if (cfg == NULL) {
        return false;
    }
    ServerGroupCfgDef* def = cfg->getServerGroupCfgDefByGroupid(mGroupId);
    if (def == NULL) {
        return false;
    }
    
    int beginTime = def->mBeginTime;
    int endTime   = def->mEndTime;
    
    time_t nowtime = time(NULL);
    if (nowtime < beginTime || endTime < nowtime) {
        return false;
    }
    
    return true;
}

void
CServerGroup::addRankListLog()
{
    switch (mSortType) {
        case eSortBat:
        {
            for (int i = 0; i < mRankList.size(); i++) {
                //    printf("%d index: %d, %s: %d, rank: %d\n",  mRankList[i]->getSortValueByType(eSortServerid),
                //           mRankList[i]->getSortValueByType(eSortIndex),
                //           key.c_str(),
                //           mRankList[i]->getSortValueByType(mSortType),
                //           mRankList[i]->getSortRankByType(mSortType));
                //        log_info("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
                int serverid = mRankList[i]->getSortValueByType(eSortServerid);
                ServerEntity* server = getServerEntityById(serverid);
                if (server == NULL) {
                    continue;
                }
                string serverName    = server->getName();
                char buf[1024] = {0};
                sprintf(buf, "%d(%s)", serverid, serverName.c_str());
                
                CenterLog log = createCrossServiceBattleRankListLog();
                log.setField("server", buf);
                log.setField("roleid", mRankList[i]->getSortValueByType(eSortIndex));
                log.setField("value", mRankList[i]->getSortValueByType(mSortType));
                log.setField("rank", mRankList[i]->getSortRankByType(mSortType));
                log.setField("rolename", ((SortInfo*)mRankList[i])->rolename);
                log.setField("serverGroupId", mGroupId);
            }
        }
            break;
        case eSortRecharge:
        {
            for (int i = 0; i < mRankList.size(); i++) {
                //    printf("%d index: %d, %s: %d, rank: %d\n",  mRankList[i]->getSortValueByType(eSortServerid),
                //           mRankList[i]->getSortValueByType(eSortIndex),
                //           key.c_str(),
                //           mRankList[i]->getSortValueByType(mSortType),
                //           mRankList[i]->getSortRankByType(mSortType));
                //        log_info("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
                int serverid = mRankList[i]->getSortValueByType(eSortServerid);
                ServerEntity* server = getServerEntityById(serverid);
                if (server == NULL) {
                    continue;
                }
                string serverName    = server->getName();
                char buf[1024] = {0};
                sprintf(buf, "%d(%s)", serverid, serverName.c_str());
                
                CenterLog log = createCrossServiceRechargeRankListLog();
                log.setField("server", buf);
                log.setField("roleid", mRankList[i]->getSortValueByType(eSortIndex));
                log.setField("value", mRankList[i]->getSortValueByType(mSortType));
                log.setField("rank", mRankList[i]->getSortRankByType(mSortType));
                log.setField("rolename", ((SortInfo*)mRankList[i])->rolename);
                log.setField("serverGroupId", mGroupId);
            }
        }
            break;
        case eSortConsume:
        {
            for (int i = 0; i < mRankList.size(); i++) {
                //    printf("%d index: %d, %s: %d, rank: %d\n",  mRankList[i]->getSortValueByType(eSortServerid),
                //           mRankList[i]->getSortValueByType(eSortIndex),
                //           key.c_str(),
                //           mRankList[i]->getSortValueByType(mSortType),
                //           mRankList[i]->getSortRankByType(mSortType));
                //        log_info("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
                int serverid = mRankList[i]->getSortValueByType(eSortServerid);
                ServerEntity* server = getServerEntityById(serverid);
                if (server == NULL) {
                    continue;
                }
                string serverName    = server->getName();
                char buf[1024] = {0};
                sprintf(buf, "%d(%s)", serverid, serverName.c_str());
                
                CenterLog log = createCrossServiceConsumeRankListLog();
                log.setField("server", buf);
                log.setField("roleid", mRankList[i]->getSortValueByType(eSortIndex));
                log.setField("value", mRankList[i]->getSortValueByType(mSortType));
                log.setField("rank", mRankList[i]->getSortRankByType(mSortType));
                log.setField("rolename", ((SortInfo*)mRankList[i])->rolename);
                log.setField("serverGroupId", mGroupId);
            }
        }
            break;
        case eSortPet:
        {
            for (int i = 0; i < mRankList.size(); i++) {
                //    printf("%d index: %d, %s: %d, rank: %d\n",  mRankList[i]->getSortValueByType(eSortServerid),
                //           mRankList[i]->getSortValueByType(eSortIndex),
                //           key.c_str(),
                //           mRankList[i]->getSortValueByType(mSortType),
                //           mRankList[i]->getSortRankByType(mSortType));
                //        log_info("+++++++++++++++++++++++++++++++++++++++++++++++++++++");
                int serverid = mRankList[i]->getSortValueByType(eSortServerid);
                ServerEntity* server = getServerEntityById(serverid);
                if (server == NULL) {
                    continue;
                }
                string serverName    = server->getName();
                char buf[1024] = {0};
                sprintf(buf, "%d(%s)", serverid, serverName.c_str());
                
                CenterLog log = createCrossServicePetRankListLog();
                log.setField("server", buf);
                log.setField("petid", mRankList[i]->getSortValueByType(eSortIndex));
                log.setField("value", mRankList[i]->getSortValueByType(mSortType));
                log.setField("rank", mRankList[i]->getSortRankByType(mSortType));
                log.setField("rolename", ((SortPetInfo*)mRankList[i])->mastername);
                log.setField("petmod", ((SortPetInfo*)mRankList[i])->petmod);
                log.setField("serverGroupId", mGroupId);
            }
        }
            break;
            
        default:
            break;
    }
}

void
CServerGroup::noticeServerEntityOnRefresh()
{
    notify_sync_refresh_crossServiceRankList req;

    map<int, ServerEntity*>::iterator iter;
    for (iter = mServerMap.begin(); iter != mServerMap.end(); iter++) {
        ServerEntity* tmp = iter->second;
        if (tmp == NULL || tmp->getState() == ServerEntity::kDisconnect) {
            continue;
        }
        int sessionid = tmp->getSession();
//        log_info("serverid: " << tmp->getServerId());
        sendNetPacket(sessionid, &req);
    }
}



