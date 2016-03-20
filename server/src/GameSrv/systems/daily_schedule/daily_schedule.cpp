//
//  daily_schedule_item.cpp
//  GameSrv
//
//  Created by 丁志坚 on 11/17/14.
//
//

#include "daily_schedule.h"
#include "EnumDef.h"
#include "json/json.h"
#include "DataCfg.h"
#include "Game.h"

bool DailySchedule::init(DailyScheduleCfgDef* cfgDef)
{
    mType = cfgDef->type;
    mTargetProgress = cfgDef->progress;
    mTarget = cfgDef->target;
    return true;
}

void DailySchedule::clear()
{
    mProgress = 0;
    mIsCompleted = false;
}

bool DailySchedule::trigger(int target, int progress)
{
    if (mTarget != 0 && mTarget != target) {
        return false;
    }
    
    return true;
}

bool DailySchedule::check(int target, int progress)
{
    if (mIsCompleted) {
        return false;
    }
    
    if (!trigger(target, progress)) {
        return false;
    }
    
    update(target, progress);
    return true;
}

class AccValueDailySchedule : public DailySchedule
{
public:
    virtual void update(int target, int progress)
    {
        mProgress += progress;
        if (mProgress >= mTargetProgress) {
            mProgress = mTargetProgress;
            mIsCompleted = true;
        }
    }
};

class UpLimitDailySchedule : public DailySchedule
{
public:
    virtual void update(int target, int progress)
    {
        mProgress = mTargetProgress;
        mIsCompleted = true;
    }
};

class DownLimitDailySchedule : public DailySchedule
{
public:
    virtual void update(int target, int progress)
    {
        mProgress = mTargetProgress;
        mIsCompleted = true;
    }
};

DailySchedule* DailySchedule::create(DailyScheduleCfgDef* cfgDef)
{
    Json::Reader reader;
    Json::Value value;
    
    DailySchedule* schedule = NULL;
    
    int scheduleTye = cfgDef->type;
    switch (scheduleTye) {
        case kDailyScheduleDungeon:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleGuildDonate:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleAnswer:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleMysteriousShop:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleDailyQuest:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleTeamCopy:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleMysteriousCopy:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleEliteCopy:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleWorldBoss:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleCapsuleToy:
            schedule = new AccValueDailySchedule();
            break;
        case kDailySchedulePetMatch:
            schedule = new AccValueDailySchedule();
            break;
        case kDailySchedulePetAdventure:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleDefendStatue:
            schedule = new AccValueDailySchedule();
            break;
        case kDailySchedulePrintCopy:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleFriendDungeon:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleGuildChallenge:
            schedule = new AccValueDailySchedule();
            break;
        case kDailySchedulePvp:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleSyncTeamCopy:
            schedule = new AccValueDailySchedule();
            break;
        case kDailySchedulePetElite:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleFamousHall:
            schedule = new UpLimitDailySchedule();
            break;
        case kDailyScheduleGuildBoss:
            schedule = new AccValueDailySchedule();
            break;
        case kDailySchedulePetArena:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleSyncTreasureFight:
            schedule = new AccValueDailySchedule();
            break;
        case kDailyScheduleSyncPvp:
            schedule = new AccValueDailySchedule();
            break;
        default:
            break;
    }
    
    if (schedule == NULL) {
        return NULL;
    }
    
    schedule->init(cfgDef);
    
    return schedule;
}
