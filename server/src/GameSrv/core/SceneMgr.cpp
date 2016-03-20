//
//  SceneMgr.cpp
//  GameSrv
//
//  Created by xinyou on 14-5-12.
//
//

#include "SceneMgr.h"

#include "Scene.h"
#include "Obj.h"
#include "Game.h"


/*
 static -------------------------------------------------
 */
std::map<int, SceneList > SceneMgr::mScenes;

int SceneMgr::sNextRecycleTime = 0;
int SceneMgr::sBatCreateSceneNum = 2;

void SceneMgr::init()
{
    vector<int> sceneids;
    SceneCfg::findSceneForType(stTown, sceneids);
    for (int i = 0; i < sceneids.size(); i++)
    {
        SceneList scenelist;
        mScenes.insert(std::make_pair(sceneids[i], scenelist));
    }
}

void SceneMgr::dump()
{
    std::map<int, SceneList >::iterator iter;
    
    for (iter = mScenes.begin(); iter != mScenes.end(); iter++)
    {
        int index = 0;
        printf("----------------Scene %d---------------\n", iter->first);
        for (SceneListIter liter = iter->second.begin();
             liter != iter->second.end(); liter++)
        {
            printf("%d %ld\n", index++, (*liter)->mObjs.size());
        }
        printf("---------------------------------------\n\n");
    }
}

Scene* SceneMgr::create(int _id){
    std::map<int, SceneList >::iterator iter = mScenes.find(_id);
    if (iter == mScenes.end())
    {
        return NULL;//todo log
    }
    Scene* s = new Scene(_id);
    iter->second.push_back(s);
    return s;
}

void SceneMgr::updateAll(float dt)
{
    std::map<int, SceneList >::iterator it;
    for (it=mScenes.begin(); it!=mScenes.end(); it++)
    {
        SceneList& scenes = it->second;
        for (SceneListIter iter = scenes.begin(); iter != scenes.end(); iter++ )
        {
            (*iter)->update(dt);
        }
    }
    
    recycleScenes();
}

void SceneMgr::recycleScenes()
{
    //定期回收场景
    if (sNextRecycleTime == 0)
    {
        sNextRecycleTime = Game::tick + 60;
    }
    
    if (sNextRecycleTime > Game::tick)
    {
        return;
    }
    
    sNextRecycleTime = Game::tick + 60;
    std::map<int, SceneList >::iterator it;
    for (it = mScenes.begin(); it != mScenes.end(); it++)
    {
        SceneList& scenes = it->second;
        if (scenes.size() <= sBatCreateSceneNum)
        {
            continue;
        }
        
        int count = 0;
        std::vector<Scene*>::iterator posIter = scenes.end();
        for (std::vector<Scene*>::reverse_iterator riter = scenes.rbegin(); riter != scenes.rend(); riter++ )
        {
            Scene* scene = *riter;
            if (scene->getRoleNum() > 0)
            {
                break;
            }
            
            count++;
            if (count % sBatCreateSceneNum == 0)
            {
                posIter = riter.base();
            }
        }
        
        if (posIter != scenes.end())
        {
            for (SceneListIter iter = posIter; iter != scenes.end(); iter++ )
            {
                Scene* scene = *iter;
                delete scene;
            }
            scenes.erase(posIter, scenes.end());
        }
    }
}


SceneList* SceneMgr::getSceneList(int sceneId)
{
    map<int, SceneList>::iterator iter = mScenes.find(sceneId);
    if (iter == mScenes.end())
    {
        return NULL;
    }
    
    return &(iter->second);
}


int SceneMgr::getSceneThreadNum(int sceneId)
{
    map<int, SceneList>::iterator iter = mScenes.find(sceneId);
    if (iter == mScenes.end())
    {
        return 0;
    }
    
    return iter->second.size();
}

Scene* SceneMgr::getScene(int sceneId)
{
    map<int, SceneList>::iterator iter = mScenes.find(sceneId);
    if (iter == mScenes.end())
    {
        return NULL;
    }
    
    Scene* scene = NULL;
    SceneList& sceneList = iter->second;
    SceneListIter siter;
    for (siter = sceneList.begin(); siter != sceneList.end(); siter++)
    {
        scene = *siter;
        if (scene->getRoleNum() < GlobalCfg::ReadInt("city_full_num", 15))
        {
            break;
        }
    }
    
    if (siter == sceneList.end() || scene == NULL)
    {
        scene = SceneMgr::create(sceneId);
        scene->setThreadId(sceneList.size());
        for (int i = 1; i < sBatCreateSceneNum; i++)
        {
            Scene* extraScene = SceneMgr::create(sceneId);
            extraScene->setThreadId(sceneList.size());
        }
    }
    return scene;
}

Scene* SceneMgr::getScene(int sceneId, int threadId)
{
    map<int, SceneList>::iterator iter = mScenes.find(sceneId);
    if (iter == mScenes.end())
    {
        return NULL;
    }
    
    if (threadId > iter->second.size() || threadId <= 0)
    {
        return NULL;
    }
    
    return iter->second[threadId - 1];
}