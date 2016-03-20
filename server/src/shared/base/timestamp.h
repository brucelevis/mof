//
//  clock.h
//  GameSrv
//
//  Created by prcv on 13-6-9.
//
//

#ifndef shared_timestamp_h
#define shared_timestamp_h

#include <stdint.h>
#include <string>
#include <map>
using namespace std;

class Timestamp
{
public:
    
    Timestamp ();
    ~Timestamp ();
    
    //  High precision timestamp.
    static uint64_t now_us ();
    static uint64_t now_ms ();
    
    //  Low precision timestamp. In tight loops generating it can be
    //  10 to 100 times faster than the high precision timestamp.
    uint64_t update ();
    
    uint64_t elapsed();
    
private:
    
    //  Physical time corresponding to the TSC above (in milliseconds).
    uint64_t ts;
    
    Timestamp (const Timestamp&);
    const Timestamp &operator = (const Timestamp&);
};

class TimeProfiler
{
public:
    struct ProfileData
    {
        uint64_t mTimes;
        uint64_t mCost;
    };
    
    void profile(const string& name, uint64_t ms);
    
    void dump();
    
    
    map<string, ProfileData> mDatas;
};

class TimeTrack
{
public:
    explicit TimeTrack(const char* name, TimeProfiler* profiler)
    {
        mProfiler = profiler;
        mStartTime = Timestamp::now_us();
        mName = name;
    }
    ~TimeTrack()
    {
        uint64_t endTime = Timestamp::now_us();
        mProfiler->profile(mName, endTime - mStartTime);
    }
    
    string mName;
    uint64_t mStartTime;
    TimeProfiler* mProfiler;
};

extern TimeProfiler g_TimeProfiler;
#define PROFILE_TIME(name)  TimeTrack timeTrack##name(#name, &g_TimeProfiler)

#endif
