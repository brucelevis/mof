//
//  scene_handler.cpp
//  GameSrv
//
//  Created by prcv on 14-3-14.
//
//

#include "hander_include.h"
#include "world_cmdmsg.h"
#include "basedef.h"
#include "GuildRole.h"
#include "GuildMgr.h"
#include "Skill.h"
#include "Retinue.h"

WorldRoleInfo getWorldRoleInfo(Role* role)
{
    WorldRoleInfo info;
    
    info.mRoleId = role->getInstID();
    info.mRoleName = role->getRolename();
    info.mRoleType = role->getRoleType();
    info.mLvl = role->getLvl();
    info.mVipLvl = role->getVipLvl();
	info.mHonorId = role->getUseHonor();
    info.mGuildPosition = SRoleGuild(role->getInstID()).getPosition();
    info.mGuildName = SGuildMgr.getRoleGuild(role->getInstID()).getGuildName();
    info.mBodyFashion = role->getBodyFashion();
    info.mWeaponFashion = role->getWeaponFashion();
    
    info.mMaxHp = role->mRealBattleProp.getMaxHp();
    info.mCri = role->mRealBattleProp.getCri();
    info.mDodge = role->mRealBattleProp.getDodge();
    info.mHit = role->mRealBattleProp.getHit();
    info.mDef = role->mRealBattleProp.getDef();
    info.mAtk = role->mRealBattleProp.getAtk();
    info.mBattle = role->getBattleForce();
    
    if (role->getSkillMod())
    {
        info.mSkills = role->getSkillMod()->getEquipSkillList();
    }
    
    int equipId = role->getPlayerEquip()->GetItem(kEPTWeapon).item;
    if(GameFunctionCfg::getGameFunctionCfg().equipType == eMultipleEquipDrop)
    {
        info.mWeaponQuality = ItemCfg::readInt(equipId, "model");
    }
    else //大陆版单装备的品质
    {
        info.mWeaponQuality = ItemCfg::readInt(equipId, "qua");
    }
    
    info.mBodyFashion = role->getBodyFashion();
    info.mWeaponFashion = role->getWeaponFashion();
    info.mEnchantLvl = role->getEnchantLvl();
    info.mEnchantId = role->getEnchantId();
    info.mAwakeLvl = role->getRoleAwakeLvl();
    
    info.mTriggerSkill = WeaponEnchantCfg::getSkillid(role->getEnchantId(), role->getEnchantLvl());
    info.mTriggerSkillPro = WeaponEnchantCfg::getProbability(role->getEnchantId(), role->getEnchantLvl());
    
    return info;
}

WorldRetinueInfo getWorldRetinueInfo(Role* role, Retinue *retinue)
{
    WorldRetinueInfo info;
    if(retinue && role) {
        info.mOwnerid = retinue ->  getOwner();
        info.mRetinuemod = retinue -> getModId();
        info.mLvl = retinue -> getLvl();
        info.mRetinueId = retinue -> getId();
        info.mSkills =  role -> getRetinueMgr() -> mSkillMgr -> getEquipedSkills();
    }
    return info;
}

WorldPetInfo getWorldPetInfo(Pet* pet)
{
    WorldPetInfo info;
    if (pet)
    {
    
        info.mAtk = pet->mRealBattleProp.getAtk();
        info.mDef = pet->mRealBattleProp.getDef();
        info.mHit = pet->mRealBattleProp.getHit();
        info.mDodge = pet->mRealBattleProp.getDodge();
        info.mMaxHp = pet->mRealBattleProp.getMaxHp();
        info.mCri = pet->mRealBattleProp.getCri();
        info.mLvl  = pet->getLvl();
    
        info.mPetId = pet->getId();
        info.mPetMod = pet->petmod;
        
        info.mMoveSpeed = pet->getMoveSpeed();
        
        info.mSkills = pet->getSkill().toArray();
    }
    else
    {
        info.mPetId = 0;
    }
    return info;
}

WorldPlayerInfo getWorldPlayerInfo(Role* role)
{
    WorldPlayerInfo playerInfo;
    playerInfo.mSessionId = role->getSessionId();
    playerInfo.mRoleId = role->getInstID();
    playerInfo.mRoleInfo = getWorldRoleInfo(role);
   
    if (role->mPetMgr->getActivePet())
    {
        playerInfo.mPetInfos.push_back(getWorldPetInfo(role->mPetMgr->getActivePet()));
    }
    
    if (role -> mRetinueMgr -> getActiveRetinue())
    {
        playerInfo.mRetinueInfo = getWorldRetinueInfo(role, role -> mRetinueMgr -> getActiveRetinue());
    }

    return playerInfo;
}

void createWorldScene(int sceneMod, const string& extendStr, const vector<WorldPlayerInfo>& playerInfos)
{
    create_cmd(CreateWorldScene, cmd);
    cmd->sceneMod = sceneMod;
    cmd->extendStr = extendStr;
    cmd->playerinfos = playerInfos;
    sendMessageToWorld(0, CMD_MSG, cmd, 0);
}


void createWorldScene(int sceneMod, const string& extendStr, const WorldPlayerInfo& playerInfo)
{
    create_cmd(CreateWorldScene, cmd);
    cmd->sceneMod = sceneMod;
    cmd->extendStr = extendStr;
    cmd->playerinfos.push_back(playerInfo);
    sendMessageToWorld(0, CMD_MSG, cmd, 0);
}

void enterWorldScene(int sceneInstId, const vector<WorldPlayerInfo>& playerInfos)
{
    create_cmd(EnterWorldScene, cmd);
    cmd->sceneInstId = sceneInstId;
    cmd->playerinfos = playerInfos;
    sendMessageToWorld(0, CMD_MSG, cmd, 0);
}

void enterWorldScene(int sceneInstId, const WorldPlayerInfo& playerInfo)
{
    create_cmd(EnterWorldScene, cmd);
    cmd->sceneInstId = sceneInstId;
    cmd->playerinfos.push_back(playerInfo);
    sendMessageToWorld(0, CMD_MSG, cmd, 0);
}

void quickEnterWorldScene(int sceneMod, const WorldPlayerInfo& playerInfo)
{
    create_cmd(QuickEnterWorldScene, cmd);
    cmd->sceneMod = sceneMod;
    cmd->playerinfo = playerInfo;
    sendMessageToWorld(0, CMD_MSG, cmd, 0);
}

handler_msg(req_enter_world_scene, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    CheckCondition(req.sceneMod > 0, return)
    CheckCondition(role->getSceneID() == role->getCurrSceneId(), return) //只有在主城才能进入世界场景
    CheckCondition(role->getWorldState() == eWorldStateNone, return)
    
    SceneCfgDef* sceneCfg = SceneCfg::getCfg(req.sceneMod);
    if (sceneCfg == NULL) {
        return;
    }
    
    int sceneType = sceneCfg->sceneType;
    switch (sceneType) {
        case stSyncPvp:
            role->addSyncPvpBuff();
            break;
        case stTreasureFight:
            role->addTreasureFightBuff();
            break;
            
        default:
            break;
    }
    
    role->setWorldState(eWorldStateEntering);
    role->setCurrSceneId(req.sceneMod);
    
    WorldPlayerInfo playerInfo = getWorldPlayerInfo(role);
    if (req.sceneId > 0) {
        enterWorldScene(req.sceneId, playerInfo);
    } else {
        quickEnterWorldScene(req.sceneMod, playerInfo);
    }
}}


handler_msg(req_leave_world_scene, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (!role->tryLeaveWorldScene())
    {
        ack_leave_world_scene ack;
        ack.errorcode = 1;
        sendNetPacket(sessionid, &ack);
    }
}}