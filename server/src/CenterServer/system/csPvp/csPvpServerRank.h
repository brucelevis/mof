#pragma once

#include <map>
#include <vector>
#include <string>
#include <set>
#include "csPvpDefine.h"
using namespace std;

class CsPvp;

class PvpServerInfo
{
public:
	int mServerId;
	int mScore;
	int mRank;
};

class PvpServerRankList
{
public:
	static PvpServerRankList* create(CsPvp* csPvp);

	void load();
	void addScore(int serverId, int score);
	void clear();

	PvpServerInfo* getServerInfoByRank(int rank);

	vector<PvpServerInfo*> mRankList;
	map<int, PvpServerInfo*> mServerData;

	CsPvp* mCsPvp;
};
