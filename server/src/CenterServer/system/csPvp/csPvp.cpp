#include "csPvp.h"
#include "dbMgr.h"
#include "datetime.h"
#include "configManager.h"
#include "serverCfg.h"
#include "RankList.h"
#include "serverEntity.h"
#include "serverList.h"
#include "DataCfg.h"
#include "csPvp/csPvpRankAward.h"


extern int finishCsPvpScore(int rank);

CsPvpMgr* 
getCsPvpMgr() {
	static CsPvpMgr csPvpMgr;
	return &csPvpMgr;
}

CsPvp*
CsPvp::create(int groupId) {
	CsPvp* csPvp = new CsPvp;
	csPvp->mGroupId = groupId;
	csPvp->init();
	return csPvp;
}

bool
CsPvp::init() {
	mPvpServerRankList = PvpServerRankList::create(this);
	mPvpRoleRankList = PvpRoleRankList::create(this);
	mPvpStateMgr = PvpStateMgr::create(this);
	mPvpLogMgr = PvpLogMgr::create(this);
	load();

	return true;
}

void
CsPvp::load() {
	//load group info
	RedisResult result(redisCmd("hmget cspvp:%d starttime state", mGroupId));
	mStartTime = result.readHash(0, 0);;
	mState = result.readHash(1, 0);

	time_t nowTime = time(NULL);
	if (CsPvpGroupCfg::isGroupClose(mGroupId, nowTime)) {
		closeActivity();
		return;
	} 

	int thisWeekStartTime = DateTime::getWeekDateTime();
	if (thisWeekStartTime > mStartTime) {
		startActivity();
	} else {
		mPvpRoleRankList->load();
		mPvpServerRankList->load();
		mPvpLogMgr->load();
	}
}

void
CsPvp::checkStartActivity() {
	time_t nowTime = time(NULL);
	if (CsPvpGroupCfg::isGroupClose(mGroupId, nowTime)) {
		closeActivity();
	} else {
		startActivity();
	}
}

void
CsPvp::update() {
	int thisWeekStartTime = DateTime::getWeekDateTime();
	if (thisWeekStartTime != mStartTime) {
		checkStartActivity();
	}

	time_t nowTime = time(NULL);
	tm nowTm;
	localtime_r(&nowTime, &nowTm);
	switch (mState) {
		case kActivityStart : {
			preBattle();
			break;
		}
		case kPreBattle: {
			if (CrossServiceWarWorshipCfg::isWorshipEnd(nowTm.tm_wday, nowTm.tm_hour, nowTm.tm_min)) {
				startBattle();
			}
			break;
		}
		case kInBattle: {
			if (CrossServiceWarWorshipCfg::isBattleEnd(nowTm.tm_wday, nowTm.tm_hour, nowTm.tm_min)) {
				endBattle();
			}
			break;
		}
		case kEndBattle: {
			endActivity();
			break;
		}
		case kActivityEnd:
		case kActivityClose: {
			break;
		}
	}
}

vector<PvpRoleInfo*>
CsPvp::getChallengeData(int rank) {
	int challengeNum = 6;

	int forthNum = rank <= challengeNum ? rank - 1 : challengeNum;
	vector<PvpRoleInfo*> forthRoles = getChallengeDataForth(rank, forthNum);
	if (forthRoles.size() < challengeNum) {
		int backNum = challengeNum - forthRoles.size();
		vector<PvpRoleInfo*> backRoles = getChallengeDataBack(rank, backNum);
		forthRoles.insert(forthRoles.end(), backRoles.begin(), backRoles.end());
	}
	return forthRoles;
}


vector<PvpRoleInfo*> 
CsPvp::getChallengeDataForth(int rank, int num) {
	vector<PvpRoleInfo*> result;
	for (int i = num; i > 0; i--) {
		int roleRank = rank - i;
		PvpRoleInfo* roleInfo = mPvpRoleRankList->getRoleInfoByRank(roleRank);
		if (roleInfo == NULL) {
			break;
		}
		result.push_back(roleInfo);
	}

	return result;
}

vector<PvpRoleInfo*> 
CsPvp::getChallengeDataBack(int rank, int num) {
	vector<PvpRoleInfo*> result;
	for (int i = 1; i <= num; i++) {
		int testRank = rank + i;
		PvpRoleInfo* roleInfo = mPvpRoleRankList->getRoleInfoByRank(testRank);
		if (roleInfo == NULL) {
			break;
		}
		result.push_back(roleInfo);
	}

	return result;
}



PvpRoleRankList* 
CsPvp::getPvpRoleRankList() {
	return mPvpRoleRankList;
}

PvpServerRankList* 
CsPvp::getPvpServerRankList() {
	return mPvpServerRankList;
}

PvpStateMgr* 
CsPvp::getPvpStateMgr() {
	return mPvpStateMgr;
}

PvpLogMgr* 
CsPvp::getPvpLogMgr() {
	return mPvpLogMgr;
}

void
CsPvp::startActivity() {
	mPvpLogMgr->clear();
	mPvpServerRankList->clear();
	mPvpRoleRankList->clear();
	mState = kActivityStart;
	mStartTime = DateTime::getWeekDateTime();
	doRedisCmd("hmset cspvp:%d starttime %d state %d", mGroupId, mStartTime, mState);
}

void 
CsPvp::preBattle() {
	mState = kPreBattle;
	doRedisCmd("hmset cspvp:%d state %d", mGroupId, mState);
}

void 
CsPvp::startBattle() {
	//获取战斗力排行榜数据
	mState = CsPvp::kInBattle;
	genRoleRankList();
	mPvpLogMgr->clear();
	mPvpServerRankList->clear();
	mPvpStateMgr->clear();
	doRedisCmd("hmset cspvp:%d state %d", mGroupId, mState);
}

void 
CsPvp::endBattle() {
	mState = CsPvp::kEndBattle;
	doRedisCmd("hmset cspvp:%d state %d", mGroupId, mState);
}

void
CsPvp::endActivity() {
	map<int, int> serverScore;
	for (int rank = 1;;rank++) {
		PvpRoleInfo* pvpRoleInfo = mPvpRoleRankList->getRoleInfoByRank(rank);
		if (pvpRoleInfo == NULL) {
			break;
		}

		int score = finishCsPvpScore(rank);
		int serverId = getPvpRoleServerId(pvpRoleInfo->mRoleId);
		pvpRoleInfo->mScore += score;
		serverScore[serverId] += score;
	}

	for (map<int, int>::iterator iter = serverScore.begin(); iter != serverScore.end(); iter++) {
		mPvpServerRankList->addScore(iter->first, iter->second);
	}

	sendAward();

	mState = CsPvp::kActivityEnd;
	doRedisCmd("hmset cspvp:%d state %d", mGroupId, mState);
}


void 
CsPvp::closeActivity() {
	mStartTime = DateTime::getWeekDateTime();
	mState = kActivityClose;
	doRedisCmd("hmset cspvp:%d starttime %d state %d", mGroupId, mStartTime, mState);
}

void
CsPvp::sendAward() {
	for (int rank = 1;;rank++) {
		PvpRoleInfo* pvpRoleInfo = mPvpRoleRankList->getRoleInfoByRank(rank);
		if (pvpRoleInfo == NULL) {
			break;
		}

		int serverId = getPvpRoleServerId(pvpRoleInfo->mRoleId);
		int roleId = getPvpRoleRoleId(pvpRoleInfo->mRoleId);

		ServerEntity* server = getServerList()->getServer(serverId);
		if (server == NULL) {
			continue;
		}

		string roleAward = CrossServiceWarRankingsimpleCfg::getAwardByRank(rank);
        if (roleAward.empty()) {
            break;
        }
		server->getCsPvpRankAward()->addRoleAward(roleId, pvpRoleInfo->mRank, roleAward);
	}

	for (int rank = 1;;rank++) {
		PvpServerInfo* pvpServerInfo = mPvpServerRankList->getServerInfoByRank(rank);
		if (pvpServerInfo == NULL) {
			break;
		}

		int serverId = pvpServerInfo->mServerId;
		ServerEntity* server = getServerList()->getServer(serverId);
		if (server == NULL) {
			continue;
		}

		string serverAward = CrossServiceWarRankingserviceCfg::getAwardByRank(rank);
        if (serverAward.empty()) {
            break;
        }
		server->getCsPvpRankAward()->addServerAward(pvpServerInfo->mRank, mStartTime, serverAward);
	}

}

void 
CsPvp::genRoleRankList() {
	mPvpRoleRankList->clear();

	ServerCfg* serverCfg = g_ConfigManager.getServerCfg();
	vector<int> cfgServerIds;
	if (CsPvpGroupCfg::sAllServer) {
		cfgServerIds = serverCfg->getAllRunId();
	} else {
		CsPvpGroupCfgDef* groupCfgDef = CsPvpGroupCfg::getCfgDef(mGroupId);
		if (groupCfgDef == NULL) {
			return;
		}
		cfgServerIds = groupCfgDef->mServerIds;
	}

	vector<vector<PaihangInfo*> > paihangs;
	vector<int> paihangServers;
	for (vector<int>::iterator iter = cfgServerIds.begin(); iter != cfgServerIds.end(); iter++) {
		int serverId = *iter;
		if (!serverCfg->existRunId(serverId)) {
			continue;
		}

		ServerEntity* server = getServerList()->getServer(serverId);
		if (server == NULL) {
			continue;
		}

		CPaihangList* paihang = server->getPaihangList(eSortBat);
		if (paihang == NULL) {
			continue;
		}

		paihangs.push_back(vector<PaihangInfo*>());
		paihangServers.push_back(serverId);

		paihang->getList(*(paihangs.rbegin()));
	}

	vector<int> curIndices;
	curIndices.resize(paihangs.size());
	if (paihangs.size() <= 0) {
		return;
	}

	//归并
	vector<SortInfo*> batRankList;
	vector<int> rankServerIds;
	for (;;) {
		int i;
		int maxIndex;
		for (i = 0; i < curIndices.size(); i++) {
			int curIndex = curIndices[i];
			if (curIndex < paihangs[i].size()) {
				break;
			}
		}

		if (i == curIndices.size()) {
			break;
		}

		int curIndex = curIndices[i];
		SortInfo* maxInfo = (SortInfo*)paihangs[i][curIndex];
		int maxServerId = paihangServers[i];
		maxIndex = i;
		for (i = i + 1; i < curIndices.size(); i++) {
			curIndex = curIndices[i];
			if (curIndex >= paihangs[i].size()) {
				continue;
			}

			SortInfo* info = (SortInfo*)paihangs[i][curIndex];
			if (CSort::cmpBat(maxInfo, info)) {
				maxInfo = info;
				maxServerId = paihangServers[i];
				maxIndex = i;
			}
		}
		batRankList.push_back(maxInfo);
		rankServerIds.push_back(maxServerId);
		curIndices[maxIndex]++;
	}


	for (int i = 0; i < batRankList.size(); i++) {
		SortInfo* sortInfo = batRankList[i];
		int serverId = rankServerIds[i];
		PvpRoleId pvpRoleId = genPvpRoleId(serverId, sortInfo->roleid);
		int lvl = sortInfo->lvl;
		int bat = sortInfo->battle;
		int roletype = sortInfo->job;
		string rolename = sortInfo->rolename;

		int openLvl = CrossServiceWarWorshipCfg::getOpenLvl();
		if (lvl < openLvl) {
			continue;
		}

		mPvpRoleRankList->updateRoleInfo(pvpRoleId, rolename, roletype, lvl, bat);
	}

}

bool
CsPvpMgr::init() {
	load();
	return true;
}

void
CsPvpMgr::load() {
	if (CsPvpGroupCfg::sAllServer) {
		CsPvp* csPvp = CsPvp::create(0);
		mData[0] = csPvp;
		ServerCfg* serverCfg = g_ConfigManager.getServerCfg();
		vector<int> allRunIds = serverCfg->getAllRunId();
		for (vector<int>::iterator iter = allRunIds.begin(); iter != allRunIds.end(); iter++) {
			mServerIdIndex[*iter] = csPvp;
		}
	} else {
		map<int, CsPvpGroupCfgDef*>::iterator iter;
		for (iter = CsPvpGroupCfg::sGroups.begin(); iter != CsPvpGroupCfg::sGroups.end(); iter++) {
			int groupId = iter->first;
			CsPvpGroupCfgDef* groupCfgDef = iter->second;

			CsPvp* csPvp = CsPvp::create(groupId);
			mData[groupId] = csPvp;
			vector<int>& serverIds = groupCfgDef->mServerIds;
			ServerCfg* serverCfg = g_ConfigManager.getServerCfg();
			for (int i = 0; i < serverIds.size(); i++) {
				int serverId = serverIds[i];
				if (!serverCfg->existRunId(serverId)) {
					continue;
				}
				mServerIdIndex[serverId] = csPvp;
			}
		}
	}
}

CsPvp* 
CsPvpMgr::getCsPvp(int groupId) {
	map<int, CsPvp*>::iterator iter = mData.find(groupId);
	if (iter == mData.end()) {
		return NULL;
	}
	return iter->second;
}

CsPvp* 
CsPvpMgr::getCsPvpByServerId(int serverId) {
	map<int, CsPvp*>::iterator iter = mServerIdIndex.find(serverId);
	if (iter == mServerIdIndex.end()) {
		return NULL;
	}
	return iter->second;
}

void
CsPvpMgr::update() {
	for (map<int, CsPvp*>::iterator iter = mData.begin(); iter != mData.end(); iter++) {
		CsPvp* csPvp = iter->second;
		csPvp->update();
	}
}









