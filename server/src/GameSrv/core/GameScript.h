//
//  scripts.h
//  GameSrv
//
//  Created by prcv on 13-10-18.
//
//

#ifndef __GameSrv__gamescript__
#define __GameSrv__gamescript__

#include <iostream>
#include <string>
#include "lua_helper.h"
using namespace std;

class GameScript : public LuaState
{
public:
    bool init();
    bool roleCall(Role* role, const char* func, CMsgTyped& ret);
    
    int getErrorHandle();
public:
    static bool globalInit();
    static void globalDestory();
    static int  luaErrorHandle(lua_State* state);
    static GameScript* instance();
    
private:
    int mErrorHandle;
    static GameScript* sInstance;
};

#endif /* defined(__GameSrv__scripts__) */
