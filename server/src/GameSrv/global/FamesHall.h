//
//  FamesHall.h
//  GameSrv
//
//  Created by pireszhi on 13-11-22.
//
//

#ifndef __GameSrv__FamesHall__
#define __GameSrv__FamesHall__
#include "std_includes.h"
#include <iostream>
#include <vector>
#include "msg.h"
#include "Defines.h"
#include "RedisHashJson.h"

#define FAMESHALL_RANK_COUNT 100
using namespace std;

class Role;

struct FamesInfo
{
    FamesInfo()
    {
        personId = position = 0;
        roleType = 0;
        lvl = 0;
        name = "";
        enchantId = 0;
        enchantLvl = 0;
        awakeLvl = 0;
    }
    int personId;
    int position;
    int roleType;
    std::string name;
    int weaponQua;  //武器品质
    int lvl;
    int enchantId;  //附魔id
    int enchantLvl; //附魔等级
    int awakeLvl;
    
    FamesInfo operator=(const FamesInfo& info)
    {
        personId = info.personId;
        position = info.position;
        roleType = info.roleType;
        name = info.name;
        weaponQua = info.weaponQua;
        lvl = info.lvl;
        enchantId = info.enchantId;
        enchantLvl = info.enchantLvl;
        return *this;
    }
};

typedef vector<FamesInfo> FamesList;    //名人列表
typedef map<int, FamesList> AllJobFamesList;
struct FamesRankData
{
    int roleid;
    int roleType;
    int job;
    int points;
    string name;
    int lvl;
    int topLayerPassed;
    int rank;
    int weaponQua;
    int chantid;        //附魔id
    int chantLvl;       //附魔等级
    int awakeLvl;
};
struct FamesChallData
{
    FamesChallData()
    {
        startLayer = topLayer = currtLayer = currPoints = 0;
    }
    int topLayer;
    int currtLayer;
    int currPoints;
    int startLayer;
};
    
struct FamesHallPointFactor
{
    int roleBattleForce;
};

typedef vector<string> FamesHallLog;
    
struct FamesHallDailyLog
{
    time_t replaceTime;
    vector<FamesHallLog> logs;
};
    
typedef vector<FamesRankData*> FamesHallRank;
//名人活动管理器
class FameHallMgr
{
public:
    
    //给Game线程调用的一些接口
    void safe_getFamesHallFameList(Role* role);
    
    void safe_getFamesHallPlayerData(Role* role, int versionid);
    
    void safe_beginFamesHallBattle(Role* role, int checkLayer, int sceneid);
    
    void safe_battleHandle(Role* role, int result, int layer, int sceneid, FamesHallPointFactor factor);
    
    
public:     //以下是在Global线程调用的方法
    
    void onClientGetFamesHallFameList(int roleid, int sessionid);
    
    int onGetFameInfo(ObjJob job, vector<obj_fames_info>& outdata);
    
    // 新的获取fameinfo的方法，根据不同玩家取不同的fameinfo
    int onGetFameInfoNew(ObjJob job, int currentLayer, vector<obj_fames_info>& outdata);
    
    int onGetPlayerData(int roleid, int sessionid, int versionId);
    
    int onBeginBattle(int roleid, int checkLayer);
    
    int onBattleHandle(int roleid, ObjJob job, int result, int layer, FamesHallPointFactor factor);
    
    AllJobFamesList& gameServerGetAllJobFamesList(){return mFamesLists;}
    
public:         //重要是一些内部用的函数
    FameHallMgr();
    
    void release();
    
    //初始化
    void init();
    
    //加载名人列表
    void loadFamesList();
    // 从排名重新读取名人堂
    void loadFamesListByRank();
    
    //加载排行榜1
    void loadRankFromDB();
    
    //加载玩家的战斗信息，只在服务器启动功能初始化时用到
    void loadChallData();
    
    //加载名人替换日志，暂时还没决定实现与否
    void loadReplaceLog();
    
    //模块主循环，检测排行榜刷新，名人替换等
    void onHeartBeat();
    
    //设置下次替换时间点
    void setNextFreshFamesListTime();
    
    //设置下次重新从数据库加载排行榜的时间点
    void setNextFreshRankTime();
    
    //刷新名人列表，根据排行榜的顺序进行替换
    void freshFamesList();

    //设置名人的信息，如等级，名字等
    void setFameData(FamesInfo& fame, FamesRankData* playerdata);
    
    //检测刷新玩家挑战数据
    void checkChallDataFresh();
    
    //检测是否刷新名人列表
    void checkFamesListFresh();
    
    //检测是否刷新排行榜
    void checkRankFresh();
    
    //清除内存中得排行榜信息
    void clearRank();
    
    //清除数据库中排行榜的信息
    void clearRankInDB();
    
    //根据职业类型替换名人操作
    bool famesReplace(ObjJob job);
    //不再是替换的规则，而是根据排名重新分配，上面的接口不再用
    bool famesRefresh();
    
    //战斗结束，更新内存中玩家的信息并提交到数据库
    bool commitPlayerChallData(int roleid, FamesChallData newdata);
    
    //战斗结束，更新玩家在排行榜的数据
    bool updatePlayerPointsInRanking(int roleid, ObjJob job, int points);
    
    //检测活动是否开始
    bool CheckActiveOpen();
    
    //组装并保存名人列表
    bool saveFameHallList();
    
    //直接保存名人列表
    bool onSaveFameHallList(string list);
    
    //获取玩家在名人堂中得位置，如果不在则返回-1
    int  getPositionInHall(int roleid, ObjJob job);
    
    //加载当个名人信息，
    FamesInfo loadFameInfo(int roleid);
    
    //获取玩家的战斗数据
    FamesChallData getPlayerchallData(int roleid);
    
    void onGetRankData(ObjJob job, int beginRank, int endRank, vector<FamesRankData>& outData);
    
    void testRank();
    
    void testMyPoints(obj_roleinfo roleinfo, int sessionid);
    
    void setPlayerBattleIndex(int roleid);
    
    int getPlayerBattleIndex(int roleid);
    
private:
    map<int, FamesList> mFamesLists; //各个职业的名人名单，目前三个职业，下标同职业枚举值

    map<int, FamesHallRank> mRanks;   //各个职业的当前排名，目前三个职业，下标同职业枚举值
    
    //排行榜里面的数据
    std::map<int, FamesRankData*> mPlayerRankData;
private:
    //即时的挑战数据
    typedef RedisHashJson<int> PlayerDataMap;
    PlayerDataMap mPlayerDataMap;
    
    //替换日志
    vector<FamesHallDailyLog> mDailyLog;
    
    map<int, int> mPlayerBattleIndex;     //roleid, battleIndex
    
    int mTopLvlPassed;
    
    int mRunning;
    
    int mActiveId;
    
    int mNextFreshFamesListTime;
    
    int mNextFreshRankTime;
    
    int mFamesVersion;
    
    int mBattleIndex;       //新加,处理玩家在刷新期间在副本战斗时的情况，add by wangzhigang 2015-3-17
    
};

//换取名人堂信息的回调
typedef void (*GETFAMESHALLDATAFUNC)(const map<int, FamesList>& data);

//获取名人堂信息的接口
void getFamesHallData(GETFAMESHALLDATAFUNC cbfunc);

extern FameHallMgr g_fameHallMgr;
#endif /* defined(__GameSrv__FamesHall__) */
