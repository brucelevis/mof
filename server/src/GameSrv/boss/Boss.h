//
//  Boss.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-9-27.
//
//

#pragma once
#include <string>
#include <map>
#include <vector>
#include "MultiPlayerRoom.h"
#include "Monster.h"

/* boss 是活动房间也是一个Monster
 用到idle和died两状态
 tplt,hp,maxmp,等
 
 totalHurt          是总伤害，用来排名
 totalRoundHurt     奖励伤害，按30秒为一个轮，每轮可奖励伤害有最大值，超过此值会忽略
 currentRoundHurt
 
 */

#define BOSS_ROOM_NAME "Boss"

struct BossHurt
{
    obj_hurts totalHurt;    // 总伤害
    int totalRoundHurt;     // 分段伤害
    int currentRoundHurt;   // 当前段伤害
};

enum BossState
{
    kBossActivityOpen,
    kBossCreated,
    kBossKilled,
    kBossActivityClose
};

class Boss: public BaseActRoom, public Monster
{
public:
    Boss();
    virtual ~Boss();
public:
    void openActivity(void* param)
    {
        mActivityState = kBossActivityOpen;
        OnActiveOpen(param);
    }
    void closeActivity(void* param)
    {
        mActivityState = kBossActivityClose;
        OnActiveClose(param);
    }
    
    virtual void OnActiveOpen(void * param) = 0;//开波
    
    virtual void OnBossCreated() {}
    virtual void OnBossKilled()  {}  // 成功
    
    virtual void OnActiveClose(void * param)= 0;//结束
    
    virtual void OnReqEnter(Role* role , req_enter_boss_scene& req)= 0;// 请求进入
    
    virtual int getClientRankNum(); // 排行榜
    virtual int getRoundHurtMax();  // 分段最大伤害
    
    
    virtual float hurtAwardBattlePointFactor();//伤害奖励战功系数
    virtual float hurtAwardGoldFactor();//伤害奖励金币系数
    
protected:
    void Init();    //初始化
    
    void createBoss();
    void death();
    

public:
    int GetBossId();
    void beAtk(Role* role,req_attack&);        // 被攻击
    void beSkill(Role* role, req_skill_hurt&);   // 被skill
    void SendRoleBattleInfo(int roleid, int reqRole);// 发送同屏role战斗信息
    
    int getActivityState() {return mActivityState;}

public:
    virtual void OnUpdate(float dt); // 场景帧
    virtual void OnHeartBeat(){}// 场景心跳
    virtual void OnUpdate(Obj* obj, OBJ_INFO& info, float dt){}// 对象帧
    virtual void OnHeartBeat(Obj* obj, OBJ_INFO& info){}// 对象心跳
    virtual void OnEnterRoom(Obj* obj);// 有对象加入场景
    virtual void OnLeaveRoom(Obj* obj){}// 对象离开
    
protected:
    void DamageInput(Role* src, int objtype , int dmg );      // 玩家伤害输入
    
    void SyncBossHPAndRank(); // 同步HP和排行
    void ResortRank();              // 排序伤害来结算奖励
    void InputRank(int lastRoleid); // 有受伤时排序排行
    void RefreshRoundHurt(); //刷新分段伤害
    int hurtConvertAwardPoint(int roleid);//伤害值转换为奖励点
    
    
    bool static sortRank(obj_hurts h1, obj_hurts h2){
        return h1.dmg > h2.dmg;
    }
    
protected:
    // 防作弊处理，10秒内攻击次数不能超过
    const float VALID_HURT_SECOND; // 10S
    int VALID_HURT_FACTOR;// 30 倍 ,不同类型的boss不一样
    const int VALID_HURT_HIT_WARRIOR;   // 剑士 22
    const int VALIE_HURT_HIT_ASSASSIN;   // 刺客 26
    const int VALIE_HURT_HIT_MAGE;   // 法师 18
    
    
    std::map<int,int> mValidHurt;
    float mRefreshValidHurt;
    void RefreshValidHurt(float dt);
    bool VerifyDmgOk(Role* src  ,int dmg, float clientElapse);
    
protected:
    int                         mBossId;        // 全球唯一标记
    float                       mSynHPTimer;    // 同步HP时间
    typedef std::map<int, BossHurt> HurtMap;
    typedef std::map<int, BossHurt>::iterator HurtMapIter;
    HurtMap                     mHurts;          // 所有伤害int:roleid
    std::vector<obj_hurts>      mRank;          // 实时伤害排名 roleid

    float                      mRefreshRoundTime;// 刷新分段伤害奖励时间点
    string                      mWhokill;       // 最后一击
    
    float                       mElapsedSecond;   // 活动开始多少秒
    
    int mCreateTime;
    
    int mActivityState;
};


