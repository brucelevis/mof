//
//  Attack.cpp
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//
#include "hander_include.h"
#include "Role.h"
#include "Activity.h"
#include "BossController.h"
#include "GuildRole.h"
#include "BossFactory.h"


inline Boss* checkRoleBossRoom(Role* role)
{
    assert(role);
    
    if( 0 != strcmp( role->getCurrentActRoomName().c_str() ,  BOSS_ROOM_NAME) )
        return NULL;
    
   return (Boss*)(role->getCurrentActRoom());
}

hander_msg(req_enter_boss_scene, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_enter_boss_scene ack;
    ack.err = eWorldBossOK;
    
    Boss* bossIng = checkRoleBossRoom(role);
    if( bossIng != NULL )
    {
        bossIng->Remove(role);
    }

    int bossid = 0;

    if(eBossType_WorldBoss == req.bossType ) //世界boss
    {
        bossid = g_BossActivityController.getWorldBossId();
        if( 0 == bossid)
        {
            ack.err = eActNoOpen;// 活动未开始
            return sendNetPacket(sessionid, &ack);
        }
    }

    if( eBossType_GuildBoss == req.bossType ) //公会boss
    {
        RoleGuildProperty& property = SRoleGuild(roleid);
        if( property.noGuild() )
        {
            ack.err = eActNoOpen;// 没有公会
            return sendNetPacket(sessionid, &ack);
        }
        
        bossid = g_BossActivityController.getGuildBossId( property.getGuild() );
        if( 0 == bossid)
        {
            ack.err = eActNoOpen;// 活动未开始
            return sendNetPacket(sessionid, &ack);
        }
    }
    
    Boss* boss = SBossFactory.getBoss(bossid);
    xyassert(boss);
    
    if(boss->getState() == eDEAD)
    {
        ack.err = eBossDied; //boss已被打死
        return sendNetPacket(sessionid, &ack);
    }
    
//    role->CalcPlayerProp();
    role->sendRolePropInfoAndVerifyCode(true);
    boss->OnReqEnter(role, req);
}}

//hander_msg(req_enter_boss_scene, req)
//{
//    hand_Sid2Role_check(sessionid, roleid, role)
//    
//    WorldBoss* pRoom = (WorldBoss*)ActRoomMGR.find("WorldBoss");
//    ack_enter_boss_scene ack;
//    if (pRoom == NULL) {
//       ack.err = eActNoOpen;// 活动未开放
//    }else
//    if (!pRoom->IsOpen()) {
//        ack.err = eActNoOpen;// 活动未开始
//    }else
//    if(role->getLvl() < WorldBossCfg::enter_lvl){
//        ack.err = eLvlNoReach; // lvl limit
//    }else
//    if (pRoom->getState() == eDEAD) {
//        ack.err = eBossDied; //boss已被打死
//    }else
//    if(pRoom->Add(role, 0, 0, false)){
//        ack.sceneid = req.sceneid;
//        ack.err = eWorldBossOK;
//        ack.bossid = pRoom->getTpltID();
//        ack.sceneid= pRoom->getSceneID();
//        ack.bossHP = pRoom->getHp();
//        ack.reborn_seconds = WorldBossCfg::reborn_seconds;
//    }else {
//        pRoom->Remove(role);
//        if(pRoom->Add(role, 0, 0, false)){
//            ack.sceneid = req.sceneid;
//            ack.err = eWorldBossOK;
//            ack.bossid = pRoom->getTpltID();
//            ack.sceneid= pRoom->getSceneID();
//            ack.bossHP = pRoom->getHp();
//            ack.reborn_seconds = WorldBossCfg::reborn_seconds;
//        }else
//        ack.err = eAlreadyIn; // 进入失败，已在房间
//    }
//    sendNetPacket(sessionid, &ack);
//    //log_info("role enter worldboss scene: " << role->getRolename() << " err code: " << ack.err);
//}}


hander_msg(req_leave_boss_scene, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    Boss* boss = checkRoleBossRoom(role);
    if(NULL == boss) return;

    boss->Remove(role);
    
    ack_leave_boss_scene ack;
    ack.err = 0;
    ack.sceneid = boss->getSceneID();
    sendNetPacket(sessionid, &ack);
    
    // 改回主城ID
//    role->setCurrSceneId( role->getSceneID() );
    role->backToCity();
}}

hander_msg(req_role_battle_info, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)   
    Boss* boss = checkRoleBossRoom(role);
    if( NULL == boss )  return;
    
    if (boss->getState() == eDEAD) {
        return;
    }
    boss->SendRoleBattleInfo(req.roleid, roleid);

}}

hander_msg(req_attack, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)    
    Boss* boss = checkRoleBossRoom(role);
    if( NULL == boss )  return;
    
    if (boss->getState() == eDEAD) {
        return;
    }
    
    if (boss->getActivityState() != kBossCreated) {
        return;
    }
    
    boss->beAtk(role, req);
}}

hander_msg(req_skill_hurt, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)   
    Boss* boss = checkRoleBossRoom(role);
    if( NULL == boss )  return;
    
    if (boss->getState() == eDEAD) {
        return;
    }
    
    if (boss->getActivityState() != kBossCreated) {
        return;
    }
    
    boss->beSkill(role,req);
}}


pair<int, float>* getAffectedBuf(Obj* obj, int type)
{
    pair<int, float>* affectedBuf = NULL;
    switch (type) {
        case eBuffIncrHP:
        {
            affectedBuf = &obj->mHpBuf;
        }
            break;
        case eBuffChangeAttack:
        {
            affectedBuf = &obj->mAtkBuf;
            
        }      //改变攻击
            break;
        case eBuffChangeDogge:
        {
            affectedBuf = &obj->mDodgeBuf;
            
        }       //改变闪避
            break;
        case eBuffChangeCri:
        {
            affectedBuf = &obj->mCriBuf;
            
        }         //改变暴击
            break;
        case eBuffChangeDef:
        {
            affectedBuf = &obj->mDefBuf;
            
        }         //改变防御
            break;
        case eBuffChangeHit:
        {
            affectedBuf = &obj->mHitBuf;
            
        }       //改变命中
            break;
        default:
            break;
    }

    return affectedBuf;
}

hander_msg(notify_role_release_triggerskill, notify)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    int skillId = notify.skillid;
    bool exist;
    SkillCfgDef* cfgDef = SkillCfg::getCfg(skillId, exist);
    if (cfgDef == NULL) {
        return;
    }
    
    vector<SkillEffectVal*>::iterator iter;
    for (iter = cfgDef->effectList.begin(); iter != cfgDef->effectList.end(); iter++) {
        SkillEffectVal* effectVal = *iter;
        
        pair<int, float>* affectedBuf = getAffectedBuf(role, effectVal->affectType);
        if (affectedBuf) {
            affectedBuf->first += effectVal->affectValue;
            affectedBuf->second += effectVal->affectfloat;
        }
    }
}}

hander_msg(notify_role_stop_triggerskill, notify)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    int skillId = notify.skillid;
    bool exist;
    SkillCfgDef* cfgDef = SkillCfg::getCfg(skillId, exist);
    if (cfgDef == NULL) {
        return;
    }
    
    vector<SkillEffectVal*>::iterator iter;
    for (iter = cfgDef->effectList.begin(); iter != cfgDef->effectList.end(); iter++) {
        SkillEffectVal* effectVal = *iter;
        
        pair<int, float>* affectedBuf = getAffectedBuf(role, effectVal->affectType);
        if (affectedBuf) {
            affectedBuf->first -= effectVal->affectValue;
            affectedBuf->second -= effectVal->affectfloat;
        }
    }
}}


//hander_msg(req_pk,req)
//{
//    hand_Sid2Role_check(sessionid, roleid, role)
//    Role* tarrole = SRoleMgr.GetRole(req.roleid);
//    if (tarrole) {
//        ack_pk_ready ack;
//        ack.skills = tarrole->getSkillMod()->getStudySkillList();
//            
//        tarrole->CalcPlayerProp();
//        ack.errorcode = 0;
//        ack.maxhp = tarrole->mMaxHp;
//        ack.atk = tarrole->mAtk;
//        ack.def = tarrole->mDef;
//        ack.hit = tarrole->mHit;
//        ack.dodge = tarrole->mDodge;
//        ack.cri = tarrole->mCri;
//        
//        Pet* pet = SPetMgr.getPet(tarrole->activepet);
//        obj_petinfo info;
//        if (pet)
//        {
//            info.petid = pet->petid;
//            info.pettypeid = pet->petmod;
//            info.isactive = pet->isactive;
//            
//            info.exp = pet->exp;
//            info.growth = pet->growth;
//            info.lvl = pet->lvl;
//            
//            info.skills = pet->skills;
//            info.bhit = pet->bhit;
//            info.bdodge = pet->bdodge;
//            info.batk = pet->batk;
//            info.bdef = pet->bdef;
//            info.bhp = pet->bhp;
//        }
//        else
//        {
//            info.petid = 0;
//        }
//        
//        ack.petinfo = info;
//        ack.roleid=req.roleid;
//        ack.rolename=tarrole->getRolename();
//        ack.lvl=tarrole->getLvl();
//        ack.roletype=tarrole->getRoleType();
//        ack.pos=1;
//        sendNetPacket(sessionid, &ack);
//        
//        
//        
//        ack.skills = role->getSkillMod()->getStudySkillList();
//        
//        role->CalcPlayerProp();
//        ack.errorcode = 0;
//        ack.maxhp = role->mMaxHp;
//        ack.atk = role->mAtk;
//        ack.def = role->mDef;
//        ack.hit = role->mHit;
//        ack.dodge = role->mDodge;
//        ack.cri = role->mCri;
//        
//        pet = SPetMgr.getPet(role->activepet);
//        if (pet)
//        {
//            info.petid = pet->petid;
//            info.pettypeid = pet->petmod;
//            info.isactive = pet->isactive;
//            
//            info.exp = pet->exp;
//            info.growth = pet->growth;
//            info.lvl = pet->lvl;
//            
//            info.skills = pet->skills;
//            info.bhit = pet->bhit;
//            info.bdodge = pet->bdodge;
//            info.batk = pet->batk;
//            info.bdef = pet->bdef;
//            info.bhp = pet->bhp;
//        }
//        else
//        {
//            info.petid = 0;
//        }
//        
//        ack.roleid = roleid;
//        ack.rolename=role->getRolename();
//        ack.lvl=role->getLvl();
//        ack.roletype=role->getRoleType();
//        ack.pos=2;
//        sendRolePacket(req.roleid, &ack);
//    }
//    
//}}
//
//hander_msg(ack_pk,req)
//{
//    
//}}
//
//hander_msg(ack_pk_ready,req)
//{
//    
//}}
//
//hander_msg(syn_pk_role_event,syn)
//{
//    hand_Sid2Role_check(sessionid, roleid, role)
//    Role* tarrole = SRoleMgr.GetRole(syn.objid);
//    if (tarrole) {
//        syn_pk_role_event ack;ack = syn;
//        ack.objid=roleid;
//        //sendNetPacket(sessionid, &ack);
//        sendRolePacket(syn.objid, &ack);
//    }
//
//}}
//
//hander_msg(syn_pk_pet_event,syn)
//{
//    
//}}
