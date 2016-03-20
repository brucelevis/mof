#include "csPvpServerRank.h"
#include "dbMgr.h"
#include "csPvp.h"

PvpServerRankList*
PvpServerRankList::create(CsPvp* csPvp) {
	PvpServerRankList* rank = new PvpServerRankList;
	rank->mCsPvp = csPvp;
	return rank;
}

void
PvpServerRankList::load() {
	int groupId = mCsPvp->getGroupId();
	RedisResult result(redisCmd("hgetall cspvp:%d:serverrank", groupId));
	int count = result.getHashElements();
	for (int i = 0; i < count; i += 2) {
		int serverId = result.readHash(i, 0);
		int score = result.readHash(i + 1, 0);
		addScore(serverId, score);
	}
}

void
PvpServerRankList::clear() {
    for (int i = 0; i < mRankList.size(); i++) {
        delete mRankList[i];
    }
    mRankList.clear();
    mServerData.clear();
	doRedisCmd("del cspvp:%d:serverrank", mCsPvp->getGroupId());
}

void 
PvpServerRankList::addScore(int serverId, int score) {
	map<int, PvpServerInfo*>::iterator iter = mServerData.find(serverId);
	PvpServerInfo* serverInfo = NULL;
	if (iter == mServerData.end()) {
		serverInfo = new PvpServerInfo;
		serverInfo->mServerId = serverId;
		mRankList.push_back(serverInfo);
		serverInfo->mRank = mRankList.size();
		serverInfo->mScore = 0;
		mServerData[serverId] = serverInfo;
	} else {
		serverInfo = iter->second;
	}
	serverInfo->mScore += score;

	int rank = serverInfo->mRank;
	while (rank > 1) {
		PvpServerInfo* prevServer = mRankList[rank - 2];
		if (serverInfo->mScore <= prevServer->mScore) {
			break;
		}

		mRankList[rank - 1] = prevServer;
        prevServer->mRank = rank;
		rank--;
	}
	serverInfo->mRank = rank;
	mRankList[rank - 1] = serverInfo;

	doRedisCmd("hset cspvp:%d:serverrank %d %d", mCsPvp->getGroupId(), serverId, serverInfo->mScore);
}

PvpServerInfo*
PvpServerRankList::getServerInfoByRank(int rank) {
	if (rank > mRankList.size() || rank <= 0) {
		return NULL;
	}

	return mRankList[rank - 1];
}



