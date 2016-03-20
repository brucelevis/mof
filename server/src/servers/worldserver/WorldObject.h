//
//  WorldObject.h
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#ifndef __GameSrv__WorldObject__
#define __GameSrv__WorldObject__

#include <iostream>
#include <list>
#include <sys/time.h>

#include "WorldBase.h"
#include "WorldGeometry.h"

#include "process.h"
#include "reference.h"
#include "basedef.h"
#include "timestamp.h"

#include "Defines.h"

using namespace std;

class WorldScene;
class WorldObject;
class WorldScheduler;


enum WorldObjectType
{
    eWorldObjectScene = 1,
    eWorldObjectRole,
    eWorldObjectPet,
    eWorldObjectNPC,
    eWorldObjectMonster = 5,
    eWorldObjectRobot,
    eWorldObjectSkillAction,
    eWorldObjectSkillFly,
    eWorldObjectItem,
    eWorldObjectRetinue,
    eWorldObjectTypeMax
};


typedef void (WorldObject::*SEL_SCHEDULE)(uint64_t dt);
class WorldObject : public Properties, public RefCountedObject
{
public:
    
    READWRITE(int, mInstId, InstId)
    READWRITE(int, mModId, ModId)
    READWRITE(WorldScheduler*, mScheduler, Scheduler)
    READWRITE(WorldObjectType, mType, Type)
    READWRITE(bool, mValid, Valid)
    
public:
    WorldObject(WorldObjectType type);
    virtual ~WorldObject();
    
    virtual void update(uint64_t ms);
    
    void schedule(SEL_SCHEDULE selector, int64_t interval, unsigned int repeat, int64_t delay);
    void schedule(SEL_SCHEDULE selector, int64_t interval);
    void scheduleOnce(SEL_SCHEDULE selector, int64_t delay);
    void schedule(SEL_SCHEDULE selector);
    void unschedule(SEL_SCHEDULE selector);
    void unscheduleAll();
    
    uint64_t mRunTime;
    int   mBeat;
    
    bool isCreature() const;
    
};

class WorldObjectMgr
{
public:
    bool init(int type, int maxNum);
    bool addObject(WorldObject* obj);
    WorldObject* findObject(int instId);
    void removeObject(int instId);
    int genId(int type);
    int getObjectNum(int type);
    
public:
    WorldObjectMgr();
    ~WorldObjectMgr();
private:
    WorldObject** mObjects[eWorldObjectTypeMax];
    int mMaxNum[eWorldObjectTypeMax];
    int mCapacity[eWorldObjectTypeMax];
    int mCurIdx[eWorldObjectTypeMax];
    int mCurNum[eWorldObjectTypeMax];
    int mHigh[eWorldObjectTypeMax];
    int mMaxLow[eWorldObjectTypeMax];
};

extern WorldObjectMgr g_WorldObjectMgr;

#define schedule_selector(_SELECTOR) (SEL_SCHEDULE)(&_SELECTOR)

#endif /* defined(__GameSrv__WorldObject__) */
