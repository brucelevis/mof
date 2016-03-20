//
//  LuaRole.cpp
//  GameSrv
//
//  Created by 丁志坚 on 15/5/14.
//
//

#include "LuaRole.h"
#include "GameScript.h"
#include "Role.h"
#include "RoleExport.h"
#include "lua_helper.h"

static void
readBaseProp(lua_State* state, int index, void* param) {
    BaseProp* baseProp = (BaseProp*)param;
    double value;
    lua_getfield(state, index, "mStre");
    value = lua_tonumberx(state, -1, NULL);
    baseProp->mStre += value;
    lua_pop(state, 1);
    
    lua_getfield(state, index, "mInte");
    value = lua_tonumberx(state, -1, NULL);
    baseProp->mInte += value;
    lua_pop(state, 1);
    
    lua_getfield(state, index, "mPhys");
    value = lua_tonumberx(state, -1, NULL);
    baseProp->mPhys += value;
    lua_pop(state, 1);
    
    lua_getfield(state, index, "mCapa");
    value = lua_tonumberx(state, -1, NULL);
    baseProp->mCapa += value;
    lua_pop(state, 1);
}

static void
readBattleProp(lua_State* state, int index, void* param) {
    BattleProp& batProp = *((BattleProp*)param);
    double value;
    
    lua_getfield(state, index, "mAtk");
    value = lua_tonumberx(state, -1, NULL);
    batProp.mAtk += value;
    lua_pop(state, 1);
    
    lua_getfield(state, index, "mDef");
    value = lua_tonumberx(state, -1, NULL);
    batProp.mDef += value;
    lua_pop(state, 1);
    
    lua_getfield(state, index, "mDodge");
    value = lua_tonumberx(state, -1, NULL);
    batProp.mDodge += value;
    lua_pop(state, 1);
    
    lua_getfield(state, index, "mHit");
    value = lua_tonumberx(state, -1, NULL);
    batProp.mHit += value;
    lua_pop(state, 1);
    
    lua_getfield(state, index, "mCri");
    value = lua_tonumberx(state, -1, NULL);
    batProp.mCri += value;
    lua_pop(state, 1);
    
    lua_getfield(state, index, "mMaxHp");
    value = lua_tonumberx(state, -1, NULL);
    batProp.mMaxHp += value;
    lua_pop(state, 1);
    
}


LuaRole*
LuaRole::create(Role *role) {
    LuaRole* luaRole = new LuaRole();
    if (!luaRole->init(role)) {
        delete luaRole;
        luaRole = NULL;
    }
    
    return luaRole;
}


LuaRole::LuaRole() {
    mRawRef = 0;
    mLuaRef = 0;
}

LuaRole::~LuaRole() {
    lua_State* state = GameScript::instance()->getState();
    if (mRawRef > 0) {
        luaL_unref(state, LUA_REGISTRYINDEX, mRawRef);
    }
    if (mLuaRef > 0) {
        luaL_unref(state, LUA_REGISTRYINDEX, mLuaRef);
    }
    
}

bool
LuaRole::init(Role* role) {
    lua_State* state = GameScript::instance()->getState();
    
    Role** data = (Role**)lua_newuserdata(state, sizeof(Role*));
    *data = role;
    luaL_getmetatable(state, "GameSrv.RawRole");
    lua_setmetatable(state, -2);
    mRawRef = luaL_ref(state, LUA_REGISTRYINDEX);
    if (mRawRef <= 0) {
        return false;
    }
    
    LuaCall luaCall(GameScript::instance()->getState(), "createLuaRole");
    luaCall.addParameter(LUA_REGISTRYINDEX, mRawRef);
    if (luaCall.doCall(GameScript::instance()->getErrorHandle())) {
        mLuaRef = luaCall.getRet(1, LUA_REGISTRYINDEX);
    } else {
        return false;
    }
    
    return true;
}

void
LuaRole::accuProperty(BaseProp &baseProp, BattleProp &batProp) {
    LuaCall luaCall(GameScript::instance()->getState(), LUA_REGISTRYINDEX, mLuaRef, "calcAbility");
    if (luaCall.doCall(GameScript::instance()->getErrorHandle())) {
        BaseProp luaBaseProp;
        BattleProp luaBattleProp;
        luaCall.getRet(1, readBaseProp, &luaBaseProp);
        luaCall.getRet(2, readBattleProp, &luaBattleProp);
        
        baseProp += luaBaseProp;
        batProp += luaBattleProp;
    }
}

void
LuaRole::loadData() {
    LuaCall luaCall(GameScript::instance()->getState(), LUA_REGISTRYINDEX, mLuaRef, "loadData");
    luaCall.doCall(GameScript::instance()->getErrorHandle());
}
