#pragma once
#include "csPvpLog.h"
#include "csPvpState.h"
#include "csPvpRoleRank.h"
#include "csPvpServerRank.h"
#include "csPvpDefine.h"
#include <map>
#include <set>
using namespace std;

class CsPvp
{
public:
	static CsPvp* create(int groupId);
	enum CsPvpState{
		kPvpStateNone,
		kActivityStart,
		kPreBattle,
		kInBattle,
		kEndBattle,
		kActivityEnd,
		kActivityClose,
	};


	CsPvp() {}
	~CsPvp() {}

	bool init();
	void load();
	void update();

	//void addRole(PvpRoleId pvpRoleId);
	void swapRank(int first, int second);

	void checkStartActivity();

	void startActivity();
	void preBattle();
	void startBattle();
	void endBattle();
	void endActivity();
	void closeActivity();

	void genRoleRankList();
	void sendAward();

	vector<PvpRoleInfo*> getChallengeData(int rank);
	vector<PvpRoleInfo*> getChallengeDataForth(int rank, int num);
	vector<PvpRoleInfo*> getChallengeDataBack(int rank, int num);


	PvpRoleRankList* getPvpRoleRankList();
	PvpServerRankList* getPvpServerRankList();
	PvpStateMgr* getPvpStateMgr();
	PvpLogMgr* getPvpLogMgr();

	int getState() {return mState;}
	int getGroupId() {return mGroupId;}
private:
	int mState;
	int mNextStateTime;
	int mGroupId;
	int mStartTime;

	PvpRoleRankList* mPvpRoleRankList;
	PvpServerRankList* mPvpServerRankList; 
	PvpStateMgr* mPvpStateMgr;
	PvpLogMgr* mPvpLogMgr;
};

class CsPvpMgr
{
public:
	bool init();
	void load();
	CsPvp* getCsPvp(int groupId);
	CsPvp* getCsPvpByServerId(int serverId);

	void update();

private:
	map<int, CsPvp*> mData;
	map<int, CsPvp*> mServerIdIndex;
	map<int, set<int> > mGroupInfo;
};

extern CsPvpMgr* getCsPvpMgr();
