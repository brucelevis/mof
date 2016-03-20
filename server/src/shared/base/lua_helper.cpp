//
//  lua_helper.cpp
//  GameSrv
//
//  Created by prcv on 13-8-30.
//
//

#include "lua_helper.h"
#include "os.h"

LuaStateReset::LuaStateReset(lua_State* state) {
    mState = state;
    mStackTop = lua_gettop(mState);
}

LuaStateReset::~LuaStateReset() {
    lua_settop(mState, mStackTop);
}

LuaCall::LuaCall(lua_State* state, const char* func) {
    mState = state;
    mStackTop = lua_gettop(mState);
    lua_getglobal(mState, func);
}

LuaCall::LuaCall(lua_State* state, const char* table, const char* func) {
    mState = state;
    mStackTop = lua_gettop(mState);
    lua_getglobal(mState, table);
    lua_getfield(mState, -1, func);
    lua_remove(mState, -2);
}

LuaCall::LuaCall(lua_State* state, int index, int ref, const char* func) {
    mState = state;
    mStackTop = lua_gettop(mState);
    lua_rawgeti(mState, index, ref);
    lua_getfield(mState, -1, func);
    lua_insert(mState, -2);
}

bool
LuaCall::doCall(int errHandle) {
    int stackTop = lua_gettop(mState);
    int paramNum = stackTop - mStackTop - 1;
    
    int ret = lua_pcall(mState, paramNum, LUA_MULTRET, errHandle);
    mActive = ret == LUA_OK;
    if (!mActive) {
        printf("%s\n", lua_tostring(mState, -1));
    }
    return mActive;
}

void
LuaCall::addParameter(LuaPack packer, void* data) {
    LuaState::pushValue(mState, packer, data);
}

void
LuaCall::addParameter(int index, int ref) {
    LuaState::pushValue(mState, index, ref);
}

void
LuaCall::getRet(int index, LuaUnpack unpacker, void *data) {
    if (index > 0) {
        index += mStackTop;
    }
    LuaState::getValue(mState, index, unpacker, data);
}

int
LuaCall::getRet(int index, int refIndex) {
    if (index > 0) {
        index += mStackTop;
    }
    lua_pushvalue(mState, index);
    int ref = luaL_ref(mState, refIndex);
    return ref;
}

void
LuaCall::end() {
    lua_settop(mState, mStackTop);
    mActive = false;
}

LuaCall::~LuaCall() {
    end();
}



LuaState::LuaState()
{
    mState = NULL;
}

LuaState::~LuaState()
{
    if (mState != NULL)
    {
        lua_close(mState);
    }
}

LuaState* LuaState::create()
{
    LuaState* state = new LuaState;
    if (state->init())
    {
        return state;
    }
    
    delete state;
    return NULL;
}

bool LuaState::init()
{
    mState = luaL_newstate();
    luaL_openlibs(mState);
    return true;
}

void
LuaState::addSearchPath(const char *path) {
    string pathType = "path";
    if (strstr(path, ".so") != NULL) {
        pathType = "cpath";
    }
    
    int stackTop = lua_gettop(mState);
    lua_getglobal(mState, "package");
    lua_pushstring(mState, pathType.c_str());
    lua_gettable(mState, -2);
 
    string searchPath = lua_tostring(mState, -1);
    lua_pop(mState, 1);
    searchPath.append(";");
    searchPath.append(path);
    
    lua_pushstring(mState, pathType.c_str());
    lua_pushstring(mState, searchPath.c_str());
    lua_settable(mState, -3);
    
    lua_settop(mState, stackTop);
}

void LuaState::loadDir(const char *path)
{
    vector<string> files;
    fs::listFiles(path, files);
    
    for (int i = 0; i < files.size(); i++)
    {
        size_t pos = files[i].rfind(".lua");
        if (pos == string::npos || pos != files[i].length() - 4)
        {
            continue;
        }
        load(files[i].c_str());
    }
}

void LuaState::load(const char* script)
{
    int stackTop = lua_gettop(mState);
    int ret = luaL_dofile(mState, script);
    if (ret != LUA_OK) {
        printf("%s\n", lua_tostring(mState, -1));
    }
    lua_settop(mState, stackTop);
}

void
LuaState::doString(const char *script) {
    int stackTop = lua_gettop(mState);
    int ret = luaL_dostring(mState, script);
    if (ret != LUA_OK) {
        printf("%s\n", lua_tostring(mState, -1));
    }
    lua_settop(mState, stackTop);
}

int
LuaState::call(lua_State *state, const char *func, CMsgTyped &param) {
    LuaStateReset reset(state);
    
    int stackTop = lua_gettop(state);
    lua_getglobal(state, func);
    if (!lua_isfunction(state, -1)) {
        return -1;
    }
    
    param.SeekToBegin();
    bool paramError = false;
    bool reachEnd = false;
    for (;;) {
        
        DATA_TYPES type = param.TestType();
        switch (type) {
            case TYPE_INT:
                lua_pushinteger(state, param.IntVal());
                break;
            case TYPE_FLOAT:
                lua_pushnumber(state, param.FloatVal());
                break;
            case TYPE_STRING:
                lua_pushstring(state, param.StringVal());
                break;
            case TYPE_NONE:
                reachEnd = true;
                break;
            default:
                paramError = true;
                break;
        }
        
        if (paramError || reachEnd) {
            break;
        }
    }
    
    if (paramError) {
        return -1;
    }
    
    int inParamNum = lua_gettop(state) - stackTop - 1;
    bool ret = doCall(state, inParamNum);
    if (ret) {
        printf("exec lua func:%s with error: %s\n", func, lua_tostring(state, -1));
        return -1;
    }
    
    int nresult = lua_gettop(state) - stackTop;
    CMsgTyped retparam;
    for (int i = -nresult; i < 0; ++i) {
        int type = lua_type(state, i);
        switch (type) {
            case LUA_TNUMBER:
                retparam.SetFloat(lua_tonumber(state, i));
                break;
            case LUA_TSTRING:
                retparam.SetString(lua_tostring(state, i));
                break;
            default:
                break;
        }
    }
    
    param = retparam;
    param.SeekToBegin();
    
    return nresult;
}

bool
LuaState::doCall(lua_State *state, int inParamNum) {
    int ret = lua_pcall(state, inParamNum, LUA_MULTRET, 0);
    return (ret != LUA_OK);
}

int
LuaState::call(const char* func, CMsgTyped& param) {
    return LuaState::call(mState, func, param);
}

void
LuaState::pushValue(lua_State* state, int value) {
    lua_pushinteger(state, value);
}
int
LuaState::getValue(_TYPE<int>&, lua_State* state, int index) {
    int num = lua_tointeger(state, index);
    return num;
}
void
LuaState::pushValue(lua_State* state, double value) {
    lua_pushnumber(state, value);
}
double
LuaState::getValue(_TYPE<double>&, lua_State* state, int index) {
    double value = lua_tonumber(state, index);
    return value;
}
void
LuaState::pushValue(lua_State* state, const char* value) {
    lua_pushstring(state, value);
}
string
LuaState::getValue(_TYPE<string>&, lua_State* state, int index) {
    string value = lua_tostring(state, index);
    return value;
}
void
LuaState::pushValue(lua_State* state, LuaPack packer, void* data) {
    packer(state, data);
}
void
LuaState::getValue(lua_State* state, int index, LuaUnpack unpacker, void* data) {
    unpacker(state, index, data);
}

void
LuaState::pushValue(lua_State *state, int index, int ref) {
    lua_rawgeti(state, index, ref);
}