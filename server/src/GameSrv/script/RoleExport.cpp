#include "RoleExport.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#ifdef __cplusplus
}
#endif

#include "Role.h"

static Role*
l_check_role(lua_State *L, int n) {
    return *(Role**)luaL_checkudata(L, n, "GameSrv.RawRole");
}

static int
lua_getRawRole(lua_State* L) {
    int sessionId = lua_tonumber(L, -1);
    Role* role = SRoleMgr.getRoleBySessionId(sessionId);
    if (role) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, role->getRawRef());
    } else {
        return 0;
    }
    return 1;
}

static int
lua_Role_getInstID(lua_State* L) {
    Role* role = l_check_role(L, 1);
    lua_pushinteger(L, role->getInstID());
    return 1;
}

static int
lua_Role_getLuaRole(lua_State* L) {
    Role* role = l_check_role(L, 1);
    lua_rawgeti(L, LUA_REGISTRYINDEX, role->getLuaRef());
    return 1;
}

static int
lua_Role_loadLuaRole(lua_State* L) {
    Role* role = l_check_role(L, 1);
    
    RedisResult redisResult(redisCmd("hgetall LuaRole:%d", role->getInstID()));
    int eleNum = redisResult.getHashElements();
    lua_createtable(L, 0, eleNum / 2);
    for (int i = 0; i < eleNum; i += 2) {
        string key = redisResult.readHash(i);
        string val = redisResult.readHash(i + 1);
        lua_pushstring(L, val.c_str());
        lua_setfield(L, -2, key.c_str());
    }
    return 1;
}

static int
lua_Role_saveLuaRole(lua_State* L) {
    Role* role = l_check_role(L, 1);
    string luaRoleKey = strFormat("LuaRole:%d", role->getInstID());
    
    lua_pushvalue(L, -1);
    lua_pushnil(L);
    
    RedisArgs redisArgs;
    RedisHelper::beginHmset(redisArgs, luaRoleKey.c_str());
    while (lua_next(L, -2)) {
        lua_pushvalue(L, -2);
        string hkey = lua_tostring(L, -1);
        string val = lua_tostring(L, -2);
        lua_pop(L, 2);
        RedisHelper::appendHmset(redisArgs, hkey.c_str(), val.c_str());
    }
    RedisHelper::commitHmset(get_DbContext(), redisArgs);
    
    lua_pop(L, 1);

    return 0;
}


#define RoleGetDeclare(name) lua_Role_get##name
#define RoleGetDefine(name) \
static int RoleGetDeclare(name)(lua_State* L) {\
    Role* role = l_check_role(L, 1); \
    int value = role->get##name(); \
    lua_pushinteger(L, value); \
    return 1;\
}

#define RoleAddDeclare(name) lua_Role_add##name
#define RoleAddDefine(name) \
static int RoleAddDeclare(name)(lua_State* L) {\
Role* role = l_check_role(L, 1); \
int value = lua_tonumber(L, 2);\
string comeFrom = lua_tostring(L, 3);\
role->add##name(value); \
return 0;\
}

#define RolePropDefine(name) \
RoleGetDefine(name) \
RoleAddDefine(name)

RolePropDefine(Gold)
RolePropDefine(Rmb)
RolePropDefine(Fat)
RolePropDefine(Exp)
RolePropDefine(BattlePoint)
RolePropDefine(ConstellVal)
RolePropDefine(VipExp)
RolePropDefine(EnchantDust)
RolePropDefine(EnchantGold)

static int
lua_Role_addAward(lua_State* L) {
    Role* role = l_check_role(L, 1);
    role->getGold();
    return 0;
}

bool
exportRawRole(lua_State *L) {
    lua_register(L, "getRawRole", lua_getRawRole);
    
    luaL_Reg roleCoreRegs[] = {
        {"getInstID", lua_Role_getInstID},
        {"getLuaRole", lua_Role_getLuaRole},
        {"loadLuaRole", lua_Role_loadLuaRole},
        {"saveLuaRole", lua_Role_saveLuaRole},

#define RolePropReg(name) {"get"#name, RoleGetDeclare(name)}, {"add"#name, RoleAddDeclare(name)}
        RolePropReg(Gold),
        RolePropReg(Rmb),
        RolePropReg(Fat),
        RolePropReg(Exp),
        RolePropReg(BattlePoint),
        RolePropReg(ConstellVal),
        RolePropReg(VipExp),
        RolePropReg(EnchantDust),
        RolePropReg(EnchantGold),
    
        {NULL, NULL}
    };
    
    luaL_newmetatable(L, "GameSrv.RawRole");
    luaL_setfuncs(L, roleCoreRegs, 0);
    //luaL_setfuncs(L, rolePropRegs, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    lua_setglobal(L, "RawRole");
    return true;
}
