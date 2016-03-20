
#pragma once

#ifndef ___LUASTATE_WAPPER_H___
#define ___LUASTATE_WAPPER_H___

#include "src/lua.h"
#include "src/lauxlib.h"
#include "src/lualib.h"

#include "../MessageQueue/MsgBuf.h"
#include "../StringUtils/StringUtil.h"

 static void StackDump (lua_State *L)
 {
    int i;
    int top = lua_gettop(L);
    for (i = 1; i <= top; i++)
    {
        int t = lua_type(L, i);
        switch (t)
        {
        case LUA_TSTRING:
            {
                printf("%s", lua_tostring(L, i));
                break;
            }
        case LUA_TBOOLEAN:
            {
                printf(lua_toboolean(L, i) ? "true" : "false");
                break;
            }
        case LUA_TNUMBER:
            {
                printf("%g", lua_tonumber(L, i));
                break;
            }
        default:
            {
                printf("%s", lua_typename(L, t));
                break;
            }
        }
        printf(" ");
    }
    printf("\n");
}

class LuaState
{
public:
    LuaState();
    virtual ~LuaState();

public:
    bool Open();
    bool LoadFile(const char* filename);
    bool DoFile(const char* filename);
    int GetInt(const char* varname);
    const char* GetString(const char* varname);
    float GetFloat(const char* varname);
    double GetDouble(const char* varname);
    bool GetBool(const char* varname);
    bool Call(const char* fun, CMsgTyped* param);
    void Register(const char* name, lua_CFunction fun);
    void DumpStack() { if (L) StackDump( L ); }
protected:
    lua_State* L;
};


#endif


