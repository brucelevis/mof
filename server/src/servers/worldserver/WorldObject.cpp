//
//  WorldObject.cpp
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#include "WorldObject.h"
#include "WorldScene.h"
#include "WorldScheduler.h"

#include "log.h"
#include "main.h"
#include "msg.h"

WorldObjectMgr g_WorldObjectMgr;

WorldObject::WorldObject(WorldObjectType type) : mType(type)
{
    mBeat = 1;
    mRunTime = 0;
    mScheduler = NULL;
    mValid = true;
    
    mInstId = g_WorldObjectMgr.genId(type);
    g_WorldObjectMgr.addObject(this);
}


WorldObject::~WorldObject()
{
    g_WorldObjectMgr.removeObject(mInstId);
    if (mScheduler)
    {
        delete mScheduler;
    }
}

void WorldObject::update(uint64_t ms)
{
    mRunTime += ms;

    if (mScheduler)
    {
        mScheduler->update(ms);
    }
}


void WorldObject::schedule(SEL_SCHEDULE selector, int64_t interval, unsigned int repeat, int64_t delay)
{
    if (mScheduler == NULL)
    {
        mScheduler = new WorldScheduler;
    }
    getScheduler()->scheduleSelector(selector, this, interval, repeat, delay);
}

void WorldObject::schedule(SEL_SCHEDULE selector, int64_t interval)
{
    if (mScheduler == NULL)
    {
        mScheduler = new WorldScheduler;
    }
    getScheduler()->scheduleSelector(selector, this, interval);
}

void WorldObject::scheduleOnce(SEL_SCHEDULE selector, int64_t delay)
{
    if (mScheduler == NULL)
    {
        mScheduler = new WorldScheduler;
    }
    getScheduler()->scheduleSelector(selector, this, 0, 1, delay);
}

void WorldObject::schedule(SEL_SCHEDULE selector)
{
    if (mScheduler == NULL)
    {
        mScheduler = new WorldScheduler;
    }
    getScheduler()->scheduleSelector(selector, this, 0);
}

void WorldObject::unschedule(SEL_SCHEDULE selector)
{
    WorldScheduler *sch = getScheduler();
    if (sch) {
        sch->unscheduleSelector(selector, this);
    }
}

void WorldObject::unscheduleAll()
{
    WorldScheduler *sch = getScheduler();
    if (sch) {
        sch->unscheduleAll();
    }
}

bool WorldObject::isCreature() const
{
    return mType == eWorldObjectRole || mType == eWorldObjectPet || mType == eWorldObjectRetinue ||
        mType == eWorldObjectNPC || mType == eWorldObjectMonster;
}





WorldObjectMgr::WorldObjectMgr()
{
    for (int i = 0; i < eWorldObjectTypeMax; i++) {
        mObjects[i] = NULL;
    }
}

WorldObjectMgr::~WorldObjectMgr()
{
    for (int i = 0; i < eWorldObjectTypeMax; i++) {
        if (mObjects[i] != NULL) {
            delete[] mObjects[i];
            mObjects[i] = NULL;
        }
    }
}

bool WorldObjectMgr::init(int type, int maxNum)
{
    if (type < 1 || type >= eWorldObjectTypeMax)
    {
        return false;
    }
    
    mHigh[type] = (type << 24);
    mMaxLow[type] = (1 << 24) - 1;
    mMaxNum[type] = maxNum;
    if (maxNum > mMaxLow[type] + 1)
    {
        return false;
    }
    
    mCapacity[type] = 1;
    while (mCapacity[type] < maxNum)
    {
        mCapacity[type] <<= 1;
    }
    
    mCurIdx[type] = 0;
    mCurNum[type] = 0;
    mObjects[type] = new WorldObject*[mCapacity[type]];
    memset(mObjects[type], 0, sizeof(WorldObject*) * mCapacity[type]);
    
    return true;
}

bool WorldObjectMgr::addObject(WorldObject* obj)
{
    int instId = obj->getInstId();
    int type = instId >> 24;
    int idx = instId & (mCapacity[type] - 1);
    
    if (mObjects[type][idx] == NULL)
    {
        mObjects[type][idx] = obj;
        mCurNum[type]++;
    }
    return true;
}

WorldObject* WorldObjectMgr::findObject(int instId)
{
    int type = instId >> 24;
    int idx = instId & (mCapacity[type] - 1);
    
    if (mObjects[type] == NULL) {
        return NULL;
    }
    
    WorldObject* obj = mObjects[type][idx];
    if (obj && instId == obj->getInstId()) {
        return obj;
    }
    
    return NULL;
}

void WorldObjectMgr::removeObject(int instId)
{
    int type = instId >> 24;
    int idx = instId & (mCapacity[type] - 1);
    
    WorldObject* obj = mObjects[type][idx];
    if (obj && obj->getInstId() == instId)
    {
        mObjects[type][idx] = NULL;
        mCurNum[type]--;
    }
}


int WorldObjectMgr::genId(int type)
{
    //
    assert(mCurNum[type] < mMaxNum[type]);
    
    int idx;
    for (;;)
    {
        mCurIdx[type] &= mMaxLow[type];
        idx = mCurIdx[type]++;
        
        int slot = idx & (mCapacity[type] - 1);
        if (mObjects[type][slot] == NULL)
        {
            break;
        }
    }
    
    int instId = mHigh[type] + idx;
    
    return instId;
}

int WorldObjectMgr::getObjectNum(int type)
{
    return mCurNum[type];
}
