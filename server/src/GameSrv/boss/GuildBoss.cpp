//
//  GuildBoss.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-9-27.
//
//

#include "GuildBoss.h"
#include "GuildMgr.h"
#include "GameLog.h"
#include "mail_imp.h"
#include "Game.h"
#include "daily_schedule_system.h"
#include "EnumDef.h"
#include "Role.h"

extern bool checkCanEnterCopy(Role* role, int copyid);

bool GuildBoss::RejectEnterRoom(Role* src)
{
    Guild& guild = SGuild(mGuildId);
    if(guild.isNull())
        return true;
    
    return ! guild.getMembers().exist( src->getInstID() );
}

//开波
void GuildBoss::OnActiveOpen(void * param) 
{
    xyassert(param);
    GuildBossParam* bossParam = (GuildBossParam*)param;
    
    mGuildId = bossParam->guildid;
    Guild& guild = SGuild( mGuildId );
    mBossExp = guild.getBossExp();
    
    GuildBossLvlDef* def = GuildCfgMgr::getGuildBossLvlDef(mBossExp);
    xyassert(def);
    setSceneID(def->sceneId);
    
    MonsterCfgDef* md = MonsterCfg::getCfg(def->monsterId);
    xyassert(md);
    MonsterInit(md);
    
    setTpltID( def->monsterId );
    
    mCreateTime = time(NULL) + GuildCfgMgr::getBossPrepareTime() - bossParam->startedSec;
}

void GuildBoss::OnBossCreated()
{
    Init();
    // 关掉boss伤害拦截，减轻客户端看似回血的bug
    VALID_HURT_FACTOR = 70;
}


//结束
void GuildBoss::OnActiveClose(void * param)
{
    if (getState() != eDEAD) {
        SendReward(false);
    }
    
    //杀死BOSS后，经验归0
    Guild& guild = SGuild( mGuildId );
    guild.addBossExp( -1 * guild.getBossExp() );
}

void GuildBoss::OnReqEnter(Role* role , req_enter_boss_scene& req)
{
    ack_enter_boss_scene ack;
    if( RejectEnterRoom(role) ){
        ack.err = eNotGuildMember;
        sendNetPacket(role->getSessionId(), &ack);
        return;
    }
    
    //判断可否进入，防止同时进入多个副本
    if (!checkCanEnterCopy(role, this->getSceneID())) {
        return;
    }
    
    GuildBossKillDef* killDef = GuildCfgMgr::getGuildBossKillCfg();
    xyassert(killDef);
    
    if(this->Add(role, 0, 0, false)){
        ack.err = eWorldBossOK;
        ack.sceneid = this->getSceneID();
        ack.bossid = this->getTpltID();
        ack.bossHP = this->getHp();
        ack.reborn_seconds = killDef->rebornSeconds;
        ack.elaps_seconds = mElapsedSecond;
        ack.remain_seconds = mState >= kBossCreated && Game::tick - mCreateTime > 0 ? 0 : mCreateTime - Game::tick;
        
        role->setPreEnterSceneid( this->getSceneID() );
        if (role->getDailyScheduleSystem()) {
            role->getDailyScheduleSystem()->onEvent(kDailyScheduleGuildBoss, 0, 1);
        }
        
    }else{
        ack.err = eBossUnknowError;
    }
    
    sendNetPacket(role->getSessionId(), &ack);
}

void GuildBoss::OnBossKilled()  // 成功
{
    SendReward(true);
    ClearAll();
}

void GuildBoss::SendReward(bool succed)
{
    Guild& guild = SGuild( mGuildId);
    if(guild.isNull())//注意这里有可能被解散了
    {
        xyerr("公会被解散，公会BOSS不发奖励%d",mGuildId);
        return;
    }
    
    // sort all hurts
    std::vector<obj_hurts> hurt; // 所有伤害int:roleid
    for (HurtMap::iterator it=mHurts.begin();
         it!=mHurts.end(); it++) {
        hurt.push_back(it->second.totalHurt);
    }
    std::sort(hurt.begin(), hurt.end(),sortRank);
    
    // send
    int rank=1;
    for (vector<obj_hurts>::iterator iter = hurt.begin();
         iter!=hurt.end(); iter++, rank++)
    {
        static int rank1dmg;
        if (rank==1) { //保证rank1dmg  >= 1
            rank1dmg = (*iter).dmg;
            rank1dmg = rank1dmg < 1 ? 1 : rank1dmg;
        }
        
        
        int roleid      = (*iter).roleid;
        string rolename = (*iter).rolename;

        if(succed)
            MailReward(roleid,rank,rolename , iter->dmg);
        else
            MailFailReward(roleid,rank ,rolename , iter->dmg);

    }
    
    if(succed)
    {
        //加公会财富
        GuildBossLvlDef* def = GuildCfgMgr::getGuildBossLvlDef( mBossExp );
        xyassert(def);
        
        string allmembers_str = GameTextCfg::getString("1045");
        string guildboss_str = GameTextCfg::getString("1046");
        
        guild.addFortuneConstruction(def->fortuneAward, def->constructionAward, allmembers_str.c_str(), guildboss_str.c_str());
                                     //"全体会员","公会BOSS");
    }
    
    mHurts.clear();mRank.clear();
}

void GuildBoss::MailFailReward(int roleid,int rank ,string rolename , int dmg)
{    
    Guild& guild = SGuild( mGuildId );
    if( ! guild.getMembers().exist(roleid))
    {
        return;
    }
    
    Role* role = SRoleMgr.GetRole(roleid);
    
    if (role) {
        
        role->backToCity();
        
        notify_boss_fail noti;
        
        sendNetPacket(role->getSessionId(),&noti);
    }

    GuildBossLvlDef* def = GuildCfgMgr::getGuildBossLvlDef( mBossExp );
    xyassert(def);
    
    GuildBossKillDef* killDef = GuildCfgMgr::getGuildBossKillCfg();
    xyassert(killDef);
    
    int bonus = killDef->getRankBonus(rank);
    
    int constribute = bonus * def->constributeAwardFactor;
    int exploit = bonus * def->exploitAwardFactor;

    //有伤害才发邮件 wangzhigang 2014-10-10
    //if (dmg >= 1) {
        CreateMail(roleid,rank ,rolename , constribute ,exploit);
    //}
}

void GuildBoss::MailReward(int roleid, int rank, string rolename ,int dmg)
{
    GuildBossLvlDef* def = GuildCfgMgr::getGuildBossLvlDef( mBossExp );
    xyassert(def);
    
    GuildBossKillDef* killDef = GuildCfgMgr::getGuildBossKillCfg();
    xyassert(killDef);
    
    Guild& guild = SGuild( mGuildId );
    if( ! guild.getMembers().exist(roleid))
    {
        return;
    }
    
    Role* role = SRoleMgr.GetRole(roleid);
    
    int bonus = killDef->getRankBonus(rank);
    
    notify_dead_boss noti;
    noti.rolewhokill = mWhokill;
    
    noti.guildFortune = def->fortuneAward;
    noti.guildConstruction = def->constructionAward;
    noti.constribute = bonus * def->constributeAwardFactor;
    noti.exploit = bonus * def->exploitAwardFactor;
    
    noti.hurts = mHurts[roleid].totalHurt.dmg;
    
    if (role) {
        
        role->backToCity();
        sendNetPacket(role->getSessionId(),&noti);
    }

    //有伤害才发邮件 wangzhigang 2014-10-10
    //if (dmg >= 1) {
        CreateMail(roleid,rank ,rolename , noti.constribute ,noti.exploit);
    //}
}


void GuildBoss::CreateMail(int roleid ,int rank , string rolename , int constrib , int exploit)
{

    stringstream attach;
    attach<<"constrib "<< constrib <<"*1;";
    attach<<"exploit "<< exploit <<"*1;";
    
    MailFormat *f = MailCfg::getCfg("guildboss_over");
    if (f)
    {
        //snprintf(buf, 500, f->content.c_str(), rank, constrib, exploit);
        string rank_str = Utils::itoa(rank);
        string constrib_str = Utils::itoa(constrib);
        string exploit_str = Utils::itoa(exploit);
        string mail_content = f->content;
        
        //公会boss活动结束，你获得第%d名奖励：%d个人贡献，%d功勋奖励
        if (3 != find_and_replace(mail_content, 3, rank_str.c_str(), constrib_str.c_str(), exploit_str.c_str()))
        {
            log_warn("mail content format error. tag: [guildboss_over]");
        }
        
        
        bool  ret = sendMail(0,
                             f->sendername.c_str(),
                             rolename.c_str(),
                             f->title.c_str(),
                             mail_content.c_str(),
                             attach.str().c_str(),
                             Utils::itoa(roleid).c_str());
        
        if( false ==  ret ){
            StoreOfflineItem(roleid,attach.str().c_str());
        }
        
        LogMod::addLogSendRoleMail(roleid, f->title.c_str(),attach.str().c_str(), ret);
        
        Xylog log(eLogName_GuildBossAward, roleid);
        log << constrib << exploit << "";
    }
    else
    {
        log_warn("mailMultilanguage.ini [guildboss_over] not found.");
    }
}


int GuildBoss::getClientRankNum()
{
    GuildBossKillDef* killDef = GuildCfgMgr::getGuildBossKillCfg();
    if(NULL == killDef)
        return 2;
    return killDef->clientRankNum;
}
