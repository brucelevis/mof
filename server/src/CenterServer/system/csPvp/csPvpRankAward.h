#pragma once

#include <string>
#include <map>
using namespace std;

class ServerEntity;

class CsPvpRoleRankAward
{
public:
	int mRoleId;
	int mRank;
	string mAward;
	bool mGetted;
    
    string encode();
    void decode(const char* str);
};

class CsPvpServerRankAward
{
public:
	int mRank;
    int mWeekTime;
	string mAward;
	bool mGetted;
};

class CsPvpRankAward
{
public:
	static CsPvpRankAward* create(ServerEntity* server);

	bool init();
	void load();
	void update();
	void clear();
	void addServerAward(int rank, int weekTime, const string& award);
	void addRoleAward(int roleId, int rank, const string& award);

	CsPvpServerRankAward mServerAward;
	map<int, CsPvpRoleRankAward*> mRoleAward;
	ServerEntity* mServer;
};