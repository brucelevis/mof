//
//  Pvp.h
//  GameSrv
//
//  Created by pireszhi on 13-4-17.
//
//

#ifndef __GameSrv__Pvp__
#define __GameSrv__Pvp__

#include <iostream>
#include "std_includes.h"
#include "Role.h"

#define PVP_OPEN_LVL 11
#define PVP_RANK_LIST_NUM 6
#define PVP_LOG_COUNT 4
#define PVP_RANK_PART_ONE 200
#define PVP_SCENEID 501221
using namespace std;
struct PvpPetInfo
{
    int petid;
    int isactive;
    int pettypeid;
    int growth;
    int lvl;
    int exp;
    int batk;
    int bdef;
    int bhp;
    int bdogde;
    int bhit;
    vector<int> skills;
    int capa;
    int inte;
    int stre;
    int phy;
    int star;
    int starlvl;
};

enum PvpObjType
{
    ePvpObj_Role,
    ePvpObj_Robot,
};

struct PvpObjInfo
{
    PvpObjInfo():objId(0),objType(ePvpObj_Role)
    {
        
    }
    int objId;
    PvpObjType objType;
};

class PvpRoleState
{
public:
    PvpRoleState() : mChallengedBy(0), mRoleid(0), isChallenge(false), isBeChallenge(false)
    {
        
    }
    
    int64_t mRoleid;
    bool isChallenge;
    bool isBeChallenge;
    int64_t mChallengedBy;
};

struct PvpLogData
{
    PvpLogData()
    {
        mChallengeRoleId = mRank = mChangeType = 0;
        mIsWin = mIsBeChallege =false;
    }
    
    int64_t mChallengeRoleId;
    bool mIsBeChallege;
    bool mIsWin;
    int mRank;
    int mChangeType;
};

class PvpLog
{
public:
    PvpLog(int64_t challengeid, int rank, int changeType, bool iswin, bool isBechallenge);
    void updatePvpLog(int64_t challengeid, int rank, int changeType, bool iswin, bool isBechallenge);
    void getPvpLog( vector<struct PvpLogData* >& outputData );
    void setTodayAward(int rank);
    
    int getTodayAward();
private:
    PvpLogData mlogList[PVP_LOG_COUNT];
    int mIndex;
    int mAwardRank;
};

struct PvpRankOccupation
{
    PvpRankOccupation():occupBegin(0),
                        rank(0)
    {
        
    }
    time_t occupBegin;
    int rank;
};

class PvpMgr
{
public:
    int safe_GetRoleAwardRank(int roleid);
    
    void safe_DeleteRoleInpvp(int roleid, ObjectType type = kObjectRole);
    
public:
    PvpMgr();
    ~PvpMgr(){}
    bool loadDataFromDB();
    bool SaveDataInDB();
    bool addRoleToRank(int64_t roleid);
    bool changRolesRank(int64_t roleid, int64_t beChallengeRoleid);
    bool DeleteRoleInRank(int64_t roleid);
    
    bool pushRoleInPvp(int64_t roleid);
    int eraseRoleInPvp(int64_t roleid, bool myself = false);
    bool IsRealRanking(int checkRank, int64_t checkRoleid);
    bool IsRoleInPvp(int64_t roleid);
    bool IsInCoolingDown(int64_t roleid);
    int getRankingSize();
    int64_t getRoleIdByRank(int ranking);
    int getRankingByRoleId(int64_t roleid);
    
    void setAward(int rank);
    void SaveNewAwardRank();
    
    void updatePvpLog(int64_t roleid, int64_t challengeid, int changeType, bool iswin, bool isBechallenge);
    bool getPvpLog(int64_t roleid, vector<struct PvpLogData* >& outputData );
    
    void onHeartBeat(float dt);
    
    void setNextDistribTime();
    
    int safe_getNextDistribAwardTime();
    
    void setRolePvpState(int64_t roleId, int64_t targetId);
    void resetRolePvpState(int64_t roleId, int64_t targetId);
    
    bool isRoleChallenging(int64_t roleId);
    bool isRoleChallenged(int64_t roleId);
    
    int onGetPvpRealRank(int64_t roleid);
    
    void sendPvpDayAward();
private:
    
    void pvpRankSettle();
    
    void setNextRankSettleTime();
private:
    vector<int64_t>  mPvpRanking;
    map<int64_t, int> mRankingIndexs;
    list<int64_t>  mPvpRoles;
    
    map<int64_t, PvpRoleState> mPvpRoleStates;
    
    std::map<int64_t, struct PvpLog> mPvplog;
    std::map<int, bool> mAwardIndex;    //int:名次，bool:对应名次的位置发生改变，下次领奖的奖励有变化
    int mDistribAwardTime;
    
    //2.8新增排行结算时间
    int mRankSettleTime;
};

//int64_t makePvpId(int roleid, int objtype);
void getPvpObjTypeAndObjId(int64_t inId, int& outId, int& outType);
int calcPvpPointsAward(int rank);

#define SPvpMgr Singleton<PvpMgr>::Instance()

extern void sendPvpAwardMail(int  roleid, const char* roleName, int rank);

#endif /* defined(__GameSrv__Pvp__) */
