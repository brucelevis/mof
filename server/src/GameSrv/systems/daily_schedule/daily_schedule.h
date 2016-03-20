//
//  daily_schedule_item.h
//  GameSrv
//
//  Created by 丁志坚 on 11/17/14.
//
//

#ifndef __GameSrv__daily_schedule__
#define __GameSrv__daily_schedule__

#include <stdio.h>

class DailyScheduleCfgDef;

class DailySchedule
{
public:
    static DailySchedule* create(DailyScheduleCfgDef* cfgDef);
    
    bool init(DailyScheduleCfgDef* cfgDef);
    void setProgress(bool isCompleted, int progress) {mProgress = progress; mIsCompleted = isCompleted;}
    virtual bool check(int target, int progress);
    virtual bool trigger(int target, int progress);
    virtual void update(int target, int progress) {}
    void clear();
    
    int mType;
    int mTarget;
    int mTargetProgress;
    
    int mId;
    int mProgress;
    bool mIsCompleted;
    
};


#endif /* defined(__GameSrv__daily_schedule_item__) */
