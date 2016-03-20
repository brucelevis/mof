//
//  serverGroupMgr.h
//  CenterServer
//
//  Created by Cheung Zirpon on 15-6-24.
//
//

#ifndef __CenterServer__serverGroupMgr__
#define __CenterServer__serverGroupMgr__

#include <iostream>
#include <map>
#include <set>

#include "serverGroup.h"
#include "serverEntity.h"

#include "paihangSortInfo.h"

using namespace std;

class CServerGroupMgr
{
    
public:
    void init(int type);
    
    bool            addServerEntity(ServerEntity* server);
    CServerGroup*   getServerGroupByGroupId(int groupid);
    CServerGroup*   getServerGroupByServerId(int serverid);

    ServerEntity*   getServerEntity(int serverid);
    
    void            onHeartBeat();
    
    
private:
    map<int, CServerGroup*> mGroupMgr;
    int                mSortType;
};

extern CServerGroupMgr* g_BatRankListGroupMgr;
extern CServerGroupMgr* g_ConsumeRankListGroupMgr;
extern CServerGroupMgr* g_RechargeRankListGroupMgr;
extern CServerGroupMgr* g_PetRankListGroupMgr;

extern CServerGroupMgr* getRankListGroupMgrByType(int type);



#endif /* defined(__CenterServer__serverGroupMgr__) */
