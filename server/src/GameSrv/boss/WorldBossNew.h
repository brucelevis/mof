//
//  WorldBossNew.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-9-27.
//
//
#pragma once
#include "Boss.h"

/*!
 */


class WorldBossNew : public Boss
{
public:    
    WorldBossNew(){}
    ~WorldBossNew(){}
    
    virtual void OnActiveOpen(void * param) ;//开波
    
    virtual void OnBossCreated();
    virtual void OnBossKilled();  // 成功
    
    virtual void OnActiveClose(void * param);//结束
    
    virtual void OnReqEnter(Role* role , req_enter_boss_scene& req);
    virtual int getClientRankNum();
    
    virtual float hurtAwardBattlePointFactor();//伤害奖励战功
    virtual float hurtAwardGoldFactor();//伤害奖励金币
    
protected:
    virtual int getRoundHurtMax();
    
    void SendReward(bool succed);
    void SuccedReward(int roleid, int rank, string rolename,int selfdmg, int rank1dmg);
    void FailedReward(int roleid, int rank, string rolename,int selfdmg, int rank1dmg);
    void CreateMail(int roleid, int rank, const string& rolename , bool succed);
    
    void rollBackBeforeBoss();
};
