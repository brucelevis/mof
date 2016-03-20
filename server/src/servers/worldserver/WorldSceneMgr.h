//
//  WorldSceneMgr.h
//  GameSrv
//
//  Created by xinyou on 14-4-3.
//
//

#ifndef __GameSrv__WorldSceneMgr__
#define __GameSrv__WorldSceneMgr__

#include "WorldScene.h"


class WorldSceneTemp
{
public:
    static WorldSceneTemp* create(int sceneModId);
    void traverseScenes(TraverseCallback cb, void* param);
    WorldScene* createScene(const string& customProp);
    
    int getType();
    WorldScene* getScene(int remainCap);
    
    SceneCfgDef* mSceneDef;
    map<int, WorldScene*> mScenes;
    
    void update(uint64_t ms);
};

class WorldSceneMgr
{
public:
    map<int, WorldSceneTemp*> mWorldSceneTemps;
    
    bool init()
    {
        if (!g_WorldObjectMgr.init(eWorldObjectScene, 10000)) {
            return false;
        }
        
        return true;
    }
    
    void update(uint64_t ms)
    {
        map<int, WorldSceneTemp*>::iterator iter = mWorldSceneTemps.begin();
        map<int, WorldSceneTemp*>::iterator iterEnd = mWorldSceneTemps.end();
        
        while (iter != iterEnd) {
            iter->second->update(ms);
            ++iter;
        }
    }
    
    WorldSceneTemp* findSceneTemp(int sceneMod)
    {
        WorldSceneTemp* sceneTemp = NULL;
        map<int, WorldSceneTemp*>::iterator iter = mWorldSceneTemps.find(sceneMod);
        if (iter != mWorldSceneTemps.end()) {
            sceneTemp = iter->second;
        } else {
            sceneTemp = WorldSceneTemp::create(sceneMod);
            if (sceneTemp) {
                mWorldSceneTemps[sceneMod] = sceneTemp;
            }
        }
        
        return sceneTemp;
    }
    
    WorldScene* createScene(int sceneModId, const string& extendProp)
    {
        WorldSceneTemp* sceneTemp = NULL;
        map<int, WorldSceneTemp*>::iterator iter = mWorldSceneTemps.find(sceneModId);
        if (iter == mWorldSceneTemps.end()) {
            sceneTemp = WorldSceneTemp::create(sceneModId);
            if (sceneTemp == NULL) {
                return NULL;
            }
            mWorldSceneTemps[sceneModId] = sceneTemp;
        } else {
            sceneTemp = iter->second;
        }
        
        WorldScene* scene = sceneTemp->createScene(extendProp);
        return scene;
    }
};


extern WorldSceneMgr g_WorldSceneMgr;



#endif /* defined(__GameSrv__WorldSceneMgr__) */
