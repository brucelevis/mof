//
//  script_scene.h
//  GameSrv
//
//  Created by mac on 14-7-17.
//
//

#ifndef GameSrv_script_scene_h
#define GameSrv_script_scene_h

#include "WorldScene.h"
#include "../WorldSceneMgr.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#define handler_lua_scene(funcname)                                 \
static int CfLua_##scene_##funcname( lua_State *context ){          \
    LuaScene * pScene = NULL;                                       \
    if(lua_isuserdata(context,1))                                   \
        pScene = *(LuaScene **)lua_touserdata(context,1);           \
    else{                                                           \
        /* 传入的不是人物对象指针，我们设置－1为返回值*/                   \
        lua_pushnumber(context, -1);                                \
        return 1;                                                   \
    }



#define handler_lua_player(funcname)                                \
static int CfLua_##player_##funcname( lua_State *context ){         \
    WorldPlayer * pPlayer = NULL;                                   \
    if(lua_isuserdata(context,1))                                   \
        pPlayer = *(WorldPlayer **)lua_touserdata(context,1);       \
    else{                                                           \
        /* 传入的不是人物对象指针，我们设置－1为返回值*/                   \
        lua_pushnumber(context, -1);                                \
        return 1;                                                   \
    }


#define handler_lua_monster(funcname)                               \
static int CfLua_##monster_##funcname( lua_State *context ){        \
    WorldMonster * pMonster = NULL;                                 \
    if(lua_isuserdata(context,1))                                   \
        pMonster = *(WorldMonster **)lua_touserdata(context,1);     \
    else{                                                           \
        /* 传入的不是怪物对象指针，我们设置－1为返回值*/                   \
        lua_pushnumber(context, -1);                                \
        return 1;                                                   \
    }

#define lua_param_int(a,n)                  int a = lua_tonumber(context, n);
#define lua_param_object(objtype, obj,n)    objtype * obj = *(objtype **)lua_touserdata(context, n);
#define lua_param_boolean(b,n)              bool b = lua_toboolean(context, n);

#define lua_ret_int(a)                      lua_pushnumber(context, a);
#define lua_ret_object(objtype,obj)         objtype ** pObj = (objtype **)lua_newuserdata(context, sizeof(objtype *));  \
                                            *pObj = obj;


// 我们的 LuaScene 继承自 WorldScenePvp, 专门用来实现同步战斗
class LuaScene : public WorldScene {
    
public:
    LuaScene(lua_State * pContext){
        m_Lua_context = pContext;
    }
    ~LuaScene();
    
public:
    bool start();
    void destroy();
    
    virtual void update(uint64_t ms);
    //virtual bool addPlayer(WorldPlayer* player);
    virtual void onAddedPlayer(WorldPlayer* player);
    virtual bool init(const SceneCfgDef* def, const char* extendProp)
    {
        if (!WorldScene::init(def, extendProp))
        {
            return false;
        }
        m_StageTime = 0;

        // 设置当前活动区域为第0个区域
        activeArea(0);

        // 通知脚本创建场景
        if( start() == false )
            return false;
        return true;
    }
    virtual void onAddMonster(WorldMonster* monster);
    virtual void onCreatureDead(WorldCreature* attacker, WorldCreature* victim);
    //virtual void onPlayerLeave(WorldPlayer* player);
    virtual void beforePlayerLeave(WorldPlayer* player);
    virtual void afterPlayerLeave() {}
    virtual int getMaxPetCount();

    //int luaAddPlayer( WorldPlayer * player );
    //int releasePlayer( WorldPlayer * pPlayer );
    
    //WorldMonster * createMonster( int monTplId );
    //WorldNpc * createNpc( int npcTplId );
    //int releaseMonster( WorldMonster * pMonster );
    
    LivingObject * searchTarget( LivingObject * pSearcher, int monType, int searchHint );
    
public:
    
    void lua_update(uint64_t ms);
    bool lua_canPlayerEnter(string roleName, int lvl, int roleJob, int vipLvl, int honorId, string guildName, int guildPosition, int weaponFasion, int bodyFasion, int maxHp, int atk, int def, float hit, float dodge, float cri);
    bool lua_addPlayer( WorldRole * player );
    bool lua_addPet( WorldPet * pet );
    int lua_getMaxPetCount();
    void lua_onCreatureDead(WorldCreature* victim, WorldCreature* attacker);
    void lua_onPlayerLeave(WorldPlayer *player);
    
    bool lua_isEnemyTarget( WorldCreature * pCreature, WorldCreature * pTarget );
    int lua_getPetCtrlMode();
    
public:
    int m_SceneId;
    int m_StageTime;        // 场景持续时间
    int m_LastRun;
    int m_NextRun;
    
private:
    lua_State * m_Lua_context;
};

class LuaScriptManager{
   
public:
    ~LuaScriptManager();
    
private:
    map<string, lua_State *> mStates;
    
public:
    lua_State * getStateContext(string& luapath, string& filepath);
    void removeStateContext(string& filename);
};

#endif
