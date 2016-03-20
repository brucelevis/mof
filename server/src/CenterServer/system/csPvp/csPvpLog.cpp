#include "csPvpLog.h"
#include "dbMgr.h"
#include "csPvp.h"
#include <time.h>
#include "json/json.h"


string 
PvpLog::encode() {
	Json::Value value;
	value["role_id"] = strFormat("%lld", mRoleId);
	value["role_name"] = mRoleName;
	value["role_rank"] = mRoleRank;
	value["target_id"] = strFormat("%lld", mTargetId);
	value["target_name"] = mTargetName;
	value["target_rank"] = mTargetRank;
	value["swap_rank"] = mSwapRank;
	value["is_win"] = mIsWin;
	value["time"] = mTime;
	return Json::FastWriter().write(value);
}

void 
PvpLog::decode(const char* str) {
	Json::Value value;
	Json::Reader reader;
	reader.parse(str, value);
	if (!value.isObject()) {
		return;
	}

	mRoleId = atoll(value["role_id"].asString().c_str());
	mRoleName = value["role_name"].asString();
	mRoleRank = value["role_rank"].asInt();
	mTargetId = atoll(value["target_id"].asString().c_str());
	mTargetName = value["target_name"].asString();
	mTargetRank = value["target_rank"].asInt();
	mSwapRank = value["swap_rank"].asInt();
	mIsWin = value["is_win"].asInt();
	mTime = value["time"].asInt();
}

PvpLogMgr*
PvpLogMgr::create(CsPvp* csPvp) {
	PvpLogMgr* logMgr = new PvpLogMgr;
	logMgr->mCsPvp = csPvp;
	return logMgr;
}

void
PvpLogMgr::load() {
	RedisResult result(redisCmd("lrange cspvp:%d:log 0 -1", mCsPvp->getGroupId()));
	int count = result.getHashElements();
	for (int i = 0; i < count; i++) {
		string str = result.readHash(i);
		PvpLog* pvpLog = new PvpLog;
		pvpLog->decode(str.c_str());
		addLog2Memory(pvpLog);
	}
}

void
PvpLogMgr::clear() {
	for (int i = 0; i < mLogs.size(); i++) {
		delete mLogs[i];
	}
	mLogs.clear();
	mRoleLogs.clear();
	doRedisCmd("del cspvp:%d:log", mCsPvp->getGroupId());
}

void
PvpLogMgr::addLog2Memory(PvpLog* pvpLog) {
	mLogs.push_back(pvpLog);
	roleRefLog(pvpLog->mRoleId, pvpLog);
	roleRefLog(pvpLog->mTargetId, pvpLog);
}

void 
PvpLogMgr::addLog(PvpRoleId roleId, string& roleName, int rank, 
	PvpRoleId targetId, string& targetName, int targetRank, 
	bool isWin, bool swapRank) {
	PvpLog* pvpLog = new PvpLog;
	pvpLog->mRoleId = roleId;
	pvpLog->mRoleName = roleName;
	pvpLog->mRoleRank = rank;
	pvpLog->mTargetId = targetId;
	pvpLog->mTargetName = targetName;
	pvpLog->mTargetRank = targetRank;
	pvpLog->mIsWin = isWin;
	pvpLog->mSwapRank = swapRank;
	pvpLog->mTime = time(NULL);

	addLog2Memory(pvpLog);
	doRedisCmd("lpush cspvp:%d:log %s", mCsPvp->getGroupId(), pvpLog->encode().c_str());
}

void
PvpLogMgr::roleRefLog(PvpRoleId pvpRoleId, PvpLog* pvpLog) {
	map<PvpRoleId, vector<PvpLog*> >::iterator iter = mRoleLogs.find(pvpRoleId);
	if (iter == mRoleLogs.end()) {
		iter = mRoleLogs.insert(make_pair(pvpRoleId, vector<PvpLog*>())).first;
	}

	iter->second.push_back(pvpLog);
}


void 
PvpLogMgr::traverseRoleLog(PvpRoleId pvpRoleId, Traverser* traverser) {
	map<PvpRoleId, vector<PvpLog*> >::iterator iter = mRoleLogs.find(pvpRoleId);
	if (iter == mRoleLogs.end()) {
		return;
	}

	vector<PvpLog*>& logs = iter->second;
	for (int i = logs.size() - 1; i >= 0; i--) {
		if (!traverser->callback(logs[i])) {
			break;
		}
	}
}



