//
//  RealPvpMgr.h
//  GameSrv
//
//  Created by pireszhi on 14-3-21.
//
//

#ifndef __GameSrv__RealPvpMgr__
#define __GameSrv__RealPvpMgr__

#include <iostream>
#include <stdio.h>
#include <vector>
#include <map>
#include <list>
#include "RedisHash.h"
#include "Utils.h"
#include "cyclemsg.h"
#include "Role.h"
#define SYNPVP_MAP 501227 
struct MatchModel
{
    MatchModel(int roleid, int battleForce): roleid(roleid),
                                                battleForce(battleForce),
                                                lvl(0),
                                                roleName(""),
                                                enterTime(0),
                                                ranking(0),
                                                inCombat(false),
                                                todayTimes(0),
                                                inMatching(false)
    {
        playdata.load(roleid);
    }
    
    void getData(obj_realpvp_info& info)
    {
        info.medal = playdata.getMedal();
        info.honour = playdata.getHonour();
        info.winningstreak = playdata.getWinningStreak();
        info.winningtimes = playdata.getWinningTimes();
        info.losingtimes = playdata.getLosingTimes();
        info.exittimes = playdata.getExitTimes();
        info.recentgains = playdata.getRecentGains();
        
        info.roleLvl = lvl;
        info.rolename = roleName;
        info.roleType = roleType;
        info.battleForce = battleForce;
    }
    
    int roleid;                 //角色id
    int ranking;                //
    int battleForce;                 //战斗力
    int lvl;
    int roleType;
    string roleName;
    int enterTime;              //开始匹配的时间
    bool inCombat;              //是否在战斗中
    bool inMatching;            //是否处于等待匹配状态
    int todayTimes;           //今天已参加匹配的次数
    RealPvpData playdata;       //角色在同步PVP中的信息
};

typedef pair<int, int> CompetitionMember;

struct CompetitionData
{
    CompetitionData()
    {
        matchTime = 0;
    }
    
    int matchTime;
    
    vector<CompetitionMember> members;
};

/*******************************************************************************/

class MatchingWorker
{
public:
    
    bool receiveModel(MatchModel* model);
    
    bool matching();
    
    bool checkMatchCondition(MatchModel* firstModel, MatchModel* secondModel);
    
    bool checkContinuousEnemy(MatchModel* model, int enemyid);
    
    vector<CompetitionMember>& getCompetitors(){return mCompetitors;}
    
    void clearCompetitors()
    {
        mCompetitors.clear();
    }
    
private:
    std::list<MatchModel*> mQueue;
    
    vector<CompetitionMember> mCompetitors;
};


class RealPvpMgr
{
public:
    void safe_getMyRealpvpInfo(Role* role);
    
    void safe_readyToRealpvpMatching(Role* role);
    
    void safe_cancelMatching(int roleid);
    
    void safe_medalExchange(int roleid, int itemIndex, int itemCount);
    
    void safe_addMedal(int roleid, int addMedal);
    
    void safe_receiveFightingResult(int winner, int loser);
    
    void safe_gmSetHons(int roleid, int setHons);
    
    void safe_gmSetWinningTimes(int roleid, int setWinningTimes);
    
    void safe_gmSetLostTimes(int roleid, int setLostTimes);
    
    void safe_gmSetMedal(int roleid, int setMeds);
    
    void safe_gmSetMilitary(int roleid, int Index);
    
    void safe_gmAddWins(int roleid, int addWins);

public:
    RealPvpMgr():mRunning(false)
    {
        mCompetitions.clear();
    }
    
    ~RealPvpMgr()
    {

    }
    
    void initialize();
    
    int getIndex();
    
    int getBeginTimes();
    
    MatchModel* CreateModel(int roleid, obj_realpvp_info& roleinfo);
    
    void clearPlayerData();
    
    void update(float dt);
    
    bool throwToMatchingWorker(int roleid, obj_realpvp_info& roleinfo, int todayTimes);
    
    bool cancelMatching(int roleid);
    
    bool getMyRealpvpInfo(int roleid, int matchingIndex, obj_realpvp_info roleData, obj_realpvp_info& outData);
    
    bool handleCompetition(vector<CompetitionMember> competitions);
    
    void delaySendBeginMsgToSynScene();
    
    void onSendBeginMsgToSynScene(vector<CompetitionMember>& members);
    
    void handleFightingResult(int winnerid, int loserid, SynPvpAward& winaward, SynPvpAward& loseaward);
    
    int medalExchange(int roleid, int itemIndex, int itemCount);
    
    void addMedal(int roleid, int medalCount);
    
    void gmAddWins(int roleid, int addWins);
    
    //void gmSetMedal(int roleid, int setMeds);
    void gmSetMilitary(int roleid, int setHons);
    
    void gmSetHonour(int roleid, int setHons);
    
    void gmSetMedal(int roleid, int setMeds);
    
    void gmSetWinningTimes(int roleid, int setWinTimes);
    
    void gmSetLostTimes(int roleid, int setLostTimes);
    
    void loadMgrDataFromDB();
    
    void saveMgrData();
    
    void sendRoleInfoChangeLog(int roleid, string roleName, int totalcount, int addcount, string paramType, string changeFrom);
    
    void resetPlayerState(int roleid);
    
    MatchModel* findModel(int roleid);
private:
    
    void open(int actNum, int beginTime);
    
    void close();
    
    static void activityOpen(int actId, int number, string& params);
    
    static void activityClose(int actId, int number, string& params);
    
private:
    
    bool mRunning;
    
    MatchingWorker mWorker;
    
    list<CompetitionData> mCompetitions;
    
    map<int, MatchModel*> mAllPlayerData;
    
    int mActivityIndex;    //用于区分不同的场次, 2.6之后没存数据库的必要
    int mBeginTime;        //开始时间

};

extern RealPvpMgr g_RealpvpMgr;
extern void addRealPvpMatchFailAward(MatchModel* model);

#endif /* defined(__GameSrv__RealPvpMgr__) */
