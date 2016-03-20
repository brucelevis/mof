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
#include "basedef.h"
#include "EnumDef.h"
#include "WorldMonster.h"
#include "WorldSceneScript.h"

obj_pos_info getSceneObjectPosInfo(SceneObject* obj);
obj_worldrole_info getRoleInfo(WorldRole* role);
obj_worldpet_info getPetInfo(WorldPet* pet);


handler_cmd(CreateWorldScene)
{
    create_cmd(CreateWorldSceneRet, createWorldSceneRet);
    createWorldSceneRet->sceneInstId = 0;
    createWorldSceneRet->sceneMod = sceneMod;
    createWorldSceneRet->extendStr = extendStr;
    
    WorldScene* scene = NULL;
    do
    {
        WorldSceneTemp* sceneTemp = g_WorldSceneMgr.findSceneTemp(sceneMod);
        if (sceneTemp == NULL)
        {
            sendMessageToGame(0, CMD_MSG, createWorldSceneRet, 0);
            return;
        }
        
        if (sceneTemp->getType() == stPublicScene)
        {
            scene = sceneTemp->getScene(1);
        }
        
        if (scene == NULL)
        {
            scene = sceneTemp->createScene(extendStr);
            if( scene == NULL )
            {
                sendMessageToGame(0, CMD_MSG, createWorldSceneRet, 0);
                return;
            }
        }
        
        createWorldSceneRet->sceneInstId = scene->getInstId();
        
    }
    while (0);
    
    if (scene && playerinfos.size() > 0)
    {
        vector<WorldRole*> successRoles;
        for (int i = 0; i < playerinfos.size(); i++)
        {
            WorldPlayerInfo playerInfo = playerinfos[i];
            WorldPlayer* player = g_WorldPlayerMgr.getPlayer(playerInfo.mSessionId);
            
            if (player)
            {
                log_info("delete player that already exist");
                if (player->getScene())
                {
                    player->getScene()->removePlayer(player);
                }
                g_WorldPlayerMgr.destroyPlayer(player);
            }
            
            // added by jianghan for 脚本逻辑
            if( scene->getConfig().sceneType == stScriptScene )
            {
                // -- 判断脚本是否允许玩家加入
                string roleName = playerInfo.mRoleInfo.mRoleName;
                int lvl = playerInfo.mRoleInfo.mLvl;
                int roleJob = RoleType2Job(playerInfo.mRoleInfo.mRoleType);
                int vipLvl =  playerInfo.mRoleInfo.mLvl;;
                int honorId = playerInfo.mRoleInfo.mHonorId;
                string guildName = playerInfo.mRoleInfo.mGuildName;
                int guildPosition = playerInfo.mRoleInfo.mGuildPosition;
                int weaponFasion = playerInfo.mRoleInfo.mWeaponFashion;
                int bodyFasion = playerInfo.mRoleInfo.mBodyFashion;
                
                int maxHp = playerInfo.mRoleInfo.mMaxHp;
                int atk = playerInfo.mRoleInfo.mAtk;
                int def = playerInfo.mRoleInfo.mDef;
                float hit = playerInfo.mRoleInfo.mHit;
                float dodge = playerInfo.mRoleInfo.mDodge;
                float cri = playerInfo.mRoleInfo.mCri;
                
                if( ((LuaScene *)scene)->lua_canPlayerEnter(roleName,
                                                            lvl,
                                                            roleJob,
                                                            vipLvl,
                                                            honorId,
                                                            guildName,
                                                            guildPosition,
                                                            weaponFasion,
                                                            bodyFasion,
                                                            maxHp,
                                                            atk,
                                                            def,
                                                            hit,
                                                            dodge,
                                                            cri) == false )
                    continue;
            }
            // add end
            
            player = g_WorldPlayerMgr.createPlayer(playerInfo);
            scene->addPlayer(player);
            createWorldSceneRet->roleRets.push_back(0);
            createWorldSceneRet->roleIds.push_back(playerinfos[i].mRoleInfo.mRoleId);
        }
        
        // added by jianghan for test
        if( scene->getSceneType() != stScriptScene )
        {
            // 表示场景是新建，如果==－1表示进入已存在的场景，是不增加怪物的
            
            //WorldMonster * monster = g_WorldMonsterMgr.createMonster(10004, 202);
            
            // 世界boss
            //WorldMonster * monster = g_WorldMonsterMgr.createMonster(17001, 258);
            
            // 加入一组小怪,这些怪物的删除操作g_WorldMonsterMgr.destroyMonster
            // 会在WorldScene::ClearAll()中调用
            //for( int k=0;k<4;k++ ){
            //    WorldMonster * monster = scene->createMonster(10004, 00+rand()%400, 1000+rand()%400);
            //monster->enterScene(scene);
            //monster->setPosition(ccp(800+rand()%400,1000+rand()%400));
            // m->setActArea(getArea(m->getPosition()));
            // m->MonsterAIStart();
            //monster->setOrient(eLeft);
            //schedule(schedule_selector(WorldMonster::updateState), getActionPeriod());
            //monster->MonsterAIStart();
            //}
        }
        // end add
    }
    
    sendMessageToGame(0, CMD_MSG, createWorldSceneRet, 0);
}



handler_cmd(EnterWorldScene)
{
    create_cmd(EnterWorldSceneRet, cmdRet);
    cmdRet->sceneInstId = sceneInstId;
    
    WorldScene* scene = (WorldScene*)g_WorldObjectMgr.findObject(sceneInstId);
    if (scene && playerinfos.size() > 0)
    {
        vector<WorldRole*> successRoles;
        for (int i = 0; i < playerinfos.size(); i++)
        {
            WorldPlayerInfo playerInfo = playerinfos[i];
            WorldPlayer* player = g_WorldPlayerMgr.getPlayer(playerInfo.mSessionId);
            
            if (player)
            {
                log_info("delete player that already exist");
                if (player->getScene())
                {
                    player->getScene()->removePlayer(player);
                }
                g_WorldPlayerMgr.destroyPlayer(player);
            }
            
            // added by jianghan for 脚本逻辑
            if( scene->getConfig().sceneType == stScriptScene )
            {
                // -- 判断脚本是否允许玩家加入
                string roleName = playerInfo.mRoleInfo.mRoleName;
                int lvl = playerInfo.mRoleInfo.mLvl;
                int roleJob = RoleType2Job(playerInfo.mRoleInfo.mRoleType);
                int vipLvl =  playerInfo.mRoleInfo.mLvl;;
                int honorId = playerInfo.mRoleInfo.mHonorId;
                string guildName = playerInfo.mRoleInfo.mGuildName;
                int guildPosition = playerInfo.mRoleInfo.mGuildPosition;
                int weaponFasion = playerInfo.mRoleInfo.mWeaponFashion;
                int bodyFasion = playerInfo.mRoleInfo.mBodyFashion;
                
                int maxHp = playerInfo.mRoleInfo.mMaxHp;
                int atk = playerInfo.mRoleInfo.mAtk;
                int def = playerInfo.mRoleInfo.mDef;
                float hit = playerInfo.mRoleInfo.mHit;
                float dodge = playerInfo.mRoleInfo.mDodge;
                float cri = playerInfo.mRoleInfo.mCri;
                
                if( ((LuaScene *)scene)->lua_canPlayerEnter(roleName,
                                                            lvl,
                                                            roleJob,
                                                            vipLvl,
                                                            honorId,
                                                            guildName,
                                                            guildPosition,
                                                            weaponFasion,
                                                            bodyFasion,
                                                            maxHp,
                                                            atk,
                                                            def,
                                                            hit,
                                                            dodge,
                                                            cri) == false )
                    continue;
            }
            // add end
            
            player = g_WorldPlayerMgr.createPlayer(playerInfo);
            scene->addPlayer(player);
            cmdRet->roleRets.push_back(0);
            cmdRet->roleIds.push_back(playerinfos[i].mRoleInfo.mRoleId);
        }
        
        cmdRet->sceneMod = scene->getSceneModId();
    }
    
    sendMessageToGame(0, CMD_MSG, cmdRet, 0);
}

handler_cmd(QuickEnterWorldScene)
{
    create_cmd(QuickEnterWorldSceneRet, cmdRet);
    cmdRet->sceneMod = sceneMod;
    cmdRet->roleId = playerinfo.mRoleId;
    cmdRet->sceneInstId = 0;
    
    WorldScene* scene = NULL;
    do
    {
        WorldSceneTemp* sceneTemp = g_WorldSceneMgr.findSceneTemp(sceneMod);
        if (sceneTemp == NULL)
        {
            break;
        }
        
        if (sceneTemp->getType() == stPublicScene) {
            scene = sceneTemp->getScene(1);
        } else {
            scene = sceneTemp->createScene("");
        }
    }
    while (0);
    
    if (scene)
    {
        WorldPlayer* player = g_WorldPlayerMgr.getPlayer(playerinfo.mSessionId);
        if (player)
        {
            log_info("delete player that already exist");
            if (player->getScene())
            {
                player->getScene()->removePlayer(player);
            }
            g_WorldPlayerMgr.destroyPlayer(player);
        }
        
        player = g_WorldPlayerMgr.createPlayer(playerinfo);
        scene->addPlayer(player);
        cmdRet->sceneInstId = scene->getInstId();
    }
    
    sendMessageToGame(0, CMD_MSG, cmdRet, 0);
}


handler_cmd(LeaveWorld)
{
    WorldPlayer* player = g_WorldPlayerMgr.getPlayer(sessionId);
    if (!player) {
        return;
    }
    
    if (player && player->getScene()) {
        WorldScene* scene = player->getScene();
        scene->removePlayer(player);
        
        create_cmd(LeaveWorldSceneRet, ret);
        ret->sceneId = scene->getInstId();
        ret->sceneMod = scene->getSceneModId();
        ret->ret = 0;
        ret->reason = eLeaveWorldSceneLeaveWorld;
        ret->roleId = player->mRoleId;
        ret->sessionId = player->getSessionId();
        ret->extendStr = scene->getExtendProp();
        sendMessageToGame(0, CMD_MSG, ret, 0);
    }
    
    if (player) {
        g_WorldPlayerMgr.destroyPlayer(player);
    }
}

handler_cmd(LeaveWorldScene)
{
    WorldPlayer* player = g_WorldPlayerMgr.getPlayer(sessionId);
    if (!player)
    {
        return;
    }
    
    WorldScene* scene = player->getScene();
    if (player && scene) {
        scene->removePlayer(player);
        
        create_cmd(LeaveWorldSceneRet, ret);
        ret->sceneId = scene->getInstId();
        ret->sceneMod = scene->getSceneModId();
        ret->ret = 0;
        ret->reason = eLeaveWorldSceneLeaveScene;
        ret->roleId = player->mRoleId;
        ret->sessionId = player->getSessionId();
        ret->extendStr = scene->getExtendProp();
        
        bool dead = false;
        if (player->getRole()) {
            dead = player->getRole()->isDead();
        }
        ret->isDead = dead;
        sendMessageToGame(0, CMD_MSG, ret, 0);
    }
    
    if (player) {
        g_WorldPlayerMgr.destroyPlayer(player);
    }
}


handler_cmd(DestroyWorldScene)
{
    create_cmd(DestroyWorldSceneRet, ret);
    WorldObject* object = g_WorldObjectMgr.findObject(sceneId);
    WorldScene* scene = NULL;
    if (object && object->getType() == eWorldObjectScene) {
        scene = dynamic_cast<WorldScene*>(object);
    }
    
    if (scene) {
        scene->pendEnd();
        ret->ret = 0;
    } else {
        ret->ret = 1;
    }
    ret->sceneId = sceneId;
    sendMessageToGame(0, CMD_MSG, ret, 0);
    
}
