//
//  serverGroupMgr.cpp
//  CenterServer
//
//  Created by Cheung Zirpon on 15-6-24.
//
//

#include "serverGroupMgr.h"
#include "configManager.h"

CServerGroupMgr* getRankListGroupMgrByType(int type)
{
    switch (type) {
        case eSortBat:
            return g_BatRankListGroupMgr;
        case eSortConsume:
            return g_ConsumeRankListGroupMgr;
        case eSortRecharge:
            return g_RechargeRankListGroupMgr;
        case eSortPet:
            return g_PetRankListGroupMgr;
        default:
            break;
    }
    return NULL;
}

void
CServerGroupMgr::init(int type)
{
    mSortType = type;
    
    vector<int> groupList;
    ServerGroupCfg* cfg = g_ConfigManager.getRankListGroupCfg(mSortType);
    if (cfg == NULL) {
        return;
    }
    cfg->getGroupList(groupList);
    
    for (int i = 0; i <  groupList.size(); i++) {
        int groupid = groupList[i];
        ServerGroupCfgDef* def = cfg->getServerGroupCfgDefByGroupid(groupid);
        assert(def != NULL);
        
        CServerGroup* tmp = CServerGroup::createGroup(def);
        
        if (tmp != NULL) {
            mGroupMgr.insert(make_pair(groupid, tmp));
        }
    }
}

bool
CServerGroupMgr::addServerEntity(ServerEntity* server)
{
    if (server == NULL) {
        return false;
    }
    
    ServerGroupCfg* cfg = g_ConfigManager.getRankListGroupCfg(mSortType);
    if (cfg == NULL) {
        return false;
    }
    int groupId = cfg->getServerGroupId(server->mServerId);
    if (groupId == -1) {
        return false;
    }
    
    CServerGroup* group = getServerGroupByGroupId(groupId);
    if (group == NULL) {
        return false;
    }
    
    if (!group->addServerEntity(server)) {
        return false;
    }
    
    return true;
    
}

CServerGroup*
CServerGroupMgr::getServerGroupByGroupId(int groupid)
{
    map<int, CServerGroup*>::iterator iter = mGroupMgr.find(groupid);
    if (iter != mGroupMgr.end()) {
        return iter->second;
    }
    return NULL;
}

ServerEntity*
CServerGroupMgr::getServerEntity(int serverid)
{
    ServerGroupCfg* cfg = g_ConfigManager.getRankListGroupCfg(mSortType);
    if (cfg == NULL) {
        return NULL;
    }
    int groupId = cfg->getServerGroupId(serverid);
    CServerGroup* group = getServerGroupByGroupId(groupId);
    if (group == NULL) {
        return NULL;
    }
    return group->getServerEntityById(serverid);
}



CServerGroup*
CServerGroupMgr::getServerGroupByServerId(int serverid)
{
    ServerGroupCfg* cfg = g_ConfigManager.getRankListGroupCfg(mSortType);
    if (cfg == NULL) {
        return NULL;
    }
    int groupid = cfg->getServerGroupId(serverid);
    return getServerGroupByGroupId(groupid);
}

void
CServerGroupMgr::onHeartBeat()
{
    map<int, CServerGroup*>::iterator iter;
    for (iter = mGroupMgr.begin(); iter != mGroupMgr.end(); iter++) {
        iter->second->onHeartBeat();
    }
}





