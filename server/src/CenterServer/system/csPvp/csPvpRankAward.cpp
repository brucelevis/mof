#include "csPvpRankAward.h"
#include "serverEntity.h"
#include "centerLog.h"

extern CenterLog createCsPvpRoleRankAwardLog();
extern CenterLog createCsPvpServerRankAwardLog();

string
CsPvpRoleRankAward::encode() {
	Json::Value value;
	value["role_id"] = mRoleId;
	value["award"] = mAward;
	value["rank"] = mRank;
	value["getted"] = mGetted;
	return Json::FastWriter().write(value);
}

void
CsPvpRoleRankAward::decode(const char* str) {
	Json::Value value;
	Json::Reader reader;
	reader.parse(str, value);
	if (!value.isObject()) {
		return;
	}
	mRank = value["rank"].asInt();
	mRoleId = value["role_id"].asInt();
	mAward = value["award"].asInt();
	mGetted = value["getted"].asBool();
}


CsPvpRankAward*
CsPvpRankAward::create(ServerEntity* server) {
	CsPvpRankAward* serverAward = new CsPvpRankAward;
	serverAward->mServer = server;
	serverAward->init();
	return serverAward;
}

bool
CsPvpRankAward::init() {
	mServerAward.mGetted = true;
	load();
	return true;
}

void 
CsPvpRankAward::load() {
    RedisResult result(redisCmd("hmget cspvp:rolerankaward:%d rank weektime award getted", mServer->getServerId()));
    mServerAward.mRank = result.readHash(0, 0);
    mServerAward.mWeekTime = result.readHash(1, 0);
    mServerAward.mAward = result.readHash(2);
    mServerAward.mGetted = result.readHash(3, 1);
    
    result.setData(redisCmd("hgetall cspvp:serverrankaward:%d", mServer->getServerId()));
    int count = result.getHashElements();
    for (int i = 0; i < count; i += 2) {
        int roleId = result.readHash(i, 0);
        string awardStr = result.readHash(i + 1);
        
        CsPvpRoleRankAward* award;
        award->decode(awardStr.c_str());
        mRoleAward[roleId] = award;
    }
}

void 
CsPvpRankAward::clear() {
	doRedisCmd("del cspvp:rolerankaward:%d", mServer->getServerId());
	doRedisCmd("del cspvp:serverrankaward:%d", mServer->getServerId());
    
	for (map<int, CsPvpRoleRankAward*>::iterator iter = mRoleAward.begin(); iter != mRoleAward.end(); ) {
		CsPvpRoleRankAward* award = iter->second;
		delete award;
	}
    mRoleAward.clear();
    
    mServerAward.mAward = "";
    mServerAward.mGetted = true;
}

void
CsPvpRankAward::update() {
	if (mServer->getState() == ServerEntity::kDisconnect) {
		return;
	}

	if (mServerAward.mGetted == false) {
		notify_cs_cspvp_serverrank_award notify;
		notify.rank = mServerAward.mRank;
		notify.award = mServerAward.mAward;
        notify.week_time = mServerAward.mWeekTime;
		sendNetPacket(mServer->getSession(), &notify);
		mServerAward.mGetted = true;
	}

	for (map<int, CsPvpRoleRankAward*>::iterator iter = mRoleAward.begin(); iter != mRoleAward.end(); ) {
		CsPvpRoleRankAward* award = iter->second;
		notify_cs_cspvp_rolerank_award notify;
		notify.rank = award->mRank;
		notify.award = award->mAward;
		notify.role_id = award->mRoleId;
		sendNetPacket(mServer->getSession(), &notify);
		delete award;

		mRoleAward.erase(iter++);
	}
    
    clear();
}

void 
CsPvpRankAward::addServerAward(int rank, int weekTime, const string& award) {
	mServerAward.mRank = rank;
	mServerAward.mAward = award;
	mServerAward.mGetted = false;
    mServerAward.mWeekTime = weekTime;
    doRedisCmd("hmset cspvp:serveerrankaward:%d rank %d weektime %d award %s getted 0",
               mServer->getServerId(), rank, weekTime, award.c_str());
    
    CenterLog log = createCsPvpRoleRankAwardLog();
    log.setField("serverid", mServer->getServerId());
    log.setField("award", award.c_str());
    log.setField("weektime", weekTime);
    log.setField("rank", rank);
}

void 
CsPvpRankAward::addRoleAward(int roleId, int rank, const string& award) {
	CsPvpRoleRankAward* roleAward = new CsPvpRoleRankAward;
	roleAward->mGetted = false;
	roleAward->mRank = rank;
	roleAward->mAward = award;
	roleAward->mRoleId = roleId;
	mRoleAward[roleId] = roleAward;
    doRedisCmd("hmset cspvp:rolerankaward:%d %d %s", mServer->getServerId(), roleId, roleAward->encode().c_str());
    
    CenterLog log = createCsPvpServerRankAwardLog();
    log.setField("serverid", mServer->getServerId());
    log.setField("award", award.c_str());
    log.setField("roleid", roleId);
    log.setField("rank", rank);
}
