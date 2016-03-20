//
//  push_notify_cfg.cpp
//  GameSrv
//
//  Created by 丁志坚 on 11/20/14.
//
//

#include "push_notify_cfg.h"
#include "inifile.h"
#include <list>
#include "Utils.h"
#include "GameIniFile.h"
using namespace std;

void PushNotifyCfg::load(const char *fullpath)
{
    try {
        GameInifile ini(fullpath);
        list<string> sections;
        ini.getSections(std::inserter(sections, sections.begin()));
        
        for (list<string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
            
            string section = *iter;
            
            string periodStr = ini.getValue(section, "type");
            int periodType;
            if (periodStr == "daily") {
                periodType = kPeriodDaily;
            } else if (periodStr == "weekly") {
                periodType = kPeriodWeekly;
            } else {
                continue;
            }
            
            PushNotifyCfgDef* cfgDef = new PushNotifyCfgDef;
            cfgDef->mPeriodType = periodType;
            
            string ticker = ini.getValue(section, "ticker");
            string title = ini.getValue(section, "title");
            string content = ini.getValue(section, "content");
            cfgDef->mTicker = ticker;
            cfgDef->mTitle = title;
            cfgDef->mContent = content;
            
            string daysStr = ini.getValue(section, "days");
            vector<string> daysVec = StrSpilt(daysStr, ";");
            memset(cfgDef->mDays, 0, sizeof(cfgDef->mDays));
            for (int i = 0; i < daysVec.size(); i++) {
                int day = Utils::safe_atoi(daysVec[i].c_str());
                if (day > 0 && day < 7) {
                    cfgDef->mDays[day - 1] = 1;
                }
            }
            
            string dayTimesStr = ini.getValue(section, "daytimes");
            vector<string> daytimesVec = StrSpilt(dayTimesStr, ";");
            for (int i = 0; i < daytimesVec.size(); i++) {
                vector<string> dayTimesStrs = StrSpilt(daytimesVec[i], ":");
                if (dayTimesStrs.size() != 2) {
                    continue;
                }
                
                int hour = Utils::safe_atoi(dayTimesStrs[0].c_str());
                int minute = Utils::safe_atoi(dayTimesStrs[1].c_str());
                if (hour >= 60 || hour < 0 || minute >= 60 || minute < 0) {
                    continue;
                }
                
                DayTime dayTime;
                dayTime.mHour = hour;
                dayTime.mMinute = minute;
                dayTime.mSecond = 0;
                cfgDef->mDayTimes.push_back(dayTime);
            }
        }
        
    } catch(...) {
        
    }
}


void PushNotifyCfg::traverseNotifies(Traverser *traverser)
{
    vector<PushNotifyCfgDef*>::iterator iter;
    for (iter = mNotifies.begin(); iter != mNotifies.end(); iter++) {
        if (!traverser->callback(*iter)) {
            break;
        }
    }
}