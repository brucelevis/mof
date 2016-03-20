#include "GuildRole.h"
#include "DBRolePropName.h"
#include "RedisResultHelper.h"
#include "GuildShare.h"
#include "Role.h"
#include "GameLog.h"
#include "GuildMgr.h"
#include "Honor.h"

RoleGuildProperty::RoleGuildProperty():
    mViplvl(0),mJob(0),mLevel(0),mLastActive(0),
    mBattleForce(0),mRoleType(0)
{
}

int  RoleGuildProperty::getViplvl()
{
    Role* role = SRoleMgr.GetRole( getId() );
    if(role){
        return role->getVipLvl();
    }
    return mViplvl;
}

int  RoleGuildProperty::getLevel()
{
    Role* role = SRoleMgr.GetRole( getId() );
    if(role){
        return role->getLvl();
    }
    return mLevel;
}

int  RoleGuildProperty::getLastActive()
{
    Role* role = SRoleMgr.GetRole( getId() );
    if(role){
        return 0;
    }
    return mLastActive;
}

int  RoleGuildProperty::getBattleForce()
{
    Role* role = SRoleMgr.GetRole( getId() );
    if(role){
        mBattleForce = role->getBattleForce();
    }
    return mBattleForce;
}

bool RoleGuildProperty::addConstrib(float val,const char* des)
{
	if( isFloatZero(val))
		return true;
	
    if(val < 0 && getConstrib() + val < 0)
    {
        return false;
    }
    
    Xylog log(eLogName_GuildConstribute,getId());
    log << getGuild() << getConstrib();
	
    setConstrib( getConstrib() + val);
	
    log << getConstrib() << val << des;
    
	syn_player_guild_constrib nty;
	nty.constrib = getConstrib();
	sendRolePacket(getId(), &nty);
	
	// 称号检查
	SHonorMou.procHonor(eHonorKey_Constrib, getId());
    return true;
}

void RoleGuildProperty::addExploit(int val, const char* des)
{
    if(val == 0)
        return;
    
    Xylog log (eLogName_GuildExploit, getId());
    log << getGuild() << getExploit();
    
    setExploit( getExploit() + val);
    sendPlayerGuildDetail();
    
    log << getExploit() << val << des;
	
	// 称号检查
	SHonorMou.procHonor(eHonorKey_Exploit, getId());
}

void RoleGuildProperty::clearExploit(const char* des)
{
    Xylog log (eLogName_GuildExploit, getId());
    log << getGuild() << getExploit() ;
    
    setExploit( 0 );
    sendPlayerGuildDetail();
	
	log << getExploit() << 0 << des;
}

void RoleGuildProperty::sendPlayerGuildDetail()
{
    Role* role = SRoleMgr.GetRole( getId() );
    xycheck(role);
    
    notify_player_guild_detail nty;
    nty.guildid = getGuild();
    nty.guildname = SGuild(nty.guildid).getGuildName();
    
    for (RoleGuildSkill::Iterator it = getSkill().begin();
         it != getSkill().end(); ++it)
    {
        obj_guild_skill obj;
        obj.skillid = it->first;
        obj.level = it->second;
        nty.skills.push_back(obj);
    }
    nty.constrib = getConstrib();
    nty.exploit = getExploit();
    nty.dnoateGlodBatNum = role->getGuildDnoateNum();
    nty.position = getPosition();
    nty.itemTrainBossTime = role->getGuildItemTrainBossTime();
    nty.rmbTrainBossTime = role->getGuildRmbTrainBossTime();
    nty.blessmate = role->getGuildBlessMate().toArray();
    nty.blessed = role->getGuildBlessed();
	nty.guildRank = SGuild(nty.guildid).getRank(role->getInstID());
    
    sendNetPacket(role->getSessionId(),&nty);
}

//外部保证roleid存在
RoleGuildProperty::RoleGuildProperty(int roleid)
{
    RoleGuildProp::load(roleid);
    
    Role* role = SRoleMgr.GetRole(roleid);
    if(role){
        mRoleName = role->getRolename();
        mViplvl = role->getVipLvl();
        mJob = role->getJob();
        mLevel = role->getLvl();
        mRoleType = role->getRoleType();
        mLastActive = 0;
        return;
    }
    
    
    const char* lastlogin = "lastlogin";
    RedisHMGET result("HMGET role:%d %s %s %s %s %s %s",
                      roleid,
                      GetRolePropName(eRolePropRoleName),
                      GetRolePropName(eRolePropVipLvl),
                      GetRolePropName(eRolePropRoleType),
                      GetRolePropName(eRolePropLvl),
                      lastlogin,
                      GetRolePropName(eRoleBattleForce)
                      );
    
    mRoleName = result.readString( GetRolePropName(eRolePropRoleName) );
    mViplvl = result.readInt( GetRolePropName(eRolePropVipLvl) );
    mRoleType = result.readInt( GetRolePropName(eRolePropRoleType) ) ;
    mJob = RoleType2Job(mRoleType);
    mLevel = result.readInt( GetRolePropName(eRolePropLvl) ) ;
    mLastActive = result.readInt( lastlogin ) ;
    mBattleForce = result.readInt( GetRolePropName(eRoleBattleForce) ) ;
	
	// 修正非法技能的逻辑处理　04-23 by jin
	getSkill().correctIllegalSkill();
}

void RoleGuildProperty::getGuildSkill(vector<int>& skills)
{
    for (RoleGuildSkill::Iterator itSkill = getSkill().begin();
         itSkill != getSkill().end(); ++itSkill)
    {
        int skillFullId = itSkill->first + itSkill->second;//400800 + 1 -> 400801才是id
        skills.push_back(skillFullId);
    }
}

#pragma mark -
#pragma mark RoleGuildMgr implement


RoleGuildMgr SRoleGuildMgr;

RoleGuildProperty& RoleGuildMgr::getRoleGuild(int roleid)
{
    RoleGuildPropMap::iterator iter = mRoleGuildProps.find(roleid);
    if(iter != mRoleGuildProps.end() )
        return iter->second;
    
    if(! isRoleExistInDbForMainThread(roleid))
        return mDummy;
    
    mRoleGuildProps.insert(make_pair(roleid, RoleGuildProperty(roleid)));
    return mRoleGuildProps[roleid];
}


void RoleGuildMgr::saveAllRoleGuildProperty()
{
    RoleGuildPropMap::iterator iter;
    for (iter = mRoleGuildProps.begin(); iter != mRoleGuildProps.end(); ++iter)
    {
        iter->second.save();
    }
}

void RoleGuildMgr::offline(int roleid)
{
    RoleGuildProperty& rguild = getRoleGuild(roleid);
    Role* role = SRoleMgr.GetRole(roleid);
    if(role){
        rguild.setViplvl(role->getVipLvl());
        rguild.setLevel(role->getLvl());
        rguild.setLastActive(time(0));
    }
    rguild.save();
}




