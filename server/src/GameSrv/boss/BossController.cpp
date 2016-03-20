//
//  ActivityWorldBossController.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-9-27.
//
//

#include "BossController.h"
#include "WorldBossNew.h"
#include "BossFactory.h"
#include "GameLog.h"
#include "GuildBoss.h"
#include "GuildMgr.h"
#include "Activity.h"
#include <map>


typedef std::map<int,int>   GuildBossId;
static int mWorldBossId = 0;
static GuildBossId mGuildBossId; // guild id -> boss id


static void OnWorldBossActiveOpen(int actId, int number, string& params)
{
    int startedSec = Utils::safe_atoi(params.c_str(), 0);
    
    //造boss
    WorldBossNew* boss = static_cast<WorldBossNew*>( SBossFactory.create(eBossType_WorldBoss) );
    xyassert(boss);
    
    mWorldBossId = boss->GetBossId();
    
    //开波
    boss->openActivity(&startedSec);
}

static void OnWorldBossActiveClose(int actId, int number, string& params)
{
    WorldBossNew* boss = static_cast<WorldBossNew*>( SBossFactory.getBoss(mWorldBossId) );
    xyassert(boss);
    
    //收工
    boss->closeActivity(NULL);
    
    //回收boss
    SBossFactory.release(mWorldBossId);
    
    mWorldBossId = 0;
}


static void OnGuildBossActiveOpen(int actId, int number, string& params)
{
    mGuildBossId.clear();
    
    const vector<int>& vec = SGuildMgr.GetAllGuild();
    for (int i = 0; i < vec.size(); ++i)
    {
        Xylog log(eLogName_GuildBossActivity, 0);
        
        Guild& guild = SGuild( vec[i] );
        
        log<<guild.getGuildName();
        
        GuildBossLvlDef* def = GuildCfgMgr::getGuildBossLvlDef( guild.getBossExp() );
        if(NULL == def)
            continue;
        
        if( guild.getLevel()  < def->minGuildLvl )
        {
            log<<"lvlfail";
            
            Xylog faillog(eLogName_GuildCommon, 0);
            faillog<<guild.getGuildName()<<"公会等级不够，不能开公会BOSS活动";
            continue;
        }
        
        
        GuildBossParam param;
        param.startedSec = Utils::safe_atoi(params.c_str(), 0);
        param.guildid = vec[i];
        param.filed = 2;
        
        //造boss
        Boss* boss = SBossFactory.create(eBossType_GuildBoss);
        xyassert(boss);
        
        //开波
        boss->openActivity(&param);
        
        mGuildBossId[param.guildid] = boss->GetBossId();
        
        log<<"succ";
    }
}

static void OnGuildBossActiveClose(int actId, int number, string& params)
{
    
    Xylog log(eLogName_GuildBossActivity, 0);
    log<<"Close";
    
    for( GuildBossId::iterator iter = mGuildBossId.begin();
        iter != mGuildBossId.end() ; ++ iter)
    {
        int bossid = iter->second;
        Boss* boss = SBossFactory.getBoss(bossid) ;
        xyassert(boss);
        
        //收工
        boss->closeActivity(NULL);
        
        //回收boss
        SBossFactory.release(bossid);
    }
    
    mGuildBossId.clear();
}


int BossActivityController::getWorldBossId()
{
    return mWorldBossId;
}

int BossActivityController::getGuildBossId(int guildid)
{
    GuildBossId::iterator iter = mGuildBossId.find(guildid);
    if(iter != mGuildBossId.end())
        return iter->second;
    return 0;
}


void BossActivityController::Init()
{    
    // 世界boss
    SActMgr.SafeCreateTimer(ae_world_boss, att_open, 0, OnWorldBossActiveOpen, "worldboss");
    SActMgr.SafeCreateTimer(ae_world_boss, att_close, 0, OnWorldBossActiveClose, "");
    
    // 公会boss
    SActMgr.SafeCreateTimer(ae_guild_boss, att_open, 0, OnGuildBossActiveOpen, "guildboss");
    SActMgr.SafeCreateTimer(ae_guild_boss, att_close, 0, OnGuildBossActiveClose, "");
}


BossActivityController g_BossActivityController;
