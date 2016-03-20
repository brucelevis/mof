#pragma once

#include <map>
#include <vector>
#include <string>
#include "csPvpDefine.h"
using namespace std;

class CsPvp;

class PvpStateMgr
{
public:
	static PvpStateMgr* create(CsPvp* csPvp);

	map<PvpRoleId, PvpRoleId> mChallengeList;
	map<PvpRoleId, PvpRoleId> mBeChallengedList;

	PvpRoleId getBeChallengedRole(PvpRoleId roleId);

	bool isChallenging(PvpRoleId roleId);
	bool isBeChallenged(PvpRoleId roleId);

	void startPvp(PvpRoleId roleId, PvpRoleId targetId);
	void endPvp(PvpRoleId roleId);
    void endPvpByServerId(int serverId);
	void cancelPvp(PvpRoleId roleId);
	void clear();
	CsPvp* mCsPvp;
};
