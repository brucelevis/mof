#pragma once

#include <iostream>
#include <string>
#include "lua_helper.h"
using namespace std;

class CenterScript : public LuaState
{
public:
    bool init();

public:
    static bool globalInit();
    static void globalDestory();
    static CenterScript* instance();
    
private:
    static CenterScript* sInstance;
};
