#include "csPvpRoleRank.h"
#include "dbMgr.h"
#include "csPvp.h"
#include "json/json.h"



string 
PvpRoleInfo::encode() {
	Json::Value value;
	value["lvl"] = mLvl;
	value["bat"] = mBat;
	value["rank"] = mRank;
	value["roletype"] = mRoleType;
	value["score"] = mScore;
	value["rolename"] = mRoleName;
	return Json::FastWriter().write(value);
}

void 
PvpRoleInfo::decode(const char* str) {
	Json::Value value;
	Json::Reader reader;
	reader.parse(str, value);
	if (!value.isObject()) {
		return;
	}
	mRank = value["rank"].asInt();
	mScore = value["score"].asInt();
	mRoleType = value["roletype"].asInt();
	mRoleName = value["rolename"].asString();
	mLvl = value["lvl"].asInt();
	mBat = value["bat"].asInt();
}


PvpRoleRankList* 
PvpRoleRankList::create(CsPvp* csPvp) {
	PvpRoleRankList* rank = new PvpRoleRankList;
	rank->mCsPvp = csPvp;
	rank->init();
	return rank;
}


bool
PvpRoleRankList::init() {
	mDbKey = strFormat("cspvp:%d:rolerank", mCsPvp->getGroupId());
	return true;
}

void
PvpRoleRankList::load() {
	int groupId = mCsPvp->getGroupId();
	RedisResult result(redisCmd("hgetall %s", mDbKey.c_str()));
	int count = result.getHashElements();
	mRankList.resize(count / 2, NULL);

	vector<PvpRoleInfo*> invalidInfo;
	for (int i = 0; i < count; i += 2) {
		string roleIdStr = result.readHash(i);
		string roleInfoStr = result.readHash(i + 1);
		PvpRoleId pvpRoleId = atoll(roleIdStr.c_str());

		PvpRoleInfo* roleInfo = new PvpRoleInfo;
		roleInfo->mRoleId = pvpRoleId;
		roleInfo->decode(roleInfoStr.c_str());

		int rank = roleInfo->mRank;
		if (mRankList.size() < rank) {
			invalidInfo.push_back(roleInfo);
			continue;
		} 

		if (mRankList[rank - 1] != NULL) {
			invalidInfo.push_back(roleInfo);
			continue;
		}

		mRankList[rank - 1] = roleInfo;
		mRoleData[pvpRoleId] = roleInfo;
	}

	if (invalidInfo.empty()) {
		return;
	}

	//容错处理
	for (int i = 0, j = 0; i < mRankList.size(); i++) {
		if (mRankList[i] == NULL) {
			PvpRoleInfo* pvpRoleInfo = invalidInfo[j++];
			pvpRoleInfo->mRank = i + 1;
			mRankList[i] = pvpRoleInfo;
		}
	}
	savePvpRoleInfo(invalidInfo);
}


void 
PvpRoleRankList::traverseRankList(int startRank, int endRank, Traverser* traverser) {
	
	if (startRank < 0) {
		startRank = mRankList.size() + startRank;
	} else if (startRank >= mRankList.size()) {
		startRank = mRankList.size() - 1;
	}

	if (endRank < 0) {
		endRank = mRankList.size() + endRank;
	} else if (endRank >= mRankList.size()) {
		endRank = mRankList.size() - 1;
	}

	for (int i = startRank; i <= endRank; i++) {
		PvpRoleInfo* roleInfo = mRankList[i];
		traverser->callback(roleInfo);
	}
}


void
PvpRoleRankList::clear() {
	for (map<PvpRoleId, PvpRoleInfo*>::iterator iter = mRoleData.begin(); iter != mRoleData.end(); iter++) {
		PvpRoleInfo* info = iter->second;
		delete info;
	}
	mRoleData.clear();
	mRankList.clear();
	doRedisCmd("del cspvp:%d:rolerank", mCsPvp->getGroupId());
}

void
PvpRoleRankList::collectInitData() {

}

void
PvpRoleRankList::savePvpRoleInfo(vector<PvpRoleInfo*>& roles) {
	string updateCmd = "hmset ";
	updateCmd.append(mDbKey);
	for (int i = 0; i < roles.size(); i++) {
		PvpRoleInfo* pvpRoleInfo = roles[i];			
		string roleInfo = pvpRoleInfo->encode();
		if (roleInfo.empty()) {
			roleInfo = "\"\"";
		}
		updateCmd.append(strFormat(" %lld %s", pvpRoleInfo->mRoleId, roleInfo.c_str()));
	}
	doRedisCmd(updateCmd.c_str());
}


void 
PvpRoleRankList::savePvpRoleInfo(PvpRoleInfo* role) {
	doRedisCmd("hset %s %lld %s", mDbKey.c_str(), role->mRoleId, role->encode().c_str());
}

void
PvpRoleRankList::swapRank(PvpRoleInfo* first, PvpRoleInfo* second) {
	int firstRank = first->mRank;
	int secondRank = second->mRank;
	first->mRank = secondRank;
	second->mRank = firstRank;
	mRankList[first->mRank - 1] = first;
	mRankList[second->mRank - 1] = second;
}

PvpRoleInfo*
PvpRoleRankList::getRoleInfoByRank(int rank) {
	if (!isValidRank(rank)) {
		return 0;
	}

	return mRankList[rank - 1];
}

PvpRoleInfo*
PvpRoleRankList::getRoleInfoByRoleId(PvpRoleId pvpRoleId) {
	map<PvpRoleId, PvpRoleInfo*>::iterator iter = mRoleData.find(pvpRoleId);
	if (iter == mRoleData.end()) {
		return NULL;
	}

	return iter->second;
}

bool
PvpRoleRankList::isInRank(PvpRoleId pvpRoleId) {
	map<PvpRoleId, PvpRoleInfo*>::iterator iter = mRoleData.find(pvpRoleId);
	return (iter != mRoleData.end());
}
 
void
PvpRoleRankList::updateRoleInfo(PvpRoleId pvpRoleId, const string& rolename, int roletype, int lvl, int bat) {
	map<PvpRoleId, PvpRoleInfo*>::iterator iter = mRoleData.find(pvpRoleId);
	PvpRoleInfo* roleInfo = NULL;
	if (iter != mRoleData.end()) {
		roleInfo = iter->second;
	} else {
		roleInfo = new PvpRoleInfo;	
		mRankList.push_back(roleInfo);
		int rank = mRankList.size();
		mRoleData[pvpRoleId] = roleInfo;
		roleInfo->mRank = rank;
		roleInfo->mScore = 0;
		roleInfo->mRoleId = pvpRoleId;
	}

	roleInfo->mLvl = lvl;
	roleInfo->mBat = bat;
	roleInfo->mRoleName = rolename;
	roleInfo->mRoleType = roletype;

	string roleInfoStr = roleInfo->encode();
	doRedisCmd("hset %s %lld %s", mDbKey.c_str(), pvpRoleId, roleInfoStr.c_str());
}

bool
PvpRoleRankList::isValidRank(int rank) {
	return mRankList.size() >= rank & rank > 0;
}

