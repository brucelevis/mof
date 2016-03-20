#include "centerScript.h"
#include "lua_helper.h"
#include "process.h"


CenterScript* CenterScript::sInstance = NULL;

bool CenterScript::init()
{
    if (!LuaState::init()) {
        return false;
    }
    
    //加载策划公式
    string lang = Process::env.getString("lang");
    string configScript = Process::execpath + "/../res/" + lang + "/lua/";
    loadDir(configScript.c_str());
    
    return true;
}

CenterScript* CenterScript::instance()
{
    return sInstance;
}

bool CenterScript::globalInit()
{
    sInstance = new CenterScript;
    
    if (!sInstance->init()) {
        delete sInstance;
        sInstance = NULL;
        return false;
    }
    return true;
}

void CenterScript::globalDestory()
{
    delete sInstance;
    sInstance = NULL;
}