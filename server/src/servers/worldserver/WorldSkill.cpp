//
//  Skill.cpp
//  GoldOL
//
//  Created by Mike Chang on 12-10-23.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "WorldSkill.h"
#include "log.h"
#include "WorldScene.h"
#include "hander_include.h"
#include "WorldSkillAction.h"
#include "WorldCreature.h"

map<int, world::EFFECTFUNC> SkillEffectMgr::m_effectFunc;

bool SkillEffectMgr::InitEffectTable()
{
    m_effectFunc.clear();
    InsertEffectTable(m_effectFunc);
    if(m_effectFunc.size()==0)  return false;
    return true;
}
world::EFFECTFUNC SkillEffectMgr::GetEffectFunc(SkillAffect effectType)
{
    map<int, world::EFFECTFUNC>::iterator iter;
    iter = SkillEffectMgr::m_effectFunc.find(effectType);
    if(iter != SkillEffectMgr::m_effectFunc.end())
    {
        world::EFFECTFUNC func = iter->second;
        return func;
    }
    return NULL;
}

world::Skill::Skill(int id)
{
    skillid = id;
    bool exist = false;
    skillData = SkillCfg::getCfg(id, exist);
}

/*
 ----------------------------------------------------------------------------------------
 效果小函数，被技能击中的效果可在此表现
 */

bool IsHited(WorldCreature* owner, WorldCreature* target)
{
    /*
    if(target->getDodge() <= 0)
        return true;
    float probability = (float)owner->getHit()/(target->getDodge() + 100);
    if (probability >= 1.0f)
    {
        return true;
    }
    else
    {
        int rol = rand()%100;
        if ((int)(probability*100) > rol)
            return true;
        else{
            return false;
        }
    }
    */
    
    // modified by jianghan for 如果两者属性相差过大会出现基本无法打中的情况，需要有所调整
    if(target->getDodge() <= 0)
        return true;
    float probability = ((float)owner->getHit()+target->mDodgeCount*target->getDodge()/20)/(target->getDodge() + 100);
    if (probability >= 1.0f)
    {
        return true;
    }
    else
    {
        int rol = rand()%100;
        if ((int)(probability*100) > rol){
            if( target->mDodgeCount > 0 )
                target->mDodgeCount -= 2;
            return true;
        }else{
            target->mDodgeCount ++;
            return false;
        }
    }
    // end modify
}

//计算一般技能的伤害值，包含暴击的计算
HitType GetSkillDamageValue(WorldCreature* owner, WorldCreature* target, SkillEffectVal *effect, int& damage)
{
    //bool isCrit;
    float rol1 = rand()%100;
    int dmg = owner->getAtk() * ( 0.95f + 0.1 *(rol1/100));
    int skillAttack = dmg * ( 1 + effect->getAffectFloat()) + effect->getAffectValue();
    
    float DefPro = target->getDef() / (float)(target->getDef() + target->getLvl()*60 + 500);
    
    damage = skillAttack*(1.0f - DefPro);
    damage = damage > 0 ? damage : 1;
    
    damage = skillAttack*(1.0f - DefPro);
    damage = damage > 0 ? damage : 1;
    
    int rol = rand()%10000; //精确到小数点后两位
    float realCri = powf(owner->getCri() / (50 * owner->getLvl()), 0.5);
    
    if ( (int)(realCri * 10000) > rol) {
        damage = damage * 1.5f;
        return eCri;
    }
    return eHit;
}

//计算某技能的合理施法范围，目前只为战斗AI服务
CCRect CalSkillCastRect(WorldCreature* attackObj, SkillCfgDef* skilldata, ObjOrientation atkOrient)
{
    if (skilldata == NULL) {
        return CCRect();
    }
    
    if (skilldata->isBothSide) {
        return CCRect(attackObj->getPositionX() - skilldata->getCastRangeX(), attackObj->getPositionY() - skilldata->getCastRangeY()/2
                      ,2*skilldata->getCastRangeX(),skilldata->getCastRangeY());
    }
    else{
        if (atkOrient == eRight) {
            return CCRect(attackObj->getPositionX() + skilldata->getAttackRectX(), attackObj->getPositionY() - skilldata->getCastRangeY()/2, skilldata->getCastRangeX() - skilldata->getAttackRectX(), skilldata->getCastRangeY());
        }
        else{
            return CCRect(attackObj->getPositionX() - skilldata->getCastRangeX(), attackObj->getPositionY() - skilldata->getCastRangeY()/2, skilldata->getCastRangeX() - skilldata->getAttackRectX(), skilldata->getCastRangeY());
        }
        
    }
}

//计算减血buff，如中毒，的伤害，不计算暴击
int CalDecrHpBuffDamage(WorldCreature* owner, WorldCreature* target, SkillEffectVal *effect)
{
    int skillAttack = owner->getAtk() * ( 1 + effect->getAffectFloat()) + effect->getAffectValue();
    float DefPro = target->BattleProp::getDef() / (float)(target->BattleProp::getDef() + target->getLvl()*60 + 500);
    int damage;
    damage = skillAttack*(1.0f - DefPro);
    damage = damage > 0 ? damage : 1;
    
    return damage;
}

/*
 ------------------------------------------------------------------------------------------------
 Skill的效果处理函数
 ------------------------------------------------------------------------------------------------
 */
void skillDecHp(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect, HitType hurtType);
/*
 ------------------------------------------------------------------
 受创的效果
 ------------------------------------------------------------------
 */

//技能引起的受伤掉血
void DecHp(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect, HitType hurtType, int damage)
{
    target->damage(damage, owner);
    
    // modified by jianghan for 场景伤害集中播放
    /*
     notify_sync_damage notify_dmg;
     obj_attackedTarget atk;
     atk.sourceID = owner->getInstId();
     atk.targetID =  target->getInstId();
     atk.effectLife = target->getHp();
     atk.hitType = hurtType;
     atk.atkIndex = -1;
     notify_dmg.attacked.push_back(atk);
     
     WorldScene* scene = target->getScene();
     if (scene)
     {
     scene->broadcastPacket(&notify_dmg);
     }
     */
    obj_attackedTarget atk;
    atk.sourceID = owner->getInstId();
    atk.targetID =  target->getInstId();
    atk.effectLife = target->getHp();
    atk.hitType = hurtType;
    atk.atkIndex = -1;
    
    WorldScene* scene = target->getScene();
    if (scene){
        scene->addSceneDamage(&atk);
        if( atk.effectLife <= 0 )
            // 如果是挂了，需要立刻发送，免得搞乱逻辑
            scene->doDamageBroadcast();
    }
    // end modify
}

//通用的受伤表现
void UniversalHurt(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect, eAnimationID animation=eAnim_bruise)
{
    // 计算技能伤害值，并判断是不是暴击伤害
    int damage = 0;
    HitType hurtType = GetSkillDamageValue(owner, target, effect, damage);
    if(hurtType != eDodge)
    {
        target->hittedAction(owner, effect->getHitBackTime(), effect->getHitBackDistance(), animation);//受创表现
        //target->standStrongly();
    }
    
    DecHp(owner, target, skillID, effect, hurtType, damage);
}


//普通受击::
void Hitted(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    UniversalHurt(owner, target, skillID, effect);
}

//击退
void KnockBack(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    if(! target->getIsUnhurtState()) //非霸体状态才播放击退效果
    {
        // todo
        if(target->canChangeToState(eSKILL)) //能切换到技能状态才击退。击退也算是技能状态
        {
            WorldScene* scene = target->getScene();
            if (scene == NULL)
            {
                return;
            }
            
            //播放击退效果
            KnockBackAction* skillAct = KnockBackAction::create(owner, target, skillID, effect);
            skillAct->setOwner(owner);
            skillAct->start();
            
            // 下发被技能攻击的消息
            //notify_sync_beskill noti;
            //noti.sourceID = owner->getInstId();
            //noti.targetID = target->getInstId();
            //noti.skill_id = skillID;
            //noti.effectname = effect->affect;
            //noti.x = target->getPositionX();
            //noti.y = target->getPositionY();
            //noti.orient = target->getOrient();
            //scene->broadcastPacket(&noti);
        }
    }
    // 计算技能伤害值，并判断是不是暴击伤害
    int damage = 0;
    HitType hurtType = GetSkillDamageValue(owner, target, effect, damage);
    DecHp(owner, target, skillID, effect, hurtType, damage);
}

//击倒
void KnockDown(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    UniversalHurt(owner, target, skillID, effect, eAnim_fell);
}

//晕眩
void Dizzy(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    
}


/*
 ------------------------------------------------------------------
 冲锋类
 ------------------------------------------------------------------
 */
void Rush(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    //播放冲锋效果
    SkillAction* skillAct = Rush::create(owner, target, skillID, effect);
#if 0
    Game::getObjLayer()->addChild(skillAct);
#endif
    skillAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            SkillAction* skillAct = Rush::create(owner, selfGroupers[i], skillID, effect);
#if 0
            Game::getObjLayer()->addChild(skillAct);
#endif
            skillAct->start();
        }
    }
}

/*
 ------------------------------------------------------------------
 飞行类
 ------------------------------------------------------------------
 */
//一次性飞行效果（撞到物体就消失）
void OnceFlyEffect(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    SkillAction* skillAct = Fly::create(owner, target, skillID, effect);
#if 0
    Game::getObjLayer()->addChild(skillAct);
#endif
    skillAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            SkillAction* skillAct = Fly::create(owner, selfGroupers[i], skillID, effect);
#if 0
            Game::getObjLayer()->addChild(skillAct);
#endif
            skillAct->start();
        }
    }
}

//一次性飞行效果（撞到物体不消失，穿过继续飞行）
void ContinueFlyEffect(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    SkillAction* skillAct = ContinueFly::create(owner, target, skillID, effect);
#if 0
    Game::getObjLayer()->addChild(skillAct);
#endif
    skillAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            SkillAction* skillAct = ContinueFly::create(owner, selfGroupers[i], skillID, effect);
#if 0
            Game::getObjLayer()->addChild(skillAct);
#endif
            skillAct->start();
        }
    }
}


/*
 ------------------------------------------------------------------
 静态特效类
 ------------------------------------------------------------------
 */

//通用的静态技能
void UniversalStaticEffect(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    //没有动作的静态效果服务器不执行
    if (effect->mCharacterAnimation.empty() && effect->getAttackCount() == 0) {
        return;
    }
    SkillAction* skillAct = StaticEffect::create(owner, target, skillID, effect);
#if 0
    Game::getObjLayer()->addChild(skillAct);
#endif
    skillAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            SkillAction* skillAct = StaticEffect::create(owner, selfGroupers[i], skillID, effect);
            skillAct->start();
        }
    }
}

//播放火球爆炸效果
void BigFireExplosion(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    SkillAction* skillAct = HurtEffect::create(owner, target, skillID, effect);
    skillAct->start();
    
    int damage = 0;
    HitType hurtType = GetSkillDamageValue(owner, target, effect, damage);
    //受创表现
    target->hittedAction(owner, effect->getHitBackTime(), effect->getHitBackDistance(), effect->mCharacterAnimation);
    target->standStrongly();
    DecHp(owner, target, skillID, effect, hurtType, damage);
}

void ShakeScreen(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    SkillAction* skillAct = ShakeScreenEffect::create(target, skillID, effect);
#if 0
    Game::getObjLayer()->addChild(skillAct);
#endif
//    target->addChild(skillAct);
    skillAct->start();
}

/*
 ------------------------------------------------------------------
 buff类
 ------------------------------------------------------------------
 */

//根据技能特效的数值改变属性（用当前属性的百分比来更改当前属性的值）
float ChangeAttribute(float originalValue, SkillEffectVal *effect)
{
    float result = originalValue * (1 + effect->getAffectFloat());//先加百分比
    result += effect->getAffectValue();//再加数值
    return result;
}
//根据技能特效的数值还原已经改变的属性
float ResumeAttribute(float originalValue, SkillEffectVal *effect)
{
    float result = originalValue - effect->getAffectValue();//先减数值
    result /= (1 + effect->getAffectFloat());//再减百分比
    return result;
}

//用另一个属性的百分比来改变当前属性
float ChangeAttributeWithAnotherAttribute(float originalValue, float anotherValue, SkillEffectVal *effect)
{
    float result = originalValue  + anotherValue * effect->getAffectFloat();//先加百分比
    result += effect->getAffectValue();//再加数值
    return result;
}
float ResumeAttributeWithAnotherAttribute(float originalValue, float anotherValue, SkillEffectVal *effect)
{
    float result = originalValue - effect->getAffectValue();//先减数值
    result -= anotherValue * effect->getAffectFloat();//再减百分比
    return result;
}

BuffEffect* UniversalAddBuff(WorldCreature* owner, WorldCreature* target,int skillID, SkillEffectVal *effect)
{
    BuffEffect* skillAct = BuffEffect::create(owner, target, skillID, effect);
#if 0
    Game::getObjLayer()->addChild(skillAct);
#endif
    return skillAct;
}

void BuffEffectOnly(WorldCreature* owner, WorldCreature* target,int skillID, SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            BuffEffect* buffAct = UniversalAddBuff(owner, selfGroupers[i], skillID, effect);
            buffAct->start();
        }
    }
}

//更改攻击力
void StartChangeAttack(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
//    target->setAtk(ChangeAttribute(target->getAtk(), effect));
    target->changeAtk(effect->getAffectFloat(), effect->getAffectValue());
}
void EndChangeAttack(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
//    target->setAtk(ResumeAttribute(target->getAtk(), effect));
    target->resumeAtk(effect->getAffectFloat(), effect->getAffectValue());
}
void BuffChangeAttack(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(StartChangeAttack, NULL, EndChangeAttack);
    buffAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            BuffEffect* buffAct = UniversalAddBuff(owner, selfGroupers[i], skillID, effect);
            buffAct->setBuffCallback(StartChangeAttack, NULL, EndChangeAttack);
            buffAct->start();
        }
    }
}

//更改闪避
void StartChangeDogge(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->changeDodge(effect->getAffectFloat(), effect->getAffectValue());
}
void EndChangeDogge(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->resumeDodge(effect->getAffectFloat(), effect->getAffectValue());
}
void BuffChangeDogge(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(StartChangeDogge, NULL, EndChangeDogge);
    buffAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            BuffEffect* buffAct = UniversalAddBuff(owner, selfGroupers[i], skillID, effect);
            buffAct->setBuffCallback(StartChangeDogge, NULL, EndChangeDogge);
            buffAct->start();
        }
    }
}

//更改暴击
void StartChangeCri(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->changeCri(effect->getAffectFloat(), effect->getAffectValue());
}
void EndChangeCri(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->resumeCri(effect->getAffectFloat(), effect->getAffectValue());
}
void BuffChangeCri(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(StartChangeCri, NULL, EndChangeCri);
    buffAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            BuffEffect* buffAct = UniversalAddBuff(owner, selfGroupers[i], skillID, effect);
            buffAct->setBuffCallback(StartChangeCri, NULL, EndChangeCri);
            buffAct->start();
        }
    }
}

//更改防御
void StartChangeDef(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->changeDef(effect->getAffectFloat(), effect->getAffectValue());
}
void EndChangeDef(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->resumeDef(effect->getAffectFloat(), effect->getAffectValue());
}
void BuffChangeDef(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(StartChangeDef, NULL, EndChangeDef);
    buffAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            BuffEffect* buffAct = UniversalAddBuff(owner, selfGroupers[i], skillID, effect);
            buffAct->setBuffCallback(StartChangeDef, NULL, EndChangeDef);
            buffAct->start();
        }
    }
}

//更改命中
void StartChangeHit(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->changeHit(effect->getAffectFloat(), effect->getAffectValue());
}
void EndChangeHit(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->resumeHit(effect->getAffectFloat(), effect->getAffectValue());
}
void BuffChangeHit(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(StartChangeHit, NULL, EndChangeHit);
    buffAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            BuffEffect* buffAct = UniversalAddBuff(owner, selfGroupers[i], skillID, effect);
            buffAct->setBuffCallback(StartChangeHit, NULL, EndChangeHit);
            buffAct->start();
        }
    }
}


//沉默
void StartSilence(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->incSilenceRefCount();
}
void EndSilence(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->decSilenceRefCount();
}
void BuffSilence(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(StartSilence, NULL, EndSilence);
    buffAct->start();
}

//控制效果
void StartControl(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->stopMove();
    target->incControlRefCount();
}
void EndControl(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->decControlRefCount();
}
void BuffControl(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(StartControl, NULL, EndControl);
    buffAct->start();
}

//束缚效果
void StartConstraint(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->stopMove();
    target->incConstraintRefCount();
}
void EndConstraint(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->decConstraintRefCount();
}
void BuffConstraint(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(StartConstraint, NULL, EndConstraint);
    buffAct->start();
}

//更改速度
void StartChangeSpeed(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->changeSpeed(effect->getAffectFloat(), effect->getAffectValue());
    target->notifySpeedFactor();
}
void EndChangeSpeed(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    target->resumeSpeed(effect->getAffectFloat(), effect->getAffectValue());
    target->notifySpeedFactor();
}
void BuffChangeSpeed(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(StartChangeSpeed, NULL, EndChangeSpeed);
    buffAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            BuffEffect* buffAct = UniversalAddBuff(owner, selfGroupers[i], skillID, effect);
            buffAct->setBuffCallback(StartChangeSpeed, NULL, EndChangeSpeed);
            buffAct->start();
        }
    }
}

void IncrHp(WorldCreature* target, int incrHp)
{
    if(target->getHp()+incrHp > target->getMaxHp())
        incrHp = target->getMaxHp() - target->getHp();
    if(incrHp == 0)
        return;

    target->setHp(target->getHp() + incrHp);
    
    // modified by jianghan for 场景伤害集中播放
    /*
     notify_sync_damage notify_dmg;
     obj_attackedTarget atk;
     if (target->getAtker())
     atk.sourceID = target->getAtker()->getInstId();
     else
     atk.sourceID = 0;
     atk.targetID = target->getInstId();
     atk.effectLife = target->getHp();
     atk.hitType = eNoneHitType;
     atk.atkIndex = -1;
     notify_dmg.attacked.push_back(atk);
     WorldScene *scene;
     scene = target->getScene();
     scene->broadcastPacket(&notify_dmg);
     */
    
    obj_attackedTarget atk;
    if (target->getAtker())
        atk.sourceID = target->getAtker()->getInstId();
    else
        atk.sourceID = 0;
    
    atk.targetID = target->getInstId();
    atk.effectLife = target->getHp();
    atk.hitType = eNoneHitType;
    atk.atkIndex = -1;
    
    WorldScene *scene = target->getScene();
    if( scene != NULL ){
        scene->addSceneDamage(&atk);
        if( atk.effectLife <= 0 )
            // 如果有人挂了，需要立刻发送伤害，以免搞乱逻辑
            scene->doDamageBroadcast();
    }
    // end modify

}

//生命回血
void IncrHp(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    int hp = ChangeAttributeWithAnotherAttribute(target->getHp(), target->getMaxHp(), effect);
    if(hp > target->getMaxHp())
        hp = target->getMaxHp();
    
    IncrHp(target, hp - target->getHp());
}

void BuffIncrHp(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(IncrHp, IncrHp, NULL);
    buffAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            BuffEffect* buffAct = UniversalAddBuff(owner, selfGroupers[i], skillID, effect);
            buffAct->setBuffCallback(IncrHp, IncrHp, NULL);
            buffAct->start();
        }
    }
}

//吸血技能
void SuckBlood(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    int dmgOtherNum = target->getDmgOther();//获取伤害记录
    target->setDmgOther(0);//清空伤害记录
    
    int incrHp = dmgOtherNum * effect->getAffectFloat() + effect->getAffectValue();
    IncrHp(target, incrHp);
}

void BuffSuckBlood(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    SuckBloodEffect* skillAct = SuckBloodEffect::create(owner, target, skillID, effect);
#if 0
    Game::getObjLayer()->addChild(skillAct);
#endif
    skillAct->setIncrHpCallback(SuckBlood);
    skillAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            skillAct = SuckBloodEffect::create(owner, selfGroupers[i], skillID, effect);
#if 0
            Game::getObjLayer()->addChild(skillAct);
#endif
            skillAct->setIncrHpCallback(SuckBlood);
            skillAct->start();
        }
    }
}

//中毒
void IntervalBuffPoisoned(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    //中毒过程中不断的掉血
    int dmg;//= effect->getAffectValue();
    dmg = CalDecrHpBuffDamage(owner, target, effect);
    dmg = dmg ? dmg : 1;
    DecHp(owner, target, skillID, effect, eHit, dmg);
}

void BuffPoisoned(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    BuffEffect* buffAct = UniversalAddBuff(owner, target, skillID, effect);
    buffAct->setBuffCallback(NULL, IntervalBuffPoisoned, NULL);
    buffAct->start();
}

///////////////////变身特效////////////////////////
void StartBuffNewBody(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    //添加霸体状态
    target->setIsUnhurtState(true);
}
void EndBuffNewBody(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect)
{
    //消除霸体状态
    target->setIsUnhurtState(false);
}
void BuffNewBody(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    NewBodyEffect* skillAct = NewBodyEffect::create(owner, owner, skillID, effect);
#if 0
    Game::getObjLayer()->addChild(skillAct);
#endif
//    target->addChild(skillAct);
    skillAct->setBuffCallback(StartBuffNewBody, NULL, EndBuffNewBody);
    skillAct->start();
}

///////////////////旋风斩特效////////////////////////
void MovableNewBodyAttack(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    MovableNewBodyAttackEffect* skillAct = MovableNewBodyAttackEffect::create(owner, owner, skillID, effect);
#if 0
    Game::getObjLayer()->addChild(skillAct);
#endif
//    target->addChild(skillAct);
    skillAct->start();
}

///////////////////锁定目标的技能///////////////////////////////////
void LockTarget(WorldCreature* owner, WorldCreature* target, int skillID , SkillEffectVal *effect)
{
    LockTargetEffect* skillAct = LockTargetEffect::create(owner, target, skillID, effect);
#if 0
    Game::getObjLayer()->addChild(skillAct);
#endif
    skillAct->start();
    
    //群体技能
    if(effect->mTargetType == eForSelfGroup)
    {
#if 0
        Scene* scene = SceneManager::getScene(owner->getSceneID());
#endif
        WorldScene *scene = owner->getScene();
        if(scene == NULL)
            return;
        
        vector<WorldCreature*> selfGroupers = scene->getAreaSelfGroup(owner);
        for (int i=0; i<selfGroupers.size(); ++i)
        {
            LockTargetEffect* skillAct = LockTargetEffect::create(owner, target, skillID, effect);
            skillAct->start();
        }
    }
}


void skillDecHp(WorldCreature* owner, WorldCreature* target, int skillID, SkillEffectVal *effect, HitType hurtType)
{
    int dmg;//= effect->getAffectValue();
    dmg = CalDecrHpBuffDamage(owner, target, effect);
    dmg = dmg ? dmg : 1;
    WorldScene *scene;
    scene = owner->getScene();
    DecHp(owner, target, skillID, effect, hurtType, dmg);
}

/*
 ------------------------------------------------------------------
 效果函数的映射表
 ------------------------------------------------------------------
 */
void InsertEffectTable(map < int, world::EFFECTFUNC>& effectTable)
{
    effectTable.clear();
    //攻击类
    effectTable.insert(std::make_pair(eSkillRush,                   Rush));
    effectTable.insert(std::make_pair(eSkillStaticEffect,           UniversalStaticEffect));
    effectTable.insert(std::make_pair(eSkillFlyOnce,                OnceFlyEffect));
    effectTable.insert(std::make_pair(eSkillFlyContinue,            ContinueFlyEffect));
    effectTable.insert(std::make_pair(eSkillBuffEffect,             BuffEffectOnly));
    effectTable.insert(std::make_pair(eSkillNewBody,                BuffNewBody));
    effectTable.insert(std::make_pair(eSkillMovableNewBodyAttack,   MovableNewBodyAttack));
    effectTable.insert(std::make_pair(eSkillLockTargetEffect,       LockTarget));
    
    //纯表现类
    effectTable.insert(std::make_pair(eShakeScreen,                 ShakeScreen));
    
    //伤害类
    effectTable.insert(std::make_pair(eHurtHitted,                  Hitted));
    effectTable.insert(std::make_pair(eHurtKnockBack,               KnockBack));
    effectTable.insert(std::make_pair(eHurtKnockDown,               KnockDown));
    effectTable.insert(std::make_pair(eHurtDizzy,                   Dizzy));
    effectTable.insert(std::make_pair(eHurtFireExplosion,           BigFireExplosion));
    effectTable.insert(std::make_pair(eHurtPoisoned,                BuffPoisoned));
    
    
    //数值类
    effectTable.insert(std::make_pair(eBuffIncrHP,                  BuffIncrHp));
    effectTable.insert(std::make_pair(eBuffSuckBlood,               BuffSuckBlood));
    effectTable.insert(std::make_pair(eBuffChangeAttack,            BuffChangeAttack));
    effectTable.insert(std::make_pair(eBuffChangeDogge,             BuffChangeDogge));
    effectTable.insert(std::make_pair(eBuffChangeCri,               BuffChangeCri));
    effectTable.insert(std::make_pair(eBuffChangeHit,               BuffChangeHit));
    effectTable.insert(std::make_pair(eBuffChangeDef,               BuffChangeDef));
    effectTable.insert(std::make_pair(eBuffChangeSpeed,             BuffChangeSpeed));
    
    effectTable.insert(std::make_pair(eSilenceEffect,               BuffSilence));
    effectTable.insert(std::make_pair(eControlEffect,               BuffControl));
    effectTable.insert(std::make_pair(eConstraintEffect,               BuffConstraint));
}