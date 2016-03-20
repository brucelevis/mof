//
//  WorldScene.h
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#ifndef __GameSrv__WorldScene__
#define __GameSrv__WorldScene__

#include <iostream>
#include <map>
#include <list>
#include "WorldObject.h"
#include "Defines.h"
#include "Utils.h"
#include "INetPacket.h"
#include "basedef.h"
#include "WorldGeometry.h"
#include "DataCfg.h"

class WorldRole;
class WorldNPC;
class WorldCreature;
class WorldPet;
class WorldPlayer;
class SceneObject;
class WorldMonster;

using namespace std;


enum WorldSceneState
{
    kWorldSceneNone = 0,
    kWorldSceneInitialized,
    kWorldSceneRunning,
    kWorldSceneEnd,
};

struct WorldSceneEvent
{
    int eventId;
    long lParam;
    long rParam;
};

class SceneArea
{
    friend class WorldScene;
protected:
    vector<CCPoint> mLandPoses;
    CCRect mBoundingBox;
    
    SceneAreaDef mAreaDef;
public:
    static SceneArea* create(const SceneAreaDef& def);
    
    const CCRect& getBoundingBox()
    {
        return mBoundingBox;
    }
    
    void setBoundingBox(const CCRect& boundingBox)
    {
        mBoundingBox = boundingBox;
    }
    
    vector<CCPoint>& getLandPoses();
    CCPoint getLandPos(int index);
};

class WorldScene : public WorldObject
{
protected:
    SceneCfgDef mCfg;
    vector<SceneArea*> mAreas;
    
    int    mBeat;
    string mExtendProp;
    
    bool   mInitialized;
    bool   mWaitingEnd;
    int    mWaitEndTime;
    int    mStateTime;
    
    list<WorldPlayer*> mUnreadyPlayers;
    list<WorldPlayer*> mPlayers;
    list<WorldPlayer*> mLeavedPlayers;
    
    list<WorldObject*> mObjects;
    list<WorldObject*> mInvalidObjects;
    
public:
    
    READWRITE(int, mActiveArea, ActiveArea)
    
    const string& getExtendProp()
    {
        return mExtendProp;
    }
    
    void setExtendProp(const string& extendProp)
    {
        mExtendProp = extendProp;
    }
    
    static WorldScene* create(const SceneCfgDef* def, const char* extendProp);
    static void destroy(WorldScene* scene);
    
    WorldScene();
    virtual ~WorldScene();
    
    void clearAll();

    // modified by jianghan for 将init设置为virtual, 脚本类需要重写
    virtual bool init(const SceneCfgDef* def, const char* extendProp);
    
    void run(uint64_t ms);
    void end();
    void deinit();
    
    
    SceneArea* getArea(int areaIdx)
    {
        return mAreas[areaIdx];
    }
    
    WorldCreature* getWorldCreature(int instId);
    
    CCRect getBoundingBox(int areaIdx)
    {
        return mAreas[areaIdx]->getBoundingBox();
    }
    
    CCPoint getRandPos(int areaIdx)
    {
        SceneArea* area = getArea(areaIdx);
        return area->getLandPos(areaIdx);
    }
    
    // added by jianghan
    SceneCfgDef& getConfig()
    {
        return mCfg;
    }
    // end add
    
    void pendEnd();
    
    void activeArea(int areaIdx);
    virtual void update(uint64_t ms);
    
    bool addPlayer(WorldPlayer* player);
    void removePlayer(WorldPlayer* player);
    
    virtual bool preAddPlayer(WorldPlayer* player) {return true;}
    virtual void onAddedPlayer(WorldPlayer* player) {}
    virtual void beforePlayerLeave(WorldPlayer* player) {}
    virtual void afterPlayerLeave() {}
    virtual void onPlayerReady(WorldPlayer* player) {}
    
    virtual void onCreatureDead(WorldCreature* victim, WorldCreature* killer){}
    
    void addMonster(WorldMonster* monster);
    virtual void onAddMonster(WorldMonster* monster);
    
    void traverseObjects(TraverseCallback callback, void* param);
    void broadcastPacket(INetPacket* packet, bool shouldReady = true);
    
    // added by jianghan
    void * mDmgNotify;
    void doDamageBroadcast();
    void addSceneDamage(void * pAtkData);
    
    void * mBeSkilledNotify;
    //void doBeSkilledBroadcast();
    void addSceneBeSkilled(void * pBeSkilledData);
    
    void * mBeAtkedNotify;
    //void doBeSkilledBroadcast();
    void addSceneBeAtked(void * pBeAtkedData);
    // end add
    
    
    WorldObject* findObject(int instId);
    void addObject(WorldObject* obj);
    void removeObject(WorldObject* obj);

    vector<WorldCreature*> getAttackObjsByArea(SceneObject* srcObj, const CCRect& area);
    vector<WorldCreature*> getAttackObjsByArea(SceneObject* srcObj, WorldObjectType srcObjType, const CCRect& area);
    vector<WorldCreature*> atkObjs(WorldCreature* obj);

    vector<WorldCreature*> getAreaCanAttackObjs(SceneObject* srcObj, WorldObjectType srcObjType);
    vector<WorldCreature*> getAreaSelfGroup(WorldCreature* srcObj);

    bool outOfArea(WorldObject* obj);//如果出界，矫正回来
    
    
    WorldPlayer* getPlayerByCreature(WorldCreature* creature);
    
    void updateObjects(uint64_t ms);
    void removeInvalidObjects();
    
    //
    int getSceneModId();
    int getSceneType();
    int getAreaIdByPosition(int x, int y);

    void handleLeavedPlayers();
    
    virtual void onInit() {}
    virtual void onDeinit() {}
    virtual void onEvent(int eventId, long lParam, long rParam);
    
    void emitEvent(int eventId, long lParam, long rParam);
    
    WorldMonster* createMonster(int monsterId, int posX, int posY);
public:
    typedef map<WorldCreature*, int> CreatureHp;

    const CreatureHp collectRoleHp() const;
    const CreatureHp findHpChangedCreatures(const CreatureHp &ch) const;
    
    vector<WorldSceneEvent> mEvents;
};


#endif /* defined(__GameSrv__WorldScene__) */
