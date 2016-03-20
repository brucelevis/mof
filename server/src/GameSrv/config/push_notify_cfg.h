//
//  push_notify_cfg.h
//  GameSrv
//
//  Created by 丁志坚 on 11/20/14.
//
//

#ifndef __GameSrv__push_notify_cfg__
#define __GameSrv__push_notify_cfg__

#include <stdio.h>
#include <string>
#include <vector>
#include "basedef.h"
using namespace std;


enum PeriodType
{
    kPeriodDaily,
    kPeriodWeekly,
};

struct DayTime {
    int mHour;
    int mMinute;
    int mSecond;
};

struct PushNotifyCfgDef
{
    int mId;
    int mPeriodType;
    int mDays[7];
    
    vector<DayTime> mDayTimes;
    
    string mTicker;
    string mTitle;
    string mContent;
};

class PushNotifyCfg
{
public:
    void load(const char* fullpath);
    void traverseNotifies(Traverser* traverser);
    
    vector<PushNotifyCfgDef*> mNotifies;
};


#endif /* defined(__GameSrv__push_notify_cfg__) */
