//
//  SceneMgr.h
//  GameSrv
//
//  Created by xinyou on 14-5-12.
//
//

#ifndef __GameSrv__SceneMgr__
#define __GameSrv__SceneMgr__

#include <iostream>
#include <vector>
#include <map>
using namespace std;

class Obj;
class Scene;

typedef std::vector<Scene*> SceneList;
typedef std::vector<Scene*>::iterator SceneListIter;

class SceneMgr {
public:
    static void dump();
    static void init();
    static void updateAll(float dt);
    
    static void recycleScenes();
    
    static Scene* create(int _id);
    static Scene* objJoin(Obj* r);
    static std::map<int, SceneList> mScenes;
    
    static int getSceneThreadNum(int sceneId);
    
    static SceneList* getSceneList(int sceneId);
    static Scene* getScene(int sceneId);
    static Scene* getScene(int sceneId, int threadId);
    
    //static float sRecycleTimer;
    static int   sNextRecycleTime;
    static int   sBatCreateSceneNum;
};

#endif /* defined(__GameSrv__SceneMgr__) */
