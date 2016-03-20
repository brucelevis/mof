//
//  datetime.cpp
//  GameSrv
//
//  Created by prcv on 13-6-17.
//
//

#include "datetime.h"
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int DateTime::getLastTimeHitHour(int hour)
{
    time_t nowtime = time(NULL);
    
    tm nowtm;
    localtime_r(&nowtime, &nowtm);
    
    if (nowtm.tm_hour > hour){
        return nowtime - nowtm.tm_min * 60 - nowtm.tm_sec - (nowtm.tm_hour - hour) * 3600;
    }
    
    nowtm.tm_hour = hour;
    nowtm.tm_min = nowtm.tm_sec = 0;
    return mktime(&nowtm) - SECONDS_PER_DAY;
}


int DateTime::getNextTimeHitHour(int hour)
{
    return getLastTimeHitHour(hour) + SECONDS_PER_DAY;
}

int DateTime::getDateTime()
{
    time_t now = time(NULL);
    struct tm nowtm;
    localtime_r(&now, &nowtm);

    return now - nowtm.tm_hour * 3600 - nowtm.tm_min * 60 - nowtm.tm_sec;
}
int DateTime::getTomorrowDateTime()
{
    return getDateTime() + 3600 * 24;
}

int DateTime::getWeekDateTime()
{
    return getNextWeekDateTime() - 7 * 24 * 3600;
}

int DateTime::getNextWeekDateTime()
{
    time_t now = time(NULL);
    struct tm nowtm;
    localtime_r(&now, &nowtm);
    time_t daybegin = now - nowtm.tm_hour * 3600 - nowtm.tm_min * 60 - nowtm.tm_sec;

    time_t weekbegin;
    if (nowtm.tm_wday == 0)
    {
        weekbegin = daybegin + 3600 * 24;
    }
    else
    {
        weekbegin = daybegin + (8 - nowtm.tm_wday) * 3600 * 24;
    }

    return weekbegin;
}

int DateTime::getMonthDateTime()
{
    return 0;
}
int DateTime::getNextMonthDateTime()
{
    return 0;
}

bool DateTime::isLeap(int year)
{
    return year % 400 ? (year % 100 ? (year % 4 ? 0 : 1) : 0) : 1;
}


time_t DateTime::str2Time(const char* tstr)
{
    struct tm ttm;
    memset(&ttm, 0, sizeof(ttm));

    sscanf(tstr, "%d-%d-%d %d:%d:%d", &ttm.tm_year, &ttm.tm_mon, &ttm.tm_mday,
           &ttm.tm_hour, &ttm.tm_min, &ttm.tm_sec);

    ttm.tm_year -= 1900;
    ttm.tm_mon -= 1;

    return mktime(&ttm);
}

string DateTime::time2Str(time_t t)
{
    struct tm ttm;
    localtime_r(&t, &ttm);
    char timestr[64];
    sprintf(timestr, "%d-%d-%d %d:%d:%d", ttm.tm_year + 1900, ttm.tm_mon + 1, ttm.tm_mday,
            ttm.tm_hour, ttm.tm_min, ttm.tm_sec);

    return timestr;
}

int DateTime::getWeekDay(time_t t)
{
    struct tm ttm;
    localtime_r(&t, &ttm);
    
    return ttm.tm_wday;
}

TimeInfo::TimeInfo()
{
    update();
}

TimeInfo::~TimeInfo()
{
    
}

void TimeInfo::update()
{
    gettimeofday(&mTime, NULL);
    localtime_r(&mTime.tv_sec, &mTm);
}
