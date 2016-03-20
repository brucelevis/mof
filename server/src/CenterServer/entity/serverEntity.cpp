#include "serverEntity.h"
#include "msg.h"
#include "EnumDef.h"
#include "sendMessage.h"
#include "dbMgr.h"
#include <time.h>
#include "csPvp/csPvpRankAward.h"


ServerEntity*
ServerEntity::create(int serverId, const char* name) {
	ServerEntity* server = new ServerEntity;
    server->mSession = 0;
    server->mServerId = serverId;
    server->mName = name;
    if (!server->init()) {
    	delete server;
    	server = NULL;
    }

    return server;
}


bool 
ServerEntity::init() {
    mRoleDataMgr = new CPaihangInfoMgr;
    mRoleDataMgr->init(mServerId, "role");
    
    mPetDataMgr = new CPaihangInfoMgr;
    mPetDataMgr->init(mServerId, "pet");
    
    vector<CmpFun> batListFun;
    batListFun.push_back(CSort::cmpBat);
//    batListFun.push_back(CSort::cmpAddress);

    mBatList = new CPaihangList;
    mBatList->init(mServerId, eSortBat, batListFun);
    
    vector<CmpFun> consumeListFun;
    consumeListFun.push_back(CSort::cmpConsume);
//    consumeListFun.push_back(CSort::cmpAddress);
    mConsumeList = new CPaihangList;
    mConsumeList->init(mServerId, eSortConsume, consumeListFun);
    
    vector<CmpFun> rechargeListFun;
    rechargeListFun.push_back(CSort::cmpRecharge);
//    rechargeListFun.push_back(CSort::cmpAddress);

    mRechargeList = new CPaihangList;
    mRechargeList->init(mServerId, eSortRecharge, rechargeListFun);
    
    
    vector<CmpFun> PetListFun;
    PetListFun.push_back(CSort::cmpPetBattle);
//    PetListFun.push_back(CSort::cmpAddress);
    mPetList = new CPaihangList;
    mPetList->init(mServerId, eSortPet, PetListFun);

    mCsPvpRankAward = CsPvpRankAward::create(this);

    mState = ServerEntity::kDisconnect;

    load();
    

    return true;
}

void
ServerEntity::load() {
}

void 
ServerEntity::deinit() {
    delete mRoleDataMgr;
    mRoleDataMgr = NULL;
    
    delete mPetDataMgr;
    mPetDataMgr = NULL;
    
    delete mBatList;
    mBatList = NULL;
    
    delete mConsumeList;
    mConsumeList = NULL;
    
    delete mRechargeList;
    mRechargeList = NULL;
}


CsPvpRankAward*   
ServerEntity::getCsPvpRankAward() {
    return mCsPvpRankAward;
}

void
ServerEntity::onConnect(int session) {
    mSession = session;
    mState = ServerEntity::kConnect;

    req_cs_loadPaihangData reqBat;
    reqBat.type = eSortBat;
    sendNetPacket(mSession, &reqBat);
    
    req_cs_loadPaihangData reqConsume;
    reqConsume.type = eSortConsume;
    sendNetPacket(mSession, &reqConsume);
    
    req_cs_loadPaihangData reqRecharge;
    reqRecharge.type = eSortRecharge;
    sendNetPacket(mSession, &reqRecharge);
    
    req_cs_loadPaihangData reqPet;
    reqPet.type = eSortPet;
    sendNetPacket(mSession, &reqPet);
    
}

void
ServerEntity::onDisconnect() {
    mSession = 0;
    mState = ServerEntity::kDisconnect;
}

void
ServerEntity::sendRemote(void* data, int len) {
}

void
ServerEntity::keepAlive() {
}


void
ServerEntity::onHeartBeat()
{
    mBatList->onHeartBeat();
    mConsumeList->onHeartBeat();
    mRechargeList->onHeartBeat();
    mPetList->onHeartBeat();
    mCsPvpRankAward->update();
}



CPaihangList*
ServerEntity::getPaihangList(int type)
{
    switch (type) {
        case eSortBat:
            return mBatList;
        case eSortConsume:
            return mConsumeList;
        case eSortRecharge:
            return mRechargeList;
        case eSortPet:
            return mPetList;
        default:
            break;
    }
    return NULL;
}


CPaihangInfoMgr*
ServerEntity::getObjDataMgr(int type)
{
    if (eSortPet == type) {
        return mPetDataMgr;
    }
    return mRoleDataMgr;
}

