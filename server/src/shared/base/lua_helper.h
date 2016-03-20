//
//  lua_helper.h
//  GameSrv
//
//  Created by prcv on 13-8-30.
//
//

#ifndef shared_lua_helper_h
#define shared_lua_helper_h

#include "MsgBuf.h"
#include <string>
#include <vector>

#ifdef __cplusplus
extern "C"
{
#endif
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#ifdef __cplusplus
}
#endif

using namespace std;

struct lua_State;
class LuaState;
class Role;

template<class T> struct _TYPE{};
typedef void (*LuaPack)(lua_State* state, void* param);
typedef void (*LuaUnpack)(lua_State* state, int index, void* param);

class LuaStateReset
{
public:
    LuaStateReset(lua_State* state);
    ~LuaStateReset();
    
private:
    lua_State* mState;
    int mStackTop;
};

class LuaCall
{
public:
    LuaCall(lua_State* state, const char* func);
    LuaCall(lua_State* state, const char* table, const char* func);
    LuaCall(lua_State* state, int index, int ref, const char* func);
    bool doCall(int errHandle);
    void end();
    lua_State* getState() {return mState;}
    ~LuaCall();
    
    template<class T>
    void addParameter(T param);
    void addParameter(LuaPack packer, void* data);
    void addParameter(int index, int ref);
    template<class T>
    T getRet(int index);
    void getRet(int index, LuaUnpack unpacker, void* data);
    int  getRet(int index, int refIndex);
    
private:
    lua_State* mState;
    int mStackTop;
    bool mActive;
};

class LuaState
{
public:
    static LuaState* create();
    LuaState();
    ~LuaState();
    
    bool init();
    void load(const char* scriptPath);
    void loadDir(const char* dirPath);
    void doString(const char* script);
    int call(const char* func, CMsgTyped& param);
    
    virtual int getErrorHandle() {return 0;}
    lua_State* getState() {return mState;}
    
    void addSearchPath(const char* path);
    const char* getSearchPath();
    
    static bool doCall(lua_State* state, int inParamNum);
    
    static int call(lua_State* state, const char* func, CMsgTyped& msg);
    static void pushValue(lua_State* state, int value);
    static int  getValue(_TYPE<int>&, lua_State* state, int index);
    static void pushValue(lua_State* state, double value);
    static double  getValue(_TYPE<double>&, lua_State* state, int index);
    static void pushValue(lua_State* state, const char* value);
    static string  getValue(_TYPE<string>&, lua_State* state, int index);
    static void pushValue(lua_State* state, LuaPack unpacker, void* data);
    static void  getValue(lua_State* state, int index, LuaUnpack unpacker, void* data);
    
    static void pushValue(lua_State* state, int index, int ref);
    
protected:
    lua_State* mState;
};

template<class T>
void LuaCall::addParameter(T param) {
    LuaState::pushValue(mState, param);
}

template<class T>
T LuaCall::getRet(int index) {
    if (index > 0) {
        index += mStackTop;
    }
    LuaState::getValue(_TYPE<T>(), mState, index);
}

#endif
