//
//  RoleAwake.cpp
//  GameSrv
//
//  Created by pireszhi on 15-4-1.
//
//

#include <stdio.h>
#include "RoleAwake.h"
#include "Role.h"
#include "Skill.h"
#include "DBRolePropName.h"
#include "FashionCollect.h"
#include "EnumDef.h"

RoleAwake* RoleAwake::Create(Role *role)
{
    if (role == NULL) {
        return NULL;
    }
    
    RoleAwake*  awake = new RoleAwake(role);
    return awake;
}

void RoleAwake::init()
{
    RoleAwakeAddtionInfo awakeInfo;
    
    setBatPropAdd();
}

int RoleAwake::skillReplace(int preSkillId)
{
    int newSkillId = 0;
    
    RoleAwakeAddtionInfo awakeInfo;
    RoleAwakeCfg::getRoleAwakeInfo(mMaster->getJob(), mMaster->getRoleAwakeLvl(), awakeInfo);
    
    if (awakeInfo.awakeLvl == 0) {
        return CE_READ_CFG_ERROR;
    }
    
    for (int i = 0; i < awakeInfo.skillsReplace.size(); i++) {
        if( getSkillIndex(preSkillId) == getSkillIndex(awakeInfo.skillsReplace[i].preSkillId))
        {
            newSkillId = getSkillIndex(awakeInfo.skillsReplace[i].preSkillId) + CalcSkillLvl(preSkillId);
            break;
        }
    }
    
    return onSkillReplace(preSkillId, newSkillId);
}

int RoleAwake::allSkillReplace()
{
    RoleAwakeAddtionInfo awakeInfo;
    RoleAwakeCfg::getRoleAwakeInfo(mMaster->getJob(), mMaster->getRoleAwakeLvl(), awakeInfo);
    
    if (awakeInfo.awakeLvl == 0) {
        return CE_READ_CFG_ERROR;
    }
    
    bool saveSkills = false;
    for (int i = 0; i < awakeInfo.skillsReplace.size(); i++) {

        int preSkillId = awakeInfo.skillsReplace[i].preSkillId;
        int newSkillId = awakeInfo.skillsReplace[i].newSkillId;
        
        if (onSkillReplace(preSkillId, newSkillId) == CE_OK)
        {
            saveSkills = true;
        }
    }
    
    if (saveSkills) {
        mMaster->getSkillMod()->saveEquipSkillList(mMaster->getInstID());
        mMaster->getSkillMod()->saveStudySkillList();
    }
    return CE_OK;
}

int RoleAwake::onSkillReplace(int preSkillId, int newSkillId)
{
    if (preSkillId == 0 || newSkillId == 0) {
        return CE_READ_CFG_ERROR;
    }
    
//    if ( !mMaster->getSkillMod()->IsSkillLearn(preSkillId)) {
//        return;
//    }
    
    if (mMaster->getSkillMod()->skillReplace(preSkillId, newSkillId)) {
        return CE_OK;
    }
    
    return CE_ROLE_AWAKE_SKILLREPLACE_ERROR;
}

int RoleAwake::awakeLvlUp()
{
    int nextLvl = mMaster->getRoleAwakeLvl() + 1;
    
    RoleAwakeAddtionInfo awakeInfo;
    RoleAwakeCfg::getRoleAwakeInfo(mMaster->getJob(), nextLvl, awakeInfo);
    
    if (awakeInfo.awakeLvl == 0 && nextLvl > 0) {
        return CE_ROLE_AWAKE_FULL_LVL;
    }
    
    if (mMaster->getLvl() < awakeInfo.needLvl) {
        return CE_YOUR_LVL_TOO_LOW;
    }
    
    ItemArray items;
    GridArray effectGrids;
    
    for (int i = 0; i < awakeInfo.needItem.size(); i++) {
        int myCount = mMaster->getBackBag()->GetItemNum(awakeInfo.needItem[i].itemid);
        
        if (myCount < awakeInfo.needItem[i].count) {
            return CE_ROLE_AWAKE_NEED_ITEM_NOT_ENOUGH;
        }
        
        items.push_back(ItemGroup(awakeInfo.needItem[i].itemid, awakeInfo.needItem[i].count));
    }
    
    if (mMaster->getBackBag()->PreDelItems(items, effectGrids) == false) {
        return CE_ROLE_AWAKE_NEED_ITEM_NOT_ENOUGH;
    }
    
    mMaster->playerDeleteItemsAndStore(effectGrids, items, "role_awake_lvlup", true);
    
    mMaster->setRoleAwakeLvl(nextLvl);
    
    //重新计算属性什么的
    setBatPropAdd();
    
    mMaster->CalcPlayerProp();

    mMaster->saveProp(GetRolePropName(eRoleAwakeLvl), nextLvl);
    
    if (mMaster->getFashionCollect()) {
        mMaster->getFashionCollect()->sendPropAddNotify();
    }
    
    return CE_OK;
}

void RoleAwake::calcAwakeAddProp(BattleProp& bat, BaseProp& base)
{
    bat += mBatProp;
}

void RoleAwake::setRatioAdd(RoleAwakeAddtionInfo &info)
{
    mFashionCollectAdd = info.fashionCollectAdd;    //时装收集属性加成
    mWeaponEnchantsAdd = info.weaponEnchantsAdd;    //武器附魔加成
}

void RoleAwake::setBatPropAdd()
{
    mBatProp = BattleProp();
    
    for (int i = 1; i <= mMaster->getRoleAwakeLvl(); i++) {
        
        RoleAwakeAddtionInfo awakeInfo;
        
        if (!RoleAwakeCfg::getRoleAwakeInfo(mMaster->getJob(), i, awakeInfo)) {
            continue;
        }
        
        //累加各级的属性
        onSetBatPropAdd(awakeInfo);
        
        //加当前等级的时装收集，附魔百分比
        if (awakeInfo.awakeLvl == mMaster->getRoleAwakeLvl()) {
            setRatioAdd(awakeInfo);
        }
    }
    
}

void RoleAwake::onSetBatPropAdd(RoleAwakeAddtionInfo& info)
{
    BattleProp battle;
    
    for (int i = 0; i < info.propAddtion.size(); i++) {
        
        AwakePropAddtion& addtion = info.propAddtion[i];
        
        if( addtion.propName == "atk")
        {
            battle.setAtk(battle.getAtk() + addtion.addRatio);
        }
        else if (addtion.propName == "def")
        {
            battle.setDef(battle.getDef() + addtion.addRatio);
        }
        else if (addtion.propName == "hp")
        {
            battle.setMaxHp(battle.getMaxHp() + addtion.addRatio);
        }
        else if (addtion.propName == "dodge")
        {
            battle.setDodge(battle.getDodge() + addtion.addRatio);
        }
        else if (addtion.propName == "cri")
        {
            battle.setCri(battle.getCri() + addtion.addRatio);
        }
        else if (addtion.propName == "hit")
        {
            battle.setHit(battle.getHit() + addtion.addRatio);
        }
    }
    
    mBatProp += battle;
}

//void RoleAwake::weaponEnchantsAddRatio(BattleProp& bat)
//{
//    bat *= (1 + mWeaponEnchantsAdd);
//}
//void RoleAwake::fashionCollectAddRatio(BattleProp& bat)
//{
//    bat *= (1 + mFashionCollectAdd);
//}

float RoleAwake::getFashionPropAddRatio()
{
    return mFashionCollectAdd;
}

float RoleAwake::getEnchantsAddRatio()
{
    return mWeaponEnchantsAdd;
}
