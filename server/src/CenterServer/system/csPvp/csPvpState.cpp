#include "csPvpState.h"
#include "dbMgr.h"
#include "csPvp.h"

PvpStateMgr*
PvpStateMgr::create(CsPvp* csPvp) {
	PvpStateMgr* stateMgr = new PvpStateMgr;
	stateMgr->mCsPvp = csPvp;
	return stateMgr;
}

PvpRoleId 
PvpStateMgr::getBeChallengedRole(PvpRoleId roleId) {
	map<PvpRoleId, PvpRoleId>::iterator iter = mChallengeList.find(roleId);
	if (iter == mChallengeList.end()) {
		return 0;
	}
	return iter->second;
}

bool 
PvpStateMgr::isChallenging(PvpRoleId roleId) {
	return mChallengeList.find(roleId) != mChallengeList.end();
}

bool 
PvpStateMgr::isBeChallenged(PvpRoleId roleId) {
	return mBeChallengedList.find(roleId) != mBeChallengedList.end();
}

void 
PvpStateMgr::startPvp(PvpRoleId roleId, PvpRoleId targetId) {
	if (isChallenging(roleId) || isBeChallenged(targetId)) {
		return;
	}

	mChallengeList[roleId] = targetId;
	mBeChallengedList[targetId] = roleId;
}

void 
PvpStateMgr::endPvp(PvpRoleId roleId) {
	map<PvpRoleId, PvpRoleId>::iterator iter = mChallengeList.find(roleId);
	mBeChallengedList.erase(iter->second);
	mChallengeList.erase(iter);
}

void
PvpStateMgr::endPvpByServerId(int serverId) {
    for (map<PvpRoleId, PvpRoleId>::iterator iter = mChallengeList.begin();
         iter != mChallengeList.end(); ){
        PvpRoleId pvpRoleId = iter->first;
        if (getPvpRoleServerId(pvpRoleId) == serverId) {
            mChallengeList.erase(iter++);
        } else {
            iter++;
        }
    }
    
}

void
PvpStateMgr::cancelPvp(PvpRoleId roleId) {
	map<PvpRoleId, PvpRoleId>::iterator iter = mChallengeList.find(roleId);
	mBeChallengedList.erase(iter->second);
	mChallengeList.erase(iter);
}

void
PvpStateMgr::clear() {
	mBeChallengedList.clear();
	mChallengeList.clear();
}