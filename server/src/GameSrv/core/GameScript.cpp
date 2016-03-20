//
//  scripts.cpp
//  GameSrv
//
//  Created by prcv on 13-10-18.
//
//

#include "GameScript.h"
#include "Game.h"
#include "lua_helper.h"
#include "RoleExport.h"
#include "NetExport.h"
#include "JsonExport.h"
#include "process.h"

extern int luaopen_cjson(lua_State *l);

GameScript* GameScript::sInstance = NULL;

bool GameScript::init()
{
    if (!LuaState::init()) {
        return false;
    }
    
    //注册c++方法
    exportRawRole(mState);
    exportNetLibrary(mState);
    exportJson(mState);
    
    //加载策划公式
    string lang = Process::env.getString("lang");
    string configScript = Process::execpath + "/../res/" + lang + "/lua/";
    loadDir(configScript.c_str());
    
    //加载逻辑代码
    string logicScript = Process::execpath  + "/../res/" + lang + "/script";
    string commonPath = logicScript + "/common/?.lua";
    string modulePath = logicScript + "/module/?.so";
    string gamePath =   logicScript + "/game/?.lua";
    sInstance->addSearchPath(commonPath.c_str());
    sInstance->addSearchPath(modulePath.c_str());
    sInstance->addSearchPath(gamePath.c_str());
    sInstance->doString("require 'main'");
    
    return true;
}

GameScript* GameScript::instance()
{
    return sInstance;
}

bool GameScript::globalInit()
{
    sInstance = new GameScript;
    
    if (!sInstance->init()) {
        delete sInstance;
        sInstance = NULL;
        return false;
    }
    
    lua_pushcfunction(sInstance->mState, GameScript::luaErrorHandle);
    sInstance->mErrorHandle = lua_gettop(sInstance->mState);
    return true;
}

int
GameScript::getErrorHandle() {
    return mErrorHandle;
}

int
GameScript::luaErrorHandle(lua_State *state) {
    CMsgTyped param;
    LuaState::call(state, "debugTraceback", param);
    return 0;
}

void GameScript::globalDestory()
{
    delete sInstance;
    sInstance = NULL;
}