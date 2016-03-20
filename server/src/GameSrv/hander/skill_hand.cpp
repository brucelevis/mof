 //
//  skill_hand.cpp
//  GameSrv
//
//  Created by cxy on 13-1-10.
//
//

#include "Skill.h"
#include "hander_include.h"
#include "GameLog.h"
#include "MysticalCopy.h"
#include "RoleAwake.h"
#include "Global.h"

//bool SaveSkillTable(int roleid, vector<int>& skilllist)
//{
//    redisReply* rreply;
//    string skillTable = "";
//    char temp[10] = {0};
//    for (int i = 0; i < skilllist.size(); i++) {
//        sprintf(temp, "%d", skilllist[i]);
//        skillTable += temp;
//        skillTable += ":";
//    }
////    cout<<"test skilltable:"<<skillTable<<endl;
//    
//    rreply=redisCmd("hmset role:%d skilltable %s",roleid, skillTable.c_str());
//    freeReplyObject(rreply);
//    
//    return true;
//}

hander_msg(req_studyable_skill_list,req)
{
//    hand_Sid2Role_check(sessionid, roleid, role)
//    //int lvl = role->getLvl();
//    
//    if(role==NULL)
//    {
//        cout<<"get role Error"<<endl;
//    }
//    ack_studyable_skill_list ack;
//    ack.errorcode = 0;
//    
//    vector<obj_studyable_skill> skills;
//    skills.clear();
//    
//
//    skills = role->GetStudyableSKills();
//    ack.skillList = skills;
//    sendNetPacket(sessionid,&ack);
}}

// 学习技能
hander_msg(req_study_skill,req)
{
    int skillId = req.skillID;
    hand_Sid2Role_check(sessionid, roleid, role)
    
    int nowLevel = role->getSkillMod()->getSkillLevel(getSkillIndex(req.skillID));
    int targetLevel = CalcSkillLvl(req.skillID);
    
    int needGold = 0;
    int needBatpoint = 0;
    
    if (targetLevel <= nowLevel) {
        return;
    }
    
    ack_study_skill ack;
    
    ack.skillID = getSkillIndex(req.skillID) + targetLevel;
    
    //检查该技能是否属于该角色
    if (!role->skillCanLearn(req.skillID)) {
        
        ack.errorcode = 1;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    //判断配置是否存在
    bool exist;
    SkillCfgDef* def = SkillCfg::getCfg(skillId,exist);
    if (!exist) {
        
        ack.errorcode = 1;
        sendNetPacket(sessionid, &ack);
        return;
        
    }
    
    //判断等级、职业
    if (def->learnLvl > role->getLvl() || !checkSkillOwner(def->owner, role->getJob()))
    {
        ack.errorcode = 1;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    for (int i = 0; i < (targetLevel - nowLevel); i++) {
        SkillCfgDef* skillCfg = SkillCfg::getCfg(req.skillID - i);
        if (skillCfg) {
            needGold += skillCfg->needGold;
            needBatpoint += skillCfg->battlepoint;
        }
        else{
            ack.errorcode = 1;
            sendNetPacket(sessionid, &ack);
            return;
        }
    }
    
    
    //判断金币或战功是否足够
    if (needGold > role->getGold() || needBatpoint > role->getBattlePoint())
    {
        ack.errorcode = 1;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    bool isSucc = role->getSkillMod()->learn(skillId); 
    if(isSucc)
    {
        ack.errorcode = 0;
        ack.skillID = skillId;
        
        role->addGold(- needGold, "study_skill");
        role->addBattlePoint( - needBatpoint, "study_skill");
        
        UpdateQuestProgress("studyskill", skillId, 1, role, true);
        
        //判断技能是否为被动技能
        if (ePassive == def->getSKillType())
        {
            UpdateQuestProgress("studypassiveskill", skillId, 1, role, true);
        }
        
        // 日志记录
        LogMod::addLogUpgradeSkill(roleid, skillId, def->skillname.c_str());
        
        //保存新的技能列表
//        vector<int> skillList;
//        skillList = role->getSkillMod()->getStudySkillList();
//        SaveSkillTable(roleid, skillList);
        role->getSkillMod()->saveStudySkillList();
        
        //重新计算玩家的属性
        role->CalcPlayerProp();
        
        //更新战斗力排行
        role->UploadPaihangData(eUpdate);
        
        role->getSkillMod()->checkAutoEquip(req.skillID);
        
        role->getSkillMod()->saveEquipSkillList(roleid);

		
    }else ack.errorcode = 1;
    
    sendNetPacket(sessionid, &ack);

}}


hander_msg(req_equip_skill,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_equip_skill ack;
    ack.errorcode = CE_OK;
    do
    {
        bool isStudy = false;
        
        //判断是否已学过改技能
        isStudy = role->getSkillMod()->IsSkillLearn(req.skillID);
        
        if (isStudy == false) {
            ack.errorcode = CE_YOU_HAVE_NOT_STUDY_THIS_SKILL;
            break;
        }
        
        if( !role->getSkillMod()->equipInitiativeSkill(req.skillID, req.index) )
        {
            ack.errorcode = CE_YOU_EQUIP_SKILL_POSITION_NOT_RIGHT;
            break;
        }
        
    }while(false);
    if (ack.errorcode == CE_OK) {
        ack.skillID = req.skillID;
        ack.index = req.index;
        role->getSkillMod()->saveEquipSkillList(roleid);
    }
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_unequip_skill,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_unequip_skill ack;
    ack.errorcode = CE_YOU_EQUIP_SKILL_POSITION_NOT_RIGHT;
    if( role->getSkillMod()->unEquipInitiativeSkill(req.index) )
    {
        ack.errorcode = CE_OK;
        ack.index = req.index;
        role->getSkillMod()->saveEquipSkillList(roleid);
    }
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_equiped_skill_list,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_equiped_skill_list ack;
    ack.errorcode = CE_OK;
    
    vector<int> skills = role->getSkillMod()->getEquipSkillList();
    
    for (int i = 0; i < skills.size(); i++) {
        obj_equiped_skill skill;
        skill.index = i;
        skill.skillID = skills[i];
        ack.skillList.push_back(skill);
    }
    
    sendNetPacket(sessionid, &ack);
    
}}

//技能得施放后台只作部分验证。
hander_msg(req_emit_skill,req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    bool isLearn = role->getSkillMod()->IsSkillLearn(req.skillID);
    if (!isLearn) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    bool exist;
    SkillCfgDef* def = SkillCfg::getCfg(req.skillID, exist);
    if (def == NULL)
    {
        return;
    }
    
    if (def->skillType == eAnger)
    {
        RoleTpltCfg* roleCfg = RoleTpltCfg::getCfg(role->getRoleType());
        if (role->getRoleAnger() != roleCfg->getMaxAnger())
        {
            kickAwayRoleWithTips(role, "");
            return;
        }
        role->setRoleAnger(0);
    }
}}

hander_msg(req_get_player_skill_table, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    //获取已学习的技能列表
    vector<int> skillList;
    skillList = role->getSkillMod()->getStudySkillList();
    
    int nErrorCode = 0;
    ack_get_player_skill_table ack;
    ack.errorcode = nErrorCode;
    ack.skillTable.clear();
    for(int i = 0;i < skillList.size(); i++)
    {
        ack.skillTable.push_back(skillList[i]);
    }
    sendNetPacket(sessionid, &ack);
}}



#pragma mark -
#pragma mark 技能书学习技能


#define return_SendAckPetStudySkill( err ) \
do{\
    ack.errcode = err;\
    return sendNetPacket(sessionid, &ack);\
}while(0)


// 宠物是否已学某技能，学过的返回前一个技能id
//int checkSkillStudied(int skillid , Pet* pet)
//{
//    if( NULL == pet )
//        return 0;
//    
//    int skillidcfg = calSkillCfgIdAndLevel( skillid ).first;
//    
//    IntArrayObj::const_iterator it = pet->getSkill().begin();
//    
//    for (; it != pet->getSkill().end(); ++it)
//    {
//        int studiedSkillId = calSkillCfgIdAndLevel( *it ).first;
//        
//        if(skillidcfg == studiedSkillId)
//        {
//            return *it;
//        }
//    }
//    
//    return 0;
//}

void petStudySkill(Role* role , req_pet_study_skill& req , int gold , bool succ)
{
    xyassert(role);
    int roleid = role->getInstID();
    int sessionid = role->getSessionId();
    ack_pet_study_skill ack;
    ack.petid = req.petid;
    ack.skillid = req.skillid;
    
    //收钱
    int errorCode = role->CheckMoneyEnough(gold, eGoldCost,"PetStudySkill");
    xyassert(errorCode == CE_OK);
    
    //wangzhignag 2014-11-19
    //吃掉书
    ItemArray materail,merged;
    GridArray effeGrids;
    for (int i = 0; i< req.bookid.size(); ++i)
    {
        ItemGroup itemgroup;
        itemgroup.item = req.bookid[i];
        itemgroup.count = 1;
        materail.push_back(itemgroup);
    }
    
    //因为PreDelItems 不能有重复的
    mergeItems(materail, merged);
    
    bool eatBookSucc = role->getBackBag()->PreDelItems(merged, effeGrids);
    
    if( ! eatBookSucc )
        return_SendAckPetStudySkill( ePetStudySkill_BookSkillNotMatch );
    
    role->updateItemsChange(effeGrids);
    ////////
    { // use item log
        LogMod::addUseItemLog(role, merged, "req_pet_study_skill");
    }
    
    Pet* pet = role->mPetMgr->getPet(req.petid); xyassert(pet);
    int reqSkillCfgId = calSkillCfgIdAndLevel(req.skillid).first;
    
    //写日志
    Xylog log(eLogName_PetStudySkill,roleid);
    log<<pet->petid<<"petmod:"<<pet->petmod<<"skill:"<<req.skillid<<"gold:"<<gold<<"succ:"<<succ<<"book:";
    for (int i = 0; i< req.bookid.size(); ++i)
    {
        log<<req.bookid[i];
    }
    
    ItemCfgDef* itemcfg = ItemCfg::getCfg(req.bookid[0]);
    xycheck( itemcfg && (kItemSkillBook == itemcfg->ReadInt("type"))  );
    SkillCfgDef* skillcfg = SkillCfg::getCfg(req.skillid);
    xycheck( skillcfg && skillcfg->needPages > 0 ); // 页为0表示非宠物技能
    
    //升级
    if( succ )
    {
        int before = getPetSkillStudied(req.skillid , pet);
        if( before != 0)
            pet->getSkill().erase(before);
        pet->getSkill().push_back(req.skillid);
        
        ack.failtimes = 0;
        pet->saveNewProperty();
        return_SendAckPetStudySkill( ePetStudySkill_Success );
    }
    
    //未升
    PetUpSkillProbData *data = pet->getUpSkillProbData().find(req.skillid);
    if (data != NULL) {
        ack.failtimes = data->upSkillFailTimes;
    } else {
        ack.failtimes = 0;
    }
    pet->saveNewProperty();
    return_SendAckPetStudySkill( ePetStudySkill_Fail );
}

// 宠物配置表的技能
bool petCfgHasSkill(int skillid , PetCfgDef* petcfg)
{
    if(NULL == petcfg)
        return false;
    
    bool hasSkill = false ;
    
    int reqSkillCfgId = calSkillCfgIdAndLevel(skillid).first;
    
    for (int i = 0; i< petcfg->skills.size(); ++i)
    {
        int cfgid = calSkillCfgIdAndLevel( petcfg->skills[i] ).first;
        if( reqSkillCfgId ==  cfgid)
        {
            hasSkill = true;
            break;
        }
    }
    return hasSkill;
}



// 如果是升级技能，判断新技能的等级是否合法
bool checkUpgradeSkillLevel( int before , int newskill , int quaMaxLvl )
{
    int beforelvl = calSkillCfgIdAndLevel( before ).second;
    int newlvl = calSkillCfgIdAndLevel( newskill ).second;
    
    if( beforelvl + 1 != newlvl )
        return false;
    
    if( newlvl > quaMaxLvl )
        return false;
    
    return true;
}

// 失败总概率+下一次升级概率
float getTotalFailProb(float rate, float addProb, int times)
{
    int totalTimes = 0;
    for (int i = 1; i <= times + 1; ++i) {
        totalTimes += i;
    }
    
    float totalProb = times * rate + totalTimes * addProb;
    return totalProb;
}

// 技能书学习技能
handler_msg(req_pet_study_skill, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_pet_study_skill ack;
    ack.petid = req.petid;
    ack.skillid = req.skillid;
    
    SkillCfgDef* skillcfg = SkillCfg::getCfg(req.skillid);
    xycheck( skillcfg && skillcfg->needPages > 0 ); // 页为0表示非宠物技能
    
    Pet* pet = role->mPetMgr->getPet(req.petid);
    xycheck(pet);
    
    PetCfgDef* petcfg = PetCfg::getCfg(pet->petmod);
    xyassertf(petcfg,"petmod = %d", pet->petmod);
    
    
    if (skillcfg->needGold > role->getGold() )
        return_SendAckPetStudySkill( ePetStudySkill_NeedMoreGold );
    
    //现在只用一本书
    xycheck(req.bookid.size() > 0);
    ItemCfgDef* itemcfg = ItemCfg::getCfg(req.bookid[0]);
    xycheck( itemcfg && (kItemSkillBook == itemcfg->ReadInt("type"))  );

    int is_commonskill = itemcfg->ReadInt("is_commonskill",0);
    
    int quaMaxLvl = 0;
    
    // common skill
    if( skillcfg->getPetSkillType() == ePetSkillType_Common)
    {
        xycheck( is_commonskill == 1 );
        quaMaxLvl = PetCfg::getMaxCommmonSkills(petcfg->getQua());
        
    }else{  // special skill
        xycheck( is_commonskill  == 0);
        quaMaxLvl = PetCfg::getMaxSpecialSkills(petcfg->getQua());
        
        int bookqua = itemcfg->ReadInt("qua",0);    // check qua
        if( bookqua != petcfg->getQua())
            return_SendAckPetStudySkill( ePetStudySkill_BookSkillNotMatch );
    }
    
    
    int beforeskill =  getPetSkillStudied(req.skillid, pet);
     
    if( 0  != beforeskill) // upgrade
    {
        xycheck( checkUpgradeSkillLevel(beforeskill,req.skillid,quaMaxLvl) );
        
        float rate = itemcfg->ReadInt("pages",0) * 1.0f / skillcfg->getNeedPages();
        float upProb;
        PetUpSkillProbData *data = pet->getUpSkillProbData().find(req.skillid);
        if (data == NULL) {
            upProb = rate;
        } else if (data->upFailTotalProb < skillcfg->getTopTotalProb()) {
            upProb = data->upSkillProb;
        } else {
            upProb = 1.0;
        }
        
        float randnum = range_randf(0.0, 1.0);
        if(randnum < upProb){
            if (data != NULL) {
                pet->getUpSkillProbData().erase(req.skillid);
                pet->save();
            }
            
            ack.failtimes = 0;
            return petStudySkill(role,req,skillcfg->needGold,true);// success upgrade
        }
        
        PetUpSkillProbData upskilldata;
        if (data == NULL) {
            upskilldata.skillid = req.skillid;
            upskilldata.upSkillFailTimes = 1;
            upskilldata.upFailTotalProb = getTotalFailProb(rate, skillcfg->getUpFailAddProb(), 1);
            upskilldata.upSkillProb = rate + skillcfg->getUpFailAddProb();         
        } else {
            upskilldata.skillid = req.skillid;
            upskilldata.upSkillFailTimes = data->upSkillFailTimes + 1;
            upskilldata.upFailTotalProb = getTotalFailProb(rate, skillcfg->getUpFailAddProb(), upskilldata.upSkillFailTimes);
            upskilldata.upSkillProb = rate + upskilldata.upSkillFailTimes * skillcfg->getUpFailAddProb();
        }
        pet->getUpSkillProbData().add(req.skillid, upskilldata);
        pet->save();
        
        ack.failtimes = upskilldata.upSkillFailTimes;
        return petStudySkill(role,req,skillcfg->needGold,false);// fail upgrade
    }
    
    //unlock skill , 解锁技能要判断配置表是否有这技能。升级技能则不必了，有些老技能被拿掉了，但仍可以升级
    xycheck( petCfgHasSkill(req.skillid, petcfg) );
    xycheck( pet->getLockSkill().exist(req.skillid) );
    pet->getLockSkill().erase( req.skillid );

    return petStudySkill(role,req,skillcfg->needGold,true);
    
}}

// 校验宠物技能解锁的基本条件,失败返回-1,
static int _checkPetUnlockSkillCondition(Role *role, req_pet_unlock_skill &req, ack_pet_unlock_skill &ack)
{
    ack.errcode = ePetUnlockSkill_UnknownError;
    ack.petid = req.petid;
    ack.skillid = req.skillid;
    
    int petskillid = req.skillid;
    int needgold = -1;

    do
    {
        // 只用一本书
        if (req.bookid.size() != 1)
        {
            break ;
        }
        
        // 没有这只宠物
        Pet *pet = role->mPetMgr->getPet(req.petid);
        if (!pet)
        {
            ack.errcode = ePetUnlockSkill_CfgError;
            break ;
        }
        
        // 页为0表示非宠物技能
        SkillCfgDef* skillcfg = SkillCfg::getCfg(req.skillid);
        if (!skillcfg || skillcfg->needPages <= 0)
        {
            ack.errcode = ePetUnlockSkill_CfgError;
            break ;
        }
        
        // 宠物模板有问题
        PetCfgDef* petcfg = PetCfg::getCfg(pet->petmod);
        if (!petcfg)
        {
            ack.errcode = ePetUnlockSkill_CfgError;
            break ;
        }
        
        // 没有该物品,或者不是技能书或者不是解锁宠物技能的书
        ItemCfgDef* itemcfg = ItemCfg::getCfg(req.bookid[0]);
        if (!itemcfg || kItemSkillBook != itemcfg->ReadInt("type") ||
            (1 != itemcfg->ReadInt("skill_book_type", 0)))
        {
            ack.errcode = ePetUnlockSkill_ItemNotMatched;
            break ;
        }
        
        // 物品颜色和宠物颜色不一样/技能不匹配
        int is_commonskill = itemcfg->ReadInt("is_commonskill",0);
        if( skillcfg->getPetSkillType() == ePetSkillType_Common)
        {
            if (is_commonskill != 1)
            {
                break;
            }
        }
        else
        {
            if (is_commonskill != 0)
            {
                break;
            }
            int bookqua = itemcfg->ReadInt("qua",0);
            if( bookqua != petcfg->getQua())
            {
                ack.errcode = ePetUnlockSkill_ItemNotMatched;
                break ;
            }
        }
        
        // 技能没找到
        set<int> cfgskill(petcfg->skills.begin(), petcfg->skills.end());
        vector<int> petskill = pet->getSkill().toArray();
        
        for (size_t i = 0; i < petskill.size(); ++i)
        {
            int reqSkillCfgId = calSkillCfgIdAndLevel(petskill[i]).first;
            for (int i = 0; i< petcfg->skills.size(); ++i)
            {
                int cfgid = calSkillCfgIdAndLevel( petcfg->skills[i] ).first;
                if( reqSkillCfgId ==  cfgid)
                {
                    cfgskill.erase(petcfg->skills[i]);
                    break;
                }
            }
        }
        
        // 剩下的配置表中技能
        bool isnewskill = false;
        for (set<int>::iterator iter = cfgskill.begin(); iter != cfgskill.end(); ++iter)
        {
            if (*iter == petskillid)
            {
                isnewskill = true;
                break ;
            }
        }
        if (!isnewskill)
        {
            break ;
        }
        
        
        // 钱不够
        if (skillcfg->needGold > role->getGold())
        {
            ack.errcode = ePetUnlockSkill_NotEnoughMoney;
            break ;
        }
        
        needgold = skillcfg->needGold;
        
        return needgold;
    }
    while (0);
    
    return needgold;
}

static void _unlockPetSkill(Role *role, req_pet_unlock_skill &req, ack_pet_unlock_skill &ack)
{
    // 从背包删除物品
//    bool eatBookSucc = BackBag::SafeDelItem(role->getInstID(), req.bookid);
//    if(!eatBookSucc)
//    {
//        // 背包没找到相应物品
//        ack.errcode = ePetUnlockSkill_NotEnoughItem;
//        return ;
//    }
    
    //wangzhignag 2014-11-19
    //吃掉书
    ItemArray materail,merged;
    GridArray effeGrids;
    for (int i = 0; i< req.bookid.size(); ++i)
    {
        ItemGroup itemgroup;
        itemgroup.item = req.bookid[i];
        itemgroup.count = 1;
        materail.push_back(itemgroup);
    }
    
    //因为PreDelItems 不能有重复的
    mergeItems(materail, merged);
    
    bool eatBookSucc = role->getBackBag()->PreDelItems(merged, effeGrids);
    if(!eatBookSucc)
    {
        // 背包没找到相应物品
        ack.errcode = ePetUnlockSkill_NotEnoughItem;
        return ;
    }
    
    role->updateItemsChange(effeGrids);
    
    { // use item log
        LogMod::addUseItemLog(role, merged, "req_pet_unlock_skill");
    }
    
    Pet* pet = role->mPetMgr->getPet(req.petid);
    pet->getSkill().push_back(req.skillid);
    pet->saveNewProperty();
    
    ack.errcode = ePetUnlockSkill_Success;
    
    //写日志
    Xylog log(eLogName_UnlockPetSkill, role->getInstID());
    log << pet->petid << req.skillid << req.bookid[0];
}

// 宠物技能解锁
hander_msg(req_pet_unlock_skill, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_pet_unlock_skill ack;
    do
    {
        int needgold = _checkPetUnlockSkillCondition(role, req, ack);
        if (needgold < 0)
        {
            break ;
        }
        
        // 扣钱
        role->addGold(-needgold, "unlock_pet_skill");
        
        // 解锁宠物技能
        _unlockPetSkill(role, req, ack);
    } while (0);
    
    role->send(&ack);
}}


#pragma mark -

/*
 def_object(obj_type_and_id, caster)                      //释放者（ObjType 和 ID）
 def_object_arr(obj_skill_effect_info,casterSkilleffectList)                         //释放者身上的技能id列表
 def_object(obj_current_battleprop_info, casterProp)      //释放者的当前属性（加buff后的6个核心属性）
 def_object(obj_type_and_id, hurter)                      //受害者（ObjType 和 ID
 def_object_arr(obj_skill_effect_info,hurterSkilleffectList) //受害者身上的技能id效果列表
 def_object(obj_current_battleprop_info, hurterProp)       //受害者的当前属性（加buff后的6个核心属性)
 def_int(hurtType)                                          //是否为暴击 1 暴击，0 普通攻击
 def_int(hurtNumber)
 */



hander_msg(req_verify_skill_hurt, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
//    return;
    
    SceneCfgDef* currScene = SceneCfg::getCfg(role->getCurrSceneId());
    
    if (currScene == NULL || currScene->sceneType == stTown) {
        return;
    }
    
    {
        Xylog log(eLogName_BattleDataCollect, roleid);
        log << role->getBattleTime() << role->getCurrSceneId() << currScene->fightpoint << role->getBattleForce();
        
        log << req.skillType << req.skillID;
        
        log << strFormat("%d,%d", req.caster.objType, req.caster.objID);
        string casterEffects = "[";
        for (int i = 0; i < req.casterSkilleffectList.size(); i++) {
            string skillEffect = strFormat("%d:%d,", req.casterSkilleffectList[i].skillid,
                                           req.casterSkilleffectList[i].skilleffectindex);
            casterEffects.append(skillEffect);
        }
        casterEffects.append("]");
        log << casterEffects;
        
        string clientBatProp = strFormat("%d,%d,%f,%f,%f,%d", req.casterProp.atk, req.casterProp.def,
                                         req.casterProp.hit, req.casterProp.dodge, req.casterProp.cri,
                                         req.casterProp.hp);
        log << clientBatProp;
        
        string hurter = strFormat("%d,%d", req.hurter.objType, req.hurter.objID);
        log << hurter;
        
        string hurtEffects = "[";
        for (int i = 0; i < req.hurterSkilleffectList.size(); i++) {
            string skillEffect = strFormat("%d:%d,", req.hurterSkilleffectList[i].skillid,
                                           req.hurterSkilleffectList[i].skilleffectindex);
            hurtEffects.append(skillEffect);
        }
        hurtEffects.append("]");
        log << hurtEffects;
        
        string hurterProp = strFormat("%d,%d,%f,%f,%f,%d", req.hurterProp.atk, req.hurterProp.def,
                                      req.hurterProp.hit, req.hurterProp.dodge, req.hurterProp.cri,
                                      req.hurterProp.hp);
        log << hurterProp;
        
        log << req.hurtType;
        log << req.hurtNumber;
    }
    
    
    //verifyObjProp();
    //verifyObjProp();
    
    //伤害类型（技能 1 或普攻 0）
    if (req.skillType == 0)
    {
        //verifyAttackHurt();
        
        
    }
    else if (req.skillType == 1)
    {
        //verifySkillHurt();
        
        do
        {
            //这些副本的怪物不检查
            if (currScene->sceneType == stNewbie ||
                currScene->sceneType == stDefendStatue ||
                currScene->sceneType == stChallenge ||
                currScene->sceneType == stLimitTimeKilled ||
                currScene->sceneType == stPetArena) {
                break;
            }
            
            //如果受伤害的是怪物就验证当前场景是否存在这种怪
            if (req.hurter.objType == eMonster) {
                if (!checkMonsterExist(req.hurter.objID, role, currScene)) {
                    string desc = strFormat("修改怪物id。场景%d出现怪物%d", role->getCurrSceneId(), req.hurter.objID);

                    kickAwayRoleWithTips(role, desc.c_str());
                    return;
                }
            }
        }while(false);
        
        //放技能者是主角，检查是否有学习这技能
        if (req.caster.objType == eMainRole) {
            if(!role->getSkillMod()->IsSkillLearn(req.skillID) )
            {
                string desc = Utils::makeStr("clientSkill:%d", req.skillID);
                kickAwayRoleWithTips(role, desc.c_str());
                return;
            }
            
//            LogMod::addLogSkillHurtRecord(role, req.skillID, 0, req.hurtNumber);
        }
        
        //放技能的是宠物
        if (req.caster.objType == ePet) {
        
            //如果是自己的战宠，检查是否有学这技能
            Pet* pet = role->mPetMgr->getPet(req.caster.objID);
            if (pet) {
                if ( !pet->hasSkill(req.skillID) && !pet->isBattleSkill(req.skillID)) {
                    kickAwayRoleWithTips(role, "");
                }
                return;
            }
            
            //如果不是自己的宠物，在以下场景不可能出现宠物
            if (currScene->sceneType == stCopy ||
                currScene->sceneType == stEliteCopy ||
                currScene->sceneType == stDungeon ||
                currScene->sceneType == stLimitTimeKilled ||
                currScene->sceneType == stMysticalCopy ||
                currScene->sceneType == stFamousDunge) {
                
                kickAwayRoleWithTips(role, "");
                return;
            }
            
            //如果是竞技场，释放者有可能是敌人宠物
            if (currScene->sceneType == stPvp) {
                
                VerifyInfo& enemy = role->getEnemyVerifyInfo();
                
                if (req.caster.objID != enemy.activePetid) {
                    kickAwayRoleWithTips(role, "");
                }
                return;
            }
            
            //在好友地下城或者好友副本，可能是好友的宠物
            if (currScene->sceneType == stFriendDunge || currScene->sceneType == stTeamCopy) {
                VerifyInfo* friendInfo = role->getFightedFriendInfo();
                
                if (friendInfo == NULL) {
                    return;
                }
                
                if (req.caster.objID != friendInfo->activePetid) {
                    kickAwayRoleWithTips(role, "");
                }
                return;
            }
            
            // 幻兽试炼场中，释放技能的幻兽只能是出战幻兽里面的
            if (currScene->sceneType == stPetEliteCopy) {
                bool findpet = false;
                for (int i=0; i<role->mCurrentPetElitePets.size(); i++) {
                    if (role->mCurrentPetElitePets[i] == req.caster.objID ) {
                        findpet = true;
                        break;
                    } 
                }
                if(!findpet){
                    kickAwayRoleWithTips(role, "");
                    return;
                }
            }
        }
        
    }
    else
    {
        //shouldn't be here.
    }
    

}}

handler_msg(req_pet_studyskill_prob, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_pet_studyskill_prob ack;
    ack.petid = req.petid;
    ack.skillid = req.skillid;
    
    Pet* pet = role->mPetMgr->getPet(req.petid);
    if (pet == NULL) {
        return;
    }
   
    PetUpSkillProbData *data = pet->getUpSkillProbData().find(req.skillid);
    if (data == NULL) {
        ack.failtimes = 0;
    } else {
        ack.failtimes = data->upSkillFailTimes;
    }
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_role_awake_skill_replace, req)
{
//    hand_Sid2Role_check(sessionid, roleid, role);
    
//    ack_role_awake_skill_replace ack;
//    ack.errorcode = role->getRoleAwake()->skillReplace(req.preSkillId);
//    
//    sendNetPacket(sessionid, &ack);
}}


