//
//  SkillAction.cpp
//  GoldOL
//
//  Created by Forest Lin on 13-2-26.
//
//  技能实现类的合集
#include "WorldCreature.h"
#include "WorldGeometry.h"
#include "WorldSceneMgr.h"
#include "WorldSkillFly.h"
#include "WorldSkillAction.h"
#include "WorldScheduler.h"
#include "DataCfg.h"

bool SkillAction::globalInit(int maxObjNum)
{
    g_WorldObjectMgr.init(eWorldObjectSkillAction, maxObjNum);
    return true;
}

///////////////////////////////////////
void SkillAction::addGenarateObj(GameObject *obj)
{
    obj->retain();
    mGenarateObjs.push_back(obj);
}

SkillAction::SkillAction(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
:WorldObject(eWorldObjectSkillAction), mSrcObj(src), mTargetObj(target), mSkillID(skillID), mSkillEffect(skillEffect)
{
    mHasSkillEnd = false;
    setType(eNoSkillAction);
}

SkillAction::~SkillAction()
{
}


void SkillAction::schedule(SEL_SCHEDULE selector, float interval)
{
    WorldObject::schedule(selector, interval * 1000);
}

void SkillAction::scheduleOnce(SEL_SCHEDULE selector, float delay)
{
    WorldObject::scheduleOnce(selector, delay * 1000);
}

void SkillAction::start()
{
    bool exist = false;
    mSkillData = SkillCfg::getCfg(mSkillID, exist);
    
    //使用场景来清除SkillAction，场景自动清除getValid()为false的SkillAction
    mTargetObj->getScene()->addObject(this);
    setValid(exist);
}

void SkillAction::end(float dt)
{
    if (mHasSkillEnd) {
        return;
    }
    mHasSkillEnd = true;
    
    //清除攻击效果
    for (int i = 0; i < mSchedulers.size(); i++)
    {
        unschedule(mSchedulers[i]);
    }
    mSchedulers.clear();
    
    //清除创建的物体
    for(int i = 0; i < mGenarateObjs.size(); ++i)
    {
        if(mGenarateObjs[i] != NULL)
        {
            mGenarateObjs[i]->setValid(false);
            mGenarateObjs[i]->release();
        }
    }
    mGenarateObjs.clear();

    //清除施放者和作用者身上的技能记录
    if(mTargetObj && mTargetObj->getState() != eDEAD)
    {
        mTargetObj->removeSkillAction(getInstId());
    }
    if(mTargetObj != mSrcObj && mSrcObj && mSrcObj->getState() != eDEAD)
    {
        mSrcObj->removeSkillAction(getInstId());
    }
    
    if (FLOAT_GT(mSkillEffect->getLastTime(), mSkillEffect->getCastTime()))
    {
        setValid(false);
    }
}

//施法结束
void SkillAction::castEnd(float dt)
{
    if(mSrcObj && mSrcObj->getState() == eSKILL)
    {
        mSrcObj->clearState();
        //mSrcObj->stand();
    }
    
    if(!FLOAT_GT(mSkillEffect->getLastTime(), mSkillEffect->getCastTime()))
    {
        setValid(false);
    }
}

void SkillAction::clearSelf()
{

}

//强行结束
void SkillAction::endStrongly()
{
    if (mHasSkillEnd) {
        return;
    }
    
    //主动攻击类的技能被强制停止，需要清除角色的技能状态。
    //如果不是主动攻击类的技能，则在子类里重写endStrongly()。
    //待完善，先出版本要紧。---linshusen
    if(mSrcObj && mSrcObj->getState() == eSKILL)
    {
        mSrcObj->clearState();
        mSrcObj->stand();
    }
    this->end(0);
    
    if(mSkillEffect->getCastTime() >= mSkillEffect->getLastTime())
    {
        clearSelf();
    }
}

//检测周围物体，并攻击
void SkillAction::checkAttack(float dt)
{
    //取得技能的攻击范围
    WorldScene* scene = mTargetObj->getScene();
    CheckCondition(scene, return)
    
    if(mSrcObj == NULL || mSrcObj->getState() == eDEAD ||
       mTargetObj == NULL || mTargetObj->getState() == eDEAD)
    {
        return;
    }
    
    bool isHitted = false;
    CCRect atkArea = this->calAttackArea(mTargetObj);
    vector<WorldCreature*> objs = scene->getAttackObjsByArea(mTargetObj, atkArea);
    vector<WorldCreature*>::iterator it = objs.begin();
    for (; it!=objs.end(); it++)
    {
        WorldCreature* hittedObj = *it;
        vector<SkillEffectVal*> hitEffects = SkillCfg::GetEffectFuncByTargetType(mSkillID, eForEnemies);
        //调用受击
        for (int i = 0; i < hitEffects.size(); i++)
        {
            int skillResult = hittedObj->beSkill(mTargetObj, mSkillID, hitEffects[i]);
            if(skillResult > 0)
            {
                isHitted = true;
            }
        }
    }
    
    if(isHitted)
    {
        mTargetObj->addHitStat(); //加连击数
    }
}

//计算攻击范围
CCRect SkillAction::calAttackArea(GameObject* attackObj)
{
    CCRect attackArea;
    attackArea.size = CCSize(mSkillData->getAttackRectWidth(), mSkillData->getAttackRectHeight());
    
    bool isFaceLeft = attackObj->isFaceLeft();
    CCPoint ori;
    
    if(mSkillData->isBothSide)
    {
        ori.x = attackObj->getPositionX() - mSkillData->getAttackRectWidth() * 0.5;
        ori.y = attackObj->getPositionY() - attackArea.size.height * 0.5f + mSkillData->getAttackRectY();
    }
    else
    {
        if(isFaceLeft)
            ori.x = attackObj->getPositionX() - mSkillData->getAttackRectX() - mSkillData->getAttackRectWidth();
        else
            ori.x = attackObj->getPositionX() + mSkillData->getAttackRectX();
        ori.y = attackObj->getPositionY() - attackArea.size.height * 0.5f + mSkillData->getAttackRectY();
    }
    
    attackArea.origin = ori;
    
    return attackArea;
}
///////////////////////////////冲锋类///////////////////////////////
Rush* Rush::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    Rush* skillAction = new Rush(src, target, skillID, skillEffect);
    return skillAction;
}

//开始技能
void Rush::start()
{
    SkillAction::start();
    CheckCondition(mSkillData && mSkillEffect, return)
    
    mTargetObj->addSkillAction(this); //施放者和作用者记录下此技能，以便他死亡时清除技能
    if(mTargetObj != mSrcObj)
    {
        mSrcObj->addSkillAction(this);
    }
    
    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(Rush::perform);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
    {
        this->perform(0);
    }
}

//执行
void Rush::perform(float dt)
{
    float distance = mSkillEffect->getMoveLength();
    float speed = mSkillEffect->getSpeed();
    
    //冲锋结束的回调
    mSkillEffect->setLastTime(distance/speed);
    scheduleOnce(schedule_selector(Rush::end), mSkillEffect->getLastTime());
    
    //定时攻击频率检测
    SEL_SCHEDULE seleter = schedule_selector(SkillAction::checkAttack);
    mSchedulers.push_back(seleter);
    if(mSkillEffect->getAttackCount() > 1)//次数大于1，说明是持续性攻击
    {
        schedule(seleter, mSkillEffect->getAttackIntervalTime());
    }
    else    //冲锋是一次性攻击？可能性不大
    {
        scheduleOnce(seleter, mSkillEffect->getAttackIntervalTime());
    }
    
    //角色冲锋
    mTargetObj->setActionMoveSpeed(speed);//先设置冲锋速度，因为受击者需要这个速度后退
    if(mSkillEffect->getAttackCount() > 1)//次数大于1，说明是持续性攻击,才需要提前攻击检测
    {
        SkillAction::checkAttack(0);
    }
    
    if(! mSkillEffect->mCharacterAnimation.empty())
    {
        //有时候在冲锋之前角色时正在受击状态，受击时无法冲锋出来。为了保证可以冲锋起来，要清除状态。
        mTargetObj->clearState();
        
        mTargetObj->skillWalkTo(mTargetObj->getForwardPos(distance), mSkillEffect->mCharacterAnimation, speed);
        mTargetObj->setIsKeepStateWhenAnimationEnd(true); //冲锋动作结束，依然保持技能状态
    }
}

//冲锋结束
void Rush::end(float dt)
{
    if (mHasSkillEnd) {
        return;
    }
    
    unschedule(schedule_selector(SkillAction::checkAttack));
    
    //冲锋结束后，结束施法
    if(mTargetObj->getState() == eSKILL)
    {
        mTargetObj->clearState();
    }
    mTargetObj->stand();

    SkillAction::end(dt);
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
    {
        SkillAction::clearSelf();
    }
}

///////////////////////////////飞行类技能//////////////////////////////////////////
Fly* Fly::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    Fly* skillAction = new Fly(src, target, skillID, skillEffect);
    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //{
    //    sAllSkillActions[skillAction->getID()] = skillAction;
    //}
   
    return skillAction;
}

//开始技能
void Fly::start()
{
    //log_info("Fly::start!!!!");
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    mTargetObj->addSkillAction(this); //施放者和作用者记录下此技能，以便他死亡时清除技能
    if(mTargetObj != mSrcObj)
        mSrcObj->addSkillAction(this);
    
    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(Fly::perform);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->perform(0);
    
    
}

//执行
void Fly::perform(float dt)
{
    //log_info("Fly::perform!!!!");
    if(!mSkillEffect)
        return;
    
    float distance = mSkillEffect->getMoveLength();
    float speed = mSkillEffect->getSpeed();
    
    //角色动作
    if(! mSkillEffect->mCharacterAnimation.empty())
        mTargetObj->skillStand(mSkillEffect->mCharacterAnimation);
    
    //施法
    if(mSkillEffect->getCastTime() > 0)
    {
        scheduleOnce(schedule_selector(SkillAction::castEnd), mSkillEffect->getCastTime());
        mTargetObj->setIsKeepStateWhenAnimationEnd(true);
    }
    
    //定时攻击检测
    SEL_SCHEDULE seleter = schedule_selector(Fly::checkAttack);
    mSchedulers.push_back(seleter);
    schedule(seleter, mSkillEffect->getAttackIntervalTime());
    
    //未击中目标后，结束的回调
    mSkillEffect->setLastTime(distance/speed);
    scheduleOnce(schedule_selector(Fly::end), mSkillEffect->getLastTime());
    
    
    //飞行特效
    if(ResCfg::sAllRes.find(mSkillEffect->getEffectResId()) != ResCfg::sAllRes.end())
    {
        mFlyObj = WorldSkillFly::create();//eSkillEffect, mSkillEffect->getEffectResId());
        addGenarateObj(mFlyObj);
        
        //飞行物的位置在释放者包围盒上沿出现
        mFlyObj->setPosition(mTargetObj->getForwardPos(mSkillEffect->getAppearOffset()));
        //mFlyObj->setPosition(ccpAdd(mFlyObj->getPosition()
        //                            , ccp(0, mTargetObj->getBoundingBox().size.height))
        //                     );
        mFlyObj->setOrient(mTargetObj->getOrient());
        mFlyObj->enterScene(mTargetObj->getScene());
        mFlyObj->setAreaId(mTargetObj->getAreaId());
        
        CCPoint dest = mFlyObj->getForwardPos(distance);
        mFlyObj->moveTo(speed, dest);
    }
    
    checkAttack(0.0);
}

//技能结束
void Fly::end(float dt)
{
    //log_info("Fly::end!!!!");
    
    if (mHasSkillEnd) {
        return;
    }
    
    
    unschedule(schedule_selector(Fly::end));
    
    SkillAction::end(dt);
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
}

CCRect Fly::calAttackArea(GameObject* attackObj)
{
    CCRect rect = SkillAction::calAttackArea(attackObj);
    if (mTargetObj) {
        rect.origin = ccpAdd(rect.origin, ccp(0, mTargetObj->getBoundingBox().size.height));
    }
    return rect;
}


void Fly::checkAttack(float dt)
{
    if(mFlyObj == NULL)
        return;
    
    WorldScene* scene = mTargetObj->getScene();
    CheckCondition(scene, return)

    bool isHitTarget = false;
    bool isRealHitted = false;
    //取得技能的攻击范围
    WorldObjectType attackType = mTargetObj->getType();
    vector<WorldCreature*> objs = scene->getAttackObjsByArea(mTargetObj, attackType, this->calAttackArea(mFlyObj));
    vector<WorldCreature*>::iterator it = objs.begin();
    for (; it!=objs.end(); it++)
    {
        WorldCreature* hittedObj = *it;
        
        isHitTarget = true;
        
        vector<SkillEffectVal*> hitEffects = SkillCfg::GetEffectFuncByTargetType(mSkillID, eForEnemies);
        //调用受击的特效
        for (int i = 0; i < hitEffects.size(); i++)
        { 
            int skillResult = hittedObj->beSkill(mTargetObj, mSkillID, hitEffects[i]);
            if(skillResult > 0)
                isRealHitted = true;
        }

    }
    if(isRealHitted)
        mTargetObj->addHitStat();//加连击数
    
    //飞行物击中了物体，结束飞行特效
    if(isHitTarget)
    {
        this->end(0);
    }
}

///////////////////////////////持续性攻击的飞行类技能//////////////////////////////////////////
ContinueFly* ContinueFly::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    ContinueFly* skillAction = new ContinueFly(src, target, skillID, skillEffect);    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //{
    //    sAllSkillActions[skillAction->getID()] = skillAction;
    //}
    
    return skillAction;
}

//开始技能
void ContinueFly::start()
{
    //log_info("ContinueFly::start!!!!");
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    mTargetObj->addSkillAction(this); //施放者和作用者记录下此技能，以便他死亡时清除技能
    if(mTargetObj != mSrcObj)
        mSrcObj->addSkillAction(this);
    
    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(ContinueFly::checkAttackFirst);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->checkAttackFirst(0);
    
    
}

void ContinueFly::checkAttackFirst(float dt)
{
    //因为速度比较快，近身的敌人很难击中
    //所以先执行攻击检测，过一段时间后，再开始飞行。
    
   /* SEL_SCHEDULE seleter = schedule_selector(ContinueFly::checkAttack);
    mSchedules.push_back(seleter);
    
    schedule(seleter, mSkillEffect->getAttackIntervalTime());
    
    //过一段时间再开始冲锋
    SEL_SCHEDULE seleter2 = schedule_selector(ContinueFly::perform);
    mSchedules.push_back(seleter2);
    scheduleOnce(seleter2, mSkillEffect->getAttackIntervalTime());*/
    
    ContinueFly::perform(0);
    
    if(mSkillEffect->getAttackCount() != 0)
    {
        checkAttack(0.0);
        //定时攻击频率检测
        SEL_SCHEDULE seleter = schedule_selector(ContinueFly::checkAttack);
        mSchedulers.push_back(seleter);
        schedule(seleter, mSkillEffect->getAttackIntervalTime());
    }
}

//执行
void ContinueFly::perform(float dt)
{
    //log_info("ContinueFly::perform!!!!");
    if(!mSkillEffect)
        return;
    
    float distance = mSkillEffect->getMoveLength();
    float speed = mSkillEffect->getSpeed();
    
    //角色动作
    if(! mSkillEffect->mCharacterAnimation.empty())
        mTargetObj->skillStand(mSkillEffect->mCharacterAnimation);
    
    //未击中目标后，结束的回调
    mSkillEffect->setLastTime(distance/speed);
    scheduleOnce(schedule_selector(ContinueFly::end), mSkillEffect->getLastTime());
    
    //飞行特效
    //if(ResCfg::sAllRes.find(mSkillEffect->getEffectResId()) != ResCfg::sAllRes.end())
    {
        mFlyObj = WorldSkillFly::create();//eSkillEffect, mSkillEffect->getEffectResId());
        addGenarateObj(mFlyObj);
        
        //飞行物的位置在释放者包围盒上沿出现
        mFlyObj->setPosition(mTargetObj->getForwardPos(mSkillEffect->getAppearOffset()));
        //mFlyObj->setPosition(ccpAdd(mFlyObj->getPosition()
        //                            , ccp(0, mTargetObj->getBoundingBox().size.height))
         //                    );
        
        mFlyObj->setOrient(mTargetObj->getOrient());
        mFlyObj->enterScene(mTargetObj->getScene());
        mFlyObj->setAreaId(mTargetObj->getAreaId());
        
        CCPoint dest = mFlyObj->getForwardPos(distance);
        mFlyObj->moveTo(speed, dest);
    }
}

//技能结束
void ContinueFly::end(float dt)
{
    //log_info("ContinueFly::end!!!!");
    
    if (mHasSkillEnd) {
        return;
    }
    
    
    unschedule(schedule_selector(ContinueFly::end));
    SkillAction::end(dt);
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
}


CCRect ContinueFly::calAttackArea(GameObject* attackObj)
{
    CCRect rect = SkillAction::calAttackArea(attackObj);
    if (mTargetObj) {
        rect.origin = ccpAdd(rect.origin, ccp(0, mTargetObj->getBoundingBox().size.height));
    }
    return rect;
}


void ContinueFly::checkAttack(float dt)
{
    if(mFlyObj == NULL)
        return;
 
    
    
    WorldScene* scene = mTargetObj->getScene();
    if (scene == NULL)
        return;
    
    bool isHitted = false;
    //取得技能的攻击范围
    WorldObjectType attackType = mTargetObj->getType();
    vector<WorldCreature*> objs = scene->getAttackObjsByArea(mTargetObj, attackType, this->calAttackArea(mFlyObj));
    vector<WorldCreature*>::iterator it = objs.begin();
    for (; it!=objs.end(); it++)
    {
        WorldCreature* hittedObj = *it;
        
        vector<SkillEffectVal*> hitEffects = SkillCfg::GetEffectFuncByTargetType(mSkillID, eForEnemies);
        //调用受击的特效
        for (int i = 0; i < hitEffects.size(); i++)
        {
            int skillResult = hittedObj->beSkill(mTargetObj, mSkillID, hitEffects[i]);
            if(skillResult > 0)
                isHitted = true;
        }
    }
    
    if(isHitted)
        mTargetObj->addHitStat(); //加连击数
}



///////////////////////////////攻击固定特效类////////////////////////////////////////////
StaticEffect* StaticEffect::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    StaticEffect* skillAction = new StaticEffect(src, target, skillID, skillEffect);    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //    sAllSkillActions[skillAction->getID()] = skillAction;
    
    return skillAction;
}

//开始技能
void StaticEffect::start()
{
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    mTargetObj->addSkillAction(this); //施放者和作用者记录下此技能，以便他死亡时清除技能
    if(mTargetObj != mSrcObj)
        mSrcObj->addSkillAction(this);
    
    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(StaticEffect::perform);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->perform(0);
    
    
}

//执行
void StaticEffect::perform(float dt)
{
    if(!mSkillEffect)
        return;
    
    //角色动作
    if(! mSkillEffect->mCharacterAnimation.empty())
        mTargetObj->skillStand(mSkillEffect->mCharacterAnimation);
    
    //攻击频率检测
    SEL_SCHEDULE seleter = schedule_selector(SkillAction::checkAttack);
    mSchedulers.push_back(seleter);
    if(mSkillEffect->getAttackCount() > 1 && mSkillEffect->getAttackIntervalTime() > 0)//次数大于1，说明是持续性攻击
        schedule(seleter, mSkillEffect->getAttackIntervalTime());
    else if(mSkillEffect->getAttackIntervalTime() > 0)//一次性攻击
        scheduleOnce(seleter, mSkillEffect->getAttackIntervalTime());
    
    //施法结束的回调
    if(mSkillEffect->getCastTime() > 0)
    {
        scheduleOnce(schedule_selector(SkillAction::castEnd), mSkillEffect->getCastTime());
        mTargetObj->setIsKeepStateWhenAnimationEnd(true);
    }
    //效果结束的回调
    scheduleOnce(schedule_selector(StaticEffect::end), mSkillEffect->getLastTime());
}

//技能结束
void StaticEffect::end(float dt)
{
    //log_info("StaticEffect::end!!!!");
    
    if (mHasSkillEnd) {
        return;
    }
    
    unschedule(schedule_selector(StaticEffect::end));
    SkillAction::end(dt);
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
    
}

///////////////////////////////伤害特效类技能////////////////////////////////////////////
HurtEffect* HurtEffect::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    HurtEffect* skillAction = new HurtEffect(src, target, skillID, skillEffect);
    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //    sAllSkillActions[skillAction->getID()] = skillAction;
    
    return skillAction;
}

//开始技能
void HurtEffect::start()
{
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    mTargetObj->addSkillAction(this); //施放者和作用者记录下此技能，以便他死亡时清除技能
    if(mSrcObj != mTargetObj)
        mSrcObj->addSkillAction(this);
    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(HurtEffect::perform);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->perform(0);
    
    
}

//执行
void HurtEffect::perform(float dt)
{
    if(!mSkillEffect)
        return;
    
    if (!mTargetObj->getIsUnhurtState())
    {
        float distance = mSkillEffect->mHitBackDistance;
        float lastTime = mSkillEffect->mHitBackTime;
        float speed = distance / speed;
        
    }
    
    //角色动作
//    if(! mSkillEffect->mCharacterAnimation.empty())
//    {
//        //非霸体状态才播放受创动作
//        if(! mTargetObj->getIsUnhurtState())
//        {
//            mTargetObj->skillStand(mSkillEffect->mCharacterAnimation);
//            mTargetObj->playHittedScream();
//        }
//        
//        //mTargetObj->playHittedSound();
//    }
    
    //频率检测
    SEL_SCHEDULE seleter = schedule_selector(SkillAction::checkAttack);
    mSchedulers.push_back(seleter);
    if(mSkillEffect->getAttackCount() > 1 && mSkillEffect->getAttackIntervalTime() > 0)//次数大于1，说明是持续性伤害
        schedule(seleter, mSkillEffect->getAttackIntervalTime());
    else if(mSkillEffect->getAttackIntervalTime() > 0)//一次性伤害
        scheduleOnce(seleter, mSkillEffect->getAttackIntervalTime());
    
    //效果结束的回调
    scheduleOnce(schedule_selector(HurtEffect::end), mSkillEffect->getLastTime());
}

//技能结束
void HurtEffect::end(float dt)
{
    //log_info("StaticEffect::end!!!!");
    
    if (mHasSkillEnd) {
        return;
    }
    
    
    unschedule(schedule_selector(HurtEffect::end));
    SkillAction::end(dt);
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
}

///////////////////////////////BUF特效类////////////////////////////////////////////
BuffEffect* BuffEffect::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    BuffEffect* skillAction = new BuffEffect(src, target, skillID, skillEffect);
    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //    sAllSkillActions[skillAction->getID()] = skillAction;
     
    return skillAction;
}

//开始技能
void BuffEffect::start()
{
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    //施放者和作用者记录下此技能，以便他死亡时清除技能
    mTargetObj->addSkillAction(this); 
    if(mTargetObj != mSrcObj)
        mSrcObj->addSkillAction(this);
    
    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(BuffEffect::perform);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->perform(0);
}

//执行
void BuffEffect::perform(float dt)
{
    if(!mSkillEffect)
        return;
    
#if 0
    //角色动作，一般的buff角色没有动作
    if(! mSkillEffect->mCharacterAnimation.empty())
    {
        //非霸体状态才播放受创动作
        if(! mTargetObj->getIsUnhurtState())
        {
            mTargetObj->skillStand(mSkillEffect->mCharacterAnimation);
            mTargetObj->playHittedScream();
        }
    }
#endif
    
    //buff频率检测
    SEL_SCHEDULE seleter = schedule_selector(BuffEffect::intervalBufCheck);
    mSchedulers.push_back(seleter);
    
    //次数大于1，说明是持续性buff
    if(mSkillEffect->getAttackCount() > 1 && mSkillEffect->getAttackIntervalTime() > 0)
        schedule(seleter, mSkillEffect->getAttackIntervalTime());
    
    //效果结束的回调
    scheduleOnce(schedule_selector(BuffEffect::end), mSkillEffect->getLastTime());
    
    //buff开始的回调
    mStarted = true;
    if(mBuffStartCallback)
        mBuffStartCallback(mSrcObj, mTargetObj, mSkillID, mSkillEffect);
}

//技能结束
void BuffEffect::end(float dt)
{
    if (mHasSkillEnd) {
        return;
    }
    
    //log_info("BuffEffect::end!!!!");
    unschedule(schedule_selector(BuffEffect::end));
    
    //buff结束的回调
    if(mBuffEndCallback && mStarted)
        mBuffEndCallback(mSrcObj, mTargetObj, mSkillID, mSkillEffect);
    
    SkillAction::end(dt);
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
}

//强行结束
void BuffEffect::endStrongly()
{
    if (mHasSkillEnd) {
        return;
    }
    
    this->end(0);
    if(mSkillEffect->getCastTime() >= mSkillEffect->getLastTime())
        this->clearSelf();
}


void BuffEffect::intervalBufCheck()
{
    //死亡后自动停止buff
    if(mTargetObj==NULL || mTargetObj->getState() == eDEAD)
    {
        this->end(0);
        return;
    }
    
    if(mBuffIntervalCallback)
        mBuffIntervalCallback(mSrcObj, mTargetObj, mSkillID, mSkillEffect);
}
///////////////////////////////吸血特效类////////////////////////////////////////////
SuckBloodEffect* SuckBloodEffect::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    SuckBloodEffect* skillAction = new SuckBloodEffect(src, target, skillID, skillEffect);
    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //    sAllSkillActions[skillAction->getID()] = skillAction;
    
    return skillAction;
}

void SuckBloodEffect::start()
{
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    //施放者和作用者记录下此技能，以便他死亡时清除技能
    mTargetObj->addSkillAction(this);
    if(mTargetObj != mSrcObj)
        mSrcObj->addSkillAction(this);
    
    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(SuckBloodEffect::perform);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->perform(0);
}

void SuckBloodEffect::perform(float dt)
{
    if(!mSkillEffect)
        return;
    
    SEL_SCHEDULE seleter = schedule_selector(SuckBloodEffect::intervalCheck);
    mSchedulers.push_back(seleter);
    schedule(seleter, 0.5); //间歇的加血
    
    mTargetObj->setIsRecordDmgOther(true);
    
    //效果结束的回调
    scheduleOnce(schedule_selector(SuckBloodEffect::end), mSkillEffect->getLastTime());
}

void SuckBloodEffect::end(float dt)
{
    if (mHasSkillEnd) {
        return;
    }
    
    unschedule(schedule_selector(SuckBloodEffect::end));
    mTargetObj->setIsRecordDmgOther(false);
    mTargetObj->setDmgOther(0);//清空伤害记录
    
    SkillAction::end(dt);
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
}

void SuckBloodEffect::intervalCheck(float dt)
{
    if(mIncrHpCallback != NULL)
        mIncrHpCallback(mSrcObj, mTargetObj, mSkillID, mSkillEffect);
}

void SuckBloodEffect::endStrongly()
{
    if (mHasSkillEnd) {
        return;
    }
    
    this->end(0);
    if(mSkillEffect->getCastTime() >= mSkillEffect->getLastTime())
        this->clearSelf();
}


///////////////////////////////变身特效类////////////////////////////////////////////
NewBodyEffect* NewBodyEffect::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    NewBodyEffect* skillAction = new NewBodyEffect(src, target, skillID, skillEffect);
    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //    sAllSkillActions[skillAction->getID()] = skillAction;
      
    return skillAction;
}

//开始技能
void NewBodyEffect::start()
{
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    mTargetObj->addSkillAction(this); //施放者和作用者记录下此技能，以便他死亡时清除技能
    if (mSrcObj != mTargetObj)
    {
        mSrcObj->addSkillAction(this);
    }

    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(NewBodyEffect::perform);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->perform(0);
    
    
}

//执行
void NewBodyEffect::perform(float dt)
{
    if(!mSkillEffect)
        return;
    
    //效果结束的回调
    scheduleOnce(schedule_selector(NewBodyEffect::end), mSkillEffect->getLastTime());
    
    //buff开始的回调
    if(mBuffStartCallback)
        mBuffStartCallback(mSrcObj, mTargetObj, mSkillID, mSkillEffect);
}

//技能结束
void NewBodyEffect::end(float dt)
{
    if (mHasSkillEnd) {
        return;
    }
    
    //log_info("BuffEffect::end!!!!");
#if 0
    if(mNewBody)
        mTargetObj->endChangeBody(mNewBody);
#endif
    
    unschedule(schedule_selector(NewBodyEffect::end));

    //buff结束的回调
    if(mBuffEndCallback)
        mBuffEndCallback(mSrcObj, mTargetObj, mSkillID, mSkillEffect);
    
    SkillAction::end(dt);
    /*以下2行代码是以前变身状态有bug时的临时代码，现在bug已经解决，不需要重建骨架---linshusen*/
//    mTargetObj->rebuildArmature(); //变身被清除之后，要重建骨架
//    mTargetObj->standStrongly();   //强制回到站立状态。因为上一句内部播放了站立动作，如果此时不在站立状态就会出问题。
    
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
}


///////////////////////////////可移动的变身类攻击特效 如：旋风斩////////////////////////////////////////////

MovableNewBodyAttackEffect* MovableNewBodyAttackEffect::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    MovableNewBodyAttackEffect* skillAction = new MovableNewBodyAttackEffect(src, target, skillID, skillEffect);
    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //    sAllSkillActions[skillAction->getID()] = skillAction;
        
    return skillAction;
}

//开始技能
void MovableNewBodyAttackEffect::start()
{
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    mTargetObj->addSkillAction(this); //施放者和作用者记录下此技能，以便他死亡时清除技能
    if (mSrcObj != mTargetObj)
    {
        mSrcObj->addSkillAction(this);
    }

    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(MovableNewBodyAttackEffect::perform);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->perform(0);
    
    
}

void MovableNewBodyAttackEffect::perform(float dt)
{
    //角色动作，虽然这里不需要角色做动作，但是还是要调用一下。
    //因为这样能保证执行一次skill的action，从而清除掉上一个状态和action。
    //否则上一个action未被清除，会引起错误。
    //例如战士旋风斩和普通攻击一起按下，旋风斩会消失。--- linshusen
    mTargetObj->skillStand(mSkillEffect->mCharacterAnimation);
    
    //攻击频率检测
    SEL_SCHEDULE seleter = schedule_selector(SkillAction::checkAttack);
    mSchedulers.push_back(seleter);
    if(mSkillEffect->getAttackCount() > 1 && mSkillEffect->getAttackIntervalTime() > 0)//次数大于1，说明是持续性攻击
        schedule(seleter, mSkillEffect->getAttackIntervalTime());
    else if(mSkillEffect->getAttackIntervalTime() > 0)//一次性攻击
        scheduleOnce(seleter, mSkillEffect->getAttackIntervalTime());
    
    //效果结束的回调
    scheduleOnce(schedule_selector(MovableNewBodyAttackEffect::end), mSkillEffect->getLastTime());
    
    //生成特效
    if(ResCfg::sAllRes.find(mSkillEffect->getEffectResId()) != ResCfg::sAllRes.end())
    {
        mTargetObj->startSkillMoveState(); //技能激动状态
        mTargetObj->setIsKeepStateWhenAnimationEnd(true);
    }
}

void MovableNewBodyAttackEffect::end(float dt)
{
    if (mHasSkillEnd) {
        return;
    }
    
    mTargetObj->endSkillMoveState();
    SkillAction::castEnd(0);
    unschedule(schedule_selector(MovableNewBodyAttackEffect::end));
    
    //buff结束的回调
    if(mBuffEndCallback)
        mBuffEndCallback(mSrcObj, mTargetObj, mSkillID, mSkillEffect);
    
    SkillAction::end(dt);
    /*以下2行代码是以前变身状态有bug时的临时代码，现在bug已经解决，不需要重建骨架---linshusen*/
//    mTargetObj->rebuildArmature(); //变身被清除之后，要重建骨架
//    mTargetObj->standStrongly();   //强制回到站立状态。因为上一句内部播放了站立动作，如果此时不在站立状态就会出问题。
    
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
}

///////////////////////////////锁定目标类技能////////////////////////////////////////////
//比较被锁定的目标 重载() ，用于给多个目标排序
struct LockTargetCmp
{
    WorldCreature* mSrcObj;
    
    LockTargetCmp(WorldCreature* srcObj) : mSrcObj(srcObj){}
    
    //按照每个目标与技能施放者的距离排序，升序排列
    bool operator()(WorldCreature* target1, WorldCreature* target2)
    {
        float distance1 = ccpDistanceSQ(mSrcObj->getPosition(), target1->getPosition());
        float distance2 = ccpDistanceSQ(mSrcObj->getPosition(), target2->getPosition());
        
        return distance1 < distance2;
    }
};

LockTargetEffect* LockTargetEffect::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    LockTargetEffect* skillAction = new LockTargetEffect(src, target, skillID, skillEffect);
    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //    sAllSkillActions[skillAction->getID()] = skillAction;
    
    return skillAction;
}

void LockTargetEffect::start()
{
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    //施放者和作用者记录下此技能，以便他死亡时清除技能
    mTargetObj->addSkillAction(this);
    if(mTargetObj != mSrcObj)
        mSrcObj->addSkillAction(this);
    
    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(LockTargetEffect::perform);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->perform(0);
}

void LockTargetEffect::perform(float dt)
{
    if(!mSkillEffect)
        return;
    
    //效果结束的回调
    scheduleOnce(schedule_selector(LockTargetEffect::end), mSkillEffect->getLastTime());
    
    //攻击频率检测
    SEL_SCHEDULE seleter = schedule_selector(LockTargetEffect::checkAttack);
    mSchedulers.push_back(seleter);
    //次数大于1，说明是持续性攻击
    if(mSkillEffect->getAttackCount() > 1 && mSkillEffect->getAttackIntervalTime() > 0)
        schedule(seleter, mSkillEffect->getAttackIntervalTime());
    else if(mSkillEffect->getAttackIntervalTime() > 0)//一次性攻击
        scheduleOnce(seleter, mSkillEffect->getAttackIntervalTime());
    
#if 0
    //取得所有的攻击目标
    Scene* scene = SceneManager::getScene(mSceneID);
#else
    WorldScene* scene = mTargetObj->getScene();
#endif
    if (scene == NULL)
        return;
    vector<WorldCreature*> allTargets = scene->getAreaCanAttackObjs(mTargetObj, mTargetObj->getType());
    std::sort(allTargets.begin(), allTargets.end(), LockTargetCmp(mTargetObj));//按照距离升序排列
    
    //真正的攻击目标列表
    vector<WorldCreature*> realTargets;
    if(mSkillEffect->getTargetNum() < 0) //攻击个数小于0，说明不攻击任何目标
        return;
    else if (mSkillEffect->getTargetNum() == 0)//攻击个数等于0，说明攻击当前段的所有目标
        realTargets = allTargets;
    else    //攻击个数大于0，说明从所有目标中选取最近的targetNum个目标攻击
    {
        for (int i=0; i<allTargets.size(); ++i)
        {
            if(i < mSkillEffect->getTargetNum())
                realTargets.push_back(allTargets[i]);
        }
    }
    //保存各个锁定目标的位置
    for (int i=0; i<realTargets.size(); ++i)
    {
        CCPoint pos = realTargets[i]->getPosition();
        mLockPoints.push_back(pos);
    }
}

void LockTargetEffect::end(float dt)
{
    if (mHasSkillEnd) {
        return;
    }
    
    unschedule(schedule_selector(LockTargetEffect::end));
    
    SkillAction::end(dt);
    mLockPoints.clear();
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
}

void LockTargetEffect::checkAttack(float dt)
{
    WorldScene* scene = mTargetObj->getScene();
    if (scene == NULL)
        return;
    
    if(mSrcObj == NULL || mSrcObj->getState() == eDEAD || mTargetObj == NULL || mTargetObj->getState() == eDEAD)
        return;
    
    WorldObjectType attackType = mTargetObj->getType();
    
    //由于锁定了多个目标位置，对多个目标位置分别检测伤害
    for (int objIndex = 0; objIndex<mLockPoints.size(); ++objIndex)
    {
        bool isHitted = false;
        //锁定位置后，发起攻击的物体多数情况是光效，但是有时会没有光效。所以用NULL，只要保证attackType不是宠物类型，就不会出事。
        //服务器使用技能的发起者发起攻击
        vector<WorldCreature*> objs = scene->getAttackObjsByArea(mSrcObj, attackType, calAttackArea(mLockPoints[objIndex]));
        vector<WorldCreature*>::iterator it = objs.begin();
        for (; it!=objs.end(); it++)
        {
            WorldCreature* hittedObj = *it;
            vector<SkillEffectVal*> hitEffects = SkillCfg::GetEffectFuncByTargetType(mSkillID, eForEnemies);
            //调用受击
            for (int i = 0; i < hitEffects.size(); i++)
            {
                int skillResult = hittedObj->beSkill(mTargetObj, mSkillID, hitEffects[i]);
                if(skillResult > 0)
                    isHitted = true;
            }
        }
        
        if(isHitted)
            mTargetObj->addHitStat(); //加连击数
    }
}

//根据一个位置来计算攻击区域
CCRect LockTargetEffect::calAttackArea(CCPoint pos)
{
    CCRect attackArea;
    attackArea.size = CCSize(mSkillData->getAttackRectWidth(), mSkillData->getAttackRectHeight());
    
    bool isFaceLeft = true;
    CCPoint ori;
    
    if(mSkillData->isBothSide)
    {
        ori.x = pos.x - mSkillData->getAttackRectWidth() * 0.5;
        ori.y = pos.y - attackArea.size.height * 0.5f + mSkillData->getAttackRectY();
    }
    else
    {
        if(isFaceLeft)
            ori.x = pos.x - mSkillData->getAttackRectX() - mSkillData->getAttackRectWidth();
        else
            ori.x = pos.x + mSkillData->getAttackRectX();
        ori.y = pos.y - attackArea.size.height * 0.5f + mSkillData->getAttackRectY();
    }
    
    attackArea.origin = ori;
    
    return attackArea;
}

///////////////////////////////被击退类////////////////////////////////////////////

KnockBackAction* KnockBackAction::create(WorldCreature* src, WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    KnockBackAction* skillAction = new KnockBackAction(src, target, skillID, skillEffect);
    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //    sAllSkillActions[skillAction->getID()] = skillAction;
        
    return skillAction;
}

//开始技能
void KnockBackAction::start()
{
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    mTargetObj->addSkillAction(this); //施放者和作用者记录下此技能，以便他死亡时清除技能
    if (mSrcObj != mTargetObj)
    {
        mSrcObj->addSkillAction(this);
    }
    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(KnockBackAction::perform);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->perform(0);
    
    
}

//执行
void KnockBackAction::perform(float dt)
{
    if (!mSkillEffect) {
        return;
    }
    
    if (mTargetObj->getState() == eSKILL) {
        this->end(0);
    }
    
    //如果被攻击者距离攻击者太近，则调整受击者的位置---linshusen
    float nowDistance = mTargetObj->getPositionX() - mOwner->getPositionX();
    if(nowDistance < BeAttack::sMinDistanceWithAttacker &&
       nowDistance > -BeAttack::sMinDistanceWithAttacker) {
        float targetX = mOwner->getForwardPos(BeAttack::sMinDistanceWithAttacker).x;
        mTargetObj->setPositionX(targetX);
        WorldScene* scene = mTargetObj->getScene();
        scene->outOfArea(mTargetObj);
    }
    
    float knockBackSpeed = 0.0f;
    if (FLOAT_EQ(mSkillEffect->getHitBackTime(), 0.0f)) {
        knockBackSpeed = mOwner->getActionMoveSpeed();
    } else {
        knockBackSpeed = mSkillEffect->getHitBackDistance() / mSkillEffect->getHitBackTime();
    }

    CCPoint dest = mTargetObj->getPosition();
    if (mOwner->isFaceLeft()) {
        dest.x -= mSkillEffect->getHitBackDistance();
    } else {
        dest.x += mSkillEffect->getHitBackDistance();
    }
    mTargetObj->skillWalkTo(dest, "bruise", mOwner->getActionMoveSpeed(), false);
    mTargetObj->setIsKeepStateWhenAnimationEnd(true);
    
    //buff频率检测
    SEL_SCHEDULE seleter = schedule_selector(KnockBackAction::checkEnd);
    schedule(seleter, mSkillEffect->getHitBackTime());
    
    //设置击退的最大持续时间。有时候击退不会被清除，所以超过这个时间要清除。
    //mSkillEffect->setLastTime(3);
    //scheduleOnce(schedule_selector(KnockBackAction::end), mSkillEffect->getLastTime());
}

//技能结束
void KnockBackAction::end(float dt)
{
    if (mHasSkillEnd) {
        return;
    }
    
    unschedule(schedule_selector(KnockBackAction::end));
    SkillAction::end(dt);
    
    if(mTargetObj->getState() == eSKILL)
    {
        mTargetObj->clearState();
        mTargetObj->stand();
    }
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
}

//强行结束
void KnockBackAction::endStrongly()
{
    if (mHasSkillEnd) {
        return;
    }
    
    this->end(0);
    if(mSkillEffect->getCastTime() >= mSkillEffect->getLastTime())
        this->clearSelf();
}

void KnockBackAction::checkEnd(float dt)
{
    if(mOwner->getState() != eSKILL || mTargetObj->getState() == eDEAD)
        this->end(0);
}

void KnockBackAction::setOwner(WorldCreature *owner)
{
    mOwner = owner;
}

///////////////////////////////特效类技能////////////////////////////////////////////
ShakeScreenEffect* ShakeScreenEffect::create(WorldCreature *target, int skillID, SkillEffectVal *skillEffect)
{
    ShakeScreenEffect* skillAction = new ShakeScreenEffect(target, skillID, skillEffect);
    //if(sAllSkillActions.find(skillAction->getID()) == sAllSkillActions.end())
    //    sAllSkillActions[skillAction->getID()] = skillAction;
        
    return skillAction;
}

//开始技能
void ShakeScreenEffect::start()
{
    SkillAction::start();
    if(!mSkillData || !mSkillEffect)
        return;
    
    mTargetObj->addSkillAction(this); //施放者和作用者记录下此技能，以便他死亡时清除技能
    if(mSkillEffect->getDelayTime() > 0)
    {
        SEL_SCHEDULE seleter = schedule_selector(ShakeScreenEffect::perform);
        mSchedulers.push_back(seleter);
        scheduleOnce(seleter, mSkillEffect->getDelayTime());
    }
    else
        this->perform(0);
}

//执行
void ShakeScreenEffect::perform(float dt)
{
    if(!mSkillEffect)
        return;
    //Game::getObjLayer()->runAction(CCShake::create(mSkillEffect->getLastTime(), 10));
    
    scheduleOnce(schedule_selector(ShakeScreenEffect::end), mSkillEffect->getLastTime());
}

//技能结束
void ShakeScreenEffect::end(float dt)
{
    if (mHasSkillEnd) {
        return;
    }
    
    unschedule(schedule_selector(ShakeScreenEffect::end));
    SkillAction::end(dt);
    //一般技能的持续时间大于施法时间，清除自己，技能结束。
    if(mSkillEffect->getCastTime() < mSkillEffect->getLastTime())
        SkillAction::clearSelf();
}


