#pragma once


#include <stdio.h>
#include <map>
#include <string>
#include "SortDataMgr.h"
#include "RankList.h"
#include "Defines.h"

using namespace std;

class CsPvpRankAward;

class ServerEntity
{
public:
    static ServerEntity* create(int serverId, const char* name);

    ServerEntity() {}
    ~ServerEntity() {deinit();}
    
    bool init();
    void load();
    void deinit();

public:
    READWRITE(int, mServerId, ServerId)
    READWRITE(int, mSession, Session)
    READWRITE(int, mState, State)
    READWRITE(string, mName, Name)
    READWRITE(int, mLastKeepAlive, LastKeepAlive)
    
    enum {
        kConnect,
        kDisconnect,
    };

    void onConnect(int session);
    void onDisconnect();
    void keepAlive();

    void sendRemote(void* data, int len);
    
    CPaihangInfoMgr*    getObjDataMgr(int type);
    CPaihangList*       getPaihangList(int type);
    CsPvpRankAward*   getCsPvpRankAward();
    
    void onHeartBeat();
    
private:
    CsPvpRankAward* mCsPvpRankAward;

    CPaihangInfoMgr* mRoleDataMgr;
    CPaihangInfoMgr* mPetDataMgr;
    
    CPaihangList*   mBatList;
    CPaihangList*   mConsumeList;
    CPaihangList*   mRechargeList;
    CPaihangList*   mPetList;
};
