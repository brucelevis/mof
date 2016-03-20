#include "timestamp.h"

#include <stddef.h>

#include <sys/time.h>

#include "log.h"

TimeProfiler g_TimeProfiler;

Timestamp::Timestamp () :
 ts (now_us () / 1000)
{
}

Timestamp::~Timestamp ()
{
}

uint64_t Timestamp::now_us ()
{
    //  Use POSIX gettimeofday function to get precise time.
    struct timeval tv;
    gettimeofday (&tv, NULL);
    uint64_t nowus = (tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec);
    return nowus;
}

uint64_t Timestamp::now_ms ()
{
    //  Use POSIX gettimeofday function to get precise time.
    struct timeval tv;
    gettimeofday (&tv, NULL);
    uint64_t nowus = (tv.tv_sec * (uint64_t) 1000 + tv.tv_usec / 1000);
    return nowus;
}

uint64_t Timestamp::update()
{
    ts = now_us() / 1000;
    return ts;
}

uint64_t Timestamp::elapsed()
{
    uint64_t nowms = now_us() / 1000;
    return nowms - ts;
}



void TimeProfiler::profile(const string& name, uint64_t ms)
{
    map<string, ProfileData>::iterator iter = mDatas.find(name);
    if (iter == mDatas.end())
    {
        ProfileData data;
        data.mTimes = 1;
        data.mCost = ms;
        mDatas[name] = data;
    }
    else
    {
        iter->second.mCost += ms;
        iter->second.mTimes++;
    }
}


void TimeProfiler::dump()
{
    map<string, TimeProfiler::ProfileData>::iterator iter;
    for (iter = mDatas.begin(); iter != mDatas.end(); iter++)
    {
        log_info(iter->first << ": execute " << iter->second.mTimes << " times, "
                 << "cost " << iter->second.mCost << " us");
    }
}

