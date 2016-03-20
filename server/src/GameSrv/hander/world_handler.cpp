//
//  world_handler.cpp
//  GameSrv
//
//  Created by prcv on 13-12-13.
//
//

#include "hander_include.h"
#include "World.h"
#include "WorldRole.h"
#include "WorldSceneMgr.h"
#include "WorldPlayer.h"
#include "WorldRetinue.h"
#include "WorldSceneTreasureFight.h"
#include <float.h>

#include "WorldSceneScript.h"

bool checkScenePlayer(int sessionId, WorldPlayer* &player, WorldScene*& scene)
{
    player = g_WorldPlayerMgr.getPlayer(sessionId);
    CheckCondition(player, return false);
    
    scene = player->getScene();
    CheckCondition(scene, return false);
    
    return true;
}

WorldCreature* getPlayerUnit(WorldPlayer* player, int instId)
{
    WorldCreature* creature = NULL;
    
    int instType = instId >> 24;
    switch (instType) {
        case eWorldObjectRole:
        {
            WorldRole* role = player->getRole();
            if (role->getInstId() == instId)
            {
                creature = role;
            }
            
            break;
        }
        case eWorldObjectPet:
        {
            WorldPet* pet = player->getPet(instId);
            if (pet)
            {
                creature = pet;
            }
            break;
        }
        case eWorldObjectRetinue:
        {
            WorldRetinue *retinue = player -> getRetinue();
            if(retinue){
                creature = retinue;
            }
            break;
        }
        default:
            break;
    }
    
    return creature;
}

#define CheckScenePlayer(sessionId, player, scene) \
    WorldPlayer* player = NULL; \
    WorldScene* scene = NULL;\
    if (!checkScenePlayer(sessionId, player, scene)) \
    {\
        return;\
    }

handler_msg(req_ping, req)
{
    ack_ping ack;
    ack.usec = req.usec;
    ack.sec = req.sec;
    sendNetPacket(sessionid, &ack);
}}

bool cbGetSceneTempSceneIds(void* traverseobj, void* param)
{
    CheckCondition(traverseobj, return true)
    
    WorldScene* scene = (WorldScene*)traverseobj;
    ack_get_scenes& ack = *((ack_get_scenes*)param);
    ack.scenes.push_back(scene->getInstId());
    
    return true;
}


handler_msg(req_get_scenes, req)
{
    ack_get_scenes ack;
    WorldSceneTemp* sceneTemp = g_WorldSceneMgr.findSceneTemp(req.sceneMod);
    if (sceneTemp)
    {
        sceneTemp->traverseScenes(cbGetSceneTempSceneIds, &ack);
    }
    sendNetPacket(sessionid, &ack);
}}

obj_pos_info getSceneObjectPosInfo(SceneObject* obj)
{
    obj_pos_info posinfo;
    posinfo.orientation = obj->getOrient();
    posinfo.instid = obj->getInstId();
    posinfo.posx = obj->getPositionX();
    posinfo.posy = obj->getPositionY();
    posinfo.speedx = obj->getSpeedX();
    posinfo.speedy = obj->getSpeedY();
    
    return posinfo;
}

obj_worldretinue_info getRetinueInfo(WorldRetinue *retinue)
{
    obj_worldretinue_info info;
    if(retinue){
        WorldRole* role = retinue -> mPlayer-> getRole();
        if (role)
        {
            info.ownerid = role->getInstId();
        }
        else
        {
            info.ownerid = 0;
        }

        info.retiunemod = retinue -> getRetinueInfo().mRetinuemod;
        info.group = retinue -> getGroup();
        info.lvl = retinue -> getRetinueInfo().mLvl;
    }
    return info;
}

obj_worldrole_info getRoleInfo(WorldRole* role)
{
    obj_worldrole_info info;
    
    info.roleid = role->mRoleInfo.mRoleId;
    info.rolename = role->mRoleInfo.mRoleName;
    info.roletype = role->mRoleInfo.mRoleType;
    info.lvl = role->mRoleInfo.mLvl;
    info.viplvl = role->mRoleInfo.mVipLvl;
    info.honorId = role->mRoleInfo.mHonorId;
    info.guildname = role->mRoleInfo.mGuildName;
    info.guildPosition = role->mRoleInfo.mGuildPosition;
    info.weaponquality = role->mRoleInfo.mWeaponQuality;
    info.bodyfashion = role->mRoleInfo.mBodyFashion;
    info.weaponfashion = role->mRoleInfo.mWeaponFashion;
    info.enchantid = role->mRoleInfo.mEnchantId;
    info.enchantlvl = role->mRoleInfo.mEnchantLvl;
    info.awakelvl = role->mRoleInfo.mAwakeLvl;
    info.maxhp = role->mMaxHp;
    info.hp = role->mHp;
    info.group = role->getGroup();
    
    return info;
}

obj_worldpet_info getPetInfo(WorldPet* pet)
{
    obj_worldpet_info petinfo;
    petinfo.petmod = 0;
    if (pet->mPetInfo.mPetId != 0)
    {
        petinfo.petmod = pet->mPetInfo.mPetMod;
        petinfo.maxhp = pet->getMaxHp();
        petinfo.hp = pet->getHp();
        petinfo.group = pet->getGroup();
        
        WorldRole* role = pet->mPlayer->getRole();
        if (role)
        {
            petinfo.ownerid = role->getInstId();
        }
        else
        {
            petinfo.ownerid = 0;
        }
    }
    
    return petinfo;
}

obj_worldnpc_info getWorldNpcInfo(WorldNPC* npc)
{
    obj_worldnpc_info npcInfo;
    npcInfo.npcmod = npc->getModId();
    npcInfo.hp = npc->getHp();
    npcInfo.maxhp = npc->getMaxHp();
    npcInfo.group = npc->getGroup();
    return npcInfo;
}

obj_worldnpc_info getWorldNpcInfo(WorldMonster* monster)
{
    obj_worldnpc_info npcInfo;
    npcInfo.npcmod = monster->getModId();
    npcInfo.hp = monster->getHp();
    npcInfo.maxhp = monster->getMaxHp();
    npcInfo.group = monster->getGroup();
    return npcInfo;
}

bool cbGetSceneObjects(void* traverseobj, void* param)
{
    CheckCondition(traverseobj, return true)
    
    ack_get_scene_objects& ack = *((ack_get_scene_objects*)param);
    SceneObject* obj = dynamic_cast<SceneObject*>((WorldObject*)traverseobj);
    if (!obj) return true;
    
    int instId = obj->getInstId();
    int instType = instId >> 24;
    
    obj_pos_info posinfo = getSceneObjectPosInfo(obj);
    switch (instType)
    {
        case eWorldObjectRole:
        {
            WorldRole* role = (WorldRole*)obj;
            
            obj_worldrole_info roleinfo = getRoleInfo(role);
            ack.roles.push_back(roleinfo);
            
            ack.roleposinfos.push_back(posinfo);
            
            break;
        }
        case eWorldObjectPet:
        {
            WorldPet* pet = (WorldPet*)obj;
            obj_worldpet_info petinfo = getPetInfo(pet);
            ack.pets.push_back(petinfo);
            ack.petposinfos.push_back(posinfo);
            break;
        }
        case eWorldObjectMonster:
        {
            WorldMonster* monster = dynamic_cast<WorldMonster*>(obj);
            assert(monster);
            obj_worldnpc_info npcInfo = getWorldNpcInfo(monster);
            ack.npcs.push_back(npcInfo);
            ack.npcposinfos.push_back(posinfo);
            break;
        }
        case eWorldObjectRetinue:
        {
            WorldRetinue *retinue = (WorldRetinue *)obj;
            obj_worldretinue_info retinueinfo = getRetinueInfo(retinue);
            ack.retinues.push_back(retinueinfo);
            ack.retinueposinfors.push_back(posinfo);
            break;
        }
         
        default:
            break;
    }
    
    return true;

}


handler_msg(req_get_scene_objects, req)
{
    CheckScenePlayer(sessionid, player, scene)
    
    ack_get_scene_objects ack;
    scene->traverseObjects(cbGetSceneObjects, &ack);
    
    // added by jianghan for 宠物的控制方
    if( scene->getSceneType() == stScriptScene ){
        ack.petctrlmode = ((LuaScene *)scene)->lua_getPetCtrlMode();
    }else{
        // 如果不是脚本场景，则沿用老路子,让客户端自行控制宠物
        ack.petctrlmode = eClientServerCtrl;
    }
    // end add
    
    //scene->traversePets(cbGetScenePets, &ack);
    
    sendNetPacket(sessionid, &ack);
    
    //角色可以开始接受同步消息
    player->setReady(true);
}}



bool checkValidMotionLatency(WorldCreature* role, float px, float py, float sx, float sy)
{
    float dx = px - role->getPositionX();
    float dy = py - role->getPositionY();
    
    bool xOk = false;
    bool yOk = false;
    
    xOk = (dx < 300.0f && dx > -300.0f);
    yOk = (dy < 300.0f && dy > -300.0f);
    
    return true;//xOk && yOk;
}

bool checkMotion(WorldCreature* role, float px, float py, float sx, float sy)
{
    bool ret = checkValidMotionLatency(role, px, py, sx, sy);
    if (ret){
        role->setPosition(CCPoint(px, py));
    }
    role->SceneObject::setSpeed(Vector2(sx, sy));
    
    if (sx > 0.000001){
        role->setOrient(eRight);
    }else if (sx < -0.000001){
        role->setOrient(eLeft);
    }
    
    return true;
}

handler_msg(req_sync_motion, req)
{
    CheckScenePlayer(sessionid, player, scene)
    WorldCreature* unit = getPlayerUnit(player, req.instid);
    CheckCondition(unit && unit->getScene() && !unit->isDead(), return)
    
    float pointDist = pointDistance(unit->getPosition(), CCPoint(req.posx, req.posy));
    if (pointDist > 200) {
        log_warn(unit->getInstId() << "req_sync_motion (" << unit->getPositionX() << " " <<
                 unit->getPositionY() << ")----(" << req.posx << " " << req.posy << ") " <<
                 pointDist << req.destx << ", " << req.desty);
    } else {
    }
    
    // added by jianghan for 如果该单元是服务器控制，那么这里就不接受客户端提交的动作
    if( unit->getCtrlMode() == eServerCtrl ){
        ack_sync_motion ack;
        ack.errorcode = CE_INVALID_OP_REQUEST;
        ack.posx = unit->getPositionX();
        ack.posy = unit->getPositionY();
        ack.speedx = unit->getSpeedX();
        ack.speedy = unit->getSpeedY();
        sendNetPacket(sessionid, &ack);
        return;
    }
    // end add
    
    if (isnan(req.destx) || isnan(req.desty)) {
        ack_sync_motion ack;
        ack.errorcode = CE_INVALID_DEST_COORD;
        ack.instid = req.instid;
        ack.posx = unit->getPositionX();
        ack.posy = unit->getPositionY();
        ack.speedx = unit->getSpeedX();
        ack.speedy = unit->getSpeedY();
        sendNetPacket(sessionid, &ack);
        return;
    }
//    MotionRequest mr;
//    mr.mSessionId = sessionid;
//    mr.mType = eWALK;
//    mr.mDest = CCPoint(req.destx, req.desty);
//    unit->mMotionRequestInFrame.push_back(mr);
//    return;
//
    if (unit->getState() == eSKILL && !unit->getIsSkillMoveState()) {
        
        return;
    }
        
    CCPoint dest(req.destx, req.desty);
    unit->setPosition(CCPoint(req.posx, req.posy));
    unit->addHistoryPosition(unit->getPosition());
    if (unit->walkTo(dest) == 0) {
        ack_sync_motion ack;
        ack.errorcode = CE_INVALID_OP_REQUEST;
        ack.instid = unit->getInstId();
        ack.posx = unit->getPositionX();
        ack.posy = unit->getPositionY();
        ack.speedx = unit->getSpeedX();
        ack.speedy = unit->getSpeedY();
        sendNetPacket(sessionid, &ack);
    }
}}

handler_msg(req_sync_stop, req)
{
    CheckScenePlayer(sessionid, player, scene);
    WorldCreature* unit = getPlayerUnit(player, req.instid);
    CheckCondition(unit && unit->getScene() && !unit->isDead(), return)
    
    float pointDist = pointDistance(unit->getPosition(), CCPoint(req.posx, req.posy));
    if (pointDist > 200)
    {
        log_warn(unit->getInstId() << "req_sync_stop (" << unit->getPositionX() << " " << unit->getPositionY() <<
                 ")----(" << req.posx << " " << req.posy << ") " << pointDist);
    }
    else
    {
        //log_info(unit->getInstId() << "req_sync_stop (" << unit->getPositionX() << " " << unit->getPositionY() << ")----(" << req.posx << " " << req.posy << ") " << pointDist);
    }
    
    // added by jianghan for 如果该单元是服务器控制，那么这里就不接受客户端提交的动作
    if( unit->getCtrlMode() == eServerCtrl ){
        ack_sync_stop ack;
        ack.errorcode = CE_INVALID_OP_REQUEST;
        ack.instid = unit->getInstId();
        ack.posx = unit->getPositionX();
        ack.posy = unit->getPositionY();
        sendNetPacket(sessionid, &ack);
        return;
    }
    // end add
    
//    MotionRequest mr;
//    mr.mType = eIDLE;
//    mr.mSessionId = sessionid;
//    unit->mMotionRequestInFrame.push_back(mr);
//    return;
    
    if (unit->getState() == eSKILL && !unit->getIsSkillMoveState()) {
        return;
    }
    
    unit->setPosition(CCPoint(req.posx, req.posy));
    unit->addHistoryPosition(unit->getPosition());
    if (unit->stand() == 0)
    {
        ack_sync_stop ack;
        ack.errorcode = CE_INVALID_OP_REQUEST;
        ack.instid = unit->getInstId();
        ack.posx = unit->getPositionX();
        ack.posy = unit->getPositionY();
        sendNetPacket(sessionid, &ack);
    }
}}

handler_msg(req_sync_attack, req)
{
    CheckScenePlayer(sessionid, player, scene);
    WorldCreature* unit = getPlayerUnit(player, req.instid);
    CheckCondition(unit && unit->getScene() && !unit->isDead(), return)
    
    // added by jianghan for 如果该单元是服务器控制，那么这里就不接受客户端提交的动作
    if( unit->getCtrlMode() == eServerCtrl ){
        ack_sync_attack ack;
        ack.errorcode = CE_INVALID_OP_REQUEST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    // end add
    
    ack_sync_attack ack;
    ack.errorcode = 0;
    ack.instid = req.instid;
    ack.posx = req.x;
    ack.posy = req.y;
    ack.orient = req.orient;
    if (!unit->canChangeToState(eATTACK))
    {
        ack.errorcode = CE_INVALID_OP_REQUEST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    unit->stopMove();
    //unit->SceneObject::setSpeed(Vector2(0.0f, 0.0f));
    unit->setPosition(CCPoint(req.x, req.y));
    unit->setOrient((ObjOrientation)req.orient);
    unit->addHistoryPosition(unit->getPosition());

    bool isLastAtk = (req.atkIndex == SeparateValues::sRoleMaxCombo);
    
    //const WorldScene::CreatureHp wc = scene->collectRoleHp();

    if (isLastAtk) {
        //unit->unschedule(WorldCreature::lastAttack);
        unit->lastAttack(0, req.atkIndex);
    } else {
        unit->attack(0, req.atkIndex);
    }
    
    //const WorldScene::CreatureHp wcr = scene->findHpChangedCreatures(wc);
    // modified by jianghan for 场景伤害集中播放
    /*
    /*bool immediatlySend = false;
    //obj_attackedTarget atk;
    //for (WorldScene::CreatureHp::const_iterator it = wcr.begin();
    //     it != wcr.end(); ++it) {
        atk.sourceID = unit->getInstId();
        atk.targetID = it->first->getInstId();
        atk.effectLife = it->second;
        atk.hitType = it->first->getBeHitType();
        atk.atkIndex = req.atkIndex;
        scene->addSceneDamage(&atk);
        if(atk.effectLife <= 0){
            // 有人挂了，必须立刻发送消息，不等周期到
            immediatlySend = true;
        }
    }
    if( immediatlySend )
        scene->doDamageBroadcast();
     */
    // */
    // end modify
}}

hander_msg(req_sync_skill, req)
{

    CheckScenePlayer(sessionid, player, scene);
    WorldCreature* unit = getPlayerUnit(player, req.sourceID);
    CheckCondition(unit && unit->getScene() && !unit->isDead(), return)

    // added by jianghan for 如果该单元是服务器控制，那么这里就不接受客户端提交的动作
    if( unit->getCtrlMode() == eServerCtrl ){
        ack_sync_skill ack;
        ack.errorcode = CE_INVALID_OP_REQUEST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    // end add
    
    if (unit->skill(req.skill_id, req.x, req.y, req.orient) == 0) {
        ack_sync_skill ack;
        ack.errorcode = CE_INVALID_OP_REQUEST;
        ack.sourceID = req.sourceID;
        ack.skill_id = req.skill_id;
        ack.posx = req.x;
        ack.posy = req.y;
        sendNetPacket(sessionid, &ack);
    }
}}

bool cbGetTreasureFightPlayerRankData(void* obj, void* param)
{
    TreasureFightPlayer* player = (TreasureFightPlayer*)obj;
    ack_sync_treasurefight_rank_data* ack = (ack_sync_treasurefight_rank_data*)param;
    
    obj_treasurecopy_rankdata playerdata;
    playerdata.points = player->mScore;
    playerdata.roleid = player->mPlayer->mRole->getInstId();
    ack->datas.push_back(playerdata);
    
    return true;
}

void sendTreasureFightRank(WorldSceneTreasureFight* fightScene)
{
    ack_sync_treasurefight_rank_data ack;
    fightScene->traverseFightPlayer(cbGetTreasureFightPlayerRankData, &ack);
    fightScene->broadcastPacket(&ack);
}

handler_msg(req_sync_treasurefight_rank_data, req)
{
    CheckScenePlayer(sessionid, player, scene);
    
    WorldSceneTreasureFight* fightScene = dynamic_cast<WorldSceneTreasureFight*>(scene);
    if (fightScene == NULL) {
        return;
    }
    
    ack_sync_treasurefight_rank_data ack;
    fightScene->traverseFightPlayer(cbGetTreasureFightPlayerRankData, &ack);
    sendTreasureFightRank(fightScene);
}}


handler_msg(req_sync_scene_ready, req)
{
    CheckScenePlayer(sessionid, player, scene);
}}