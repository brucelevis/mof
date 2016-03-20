//
//  GuildBoss.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-9-27.
//
//

#pragma once
#include "Boss.h"
#include <set>

struct GuildBossParam
{
    int startedSec;
    int guildid;
    int filed;
};

class GuildBoss : public Boss
{
public:
    GuildBoss(){}
    ~GuildBoss(){}
    
    virtual void OnActiveOpen(void * param) ;//开波
    
    virtual void OnBossCreated();
    
    virtual void OnBossKilled();  // 成功
    
    virtual void OnActiveClose(void * param);//结束
    virtual void OnReqEnter(Role* role , req_enter_boss_scene& req);
    virtual int getClientRankNum();
    
protected:
    void SendReward(bool succed);
    void MailReward(int roleid, int rank, string rolename ,int dmg);
    void MailFailReward(int roleid,int rank ,string rolename , int dmg);
    
    void CreateMail(int roleid ,int rank , string rolename , int constrib , int exploit);

    
    bool RejectEnterRoom(Role* src);
    
protected:
    int         mGuildId;           //公会ID
    int         mBossExp;           //开波时的经验，防止公会被解散了出错
};