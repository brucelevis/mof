//
//  Boss.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-9-27.
//
//

#include "Boss.h"
#include "Monster.h"
#include "Skill.h"
#include "mail_imp.h"
#include "GameLog.h"
#include "RoleMgr.h"
#include "Role.h"

static int genBossId()
{
    RedisResult result(redisCmd("incr gen_bossid"));
    return result.readInt();
}

Boss::Boss():
    VALID_HURT_SECOND(10),
    VALIE_HURT_HIT_MAGE(18),
    VALIE_HURT_HIT_ASSASSIN(28),
    VALID_HURT_HIT_WARRIOR(23),
    VALID_HURT_FACTOR(35)
{
    mBossId = genBossId();
    
    m_bBroadCastInOut = false;
    mSynHPTimer = 0;
    mRefreshRoundTime = 0;
    
    mElapsedSecond= 0;
    
    setState(eIDLE);
    
    m_className = BOSS_ROOM_NAME;
    ActRoomMGR.Reg(this);
}

Boss::~Boss()
{
}

int Boss::GetBossId()
{
    return mBossId;
}

void Boss::Init()
{
    setHp(getMaxHp()); // 满血
    setState(eIDLE);
    
    mRefreshValidHurt = 0;
    mValidHurt.clear();
    
    mElapsedSecond= 0;
}

void Boss::createBoss()
{
    mActivityState = kBossCreated;
    OnBossCreated();
    
    notify_boss_created notify;
    BroadCast(&notify);
}

void Boss::death()
{
    mActivityState = kBossKilled;
    OnBossKilled();
}

void Boss::beAtk(Role* role, req_attack& req )
{
    if (mActivityState != kBossCreated) {
        return;
    }
    
    int dmg=0;
    bool isCri=req.isCir;   // 暴击信任客服端，todo
    if (req.objtype == ePet){
        Pet* pet = role->mPetMgr->getActivePet();
        if (pet==NULL)
            return;
        if(isCri)
            dmg = calCriDamage(pet);
        else
            calDamage(pet, dmg);
    }
    else if(req.objtype == eMainRole){
        if(isCri)
            dmg = calCriDamage(role);
        else
            calDamage(role, dmg);
    }else{}
    
   if( ! VerifyDmgOk(role,dmg,req.elaps_seconds) )
       return;
    
    DamageInput(role,req.objtype , dmg);
}

void Boss::beSkill(Role* role, req_skill_hurt& req)
{
    if (mActivityState != kBossCreated) {
        return;
    }
    
    int skillID     = req.skillID;
    int effidx      = req.skillEffectIndex;
    bool isCriHurt  = req.isCriHurt;
    int dmg         = 0;
    SkillMod*  skmod = role->getSkillMod();
    switch( req.casterType )
    {
        case eMainRole:
            if (role->getSkillMod()->IsSkillLearn(skillID)) {
                dmg = skmod->calcDmg(skillID,effidx,isCriHurt,this,role);
            }
            else{
                dmg = 1;
            }
            break;
        case ePet:
            Pet* pet = role->mPetMgr->getActivePet();
            if (pet && pet->hasSkill(skillID))
                dmg=skmod->calcDmg(skillID,effidx,isCriHurt,this,pet);
            else
                dmg = 1; // 开挂了，给1
            break;
    }
    assert(dmg>=0);
    

    if( ! VerifyDmgOk(role , dmg, req.elaps_seconds) )
        return;
    
    DamageInput(role,req.casterType,  dmg);
}

void Boss::DamageInput(Role* src, int objtype ,int dmg)
{
       
    HurtMap::iterator iter = mHurts.find(src->getInstID());
    if(iter == mHurts.end())
    {
        BossHurt tmp;
        tmp.totalHurt.roleid = src->getInstID();
        tmp.totalHurt.rolename = src->getRolename();
        tmp.totalHurt.job = src->getJob();
        tmp.totalHurt.dmg = 0;
        
        
        tmp.totalRoundHurt = 0;
        tmp.currentRoundHurt = 0;
        
        mHurts[src->getInstID()] = tmp;
        iter = mHurts.find(src->getInstID());
    }
    
    if (dmg == 0) {
        return;
    }
    
    BossHurt& h = iter->second;
    h.totalHurt.dmg += dmg;

    
    if(h.currentRoundHurt < getRoundHurtMax() )
    {
        // 分段伤害有最大值
        h.currentRoundHurt += dmg;
        h.currentRoundHurt = min( getRoundHurtMax() , h.currentRoundHurt ) ;
    }
    
    InputRank(src->getInstID());
    
    //扣血
    setHp(getHp()-dmg);
    if (getHp() <=0){
        setHp(0);
        setState(eDEAD);
        mWhokill = src->getRolename();
        
        death();
        return;
    }
}

/*
 同步策略：mSynHPTimer 和 配置表 控制HP同步
 */
void Boss::OnUpdate(float dt)
{
    mElapsedSecond += dt;
    
    switch (mActivityState) {
        case kBossActivityOpen:
        {
            time_t nowTime = time(NULL);
            if (nowTime > mCreateTime) {
                createBoss();
            }
            break;
        }
        case kBossCreated:
        {
            mRefreshRoundTime += dt;
            RefreshRoundHurt();
            
            mSynHPTimer += dt;
            SyncBossHPAndRank();
            
            RefreshValidHurt(dt);
        }
        case kBossKilled:
        {
            break;
        }
            
        default:
            break;
    }
    
}

void Boss::OnEnterRoom(Obj* obj)
{    
    DamageInput( (Role*)obj,eMainRole,0);
}

static void writeCheatLogAndDisconnect(Role* src , float clientElapse)
{
    Xylog log(eLogName_BossCheat, src->getInstID());
    log<<clientElapse;
//    log<<src->getInstID();
//    log<<src->getRolename();
//    log<<SSessionDataMgr.getProp(src->getSessionId(), "account");
//    log<<SSessionDataMgr.getProp(src->getSessionId(), "platform");
//    log<<src->getVipLvl();
    
    //SRoleMgr.disconnect(src,eClientCheat);
    
    
}

bool Boss::VerifyDmgOk(Role* src  ,int dmg, float clientElapse)
{
    int maxDmg = calBasicDamage(src);
    maxDmg *= VALID_HURT_FACTOR;
    
    mValidHurt[src->getInstID()] += dmg;
    std::map<int,int>::iterator iter = mValidHurt.find(src->getInstID());
    
    if( iter->second > maxDmg )
    {
        iter->second -= dmg;
        writeCheatLogAndDisconnect(src,clientElapse);
        return false;
    }
    return true;
}

void Boss::RefreshValidHurt(float dt)
{
    mRefreshValidHurt += dt;
    if( mRefreshValidHurt < VALID_HURT_SECOND)
        return;
    mRefreshValidHurt = 0;
    mValidHurt.clear();
}


void Boss::RefreshRoundHurt()
{
    if (mRefreshRoundTime < WorldBossCfg::round_refresh_seconds)
        return;
    
    for (HurtMap::iterator it = mHurts.begin(); it != mHurts.end(); ++it)
    {
        it->second.totalRoundHurt = hurtConvertAwardPoint(it->first);
        it->second.currentRoundHurt = 0;
    }
    
    mRefreshRoundTime = 0;
}

void Boss::SyncBossHPAndRank()
{
    if (mSynHPTimer < WorldBossCfg::syn_seconds) {
        return;
    }
    
    syn_hurtboss_rank syn;
    syn.total_hurts = getMaxHp() - getHp();
    syn.hurts = mRank;
    
    float batpointFactor = hurtAwardBattlePointFactor();
    float goldFactor = hurtAwardGoldFactor();
    for (size_t i = 0; i < m_objs.size(); i++) {
        if (m_objs[i].first && m_objs[i].first->getType() == eRole) {
            Role* role = (Role*)m_objs[i].first;
            HurtMapIter iter = mHurts.find(role->getInstID());
            if (iter == mHurts.end()) {
                syn.myhurts = 0;
                syn.hurtAwardGold =  0;
                syn.hurtAwardBatpoint = 0;
            } else {
                syn.myhurts = iter->second.totalHurt.dmg;
                int awardPoint = hurtConvertAwardPoint(role->getInstID());
                syn.hurtAwardGold = awardPoint * goldFactor ;
                syn.hurtAwardBatpoint = awardPoint * batpointFactor;
            }
            sendNetPacket(role->getSessionId(), &syn);
        }
    }
    
    mSynHPTimer = 0;
}

int Boss::getClientRankNum()
{
    return 1;
}

void Boss::InputRank(int lastRoleid)
{
    obj_hurts last_input = mHurts[lastRoleid].totalHurt;
    assert(strcmp(last_input.rolename.c_str(),"")!=0);
    // rank 和
    int sz = mRank.size();
    if (sz>0) {
        bool inRank=false;
        for (int i = 0; i<sz; i++) {//0判断已在榜，1替换dmg，2排序
            if (mRank[i].roleid == lastRoleid) {
                mRank[i].dmg = last_input.dmg;
                ResortRank();
                inRank = true;
                break;
            }
        }
        
        if (!inRank) {//不在榜，
            if(last_input.dmg > mRank[sz-1].dmg)// 和最后一名比
            {
                mRank[sz-1] = last_input;
                ResortRank();
            }else if (sz < getClientRankNum() ) {// 排行榜未满
                mRank.push_back(last_input);
            }
        }
        
    }else{ // 第一个上榜
        mRank.push_back(last_input);
    }
}

void Boss::ResortRank()
{
    sort(mRank.begin(), mRank.end(), sortRank);
}


void Boss::SendRoleBattleInfo(int roleid, int reqRole){
    Role * role = SRoleMgr.GetRole(roleid);
    //if(role == NULL)
    return;
    notify_role_battle_info batinfo;
    batinfo.typeID = role->getRoleType();
    batinfo.name = role->getRolename();
    batinfo.lvl = role->getLvl();
    batinfo.skills = role->getSkillMod()->getStudySkillList();
    batinfo.maxhp = role->mMaxHp; batinfo.atk = role->mAtk;
    batinfo.def = role->mDef; batinfo.hit = role->mHit;
    batinfo.dodge = role->mDodge; batinfo.cri = role->mCri;
    Pet* pet = role->mPetMgr->getActivePet();
    obj_petinfo info;
    if (pet)
    {        
        pet->getPetInfo( info );
        
    }
    else{
        info.petid = 0;}
    batinfo.petinfo = info;
    sendNetPacket(role->getSessionId(), &batinfo);
}

int Boss::getRoundHurtMax()
{
    return 0;
}

float Boss::hurtAwardBattlePointFactor()//伤害奖励战功
{
    return 0.0f;
}
float Boss::hurtAwardGoldFactor()//伤害奖励金币
{
    return 0.0f;
}
int Boss::hurtConvertAwardPoint(int roleid)
{
    HurtMap::iterator iter = mHurts.find(roleid);
    if(iter == mHurts.end())
        return 0;
    return iter->second.totalRoundHurt + sqrt( iter->second.currentRoundHurt );
}
