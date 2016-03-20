#include "daily_schedule_system.h"
#include "EnumDef.h"
#include "DataCfg.h"
#include "Game.h"
#include "datetime.h"
#include "Role.h"
#include "GameLog.h"
#include <time.h>
#include "Role.h"

DailyScheduleSystem* DailyScheduleSystem::create(Role* owner)
{
    DailyScheduleSystem* system = new DailyScheduleSystem;
    system->init(owner);
    
    return system;
}

bool DailyScheduleSystem::init(Role* owner)
{
    mOwner = owner;
    //
    load(mOwner->getInstID());
    
    time_t dataTime = getDataTime();
    tm ttm;
    localtime_r(&dataTime, &ttm);
    mWeekDay = ttm.tm_wday;
    if (mWeekDay == 0) {
        mWeekDay = 7;
    }
    
    return true;
}

void DailyScheduleSystem::checkRefresh()
{
    int nowDayStartTime = DateTime::getDateTime();
    if (nowDayStartTime > getDataTime()) {
        refresh(nowDayStartTime);
    }
}

void DailyScheduleSystem::refresh(time_t datatime)
{
    setDataTime(datatime);
    setPoint(0);
    getAwardState().clear();
    getScheduleList().clear();
    save();
    
    tm ttm;
    localtime_r(&datatime, &ttm);
    mWeekDay = ttm.tm_wday;
    if (mWeekDay == 0) {
        mWeekDay = 7;
    }
}

void DailyScheduleSystem::onEvent(int type, int target, int progress)
{
    checkRefresh();
    
    DailySchedule* schedule = getScheduleList().check(type, target, progress);
    if (schedule == NULL) {
        return;
    }
    
    DailyScheduleCfgDef* cfgDef = DailyScheduleCfg::getCfgByType(schedule->mType);
    if (cfgDef == NULL) {
        return;
    }
    
    Xylog updateLog(eLogName_DailyScheduleUpdate, mOwner->getInstID());
    updateLog << cfgDef->name << target << progress << schedule->mIsCompleted << schedule->mProgress;
    
    if (schedule->mIsCompleted && cfgDef != NULL) {
        int point = getPoint() + cfgDef->getPoint(mWeekDay);
        setPoint(point);
        
        notify_daily_schedule_point notify;
        notify.point = getPoint();
        notify.type = schedule->mType;
        mOwner->send(&notify);
    
        Xylog log(eLogName_DailyScheduleComplete, mOwner->getInstID());
        log << cfgDef->name << point;
    }
    
    save();
}
