//
//  cmd_world_handler.cpp
//  GameSrv
//
//  Created by prcv on 14-3-10.
//
//


#include "world_cmdmsg.h"
#include "WorldSceneMgr.h"
#include "WorldRole.h"
#include "WorldPet.h"
#include "WorldPlayer.h"

#include "msg.h"
#include "main.h"
#include "Role.h"
#include "basedef.h"

#include "RealPvpMgr.h"
#include "TreasureFight.h"
#include "EnumDef.h"
#include "hander_include.h"
#include "WorldMonster.h"
#include "WorldSceneScript.h"

#include "SyncTeamCopy.h"

obj_pos_info getSceneObjectPosInfo(SceneObject* obj);
obj_worldrole_info getRoleInfo(WorldRole* role);
obj_worldpet_info getPetInfo(WorldPet* pet);

handler_cmd(CreateWorldSceneRet)
{
    for (int i = 0; i < roleIds.size(); i++)
    {
        Role* role = SRoleMgr.GetRole(roleIds[i]);
        CheckCondition(role, continue)
        
        if (roleRets[i] != 0)
        {
            role->setCurrSceneId(role->getSceneID());
            role->setWorldState(eWorldStateNone);
        }
        else
        {
            role->setPreEnterSceneid(sceneMod);
            role->setWorldState(eWorldStateInWorld);
        }
        
        ack_enter_world_scene ack;
        ack.errorcode = roleRets[i];
        ack.sceneMod = sceneMod;
        ack.sceneId = sceneInstId;
        sendNetPacket(role->getSessionId(), &ack);
    }
    
    SceneCfgDef* sceneCfg = SceneCfg::getCfg(this->sceneMod);
    
    if (sceneCfg) {
        if (sceneCfg->sceneType == stTreasureFight) {
            
            int copyNum = Utils::safe_atoi(this->extendStr.c_str());
            
            g_GuildTreasureFightMgr.onCreateTreasureCopy(copyNum, this->sceneInstId);
        }
        
        if (sceneCfg->sceneType == stSyncTeamCopy) {
            
            Json::Reader jsonreader;
            Json::Value jsonval;
            jsonreader.parse(this->extendStr, jsonval);
            int teamid = jsonval["team_id"].asInt();
            
            g_SyncFightingTeamMgr.onTeamEnterSceneToFight(teamid, this->sceneInstId);
        }
    }
}

handler_cmd(EnterWorldSceneRet)
{
    for (int i = 0; i < roleIds.size(); i++)
    {
        Role* role = SRoleMgr.GetRole(roleIds[i]);
        CheckCondition(role, continue)
        
        if (roleRets[i] != 0)
        {
            role->setCurrSceneId(role->getSceneID());
            role->setWorldState(eWorldStateNone);
            role->removeBattleBuff();
        }
        else
        {
            role->setPreEnterSceneid(sceneMod);
            role->setWorldState(eWorldStateInWorld);
        }
        
        if (checkCurrSceneType(this->sceneMod, stTreasureFight)) {
            
            if (roleRets[i]) {
                
                //如果进入失败，把玩家提出管理器
                g_GuildTreasureFightMgr.leaveTreasureCopy(role, this->sceneInstId, false);
            }
        }
        
        ack_enter_world_scene ack;
        ack.errorcode = roleRets[i];
        ack.sceneMod = sceneMod;
        ack.sceneId = sceneInstId;
        sendNetPacket(role->getSessionId(), &ack);
    }
}

handler_cmd(QuickEnterWorldSceneRet)
{
    Role* role = SRoleMgr.GetRole(roleId);
    CheckCondition(role, return)
    
    if (sceneInstId == 0)
    {
        role->setCurrSceneId(role->getSceneID());
        role->setWorldState(eWorldStateNone);
    }
    else
    {
        role->setPreEnterSceneid(sceneMod);
        role->setWorldState(eWorldStateInWorld);
    }
    
    ack_enter_world_scene ack;
    ack.errorcode = 0;
    ack.sceneMod = sceneMod;
    ack.sceneId = sceneInstId;
    sendNetPacket(role->getSessionId(), &ack);

}

handler_cmd(LeaveWorldSceneRet)
{
    //角色退出游戏不处理
    if (reason == eLeaveWorldSceneLeaveWorld) {
        return;
    }
    
    Role* role = SRoleMgr.GetRole(roleId);
    CheckCondition(role, return)
    
//    role->setCurrSceneId(role->getSceneID());
    role->backToCity();
    role->setWorldState(eWorldStateNone);
    
    SceneCfgDef* scene = SceneCfg::getCfg(this->sceneMod);
    if (scene) {
        
        //如果是工会宝藏战
        if (scene->sceneType == stTreasureFight) {
            g_GuildTreasureFightMgr.leaveTreasureCopy(role, this->sceneId,isDead);
        }
        
        //同步组队副本
        if (scene->sceneType == stSyncTeamCopy) {
            
            FightingTeam* myteam = g_SyncFightingTeamMgr.findTeam(role->getSyncTeamId());
            
            if (myteam && myteam->mSceneInstId) {
                g_SyncFightingTeamMgr.leaveTeam(role, role->getSyncTeamId());
            }
            
        }
    }
    
//    role->removeBattleBuff();
//    Pet* pet = role->mPetMgr->getActivePet();
//    if (pet) {
//        role->sendPetPropAndVerifycode(pet);
//    }
    
    ack_leave_world_scene ack;
    sendNetPacket(role->getSessionId(), &ack);
}

handler_cmd(WorldPvpResult)
{
    string extendStr = this->extendStr;
    
    if (strcmp(extendStr.c_str(), "real_pvp") == 0) {
        g_RealpvpMgr.safe_receiveFightingResult(this->winner, this->loser);
        return;
    }
    
    if (strcmp(extendStr.c_str(), "duel") == 0) {
        
        Role* winner = SRoleMgr.GetRole(this->winner);
        Role* loser = SRoleMgr.GetRole(this->loser);
        
        if (winner) {
            notify_duel_result result;
            result.isWin = true;
            result.enemyName = winner->getDuelEnemyName();
            sendNetPacket(winner->getSessionId(), &result);
            
            winner->clearDuelInfo();
        }
        
        if (loser) {
            notify_duel_result result;
            result.isWin = false;
            result.enemyName = loser->getDuelEnemyName();
            sendNetPacket(loser->getSessionId(), &result);
            
            loser->clearDuelInfo();
        }
    }
}


handler_cmd(WorldTeamCopyResult)
{
    //result 1表示成功，2表示失败
    
    if (checkCurrSceneType(this->sceneMod, stSyncTeamCopy)) {
        g_SyncTeamCopyMgr.fightingResultHandle(this->teamId, this->result);
    }
    
    if (result == 1) {
        //发奖励
    } else {
        
    }
}

handler_cmd(DestroyWorldSceneRet)
{
    g_GuildTreasureFightMgr.onCloseTreasureCopy(this->sceneId);
}
