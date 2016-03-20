//
//  Activity.h
//  GameSrv
//
//  Created by  tfl on 13-7-19.
//
//

#ifndef __GameSrv__Activity__
#define __GameSrv__Activity__

#include <iostream>
#include "Singleton.h"
#include "Utils.h"
#include "std_includes.h"

using namespace std;

#define MAX_ACT_ID 128

typedef void (*ACTTIMERFUN)(int actId, int number, string& params);

// 活动定义
enum ActivityEnums
{
    ae_friend_tower = 1,//好友塔防 1
    ae_world_boss = 2,//世界boss 2
    ae_gem_lottery = 3,//宝石抽奖3
    ae_print_copy = 4,// 图纸副本（魔化精英4）
    ae_pet_pk = 5,//幻兽大比拼5
    ae_friend_dunge = 6, //好友地下城6
    ae_pet_copy = 7,////幻兽大本营7
    ae_friend_copy = 8,//好友副本8
    ae_dunge = 9,//地下城9
    ae_mysterious = 10,// 神秘商人10
    ae_tower_defense = 11,//保卫神像
    ae_mystical_copy = 12, //神秘副本
    ae_pet_pvp = 13, //幻兽竞技场
    ae_daily_answer = 14, //每日答题
    ae_fameshall = 15, //名人堂

    //公会活动ID
    ae_begin_guild_activity = 40,
    ae_guild_boss = 41, // 公会boss
    ae_guild_train = 42, // 公会试练
    ae_guild_treasure_fight = 43,       //公会宝藏战活动
    
    ae_end_guild_activity = 60,
    ae_synpvp = 61,      //同步竞技场匹配活动
    
};

// 活动时间类型
enum ActivityType
{
   ACT_NONE,
   ACT_FESTIVAL,
   ACT_WEEK,
   ACT_DAY,
};

// 当前时间信息
struct timeInfo
{
    int year;
    int month;
    int mday;
    int wday;
    int hour;
    int min;
    int secs;
    int dayofyear;
};

// 活动时间片
struct Timeslice
{
public:
    bool Parse(string& str);

    // 时间参数
    int from_hour;
    int from_min;
    int to_hour;
    int to_min;
};

// 活动定时器类型
enum ActTimerType
{
    att_before_open,
    att_after_open,
    att_before_close,
    att_after_close,
    att_open, // 活动开启
    att_close, // 活动关闭

    // 类型数量
    att_cnt,
};

// 定时器
struct ActTimerItem
{
public:
    void Check(int actNumber, int tmFrom, int tmNow, int tmTo);
    void OnActiveChange(int actNumber, bool IsOpen);

    int actid;
    ActTimerType timer_type;
    int relative_secs;
    void* timerfun;
    string params;

    // 状态记录器
    int tmLastExe;
};

// 活动对象
class ActivityItem
{
    friend class ActivityManager;
public:
    ActivityItem()
    {
        m_dayActiveChk = false;
        m_dayActive = false;
        m_loopSecs = 0;
        m_tmLastExe = -1;
    }
    ~ActivityItem(){}
    
private:
    // 创建与检测
    bool Create(int actId, int type, string& name, string& day, string& time);
    void Check(timeInfo& info);
    void CheckBroadCast(timeInfo& info);

    // 创建定时器
    void CreateTimer(int actid, int type, int relative_secs, void* timerfun, string params);

private:
    // 判断活动配置是否合理
    bool CreateFest(int actId, int type, string& name, string& day, string& time);
    bool CreateWeek(int actId, int type, string& name, string& day, string& time);
    bool CreateDay(int actId, int type, string& name, string& day, string& time);

    // 判断活动是否开启
    void ProcessFest(timeInfo& info);
    void ProcessWeek(timeInfo& info);
    void ProcessDay(timeInfo& info);

private:
    // 判断时间是否命中
    bool IsInTime(int number, Timeslice& slice, timeInfo& info);
    bool IsInTime(Timeslice& slice, timeInfo& info);

    // 设置活动状态
    void SetActive(int number, bool isActive);

    // 重置状态
    void ResetTimer();

    // 判断时间是否在活动中
    bool IsInActive(time_t tm);

    // 活动已经开始多久
    int GetOpenedSecs( );
    
    //活动开始的时刻
    time_t getOpenTime();

private:
    int m_ActivityType;
    int m_nActId;
    string m_szActName;
    bool m_IsActive;

    vector<std::pair<int,int> > m_Dates; // 节日用
    vector<int> m_WeekDays;//周用
    vector<Timeslice> m_Times;// 日用
    vector<ActTimerItem> m_Timer; // 定时器

    // 循环广播
    vector<Timeslice> m_BroadCastTimes; // 空的时间片，表示全天执行
    vector<int> m_WeekDays_br;// 广播限定在指定天（week）进行， 且该天必须是活动天
    int m_loopSecs;// 时间间隔
    string m_broadCastContents;// 广播内容
    int m_tmLastExe;

    // 判断今天是否在活动时间内
    bool m_dayActiveChk;
    bool m_dayActive;
};

// class ActivityManager
class ActivityManager
{
    friend class Global;
    friend class globalmsg_regActivityTimer;
    friend class globalmsg_reqActivityStats;
public:
    ActivityManager();
    ~ActivityManager();
private:
    // 加载活动配置
    void LoadConfig();

    // 活动主循环
    void OnFrame(float sec);

    // 每天切换时
    void OnDailyRefresh();

public:
    // 判断活动是否开启，线程安全
    bool IsActive(int actId);

    // 安全创建定时器，可以在任何线程调用
    void SafeCreateTimer(int actid,
                     int type,
                     int relative_secs,
                     ACTTIMERFUN timerfun,
                     string params
                     );

    // 判断一个时间是否在当前活动内
    // @ 1 活动已经开启
    // @ 2 时间在当前时间片内
    bool IsActive(int actId, time_t tm);

    // 活动已经开始多久
    int GetOpenedSecs(int actId);
    
    //活动开始的时刻
    int getOpenTime(int actId);
    
    // 开启一个活动
    bool StartActiveByActID(int actID);

    // 开启和关闭
    void SetRunning(bool run){ m_Running = ( run ? 0 : -1 ); }

private:
    // 发送活动状态列表
    void SendActiveStats(int sessionid);

    // 创建定时器
    void CreateTimer(int actid,
                     int type,
                     int relative_secs,
                     void* timerfun,
                     string params
                     );

    // 加载定时器配置
    void LoadTimer();
    void LoadLoopBroadCast();

private:
    ActivityItem* m_Activities[MAX_ACT_ID + 1];
    int m_Running;
};


#define SActMgr Singleton<ActivityManager>::Instance()

#endif /* defined(__GameSrv__Activity__) */
