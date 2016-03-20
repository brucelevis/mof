//
//  WorldCreature.cpp
//  GoldOL
//
//  Created by Mike Chang on 12-10-10.
//  Copyright (c) 2012年__MyCompanyName__. All rights reserved.
//

#include "WorldCreature.h"
#include "WorldRole.h"
#include "WorldScene.h"
#include "WorldSkill.h"
#include "WorldSkillAction.h"
#include "World.h"
#include "WorldCreatureAI.h"

#include "EnumDef.h"
#include "main.h"

//////////////状态冲突管理/////////////

bool WorldCreature::askStateMutexTable(ObjState fromState, ObjState toState)
{
    
    return StateMutexCfg::sObjStateMutexTable[fromState][toState];
}
/////////////////////连击管理///////////////////////////
int Combo::inputCombo()
{
    if(mInputCount == mMaxCount)
        mInputCount = mMaxCount;
    else
        mInputCount ++;
    
    return mInputCount;
}

int Combo::outputCombo()
{
    if(mOutputCount == mMaxCount)
        mOutputCount = mMaxCount;
    else
        mOutputCount++;
    
    return mOutputCount;
}

////////////////////////////////////////////////
WorldCreature::~WorldCreature(void)
{
    if (!isDead()) {
        removeAllSkillAction();//置死亡状态，清除自己的技能特效，
    }

    // added by jianghan for 别忘了清除分配的资源
    for (std::vector< std::pair<uint64_t,CCPoint> * >::iterator iter = mHistoryPositions.begin(); iter != mHistoryPositions.end(); )
    {
        delete (*iter);
        iter = mHistoryPositions.erase(iter);
    }
    // end add
    
}

WorldCreature::WorldCreature(WorldObjectType type)//, int resid, int arg)
:SceneObject(type)
,mAtkRange(ccp(OBJ_DEFAULT_ATK_RANGE_X,OBJ_DEFAULT_ATK_RANGE_Y))
{
    //同步相关
    setMoveSpeed(OBJ_DEFAULT_SPEED);
    setHp(500);
    
//    scheduleUpdate();
    mNeedToRunAction = false;
    mIsKeepStateWhenAnimationEnd = false;
    mIsSkillMoveState = false;
    mIsUnhurtState = false;
    mIsAttackContinue = false;
    setState(eNone);
    mAtkRect = CCRect(35,-20,125,40);
    mBeAtkRect = CCRect(-80,-40,160,80);
    
    mRoleTplt = NULL;
    
    setAtkSpeed(0);
    
    setHitStatistics(0);
    setMaxHitStatistics(0);
    mHitStatSelector = schedule_selector(WorldCreature::clearCurHitStat);
    setBeHitStatistics(0);
    setCurAnger(0);
    setChangedHp(0);
    setActionMoveSpeed(0);
    mIsWalkToAutoFlipx = true;
    
    setDmgOther(0);
    setIsRecordDmgOther(false);
    
    mHasAtkBuff = false;
    mAtkPercentFactor=1;    mAtkValueFactor=0;
    
    mHasDefBuff = false;
    mDefPercentFactor=1;    mDefValueFactor=0;
    
    mHasHitBuff = false;
    mHitPercentFactor=1;    mHitValueFactor=0;
    
    mHasDodgeBuff = false;
    mDodgePercentFactor=1;  mDodgeValueFactor=0;
    
    mHasCriBuff = false;
    mCriPercentFactor=1;    mCriValueFactor=0;
    
    mHasSpeedBuff = false;
    mSpeedPercentFactor=1;    mSpeedValueFactor=0;
    
    //沉默，控制，束缚
    mSilenceRefCount = 0;
    mControlRefCount = 0;
    mConstraintRefCount = 0;
    
    setAtker(NULL);
    setAI(NULL);
    setIsAIStarted(false);
    setIsInProtectMode(false);
    
    setTriggerSkill(0, 0.0f);
    
    // added by jianghan for 最后一次攻击时间初始化
    setLastAttackTime(time(NULL));
    mDodgeCount = 0;
    // end add
}

void WorldCreature::setObjTpltID(int tpltID)
{
    SceneObject::setTpltID(tpltID);
    mRoleTplt = NULL;
    if(getType() == eWorldObjectRole)
    {
        if(RoleTpltCfg::sAllRoleTplts.find(this->getTpltID()) != RoleTpltCfg::sAllRoleTplts.end())
        {
            mRoleTplt = RoleTpltCfg::sAllRoleTplts[this->getTpltID()];
            setAttackDelayTimes(mRoleTplt->mAttackTimes);
        }
    }
}

WorldCreature* WorldCreature::getNearestObj(vector<WorldCreature*> objs)
{
    CCPoint pos = getPosition();
    int leftMinDistance = 100000000;
    int rightMinDistance = 100000000;
    
    WorldCreature* leftTarget = NULL;
    WorldCreature* rightTarget = NULL;
    
    int leftCount = 0;
    int rightCount = 0;
    
    for(int i = 0; i < objs.size(); i++)
    {
        if (getInstId() == objs[i]->getInstId()) {
            continue;
        }
        
        CCPoint tmp = objs[i]->getPosition();
        if (tmp.x > pos.x) {
            
            rightCount++;
            
            if (rightMinDistance > abs( (int)(pos.x - tmp.x)) )
            {
                rightTarget = objs[i];
                rightMinDistance = abs((int)(pos.x - tmp.x));
            }
        }
        else
        {
            leftCount++;
            if (leftMinDistance > abs( (int)(pos.x - tmp.x)) )
            {
                leftTarget = objs[i];
                leftMinDistance = abs((int)(pos.x - tmp.x));
            }
        }
        
    }
    return leftCount > rightCount ? leftTarget : rightTarget;
}

float WorldCreature::getAttackMoveTime()
{
    return SeparateValues::sDefaultAttackMoveTime;
}

int WorldCreature::getAttackMoveDistance()
{
    return SeparateValues::sDefaultAttackMoveDistance;
}

float WorldCreature::getLastAttackHitBackTime()
{
    return getAttackMoveTime();
}

int WorldCreature::getLastAttackHitBackDistance()
{
    return getAttackMoveDistance();
}

int WorldCreature::chgState(ObjState st)
{
    if(st != eATTACK)
    {
        if(getType() == eWorldObjectRole)
        {
            //角色，如果转换到比攻击更高的状态，例如受创，技能等，就清除连击数。
            if(WorldCreature::askStateMutexTable(eATTACK,st))
            {
                this->clearCombo(0);
                if(getIsAttackContinue())
                    endAttackContinue();
            }
            unschedule(schedule_selector(WorldCreature::attack));
            unschedule(schedule_selector(WorldCreature::lastAttack));
        }
    }
    setState(st);
    return 1;
}

void WorldCreature::clearState()
{
    setState(eNone);
    mNeedToRunAction = false;
    mCurAction = eNone;
    mCurrentAnimation = "";
    mIsKeepStateWhenAnimationEnd = false;
    //endAttackContinue();
}

bool WorldCreature::canChangeToState(ObjState toState)
{
    return WorldCreature::askStateMutexTable(getState(), toState);
}

int WorldCreature::stand(string animation)
{
    ObjState state = eIDLE;
    if(!canChangeToState( eIDLE) && getState() != eIDLE)
    {
        //在技能可移动状态，可以行走站立
        if(getState() == eSKILL && mIsSkillMoveState)
        {
            state = eSKILL;
        }
        else
        {
            return 0;
        }
    }
    
    if (getState() != eIDLE)
    {
        action(eIDLE, state, animation);
    }
    
    notify_sync_stop notify;
    notify.time = mScene->mRunTime;
    notify.instid = getInstId();
    notify.posx = getPositionX();
    notify.posy = getPositionY();
    mScene->broadcastPacket(&notify);
    
    return 1;
}

//强制进入站立状态
int WorldCreature::standStrongly(string animation)
{
    clearState();
    stand(animation);
    return 1;
}

int WorldCreature::skillStand(string animation)
{
    if(!canChangeToState( eSKILL))
        return 0;
    action(eIDLE, eSKILL, animation);
    
    //非循环动作才能直接结束状态（目前的动作都不是循环的，所以可以用下面的代码）
    uint64_t ms = 0;
    if (mResCfg)
    {
        ms = ResAnimationCfg::getAnimationDuration(mResCfg->mPath.c_str(), animation.c_str()) * (float)1000;
    }
    
    scheduleOnce(schedule_selector(WorldCreature::tryClearState), ms);
    return 1;
}

int WorldCreature::skillStandStrongly(string animation)
{
    clearState();
    skillStand(animation);
    return 1;
}
//强制进入行走状态
int WorldCreature::walkToStrongly(const CCPoint& dest,string animation)
{
    clearState();
    walkTo(dest, animation);
    return 1;
}

int WorldCreature::walkTo(const CCPoint& dest,string animation)
{
    ObjState state = eWALK;
    if(!canChangeToState( eWALK))
    {
        //在技能可移动状态，可以行走站立
        if(getState() == eSKILL && mIsSkillMoveState)
        {
            state = eSKILL;
        }
        else
        {
            return 0;
        }
    }
    
    
    setDest(dest);
    setActionMoveSpeed(getCurSpeed());
    action(eWALK, state, animation);
    
    notify_sync_motion notify;
    notify.time = mScene->mRunTime;
    notify.instid = getInstId();
    notify.posx = getPositionX();
    notify.posy = getPositionY();
    notify.speedx = getSpeedX();
    notify.speedy = getSpeedY();
    notify.destx = mDest.x;
    notify.desty = mDest.y;
    mScene->broadcastPacket(&notify);
    
    return 1;
}

void WorldCreature::onStopMove()
{
    if (getState() == eWALK) {
        setState(eIDLE);
    }
}

int WorldCreature::skillWalkTo(const CCPoint& dest,string animation, float speed, bool isAutoFlipx)
{
    if(!canChangeToState( eSKILL))
        return 0;
    
    setDest(dest);
    setActionMoveSpeed(speed);
    mIsWalkToAutoFlipx = isAutoFlipx;
    action(eWALK, eSKILL, animation);
    return 1;
}

void WorldCreature::startSkillMoveState()
{
    chgState(eSKILL);
    mIsSkillMoveState = true;
}
void WorldCreature::endSkillMoveState()
{
    mIsSkillMoveState = false;
}

#if 0
int WorldCreature::atkInput()
{
    if(!canChangeToState( eATTACK))
        return 0;
    
    if ( isInControlState() ) {
        return 0;
    }
    
    mCombo.inputCombo();
    
    unschedule(schedule_selector(WorldCreature::clearCombo));
    if(! getIsAttackContinue())
    {
        //规定连击检测的时间，过了这个时间，连击数清空
        schedule(schedule_selector(WorldCreature::clearCombo), SeparateValues::sCombeTime);
    }
    
    if(getState() != eATTACK && canChangeToState(eATTACK) && !isInControlState())//非攻击状态，发起攻击
        atkOutput();
    
    return 1;
}

int WorldCreature::atkOutput()
{
    int comboIndex = mCombo.outputCombo();
    char animate[32] = {'\0'};
    sprintf(animate, "physical_%d",comboIndex);
    
    
    bool isLastAtk = false;
    if(comboIndex == SeparateValues::sRoleMaxCombo)
    {
        isLastAtk = true;
    }
    
    float attackTime = mAttackDelayTimes[comboIndex-1];
    attackAction(animate, attackTime, isLastAtk, getAttackMoveTime(), getAttackMoveDistance());
    
#if 0
    //播放攻击音效
    playAttackSound(comboIndex);
#endif
    
    // added by jianghan for 记录最近一次进攻时间
    setLastAttackTime(time(NULL));
    // end add
    
    return 1;
}
#endif

ObjOrientation WorldCreature::getAtkOrient(WorldCreature* target)
{
    if (target->getPosition().x > getPosition().x) {
        return eRight;
    }
    else{
        return eLeft;
    }
}

void WorldCreature::clearCombo(float dt)
{
    unschedule(schedule_selector(WorldCreature::clearCombo));
    mCombo.clear();
    
}

void WorldCreature::startAttactContinue()
{
    setIsAttackContinue(true);
    mCombo.setInputCombo(SeparateValues::sRoleMaxCombo);
    
}

void WorldCreature::endAttackContinue()
{
    setIsAttackContinue(false);
    this->clearCombo(0);
}

#if 0
/**
 *	@brief	普通攻击的动作
 *
 *	@param 	animation 	动作名
 *	@param 	attackDelayTime  攻击动作开始后，真正发起攻击的时间点
 *	@param 	isLastAtk 	是否为最后一击（只有角色有最后一击）
 *	@param 	moveForwordDistance 	攻击者前进的距离
 *	@param 	moveForwordTime         攻击者前进的时间
 */
void WorldCreature::attackAction(std::string animation
              , float attackDelayTime
              , bool isLastAtk
              , float moveForwordTime
              , float moveForwordDistance
              )
{
    mDest = getForwardPos(moveForwordDistance);
    
    if(moveForwordTime > 0)
        setActionMoveSpeed(moveForwordDistance/moveForwordTime);
    else
        setActionMoveSpeed(0);
    
    action(eATTACK, eATTACK, animation);
    
    //一定时间后真正发起攻击
    if(! isLastAtk)
    {
        unschedule(schedule_selector(WorldCreature::attack));
        if(attackDelayTime > 0)
            scheduleOnce(schedule_selector(WorldCreature::attack), attackDelayTime);
        else
            attack(0);
    }
    else
    {
        unschedule(schedule_selector(WorldCreature::lastAttack));
        if(attackDelayTime > 0)
            scheduleOnce(schedule_selector(WorldCreature::lastAttack), attackDelayTime);
        else
            lastAttack(0);
    }
}
#endif

// 一次攻击结束，状态转回idle
void WorldCreature::endAttack(float dt)
{
    unschedule(schedule_selector(WorldCreature::endAttack));
    standStrongly("");
}

//普通攻击
void WorldCreature::attack(float dt, int atkIndex)
{
    unschedule(schedule_selector(WorldCreature::attack));
    
    notify_sync_attack notify_atk;
    notify_atk.sourceID = getInstId();
    notify_atk.atkIndex = atkIndex;
    notify_atk.orient = getOrient();
    notify_atk.x = getPositionX();
    notify_atk.y = getPositionY();
    mScene->broadcastPacket(&notify_atk);
    
    
    setAtkIndex(atkIndex);
    
    const WorldScene::CreatureHp wc = mScene->collectRoleHp();
    atkObjs(getAttackMoveTime(), getAttackMoveDistance());
    
    // added by jianghan for 记录最近一次进攻时间
    setLastAttackTime(time(NULL));
    // end add
    
    checkTriggerSkill();
}
//连招最后一击
void WorldCreature::lastAttack(float dt, int atkIndex)
{
    unschedule(schedule_selector(WorldCreature::lastAttack));
    
    
    notify_sync_attack notify_atk;
    notify_atk.sourceID = getInstId();
    notify_atk.atkIndex = atkIndex;
    notify_atk.orient = getOrient();
    notify_atk.x = getPositionX();
    notify_atk.y = getPositionY();
    mScene->broadcastPacket(&notify_atk);
    
    assert(getType() == eWorldObjectRole);

    setAtkIndex(atkIndex);
    
    const WorldScene::CreatureHp wc = mScene->collectRoleHp();
    atkObjs(getLastAttackHitBackTime(), getLastAttackHitBackDistance(), true);
    const WorldScene::CreatureHp wcr = mScene->findHpChangedCreatures(wc);
    
    bool immediatlySend = false;
    obj_attackedTarget atk;
    for (WorldScene::CreatureHp::const_iterator it = wcr.begin();
         it != wcr.end(); ++it) {
        atk.sourceID = getInstId();
        atk.targetID = it->first->getInstId();
        atk.effectLife = it->second;
        atk.hitType = it->first->getBeHitType();
        atk.atkIndex = 1;
        mScene->addSceneDamage(&atk);
        if(atk.effectLife <= 0){
            // 有人挂了，必须立刻发送消息，不等周期到
            immediatlySend = true;
        }
    }
    if( immediatlySend )
        mScene->doDamageBroadcast();

    
    // added by jianghan for 记录最近一次进攻时间
    setLastAttackTime(time(NULL));
    // end add
    
    checkTriggerSkill();
}

void WorldCreature::atkObjs(float beAttackBackTime, float beAttackBackDistance, bool isLastAttack)
{
    WorldScene* scene = getScene();
    
    const WorldScene::CreatureHp wc = scene->collectRoleHp();

    bool isHitted = false; //是否击中
    vector<WorldCreature*> objs = scene->atkObjs(this);
    vector<WorldCreature*>::iterator it = objs.begin();
    for (; it!=objs.end(); it++){
        WorldCreature* o = *it;
        
        // modified by jianghan ：上面得 scene->atkObjs 中已经判断了敌我关系，这里无需再次判断
        //if (this->getGroup() != o->getGroup()){
            int atkResult = o->beAtk(this, beAttackBackTime, beAttackBackDistance, isLastAttack);
            if(atkResult > 0){
                isHitted = true; //击中
            }
        //}
        // end modify
    }
    
    const WorldScene::CreatureHp wcr = scene->findHpChangedCreatures(wc);

    bool immediatlySend = false;
    obj_attackedTarget atk;
    for (WorldScene::CreatureHp::const_iterator it = wcr.begin();
         it != wcr.end(); ++it) {
        atk.sourceID = this->getInstId();
        atk.targetID = it->first->getInstId();
        atk.effectLife = it->second;
        atk.hitType = it->first->getBeHitType();
        atk.atkIndex = 0;
        scene->addSceneDamage(&atk);
        if(atk.effectLife <= 0){
            // 有人挂了，必须立刻发送消息，不等周期到
            immediatlySend = true;
        }
    }
    if( immediatlySend )
        scene->doDamageBroadcast();

//移动
    string animation = strFormat("physical_%d", getAtkIndex());
    setActionMoveSpeed(getAttackMoveDistance() / getAttackMoveTime());
    float dist = getAttackMoveDistance();
    CCPoint dest = ccpAdd(getPosition(), ccp(isFaceLeft() ? -dist : dist, 0));
    setDest(dest);
    action(eATTACK, eATTACK, animation);

    if(isHitted){
        addHitStat(); //累加连击数
    }
    
}


bool WorldCreature::isDamager(int instId)
{
    map<int, int>::iterator iter = mDamages.find(instId);
    return iter != mDamages.end();
}

void WorldCreature::recordDamager(WorldCreature *creature, int dmg)
{
    map<int, int>::iterator iter = mDamages.find(creature->getInstId());
    if (iter == mDamages.end()) {
        mDamages[creature->getInstId()] = dmg;
    } else {
        iter->second += dmg;
    }
}

void WorldCreature::damage(int dmg, WorldCreature* src)
{
    if (dmg > getHp()) {
        dmg = getHp();
    }
    
    if (getIsRecordDamager()) {
        recordDamager(src, dmg);
    }
    
    if (src && src->getIsRecordDmgOther()) {
        src->setDmgOther(src->getDmgOther() + dmg);
    }
    
    setHp(getHp() - dmg);
    if (getHp() <= 0) {
        death(src);
    }
}

bool WorldCreature::isDead()
{
    return mState == eDEAD;
}


void WorldCreature::restoreLife(WorldCreature* src)
{
    mDeathState = eDeathStateAlive;
    
    setHp(getMaxHp());
    setState(eIDLE);
    mDamages.clear();
}

void WorldCreature::death(WorldCreature* src)
{
    mDeathState = eDeathStateJustDie;
    
    setHp(0);
    setState(eDEAD);
    
    removeAllSkillAction();//置死亡状态，清除自己的技能特效，
    unscheduleAll();
    if (src) {
        mScene->onCreatureDead(this, src);
    }
}

// 被攻击结束，状态转回idle
void WorldCreature::endBeAttack(float dt)
{
    unschedule(schedule_selector(WorldCreature::endBeAttack));
    standStrongly("");
}

int WorldCreature::beAtk(WorldCreature *src, float backTime, float backDistance, bool isLastAttack)
{
    if (isDead()){
        return 0;
    }
    
    if (!IsHited(src, this)){
        // modified by jianghan for 场景伤害集中发送
        /*
         notify_sync_damage notify_dmg;
         obj_attackedTarget atk;
         atk.sourceID = src->getInstId();
         atk.targetID = getInstId();
         atk.effectLife = getHp();
         atk.hitType = eDodge;
         notify_dmg.attacked.push_back(atk);
         getScene()->broadcastPacket(&notify_dmg);
         */
        obj_attackedTarget atk;
        atk.sourceID = src->getInstId();
        atk.targetID = getInstId();
        atk.effectLife = getHp();
        atk.hitType = eDodge;
        getScene()->addSceneDamage(&atk);
        // end modify
        return 0;
    }
    
    int dmg = 0;
    HitType hitType = eHit;
    if(isLastAttack){
        dmg = calCriDamage(src);
        hitType = eCri;
    } else {
        hitType = calDamage(src, dmg);
    }
    
    setBeHitType(hitType);
    setAtker(src);
    hittedAction(src, backTime, backDistance);
    damage(dmg, src);
    clearCurHitStat(0); //清空连击数
    setBeHitStatistics(getBeHitStatistics() + 1);
    
    
    // modified by jianghan for 合并场景内的所有被打击到一起发送
    /*
    notify_sync_beatk beatk;
    beatk.sourceID = src->getInstId();
    beatk.targetID = getInstId();
    beatk.x = mPosition.x;
    beatk.y = mPosition.y;
    beatk.orient = mOrientation;
    beatk.atkIndex = src->getAtkIndex();

    getScene()->broadcastPacket(&beatk);
     */
    obj_beAttacked beAtk;
    beAtk.sourceID = src->getInstId();
    beAtk.targetID = getInstId();
    beAtk.x = mPosition.x;
    beAtk.y = mPosition.y;
    beAtk.orient = mOrientation;
    beAtk.atkIndex = src->getAtkIndex();
    getScene()->addSceneBeAtked(&beAtk);
    // end modify
    
    return 1;
}

void WorldCreature::submitSkill(WorldCreature* src, int hitType, int dmg, int skillid, int effectid){
    
}

//普通伤害的动作
void WorldCreature::hittedAction(WorldCreature *src, float backTime, float backDistance, eAnimationID animation)
{
    hittedAction(src, backTime, backDistance, "");
}

void WorldCreature::hittedAction(WorldCreature *src, float backTime, float backDistance, std::string animation)
{
    if (!canChangeToState(eHITTED)) {
        return;
    }
    //霸体状态不播放受击动作
    if (getIsUnhurtState()) {
        return;
    }
    
    float nowDistance = getPositionX() - getAtker()->getPositionX();
    if(nowDistance < BeAttack::sMinDistanceWithAttacker &&
       nowDistance > -BeAttack::sMinDistanceWithAttacker) {
        float targetX = getAtker()->getForwardPos(BeAttack::sMinDistanceWithAttacker).x;
        setPositionX(targetX);
        WorldScene* scene = getScene();
        scene->outOfArea(this);
    }
    
    setAtker(src);
    
    //受击后退的处理
    bool isNeedToTurnOver = false; //是否需要转向
    float x_delta = src->getPositionX() - this->getPositionX();
    
    isNeedToTurnOver = (x_delta > 0 && this->isFaceLeft()) || (x_delta < 0 && ! this->isFaceLeft());
    if (isNeedToTurnOver) {
        this->reverseOrient();
        mDest = getForwardPos(backDistance);
    }
    mDest = getForwardPos(-backDistance);
    
    //后退速度
    //float time = getBoneAniTime(animation);
    if(FLOAT_GT(backTime, 0)){
        setActionMoveSpeed(backDistance/backTime);
    } else {
        setActionMoveSpeed(0);
    }
    action(eHITTED, eHITTED, animation);
}

void WorldCreature::deadAction(WorldCreature *src)
{
    action(eDEAD, eDEAD, "");
}

void WorldCreature::deadAction(WorldCreature *src, float backTime, float backDistance)
{
    setAtker(src);
    
    //受击后退的位置
    if (isFaceLeft() == src->isFaceLeft()) {//同向
        mDest = getForwardPos(backDistance);
    } else {
        mDest = getForwardPos(-backDistance);//异向
    }
    
    //后退速度
    //float time = getBoneAniTime(animation);
    if (backTime > 0){
        setActionMoveSpeed( backDistance/backTime);
    } else {
        setActionMoveSpeed( 0);
    }
    action(eDEAD, eDEAD, "");
}

int WorldCreature::emitSkill(SkillCfgDef* skillCfg)
{
    vector<SkillEffectVal*> effects = SkillCfg::GetEffectFuncByTargetType(skillCfg->getSkillID(), eForSelf|eForSelfGroup);
    if (effects.size() == 0) {
        return 0;
    }
    
    notify_sync_skill notify_skill;
    notify_skill.skill_id = skillCfg->getSkillID();
    notify_skill.sourceID = getInstId();
    notify_skill.x = getPositionX();
    notify_skill.y = getPositionY();
    notify_skill.orient = getOrient();
    mScene->broadcastPacket(&notify_skill);
    
    
    //这里播放技能施放动作
    for (int i = 0; i < effects.size(); i++) {
        world::EFFECTFUNC func = SkillEffectMgr::GetEffectFunc(effects[i]->affectType);
        if (func == NULL) {
            continue;
        }
        //this->beSkill(this, skillid, effectVal);
        func(this, this, skillCfg->getSkillID(), effects[i]);
        
    }
    return 1;
}

void WorldCreature::setSkills(const vector<int>& skills)
{
    for (int i = 0; i < skills.size(); i++)
    {
        mSkills[skills[i]] = 0;
    }
}

bool WorldCreature::canCastSkill(int skillId)
{
    int actualSkillId = 0;
    if(this -> getType() == eWorldObjectRetinue){
        actualSkillId = (int) (skillId / 100 * 100);
    } else {
        actualSkillId = skillId;
    }
    map<int, uint64_t>::iterator iter = mSkills.find(actualSkillId);
    if (iter == mSkills.end())
    {
        return false;
    }
    
    uint64_t curTime = World::sCurTime;
    uint64_t coldTime = iter->second;
    
    return (coldTime <= curTime);
}

int WorldCreature::skill(int skillid, int posx, int posy, int orient)
{
    if (!canChangeToState(eSKILL) || !canCastSkill(skillid)) {
        return 0;
    }
    
    //沉默或控制状态不能放技能
    if (isInSilenceState() || isInControlState()){
        return 0;
    }
    
    setPosition(CCPoint(posx, posy));
    setOrient((ObjOrientation)orient);
    stopMove();
    addHistoryPosition(getPosition());
    
    if(this -> getType() == eWorldObjectRetinue){
        int level = skillid % 100;
        RetinueSkillDef* retinueCfg = RetinueCfg::getRetinueSkillCfg((int)(skillid / 100 * 100)); //拿到不带等级的技能号
        
        if(retinueCfg == NULL){
            return 0;
        }
        
        for(int i = 0; i < retinueCfg -> skillIds.size(); ++i){
            
            bool exist;
            SkillCfgDef* skillCfg = SkillCfg::getCfg(retinueCfg -> skillIds[i] + level, exist);
            if (skillCfg == NULL) {
                return 0;
            }
            
            int ret = emitSkill(skillCfg);
            if (ret > 0) {
                uint64_t colddown = World::sCurTime + (uint64_t)(skillCfg -> skillCD * 1000);
                mSkills[retinueCfg -> skillIds[i]] = colddown;
                checkTriggerSkill();
            }
            return ret;
            
        }
        
    } else {
    
        bool exist;
        SkillCfgDef* skillCfg = SkillCfg::getCfg(skillid, exist);
        if (skillCfg == NULL) {
            return 0;
        }
        
        int ret = emitSkill(skillCfg);
        if (ret > 0) {
            uint64_t colddown = World::sCurTime + (uint64_t)(skillCfg->skillCD * 1000);
            mSkills[skillid] = colddown;
            
            checkTriggerSkill();
        }
        return ret;
    }
}

void WorldCreature::setTriggerSkill(int skillId, float pro)
{
    mTriggerSkill.mSkillId = skillId;
    mTriggerSkill.mProbability = pro;
    mTriggerSkill.mColddown = 0;
}

int WorldCreature::checkTriggerSkill()
{
    if (mTriggerSkill.mSkillId == 0) {
        return 0;
    }
    
    int64_t curTime = World::sCurTime;
    if (curTime < mTriggerSkill.mColddown) {
        return 0;
    }
    
    int skillId = mTriggerSkill.mSkillId;
    float randNum = range_randf(0.0f, 1.0f);
    if (randNum > mTriggerSkill.mProbability) {
        return 0;
    }
    
    bool exist;
    SkillCfgDef* skillCfg = SkillCfg::getCfg(skillId, exist);
    if (skillCfg == NULL) {
        return 0;
    }
    
    
    int ret = emitSkill(skillCfg);
    if (ret > 0) {
        mTriggerSkill.mColddown = curTime + (uint64_t)(skillCfg->skillCD * 1000);
    }
    
    return ret;
}

int WorldCreature::beSkill(WorldCreature *src,int skillid,SkillEffectVal *effect)
{
    if (isDead())
    {
        return 0;
    }
    //todo effect
    
    //action(eDEAD,"dead");
    //todo: damage();
    bool exist = false;
    //bool isSucc = false;
    SkillCfgDef* skilldata = SkillCfg::getCfg(skillid, exist);
    if (!exist)
    {
        return 0;
    }
    
    if (!IsHited(src, this))
    {
#if 0
        //播放闪避的效果
        this->hittedNumEffect(eDodge, 0); //受创表现;
#endif
        // modified by jianghan for 场景伤害集中发送
        /*
         notify_sync_damage notify_dmg;
         obj_attackedTarget atk;
         atk.sourceID = src->getInstId();
         atk.targetID = getInstId();
         atk.effectLife = getHp();
         atk.hitType = eDodge;
         notify_dmg.attacked.push_back(atk);
         getScene()->broadcastPacket(&notify_dmg);
         */
        obj_attackedTarget atk;
        atk.sourceID = src->getInstId();
        atk.targetID = getInstId();
        atk.effectLife = getHp();
        atk.hitType = eDodge;
        getScene()->addSceneDamage(&atk);
        // end modify
        return 0;
    }

    setAtker(src);

    if (effect->affectType == eNon)
    {
        return 0;
    }
        
    world::EFFECTFUNC func = SkillEffectMgr::GetEffectFunc(effect->affectType);
    if (func == NULL)
    {
        return 0;
    }
    
    if ((getState() == eSKILL || getIsUnhurtState()) &&
        effect->affectType == (int)eKnockBackAction)
    {
        
    }
    else
    {
        // modified by jianghan for 将场景内的被技能攻击效果合并发送
        /*
         notify_sync_beskill noti;
         noti.sourceID = src->getInstId();
         noti.targetID = getInstId();
         noti.skill_id = skillid;
         noti.effectname = effect->affect;
         noti.x = getPositionX();
         noti.y = getPositionY();
         noti.orient = getOrient();
         mScene->broadcastPacket(&noti);
         */
        obj_beSkilled noti;
        noti.sourceID = src->getInstId();
        noti.targetID = getInstId();
        noti.skill_id = skillid;
        noti.effectname = effect->affect;
        noti.x = getPositionX();
        noti.y = getPositionY();
        noti.orient = getOrient();
        mScene->addSceneBeSkilled(&noti);
        // end modify
    }
    
    func(src, this, skillid, effect);
    clearCurHitStat(0); //清空连击数
    setBeHitStatistics(getBeHitStatistics() + 1);

    return 1;
}


void WorldCreature::action(ObjState action, ObjState state,string animation)
{
    //在这里限定状态能否转换
    if(! canChangeToState(state))
    {
        //在技能可移动状态，可以行走站立
        if(getState() == eSKILL
           && mIsSkillMoveState
           && (action == eWALK || action == eIDLE)
           )
        {
            ;
        }
        else
            return;
    }
    
    readyToRunAction(action, state, animation);
}

void WorldCreature::readyToRunAction(ObjState action, ObjState state, string animation)
{
    chgState(state);
    mCurAction = action;
    mCurrentAnimation = animation;
    //mNeedToRunAction = true;
    doRunAction(action, animation);
}

void WorldCreature::tryClearState(float dt)
{
    if (!getIsKeepStateWhenAnimationEnd())
    {
        clearState();
    }
}

void WorldCreature::doRunAction(ObjState action, std::string animation)
{
    switch (action)
    {
        case eHITTED:
        {
            moveTo(getActionMoveSpeed(), mDest);
            break;
        }
            
        case eIDLE:
            stopMove();
            break;
            
        case eWALK:
                moveTo(getActionMoveSpeed(), mDest);
            break;
            
        case eATTACK:
            moveTo(getActionMoveSpeed(), mDest);
            break;
            
        case eDEAD:
            stopMove();
            break;
            
        default:
            break;
    }
    
    if (action == eHITTED || action == eATTACK) {
        unschedule(schedule_selector(WorldCreature::tryClearState));
        uint64_t ms = ResAnimationCfg::getAnimationDuration(mResCfg->mPath.c_str(),
                                                            animation.c_str()) * 1000.0f;
        if (ms == 0) {
            Vector2 moveVec = Vector2(mDest.x - mPosition.x, mDest.y - mPosition.y);
            float length = moveVec.length();
            ms = length / getActionMoveSpeed() * 1000.0f;
        }
        scheduleOnce(schedule_selector(WorldCreature::tryClearState), ms);
    }
}


void WorldCreature::updateAlive(uint64_t ms)
{
    
}

void WorldCreature::onDie(uint64_t ms)
{
}

void WorldCreature::updateDead(uint64_t ms)
{
    
}


#define ATK_DMG_TIME 0.1
void WorldCreature::update(uint64_t ms)
{
    SceneObject::update(ms);
    
    if (mMotionRequestInFrame.size() > 0)
    {
        int i = 0;
        for (; i < mMotionRequestInFrame.size() - 1; i++)
        {
            MotionRequest& mr = mMotionRequestInFrame[i];
            switch (mr.mType) {
                case eIDLE:
                {
                    ack_sync_stop ack;
                    ack.errorcode = CE_INVALID_OP_REQUEST;
                    ack.instid = getInstId();
                    ack.posx = getPositionX();
                    ack.posy = getPositionY();
                    mScene->broadcastPacket(&ack);
                    break;
                }
                case eWALK:
                {
                    ack_sync_motion ack;
                    ack.errorcode = CE_INVALID_OP_REQUEST;
                    ack.instid = getInstId();
                    ack.posx = getPositionX();
                    ack.posy = getPositionY();
                    ack.speedx = getSpeedX();
                    ack.speedy = getSpeedY();
                    mScene->broadcastPacket(&ack);
                    break;
                }
                default:
                    break;
            }
        }
        
        MotionRequest& mr = mMotionRequestInFrame[i];
        switch (mr.mType)
        {
            case eIDLE:
            {
                if (stand() == 0)
                {
                    ack_sync_stop ack;
                    ack.errorcode = CE_INVALID_OP_REQUEST;
                    ack.instid = getInstId();
                    ack.posx = getPositionX();
                    ack.posy = getPositionY();
                    sendNetPacket(mr.mSessionId, &ack);
                }
                break;
            }
            case eWALK:
            {
                if (walkTo(mr.mDest) == 0)
                {
                    ack_sync_motion ack;
                    ack.errorcode = CE_INVALID_OP_REQUEST;
                    ack.instid = getInstId();
                    ack.posx = getPositionX();
                    ack.posy = getPositionY();
                    ack.speedx = getSpeedX();
                    ack.speedy = getSpeedY();
                    sendNetPacket(mr.mSessionId, &ack);
                }
                break;
            }
            default:
                break;
        }
        
        mMotionRequestInFrame.clear();
    }
    
    switch (mDeathState)
    {
        case eDeathStateJustDie:
        {
            onDie(ms);
            break;
        }
        case eDeathStateAlive:
        {
            updateAlive(ms);
            break;
        }
        case eDeathStateDead:
        {
            updateDead(ms);
            break;
        }
        default:
            break;
    }

    if(mNeedToRunAction)
    {
        doRunAction(mCurAction, mCurrentAnimation);
        mNeedToRunAction = false;
    }
}

//累加连击数
void WorldCreature::addHitStat()
{
    if(this->getType() != eWorldObjectRole)
        return;
    
    setHitStatistics(getHitStatistics()+1);
    if(getHitStatistics() > getMaxHitStatistics())
        setMaxHitStatistics(getHitStatistics());
    
    
    //重新定时
    unschedule(mHitStatSelector);
    scheduleOnce(mHitStatSelector, 5);
}

//清空当前连击数
void WorldCreature::clearCurHitStat(float dt)
{
    if(this->getType() != eWorldObjectRole)
        return;
    
    setHitStatistics(0);
    
    //停止定时
    unschedule(mHitStatSelector);
}

//清空所有连击记录
void WorldCreature::clearHitStat()
{
    if(this->getType() != eWorldObjectRole)
        return;
    
    setHitStatistics(0);
    setMaxHitStatistics(0);
    
    //停止定时
    unschedule(mHitStatSelector);
}

void WorldCreature::ResetBeHitStatistic()
{
    setBeHitStatistics(0);
}

void WorldCreature::addSkillAction(SkillAction *skillAction)
{
    int id = skillAction->getInstId();
    if(mSkillActions.find(id) == mSkillActions.end())
    {
        mSkillActions[id] = skillAction;
        skillAction->retain();
    }
    else
        log_info("addSkillAction error. SkillAction is already in this WorldCreature!!! ");
}
//当前角色是否有变身技能
bool WorldCreature::hasChangeBodySkillAction()
{
    map<int, SkillAction* >::iterator it = mSkillActions.begin();
    for (; it != mSkillActions.end(); ++it)
    {
        SkillAction* skillAction = it->second;
        if(skillAction->getType() == eNewBodyEffect)
            return true;
    }
    
    return false;
}

void WorldCreature::removeSkillAction(int skillActionID)
{
    map<int, SkillAction* >::iterator it = mSkillActions.find(skillActionID);
    if(it != mSkillActions.end())
    {
        //如果还没结束，则结束这个技能表现
        //if(! mSkillActions[skillActionID]->getHasSkillEnd())
        //    mSkillActions[skillActionID]->endStrongly();
        SkillAction* action = it->second;
        if (!action->getHasSkillEnd())
        {
            action->endStrongly();
        }
        mSkillActions.erase(it);
        action->release();
    }
    
//    if (mSkillActions.size() == 0)
//    {
//        standStrongly("");
//    }
}

void WorldCreature::removeAllSkillAction()
{
    vector<SkillAction*> actionBuf;
    
    //清除技能结束会removeSkillAction，
    //先清空容器，再清除技能效果，防止技能效果被多次release，导致程序奔溃
    std::map<int, SkillAction*>::iterator it;
    for (it=mSkillActions.begin(); it!=mSkillActions.end(); )
    {
        //map一边遍历一边删除的技巧
        SkillAction* action = it->second;
        actionBuf.push_back(action);
        ++it;
    }
    mSkillActions.clear();
    
    for (int i = 0; i < actionBuf.size(); i++)
    {
        SkillAction* action = actionBuf[i];
        if(! action->getHasSkillEnd())
        {
            action->endStrongly();
        }
        action->release();
    }
}

int WorldCreature::getAtk() const
{
    int atk = 0;
    if(mHasAtkBuff)
        atk = BattleProp::getAtk() * mAtkPercentFactor + mAtkValueFactor;
    else
        atk = BattleProp::getAtk();
    return atk;
}
int WorldCreature::getDef() const
{
    int def = 0;
    if(mHasDefBuff)
        def = BattleProp::getDef() * mDefPercentFactor + mDefValueFactor;
    else
        def = BattleProp::getDef();
    return def;
}
float WorldCreature::getHit() const
{
    float Hit = 0;
    if(mHasHitBuff)
        Hit = BattleProp::getHit() * mHitPercentFactor + mHitValueFactor;
    else
        Hit = BattleProp::getHit();
    return Hit;
}
float WorldCreature::getDodge() const
{
    float Dodge = 0;
    if(mHasDodgeBuff)
        Dodge = BattleProp::getDodge() * mDodgePercentFactor + mDodgeValueFactor;
    else
        Dodge = BattleProp::getDodge();
    return Dodge;
}
float WorldCreature::getCri() const
{
    float Cri = 0;
    if(mHasCriBuff)
        Cri = BattleProp::getCri() * mCriPercentFactor + mCriValueFactor;
    else
        Cri = BattleProp::getCri();
    return Cri;
}

// 获取原始属性 by wangck
int WorldCreature::getPriAtk() const
{
    return BattleProp::getAtk();
}
int WorldCreature::getPriDef() const
{
    return BattleProp::getDef();
}
float WorldCreature::getPriHit() const
{
    return BattleProp::getHit();
}
float WorldCreature::getPriDodge() const
{
    return BattleProp::getDodge();
}
float WorldCreature::getPriCri() const
{
    return BattleProp::getCri();
}
int WorldCreature::getPriHp() const
{
    return BattleProp::getMaxHp();
}
float WorldCreature::getCurSpeed()
{
    float Speed = getMoveSpeed() * mSpeedPercentFactor + mSpeedValueFactor;
    return Speed;
}

void WorldCreature::changeAtk(float percentFactor, int valueFactor)
{
    if(percentFactor == 0 && valueFactor == 0)
        return;
    
    mHasAtkBuff = true;
    mAtkPercentFactor = mAtkPercentFactor * (1 + percentFactor);
    mAtkValueFactor = mAtkValueFactor + valueFactor;
}
void WorldCreature::resumeAtk(float percentFactor, int valueFactor)
{
    mAtkPercentFactor = mAtkPercentFactor / (1 + percentFactor);
    mAtkValueFactor = mAtkValueFactor - valueFactor;
    
    if(this->getAtk() == BattleProp::getAtk())
        mHasAtkBuff = false;
}
void WorldCreature::changeDef(float percentFactor, int valueFactor)
{
    if(percentFactor == 0 && valueFactor == 0)
        return;
    
    mHasDefBuff = true;
    mDefPercentFactor = mDefPercentFactor * (1 + percentFactor);
    mDefValueFactor = mDefValueFactor + valueFactor;
}
void WorldCreature::resumeDef(float percentFactor, int valueFactor)
{
    mDefPercentFactor = mDefPercentFactor / (1 + percentFactor);
    mDefValueFactor = mDefValueFactor - valueFactor;
    
    if(this->getDef() == BattleProp::getDef())
        mHasDefBuff = false;
}
void WorldCreature::changeHit(float percentFactor, int valueFactor)
{
    if(percentFactor == 0 && valueFactor == 0)
        return;
    
    mHasHitBuff = true;
    mHitPercentFactor = mHitPercentFactor * (1 + percentFactor);
    mHitValueFactor = mHitValueFactor + valueFactor;
}
void WorldCreature::resumeHit(float percentFactor, int valueFactor)
{
    mHitPercentFactor = mHitPercentFactor / (1 + percentFactor);
    mHitValueFactor = mHitValueFactor - valueFactor;
    
    if(this->getHit() == BattleProp::getHit())
        mHasHitBuff = false;
}
void WorldCreature::changeCri(float percentFactor, int valueFactor)
{
    if(percentFactor == 0 && valueFactor == 0)
        return;
    
    mHasCriBuff = true;
    mCriPercentFactor = mCriPercentFactor * (1 + percentFactor);
    mCriValueFactor = mCriValueFactor + valueFactor;
}
void WorldCreature::resumeCri(float percentFactor, int valueFactor)
{
    mCriPercentFactor = mCriPercentFactor / (1 + percentFactor);
    mCriValueFactor = mCriValueFactor - valueFactor;
    
    if(this->getCri() == BattleProp::getCri())
        mHasCriBuff = false;
}
void WorldCreature::changeDodge(float percentFactor, int valueFactor)
{
    if(percentFactor == 0 && valueFactor == 0)
        return;
    
    mHasDodgeBuff = true;
    mDodgePercentFactor = mDodgePercentFactor * (1 + percentFactor);
    mDodgeValueFactor = mDodgeValueFactor + valueFactor;
}
void WorldCreature::resumeDodge(float percentFactor, int valueFactor)
{
    mDodgePercentFactor = mDodgePercentFactor / (1 + percentFactor);
    mDodgeValueFactor = mDodgeValueFactor - valueFactor;
    
    if(this->getDodge() == BattleProp::getDodge())
        mHasDodgeBuff = false;
}

bool WorldCreature::isInSilenceState()
{
    return mSilenceRefCount > 0;
}

void WorldCreature::incSilenceRefCount()
{
    mSilenceRefCount++;
}
void WorldCreature::decSilenceRefCount()
{
    mSilenceRefCount--;
}

bool WorldCreature::isInControlState()
{
    return mControlRefCount > 0;
}

void WorldCreature::incControlRefCount()
{
    mControlRefCount++;
}

void WorldCreature::decControlRefCount()
{
    mControlRefCount--;
}

bool WorldCreature::isInConstraintState()
{
    return mConstraintRefCount > 0;
}

void WorldCreature::incConstraintRefCount()
{
    mConstraintRefCount++;
}

void WorldCreature::decConstraintRefCount()
{
    mConstraintRefCount--;
}

void WorldCreature::changeSpeed(float percentFactor, int valueFactor)
{
    mSpeedPercentFactor = mSpeedPercentFactor * (1 + percentFactor);
    mSpeedValueFactor = mSpeedValueFactor + valueFactor;
    
    if (getState() == eWALK)
    {
        Vector2 dir = SceneObject::getSpeed();
        dir = dir.norm();
        setActionMoveSpeed(getCurSpeed());
        SceneObject::setSpeed(Vector2(dir.x * getActionMoveSpeed(), dir.y * getActionMoveSpeed()));
    }
//    
//    WorldScene* scene = getScene();
//    if (scene && getType() == eWorldObjectRole) {
//        WorldRole* role = (WorldRole*)this;
//        LogMod::addLogChangeSpeed(scene->getSceneModId(), scene->getInstId(), role->mRoleInfo.mRoleId, role->mRoleInfo.mRoleName.c_str(), percentFactor, valueFactor, mSpeedPercentFactor, mSpeedValueFactor);
//    }
}

void WorldCreature::resumeSpeed(float percentFactor, int valueFactor)
{
    mSpeedPercentFactor = mSpeedPercentFactor / (1 + percentFactor);
    mSpeedValueFactor = mSpeedValueFactor - valueFactor;
    
    if (getState() == eWALK) {
        Vector2 dir = SceneObject::getSpeed();
        dir = dir.norm();
        setActionMoveSpeed(getCurSpeed());
        SceneObject::setSpeed(Vector2(dir.x * getActionMoveSpeed(), dir.y * getActionMoveSpeed()));
    }
//    
//    WorldScene* scene = getScene();
//    if (scene && getType() == eWorldObjectRole) {
//        WorldRole* role = (WorldRole*)this;
//        LogMod::addLogChangeSpeed(scene->getSceneModId(), scene->getInstId(), role->mRoleInfo.mRoleId, role->mRoleInfo.mRoleName.c_str(), percentFactor, valueFactor, mSpeedPercentFactor, mSpeedValueFactor);
//    }
}

void WorldCreature::notifySpeedFactor()
{
    notify_sync_changespeed notify;
    notify.instid = getInstId();
    notify.percent = mSpeedPercentFactor;
    notify.value = mSpeedValueFactor;
    if (mScene)
    {
        mScene->broadcastPacket(&notify);
    }
}


void WorldCreature::enterProtectMode(uint64_t ms)
{
    if (getIsInProtectMode()) {
        return;
    }
    setIsInProtectMode(true);
    changeDef(0.0f, 10000000);
}

void WorldCreature::leaveProtectMode(uint64_t ms)
{
    if (!getIsInProtectMode()) {
        return;
    }
    setIsInProtectMode(false);
    changeDef(0.0f, -10000000);
}


CCPoint WorldCreature::calcNewPointByAtkRect(CCPoint newpoint)
{
    CCPoint ret(0.0f, 0.0f);
    if (isFaceLeft()) {
        ret.x = newpoint.x + (mAtkRect.size.width / 2) + mAtkRect.origin.x;
        ret.y = newpoint.y;
    }
    else
    {
        ret.x = newpoint.x - (mAtkRect.size.width / 2) - mAtkRect.origin.x;
        ret.y = newpoint.y;
    }
    return ret;
}

void WorldCreature::updateAI(uint64_t ms)
{
    mAI->update(ms);
}

bool WorldCreature::startAI()
{
    if (getIsAIStarted() || getAI() == NULL) {
        return false;
    }
    
    uint64_t period = 1000;
    if (getType() == eWorldObjectMonster) {
        WorldMonster* monster = dynamic_cast<WorldMonster*>(this);
        if (monster) {
            period = monster->getActionPeriod() * 1000;
        }
    }
    
    setIsAIStarted(true);
    schedule(schedule_selector(WorldCreature::updateAI), period);
    return true;
}

void WorldCreature::stopAI()
{
    if (!getIsAIStarted()) {
        return;
    }
    
    setIsAIStarted(false);
    unschedule(schedule_selector(WorldCreature::updateAI));
}

