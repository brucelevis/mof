//
//  promotion_activity.h
//  GameSrv
//
//  Created by prcv on 13-12-2.
//
//

#ifndef __GameSrv__gift_activity__
#define __GameSrv__gift_activity__

#include <iostream>
#include <vector>
#include <list>
#include <map>
using namespace std;

#include "target.h"
#include "RedisHashJson.h"
#include "Defines.h"



class Role;

//限时奖励活动，每个活动需要达成一个目标，达成目标后可以拿到活动奖励
class GiftActivity
{
public:
    
    static GiftActivity* create(int actId, BaseTarget* target, int times, const string& awards)
    {
        GiftActivity* activity = new GiftActivity;
        activity->init(actId, target, times, awards);
        
        return activity;
    }
    
    bool init(int actId, BaseTarget* target, int times, const string& awards)
    {
        mTarget = target;
        mActId = actId;
        mTimes = times;
        mAwards = awards;
        
        return true;
    }
    
    //更新进度
    //当有更新的时候返回true
    bool check(int type, int param1, int param2, TargetProgress& progress)
    {
        if (mTarget->mActionType != type)
        {
            return false;
        }
        
        return checkImp(param1, param2, progress);
    }
    
    //更新进度
    //当有更新的时候返回true
    bool checkImp(int param1, int param2, TargetProgress& progress)
    {
        if (mTarget->trigger(param1, param2))
        {
            mTarget->update(param1, param2, progress);
            return true;
        }
        
        return false;
    }
    
    bool isOpen(time_t testTime)
    {
        int openServerTime = Process::env.getInt("starttime", 0);
        int benchmarkTime = openServerTime + afterOpenServerDays * 3600 * 24;
        return testTime > mStartTime && testTime < mEndTime && testTime > benchmarkTime;
    }
    
    //目标
    BaseTarget* mTarget;
    //可完成次数
    int mTimes;
    //活动id
    int mActId;
    
    //活动奖励
    string mAwards;
    int afterOpenServerDays;
    //开始时间
    READWRITE(time_t, mStartTime, StartTime)
    //结束时间
    READWRITE(time_t, mEndTime, EndTime)
    //是否每日更新
    READWRITE(bool, mIsDaily, IsDaily)
	
	string mTitle;
	string mContent;
};

//活动状态
class GiftActivityProgress
{
public:
    //活动id
    int mActId;
    //活动完成的次数
    int mDoneTimes;
    //活动最后一次完成的时间
    int mLastDoneTime;
    //活动进度
    int mProgress;
};


//活动状态管理类
class GiftActivityProgressMgr
{
public:
    GiftActivityProgressMgr()
    {
        
    }
    
    ~GiftActivityProgressMgr()
    {
        for (map<int, GiftActivityProgress*>::iterator iter = mProgresses.begin(); iter != mProgresses.end(); iter++)
        {
            if (iter->second)
            {
                delete iter->second;
            }
        }
    }
    
    static GiftActivityProgressMgr* create(Role* role)
    {
        GiftActivityProgressMgr* mgr = new GiftActivityProgressMgr;
        mgr->init(role);
        
        return mgr;
    }
    
    bool init(Role* role);
    
    //更新活动进度
    void updateGiftActivityProgress(int actId, TargetProgress& tarProgress, bool needReset);
    
    //获取活动的状态
    GiftActivityProgress* getProgress(int actId);
    
    //更新活动进度
    void load();
    
    
    map<int, GiftActivityProgress*> mProgresses;
    Role*                           mOwner;
    
    RedisHashJson<int> mDbData;
    
    static const char* getKeyPrefix();
};


//活动管理类
class GiftActivityMgr
{
public:
    bool init();
    
    
    GiftActivity* getActivity(int activityId)
    {
        map<int, GiftActivity*>::iterator iter = mActivities.find(activityId);
        if (iter == mActivities.end())
        {
            return NULL;
        }
        
        return iter->second;
    }
    
    void check(int type, int param1, int param2, Role* role);
    
    map<int, GiftActivity*> mActivities;
    map<int, list<GiftActivity*> > mTypeActivities;
    
};

extern GiftActivityMgr g_GiftActivityMgr;


#endif /* defined(__GameSrv__promotion_activity__) */
