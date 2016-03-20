#pragma once

#include <map>
#include <vector>
#include <string>
#include "csPvpDefine.h"
#include "basedef.h"
using namespace std;

class CsPvp;

struct PvpLog
{
	PvpRoleId mRoleId;
	string mRoleName;
	int mRoleRank;
	PvpRoleId mTargetId;
	string mTargetName;
	int mTargetRank;
	bool mSwapRank;
	bool mIsWin;
	int mTime;

	string encode();
	void decode(const char* str);
};

class PvpLogMgr
{
public:
	static PvpLogMgr* create(CsPvp* csPvp);

	void load();
	void addLog2Memory(PvpLog* log);
	void addLog(PvpRoleId roleId, string& roleName, int rank, 
		PvpRoleId targetId, string& targetName, int targetRank, 
		bool isWin, bool swapRank);
	void clear();

	void roleRefLog(PvpRoleId roleId, PvpLog* log);
	void traverseRoleLog(PvpRoleId roleId, Traverser* traverser);

	CsPvp* mCsPvp;

	vector<PvpLog*> mLogs;
	map<PvpRoleId, vector<PvpLog*> > mRoleLogs;
};
