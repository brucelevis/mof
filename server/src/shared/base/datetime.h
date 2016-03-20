#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <string>
#include <sys/time.h>
using namespace std;

#define SECONDS_PER_MIN 60
#define SECONDS_PER_HOUR 3600
#define SECONDS_PER_DAY (3600 * 24) 

class DateTime
{
public:
    static const int kSecondPerMin = 60;
    static const int kSecondPerHour = 3600;
    static const int kSecondPerDay = 3600 * 24;
    
    //获取上次到某点的时间戳
    static int getLastTimeHitHour(int hour);
    
    //获取下次到某点的时间戳
    static int getNextTimeHitHour(int hour);
    
    //获取当天零点时间戳
    static int getDateTime();
    
    //获取明天零点时间戳
    static int getTomorrowDateTime();
    
    //获取本周第一天零点时间戳（从周一算一周第一天）
    static int getWeekDateTime();
    
    //获取下周第一天零点时间戳（从周一算一周第一天）
    static int getNextWeekDateTime();
    
    //获取当月第一天零点时间戳
    static int getMonthDateTime();
    
    //获取下月第一天零点时间戳
    static int getNextMonthDateTime();
    
    //是否为闰年
    static bool isLeap(int year);
    
    static int getWeekDay(time_t t);
    
    
    static time_t str2Time(const char* tstr);
    static string time2Str(time_t t);
    
    
};

class TimeInfo
{
public:
    TimeInfo();
    ~TimeInfo();
    
    void update();
    
    timeval mTime;
    tm      mTm;
};


#endif