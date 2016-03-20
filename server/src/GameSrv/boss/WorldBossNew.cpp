//
//  WorldBossNew.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-9-27.
//
//

#include "WorldBossNew.h"
#include "GameLog.h"
#include "mail_imp.h"
#include "Game.h"
#include "daily_schedule_system.h"
#include "EnumDef.h"
#include "Honor.h"
#include "Role.h"

//开波
void WorldBossNew::OnActiveOpen(void * param)
{
    int startedSec = *((int*)param);
    mCreateTime = time(NULL) + WorldBossCfg::preparetime - startedSec;
    
    int bossid;
    
    redisReply* rreply = redisCmd("get worldboss");
    if (rreply->type == REDIS_REPLY_NIL)
    {
        WorldBossCfgDef* def = WorldBossCfg::getFirstBossCfg();
        bossid = def->bossid;
        doRedisCmd("set worldboss %d",bossid);
    }else{
        bossid = Utils::safe_atoi(rreply->str);
    }
    freeReplyObject(rreply);
    
    MonsterCfgDef* md = MonsterCfg::getCfg(bossid);
    if(md == NULL)
    {
        log_info("worldboss bossid error: " << bossid);
        return;
    }
    
    MonsterInit(md);
    setTpltID(bossid);
    WorldBossCfgDef* def = WorldBossCfg::getBossCfg(bossid);
    if(def == NULL)
    {
        log_info("worldboss bossid error: " << bossid);
        return;
    }
    setSceneID(def->sceneid);
}

void WorldBossNew::OnBossCreated()
{
    //这里读配置取出boss 属性 ，读出世界boss的一些属性
    Init();
}


void WorldBossNew::OnBossKilled()
{
    SendReward(true);
    
    WorldBossCfgDef *def = WorldBossCfg::getBossCfg(getTpltID());
    int passtime = def->passtime;
    if (mElapsedSecond <= passtime){
        if (def->nextBoss !=0){
            doRedisCmd("set worldboss %d",def->nextBoss);
        }
        doRedisCmd("set worldboss_diedtime %d",time(0));
    }
    
    if( mElapsedSecond >= def->rollBackTime )
    {
        rollBackBeforeBoss(); // 回滚上一个boss
    }
    
    
    ClearAll();
    
    if(def->nextBoss == 0)
        return;
    
    MonsterCfgDef *next = MonsterCfg::getCfg(def->nextBoss);
    MonsterInit(next);
    
    WorldBossCfgDef* def2 = WorldBossCfg::getBossCfg(def->nextBoss);
    setTpltID(def2->bossid);
}

//结束
void WorldBossNew::OnActiveClose(void * param)
{
    if (getState() != eDEAD) {
        SendReward(false);// 失败
        rollBackBeforeBoss(); // 回滚前一个boss
    }
}

//进入场景
void WorldBossNew::OnReqEnter(Role* role , req_enter_boss_scene& req)
{
    if (role->getPreEnterSceneid() > 0) {
        return;
    }
    
    ack_enter_boss_scene ack;
    if(role->getLvl() < WorldBossCfg::enter_lvl){
        ack.err = eLvlNoReach; // lvl limit
        sendNetPacket(role->getSessionId(), &ack);
        return;
    }
    
    if(this->Add(role, 0, 0, false)){
        ack.err = eWorldBossOK;
        ack.sceneid = this->getSceneID();
        ack.bossid = this->getTpltID();
        ack.bossHP = this->getHp();
        ack.reborn_seconds = WorldBossCfg::reborn_seconds;
        ack.elaps_seconds = mElapsedSecond;
        ack.remain_seconds = mState >= kBossCreated && Game::tick > mCreateTime ? 0 : mCreateTime - Game::tick;
        
        role->setPreEnterSceneid(this->getSceneID());
        
        
        if (role->getDailyScheduleSystem()) {
            role->getDailyScheduleSystem()->onEvent(kDailyScheduleWorldBoss, 0, 1);
        }
        
//        role->setCurrSceneId( this->getSceneID() );
    }else{
        ack.err = eBossUnknowError;
    }
    
    sendNetPacket(role->getSessionId(), &ack);
}



void WorldBossNew::SendReward(bool succed)
{
    // sort all hurts
    std::vector<obj_hurts> hurt; // 所有伤害int:roleid
    
    for (HurtMap::iterator it = mHurts.begin(); it != mHurts.end(); ++it)
    {
        hurt.push_back( it->second.totalHurt);
    }
    
    std::sort(hurt.begin(), hurt.end(),sortRank);
    
    // send
    int rank=1;
    for (vector<obj_hurts>::iterator iter = hurt.begin();
         iter!=hurt.end(); iter++, rank++)
    {
        static int rank1dmg;
        if (rank==1) { //
            rank1dmg = (*iter).dmg;
            rank1dmg = rank1dmg < 1 ? 1 : rank1dmg;
        }
        
        int roleid      = (*iter).roleid;
        string rolename = (*iter).rolename;
        if (succed)
            SuccedReward(roleid,rank,rolename, (*iter).dmg, rank1dmg);
        else
            FailedReward(roleid, rank,rolename, (*iter).dmg, rank1dmg);
			
		// 称号检查
		if (rank == 1) {
			SHonorMou.procHonor(eHonorKey_BossRankOne, roleid);
		}
    }
    
    mHurts.clear();
    mRank.clear();
}

void WorldBossNew::CreateMail(int roleid, int rank, const string& rolename , bool succed)
{
    float succFactor = 1.0f;
    if(! succed)
        succFactor = 0.5f;
    
    WorldBossCfgDef* def = WorldBossCfg::getBossCfg(getTpltID());
    int bonus           = WorldBossCfg::getAwardBonus(rank);
    
    notify_dead_boss noti;
    noti.rolewhokill = mWhokill;
    
    noti.gold = bonus * def->goldaward * succFactor;
    noti.exp = bonus * def->expaward * succFactor;
    noti.bat_point  = bonus * def->batpointaward * succFactor;
    noti.constell = bonus * def->constellaward * succFactor;
    noti.item = WorldBossCfg::getItemAward(rank);
    
    noti.hurts = mHurts[roleid].totalHurt.dmg;
    
    noti.hurt_bat_point = hurtConvertAwardPoint(roleid)  * hurtAwardBattlePointFactor() ;
    noti.hurt_gold = hurtConvertAwardPoint(roleid) * hurtAwardGoldFactor();
    
    Role* role = SRoleMgr.GetRole(roleid);
    
    if (role) {
        if (role->getCurrentActRoom() == this) {
            role->backToCity();
        }
        
        if( succed)
            sendNetPacket(role->getSessionId(),&noti);
        else{
            notify_boss_fail fail;
            fail.gold = noti.gold;
            fail.exp = noti.exp;
            fail.bat_point = noti.bat_point;
            fail.constell = noti.constell;
            sendNetPacket(role->getSessionId(),&fail);
        }
    }
    
    MailFormat *f = NULL;
    
    if(succed)
    {
        f = MailCfg::getCfg("worldboss_killed");//世界Boss被击败
    }
    else
    {
        f = MailCfg::getCfg("worldboss_not_killed"); //世界Boss未被击败
    }
    if (f)
    {
        string mail_title = f->title;
        
        string rank_str = Utils::itoa(rank);
        
        vector<string> attaches;
        
        string awards_str1;
        {
            RewardStruct reward1;
            vector<string> contents1;
            reward1.reward_gold = noti.gold;
            reward1.reward_exp = noti.exp;
            reward1.reward_consval = noti.constell;
            reward1.reward_constrib = noti.constribute;
            reward1.reward_batpoint = noti.bat_point;
            ItemArray items1;
            if (noti.item > 0) {
                items1.push_back(ItemGroup(noti.item, 1));
            }
            reward2MailFormat(reward1, items1, &contents1, &attaches);
            awards_str1 = StrJoin(contents1.begin(), contents1.end(), ",");
        }
        
        string awards_str2;
        {
            vector<string> contents2;
            ItemArray items2;
            RewardStruct reward2 = RewardStruct();
            reward2.reward_gold = noti.hurt_gold;
            reward2.reward_batpoint = noti.hurt_bat_point;
            reward2MailFormat(reward2, items2, &contents2, &attaches);
            awards_str2 = StrJoin(contents2.begin(), contents2.end(), ",");
        }
        
        string attachstr = StrJoin(attaches.begin(), attaches.end(), ";");
        
        string mail_content = f->content;
        //你獲得第{:str:}名獎勵：{:str:}傷害獎勵:{:str:}
        find_and_replace(mail_content, 3, rank_str.c_str(), awards_str1.c_str(), awards_str2.c_str());
        
        bool ret = sendMail(0,
                            f->sendername.c_str(),
                            rolename.c_str(),
                            mail_title.c_str(),
                            mail_content.c_str(),
                            attachstr.c_str(),
                            Utils::itoa(roleid).c_str());
        
        if( false ==  ret )
        {
            StoreOfflineItem(roleid, attachstr.c_str());
        }
        
        Xylog log(eLogName_WorldBoss, roleid);
        log<<rank<<succed<<noti.gold<<noti.exp<<noti.bat_point<<noti.constell<<noti.item
        << noti.hurts <<hurtConvertAwardPoint(roleid)<<noti.hurt_bat_point<<noti.hurt_gold<< getTpltID()
        <<"rank,killsucc,gold,exp,batpoint,constel,item,totalhurt,awardhurt,hurtbatpoint,hurtgold,templateid";
        
    } //if (f)
    
}

void WorldBossNew::SuccedReward(int roleid, int rank, string rolename,int selfdmg, int rank1dmg)
{
    CreateMail(roleid, rank, rolename, true);
    
    if (strcmp(mWhokill.c_str(),rolename.c_str()) == 0) {
        
        WorldBossCfgDef* def = WorldBossCfg::getBossCfg(getTpltID());
        
        stringstream attach2;
        int bat = WorldBossCfg::lasthit * def->batpointaward;
        attach2<<"batpoint "<< bat <<"*1;";
        
        MailFormat *f = MailCfg::getCfg("worldboss_last_hit");
        if (f)
        {
            //战功
            string strbatpoint = MoneyCfg::getName("batpoint");
            string mail_content = f->content;
            string bat_str = Utils::itoa(bat);
            //你獲得最後一擊獎勵：{:str:}{:str:}
            find_and_replace(mail_content, 2, bat_str.c_str(), strbatpoint.c_str());
            
            bool ret2 = sendMail(0,
                                 f->sendername.c_str(),
                                 rolename.c_str(), f->title.c_str(),
                                 mail_content.c_str(),
                                 attach2.str().c_str(),
                                 Utils::itoa(roleid).c_str());
            
            if( false ==  ret2 )
            {
                StoreOfflineItem(roleid,attach2.str().c_str());
            }
            LogMod::addLogSendRoleMail(roleid, f->title.c_str(), attach2.str().c_str(), ret2);
		}
		// 称号检查
		SHonorMou.procHonor(eHonorKey_BossLast, roleid);
    }
    
}

void WorldBossNew::FailedReward(int roleid, int rank, string rolename,int selfdmg, int rank1dmg)
{
    //if (selfdmg == 0) {
    //    return;
    //}
    
    CreateMail(roleid, rank, rolename, false);
}

int WorldBossNew::getClientRankNum()
{
    return WorldBossCfg::clientranknum;
}

int WorldBossNew::getRoundHurtMax()
{
    WorldBossCfgDef* def     = WorldBossCfg::getBossCfg(getTpltID());
    return WorldBossCfg::round_max_hurt_base * def->level_factor;
}

//伤害奖励战功
float WorldBossNew::hurtAwardBattlePointFactor()
{
    return WorldBossCfg::hurt_batpointaward;
}

//伤害奖励金币
float WorldBossNew::hurtAwardGoldFactor()
{
    return WorldBossCfg::hurt_goldaward;
}

// 回滚前一个boss
void WorldBossNew::rollBackBeforeBoss()
{
    WorldBossCfgDef *def = WorldBossCfg::getBossCfg(getTpltID());
    if( def && def->beforeBoss != 0)
    {
        doRedisCmd("set worldboss %d",def->beforeBoss);
    }
}