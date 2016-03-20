//
//  TimeLimitKillCopy.h
//  GameSrv
//
//  Created by prcv on 13-11-25.
//
//

#ifndef __GameSrv__TimeLimitKillCopy__
#define __GameSrv__TimeLimitKillCopy__

#include <iostream>

#include "MultiPlayerRoom.h"
#include "Defines.h"
#include "DataCfg.h"
#include "ItemHelper.h"

#include <map>
#include <string>
#include <vector>
using namespace std;

struct CopyCost
{
    RewardStruct mOther;
    ItemArray    mItems;
};


class TlkMonster
{
public:
    TlkMonster(int instId, int mod, int group, int index) : mMod(mod)
    {
        mInstId = instId;
        mGroupIdx = group;
        mIndex = index;
    }
    
    static TlkMonster* create(int instId, int mod, int group, int index)
    {
        TlkMonster* monster = new TlkMonster(instId, mod, group, index);
        return monster;
    }
    
    //怪物实体id
    int mInstId;
    //怪物模版号
    int mMod;
    //怪物组索引
    int mGroupIdx;
    //怪物组内索引
    int mIndex;
};

class TimeLimitKillCopy : public BaseActRoom
{
    typedef map<int, TlkMonster*> MonsterMap;
    typedef MonsterMap::iterator MonsterIter;
    
public:
    
    static const char* getClassName() {return "TimeLimitKillCopy";}
    
    TimeLimitKillCopy(int copyId);
    ~TimeLimitKillCopy();
    
    static TimeLimitKillCopy* create(int copyId, const CopyCost& cost);
    
    bool   init(TimeLimitKillCopyCfgDef& cfg);
    bool   killMonster(int monsterId);
    void   refreshMonster();
    void   finish();
    void   handleLeave(Role* role);
    string   handleGetAward(Role* role);
    void   sendAwards(Role* role, const string& awards);
    bool   takeCost(Role* role);
    
    int    getRoleCount();
    
    virtual void OnHeartBeat();
    
    
    int arraySum(const vector<int>& arr)
    {
        int ret = 0;
        for (int i = 0; i < arr.size(); i++)
        {
            ret += arr[i];
        }
        
        return ret;
    }
    
    template<class T>
    T arraySum(const vector<T>& arr)
    {
        T ret;
        for (int i = 0; i < arr.size(); i++)
        {
            ret += arr[i];
        }
        
        return ret;
    }
    
    int getMonsterNum()
    {
        int sum = 0;
        for (int i = 0; i <= mGroupIdx; i++)
        {
            sum += arraySum(mRemainCount[i]);
        }
        return sum;
    }
    
    int getTotalKillCount()
    {
        int sum = 0;
        for (int i = 0; i <= mGroupIdx; i++)
        {
            sum += arraySum(mKilledCount[i]);
        }
        return sum;
    }
    
    int getScore()
    {
        return mScore;
    }
    
    string getAwards(int score)
    {
        int maxIndex = -1;
        int maxScore = -1;
        for (int i = 0; i < mCfg.mAwards.size(); i++)
        {
            if (mCfg.mAwards[i].score <= score)
            {
                if (mCfg.mAwards[i].score > maxScore)
                {
                    maxIndex = i;
                    maxScore = mCfg.mAwards[i].score;
                }
            }
        }
        
        string awards = "";
        if (maxIndex >= 0 && maxIndex < mCfg.mAwards.size())
        {
            awards = mCfg.mAwards[maxIndex].award;
        }
        
        return awards;
    }
    
    int getCopyId()
    {
        return mCopyId;
    }
    
    int getLastTime()
    {
        return mCfg.mLastTime;
    }
    

    
private:
    
    //怪物id索引，每生成一个怪，自增1，初始为1
    int mMonsterIdx;
    //当前刷怪的组索引
    int mGroupIdx;
    //当前分数
    int mScore;
    
    int mCopyId;
    int mEndTime;
    int mNextRefreshTime;
    
    CopyCost                mCopyCost;
    TimeLimitKillCopyCfgDef mCfg;
    vector<vector<int> >  mBufferedUnitCount;
    vector<vector<int> >  mKilledCount;
    vector<vector<int> >  mRemainCount;
    
    MonsterMap    mMonsters;
};

#endif /* defined(__GameSrv__TimeLimitKillCopy__) */
