//
//  WorldScheduler.h
//  GameSrv
//
//  Created by prcv on 14-3-17.
//
//

#ifndef __GameSrv__WorldScheduler__
#define __GameSrv__WorldScheduler__

#include <iostream>
#include <stdint.h>

#include "WorldObject.h"
class WorldObject;


class WorldTimer// : public WorldObject
{
public:
    WorldTimer(void);
    bool initWithTarget(WorldObject *target, SEL_SCHEDULE selector);
    bool initWithTarget(WorldObject *target, SEL_SCHEDULE selector, int64_t interval, uint64_t repeat, int64_t delay);
    virtual void update(uint64_t ms);
public:
    static WorldTimer* timerWithTarget(WorldObject *target, SEL_SCHEDULE selector);
    static WorldTimer* timerWithTarget(WorldObject *target, SEL_SCHEDULE selector, int64_t interval);
    
protected:
    READWRITE(WorldObject*, mTarget, Target)
    READWRITE(SEL_SCHEDULE, mSelector, Selector)
    
    READWRITE(uint64_t, mCurMs, CurMs)
    READWRITE(uint64_t, mNextMs, NextMs)
    READWRITE(uint64_t, mLastUpdateMs, LastUpdateMs)
    
    READWRITE(bool, mRunForever, RunForever)
    READWRITE(uint64_t, mTimesExecuted, TimesExecuted)
    READWRITE(uint64_t, mRepeat, Repeat)
    READWRITE(uint64_t, mDelay, Delay)
    READWRITE(int64_t, mInterval, Interval)
    
    READWRITE(bool, mValid, Valid)
};



class WorldScheduler
{
public:
    WorldScheduler();
    ~WorldScheduler(void);

    void update(uint64_t ms);
    
    void scheduleSelector(SEL_SCHEDULE selector, WorldObject *target, int64_t interval, uint64_t repeat, int64_t delay);
    void scheduleSelector(SEL_SCHEDULE selector, WorldObject *target, int64_t interval);
    void unscheduleSelector(SEL_SCHEDULE selector, WorldObject *target);
    void unscheduleAll();
    
    list<WorldTimer*> mTimers;
};

#endif /* defined(__GameSrv__WorldScheduler__) */
