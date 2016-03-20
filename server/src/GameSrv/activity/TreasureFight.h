//
//  TreasureFight.h
//  GameSrv
//
//  Created by pireszhi on 14-5-15.
//
//

#ifndef __GameSrv__TreasureFight__
#define __GameSrv__TreasureFight__

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include "DataCfgActivity.h"
#include "EnumDef.h"
#include "GuildRole.h"
#include "GuildMgr.h"

class Role;

using namespace std;

typedef set<int> TreasureGuildMember;

struct GuildTreasureFightResult
{
    GuildTreasureFightResult(int owner, int ownerlvl):ownerid(owner),
                                        ownerName(""),
                                        ownerLvl(ownerlvl),
                                        points(0),
                                        monsterKilled(0),
                                        assists(0),
                                        enemyKilled(0),
                                        hasGetAward(true)
    {
        
    }
    int ownerid;
    string ownerName;
    int ownerLvl;
    int enemyKilled;
    int monsterKilled;
    int assists;
    int points;
    bool hasGetAward;
    
    //不存储
    
};

struct TreasureCopyItem
{
    TreasureCopyItem(): maxGuildMem(0),
                        maxPlayerCount(0),
                        copyType(eTreasureNormal),
                        sceneMod(0),
                        sceneid(0),
                        copyNum(0),
                        sceneName("")
    {
        playerResult.clear();
    }
    
    int enterTreasureCopy(Role* role, GuildTreasureFightResult* roleResult);
    
    void leaveTreasureCopy(Role* role);
    
    void getTreasurecopyInfo(obj_treasure_scene& obj, int guildid);
    
    void getPlayers(vector<int>& outdata);
    
    void update()
    {
        //定时发送一些战斗结果数据给前端
    }
    
    int copyNum;             //副本编号
    int sceneid;             //实体id
    int sceneMod;            //模板id
    int maxPlayerCount;     //可容纳的最大玩家数
    int copyType;           //副本类型，是否双倍积分之类的
    int maxGuildMem;        //最大的公会成员数
    string sceneName;
    
    int isOpen;             //是否开启
    map<int, TreasureGuildMember> groups;   //公会id， 同公会的成员
    vector<GuildTreasureFightResult*> playerResult;
};

class TreasureCopyMgr
{
public:
    
    TreasureCopyItem* createTreasureCopy(int copyNum, int sceneid);
    
    bool closeAllTreasureCopy();
    
    void onCloseTreasureCopy(int copyid);
    
    vector<TreasureCopyItem*>& getValidTreasureCopyList();
    
    TreasureCopyItem* getTreasureCopy(int sceneid);
    
    int getCopyCount();
    
    void update();
    
private:    
    vector<TreasureCopyItem*> mTreasureCopys;
};

typedef vector<GuildTreasureFightResult*> TreasureFightResultArray;

struct ManorDataStruct
{
    ManorDataStruct(): guildId(0),
                       points(0),
                       position(0),
                       guildName("")
    {
        
    }
    
    int guildId;
    int points;
    int position;
    string guildName;
};

enum TreasureCopyState
{
    eTreasureInTown= 0,
    
    eTreasureInActivity,
    
    eTreasureIncopy
};

struct TreasurecopyPlayerState
{
    TreasurecopyPlayerState():  sessionid(0),
                                state(eTreasureInTown),
                                copyid(0)
    {
        
    }
    int sessionid;
    int copyid;
    TreasureCopyState state;
};

class GuildTreasureFightMgr
{
public:
    
    GuildTreasureFightMgr():mRunning(false),
                            mOpenTime(0),
                            mPlayerCountIncopy(0),
                            mNewCopyCountRemain(0)
    {
        
    }
    
    //初始化
    void guildTreasureFightInit();
    
    void update();
    
    //加载数据from Db
    void loadDataFromDB();
    
    //活动开始
    void activityOpen(int opentime);
    
    //活动结束, 完成一些活动结束时的东西
    void activityClose();
    
    //角色进入活动界面
    bool enterActivity(Role* role);
    
    //判断并计算需要创建新副本的数量
    bool checkIncreaseCopy();
    
    //创建副本
    bool onCreateTreasureCopy(int copynum, int sceneid);
    
    //角色离开活动界面
    bool leaveActivity(Role* role);
    
    //判断并计算需要关闭的副本的数量
    bool checkReduceCopy();
    
    //玩家进入副本
    int enterTreasureCopy(Role* role, int sceneid);
    
    //快速进入副本
    TreasureCopyItem* quickEnterTreasureCopy(Role* role);
    
    //玩家离开副本
    int leaveTreasureCopy(Role* role, int sceneid, bool isDeath);
    
    vector<TreasureCopyItem*>& getTreasureCopyList();
    
    int getTreasureCopyList(int beginIndex, int endIndex, int playerGuildid, vector<obj_treasure_scene>& outdata);
    
    int getActivityDuration();
    
    //玩家断线后的操作
    void playerDisconnect(Role* role);
    
    //记录玩家离开副本的时间
    void setPlayerRecoverSec(int roleid, int recoverSec);
    
    //检查玩家是否已苏醒
    int getPlayerRecoverSec(int roleid);
    
    void sortGuildResult(int weekday);
    
    TreasureCopyItem* getTreasurecopy(int sceneid);
    
    void clientGetGuildResult(int guildid, int& guildpoints, int& guildRank, int weekday = -1);
    
    GuildTreasureFightResult* getPlayerFightResult(int roleid, int weekday = -1);
    
    bool addPlayerFightPoints(int roleid, int addPoints, int addEnemysKilled, int addMonstersKilled, int addAssists);
    
    bool playerGetMyGuildAward(int weekday, int roleid);
    
    bool closeAllTreasureCopy();
    
    bool onCloseTreasureCopy(int sceneid);
    
    void sendFightingReportToPlayer(int roleid, notify_treasurecopy_activity_result& report);
    
    void makeFightingReportOfGuilds(notify_treasurecopy_activity_result& report);
    
    void makeFightingReportOfPlayer(Role* role, notify_treasurecopy_activity_result& report);
    
    TreasurecopyPlayerState getPlayerState(Role* role);
    
    void sendGuildWealthAward(int weekday);
    
    void playerLeaveGuild(Role* role);
    
    /****
     占领相关
     ****/
    
    void clientGetManors(int roleid, ack_treasurecopy_get_manors& ack);
    
    int getMyGuildManorAward(int weekday, Role* role, string &award);
    
    string getPlayerAward(GuildTreasureFightResult* playerResult);
    
    void sendPlayerAwardMail(int weekday);
    
    void clearPlayerDataInDB(int roleid, int guildid);
    
	void checkGuildRankHonor(int weekday);
	
	bool checkTreasureFightResulPlayer(int weekday, Role* role);
	bool checkTreasureFightResulGuild(vector<ManorDataStruct> &val, int guildId, int &rank);
	
	// 获取占据得分
	int getOccupyPoints(Role* role);
protected:
    void clearGuildPoints(int weekday);
    
    void clearPlayerPoints(int weekday);
    
    void loadPlayerPoints(int weekday);
    
    void loadGuildPoints(int weekday);
    
    void loadManors(int weekday);
    
    void updateManors(int weekday);
    
    //设置活动的开始时间
    void setOpenTime(int opentime);
    
    //通知前端发生变化的额副本信息
    void notifyTreasureCopyChange(TreasureCopyItem* copy);
    
    //通知前端有新增加的副本
    void notifyTreasureNewCopyAdd(TreasureCopyItem* newcopy);
    
    GuildTreasureFightResult* createPlayerResult(int weekday, int roleid, string rolename, int rolelvl);
    
    void sortPlayersResult();
    
    GuildTreasureFightResult* getGuildFightResult(int weekday, int guildid);
    
    GuildTreasureFightResult* createGuildResult(int weekday, int guildid);
    
    bool addGuildFightPoints(Guild& guild , int addPoints);
    
    void savePlayerResult(GuildTreasureFightResult* playerresult, int weekday);
    
    int setPlayerState(Role* role, TreasureCopyState state, int sceneid);
    
    void createPlayerState(Role* role, TreasureCopyState state);
    
    void erasePlayerState(Role* role);
public:
    
    
private:
    
    TreasureCopyMgr mTreasureCopyMgr;           //副本管理器
    
    int mPlayerCountIncopy;                     //统计在副本里的人数
    
    map<int, TreasurecopyPlayerState> mPlayerState;
    
    bool mRunning;                              //活动进行中
    
    time_t mOpenTime;                           //活动开始的时间
    
    int mNewCopyCountRemain;                    //剩余需要创建的新副本，为非是不允许发送创建副本命令
    
    map<int, int> mPlayerRecoverCountdown;      //玩家的苏醒时间
    
    map<int, TreasureFightResultArray> mPlayerFightResult;   //玩家的战斗结果，战斗情况
    
    map<int, TreasureFightResultArray> mGuildFightResult;    //公会的积分排行
    
    string mDbKeyPrefix;                         //数据存档的key的前缀
    
    map<int, vector<ManorDataStruct> > mGuildsManor;
};

extern GuildTreasureFightMgr g_GuildTreasureFightMgr;

extern void guildTreasureFighActOpen(int actId, int number, string& params);

extern void guildTreasureFighActClose(int actId, int number, string& params);
extern string awardToMailContentExplanation(string awardStr);


#endif /* defined(__GameSrv__TreasureFight__) */
