//
//  WorldCreature.h
//  GoldOL
//
//  Created by Mike Chang on 12-10-10.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//
// 战斗相关模块，统一管理行走，攻击，技能等战斗相关
// 其它无相关表放这

#ifndef GameSrv_WorldCreature_h
#define GameSrv_WorldCreature_h

#include "Defines.h"

#include "WorldObject.h"
#include "WorldSceneObject.h"
#include "WorldScene.h"
#include "WorldConfig.h"

//#include "world_msg.h"

#define OBJ_DEFAULT_SPEED 100
#define ROLE_WALK_SPEED 350
#define MONSTER_WALK_SPEED 100
#define OBJ_DEFAULT_ATK_RANGE_X 200
#define OBJ_DEFAULT_ATK_RANGE_Y 100


#define DEFAULT_COMBO 1
class SkillAction;
class WordNumberEffect;
class WorldCreature;
class WorldCreatureAI;

#define LivingObject WorldCreature

enum CreatureGroup
{
    eCreatureMonsterGroup = -1,
};


enum DeathState
{
    eDeathStateAlive,
    eDeathStateJustDie,
    eDeathStateDead,
};


struct Combo {
public:
    Combo(int max=DEFAULT_COMBO):mInputCount(0),mOutputCount(0),mMaxCount(max){}
    void clear(){mInputCount=mOutputCount=0;}
    int inputCombo();
    int outputCombo();
    bool hasNextCombo(){return mInputCount > mOutputCount;}
    void setInputCombo(int input){mInputCount = input;}

    READONLY(int, mInputCount, InputCount)
    READONLY(int, mOutputCount, OutputCount)
    READONLY(int, mMaxCount, MaxCount)
};

struct MotionRequest
{
    int mType;
    int mSessionId;
    CCPoint mDest;
};

typedef void (*WorldCreatureCallback)(WorldCreature* creature, void* param);

class WorldCreature : public SceneObject, public BattleProp
{   
public:
    //状态冲突管理
    static bool askStateMutexTable(ObjState fromState, ObjState toState);
private:
    // added by jianghan for 同步战斗模式控制
    READWRITE(ObjCtrlMode, mCtrlMode, CtrlMode) //当前物体的控制方式（客户端，服务器，同步）
    // end add
    
    READWRITE(int, mGroup, Group)
    READWRITE(bool, mIsWalkToAutoFlipx, IsWalkToAutoFlipx) //行走时是否自动转向
    READWRITE(RoleTpltCfg*, mRoleTplt, RoleTplt) //角色模板
    READWRITE(bool, mIsAttackContinue, IsAttackContinue)//是否在持续攻击状态
    
    //是否为霸体状态
    READWRITE(bool, mIsUnhurtState, IsUnhurtState)
    READWRITE(int, mChangedHp, ChangedHp)   //当前副本内变化的血量
    READWRITE(int, mDmgOther, DmgOther)                   //当前副本内变化的血量
    READONLY(ObjState, mState, State)
    READWRITE(float, mSpeed, Speed)
    READWRITE(float, mActionMoveSpeed, ActionMoveSpeed)//执行action时的速度
    READWRITE(bool, mIsKeepStateWhenAnimationEnd, IsKeepStateWhenAnimationEnd)
    //是否在技能可移动状态
    READONLY(bool, mIsSkillMoveState, IsSkillMoveState)
    
    READWRITE(int,  mSilenceRefCount, SilenceRefCount)
    READWRITE(int,  mControlRefCount, ControlRefCount)
    READWRITE(int,  mConstraintRefCount, ConstraintRefCount)
    
    
    READWRITE(WorldCreature*, mAtker, Atker)
    //每个攻击动作发起攻击的时间点
    READWRITE(std::vector<float>, mAttackDelayTimes, AttackDelayTimes)

    READWRITE(int, mAtkSpeed, AtkSpeed)
    
    READWRITE(CCPoint, mAtkRange, AtkRange)     //
    READWRITE(CCRect, mAtkRect, AtkRect)        //
    READWRITE(CCRect, mBeAtkRect, BeAtkRect)   //
    READONLY(int, mHitStatistics, HitStatistics)        //连击数统计
    READONLY(int, mMaxHitStatistics, MaxHitStatistics)  //最大连击数
    READONLY(int, mBeHitStatistics, BeHitStatistics)       //受击次数
    READWRITE(int, mCurAnger, CurAnger)       //当前怒气值
    
    //吸血相关的变量---linshusen
    //当开启吸血技能时，记录对别人造成的伤害。技能结束时清空
    READWRITE(bool, mIsRecordDmgOther, IsRecordDmgOther)        //是否记录对别人造成的伤害
    READWRITE(DeathState, mDeathState, DeathState)
    
    //是否记录造成伤害的对象
    READWRITE(bool, mIsRecordDamager, IsRecordDamager)
    map<int, int> mDamages;
    
    READWRITE(CCPoint, mDest, Dest)
    
    // added by jianghan for 闪避修正参数
    int mDodgeCount;
    // end add
    
public:
    
    bool isDamager(int instId);
    void recordDamager(WorldCreature* creature, int dmg);
    
    
    bool isDead();
    virtual void updateAlive(uint64_t ms);
    virtual void onDie(uint64_t ms);
    virtual void updateDead(uint64_t ms);
    
    virtual void reset(){}
    virtual void update(uint64_t ms);
    virtual bool onPick(){return false;} // 点选
    
    virtual void setObjTpltID(int tpltID);
    
    WorldCreature* getNearestObj(vector<WorldCreature*> objs);
    void submitAttack(WorldCreature* src, int hitType, int dmg);
    void submitSkill(WorldCreature* src, int hitType, int dmg, int skillid, int effectid);
/////////////////////状态转换相关////////////////////////////////////////
    bool canChangeToState(ObjState toState); //能否转到指定状态
    int chgState(ObjState st);
    void clearState();
    
    void action(ObjState action, ObjState state, std::string animation = "bruise");
//    SkillOwner toSkillOwner();
    
    virtual void restoreLife(WorldCreature* src);
    virtual void death(WorldCreature* src);       //死亡
    
    //行走激动相关
    int stand(std::string animation="stand");
    int standStrongly(std::string animation="stand");
    int skillStand(std::string animation="stand");
    int skillStandStrongly(std::string animation="stand");
    int walkTo(const CCPoint& dest,std::string animation="run");
    int walkToStrongly(const CCPoint& dest,std::string animation="run");
    int skillWalkTo(const CCPoint& dest,std::string animation, float speed, bool isAutoFlipx=true);
    
    virtual void onStopMove();
    //技能状态，并可以移动
    void startSkillMoveState();
    void endSkillMoveState();
    
    bool isSameGroup(WorldCreature* creature)
    {
        return mGroup == creature->mGroup;
    }
    
/////////////////////普通攻击相关////////////////////////////////////////
    int atkInput();
    int atkOutput();
    ObjOrientation getAtkOrient(WorldCreature* target);
    
    void clearCombo(float dt);
    
    void startAttactContinue(); //开始连续的普通攻击
    void endAttackContinue();   //结束连续的普通攻击
    
    //执行普通攻击动作
    void attackAction(std::string animation
             , float attackDelayTime
             , bool isLastAtk
             , float moveForwordTime
             , float moveForwordDistance
             );
    
    READWRITE(int, mAtkIndex, AtkIndex);
    READWRITE(HitType, mBeHitType, BeHitType);
    void attack(float dt, int atkIndex);          //普通攻击
    void lastAttack(float dt, int atkIndex);      //普通攻击的最后一击
    void endAttack(float dt);
    void endBeAttack(float dt);
    
    void atkObjs(float beAttackBackTime, float beAttackBackDistance, bool isLastAttack = false);   //检测可攻击的物体并攻击
    
    //取得普通攻击位移的时间和距离
    virtual float getAttackMoveTime();
    virtual int getAttackMoveDistance();
    virtual float getLastAttackHitBackTime();
    virtual int getLastAttackHitBackDistance();
    
    //攻击受击的声音
    void playAttackSound(int attackIndex=1);
    void playAttackHittedSound();
    void playHittedScream();
    
    //攻击范围
    CCRect calcAtkRect(){
        if (isFaceLeft()) {
            return CCRect(-mAtkRect.origin.x - mAtkRect.size.width + getPositionX()
                          ,mAtkRect.origin.y + getPositionY()
                          ,mAtkRect.size.width,mAtkRect.size.height);
        }else
            return CCRect(mAtkRect.origin.x + getPositionX()
                          ,mAtkRect.origin.y + getPositionY()
                          ,mAtkRect.size.width,mAtkRect.size.height);}
    
    CCRect calcBeAtkRect(){ return CCRect(mBeAtkRect.origin.x + getPositionX()
                                          ,mBeAtkRect.origin.y + getPositionY()
                                          ,mBeAtkRect.size.width,mBeAtkRect.size.height);}
    
    CCPoint calcNewPointByAtkRect(CCPoint newpoint);
    
/////////////////////受击相关////////////////////////////////////////
    virtual int beAtk(WorldCreature* src, float backTime, float backDistance,bool isLastAttack = false);    //受击
    
    
    
    virtual void damage(int dmg, WorldCreature* src);         //受击掉血
    
    
    //普通伤害的动作
    void hittedAction(WorldCreature *src, float backTime, float backDistance, eAnimationID animation=eAnim_bruise );
    void hittedAction(WorldCreature *src, float backTime, float backDistance, std::string  animation);
    
    void deadAction(WorldCreature *src);
    void deadAction(WorldCreature *src, float backTime, float backDistance);
//    virtual void recv(int evt, int val, CCPoint pos, int extrVal);
    virtual int emitSkill(SkillCfgDef* skillCfg);   //施放技能
    
    virtual void onLeaveScene()
    {
        SceneObject::onLeaveScene();
        
        removeAllSkillAction();
    }
    
    //
    vector<MotionRequest> mMotionRequestInFrame;


public:
    
/////////////////////技能相关////////////////////////////////////////    
    map<int, uint64_t> mSkills;
    void setSkills(const vector<int>& skills);
    bool canCastSkill(int skillId);
    int skill(int skillid, int posx, int posy, int orient);
    
    struct TriggerSkill
    {
        int mSkillId;
        float mProbability;
        uint64_t mColddown;
    };
    TriggerSkill mTriggerSkill;
    void setTriggerSkill(int skillId, float probability);
    int checkTriggerSkill();
    
    int beSkill(WorldCreature *src, int skillid,struct SkillEffectVal* effect);
    
    
    //添加和清除附着在obj身上的技能表现---linshusen
    void addSkillAction(SkillAction* skillAction);
    void removeSkillAction(int skillActionID);
    void removeAllSkillAction();
    bool hasChangeBodySkillAction();
/////////////////////buff修改基础属性相关////////////////////////////////////
private:
    bool mHasAtkBuff;
    float mAtkPercentFactor,    mAtkValueFactor;
    
    bool mHasDefBuff;
    float mDefPercentFactor,    mDefValueFactor;
    
    bool mHasHitBuff;
    float mHitPercentFactor,    mHitValueFactor;
    
    bool mHasDodgeBuff;
    float mDodgePercentFactor,  mDodgeValueFactor;
    
    bool mHasCriBuff;
    float mCriPercentFactor,    mCriValueFactor;
    
    bool mHasSpeedBuff;
    float mSpeedPercentFactor,    mSpeedValueFactor;
public:
    //在战斗中基本属性可能会被加buff。为了保证对其它模块不影响，重写几个get函数。---linshusen
    virtual int getAtk() const;
    virtual int getDef() const;
    virtual float getHit() const;
    virtual float getDodge() const;
    virtual float getCri() const;
    
    // 对基本属性get重写（无buff） by wangck
    int getPriAtk() const;
    int getPriHp() const;
    int getPriDef() const;
    float getPriHit() const;
    float getPriDodge() const;
    float getPriCri() const;
    
    float getCurSpeed();
    
    //改变和恢复属性
    void changeAtk(float percentFactor, int valueFactor);
    void resumeAtk(float percentFactor, int valueFactor);
    void changeDef(float percentFactor, int valueFactor);
    void resumeDef(float percentFactor, int valueFactor);
    void changeHit(float percentFactor, int valueFactor);
    void resumeHit(float percentFactor, int valueFactor);
    void changeCri(float percentFactor, int valueFactor);
    void resumeCri(float percentFactor, int valueFactor);
    void changeDodge(float percentFactor, int valueFactor);
    void resumeDodge(float percentFactor, int valueFactor);
    
    void changeSpeed(float percentFactor, int valueFactor);
    void resumeSpeed(float percentFactor, int valueFactor);
    void notifySpeedFactor();
    
    //沉默
    void incSilenceRefCount();
    void decSilenceRefCount();
    bool isInSilenceState();
    
    //控制
    void incControlRefCount();
    void decControlRefCount();
    bool isInControlState();
    
    //束缚
    void incConstraintRefCount();
    void decConstraintRefCount();
    bool isInConstraintState();
     
    
    READWRITE(bool, mIsInProtectMode, IsInProtectMode)
    void enterProtectMode(uint64_t ms);
    void leaveProtectMode(uint64_t ms);
    
/////////////////////攻击受击统计相关////////////////////////////////////////
    void addHitStat();          //累加连击数
    void clearCurHitStat(float dt);     //清空当前连击数
    void clearHitStat();        //清空所有连击记录
    void ResetBeHitStatistic();

    
    //AI
    READWRITE(WorldCreatureAI*, mAI, AI)
    READWRITE(bool, mIsAIStarted, IsAIStarted)
    bool startAI();
    void stopAI();
    void updateAI(uint64_t ms);
    

protected:
    virtual ~WorldCreature(void);
    WorldCreature(WorldObjectType type);
private:
    
    SEL_SCHEDULE mHitStatSelector; //清空累计连击的定时器
    
    //当前附着在obj身上的技能表现。当obj死亡时要把它们清除
    std::map<int, SkillAction*> mSkillActions; 
    
    
    bool mNeedToRunAction;          //是否需要运行action
    ObjState mCurAction;
    std::string mCurrentAnimation;  //运行action时的动作名
    void readyToRunAction(ObjState action, ObjState state, std::string animation);   //准备运行action
    void doRunAction(ObjState action,std::string animation);        //运行action
    void tryClearState(float dt);
protected:
    Combo mCombo;
    
public:
    /**
     * added by jianghan
     * 这里需要说明一下：
     * 由于网络延迟，在不同位置的两个人会为了攻击对方互相根据屏幕上显示的老位置去寻找对方
     * 这就形成了一个悖论，他们永远在寻找对方的投影予以攻击，结果导致很多空招出现，我们这里
     * 需要提供一个机制：让他们的投影依旧有效，也就是在一定时间范围内的老位置也在被攻击的矩形内
     */
    std::vector< std::pair<uint64_t, CCPoint> * > mHistoryPositions;
    void addHistoryPosition(CCPoint pos){
        uint64_t current = Timestamp::now_ms();
        std::pair<uint64_t,CCPoint> * it = new std::pair<uint64_t,CCPoint>;
        it->first = current;
        it->second = pos;
        
        mHistoryPositions.push_back(it);
        
        for (std::vector< std::pair<uint64_t,CCPoint> * >::iterator iter = mHistoryPositions.begin(); iter != mHistoryPositions.end(); )
        {
            if( current - (*iter)->first > g_WorldConfig.mHistoryPositionTime ){
                // 3秒内的历史位置都可以被当作攻击目标
                delete (*iter);
                iter = mHistoryPositions.erase(iter);
            }else{
                iter ++;
            }
        }
    }
    
    
    bool intersectsHistoryPosition(CCRect attackArea){
        
        CCRect beAck = CCRect(mBeAtkRect.origin.x + getPositionX()
                              ,mBeAtkRect.origin.y + getPositionY()
                              ,mBeAtkRect.size.width,mBeAtkRect.size.height);
        if( attackArea.intersectsRect(beAck) )
            return true;
        
        int current = Timestamp::now_ms();
        for (std::vector< std::pair<uint64_t,CCPoint> * >::iterator iter = mHistoryPositions.begin(); iter != mHistoryPositions.end(); iter++){
            
            if( current - (*iter)->first < g_WorldConfig.mHistoryPositionTime ){
                // 3秒内的历史位置都可以被当作攻击目标
                CCRect beAck = CCRect(mBeAtkRect.origin.x + (*iter)->second.x
                                      ,mBeAtkRect.origin.y + (*iter)->second.y
                                      ,mBeAtkRect.size.width,mBeAtkRect.size.height);
                if( attackArea.intersectsRect(beAck) )
                    return true;
            }
        }
        return false;
    }
    
    PointOutOfAreaType IsPointOutOfArea(CCPoint desPoint)
    {
        if(getScene() == NULL)
            return eNonOut;
        
        WorldScene * pScene = getScene();
        CCRect boundingRect = pScene->getBoundingBox(getAreaId());
        
        bool isOutOfX = desPoint.x < boundingRect.getMinX() || desPoint.x > boundingRect.getMaxX();
        
        bool isOutOfY = desPoint.y < boundingRect.getMinY() || desPoint.y > boundingRect.getMaxY();
        
        if(isOutOfX && isOutOfY)
        {
            return eOutOfBoth;
        }
        else if(isOutOfX)
        {
            return eOutOfX;
        }
        else if(isOutOfY)
        {
            return eOutOfY;
        }
        return eNonOut;
    }
    // add end

    // added by jianghan for 记录最近一次进攻动作发动的时间
    READWRITE(int, mLastAttackTime, LastAttackTime);
    // end add
};

#endif
