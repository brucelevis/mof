#include "worship_system.h"
#include "Paihang.h"
#include <stdlib.h>
#include <time.h>
#include "DataCfg.h"
#include "datetime.h"

WorshipSystem g_WorshipSystem;


bool
WorshipSystem::init() {
    mOpen = true;
    load();
	return true;
}

void 
WorshipSystem::load() {
    mBeWorshipState.load("worship", 0);
    mWorshipMember.load("worship:member", 0);

    mWeekTime = mBeWorshipState[0]["week_time"].asInt();
    if (mWeekTime == 0) {
        int weekTime = DateTime::getWeekDateTime();
        mBeWorshipState[0]["week_time"] = weekTime;
        mWeekTime = weekTime; 
    }

    Json::Value value = mWorshipMember[mWeekTime];
    if (value.isArray()) {
        for (int i = 0; i < value.size(); i++) {
            mCurMembers.insert(value[i].asInt());
        }
    }
}

void
WorshipSystem::update() {
    int weekTime = DateTime::getWeekDateTime();
    if (weekTime != mWeekTime) {

        mWorshipMember.clear();
        mBeWorshipState.clear();
 
        mWeekTime = weekTime;
        mBeWorshipState[0]["week_time"] = mWeekTime;
        mBeWorshipState.save();

        mCurMembers.clear();
    }
}

int
WorshipSystem::getBeWorshipTimes(int roleId) {
    return getCommonBeWorshipTimes(roleId) + getSpecialBeWorshipTimes(roleId);
}

int
WorshipSystem::getCommonBeWorshipTimes(int roleId) {
    return mBeWorshipState[roleId]["common_worship"].asInt();
}

int
WorshipSystem::getSpecialBeWorshipTimes(int roleId) {
    return mBeWorshipState[roleId]["special_worship"].asInt();
}
void
WorshipSystem::setCommonBeWorshipTimes(int roleId, int times) {
    mBeWorshipState[roleId]["common_worship"] = times;
}
void
WorshipSystem::setSpecialBeWorshipTimes(int roleId, int times) {
    mBeWorshipState[roleId]["special_worship"] = times;
}

void
WorshipSystem::addWorshipMember(int roleId) {
    if (mCurMembers.find(roleId) == mCurMembers.end()) {
        mWorshipMember[mWeekTime].append(roleId);
        mCurMembers.insert(roleId);
    }
}

vector<int>
WorshipSystem::getWorshipMembers(int weekTime) {
    vector<int> members;
    set<int>::iterator iter;
    for (iter = mCurMembers.begin(); iter != mCurMembers.end(); iter++) {
        members.push_back(*iter);
    }
    
    return members;
}

void
WorshipSystem::save() {
    mBeWorshipState.save();
    mWorshipMember.save();
}



