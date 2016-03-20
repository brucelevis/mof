//
//  PetPvp.h
//  GameSrv
//
//  Created by pireszhi on 13-9-27.
//
//

#ifndef __GameSrv__PetPvp__
#define __GameSrv__PetPvp__

#include <iostream>
#include <stdio.h>
#include <vector>
#include "Pet.h"
#include "Defines.h"
#include <map>
#define PETPVP_MATCH_RANGE 10;


//#define PETPVP_VERSION (1)

static const int static_PetPvpVersion = 2;

struct PetForm {
    PetForm()
    {
        mPets.clear();
    }

    std::vector<int> mPets;
    
    string tostring()
    {
        std::ostringstream formationStr;
        for (int i = 0; i < mPets.size(); i++) {
            formationStr<<mPets[i];
            formationStr<<";";
        }
        return formationStr.str();
    }
};

typedef std::vector<int> PetList;

struct PetPvpData {
    PetPvpData(int roleid): mRoleid(roleid),
                            mPvpType(0),
                            mLvl(0),
                            mRoletype(0),
                            mWinTimes(0),
                            mLostTimes(0),
                            mCups(0),
                            mPoints(0),
                            mRank(0),
                            mGlobalRank(0),
                            continuous(0),
                            mLock(false),
                            mIsBattling(false),
                            mEnemyIndexs(0),
                            mEnemyMatchIndex(0),
                            mEnemy(NULL),
                            mEnemyid(0),
                            mAwardRank(0),
                            mVersion(0)
    
    {
        mEnemyIndexs.clear();
        resetHistory();
    }
    void loadPetForm(int roleid)
    {
        
    }
    
    void getFormation(PetList& petlist)
    {
        petlist.clear();
        for (int i = 0; i < mPetsForPvp.mPets.size(); i++) {
            petlist.push_back(mPetsForPvp.mPets[i]);
        }
    }
    
    bool isLock()
    {
        return mLock;
    }
    
    void lock()
    {
        mLock = true;
    }
    
    void unLock()
    {
        if (mLock) {
            mLock = false;
        }
    }
    
    bool updateHistoryRank(int type, int newRank)
    {
        if (type >= 1 && type <= mHistoryRank.size()) {
            if (mHistoryRank[type-1] == 0 || mHistoryRank[type-1] > newRank) {
                mHistoryRank[type-1] = newRank;
                return true;
            }
        }
        return false;
    }
    
    string historyRankToString()
    {
        std::ostringstream historyStr;
        for (int i = 0; i < mHistoryRank.size(); i++) {
            historyStr<<mHistoryRank[i];
            historyStr<<";";
        }
        
        return historyStr.str();
    }
    
    bool isFormationEmpty()
    {
        bool ret = true;
        for (int i = 0; i < mPetsForPvp.mPets.size(); i++) {
            if (mPetsForPvp.mPets[i]) {
                ret = false;
            }
        }
        return ret;
    }
    
    bool formationResize(int newsize)
    {
        mPetsForPvp.mPets.resize(newsize, 0);
        return true;
    }
    
    void getHistoryBestRank(std::vector<int>& outdata)
    {
        outdata.clear();
        for (int i = 0; i < mHistoryRank.size(); i++) {
            outdata.push_back(mHistoryRank[i]);
        }
    }
    
    void resetHistory()
    {
        mHistoryRank.clear();
        mHistoryRank.resize(3, 0);
    }
    
    int mPvpType;       //不需要在DB中保存在role:
    int mRoleid;
    string mRolename;
    int mLvl;
    int mRoletype;
    int mWinTimes;      //胜场
    int mLostTimes;     //负场
    int mCups;    //每场战斗后，获胜的增加，失败的减少，用于排行
    int mPoints;  //用于兑换物品
    
    int mRank;      //在该段位中的排名  不需要在DB中保存在role:
    int mGlobalRank;    //全局排名   不需要在DB中保存在role:

    PetForm mPetsForPvp;    //宠物阵型

    int continuous;     //连胜场次
    bool mLock;
    bool mIsBattling;
    std::vector<int> mHistoryRank;
    std::vector<int> mEnemyIndexs;
    int mEnemyMatchIndex;
    PetPvpData* mEnemy;
    int mEnemyid;
    int mAwardRank;
    int mVersion;     //加个版本编号，方便以后改版是数据的更改
};
enum ReSortType
{
    eForward,
    eBackward,
};

struct PetPvpAwardData
{
    PetPvpAwardData()
    {
        mPvpType = mRank = mAwardPoint = 0;
    }
    int mPvpType;
    int mRank;
    int mAwardPoint;
};
void getIntegersOutoforder(std::vector<int>& integers);

class PetPvpMgr
{
public:
    typedef std::vector<PetPvpData*> PetPvpSectionData;
    typedef std::vector<PetPvpData*> PetPvpGlobalData;
    
public:
    void safe_getPlayerData(Role* role);
    
    void safe_editFormation(Role* role, std::vector<int> newformation);
    
    void safe_UpdatePlayerData(Role* role, bool iswin);
    
    void safe_matchEnemy(Role* role, bool isFirstSearch);
    
    void safe_beginBattle(Role* role, int enemyid, int sceneid);
    
    void safe_delPlayer(Role* role);
    
    void safe_endPlayerSearchEnemy(Role* role);
    
    void safe_removePetsInFormation(int roleid, std::vector<int> petlist);
    
    void safe_getPetPvpFormation(Role* role);
    
    void safe_buyItemsUsingPoints(Role* role, int item, int count);
    
    void safe_gmAddPetPvpPoints(int roleid, int addpoints);
    
    void safe_gmSetPetPvpPoints(int roleid, int setpoints);
    
    void safe_gmSetPetPvpCups(int roleid, int setcups);

public:
    
    void load();
    
    //匹配对手
    PetPvpData* matchEnemy(int roleid);
    
    //开始战斗
    bool onBeginBattle(int playerid, int enemyid);
    
    //战斗结束后，更新玩家的数据
    PetPvpData* onUpdatePlayerData(int roleid, bool iswin, int givePoints, int& getCups, int& enemyid, int& enemyGetPoints, int& enemySurplusPoints);
    
    //将玩家从战斗队列中删除
    void eraseRole(int roleid);
    
    //查看玩家是否正在竞技场战斗中
    bool isPlayerInPetPvp(int roleid);
    
    //获取玩家在活动中的数据
    PetPvpData* onGetPlayerData(int roleid);
    
    //删除
    void delayDelPalyer(int roleid);
    
    //编辑阵型
    bool onEditFormation(int roleid, int rolelvl, PetList newform);
    
    //删除宠物
    bool onRemovePet(int roleid, std::vector<int> petlist);
    
    void onHeartBeat();
    
    void onPlayerEndSearchEnemy(int roleid);
    
    bool onGetWeeklyAward(int roleid, PetPvpData* playerdata, PetPvpAwardData& outdata);
    
    bool isPlayerFormationEmpty(int roleid);
    
    void onGetPvpPetList(int roleid, PetList& formation);
    
    int onBuyItemsUsingPoints(int roleid, int itemid, int count, int& surplus);
    
    void addPetPvpPoints(int roleid, int addpoints);
    
    void setPetPvpCups(int roleid, int setCups);
    
    void setPetPvpPoints(int roleid, int setpoints);
    
    //配合测试用
    void testMode(obj_roleinfo roleinfo, int sessionid);
    
    void resetDataWithVersionChange();
    
    //每日重设
    void setPlayerPvpType();
    
    void savePlayerData(PetPvpData* playerdata);
    
    void updatePlayerData(int roleid, int rolelvl); 
    
private:
    
    //从数据库中加载数据
    void loadDataInDB();
    
    //
    bool onAddNewPlayer(PetPvpData* newplayer);
    
    bool onDelPlayer(int roleid);
    
    //重新根据玩家现有的分数进行排名
    bool reSort(PetPvpType pvptype, int beginIndex, ReSortType sortType);
    
    //保存阵型
    bool saveFormation(int roleid, PetList formation);
    
    bool saveHistoryRank(int roleid, std::vector<int> history);
    
    //将玩家加入战斗队列，期间不允许玩家修改阵型
    void pushBackRole(int roleid);
    
    void distribAward();
    
    void setDistribTime();
    
    bool expandMatchEnemy(PetPvpData* playerdata);
    
    PetPvpData* loadPlayerData(int roleid);
    
    void addPlayerIntoSection(PetPvpType type, PetPvpData* playerData);
    
    void saveSectionDataInDB();
    
    bool initMatchEnemy(PetPvpData *playerdata);
    
//准备在2.5后删除
private:
    
    //玩家登录的时候检查是否在排行内
//    bool onPetPvpCheckIn(int roleid, int lvl);
    
    //玩家升级的时候判断是否晋段, 2.5后不进行实时换段s
//    void onPlayerAdvance(int roleid, int lvl);

private:
    std::map<int, PetPvpData*> mPlayersData;
    std::map<int, PetPvpSectionData > mPvpDatas;
    std::set<int> mRolesInPetPvp;
    PetPvpGlobalData mGlobalRankData;
    std::vector<int> mDelPlayerList;
    int mDistribTime;
    
    int mVersion;
};

int getBestHistoryRank(int pvpType, const std::vector<int>& inputData);

#define SPetPvpMgr Singleton<PetPvpMgr>::Instance()
#endif /* defined(__GameSrv__PetPvp__) */


/****************************** petpvp ********************************/

