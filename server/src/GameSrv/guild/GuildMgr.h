#pragma once
#include "Guild.h"
#include "GuildShare.h"
#include "RedisDhash.h"
#include "msg.h"
#include <map>
#include <vector>
#include <string>
using namespace std;

class Role;

class GuildMgr
{
    typedef RedisDhash<int, int, rnGuildAllId> GuildAllId;
public:
    GuildMgr();
    ~GuildMgr();
    void init();
    void release();

    Guild& getGuild(const char* gname);
    
    Guild& getGuild(int guildid);
    
    Guild& getRoleGuild(int roleid);
    // 检查玩家权力
    bool roleHasRight(int roleid, int right);
    
    void update();
    
    void updateGuildMemberPosition();
    
    void spanDayRefresh();
    void freshGuild(Guild& guild);
    
    void doImpeachMaster(Guild& guild);

    
    int guildRename(int roleid, string newname);
    
    const vector<int>& GetAllGuild(){return mGuildRank;}
        
public:
    
    void on_req_guild_list(Role* role , req_guild_list& req);
    void on_req_create_guild(Role* role, req_create_guild& req);
    
    void on_req_dismiss_guild(Role* role,req_dismiss_guild& req);
    void on_req_cancle_dismiss_guild(Role* role, req_cancle_dismiss_guild& req);
    
    void on_req_search_guild(Role* role, req_search_guild& req);
    
    void on_req_transfer_guild_master(Role* role, req_transfer_guild_master& req);
	void on_req_cancle_transfer_master(Role* role, req_cancle_transfer_master& req);
    
    void on_req_guild_info(Role* role , req_guild_info& req);
    void on_req_guild_member_list(Role* role, req_guild_member_list& req);
    
    void on_req_player_guild_detail(Role* role);
    
    void on_req_accept_apply_guild(Role* role,req_accept_apply_guild& req);
    void on_req_reject_apply_guild(Role* role,req_reject_apply_guild& req);
    
    void on_req_guild_leave(int roleid);
    void on_req_guild_kick(Role* role,req_guild_kick& req);
    
    void on_req_modify_guild_notice(Role* role, req_modify_guild_notice& req);
    
    void on_req_guild_donate(Role* role,req_guild_donate& req);
    
	void on_req_guild_store(Role* role, req_guild_store& req);
	
	void on_req_guild_storeUpdate(Role* role, req_guild_storeUpdate& req);
	
	void on_req_guild_storeBuy(Role* role, req_guild_storeBuy& req);
	
    void on_req_upgrade_guild_skill(Role* role, req_upgrade_guild_skill& req);
    
    void on_req_guild_chat(Role* role,req_guild_chat& req);
    
    void on_req_past_guild_chat(Role* role, req_past_guild_chat& req);
    
    void on_req_guild_boss_exp(Role* role);
    
    void on_req_train_guild_boss(Role* role,req_train_guild_boss& req);
    
    void on_req_my_guild_rank(Role* role);
    
    void on_req_guild_bless(Role* role, req_guild_bless& req);
    
    void on_req_guild_inspire(Role* role, req_guild_inspire& req);
    
    void on_req_guild_impeach_info(Role *role);
    
    void on_req_guild_impeach(Role *role);
	
	// 指派职位
	void on_req_guild_appoint_position(Role *role, req_guild_appoint_position& req);

    void gmSetLvl(int roleID, int i_Lvl);
    
    void safe_gmSetLvl(int roleID,int i_Lvl);
    
    void gmSetForturn(int roleID, int i_Forturn);
    
    void safe_gmSetFortune(int roleID,int i_Forturn);
    
    void gmSetDcons(int roleID, int i_Dcons);
    
    void safe_gmSetDcons(int roleID,int i_Dcons);
    

protected:
    bool checkCreateGuildConditionOk(Role* role,const string& guildname);
    
    int createNewGuild(int master , const string& guildname);
    
    bool roleLeaveGuild(int guildid , int roleid);

    void buildGuildNameIndex();
    
    void buildGuildRank();
    
    void dissmissExpiredGuild();
	void dissmissExpiredTransferMaster();
	void transferMaster(int guildId);
    
    void kickAllMember(int guildid);
    
    void destroyGuild(int guildid);
    
    void saveAllGuildProp();
    
protected:
    typedef vector<int>         GuildRankVec;
    typedef map<string, int>    GuildNameMap;
    typedef map<int,Guild>      GuildMap;
    
    GuildAllId                  mGuildAllId;            //全部公会id
    
    GuildMap                    mGuilds;                //公会
    
    GuildRankVec                mGuildRank;             //公会排名
    
    GuildNameMap                mNameIndex;             //名字索引
    
    Guild                       *mDummyGuild;            //无效公会
};

extern GuildMgr SGuildMgr;
#define SGuild(guildid)  SGuildMgr.getGuild((guildid))

//发送弹劾失败邮件
void sendImpeachKickbackMail(const char *receiver, const int &rmb, const char *mailtype);
//当会长上线后，重置弹劾信息
void resetImpeachInfoWhenMasterOnLine(int roleid, const char *mail_type);
//退出公会的时候刚好自己又申请了弹劾
void resetImpeachApplicantIf(Guild &guild, int roleid, const char *mail_type);


