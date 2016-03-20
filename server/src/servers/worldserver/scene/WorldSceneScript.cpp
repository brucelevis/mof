//
//  script_scene.h
//  GameSrv
//
//  Created by jianghan on 14-7-17.
//
//
//  有几个点是需要客户端调整的：
//  出了对活动的功能性支持外，还需要从场景怪物敌我关系方面做一些处理，比如将敌我关系显示在各个活动体的头顶上（比如名字颜色或者其他logo）
//  而且需要在场景活动体描述协议中增加敌我描述，并增加相应逻辑
// 
//  因为活动脚本的规则是多样性的，再不能一概而论了。
//

#include "WorldSceneScript.h"
#include "WorldMonster.h"
#include "../WorldPlayer.h"

#include "msg.h"
#include "log.h"
#include "main.h"
#include "world_cmdmsg.h"

#include "../WorldCreatureAI.h"

extern ObjJob RoleType2Job(int roletype);

int luaopen_Player (lua_State * context );
int luaopen_Monster (lua_State * context );
int luaopen_Scene (lua_State * context );

static void createmetatable (lua_State *L) {
    lua_createtable(L, 0, 1);       /* table to be metatable for strings */
    lua_pushliteral(L, "");         /* dummy string */
    lua_pushvalue(L, -2);           /* copy table */
    lua_setmetatable(L, -2);        /* set table as metatable for strings */
    lua_pop(L, 1);                  /* pop dummy string */
    lua_pushvalue(L, -2);           /* get string library */
    lua_setfield(L, -2, "__index"); /* metatable.__index = string */
    lua_pop(L, 1);                  /* pop metatable */
}

static int _lua_Debug (lua_State *L) {
    string debugInfo = lua_tostring(L, 1);
    log_info(debugInfo);
    return 0;
}

LuaScriptManager g_LuaScriptMgr;

LuaScriptManager::~LuaScriptManager()
{
    map<string, lua_State*>::iterator it = mStates.begin();
    for( it=mStates.begin();it != mStates.end();it++ )
    {
        lua_close(it->second);
    }
    mStates.clear();
}

lua_State * LuaScriptManager::getStateContext(string& luapath, string& filename)
{
    lua_State * pContext = mStates[filename];
    if( pContext == NULL )
    {
        // 加载
        pContext = luaL_newstate();
        //打开lua基本库
        luaL_openlibs(pContext);
        
        /*
        // 加载 c＋＋ 的 for Lua 怪物函数库
        luaopen_Monster(pContext);
        
        // 加载 c＋＋ 的 for Lua 场景函数库
        luaopen_Scene(pContext);
        
        // 加载 c＋＋ 的 for Lua 人物函数库
        luaopen_Player(pContext);
        */
        luaL_requiref(pContext,"CCoreImpl_Monster",luaopen_Monster,1);
        lua_pop(pContext, 1); // requiref leaves the library table on the stack
        
        luaL_requiref(pContext,"CCoreImpl_Scene",luaopen_Scene,1);
        lua_pop(pContext, 1); // requiref leaves the library table on the stack
        
        luaL_requiref(pContext,"CCoreImpl_Player",luaopen_Player,1);
        lua_pop(pContext, 1); // requiref leaves the library table on the stack
        
        // 这里我们要创建一个叫做CCoreConfig的表，然后在里面的require_path里填充我们的当前路径，脚本中会在require前合并
        // CCoreConfig.require_path 到 package.path 中，于是后面require引用的其他脚本都可以在当前路径中寻找引用了
        lua_newtable(pContext);
        lua_pushstring(pContext, luapath.c_str());
        lua_setfield(pContext, -2, "require_path");
        lua_setglobal(pContext,"CCoreConfig");

        /* 注册函数 */
        lua_pushcfunction(pContext,_lua_Debug);
        lua_setglobal(pContext,"_debug");
        
        //加载我们的lua脚本文件
        //if (luaL_loadfile(pContext, filename.c_str()))
        //这个地方不能用loadfile,而是应该dofile,不然不能直接使用里面的函数
        //loadfile的情况只能用在直接执行脚本全局代码
        int s = luaL_dofile(pContext, filename.c_str());
        if (s)
        {
            log_info(lua_tostring(pContext,-1));
            lua_close(pContext);
            return NULL;
        }
        
        mStates[filename] = pContext;
    }
    return pContext;
}

void LuaScriptManager::removeStateContext(string& filename){
    map<string, lua_State*>::iterator it = mStates.find(filename);
    mStates.erase(it);
}

/**
 *  LuaScene 类实现
 *
 */

LuaScene::~LuaScene()
{
    destroy();
}

bool LuaScene::start()
{
    lua_getglobal(m_Lua_context,"script_inCall_newScene");
    
    // 传递场景id给脚本，让脚本处理，脚本还可以通过这个id获取到场景对象userdata
    lua_pushnumber(m_Lua_context, mInstId);
    
    WorldScene ** pObj = (WorldScene **)lua_newuserdata(m_Lua_context, sizeof(WorldScene *));
    *pObj = this;
    
    //运行函数并把结果压入栈
    int s = lua_pcall(m_Lua_context,2,1,0);
	if(s != 0){
		log_info(lua_tostring(m_Lua_context, -1));
        lua_pop(m_Lua_context, 1);
        return false;
	}
    
    bool ret = (bool)lua_toboolean(m_Lua_context, -1);
    lua_pop(m_Lua_context, 1);
    
    // 控制update频率
    m_LastRun = 0;
    m_NextRun = 0;
    
    return ret;
}

void LuaScene::destroy()
{
    lua_getglobal(m_Lua_context,"script_inCall_destroyScene");
    
    // 传递场景id给脚本，让脚本处理，脚本还可以通过这个id获取到场景对象userdata
    lua_pushnumber(m_Lua_context, mInstId);
    
    //运行函数并把结果压入栈
    int s=lua_pcall(m_Lua_context,1,1,0);
    if(s != 0){
		log_info(lua_tostring(m_Lua_context, -1));
	}

    lua_pop(m_Lua_context, 1);
}

void LuaScene::update(uint64_t ms)
{
    // 一定要调用父类的update,一切计时器，对象刷新等都是它在控制
    WorldScene::update(ms);
    
    // 场景持续时间
    m_StageTime += ms;
    m_LastRun += ms;
    
    if( m_LastRun > m_NextRun )
    {
        // 处理一下各个怪物的技能cooldown过程
        /* 这个过程在怪物自己的WorldScriptMonsterAI里完成
        list<WorldObject*>::iterator iter = mObjects.begin();
        list<WorldObject*>::iterator iterEnd = mObjects.end();
        while (iter != iterEnd)
        {
            WorldObject* obj = *iter;
            if( obj != NULL && obj->getType() == eWorldObjectRobot ){
                WorldMonster * pMonster = (WorldMonster *)obj;
                pMonster->DealMonsterAiState();
            }
            iter++;
        }
        */
        
        // 将控制权交给lua脚本去执行刷新逻辑,我们设定每200ms调用一次脚本的刷新逻辑
        lua_update(ms);
        m_NextRun = m_LastRun + 200;
    }
}

void LuaScene::onAddedPlayer(WorldPlayer* player)
{
    WorldScene::onAddedPlayer(player);
    
    WorldRole* role = player->getRole();
    WorldPet* pet = player->getFirstPet();
    WorldRetinue *retinue = player -> getRetinue();
    
    if (role)
    {
        int roleHp = role->mRoleInfo.mMaxHp;
        roleHp *= SynPvpFuntionCfg::mCfgDef.roleHpMultiple;
        
        role->setHp(roleHp);
        role->setMaxHp(roleHp);
        role->setGroup(player->mRoleId);
    }
    
    if (pet)
    {
        int petHp = pet->mPetInfo.mMaxHp;
        petHp *= SynPvpFuntionCfg::mCfgDef.petHpMultiple;
        pet->setMaxHp(petHp);
        pet->setHp(petHp);
        pet->setGroup(player->mRoleId);
        
        pet->setCtrlMode(eServerCtrl);
    }
    
    if(retinue){
        retinue -> setGroup(player->mRoleId);
    }
    
    //player->adjustPvpProp();
    
    // 往脚本中添加玩家对象
    lua_addPlayer(player->getRole());
    
    // 查看一下玩家是否带宠，如果是则也要通知脚本
    int petCount = this->getMaxPetCount();
    if( petCount > 0 )
    {
        for (WorldPlayer::WorldPetIter iter = player->mPetList.begin(); iter != player->mPetList.end() && petCount>0; ++iter)
        {
            WorldPet* pet = *iter;
            if (pet)
            {
                lua_addPet(pet);
            }
            petCount --;
        }
    }
    
    // WorldRole* role = player->getRole();
    // WorldPet* pet = player->getFirstPet();
    return;
}

int LuaScene::getMaxPetCount()
{
    // 咨询脚本，看每玩家最多允许携带几只宠物进场
    return lua_getMaxPetCount();
}

void LuaScene::lua_update(uint64_t ms)
{
    // 返回，告知lua结果的个数
    lua_getglobal(m_Lua_context,"script_inCall_updateScene");

    // 传递场景id给脚本，让脚本处理，脚本还可以通过这个id获取到场景对象userdata
    lua_pushnumber(m_Lua_context, mInstId);
    lua_pushnumber(m_Lua_context, ms);
    
    //运行函数并把结果压入栈
    int s = lua_pcall(m_Lua_context,2,1,0);
    if(s != 0){
		log_info(lua_tostring(m_Lua_context, -1));
	}
    
    lua_pop(m_Lua_context, 1);

}

int LuaScene::lua_getMaxPetCount()
{
    // 返回，告知lua结果的个数
    lua_getglobal(m_Lua_context,"script_inCall_getMaxPetCount");
    
    // 传递场景id给脚本，让脚本处理，脚本还可以通过这个id获取到场景对象userdata
    lua_pushnumber(m_Lua_context, mInstId);
    
    //运行函数并把结果压入栈
    int s = lua_pcall(m_Lua_context,1,1,0);
    if(s != 0){
		log_info(lua_tostring(m_Lua_context, -1));
        lua_pop(m_Lua_context, 1);
        return 1;
	}

    int ret = (bool)lua_toboolean(m_Lua_context, -1);
    lua_pop(m_Lua_context, 1);

    return ret;
}

bool LuaScene::lua_canPlayerEnter(string roleName, int lvl, int roleJob, int vipLvl, int honorId, string guildName, int guildPosition, int weaponFasion, int bodyFasion, int maxHp, int atk, int def, float hit, float dodge, float cri)
{
    // 返回，告知lua结果的个数
    lua_getglobal(m_Lua_context,"script_inCall_canPlayerEnter");

    // 传递场景id
    lua_pushnumber(m_Lua_context, mInstId);
    // 传递有关玩家的各个信息，让脚本判断
    lua_pushstring(m_Lua_context, roleName.c_str());
    lua_pushnumber(m_Lua_context, lvl);
    lua_pushnumber(m_Lua_context, roleJob);
    lua_pushnumber(m_Lua_context, vipLvl);
    lua_pushnumber(m_Lua_context, honorId);
    lua_pushstring(m_Lua_context, guildName.c_str());
    lua_pushnumber(m_Lua_context, guildPosition);
    lua_pushnumber(m_Lua_context, weaponFasion);
    lua_pushnumber(m_Lua_context, bodyFasion);
    lua_pushnumber(m_Lua_context, maxHp);
    lua_pushnumber(m_Lua_context, atk);
    lua_pushnumber(m_Lua_context, def);
    lua_pushnumber(m_Lua_context, hit);
    lua_pushnumber(m_Lua_context, dodge);
    lua_pushnumber(m_Lua_context, cri);

    //运行函数并把结果压入栈
    int s = lua_pcall(m_Lua_context,16,1,0);
    if(s)
    {
        log_info(lua_tostring(m_Lua_context, -1));
        return false;
    }
    
    bool ret = (bool)lua_toboolean(m_Lua_context, -1);
    lua_pop(m_Lua_context, 1);
    
    return ret;
}

// [function script_inCall_addPlayer( sceneId, roleInstId, playerObj )]
bool LuaScene::lua_addPlayer( WorldRole * player )
{
    if( player == NULL )
        return false;
    
    // 返回，告知lua结果的个数
    lua_getglobal(m_Lua_context,"script_inCall_addPlayer");
    
    // 传递场景id
    lua_pushnumber(m_Lua_context, mInstId);

    // 传递玩家的角色Id,WorldPlayer就是用角色id来唯一代表自己的
    lua_pushnumber(m_Lua_context, player->getInstId());

    // 传递玩家对象给脚本，让脚本保存，以后可以利用它来执行各种核心功能调用
    WorldRole ** pObj = (WorldRole **)lua_newuserdata(m_Lua_context, sizeof(WorldRole *));
    *pObj = player;

    //运行函数并把结果压入栈,3个参数
    int s = lua_pcall(m_Lua_context,3,1,0);
    if(s)
    {
        log_info(lua_tostring(m_Lua_context, -1));
        return false;
    }
    
    bool ret = (int)lua_toboolean(m_Lua_context, -1);
    lua_pop(m_Lua_context, 1);
    
    return ret;
}

// [function script_inCall_addPet( sceneId, petInstId, petObj, ownerRoleInstId )]
bool LuaScene::lua_addPet( WorldPet * pet )
{
    if( pet == NULL || pet->mPlayer == NULL )
        return false;
    
    // 返回，告知lua结果的个数
    lua_getglobal(m_Lua_context,"script_inCall_addPet");
    
    // 传递场景id
    lua_pushnumber(m_Lua_context, mInstId);
    
    // 传递宠物场景id
    lua_pushnumber(m_Lua_context, pet->getInstId());
    
    // 传递玩家对象给脚本，让脚本保存，以后可以利用它来执行各种核心功能调用
    WorldPet ** pObj = (WorldPet **)lua_newuserdata(m_Lua_context, sizeof(WorldPet *));
    *pObj = pet;
    
    // 传递主人的玩家副本id
    int ownerInstId = pet->mPlayer->getRole()->getInstId();
    lua_pushnumber(m_Lua_context, ownerInstId);
    
    //运行函数并把结果压入栈，4个参数
    int s = lua_pcall(m_Lua_context,4,1,0);
    if(s)
    {
        log_info(lua_tostring(m_Lua_context, -1));
        return false;
    }
    
    bool ret = (int)lua_toboolean(m_Lua_context, -1);
    lua_pop(m_Lua_context, 1);
    
    return ret;
}

void LuaScene::onAddMonster(WorldMonster *monster)
{
    WorldCreatureAI* ai = WorldCreatureAI::create(monster, "luascript");
    monster->setAI(ai);
    monster->setGroup(eCreatureMonsterGroup);
    
    //if (mSubState == eWorldSceneTeamCopyBattleStart) {
    monster->startAI();
    //}
}

void LuaScene::onCreatureDead(WorldCreature* victim, WorldCreature* attacker)
{
    lua_onCreatureDead(victim, attacker);
}

void LuaScene::lua_onCreatureDead(WorldCreature* victim, WorldCreature* attacker)
{
    if( victim == NULL || victim->mScene != this || attacker == NULL || attacker->mScene != this )
        return;
    
    // 返回，告知lua结果的个数
    lua_getglobal(m_Lua_context,"script_inCall_onCreatureDead");
    
    // 传递场景id
    lua_pushnumber(m_Lua_context, mInstId);
    
    // 传递牺牲者的InstId
    lua_pushnumber(m_Lua_context, victim->getInstId());
    
    // 传递杀手的Instid
    lua_pushnumber(m_Lua_context, attacker->getInstId());
    
    //运行函数并把结果压入栈，3个参数
    lua_pcall(m_Lua_context,3,1,0);
    
    // bool ret = (int)lua_toboolean(m_Lua_context, -1);
    // lua_pop(m_Lua_context, 1);
    // return ret;
}

void LuaScene::beforePlayerLeave(WorldPlayer* player)
{
    lua_onPlayerLeave(player);
}

void LuaScene::lua_onPlayerLeave(WorldPlayer *player)
{
    if( player == NULL || player->getRole()->mScene != this )
        return;
    
    // 返回，告知lua结果的个数
    lua_getglobal(m_Lua_context,"script_inCall_onPlayerLeave");
    
    // 传递场景id
    lua_pushnumber(m_Lua_context, mInstId);
    
    // 传递玩家的InstId
    lua_pushnumber(m_Lua_context, player->getRole()->getInstId());
    
    //运行函数并把结果压入栈，2个参数
    lua_pcall(m_Lua_context,2,1,0);
    
    // bool ret = (int)lua_toboolean(m_Lua_context, -1);
    // lua_pop(m_Lua_context, 1);
    // return ret;
}

bool LuaScene::lua_isEnemyTarget( WorldCreature * pCreature, WorldCreature * pTarget )
{
    if( pCreature == NULL || pCreature->mScene != this || pTarget == NULL || pTarget->mScene != this )
        return false;
    
    // 返回，告知lua结果的个数
    lua_getglobal(m_Lua_context,"script_inCall_isEnemyTarget");
    
    // 传递场景id
    lua_pushnumber(m_Lua_context, mInstId);
    
    // 传递发起者的InstId
    lua_pushnumber(m_Lua_context, pCreature->getInstId());

    // 传递目标的InstId
    lua_pushnumber(m_Lua_context, pTarget->getInstId());

    //运行函数并把结果压入栈，2个参数
    int s = lua_pcall(m_Lua_context,3,1,0);
    if(s)
    {
        log_info(lua_tostring(m_Lua_context,-1));
        return false;
    }
    
    bool ret = lua_toboolean(m_Lua_context, -1);
    lua_pop(m_Lua_context, 1);
    return ret;
}

int LuaScene::lua_getPetCtrlMode()
{
    // 返回，告知lua结果的个数
    lua_getglobal(m_Lua_context,"script_inCall_getPetCtrlMode");
    
    // 传递场景id
    lua_pushnumber(m_Lua_context, mInstId);
    
    //运行函数并把结果压入栈，1个参数
    int s = lua_pcall(m_Lua_context,1,1,0);
    if(s)
    {
        log_info(lua_tostring(m_Lua_context,-1));
        return false;
    }
    
    int ret = (int)lua_tonumber(m_Lua_context, -1);
    lua_pop(m_Lua_context, 1);
    return ret;
}

/**
 
 定义怪物对象及其各个方法
 
 */

handler_lua_monster(inIdleState){
    bool ret = pMonster->getState() == eIDLE;
    lua_pushboolean(context, ret);
    return 1;
}}

handler_lua_monster(roam){
    // （在没有目标的情况下）随机漫步
    // Patrol是来回于A,B两点巡逻
    // pMonster->MonsterPatrol();
    pMonster->MonsterHangAround();
    return 0;
}}

handler_lua_monster(follow){
    // （在没有目标的情况下）随机漫步
    // Patrol是来回于A,B两点巡逻
    // pMonster->MonsterPatrol();
    LivingObject * pTarget = *(LivingObject **)lua_touserdata(context, 2);
    pMonster->MonsterFallow(pTarget);
    return 0;
}}

handler_lua_monster(targetInSight){
    // todo: 根据怪物当前所处场景判断，怪物的目标是否再它视线范围之内
    LivingObject * pTarget = *(LivingObject **)lua_touserdata(context, 2);
    if( pMonster->mScene != pTarget->mScene ){
        // 返回结果，false
        lua_pushboolean(context, false);
        return 1;
    }

    // todo：判断
    bool ret = pMonster->TargetInSight( pTarget );
    
    // 返回结果
    lua_pushboolean(context, ret);
    return 1;
}}

handler_lua_monster(getLastHitTimeSpan){
    // todo: 距离上一次攻击的时间间隔
    int timespan = time(NULL)-pMonster->getLastAttackTime();
    // 返回结果
    lua_pushnumber(context, timespan);
    return 1;
}}

handler_lua_monster(normalAttackTarget){
    // todo: 普通攻击某目标
    LivingObject * pTarget = *(LivingObject **)lua_touserdata(context, 2);
    // 判断目标是否在自己的攻击范伟内，如果在则进攻
    if (pMonster->MonsterCanAtk(pTarget))
    {
        pMonster->stand();
        pMonster->FixAtkOrient(pTarget);
            
        pMonster->attack(100,1);
        //monster->attack(100, 1);
        
        // 1表示要返回一个参数，bool型
        // bool ret = (int)lua_toboolean(m_Lua_context, -1);
        // lua_pop(m_Lua_context, 1);
        // return ret;
        lua_pushboolean(context, true);
        return 1;
    }
    lua_pushboolean(context, false);
    return 1;
}}

handler_lua_monster(skillAttackTarget){
    // todo: 技能攻击某目标
    LivingObject * pTarget = *(LivingObject **)lua_touserdata(context, 2);
    int ret = pMonster->MonsterCastSkill( pTarget );
    
    if( ret > 0 )
        lua_pushboolean(context, true);
    else
        lua_pushboolean(context, false);
    return 1;
}}

handler_lua_monster(walkRoutine){
    // todo: 行走到某个点
    int toX = lua_tonumber(context, 2);
    int toY = lua_tonumber(context, 3);
    
    if (!pMonster->canChangeToState(eWALK)) {
        lua_pushboolean(context, false);
        return 1;
    }
    
    if (!pMonster->getSpeed()) {
        lua_pushboolean(context, false);
        return 1;
    }
    
    CCPoint newPoint = ccp(toX, toY);
    if ( pMonster->IsPointOutOfArea(newPoint) )
    {
        lua_pushboolean(context, false);
        return 1;
    }
    
    if( pMonster->walkTo(newPoint) <= 0 )
    {
        lua_pushboolean(context, false);
        return 1;
    }

    lua_pushboolean(context, true);
    return 1;
}}

handler_lua_monster(patrolAround){
    // todo: 围绕某个LivingObject巡航
    LivingObject * pTarget = *(LivingObject **)lua_touserdata(context, 2);
    pMonster->MonsterTrack(pTarget, pMonster->getMonsterType());
    
    // 无参数返回
    return 0;
}}

handler_lua_monster(pursuitTarget){
    // todo: 技能攻击某目标
    LivingObject * pTarget = *(LivingObject **)lua_touserdata(context, 2);
    pMonster->MonsterTrack(pTarget, pMonster->getMonsterType());
    
    // 无参数返回
    return 0;
}}

handler_lua_monster(getAtk){
    // 返回格式 [int]
    if( pMonster == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int atk = pMonster->mAtk;
    lua_pushnumber( context, atk );
    return 1;
}}

handler_lua_monster(getDef){
    // 返回格式 [int]
    if( pMonster == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int def = pMonster->mDef;
    lua_pushnumber( context, def );
    return 1;
}}

handler_lua_monster(getHit){
    // 返回格式 [int]
    if( pMonster == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int hit = pMonster->mHit;
    lua_pushnumber( context, hit );
    return 1;
}}

handler_lua_monster(getDodge){
    // 返回格式 [int]
    if( pMonster == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int dodge = pMonster->mDodge;
    lua_pushnumber( context, dodge );
    return 1;
}}



handler_lua_monster(getCri){
    // 返回格式 [int]
    if( pMonster == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int cri = pMonster->mCri;
    lua_pushnumber( context, cri );
    return 1;
}}

handler_lua_monster(getMaxHp){
    // 返回格式 [int]
    if( pMonster == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    // 当前血量是战斗属性，而不是角色属性
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int hp = pMonster->getMaxHp();
    lua_pushnumber( context, hp );
    return 1;
}}

handler_lua_monster(getHp){
    // 返回格式 [int]
    if( pMonster == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    // 当前血量是战斗属性，而不是角色属性
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int hp = pMonster->getHp();
    lua_pushnumber( context, hp );
    return 1;
}}

handler_lua_monster(getLevel){
    // 返回格式 [int]
    if( pMonster == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    // 当前血量是战斗属性，而不是角色属性
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int level = pMonster->getLvl();
    lua_pushnumber( context, level );
    return 1;
}}

handler_lua_monster(getMoveSpeed){
    // 返回格式 [int]
    if( pMonster == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    // 当前血量是战斗属性，而不是角色属性
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int movespeed = pMonster->getMoveSpeed();
    lua_pushnumber( context, movespeed );
    return 1;
}}

// 脚本调用它让怪物对象自行销毁掉，不需要再通知或者控制到脚本中的对象，从这一刻起，脚本就跟其脱离关系了
handler_lua_monster(invalidSelf){
    // 返回格式 [void]
    if( pMonster == NULL )
    {
        return 0;
    }
    pMonster->setValid(false);
    return 0;
}}

static luaL_Reg Object_Monster [] = {
    {"patrolAround", CfLua_monster_patrolAround},
    {"pursuitTarget", CfLua_monster_pursuitTarget},
    {"walkRoutine", CfLua_monster_walkRoutine},
    {"skillAttackTarget", CfLua_monster_skillAttackTarget},
    {"normalAttackTarget", CfLua_monster_normalAttackTarget},
    {"getLastHitTimeSpan", CfLua_monster_getLastHitTimeSpan},
    {"targetInSight", CfLua_monster_targetInSight},
    {"roam", CfLua_monster_roam},
    {"inIdleState", CfLua_monster_inIdleState},
    {"getAtk", CfLua_monster_getAtk},
    {"getDef", CfLua_monster_getDef},
    {"getHit", CfLua_monster_getHit},
    {"getDodge", CfLua_monster_getDodge},
    {"getMaxHp", CfLua_monster_getMaxHp},
    {"getHp", CfLua_monster_getHp},
    {"getCri", CfLua_monster_getCri},
    {"getMoveSpeed", CfLua_monster_getMoveSpeed},
    {"getLevel", CfLua_monster_getLevel},
    {"invalidSelf", CfLua_monster_invalidSelf},
    {"follow", CfLua_monster_follow},
    {NULL, NULL}
};

LUALIB_API int luaopen_Monster (lua_State * context ) {
    // 将 Object_Monster 注册成为 Lua 中的 Monster 类
    //luqL_register(L,"Monster", Object_Monster);
    //luaL_openlib(context, "Monster", Object_Monster, 0);
    
    //lua_newtable(context);
    /*
    luaL_newlibtable(context, Object_Monster);
    luaL_setfuncs(context, Object_Monster, 0);
    lua_setglobal(context, "CCoreImpl_Monster");
     */
    luaL_newlib(context, Object_Monster);
    return 1;
}


/**
 
 定义场景对象及其各个方法
 
 */

handler_lua_scene(getCurrentTime){
    int current = time(NULL);
    // 返回结果
    lua_pushnumber(context, current);
    return 1;
}}


handler_lua_scene(closeScene){
    // 返回格式 [bool]
    pScene->setValid(false);

    lua_pushboolean(context, true);
    return 1;
}}

handler_lua_scene(setPosition){
    // 返回格式 [bool]
    LivingObject * pTarget = *(LivingObject **)lua_touserdata(context, 2);
    int posx = lua_tonumber(context, 3);
    int posy = lua_tonumber(context, 4);
    
    //if( pTarget->mScene->getInstId() != pScene->getInstId() )
    //{
    //    lua_pushboolean(context, false);
    //    return 1;
    //}
    
    CCPoint pos = CCPoint(posx, posy);
    pTarget->setPosition(pos);
    pTarget->SceneObject::setSpeed(Vector2(0,0));
    
    lua_pushboolean(context, true);
    return 1;
}}


handler_lua_scene(getPosition){
    // 返回格式 [posx, posy]
    LivingObject * pTarget = *(LivingObject **)lua_touserdata(context, 2);
    if( pTarget == NULL )
    {
        lua_pushnumber(context, -1);
        lua_pushnumber(context, -1);
        return 2;
    }
    
    lua_pushnumber(context, pTarget->getPositionX());
    lua_pushnumber(context, pTarget->getPositionY());
    return 2;
}}

handler_lua_scene(creatureIsDead){
    // 返回格式 [posx, posy]
    LivingObject * pTarget = *(LivingObject **)lua_touserdata(context, 2);
    if( pTarget == NULL || pTarget->mScene != pScene )
    {
        lua_pushboolean(context, true);
        return 1;
    }
    if( pTarget->mState == eDEAD )
    {
        lua_pushboolean(context, true);
        return 1;
    }
    lua_pushboolean(context, false);
    return 1;
}}

obj_worldnpc_info getWorldNpcInfo(WorldMonster* robot);
obj_pos_info getSceneObjectPosInfo(SceneObject* obj);
handler_lua_scene(createMonster){
    // 返回格式 ［object, instId］
    int monId = lua_tonumber( context, 2 );
    int posx = lua_tonumber( context, 3 );
    int posy = lua_tonumber( context, 4 );
    bool bNotify = lua_toboolean(context, 5);
    
    WorldMonster * monster = pScene->createMonster(monId, posx, posy);
    if( monster == NULL )
    {
        // 传递玩家对象给脚本，让脚本保存，以后可以利用它来执行各种核心功能调用
        WorldMonster ** pObj = (WorldMonster **)lua_newuserdata(context, sizeof(WorldMonster *));
        *pObj = NULL;
        
        lua_pushnumber(context, -1);
        return 2;
    }
    
    //monster->enterScene(pScene);
    //CCPoint pos = CCPoint(posx, posy);
    //monster->setPosition(pos);
    //monster->setOrient(eLeft);
    
    if(bNotify)
    {
        // 对于AddMonster 我们需要发送消息通告客户端
        notify_worldnpc_enter_scene noti;
        obj_pos_info posinfo = getSceneObjectPosInfo(monster);
        obj_worldnpc_info monsterinfo = getWorldNpcInfo(monster);
        noti.npcinfos.push_back(monsterinfo);
        noti.posinfos.push_back(posinfo);
        pScene->broadcastPacket(&noti);
    }
    
    WorldMonster ** pObj = (WorldMonster **)lua_newuserdata(context, sizeof(WorldMonster *));
    *pObj = monster;
    lua_pushnumber(context, monster->getInstId());
    return 2;
}}


// 脚本通知释放怪物，这里的处理跟 Monster:invalidSelf 是一样的
handler_lua_scene(releaseMonster){
    // 返回格式 ［object, instId］
    WorldMonster * pMonster = *(WorldMonster **)lua_touserdata(context, 2);
    if( pMonster == NULL )
    {
        return 0;
    }
    pMonster->setValid(false);
    return 0;
}}



// 场景让核心层踢掉某玩家，这里调用remove, remove内部会继续回调脚本处理脚本的清除工作
handler_lua_scene(kickPlayer){
    // 返回格式 [bool]
    WorldPlayer * pPlayer = *(WorldPlayer **)lua_touserdata(context, 2);
    if( pPlayer == NULL )
    {
        lua_pushboolean(context, false);
        return 1;
    }

    
    //player->leaveScene();
    //强制该玩家离开，removePlayer会广播通告整个场景
    pScene->removePlayer(pPlayer);
    
    // 根据 WorldScene::ClearAll 的逻辑，我们还需要给这个玩家发送一个leaveRet消息，
    // 让它自己正常退出，就好像是自己请求的一样
    create_cmd(LeaveWorldSceneRet, leaveWorldSceneRet);
    leaveWorldSceneRet->ret = 0;
    leaveWorldSceneRet->sessionId = pPlayer->getSessionId();
    leaveWorldSceneRet->roleId = pPlayer->getRole()->mRoleInfo.mRoleId;
    sendMessageToGame(0, CMD_MSG, leaveWorldSceneRet, 0);
    
    //还要将玩家对象从全局管理器中注销
    g_WorldPlayerMgr.destroyPlayer(pPlayer);
        
    lua_pushboolean(context, true);
    return 1;
}}

/*
handler_lua_scene(kickMonster){
    // 返回格式 [bool]
    WorldMonster * pMonster = *(WorldMonster **)lua_touserdata(context, 2);
    if( pMonster == NULL )
    {
        lua_pushboolean(context, false);
        return 1;
    }
    
    pMonster->setValid(false);
    //pScene->removeMonster(pPlayer);
    lua_pushboolean(context, true);
    return 1;
}
*/

/*
handler_lua_scene(searchTarget){
    // 返回格式 [targetType, targetInstId]
    WorldCreature * originater = *(WorldCreature **)lua_touserdata(context, 2);
    
    vector<WorldCreature*>  objs = pScene->getAreaCanAttackObjs(originater, originater->getType());
    
    LivingObject* obj = getNearestObj(objs);
    
    WorldPlayer * pPlayer = *(WorldPlayer **)lua_touserdata(context, 2);
    if( pPlayer == NULL )
    {
        lua_pushboolean(context, false);
        return 1;
    }
    
    pScene->removePlayer(pPlayer);
    lua_pushboolean(context, true);
    return 1;
}}
*/

// 场景
static luaL_Reg Object_Scene [] = {
    //{"debug", CfLua_scene_debug},
    {"getCurrentTime", CfLua_scene_getCurrentTime},
    {"closeScene", CfLua_scene_closeScene},
    {"createMonster", CfLua_scene_createMonster},
    {"releaseMonster", CfLua_scene_releaseMonster},
    {"setPosition", CfLua_scene_setPosition},
    {"getPosition", CfLua_scene_getPosition},
    {"kickPlayer", CfLua_scene_kickPlayer},
    {"creatureIsDead", CfLua_scene_creatureIsDead},
    {NULL, NULL}
};


LUALIB_API int luaopen_Scene (lua_State * context ) {
    // 将 Object_Scene 注册成为 Lua 中的 Scene 类
    // luaL_register(context, "Scene", Object_Scene);
    /*
    //lua_newtable(context);
    luaL_newlibtable(context, Object_Scene);
    luaL_setfuncs(context, Object_Scene, 0);
    lua_setglobal(context, "CCoreImpl_Scene");
     */
    luaL_newlib(context, Object_Scene);
    //createmetatable(context);

    return 1;
}




/**
 
 定义玩家对象及其各个方法
 
 */

handler_lua_player(getName){
    // 返回格式 [string]
    if( pPlayer == NULL )
    {
        lua_pushstring(context, "");
        return 1;
    }
    
    lua_pushstring( context, pPlayer->getRole()->mRoleInfo.mRoleName.c_str() );
    return 1;
}}

handler_lua_player(getSex){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    ObjSex sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    //mJob = RoleType2Job(pPlayer->getRole()->mRoleInfo.mRoleType);
    lua_pushnumber( context, sex );
    return 1;
}}

handler_lua_player(getJob){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //ObjSex sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    ObjJob job = RoleType2Job(pPlayer->getRole()->mRoleInfo.mRoleType);
    lua_pushnumber( context, job );
    return 1;
}}

handler_lua_player(getVipLvl){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int viplvl = pPlayer->getRole()->mRoleInfo.mVipLvl;
    lua_pushnumber( context, viplvl );
    return 1;
}}

handler_lua_player(getGuildName){
    // 返回格式 [string]
    if( pPlayer == NULL )
    {
        lua_pushstring(context, "");
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    string guildName = pPlayer->getRole()->mRoleInfo.mGuildName;
    lua_pushstring( context, guildName.c_str() );
    return 1;
}}

handler_lua_player(getGuildPosition){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int title = pPlayer->getRole()->mRoleInfo.mGuildPosition;
    lua_pushnumber( context, title );
    return 1;
}}

handler_lua_player(getAtk){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int atk = pPlayer->getRole()->BattleProp::getAtk();
    lua_pushnumber( context, atk );
    return 1;
}}

handler_lua_player(getDef){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int def = pPlayer->getRole()->BattleProp::getDef();
    lua_pushnumber( context, def );
    return 1;
}}

handler_lua_player(getHit){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int hit = pPlayer->getRole()->BattleProp::getHit();
    lua_pushnumber( context, hit );
    return 1;
}}

handler_lua_player(getDodge){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int dodge = pPlayer->getRole()->BattleProp::getDodge();
    lua_pushnumber( context, dodge );
    return 1;
}}

handler_lua_player(getMaxHp){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int maxHp = pPlayer->getRole()->BattleProp::getMaxHp();
    lua_pushnumber( context, maxHp );
    return 1;
}}

handler_lua_player(getCri){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int cri = pPlayer->getRole()->BattleProp::getCri();
    lua_pushnumber( context, cri );
    return 1;
}}

handler_lua_player(getHp){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    // 当前血量是战斗属性，而不是角色属性
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int hp = pPlayer->getRole()->BattleProp::getHp();
    lua_pushnumber( context, hp );
    return 1;
}}

handler_lua_player(getMoveSpeed){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    // 当前血量是战斗属性，而不是角色属性
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int speed = pPlayer->getRole()->BattleProp::getMoveSpeed();
    lua_pushnumber( context, speed );
    return 1;
}}

handler_lua_player(getLevel){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushnumber(context, 0);
        return 1;
    }
    
    // 当前血量是战斗属性，而不是角色属性
    //int sex = (ObjSex)((pPlayer->getRole()->mRoleInfo.mRoleType - 1) / 3);
    int level = pPlayer->getRole()->BattleProp::getLvl();
    lua_pushnumber( context, level );
    return 1;
}}

handler_lua_player(hasItem){
    // 返回格式 [int]
    if( pPlayer == NULL )
    {
        lua_pushboolean(context, false);
        return 1;
    }
    
    // 当前血量是战斗属性，而不是角色属性
    lua_pushboolean(context, true);
    return 1;
}}

static luaL_Reg Object_Player [] = {
    {"getName", CfLua_player_getName},
    {"getSex", CfLua_player_getSex},
    {"getJob", CfLua_player_getJob},
    {"getVipLvl", CfLua_player_getVipLvl},
    {"getGuildName", CfLua_player_getGuildName},
    {"getGuildPosition", CfLua_player_getGuildPosition},
    {"getAtk", CfLua_player_getAtk},
    {"getDef", CfLua_player_getDef},
    {"getHit", CfLua_player_getHit},
    {"getDodge", CfLua_player_getDodge},
    {"getMaxHp", CfLua_player_getMaxHp},
    {"getHp", CfLua_player_getHp},
    {"getCri", CfLua_player_getCri},
    {"getMoveSpeed", CfLua_player_getMoveSpeed},
    {"getLevel", CfLua_player_getLevel},
    {"hasItem", CfLua_player_hasItem},
    {NULL, NULL}
};


LUALIB_API int luaopen_Player (lua_State * context ) {
    // 将 Object_Scene 注册成为 Lua 中的 Scene 类
    // luaL_register(context, "Scene", Object_Scene);
    
    /*
    //lua_newtable(context);
    luaL_newlibtable(context, Object_Monster);
    luaL_setfuncs(context, Object_Player, 0);
    lua_setglobal(context, "CCoreImpl_Player");
    */
    luaL_newlib(context, Object_Player);
    //createmetatable(context);

    return 1;
}