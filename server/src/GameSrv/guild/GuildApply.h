#pragma once
#include <string>
#include <map>
#include "msg.h"
#include "RedisDhash.h"
#include "RedisKeyName.h"
#include "GuildShare.h"

using namespace std;

class Role;

class GuildApplicant
{
public:
    GuildApplicant();
    ~GuildApplicant(){}
    int     roleid;
    int     date;
    int     viplvl;
    int     job;
    int     level;
    int     battle;
    string  rolename;
 
    //加新成员要全局搜GuildApplicant确定是否拷齐信息
    
    void load(const char* str);
    string str() const;
};


class GuildApplyMgr
{
public:
    typedef RedisDhash<int,int, rnRoleApplyGuildList>       Apply;    //某玩家申请列表
    typedef RedisDhash<int,GuildApplicant,rnGuildApplicant> Applicant;//某公会被申请列表

    void on_req_role_apply_guild_list(Role* role);
    
    void on_req_guild_applicant_list(Role* role,req_guild_applicant_list& req);
    
    void on_req_apply_guild(Role* role,req_apply_guild& req);
	
	// 取消公会申请
	void on_req_ancel_apply_guild(Role* role, req_cancel_apply_guild &req);
    
    void clearApply(int roleid);            //玩家创建或者加入公会后，把申请列表清了
    
    void clearApplicant(int guildid);       //公会解散后，把被申请列表清了
    
    void clearExpiredApplication();

    GuildApplyMgr(){}
    
    ~GuildApplyMgr(){}
    
	// 删除申请列表 (申请角色id, 申请公会id)
    bool remove(int roleid , int guilid);
    
protected:
    
    eApplyGuild insert(int roleid , int guildid);
    
    Apply& getApply(int roleid);
    
    Applicant& getApplicant(int guildid);
    
protected:
    
    typedef map<int,Apply>        UserMap;
    typedef map<int,Applicant>    GuildMap;
    
    UserMap       mUserMap;
    GuildMap      mGuildMap;
    Apply         mDummyApply;
    Applicant     mDummyApplicant;
};

extern GuildApplyMgr SGuildApplyMgr;
