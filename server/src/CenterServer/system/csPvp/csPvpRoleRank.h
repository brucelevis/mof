#pragma once

#include <map>
#include <vector>
#include <stdint.h>
#include <string>
#include "csPvpDefine.h"
#include "basedef.h"
using namespace std;

class CsPvp;

class PvpRoleInfo
{
public:
	PvpRoleId mRoleId;
	string mRoleName;
	int mScore;
	int mRank;
	int mLvl;
	int mBat;
	int mRoleType;

	string encode();
	void decode(const char* str);
};

class PvpRoleRankList
{
public:
	static PvpRoleRankList* create(CsPvp* csPvp);

	bool init();
	void load();
	void clear();

	void traverseRankList(int startRank, int endRank, Traverser* traverser);
	PvpRoleInfo* getRoleInfoByRank(int rank);
	PvpRoleInfo* getRoleInfoByRoleId(PvpRoleId pvpRoleId);
	void updateRoleInfo(PvpRoleId pvpRoleId, const string& rolename, int roletype, int lvl, int bat);
	bool isInRank(PvpRoleId pvpRoleId);
	bool isValidRank(int rank);
	void swapRank(PvpRoleInfo* first, PvpRoleInfo* second);

	void savePvpRoleInfo(PvpRoleInfo* role);
	void savePvpRoleInfo(vector<PvpRoleInfo*>& roles);

	void collectInitData();

	map<PvpRoleId, PvpRoleInfo*> mRoleData;
	vector<PvpRoleInfo*> mRankList;
	CsPvp*  mCsPvp;
	string mDbKey;
};
