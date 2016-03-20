//
//  ActivityMod.h
//  GameSrv
//  全部活动
//  Created by Huang Kunchao on 13-7-24.
//
//

#ifndef __GameSrv__ActivityMod__
#define __GameSrv__ActivityMod__
#include <map>
#include <stdio.h>
#include <set>
#include "FriendMgr.h" 
#include "msg.h"


using namespace std;
class Role;

//Game线程的几个活动管理
class GameActMgr
{
public:
    static void init();
    static void setOpen(int actId, int number, string& params);
    static time_t getActOpenTime(int actId);
private:
    static std::map<int, time_t> mOpenTime;
};

class GameActBase
{
public:
    virtual bool checkFresh() = 0;
    virtual void actFresh() = 0;
    virtual void setFreshTime() = 0;
private:
    time_t mFreshTime;
};

/************************  好友地下城活动  ********************************/
class FriendDungeAct
{
public:
    FriendDungeAct(Role* master, int actid):mMaster(master)
    {
        mActid = actid;
        mAttendTimes = 0;
    }
    bool IsFriendFighted(int friendid);
    bool loadFriendDungeData(int roleid);
    bool addFightedFriend(int roleid, int friendid);
    
    bool checkFresh();
    void actFresh();
    void setFreshTime();
    
    const set<int>& getFightedFriendList();
    void clearFightedFriends(int roleid);
    int getactid();
    
    int getAttendTimes();
    void addAttendTimes(int roleid, int add);
    
    int getFriendDungeRecord(int index);
    void friendDungeRecordInit(string str);
    void getFriendDungeRecord(vector<obj_friendDunge_record>& output);
    void saveFriendDungeRecord(int index, int newrecord);
    
    int getFriendDungeFriendRecord(int index);
    void friendDungeFriendRecordInit(string str);
    void saveFriendDungeFriendRecord(int index, int friendId);

private:
    int mActid;
    int mFreshTime;
    int mAttendTimes;
    Role* mMaster;
    set<int> mDungeFriends;
    std::map<int, int> mFriendDungeRecord;
    std::map<int, int> mFriendDungeFriendRecord;
};

/************************  图纸副本活动  ********************************/

struct PrintCopyInfo
{
    int copyid;
    int resettimes;
    
    PrintCopyInfo()
    {
        copyid = 0;
        resettimes = 0;
    }
};

class PrintCopyAct
{
public:
    PrintCopyAct(Role* master, int actid):mMaster(master)
    {
        mActid = actid;
    }
    bool IsCopyFinish(int copyid);
    bool loadPrintCopyData(int roleid);
    bool addFinishCopyid(int roleid, int copyid);
    bool DelCopyFromFinish(int roleid, int copyid);
    void PrintCopyReset(int roleid, int sessionid);
    void addCopyResetTimes(int roleid, int copyid, int times);
    int getCopyResetTimes(int copyid);
    const std::map<int, int>& getResetTimesList();
    const set<int>& getFinishCopy();
    int getActid(){return mActid;};
    
    bool checkFresh();
    void actFresh();
    void setFreshTime();
private:
    set<int> mFinishCopy;
    std::map<int, int> mCopyResetTimesList;
    int mActid;
    int mFreshTime;
    Role* mMaster;
};

/*****************************  tower defense  ****************************/
class TowerDefenseAct
{
public:
    TowerDefenseAct(Role* master, int actid):mMaster(master)
    {
        mActid = actid;
    }
    int getActid(){return mActid;};
    
    bool checkFresh();
    void actFresh();
    void setFreshTime();
    
    void loadData();
    void addEnterTimes(int addTimes);
    int getEnterTimes(){return mEnterTimes;}
private:
    int mEnterTimes;
    int mActid;
    int mFreshTime;
    Role* mMaster;
};


/////////////////
class PetCampAct
{
public:
    PetCampAct(Role* master, int actid):mMaster(master)
    {
        mActid = actid;
    }
    bool IsCampFinish(int copyid);
    bool loadPetCampData(int roleid);
    bool addPetCampid(int roleid, int copyid);
    bool DelCampFromFinish(int roleid, int copyid);
    void PetCampReset(int roleid, int sessionid);
    void addCampResetTimes(int roleid, int copyid, int times);
    int getCampResetTimes(int copyid);
    const std::map<int, int>& getResetTimesList();
    const set<int>& getFinishCamp();
    int getActid(){return mActid;};
private:
    set<int> mFinishCamp;
    std::map<int, int> mCampResetTimesList;
    int mActid;
    Role* mMaster;
};

/****************************** pet dunge ****************************/
//现在早已经不用了，这个活动
class PetDungeAct
{
public:
    PetDungeAct(Role* master, int actid):mMaster(master)
    {
        mActid = actid;
    }
    bool IsFriendFighted(int friendid);
    bool loadPetDungeData(int roleid);
    bool addFightedFriend(int roleid, int friendid);
    void petDungeReset();
    const set<int>& getFightedFriendList();
    void clearFightedFriends(int roleid);
    int getactid();
    
private:
    set<int> mDungeFriends;
    int mActid;
    Role* mMaster;
};
struct SceneCfgDef;
void sendFriendAwardMail(SceneCfgDef* scene, Role* role);
void saveFriendAward(SceneCfgDef* scene, Role* role);


//extern PetDunngeActMgr g_PetDunngeActMgr;
//extern FriendDungeActMgr g_friendDungeActMgr;
//extern PrintCopyActMgr g_PrintCopyActMgr;
//extern PetCampActMgr g_PetCampActMgr;

/*************************************  神秘副本  **************************************/
#define MYSTICAL_COPY_COUNT 2

struct MysticalCopyItem
{
    MysticalCopyItem(): index(0),
                        mIsOpen(false),
                        endTime(0),
                        copyType(0),
                        nextBeginTime(0),
                        enterLvl(0)
    {
        copyids.clear();
    }
    
    bool checkCopyid(int copyid)
    {
        for (int i = 0; i < copyids.size(); i++) {
            if (copyids[i] == copyid) {
                return true;
            }
        }
        
        return false;
    }
    void setOpen(bool isopen)
    {
        mIsOpen = isopen;
        //通知前端
    }
    
    bool isOpen()
    {
        return mIsOpen;
    }
    
    bool checkOpen(int now);
    
    int calcNextOpenTime(int now);
    
    int getEndTime(int now)
    {
        return endTime;
    }
    
    int index;
    bool mIsOpen;
    int endTime;
    int nextBeginTime;
    int copyType;
    int enterLvl;
    vector<int> copyids;
    vector<int> openDays;
    
};

struct MysticalCopyGroup {
    MysticalCopyGroup()
    {
        copyIndexs.clear();
        eraseTime = 0;
    }
    
    bool checkIndexAndCopyid(int index, int copyid)
    {
        for (int i = 0; i < copyIndexs.size(); i++) {
            if (copyIndexs[i].index == index && copyIndexs[i].checkCopyid(copyid)) {
                return true;
            }
        }
        
        return false;
    }
    
    bool isIndexExist(int index)
    {
        for (int i = 0; i < copyIndexs.size(); i++) {
            if (index == copyIndexs[i].index) {
                return true;
            }
        }
        return false;
    }
    
    void getIndexs(vector<obj_mysticalcopy>& outdata);
    
    std::vector<MysticalCopyItem> copyIndexs;
    int eraseTime;
};

typedef std::vector<MysticalCopyGroup> MysticalCopyGroupData;

class MysticalCopyMgr
{
public:
    MysticalCopyMgr()
    {
        m_running = false;
        mBeginCycle = false;
    }
    void init();
    void loadDataFromDB();
    void loadDataFromConfig();
    void update();
    void getCopyListIndex(vector<obj_mysticalcopy>& outdata);
    int checkCopyAndIndex(int index, int copy);
    int getSurplusSecForFreshList();
    MysticalCopyGroup createNewGroup();
    void saveMysticalCopyData();
    void resetMgr();
    void printCurrCopysIndexs();
    bool isRunning();

    void processDay();
    void getCycleCopyListIndex(vector<obj_mysticalcopy>& outdata);
private:
    bool m_running;
    bool mBeginCycle;
    int m_actid;
    int m_currentBeginIndex;
    int m_nextBeginIndex;
    time_t m_nextReplace;
//    std::vector<int> m_CurrentIndexs;
    MysticalCopyGroupData m_currentGroup;
    vector<MysticalCopyItem> mCycleCopyList;
    int mProcessWDay;
};
extern MysticalCopyMgr g_MysticalCopyMgr;

/************************************* 地下城相关 ************************************/
void friendDungeActOpen(int actId, int number, string& params);
int handlerDungeResultAward(int sceneid, int result, Role* role);
int dungeWipeOut(int dungeIndex, int beginsceneid, int endsceneId, Role* role, std::vector<obj_dungeawawrd>& outdata);

#endif /* defined(__GameSrv__ActivityPetCasino__) */
