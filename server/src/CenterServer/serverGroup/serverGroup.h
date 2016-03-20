//
//  serverGroup.h
//  CenterServer
//
//  Created by Cheung Zirpon on 15-6-24.
//
//

#ifndef __CenterServer__serverGroup__
#define __CenterServer__serverGroup__

#include <iostream>
#include "serverEntity.h"
#include <set>
#include "dbMgr.h"

using namespace std;

class CServerGroup
{
public:
    CServerGroup()
    {}
    ~CServerGroup(){}
    
    static CServerGroup* createGroup(ServerGroupCfgDef* groupcfg);
    

    bool init();
    void load();

    bool                addServerEntity(ServerEntity* server);
    ServerEntity*       getServerEntityById(int serverid);
    bool                checkServerInclude(int serverid);

    void                freshRankList(vector<PaihangInfo*> &list);
    void                rankListSort();
    void                rankListShotCutOnDB();
    
    void                onCallRemoveDataByServerid(int serverid);
    
    void                onHeartBeat();
    
    void                sendAwards(int tick);
    
    bool                isOpened();
    
    void                getRankList(vector<PaihangInfo*> &ret);
    int                 getSortRankByObjId(int objId);
    
    void                addRankListLog();
    void                noticeServerEntityOnRefresh();

private:
    int                     mGroupId;
    
    map<int, ServerEntity*> mServerMap;
    int                     mAwardSendTime;
    vector<PaihangInfo*>    mRankList;
    int                     mSortType;
};

#endif /* defined(__CenterServer__serverGroup__) */
