//
//  WorldSceneMgr.cpp
//  GameSrv
//
//  Created by xinyou on 14-4-3.
//
//

#include "WorldSceneMgr.h"
#include "WorldScene.h"
#include "scene/WorldScenePvp.h"
#include "scene/WorldSceneTreasureFight.h"


WorldSceneMgr g_WorldSceneMgr;

WorldSceneTemp* WorldSceneTemp::create(int sceneModId)
{
    SceneCfgDef* def = SceneCfg::getCfg(sceneModId);
    if (def == NULL)
    {
        return NULL;
    }
    
    // added by jianghan for 脚本场景测试,任何场景都成为一个脚本场景
    // def->sceneType = stScriptScene;
    // end add
    
    WorldSceneTemp* sceneMod = new WorldSceneTemp;
    sceneMod->mSceneDef = def;
    return sceneMod;
}

void WorldSceneTemp::traverseScenes(TraverseCallback cb, void* param)
{
    map<int, WorldScene*>::iterator iter = mScenes.begin();
    while (iter != mScenes.end())
    {
        if (!cb(iter->second, param))
        {
            break;
        }
    }
}

int WorldSceneTemp::getType()
{
    return mSceneDef->sceneType;
}

WorldScene* WorldSceneTemp::getScene(int remainCap)
{
    map<int, WorldScene*>::iterator iter = mScenes.begin();
    if (iter != mScenes.end())
    {
        return iter->second;
    }
    
    return NULL;
}


WorldScene* WorldSceneTemp::createScene(const string& extendProp)
{
    WorldScene* scene = NULL;
    scene = WorldScene::create(mSceneDef, extendProp.c_str());
    if (scene) {
        mScenes[scene->getInstId()] = scene;
    }
    
    return scene;
}

void WorldSceneTemp::update(uint64_t ms)
{
    map<int, WorldScene*>::iterator iter = mScenes.begin();
    map<int, WorldScene*>::iterator iterEnd = mScenes.end();
    
    while (iter != iterEnd) {
        WorldScene* scene = iter->second;
        if (scene->getValid()) {
            scene->run(ms);
            ++iter;
        } else {
            mScenes.erase(iter++);
            WorldScene::destroy(scene);
        }
    }
}