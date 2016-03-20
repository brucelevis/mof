#pragma once
#include "RedisHash.h"
#include "RedisDhash.h"
#include "RedisKeyName.h"
#include "msg.h"
#include "GuildLog.h"
#include "GuildChat.h"
#include <string>
using namespace std;
class SkillIncrBattle;
class BaseProp;
class BattleProp;

class Guild : public GuildProp //公会基础属性
{
public:
    typedef RedisDhash<int, int, rnGuildMember> GuildMember;
    
    Guild(){}
    ~Guild(){}
    //加载
    Guild(int guildid)
    {
        GuildProp::load(guildid);
        mMembers.load(guildid);
        mGuildLog.load(guildid);
        mGuildChat.load(guildid);
        cleanNotExistMember();
        calRankAndPosition();
    }
    
    //心跳
    void update()
    {
        GuildProp::save();
        mGuildLog.update();
        mGuildChat.update();
    }
    
    //回收
    void destroy()
    {
        mGuildLog.destroy();
        mGuildChat.destroy();
        
        mMembers.clear();
        GuildProp::deleteFromDatabase();
    }
    
    
    bool isNull(){ return getCreateDate() == 0;}
    GuildMember& getMembers()   {   return mMembers;    }
    
    void calRankAndPosition();
    
    void sendMemberList(int roleid, req_guild_member_list& req) const;
    void sendGuildInfo(int roleid);
    void sendGuildInfoAllMember();
    void sendAllMemberPacket(INetPacket* packet);
    
    int getLevel();
    int getMaxMemberSize();
    int getBossLevel();
    
    // 增加公会boss exp
    void addBossExp(int exp);
    
    //增加公会财富贡献
    void addFortuneConstruction(int fort,int cons , const char* rolename , const char* des);

    //公会日志
    void appendLog(const string& log,int level = eGuildLogLvl_D);
    void sendLog(int roleid,int start ,int num);
    
    //聊天记录
    void appendChatMsg(GuildChatMsg& msg);
    void sendChatMsg(int roleid,int start,int num);
    
    //公会改名
    bool guildRename(string newname);

	// 计算职位的成员人数
	int getMembersPositionCount(GuildMemberPosition ePosition);
	
	// 检查某职位人数是否已满
	bool checkPositionIsFull(GuildMemberPosition ePosition);
	
	// 获取成员排名
	int getRank(int roleId);
    
    VoiceInfoItem getVoiceInfo(int64_t voiceId);
protected:
    // 清掉公会里面被删的角色
    void cleanNotExistMember();
    
protected:
    GuildMember mMembers;   //成员
    GuildLog    mGuildLog;  //日志
    GuildChat   mGuildChat; //公会聊天记录
    vector<int> mRank;      //排名
    
};

typedef Guild* GuildPointer;

// 纹章对战斗力的加成
void calHeraldryPropertyAddition(int roleid , BaseProp& base , BattleProp& bat );

// 纹章对精力上限的提高
int calHeraldryFatAddition(int roleid);

// 公会被动技能对本角色战斗力的加成
void calGuildSkillPropertyAdditionForSelf(int roleid , BaseProp& base , BattleProp& bat);

// 公会被动技能对宠物战斗力的加成
void calGuildSkillPropertyAdditionForPet(int roleid , SkillIncrBattle& out);




//#define GUILD_TEST 1 //KCDO正式服改掉

#ifdef GUILD_TEST

static const int GUILD_APPLY_EXPIRED_SECOND = 1800;
static const int GUILD_LOG_EXPIRED_SECOND = 1800;
static const int GUILD_DISSMISS_SECOND = 60;
static const int GUILD_UPDATE_SECOND = 10;
static const int GUILD_POSITION_UPDATE_SECOND = 60;
static const int GUILD_CHAT_MSG = 10;
static const int GUILD_WAIT_FOR_APPLY_SECOND  = 30;

#else

static const int GUILD_APPLY_EXPIRED_SECOND = 1 * 24 * 3600;       //申请过期 1天
static const int GUILD_LOG_EXPIRED_SECOND = 3 * 24 * 3600;         //日志过期 3天
static const int GUILD_DISSMISS_SECOND = 2 * 24 * 3600;            //解散缓冲 2天
static const int GUILD_UPDATE_SECOND = 5 * 60;                     //公会刷新 5分钟
static const int GUILD_POSITION_UPDATE_SECOND = 3600;              //公会职位刷新 1小时
static const int GUILD_CHAT_MSG = 100;                             //公会聊天消息最多100条
static const int GUILD_WAIT_FOR_APPLY_SECOND  = 24 * 3600;         //离开公会后一定时间内无法再申请或创建公会

#endif




