//
//  promotion_activity.cpp
//  GameSrv
//
//  Created by prcv on 13-12-2.
//
//

#include "gift_activity.h"
#include "Role.h"
#include "target.h"
#include "datetime.h"
#include "GameLog.h"
#include "Game.h"
#include "curl_helper.h"
#include "inifile.h"



static const char* sActivityDoneTimes = "donetimes";
static const char* sActivityLastDoneTime = "lastdonetime";
static const char* sActivityProgress = "progress";

GiftActivityMgr g_GiftActivityMgr;

bool GiftActivityMgr::init()
{
	
	GameInifile ini(Cfgable::getFullFilePath("gift_activity.ini"));
	
    std::list<std::string> sections;
    ini.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        string section = *iter;
		
		int actId = Utils::safe_atoi(section.c_str(), -1);
		if (actId == 0) {
			continue;
		}
		
		
		string targetStr = ini.getValue(section, "target");
        StringArray sa = StrSpilt(targetStr, " ");
        if (sa.size() != 2)
        {
            continue;
        }
		
        StringArray params = StrSpilt(sa[1], "*");
        if (params.size() != 2)
        {
            continue;
        }
		
        int type = getTargetTypeByName(sa[0].c_str());
        if (type == -1)
        {
            continue;
        }
		
		int param1 = Utils::safe_atoi(params[0].c_str(), 0);
        int param2 = Utils::safe_atoi(params[1].c_str(), 0);
        BaseTarget* target = createTarget(type, param1, param2);
        if (target == NULL)
        {
            continue;
        }
		
		string startTimeStr = ini.getValue(section, "starttime");
        string endTimeStr = ini.getValue(section, "endtime");
        int isDaily = ini.getValueT(section, "isdaily", 0);
		
        time_t startTime = DateTime::str2Time(startTimeStr.c_str());
        time_t endTime = DateTime::str2Time(endTimeStr.c_str());
//		int openServerTime = Process::env.getInt("starttime", 0);
//        int after_openserver_days = ini.getValueT(*iter, "after_openserver_days", 0);
//        int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
//        if(time(0) > endTime || time(0) < benchmarkTime)
//            continue;
		
        int times = ini.getValueT(section, "times", 1);
        string awards = ini.getValue(section, "awards");
		
		
        GiftActivity* activity = GiftActivity::create(actId, target, times, awards);
        activity->setStartTime(startTime);
        activity->setEndTime(endTime);
        activity->setIsDaily(isDaily);
        activity->afterOpenServerDays = ini.getValueT(*iter, "after_openserver_days", 0);
		activity->mTitle = ini.getValue(section, "title");
		activity->mContent = ini.getValue(section, "content");
		
        map<int, list<GiftActivity*> >::iterator typeIter = mTypeActivities.find(actId);
        if (typeIter == mTypeActivities.end())
        {
            typeIter = mTypeActivities.insert(make_pair(target->mActionType, list<GiftActivity*>())).first;
        }
        typeIter->second.push_back(activity);
        mActivities.insert(make_pair(activity->mActId, activity));

	}
	
    return true;
}

bool checkSameDay(time_t time1, time_t time2)
{
    int diff = time1 - time2;
    if (diff > SECONDS_PER_DAY || diff < -SECONDS_PER_DAY)
    {
        return false;
    }

    tm tm1, tm2;
    localtime_r(&time1, &tm1);
    localtime_r(&time2, &tm2);

    return (tm1.tm_yday == tm2.tm_yday);
}

void GiftActivityMgr::check(int type, int param1, int param2, Role *role)
{
    time_t nowTime = Game::tick;

    map<int, list<GiftActivity*> >::iterator iter = mTypeActivities.find(type);
    if (iter == mTypeActivities.end())
    {
        return;
    }

    list<GiftActivity*>& typeActivities = iter->second;
    GiftActivityProgressMgr* mgr = role->getGiftActivityProgressMgr();
    for (list<GiftActivity*>::iterator iter = typeActivities.begin(); iter != typeActivities.end(); iter++)
    {
        GiftActivity* activity = (*iter);
        if (!activity->isOpen(Game::tick))
        {
            continue;
        }

        int actId = activity->mActId;

        TargetProgress tarProgress;
        tarProgress.mIsDone = false;
        tarProgress.mProgress = 0;

        bool needReset = false;

        GiftActivityProgress* actProgress = mgr->getProgress(actId);
        if (actProgress != NULL)
        {
            //如果在活动时间内完成过相应次数，不再监测
            bool isNewActivity = !activity->isOpen(actProgress->mLastDoneTime);
            if (isNewActivity)
            {
                needReset = true;
            }
            else if (activity->getIsDaily())
            {
                bool isSameDay = checkSameDay(nowTime, actProgress->mLastDoneTime);
                if (!isSameDay)
                {
                    needReset = true;
                }
            }

            if (!needReset && actProgress->mDoneTimes >= activity->mTimes)
            {
                continue;
            }

            tarProgress.mProgress = actProgress->mProgress;
        }

        //更新进度
        if (!activity->checkImp(param1, param2, tarProgress))
        {
            continue;
        }

        //完成了就发放奖励
        if (tarProgress.mIsDone)
        {
            vector<string> awards = StrSpilt(activity->mAwards.c_str(), ";");
            ItemArray items;
            role->addAwards(awards, items, "giftactivity");
        }

        //更新活动状态
        mgr->updateGiftActivityProgress(actId, tarProgress, needReset);
    }
}



bool GiftActivityProgressMgr::init(Role* role)
{
    mOwner = role;
    role->setGiftActivityProgressMgr(this);
    load();

    return true;
}

void GiftActivityProgressMgr::updateGiftActivityProgress(int actId, TargetProgress& tarProgress, bool needReset)
{
    GiftActivityProgress* actProgress = getProgress(actId);
    Json::Value& value = mDbData[actId];
    if (actProgress == NULL)
    {

        actProgress = new GiftActivityProgress;

        actProgress->mActId = actId;
        actProgress->mLastDoneTime = 0;
        actProgress->mDoneTimes = 0;
        actProgress->mProgress = 0;

        mProgresses.insert(make_pair(actId, actProgress));
    }

    if (needReset)
    {
        actProgress->mDoneTimes = 0;
    }

    if (tarProgress.mIsDone)
    {
        actProgress->mProgress = 0;
        actProgress->mLastDoneTime = Game::tick;
        actProgress->mDoneTimes++;
    }
    else
    {
        actProgress->mProgress = tarProgress.mProgress;
    }

    value[sActivityDoneTimes] = actProgress->mDoneTimes;
    value[sActivityLastDoneTime] = actProgress->mLastDoneTime;
    value[sActivityProgress] = actProgress->mProgress;

    mDbData.save();
}

GiftActivityProgress* GiftActivityProgressMgr::getProgress(int actId)
{
    map<int, GiftActivityProgress*>::iterator iter = mProgresses.find(actId);
    if (iter == mProgresses.end())
    {
        return NULL;
    }

    return (iter->second);
}


const char* GiftActivityProgressMgr::getKeyPrefix()
{
    return "giftactivity";
}

void GiftActivityProgressMgr::load()
{
    mDbData.load(getKeyPrefix(), mOwner->getInstID());

    for (RedisHashJson<int>::iterator iter = mDbData.begin(); iter != mDbData.end(); iter++)
    {
        Json::Value &value = iter->second;
        if (!value.isObject())
        {
            continue;
        }

        GiftActivityProgress* actProgress = new GiftActivityProgress;
        try
        {
            actProgress->mActId = iter->first;
            actProgress->mDoneTimes = value[sActivityDoneTimes].asInt();
            actProgress->mLastDoneTime = value[sActivityLastDoneTime].asInt();
            actProgress->mProgress = value[sActivityProgress].asInt();

            mProgresses.insert(make_pair(actProgress->mActId, actProgress));
        }
        catch(...)
        {
            delete actProgress;
        }
    }
}

