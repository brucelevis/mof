//
//  WorldScheduler.cpp
//  GameSrv
//
//  Created by prcv on 14-3-17.
//
//

#include "WorldScheduler.h"
#include <stdint.h>

#define kCCRepeatForever (-1)

// implementation CCTimer

WorldTimer::WorldTimer() : //WorldObject(eWorldObjectTimer),
    mSelector(NULL),
    mInterval(0),
    mTarget(NULL),
    mCurMs(0),
    mNextMs(0),
    mLastUpdateMs(0),
    mRunForever(false),
    mTimesExecuted(0),
    mRepeat(0),
    mDelay(0.0f)
{
    mValid = false;
}

WorldTimer* WorldTimer::timerWithTarget(WorldObject *pTarget, SEL_SCHEDULE pfnSelector)
{
    WorldTimer *pTimer = new WorldTimer();

    pTimer->initWithTarget(pTarget, pfnSelector, 0.0f, kCCRepeatForever, 0.0f);

    return pTimer;
}

WorldTimer* WorldTimer::timerWithTarget(WorldObject *pTarget, SEL_SCHEDULE pfnSelector, int64_t fSeconds)
{
    WorldTimer *pTimer = new WorldTimer();

    pTimer->initWithTarget(pTarget, pfnSelector, fSeconds, kCCRepeatForever, 0);

    return pTimer;
}


bool WorldTimer::initWithTarget(WorldObject *pTarget, SEL_SCHEDULE pfnSelector)
{
    return initWithTarget(pTarget, pfnSelector, 0, kCCRepeatForever, 0);
}

bool WorldTimer::initWithTarget(WorldObject *pTarget, SEL_SCHEDULE pfnSelector, int64_t ms, uint64_t repeat, int64_t delay)
{
    mTarget = pTarget;
    mSelector = pfnSelector;

    mInterval = ms;
    mDelay = delay;
    mRepeat = repeat;
    mRunForever = (repeat == kCCRepeatForever) ? true : false;

    mCurMs = 0;
    if (delay > 0)
    {
        mNextMs = delay;
    }
    else
    {
        mNextMs = mInterval;
    }
    mLastUpdateMs = 0;
    
    mValid = true;

    return true;
}

void WorldTimer::update(uint64_t ms)
{
    mCurMs += ms;
    CheckCondition(mCurMs >= mNextMs, return)

    (mTarget->*mSelector)(mCurMs - mLastUpdateMs);
    mLastUpdateMs = mCurMs;
    
    if (mInterval == 0) {
        mNextMs = mCurMs;
    } else  {
        uint64_t dt = mCurMs - mNextMs;
        mNextMs += (dt - (dt % mInterval)) + mInterval;
    }
    
    ++mTimesExecuted;
    if (!mRunForever) {
        setValid(mTimesExecuted < mRepeat);
    }
}

WorldScheduler::WorldScheduler()
{

}

WorldScheduler::~WorldScheduler()
{
    list<WorldTimer*>::iterator iter;
    for (iter = mTimers.begin(); iter != mTimers.end();)
    {
        WorldTimer* timer = *iter;
        delete timer;
        iter = mTimers.erase(iter);
    }
}

void WorldScheduler::update(uint64_t ms)
{
    list<WorldTimer*>::iterator iter;
    for (iter = mTimers.begin(); iter != mTimers.end();)
    {
        WorldTimer* timer = *iter;
        if (timer->getValid())
        {
            timer->update(ms);
            ++iter;
        }
        else
        {
            delete timer;
            iter = mTimers.erase(iter);
        }
    }

}

void WorldScheduler::scheduleSelector(SEL_SCHEDULE selector, WorldObject *target, int64_t interval, uint64_t repeat, int64_t delay)
{
    WorldTimer* timer = new WorldTimer;
    timer->initWithTarget(target, selector, interval, repeat, delay);
    mTimers.push_back(timer);
}

void WorldScheduler::scheduleSelector(SEL_SCHEDULE selector, WorldObject *target, int64_t interval)
{
    WorldTimer* timer =  WorldTimer::timerWithTarget(target, selector, interval);
    mTimers.push_back(timer);
}

void WorldScheduler::unscheduleSelector(SEL_SCHEDULE selector, WorldObject *target)
{
    list<WorldTimer*>::iterator iter;
    for (iter = mTimers.begin(); iter != mTimers.end();)
    {
        WorldTimer* timer = *iter;
        if (timer->getSelector() == selector && timer->getTarget() == target)
        {
            //delete timer;
            //iter = mTimers.erase(iter);
            timer->setValid(false);
        }
        
        ++iter;
    }
}
void WorldScheduler::unscheduleAll()
{
    list<WorldTimer*>::iterator iter;
    for (iter = mTimers.begin(); iter != mTimers.end();)
    {
        WorldTimer* timer = *iter;
        timer->setValid(false);
        ++iter;
        //delete timer;
        //iter = mTimers.erase(iter);
    }
}
