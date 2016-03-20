//
//  WorldScriptMgr.h
//  GameSrv
//
//  Created by prcv on 14-6-20.
//
//

#ifndef __GameSrv__WorldScript__
#define __GameSrv__WorldScript__

#include "lua_helper.h"

class WorldScript : public LuaState
{
public:
    bool init();
    
    static WorldScript* instance();
    static bool globalInit();
    static void globalDestory();
protected:
    WorldScript();
    ~WorldScript();
    
private:
    static WorldScript* sWorldScript;
    
};


#endif /* defined(__GameSrv__WorldScriptMgr__) */
