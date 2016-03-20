//
//  WorldScriptMgr.cpp
//  GameSrv
//
//  Created by prcv on 14-6-20.
//
//

#include "WorldScript.h"
#include "process.h"

WorldScript* WorldScript::sWorldScript = NULL;

WorldScript::WorldScript()
{
    
}

WorldScript::~WorldScript()
{

}

bool WorldScript::init()
{
    if (!LuaState::init()) {
        return false;
    }
    
    string lang = Process::env.getString("lang");
    string luapath = Process::execpath + "/../res/" + lang + "/lua/";
    loadDir(luapath.c_str());
    return true;
}

bool WorldScript::globalInit()
{
    sWorldScript = new WorldScript;
    if (sWorldScript->init()) {
        return true;
    }
    
    delete sWorldScript;
    sWorldScript = NULL;
    return false;
}

void WorldScript::globalDestory()
{
    delete sWorldScript;
    sWorldScript = NULL;
}

WorldScript* WorldScript::instance()
{
    return sWorldScript;
}
