//
//  SkillAction.h
//  GoldOL
//
//  Created by Forest Lin on 13-2-26.
//
//  技能实现类的合集

#ifndef GoldOL_SkillAction_h
#define GoldOL_SkillAction_h

#include "WorldCreature.h"
#include "DataCfg.h"
#include "WorldSkill.h"

#define GameObject SceneObject
class WorldCreature;

#include "WorldObject.h"

class SkillAction : public WorldObject
{
public:
    static bool globalInit(int maxObjNum);
    
protected:
    //所有的SkillAction

    READWRITE(SkillActionType, mType, Type)
    //目前只有buff类和伤害类技能需要区分施放者和作用者。其它类型施放者和作用者都是相同的---linshusen
    WorldCreature* mSrcObj;                           //施放技能的物体
    WorldCreature* mTargetObj;                        //技能作用的物体
    
    int mSkillID;                           //技能ID
    SkillEffectVal* mSkillEffect;           //技能效果
    SkillCfgDef* mSkillData;
    bool mHasSkillEnd;                        //是否技能已结束。一般只用于施法时间和持续时间相等时。
    
    std::vector<GameObject*> mGenarateObjs;        //放技能过程中创建的物体
    std::vector<SEL_SCHEDULE> mSchedulers;
    
    //把obj添加到临时生成表
    void addGenarateObj(GameObject* obj);

    void schedule(SEL_SCHEDULE selector, float interval);
    void scheduleOnce(SEL_SCHEDULE selector, float delay);

    
public:
    
    SkillAction(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    virtual ~ SkillAction();

    bool getHasSkillEnd(){return mHasSkillEnd;}
    
    virtual void start();                   //开始技能
    virtual void perform(float dt){}        //执行
    virtual void end(float dt);             //技能结束
    virtual void castEnd(float dt);         //施法结束
    virtual void checkAttack(float dt);     //检测可以攻击的物体，并且攻击
    
    virtual void clearSelf();
    
    virtual void endStrongly();             //强行结束
    
    //计算攻击范围
    virtual CCRect calAttackArea(GameObject* attackObj);

};

//冲锋类技能
class Rush : public SkillAction
{
private:
    Rush(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :SkillAction(src, target, skillID, skillEffect)
    {
        setType(eRush);
    }
public:
    virtual ~ Rush(){}
    
    static Rush* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束

    
};

//飞行类技能
class Fly : public SkillAction
{
private:
    Fly(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :SkillAction(src, target, skillID, skillEffect)
    {
        mFlyObj = NULL;
        setType(eFly);
    }
    
    GameObject* mFlyObj;
public:
    virtual ~ Fly(){}
    
    static Fly* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    virtual void checkAttack(float dt);     //检测可以攻击的物体，并且攻击
    
    virtual CCRect calAttackArea(GameObject* attackObj);
};

//持续性攻击的飞行类技能
class ContinueFly : public SkillAction
{
private:
    ContinueFly(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :SkillAction(src, target, skillID, skillEffect)
    {
        mFlyObj = NULL;
        setType(eContinueFly);
    }
    
    GameObject* mFlyObj;
public:
    virtual ~ ContinueFly(){}
    
    static ContinueFly* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    virtual void checkAttackFirst(float dt);
    virtual void checkAttack(float dt);     //检测可以攻击的物体，并且攻击
    
    virtual CCRect calAttackArea(GameObject* attackObj);
};



//攻击固定特效类技能
class StaticEffect : public SkillAction
{
private:
    StaticEffect(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :SkillAction(src, target, skillID, skillEffect)
    {
        setType(eStaticEffect);
    }
public:
    virtual ~ StaticEffect(){}
    
    static StaticEffect* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    
};

//伤害特效类技能(和固定特效的区别在于：特效需要显示在目标的中心)
class HurtEffect : public SkillAction
{
private:
    HurtEffect(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :SkillAction(src, target, skillID, skillEffect)
    {
        setType(eHurtEffect);
    }
public:
    virtual ~ HurtEffect(){}
    
    static HurtEffect* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    
};

//buff效果的回调函数，用于逻辑数据的同步。BuffEffect只管表现，不处理逻辑数据
typedef void (*BuffCallback)(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal *effect );

//BUFF类技能
class BuffEffect : public SkillAction
{
protected:
    BuffCallback mBuffStartCallback;      //开始buf的回调
    BuffCallback mBuffEndCallback;        //结束buf的回调
    BuffCallback mBuffIntervalCallback;   //间隔buf的回调
    bool         mStarted;
public:
    BuffEffect(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :SkillAction(src, target, skillID, skillEffect)
    {
        mBuffStartCallback = NULL;
        mBuffIntervalCallback = NULL;
        mBuffEndCallback = NULL;
        mStarted = false;
        setType(eBuffEffect);
    }
    virtual ~ BuffEffect(){}
    
    static BuffEffect* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    virtual void endStrongly();             //强行结束
    void intervalBufCheck();
    
    void setBuffCallback(BuffCallback startCallback, BuffCallback intervalCallback, BuffCallback endCallback)
    {
        mBuffStartCallback = startCallback;
        mBuffIntervalCallback = intervalCallback;
        mBuffEndCallback = endCallback;
    }
};

//吸血技能
class SuckBloodEffect : public SkillAction
{
    BuffCallback mIncrHpCallback;
public:
    SuckBloodEffect(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
        :SkillAction(src, target, skillID, skillEffect)
    {
        mIncrHpCallback = NULL;
        setType(eSuckBloodEffect);
    }
    
    virtual ~ SuckBloodEffect(){}
    
    static SuckBloodEffect* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    virtual void endStrongly();             //强行结束
    
    void intervalCheck(float dt);//间歇的显示加血效果
    
    //加血的回调
    void setIncrHpCallback(BuffCallback incrHpCallback)
    {
        mIncrHpCallback = incrHpCallback;
    }
};

//变身类技能
class NewBodyEffect : public BuffEffect
{
private:
    NewBodyEffect(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :BuffEffect(src, target, skillID, skillEffect)
    {
        mNewBody = NULL;
        setType(eNewBodyEffect);
    }
    
    GameObject* mNewBody;
public:
    virtual ~ NewBodyEffect(){}
    
    static NewBodyEffect* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    
};

//可移动的变身类攻击特效 如：旋风斩
class MovableNewBodyAttackEffect : public BuffEffect
{
private:
    MovableNewBodyAttackEffect(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :BuffEffect(src, target, skillID, skillEffect)
    {
        mNewBody = NULL;
        setType(eMovableNewBodyAttackEffect);
    }
    
    GameObject* mNewBody;
public:
    virtual ~ MovableNewBodyAttackEffect(){}
    
    static MovableNewBodyAttackEffect* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    
};

//锁定攻击目标的技能
class LockTargetEffect : public SkillAction
{
private:
    LockTargetEffect(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :SkillAction(src, target, skillID, skillEffect)
    {
        mLockPoints.clear();
        setType(eLockTargetEffect);
    }
    
    std::vector<CCPoint> mLockPoints;
public:
    virtual ~ LockTargetEffect(){}
    
    static LockTargetEffect* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    virtual void checkAttack(float dt);     //检测可以攻击的物体，并且攻击
    
    CCRect calAttackArea(CCPoint pos);      //根据一个位置来计算攻击区域
};


//被击退类
class KnockBackAction : public SkillAction
{
private:
    KnockBackAction(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :SkillAction(src, target, skillID, skillEffect)
    {
        mOwner = NULL;
        setType(eKnockBackAction);
    }
    
    WorldCreature* mOwner; //施放冲锋的人
public:
    virtual ~ KnockBackAction(){}
    
    static KnockBackAction* create(WorldCreature* src, WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    void setOwner(WorldCreature* owner);
    virtual void checkEnd(float dt);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    virtual void endStrongly();             //强行结束
};

//震屏效果
class ShakeScreenEffect : public SkillAction
{
private:
    ShakeScreenEffect(WorldCreature* target, int skillID, SkillEffectVal* skillEffect)
    :SkillAction(target, target, skillID, skillEffect)
    {
        setType(eShakeScreenEffect);
    }
    
public:
    virtual ~ ShakeScreenEffect(){}
    
    static ShakeScreenEffect* create(WorldCreature* target, int skillID, SkillEffectVal* skillEffect);
    
    virtual void start();                   //开始技能
    virtual void perform(float dt);         //执行
    virtual void end(float dt);             //技能结束
    
};

#define BeAttack 
static const float sMinDistanceWithAttacker = 100;
#endif
