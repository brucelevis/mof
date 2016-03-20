//
//  Bot.cpp
//  GoldOL
//
//  Created by Mike Chang on 12-10-12.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#include "WorldMonster.h"
#include "WorldSkill.h"
#include "WorldCreatureAI.h"

#define MON_ACT_TIME 0.5f // todo cfg
#define MON_FALLOW_DISTANCE_X 200
#define MON_FALLOW_IDSTANCE_Y 50


WorldMonster* WorldMonster::create(int tplid)
{
    WorldMonster* monster = new WorldMonster();
    if (!monster->MonsterInit(tplid)) {
        delete monster;
        monster = NULL;
    }
    return monster;
}

void WorldMonster::destroy(WorldMonster* monster)
{
    delete monster;
}

WorldMonster::WorldMonster() : LivingObject(eWorldObjectMonster)
{
    //stand();
    mStand = 0;
    mMonsterState = eNonState;
    mIsStartedAI = false;
    mTarget = NULL;
    mCanAtk = false;
    setActionPeriod(0.0f);
    mComCd = 0;
    
    setHpIncr(0);
    setFrequency(SeparateValues::sIncrHpFrequency);
}

// added by jianghan for 为WorldPet增加
WorldMonster::WorldMonster(WorldObjectType type) : LivingObject(type)
{
    //stand();
    mStand = 0;
    mMonsterState = eNonState;
    mIsStartedAI = false;
    mTarget = NULL;
    mCanAtk = false;
    setActionPeriod(0.0f);
    mComCd = 0;
    
    setHpIncr(0);
    setFrequency(SeparateValues::sIncrHpFrequency);
}
// end add

WorldMonster::~WorldMonster()
{
    if (mAI) {
        delete mAI;
    }
    
    mMonsterStudyableSkills.clear();
}

float lerp(float a, float b, float l){
    //ccpAdd(ccpMult(a, 1.f - alpha), ccpMult(b, alpha));
    return a * (1.f - l) + b * l;
}

CCPoint calToRange(CCPoint src, CCPoint dest, CCPoint range){
    CCPoint ret;
    float distance = ccpDistance(src, dest);
    
    ret = ccpLerp(src, dest, 1 - range.y / distance);
    return ret;
}
/* ai
 
 */
bool canAtk(CCPoint src, CCPoint dest, CCPoint range){
    return labs(src.x - dest.x) < range.x && labs(src.y - dest.y) < range.y;
}

//修正小怪的攻击方向
void WorldMonster::FixAtkOrient(LivingObject *target)
{
    setOrient(getAtkOrient(target));
}

bool WorldMonster::MonsterInit(int tplId)
{
    MonsterCfgDef* monsterCfg = MonsterCfg::getCfg(tplId);
    if (monsterCfg == NULL) {
        log_error("can't find monster cfg of " << tplId);
        return false;
    }
    //保存模板id
    setObjTpltID(tplId);
    setModId(tplId);
    
    // added by jianghan
    // 这个必须设置，将来在技能计算中会用到
    mResCfg = ResCfg::getCfg(monsterCfg->res_id);
    if (mResCfg == NULL) {
        log_error("can't find monster " << tplId << "'s res cfg " << monsterCfg->res_id);
        return false;
    }
    // add endß
    
    InitBasicAttributes(monsterCfg);
    
    InitOtherAttributes(monsterCfg);
    
    return true;
}

//初始化基本属性
void WorldMonster::InitBasicAttributes(MonsterCfgDef* monsterCfg)
{
    //初始化基本属性
    setLvl(monsterCfg->getLvl());
    setHp(monsterCfg->getHp());
    setMaxHp(monsterCfg->getHp());
#if 0
    setName(monsterCfg->name);
#endif
    setHit(monsterCfg->getHit());
    setDef(monsterCfg->getDef());
    setAtk(monsterCfg->getAtk());
    setDodge(monsterCfg->getDodge());
    setPerAnger(monsterCfg->getPerAnger());
    
    //是否霸体状态
    setIsUnhurtState(monsterCfg->isArmor);
    
}

//初始其它高级属性（宠物也需要这个函数）
void WorldMonster::InitOtherAttributes(MonsterCfgDef *monsterCfg)
{
    //初始化乱逛的参数，巡逻路径的长度,追击的概率
    setSpeed(monsterCfg->getWalkSpeed());
    setMoveSpeed(monsterCfg->getWalkSpeed());
    setVision(monsterCfg->getVision());
    setFollow(monsterCfg->getFollowProbality());
    setChangRoundX(monsterCfg->getChangingRoundX());
    setChangRoundY(monsterCfg->getChangingRoundY());
    setPatrolLength(monsterCfg->getPatrolLength());
    mMonsterType=monsterCfg->monsterType;
    setActionPeriod(monsterCfg->getActionperiod());
    
    //攻击相关参数
    setAtkSpeed(monsterCfg->getAtkSpeed());
    
    if (monsterCfg->getAtkRangeX() && monsterCfg->getAtkRangeY() && monsterCfg->getAtkRectOrigX() && monsterCfg->getAtkRectOrigY()) {
        mAtkRect.setRect(monsterCfg->getAtkRectOrigX(), monsterCfg->getAtkRectOrigY(), monsterCfg->getAtkRangeX(), monsterCfg->getAtkRangeY());
    }
    
    if (monsterCfg->getBeAtkRangeX() && monsterCfg->getBeAtkRangeY()) {
        mBeAtkRect.setRect(-monsterCfg->getBeAtkRangeX()/2, -monsterCfg->getBeAtkRangeY()/2, monsterCfg->getBeAtkRangeX(), monsterCfg->getBeAtkRangeY());
    }
    
    setAtkProbality(monsterCfg->getAtkProbality());
    setAtkPeriod(monsterCfg->getAtkPeriod());
    setHangAround(monsterCfg->getHangAround());
    
    //技能相关
    MonsterLearnSKills(monsterCfg->skills);
    //mMonsterStudyableSkills=monsterCfg->skills;
    mCastSkillRol=monsterCfg->castSkillRol;
    mSkillRect.setRect(-monsterCfg->getSkillRangeX()/2, -monsterCfg->getSkillRangeY()/2, monsterCfg->getSkillRangeX(), monsterCfg->getSkillRangeY());
    
#if 0
    //怪物音效
    ObjAudio* objAudio = new ObjAudio();
    objAudio->setObjAudios(monsterCfg->mAttackAudios, monsterCfg->mHittedAudios, monsterCfg->mAttackScreams, monsterCfg->mHittedScreams, 0, monsterCfg->mDeadAudio, monsterCfg->mKOScreamAudio);
    setAudio(objAudio);
#endif
}

CCRect WorldMonster::CalSkillRect(int width, int height)
{
    return CCRect(getPositionX() - width, getPositionY() - height/2
                  ,2*width,height);
}

CCRect WorldMonster::CalSkillRect()
{
    return CCRect(mSkillRect.origin.x + getPositionX(), mSkillRect.origin.y + getPositionY()
                  ,mSkillRect.size.width,mSkillRect.size.height);
}



//初始化小怪巡逻的起始点和范围
bool WorldMonster::InitPatrol()
{
    int rol = rand()%100;
    if (rol < 40) {
        setOrient(eLeft);
    }
    else
    {
        setOrient(eRight);
    }
    mPatrolData.mPatrolScale = getPatrolLength(); //MonsterCfg
    mPatrolData.mOriginPoint = getPosition();
    mPatrolData.mLeftPoint = ccp( (getPosition().x - mPatrolData.mPatrolScale / 2) ? (getPosition().x - mPatrolData.mPatrolScale / 2):0, getPosition().y);
    mPatrolData.mRightPoint = ccp(getPosition().x + mPatrolData.mPatrolScale / 2, getPosition().y);
    
    return true;
}

//小怪巡逻
void WorldMonster::MonsterPatrol()
{
    if (!canChangeToState(eWALK)) {
        return;
    }
    
    if (getState() == eWALK) {
        return;
    }
    if (!getSpeed()) {
        return ;
    }
    if(mMonsterState != ePatrol)
    {
        if (!InitPatrol())
            return;
    }
    mMonsterState = ePatrol;
    ObjOrientation orient = getOrient();
    switch (orient) {
        case eLeft:
            if ((mPatrolData.mLeftPoint.x < getPosition().x)) {
                walkTo(mPatrolData.mLeftPoint);
            }
            else walkTo(mPatrolData.mRightPoint);
            break;
        case eRight:
            if ((mPatrolData.mRightPoint.x > getPosition().x)) {
                walkTo(mPatrolData.mRightPoint);
            }
            else walkTo(mPatrolData.mLeftPoint);
            break;
        default:
            break;
    }
}

//小怪乱逛
void WorldMonster::MonsterHangAround()
{
    if (!canChangeToState(eWALK)) {
        return;
    }
    
    if (!getSpeed()) {
        return;
    }
    CCPoint newPoint;
    int maxX = getChangRoundX();
    int maxY = getChangRoundY();
    
    if ( !maxX && !maxY) {
        return;
    }
    float rolX = (float)(rand()%(maxX ? maxX : 100)) + 25;
    float rolY = (float)(rand()%(maxY ? maxY : 100)) + 25;
    int rolXOrient = rand()%100;
    int rolYOrient = rand()%100;
    if (rolXOrient<50) {
        rolX *= -1;
    }
    
    if (rolYOrient<50) {
        rolY *= -1;
    }
    newPoint = ccp(rolX, rolY);
    newPoint = ccpAdd(getPosition(), newPoint);
    
#if 0
    if(Game::getMapLayer()->IsPointOutOfArea(this, newPoint))
        return;
#else
    if ( this->IsPointOutOfArea(newPoint) )
        return;
#endif
    walkTo(newPoint);
    
}
/*
int WorldMonster::MonsterTrack(LivingObject* target, MonsterType type)
{
    if(getState() == eSKILL )
    {
        if (!mIsSkillMoveState) {
            return 0;
        }
    }
    else if (!canChangeToState(eWALK)) {
        return 0;
    }
    
    if (!getSpeed()) {
        return 0;
    }
    
    int rol = rand()%100;
    
    if (rol > getFollow())
    {
        return 0;
    }
    
    CCRect targetBeAtkRect = target->calcBeAtkRect();
    CCPoint targetPos = target->getPosition();
    CCPoint newPoint;
    
    //技能追击
    if (type == eLongRange)
    {
        CCRect monsterSkillRect = CalSkillRect();
        
        //目标在技能碰撞框内返回0
        if (monsterSkillRect.intersectsRect(targetBeAtkRect)) {
            return 0;
        }
        int distanceX = abs((int)(this->getPosition().x - targetPos.x));
        int distanceY = abs((int)(this->getPosition().y - targetPos.y));
        
        //X轴距离大于X方向的技能范围
        if (distanceX >= this->getSkillRect().size.width/2 )
        {
            //向左或向右接近目标
            if ((this->getPosition().x - targetPos.x) > 0) {
                this->walkTo( ccpAdd(targetPos, ccp(this->getSkillRect().size.width/2, 0)) );
            }
            else
            {
                this->walkTo( ccpSub(targetPos, ccp(this->getSkillRect().size.width/2, 0)) );
            }
            return 1;
        }
        
        //Y轴距离大于Y方向的技能范围
        if ( distanceY > this->getSkillRect().size.height/2 )
        {
            //在目标上方
            if ((this->getPosition().y - targetPos.y) > 0)
            {
                //偏右边，走向目标的右边
                if ( this->getPosition().x > targetPos.x)
                {
                    newPoint = ccpAdd(targetPos,ccp(this->getSkillRect().size.width/2,0));

                    if (this->IsPointOutOfArea(newPoint)) {
                       newPoint = ccpAdd(targetPos,ccp(-this->getSkillRect().size.width/2,this->getSkillRect().size.height/2));
                    }
                }
                else   //偏左边，走向目标的左边
                {
                    newPoint = ccpAdd(targetPos,ccp(-this->getSkillRect().size.width/2,0));
                    if (this->IsPointOutOfArea(newPoint)) {
                        newPoint = ccpAdd(targetPos,ccp(this->getSkillRect().size.width/2,this->getSkillRect().size.height/2));
                    }
                }
            }
            else //在目标下方
            {
                        //
                if(this->getPosition().x > targetPos.x)
                {
                    newPoint = ccpSub(targetPos, ccp(-this->getSkillRect().size.width/2,0));

                    if (this->IsPointOutOfArea(newPoint)) {
                        newPoint = ccpSub(targetPos, ccp(this->getSkillRect().size.width/2,0));
                    }
                }
                else    //
                {
                    newPoint = ccpSub(targetPos, ccp(this->getSkillRect().size.width/2,0));
                    if (this->IsPointOutOfArea(newPoint)) {
                        newPoint = ccpSub(targetPos, ccp(-this->getSkillRect().size.width/2,0));
                    }
                        
                }
            }
            this->walkTo( newPoint );
            return 1;
        }
    }
    else    //普通攻击追击
    {
        CCRect monsterAtkRect = calcAtkRect();
        if (monsterAtkRect.intersectsRect(targetBeAtkRect)) {
            return 0;
        }
        
        CCPoint origin;
        CCPoint newPoint;
        
        float atkRealwidth = targetBeAtkRect.size.width + monsterAtkRect.size.width;
        float atkRealHeight = targetBeAtkRect.size.height + monsterAtkRect.size.height;
        
        if (isFaceLeft()) {
            newPoint.x = target->getPosition().x + atkRealwidth/2;
            newPoint.y = target->getPosition().y + rand()%((int)atkRealHeight) - atkRealHeight/2;
        }
        else
        {
            newPoint.x = target->getPosition().x - atkRealwidth/2;
            newPoint.y = target->getPosition().y + rand()%((int)atkRealHeight) - atkRealHeight/2;
        }
        
        // modified by jianghan
         #if 0
         if (Game::getMapLayer()->IsPointOutOfArea(this, newPoint)) {
         #else
         if (this->IsPointOutOfArea(newPoint)) {
         #endif
         return 0;
         }
         *2/
        
        walkTo(newPoint);
        return 1;
    }
    return 0;
}
*/

int WorldMonster::MonsterTrack(LivingObject* target, MonsterType type)
{
    if(getState() == eWALK )
        return 1;
    
    if(getState() == eSKILL )
    {
        if (!mIsSkillMoveState) {
            return 0;
        }
    }
    else if (!canChangeToState(eWALK)) {
        return 0;
    }
    
    if (!getSpeed()) {
        return 0;
    }
    
    // modified by jianghan for 远程攻击怪物避免接近目标，不存在追杀跟随的可能
    if (type != eLongRange)
    {
        int rol = rand()%100;
        
        if (rol > getFollow())
        {
            return 0;
        }
    }
    
    CCRect targetBeAtkRect = target->calcBeAtkRect();
    CCPoint targetPos = target->getPosition();
    CCPoint newPoint;
    
    //技能追击
    if (type == eLongRange)
    {
        // modified by jianghan
        //目标在技能碰撞框内返回0
        /*
         CCRect monsterSkillRect = CalSkillRect();
         if (monsterSkillRect.intersectsRect(targetBeAtkRect)) {
         return 1;
         }
         int distanceX = abs((int)(this->getPosition().x - targetPos.x));
         int distanceY = abs((int)(this->getPosition().y - targetPos.y));
         
         //X轴距离大于X方向的技能范围
         if (distanceX >= this->getSkillRect().size.width/2 )
         {
         //向左或向右接近目标
         if ((this->getPosition().x - targetPos.x) > 0) {
         this->walkTo( ccpAdd(targetPos, ccp(this->getSkillRect().size.width/2, 0)) );
         }
         else
         {
         this->walkTo( ccpSub(targetPos, ccp(this->getSkillRect().size.width/2, 0)) );
         }
         return 1;
         }
         
         //Y轴距离大于Y方向的技能范围
         if ( distanceY > this->getSkillRect().size.height/2 )
         {
         //在目标上方
         if ((this->getPosition().y - targetPos.y) > 0)
         {
         //偏右边，走向目标的右边
         if ( this->getPosition().x > targetPos.x)
         {
         newPoint = ccpAdd(targetPos,ccp(this->getSkillRect().size.width/2,0));
         if (Game::getMapLayer()->IsPointOutOfArea(this, newPoint)) {
         newPoint = ccpAdd(targetPos,ccp(-this->getSkillRect().size.width/2,this->getSkillRect().size.height/2));
         }
         }else   //偏左边，走向目标的左边
         {
         newPoint = ccpAdd(targetPos,ccp(-this->getSkillRect().size.width/2,0));
         if (Game::getMapLayer()->IsPointOutOfArea(this, newPoint)) {
         newPoint = ccpAdd(targetPos,ccp(this->getSkillRect().size.width/2,this->getSkillRect().size.height/2));
         }
         }
         
         }
         else //在目标下方
         {
         //
         if(this->getPosition().x > targetPos.x)
         {
         newPoint = ccpSub(targetPos, ccp(-this->getSkillRect().size.width/2,0));
         if (Game::getMapLayer()->IsPointOutOfArea(this, newPoint)) {
         newPoint = ccpSub(targetPos, ccp(this->getSkillRect().size.width/2,0));
         }
         }
         else    //
         {
         newPoint = ccpSub(targetPos, ccp(this->getSkillRect().size.width/2,0));
         if (Game::getMapLayer()->IsPointOutOfArea(this, newPoint)) {
         newPoint = ccpSub(targetPos, ccp(-this->getSkillRect().size.width/2,0));
         }
         
         }
         }
         this->walkTo( newPoint );
         return 1;
         }
         */
        //向左或向右接近目标
#if 0
        CCRect curArea = Game::getMapLayer()->getCurArea();
#else
        CCRect curArea = mScene->getBoundingBox(getAreaId());
#endif
        
        CCPoint desPos1, desPos;
        
        if ((this->getPosition().x - targetPos.x) > 0) {
            int randomdis = (int)this->getSkillRect().size.width/4 + rand()%(int)(1+this->getSkillRect().size.width/2);
            desPos1 = ccpAdd(targetPos, ccp(/*this->getSkillRect().size.width/2*/randomdis, 0));
#if 0
            if(Game::getMapLayer()->IsPointOutOfArea(this, desPos1)){
#else
                if (this->IsPointOutOfArea(desPos1)) {
#endif
                    desPos1 = ccpSub(targetPos, ccp(20, 0));
                    
                    if( abs((int)(this->getPosition().y - targetPos.y))<200 )
                    {
                        int space = curArea.getMaxY() - this->getPosition().y;
                        int space_1 = this->getPosition().y - curArea.getMinY();
                        if( (this->getPosition().y - targetPos.y)>0 ){
                            if( space > 100 )
                                desPos = ccpAdd(desPos1, ccp(0, min(300,space) ));
                            else
                                desPos = ccpSub(desPos1, ccp(0, min(300,space_1) ));
                        }else{
                            if( space_1 > 100 )
                                desPos = ccpSub(desPos1, ccp(0, min(300,space_1) ));
                            else
                                desPos = ccpAdd(desPos1, ccp(0, min(300,space) ));
                        }
                    }
                    
                    // 如果距离目标位置很近，就不用移动了，返回0表示要跑一下了
                    if( abs((int)(desPos.x - this->getPosition().x))<10
                       && abs((int)(desPos.x - this->getPosition().x))<10  )
                        return 0;
                    
                    if( this->IsPointOutOfArea(desPos) == false )
                        this->walkTo( desPos );
                    return 1;
                    //return 0;
                }
            }
            else
            {
                int randomdis = (int)this->getSkillRect().size.width/4 + rand()%(int)(1+this->getSkillRect().size.width/2);
                
                desPos1 = ccpSub(targetPos, ccp(/*this->getSkillRect().size.width/2*/randomdis, 0));
#if 0
                if(Game::getMapLayer()->IsPointOutOfArea(this, desPos1)) {
#else
                    if (this->IsPointOutOfArea(desPos1)) {
#endif
                        //desPos1 = ccpAdd(targetPos, ccp(this->getSkillRect().size.width/2, 0));
                        //return 0;
                        desPos1 = ccpAdd(targetPos, ccp(20, 0));
                        
                        if( abs((int)(this->getPosition().y - targetPos.y))<200 )
                        {
                            int space = curArea.getMaxY() - this->getPosition().y;
                            int space_1 = this->getPosition().y - curArea.getMinY();
                            if( (this->getPosition().y - targetPos.y)>0 ){
                                if( space > 100 )
                                    desPos = ccpAdd(desPos1, ccp(0, min(300,space) ));
                                else
                                    desPos = ccpSub(desPos1, ccp(0, min(300,space_1) ));
                            }else{
                                if( space_1 > 100 )
                                    desPos = ccpSub(desPos1, ccp(0, min(300,space_1) ));
                                else
                                    desPos = ccpAdd(desPos1, ccp(0, min(300,space) ));
                            }
                        }
                        
                        // 如果距离目标位置很近，就不用移动了，返回0表示要跑一下了
                        if( abs((int)(desPos.x - this->getPosition().x))<10
                           && abs((int)(desPos.x - this->getPosition().x))<10  )
                            return 0;
                        
                        if( this->IsPointOutOfArea(desPos) == false )
                            this->walkTo( desPos );
                        return 1;
                    }
                    //this->walkTo( ccpSub(targetPos, ccp(this->getSkillRect().size.width/2, 0)) );
                }
                
                if ((this->getPosition().y - targetPos.y) > 0)
                {
                    int randomdis = (int)this->getSkillRect().size.height/4 + rand()%(int)(1+this->getSkillRect().size.height/2);
                    
                    desPos = ccpAdd(desPos1, ccp(0, randomdis/*this->getSkillRect().size.height/2*/));
#if 0
                    if(Game::getMapLayer()->IsPointOutOfArea(this, desPos)) {
#else
                        if (this->IsPointOutOfArea(desPos)) {
#endif
                            //desPos = ccpSub(desPos1, ccp(0, this->getSkillRect().size.height/2));
                            //return 0;
                        }
                    }else{
                        int randomdis = (int)this->getSkillRect().size.height/4 + rand()%(int)(1+this->getSkillRect().size.height/2);
                        desPos = ccpSub(desPos1, ccp(0, randomdis/*this->getSkillRect().size.height/2*/));
#if 0
                        if(Game::getMapLayer()->IsPointOutOfArea(this, desPos)) {
#else
                            if (this->IsPointOutOfArea(desPos)) {
#endif
                                
                                //desPos = ccpAdd(desPos1, ccp(0, this->getSkillRect().size.height/2));
                                //return 0;
                            }
                        }
                        
                        // 如果距离目标位置很近，就不用移动了
                        if( abs((int)(desPos.x - this->getPosition().x))<10
                           && abs((int)(desPos.x - this->getPosition().x))<10  )
                            return 0;
                        
                        if( this->IsPointOutOfArea(desPos) == false )
                            this->walkTo( desPos );
                        return 1;
                    }
                    else    //普通攻击追击
                    {
                        CCRect monsterAtkRect = calcAtkRect();
                        if (monsterAtkRect.intersectsRect(targetBeAtkRect)) {
                            return 1;
                        }
                        
                        CCPoint origin;
                        CCPoint newPoint;
                        
                        float atkRealwidth = targetBeAtkRect.size.width + monsterAtkRect.size.width;
                        float atkRealHeight = targetBeAtkRect.size.height + monsterAtkRect.size.height;
                        
                        if (isFaceLeft()) {
                            newPoint.x = target->getPosition().x + atkRealwidth/2;
                            newPoint.y = target->getPosition().y + rand()%((int)atkRealHeight) - atkRealHeight/2;
                        }
                        else
                        {
                            newPoint.x = target->getPosition().x - atkRealwidth/2;
                            newPoint.y = target->getPosition().y + rand()%((int)atkRealHeight) - atkRealHeight/2;
                        }
                        
#if 0
                        if(Game::getMapLayer()->IsPointOutOfArea(this, newPoint)) {
#else
                            if (this->IsPointOutOfArea(newPoint)) {
#endif
                                return 0;
                            }
                            walkTo(newPoint);
        return 1;
    }
    return 0;
}
                    
int WorldMonster::MonsterCanAtk(LivingObject* target)
{
    //先做状态限定---linshusen
    if(! this->canChangeToState(eATTACK) || target->getState() == eDEAD)
        return false;
    
    //处于控制状态，不能攻击
    if(isInControlState()){
        return false;
    }
    
    CCRect atkRect = calcAtkRect();
    CCRect beAtkRect = target->calcBeAtkRect();
    
    if (atkRect.intersectsRect(beAtkRect))
    {
        return true;
    }
    return false;
}
int WorldMonster::MonsterEscape(LivingObject* target)
{
    if (!getSpeed()) {
        return 0;
    }
    
    if(getState() == eSKILL)
    {
        if (!mIsSkillMoveState) {
            return 0;
        }
    }
    else if (getState() == eWALK)
    {
        return 0;
    }
    else if (!canChangeToState(eWALK)) {
        return 0;
    }
    
    CCPoint targetPos = target->getPosition();
    CCPoint newPoint;
    
    float distanceX = getPosition().x - targetPos.x;
    float distanceY = getPosition().y - targetPos.y;
    
    int targetXOrient = getPosition().x < target->getPosition().x ? -1 : 1;
    int targetYOrient = getPosition().y < target->getPosition().y ? -1 : 1;
    
#if 0
    CCRect curArea = Game::getMapLayer()->getCurArea();
#else
    CCRect curArea = getScene()->getBoundingBox(getAreaId());
#endif
    
    float escapDisX = curArea.size.width / 4;
    float escapDisY = curArea.size.height / 4;
    
    if ( abs((int)distanceX) >= (int)escapDisX || abs((int)distanceY) >= (int)escapDisY) {
        return 0;
    }
    
    newPoint = ccpAdd(targetPos, ccp(targetXOrient * escapDisX, targetYOrient * escapDisY));
    
    //    if (distanceX == 0) {
    //        targetXOrient = 0;
    //        newPoint = ccpAdd(targetPos, ccp(targetXOrient*escapDisX, 0));
    //    }
    //    else if (distanceY == 0) {
    //        targetYOrient = 0;
    //        newPoint = ccpAdd(targetPos, ccp(0,targetYOrient * escapDisY));
    //    }
    //    else newPoint = ccpAdd(targetPos, ccp(targetXOrient * escapDisX, targetYOrient * escapDisY));
#if 0
    PointOutOfAreaType outType = Game::getMapLayer()->IsPointOutOfArea(this, newPoint);
#else
    PointOutOfAreaType outType = IsPointOutOfArea(newPoint);
#endif
    if(outType)
    {
        switch(outType)
        {
            case eOutOfY:
                newPoint = ccpAdd(targetPos, ccp(targetXOrient * escapDisX, -targetYOrient * escapDisY));
                break;
            case eOutOfX:
                newPoint = ccpAdd(targetPos, ccp(-targetXOrient * escapDisX, targetYOrient * escapDisY));
            case eOutOfBoth:
                newPoint = ccpAdd(targetPos, ccp(-targetXOrient * escapDisX, -targetYOrient * escapDisY));
                break;
            default:
                break;
        }
    }
    
    walkTo(newPoint);
    return 1;
}


int WorldMonster::MonsterCastSkill(LivingObject* target)
{
    if (isInSilenceState() || isInControlState() ) {
        return 0;
    }
    
    if (!canChangeToState(eSKILL)) {
        return 0;
    }
    
    int rol = rand()%100;
    if ( rol < 100) {//getCastSkillRol()) {
        vector<MonsterSkills*> skills;
        
        // modified by jianghan
        if( mMonsterStudyableSkills.size() <= 0 ){
            return 0;
        }
        // end modify
        
        skills = mMonsterStudyableSkills;
        int skillRols = 1;
        
        for (int i = 0; i < skills.size(); i++) {
            skillRols += skills[i]->getRol();
        }
        
        int rol = rand()%skillRols;
        int roltmp = 0;
        for (int i = 0; i < skills.size(); i++)
        {
            if ( roltmp <= rol && rol < (roltmp + skills[i]->getRol()))
            {
                //可增加更多的条件判断
                int skillid = skills[i]->getSkillid();
                
                SkillCfgDef *skillData = SkillCfg::getCfg(skillid);
                
                //技能冷却的判断
                std::map<int, int>::iterator iter;
                iter = mCoolDownTable.find(skills[i]->getSkillid());
                
                //该技能处于冷却状态
                if (iter != mCoolDownTable.end()) {// || getComCd()) {
                    continue;
                }
                
                //判断是否处于技能范围内
                CCRect monsterSkillRect = CalSkillCastRect(this, skillData, getAtkOrient(target)); //CalSkillRect(skillData->getCastRangeX(),skillData->getCastRangeY());
                //目标在技能碰撞框内返回0
                CCRect targetBeAtkRect = target->calcBeAtkRect();
                if ( !monsterSkillRect.intersectsRect(targetBeAtkRect)) {
                    continue;
                }
                
                FixAtkOrient(target);

                if (!canCastSkill(skills[i]->getSkillid())) {
                    continue;
                }
                //自动战斗
                if(skill(skills[i]->getSkillid(), getPositionX(), getPositionY(), getOrient()))
                {
                    //设置技能cd
                    // uncommented by jianghan
                    MonsterSetSkillCoolDown(skills[i]->getSkillid());
                    // end 
                    
                    // added by jianghan for 记录最近一次进攻时间
                    setLastAttackTime(time(NULL));
                    // end add
                    
                    return 1;
                }
            }
            roltmp += skills[i]->getRol();
        }
    }
    return 0;
}

bool WorldMonster::IsSkillCoolingDown(int skillid)
{
    std::map<int, int>::iterator iter;
    iter = mCoolDownTable.find(skillid);
    if (iter == mCoolDownTable.end()) {
        return false;
    }
    else return true;
}

void WorldMonster::MonsterLearnSKills(vector<MonsterSkills*> skills)
{
    vector<int> skillIds;
    
    for (int i = 0; i < skills.size(); i++) {
        bool exist;
        SkillCfgDef* skilldata = SkillCfg::getCfg(skills[i]->getSkillid(), exist);
        if (exist) {
            mMonsterStudyableSkills.push_back(skills[i]);
        }
        (void)skilldata;
        
        skillIds.push_back(skills[i]->getSkillid());
    }
    
    setSkills(skillIds);
}

void WorldMonster::MonsterClearSkillTable()
{
    mMonsterStudyableSkills.clear();
}

int WorldMonster::MonsterSetSkillCoolDown(int skillid)
{
    bool exist = false;
    SkillCfgDef* skillData = SkillCfg::getCfg(skillid, exist);
    if (!exist) {
        return 0;
    }
    mCoolDownTable[skillid] = 1 + skillData->getSkillCD() / getActionPeriod();
    setComCd(1 + skillData->getComCD() / getActionPeriod() );
    return 1;
}

int WorldMonster::MonsterDealWithCoolDown()
{
    std::map<int, int>::iterator iter;
    for (iter = mCoolDownTable.begin(); iter != mCoolDownTable.end();) {
        iter->second -= 1;
        if (iter->second <= 0) {
            mCoolDownTable.erase(iter++);
        }
        else
            iter++;
    }
    if(mComCd > 0)
        mComCd -= 1;
    return 1;
}

void WorldMonster::damage(int dmg, LivingObject* src)
{
    WorldCreature::damage(dmg, src);
}

void WorldMonster::death(LivingObject* src)
{
    WorldCreature::death(src);
    stopAI();
}

int WorldMonster::MonsterFallow(LivingObject *target)
{
    if (!canChangeToState(eWALK)) {
        return 0;
    }
    CCPoint targetPosi = target->getPosition();
    CCPoint myPosi = getPosition();
    CCPoint newPoint = myPosi;
    
    int distanceX = targetPosi.x - myPosi.x;
    int distanceY = targetPosi.y - myPosi.y ;
    bool isChange = false;
    
    if (abs(distanceX) > MON_FALLOW_DISTANCE_X) {
        int xOrient = abs(distanceX) / distanceX;
        newPoint.x += xOrient*(abs(distanceX) - MON_FALLOW_DISTANCE_X);
        isChange =true;
    }
    
    if (abs(distanceY) > MON_FALLOW_IDSTANCE_Y) {
        int yOrient = abs(distanceY) / distanceY;
        newPoint.y += yOrient*(abs(distanceY) - MON_FALLOW_IDSTANCE_Y);
        isChange = true;
    }
    
    if (isChange) {
        walkTo(newPoint);
        return 1;
    }
    
    return 0;
}

int WorldMonster::MonsterStand(float second)
{
    int period = second / getActionPeriod() + 1;
    setStand(period);
    return 0;
}

void WorldMonster::DealMonsterAiState()
{
    MonsterDealWithCoolDown();
    
    if (getStand()) {
        mStand--;
    }
}

#if 0
void WorldMonster::starIncrHP()
{
    if(getHpIncr() == 0)
        return;
    this->schedule(schedule_selector(WorldMonster::refreshMonsterHP), getFrequency());
}
void WorldMonster::refreshMonsterHP(float dt)
{
    //当被打死后  就不在回血
    if(getState() == eDEAD)
    {
        this->unschedule(schedule_selector(WorldMonster::refreshMonsterHP));
        return;
    }
    //    if(GameMgr::getGame()->getState() == ePause)
    //        return;
    
    int addData;
    if(getHp() >= getMaxHp() - getHpIncr())
    {
        addData = getMaxHp() - getHp();
    }
    else
    {
        addData = getHpIncr();
    }
    
    if(addData == 0)
        return;
    
    setHp(getHp() + addData);
    addHpNumberEffect(addData);
    afterRefreshMonsterHPCallBack(addData);
}

void WorldMonster::afterRefreshMonsterHPCallBack(int addHP)
{
    SUIMgr.getFightingUI()->setEnemyPetsHP(-addHP);
}

void WorldMonster::deleterefreshHpSchedule()
{
    this->unschedule(schedule_selector(Monster::refreshMonsterHP));
}

#endif

// added by jianghan for 判断目标是否在自己视线范围内
bool WorldMonster::TargetInSight( WorldCreature * pTarget )
{
    CCRect sightRect;
    int sight = getVision();
    ObjOrientation orient = getOrient();
    switch (orient) {
        case eUp:
        case eDown:
            sightRect.setRect( getPosition().x-300, getPosition().y-300, 600, 600 );
            break;
            
        case eLeft:
        case eLeftUp:
        case eLeftDown:
            sightRect.setRect( getPosition().x-300-sight, getPosition().y-300, 600+sight, 600 );
            break;
            
        case eRight:
        case eRightUp:
        case eRightDown:
        default:
            sightRect.setRect( getPosition().x-300, getPosition().y-300, 600+sight, 600 );
            break;
    }
    
    CCRect beSaw = CCRect(pTarget->mBeAtkRect.origin.x + getPositionX()
                          ,pTarget->mBeAtkRect.origin.y + getPositionY()
                          ,pTarget->mBeAtkRect.size.width,pTarget->mBeAtkRect.size.height);
    
    return sightRect.intersectsRect(beSaw);
}
// end add
