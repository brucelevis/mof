#pragma once

#include <map>
#include <set>
#include "RedisHashJson.h"
using namespace std;

struct BeWorshipMember
{
public:
	int mRoleId;
	string mRoleName;
	int mLvl;
	int mJob;
	int mBattle;
};

class WorshipSystem
{
public:
	bool init();
	void load();
    void save();
    
    bool isOpen();
    void update();
    
    int getBeWorshipTimes(int roleId);
    int getCommonBeWorshipTimes(int roleId);
    int getSpecialBeWorshipTimes(int roleId);
    void setCommonBeWorshipTimes(int roleId, int times);
    void setSpecialBeWorshipTimes(int roleId, int times);

    void addWorshipMember(int roleId);
    vector<int> getWorshipMembers(int weektime);

private:
    int mWeekTime;

    set<int> mCurMembers;
    RedisHashJson<int> mWorshipMember;
	RedisHashJson<int> mBeWorshipState;
    bool mOpen;
};

extern WorshipSystem g_WorshipSystem;