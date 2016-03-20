//
//  Activity.cpp
//  GameSrv
//
//  Created by  tfl on 13-7-19.
//
//

#include "Activity.h"
#include "inifile.h"
#include "Utils.h"
#include "msg.h"
#include "main.h"
#include "Game.h"
#include "../core/BroadcastNotify.h"

#include "Global.h"
#include "GlobalMsg_def.h"

// string array
typedef std::vector<std::string> StringArray;

// 内部函数定义
int stricmp(const char* src, const char* dst);
void Trim2SideSpace(std::string& szString);
void TrimIniString(string& str);
void GetCurTimePara(timeInfo& info, time_t tm = 0);
int GetActType(string& typeName);
void TrimIniString(string& str);


///////////////////////////////////////////////////
// 字符串分割
void split_path(StringArray& paths, const char* szInput, const char* div)
{
    StringTokenizer reToken(szInput, div);
    size_t count = reToken.count();
    for (size_t i=0; i < count; i++)
    {
        paths.push_back( reToken[i] );
    }
}

int stricmp(const char* src, const char* dst)
{
#ifdef __WIN32__
    return stricmp(src, dst);
#else
    return strcasecmp(src, dst);
#endif
}

// 活动时间片
bool Timeslice::Parse(string& str)
{
    //9:30-10:30
    Trim2SideSpace(str);
    if (str.size() < 2) return false;

    if (str[0] != '(' || str[str.size() - 1] != ')') return false;

    str = str.substr(1, str.size() - 2);

    StringArray sa;
    split_path(sa, str.c_str(), "-");
    if (sa.size() != 2) return false;

    string fromtm = sa[0];
    string totm = sa[1];

    // 检测fromtm是否有效
    {
        StringArray sa2;
        split_path(sa2, fromtm.c_str(), ":");
        if (sa2.size() != 2) return false;
        from_hour = atoi(sa2[0].c_str());
        from_min = atoi(sa2[1].c_str());

        if (from_hour < 0 || from_hour > 23) return false;
        if (from_min < 0 || from_min > 59) return false;
    }
    // 检测totm是否有效
    {
        StringArray sa2;
        split_path(sa2, totm.c_str(), ":");
        if (sa2.size() != 2) return false;
        to_hour = atoi(sa2[0].c_str());
        to_min = atoi(sa2[1].c_str());

        if (to_hour < 0 || to_hour > 23) return false;
        if (to_min < 0 || to_min > 59) return false;
    }

    // 时间比较
    if ( (from_hour*60 + from_min) >= (to_hour*60 + to_min))
    {
        return false;
    }

    return true;
}

// 月, day of week, hour, min, sec
void GetCurTimePara(timeInfo& info, time_t tm)
{
    time_t now = tm == 0 ? time( NULL ) : tm;
    struct tm theTm;
    struct tm * timenow = localtime_r(&now, &theTm);

    info.year = timenow->tm_year+1900;
    info.month = timenow->tm_mon + 1;
    info.mday = timenow->tm_mday;
    info.wday = timenow->tm_wday;
    info.hour = timenow->tm_hour;
    info.min = timenow->tm_min;
    info.secs = timenow->tm_sec;
    info.dayofyear = timenow->tm_yday;

}

int GetActType(string& typeName)
{
    TrimIniString(typeName);

    // festival, week, day
    if (typeName.size() == 0) return ACT_NONE;
    if (stricmp(typeName.c_str(), "festival") == 0) return ACT_FESTIVAL;
    if (stricmp(typeName.c_str(), "week") == 0) return ACT_WEEK;
    if (stricmp(typeName.c_str(), "day") == 0) return ACT_DAY;

    return ACT_NONE;
}

void Trim2SideSpace(std::string& szString)
{
	unsigned int left_begin;
	for (left_begin = 0; left_begin < szString.size(); ++left_begin)
	{
		if (szString[left_begin] != ' ' &&
			szString[left_begin] != '\t')
		{
			break;
		}
	}

	int right_begin;
	for (right_begin = static_cast<int>(szString.size()) - 1;
		right_begin > static_cast<int>(left_begin); --right_begin)
	{
		if (szString[right_begin] != ' ' &&
			szString[right_begin] != '\t')
		{
			break;
		}
	}

	szString = szString.substr(left_begin, right_begin - left_begin + 1);
}

// 去除空格和注释
void TrimIniString(string& str)
{
    if (str.size() == 0) return;
    int n = str.rfind("//");
    if (n > 0)
    {
        str = str.substr(0, n);
    }

    Trim2SideSpace( str);
}



///////////////////////////////////////////////////////

// 活动公告
void act_notice(int actId, int number, string& params)
{
    //printf("active notice,  %d : %s\n", number, params.c_str());
    broadcastSystem( params );
}

///////////////////////////////////////////////////////

///////////////////////////////////////////////////
INSTANTIATE_SINGLETON(ActivityManager);

bool ActivityItem::Create(int actId, int type, string& name, string& day, string& time)
{
    TrimIniString(name);
    TrimIniString(day);
    TrimIniString(time);

    // 初始化
    m_ActivityType = type;
    m_nActId = actId;
    m_IsActive = false;
    m_szActName = name;

    //printf("%d, %s %s %s\n", type, name.c_str(), day.c_str(), time.c_str());

    if (type == ACT_DAY){
        return CreateDay(actId, type, name, day, time);
    }
    else if (type == ACT_FESTIVAL){
        return CreateFest(actId, type, name, day, time);
    }
    else if (type == ACT_WEEK){
        return CreateWeek(actId, type, name, day, time);
    }

    return false;
}

void ActivityItem::Check(timeInfo& info)
{
    m_dayActiveChk = false;

    if (m_ActivityType == ACT_DAY){
        ProcessDay(info);
    }
    else if (m_ActivityType == ACT_FESTIVAL){
        ProcessFest(info);
    }
    else if (m_ActivityType == ACT_WEEK){
        ProcessWeek(info);
    }

    m_dayActive = m_dayActiveChk;

    // 检测活动广播
    if (m_dayActive)
    {
        CheckBroadCast(info);
    }
}

void ActivityItem::CheckBroadCast(timeInfo& info)
{
    // 无效的活动配置
    if (m_broadCastContents.size() == 0
        || m_loopSecs < 1)
    {
        return;
    }

    // 限制执行
    if (m_WeekDays_br.size() > 0)
    {
        bool active = false;
        //
        for (size_t i=0; i < m_WeekDays_br.size(); i++)
        {
            if (m_WeekDays_br[i] == info.wday)
            {
                active = true;
                break;
            }
        }
        if (!active)
        {
            return;
        }
    }

    //当前秒数
    int now = info.hour * 3600 + info.min * 60 + info.secs;
    if (m_tmLastExe == now)
    {
        // 刚刚执行过， 不许要再执行
        return;
    }

    // 检测活动广播
    /*
    vector<Timeslice> m_BroadCastTimes; // 空的时间片，表示全天执行
    int m_loopSecs;// 时间间隔
    string m_broadCastContents;// 广播内容*/
    if (m_BroadCastTimes.size() > 0)
    {
        for (size_t i=0; i < m_BroadCastTimes.size(); i++)
        {
            // 检测时间片
            Timeslice& slice = m_BroadCastTimes[i];

            int tmFrom = slice.from_hour * 3600 + slice.from_min * 60;
            int tmTo = slice.to_hour * 3600 + slice.to_min * 60;

            if (now >= tmFrom && now < tmTo)
            {
                //检测秒
                int relativeSecs = now - tmFrom;
                if (relativeSecs % m_loopSecs == 0)
                {
                    // 广播
                    broadcastSystem(m_broadCastContents);
                    //printf("!!! %s\n", m_broadCastContents.c_str());
                    m_tmLastExe = now;
                }
            }
        }
    }
    else
    {
        if (now % m_loopSecs == 0)
        {
            // 广播
            broadcastSystem(m_broadCastContents);
            //printf("@@@ %s\n", m_broadCastContents.c_str());
            m_tmLastExe = now;
        }
    }
}

void ActivityItem::CreateTimer(
         int actid,
         int type,
         int relative_secs,
         void* timerfun,
         string params
         )
{
    if (type < 0 || type >= att_cnt) return;

    ActTimerItem item;
    item.actid = actid;
    item.timer_type = (ActTimerType)type;
    item.relative_secs = relative_secs;
    item.timerfun = timerfun;
    item.params = params;
    item.tmLastExe = -1;

    m_Timer.push_back(item);
}

// 判断活动配置是否合理
bool ActivityItem::CreateFest(int actId, int type, string& name, string& day, string& time)
{
    // 检查条件是否成立
    //day=5/1;5/2;5/3;

    // 检查日期
    StringArray sa;
    split_path(sa, day.c_str(), ";");

    for(size_t i=0; i < sa.size(); i++)
    {
        string daySlice = sa[i];

        StringArray sa2;
        split_path(sa2, daySlice.c_str(), "/");

        if (sa2.size() != 2)
        {
            printf("error date of month: %s\n", daySlice.c_str());
            return false;
        }

        int mon = atoi( sa2[0].c_str() );
        int day = atoi( sa2[1].c_str() );

        if (mon < 1 || mon > 12 || day < 1 || day > 31)
        {
            printf("error date of month: %s\n", daySlice.c_str());
            return false;
        }

        m_Dates.push_back( std::make_pair(mon,day) );
    }

    // 检测时间
    return CreateDay(actId, type, name, day, time);
}

bool ActivityItem::CreateWeek(int actId, int type, string& name, string& day, string& time)
{
    // 检查条件是否成立
    //day=1;2;3      //周一周二周三三天开展

    // 检查
    StringArray sa;
    split_path(sa, day.c_str(), ";");

    for (size_t i=0; i < sa.size(); i++)
    {
        int week = atoi( sa[i].c_str() );
        if (week < 0 || week > 6)
        {
            printf("error week of day: %s\n", sa[i].c_str());
            return false;
        }

        // 添加到容器
        m_WeekDays.push_back( week );
    }

    // 检测时间
    return CreateDay(actId, type, name, day, time);
}

bool ActivityItem::CreateDay(int actId, int type, string& name, string& day, string& time)
{
    //time=(9:30-10:30)+(16:30-17:30)

    StringArray sa;
    split_path(sa, time.c_str(), "+");

    for(size_t i=0; i < sa.size(); i++)
    {
        Timeslice slice;
        if (!slice.Parse(sa[i]))
        {
            printf("error time slice: %s\n", sa[i].c_str());
            return false;
        }

        m_Times.push_back( slice );
    }

    return true;
}

// 判断活动是否开启
void ActivityItem::ProcessFest(timeInfo& info)
{
    bool isActive = false;
    for (size_t i=0; i < m_Dates.size(); i++)
    {
        if (m_Dates[i].first == info.month && m_Dates[i].second == info.mday)
        {
            isActive = true;
            break;
        }
    }

    if (isActive)
    {
        ProcessDay(info);
    }
    else
    {
        SetActive(0, isActive);
    }
}

void ActivityItem::ProcessWeek(timeInfo& info)
{
    bool isActive = false;
    for (size_t i=0; i < m_WeekDays.size(); i++)
    {
        if (m_WeekDays[i] == info.wday)
        {
            isActive = true;
            break;
        }
    }

    if (isActive)
    {
        ProcessDay(info);
    }
    else
    {
        SetActive(0, isActive);
    }
}

void ActivityItem::ProcessDay(timeInfo& info)
{
    m_dayActiveChk = true;

    bool isActive = false;
    size_t index = 0;
    for (size_t i=0; i < m_Times.size(); i++)
    {
        if (IsInTime(i, m_Times[i], info))
        {
            // 转换状态
            index = i;
            isActive = true;
            break;
        }
    }

    SetActive(index, isActive);
}

bool ActivityItem::IsInTime(int number, Timeslice& slice, timeInfo& info)
{
    int tmFrom = slice.from_hour * 3600 + slice.from_min * 60;
    int now = info.hour * 3600 + info.min * 60 + info.secs;
    int tmTo = slice.to_hour * 3600 + slice.to_min * 60;

    // 检测定时器
    for(size_t i=0; i < m_Timer.size(); i++)
    {
        m_Timer[i].Check(number, tmFrom, now, tmTo);
    }

    // 返回
    if (now >= tmFrom && now < tmTo)
    {
        return true;
    }
    return false;
}

bool ActivityItem::IsInTime(Timeslice& slice, timeInfo& info)
{
    int tmFrom = slice.from_hour * 3600 + slice.from_min * 60;
    int now = info.hour * 3600 + info.min * 60 + info.secs;
    int tmTo = slice.to_hour * 3600 + slice.to_min * 60;

    // 返回
    if (now >= tmFrom && now < tmTo)
    {
        return true;
    }
    return false;
}

void ActivityItem::SetActive(int number, bool isActive)
{
    if (isActive == m_IsActive)
        return;

    m_IsActive = isActive;

    //printf("act:%d:%s state:%s\n", m_nActId, m_szActName.c_str(), m_IsActive ? "open":"close");

    // 检测open 和 close 消息
    for(size_t i=0; i < m_Timer.size(); i++)
    {
		m_Timer[i].OnActiveChange( number, m_IsActive );
    }

    // 通知所有玩家
    ack_update_act_stats notice;
    notice.actid = m_nActId;
    notice.isopen = isActive ? 1 : 0;
    broadcastPacket(WORLD_GROUP, &notice);
}

void ActivityItem::ResetTimer()
{
    for(size_t i=0; i < m_Timer.size(); i++)
    {
        m_Timer[i].tmLastExe = -1;
    }
    m_tmLastExe = -1;
}

bool ActivityItem::IsInActive(time_t tm)
{
    int tm1 = -1, tm2 = -1, dayofyear1 = 0, dayofyear2 = 0;
    {
        timeInfo tinfo;
        GetCurTimePara(tinfo, tm);
        dayofyear1 = tinfo.dayofyear;

        for (size_t i=0; i < m_Times.size(); i++)
        {
            if (IsInTime(m_Times[i], tinfo))
            {
                tm1 = i;
                break;
            }
        }
    }

    // 目标时间不在活动状态，返回
    if (tm1 == -1) return false;

    {
        timeInfo tinfo;
        GetCurTimePara(tinfo, 0);
        dayofyear2 = tinfo.dayofyear;

        for (size_t i=0; i < m_Times.size(); i++)
        {
            if (IsInTime(m_Times[i], tinfo))
            {
                tm2 = i;
                break;
            }
        }
    }

    // 不是同一天
    if (dayofyear1 != dayofyear2) return false;

    // 当前时间不在活动状态， 返回
    if (tm2 == -1) return false;

    return tm1 == tm2;
}

int ActivityItem::GetOpenedSecs( )
{
    // 不在活动状态
    if (!m_IsActive) return -1;

    // 查看活动在哪个分片
    timeInfo tinfo;
    GetCurTimePara(tinfo, 0);

    for (size_t i=0; i < m_Times.size(); i++)
    {
        int tmFrom = m_Times[i].from_hour * 3600 + m_Times[i].from_min * 60;
        int now = tinfo.hour * 3600 + tinfo.min * 60 + tinfo.secs;
        int tmTo = m_Times[i].to_hour * 3600 + m_Times[i].to_min * 60;

        // 返回
        if (now >= tmFrom && now < tmTo)
        {
            return now - tmFrom;
        }
    }

    return -1;
}

time_t ActivityItem::getOpenTime()
{
    // 不在活动状态
    if (!m_IsActive) return -1;
    
    // 查看活动在哪个分片
    timeInfo tinfo;
    GetCurTimePara(tinfo, 0);
    
    for (size_t i=0; i < m_Times.size(); i++)
    {
        int tmFrom = m_Times[i].from_hour * 3600 + m_Times[i].from_min * 60;
        int now = tinfo.hour * 3600 + tinfo.min * 60 + tinfo.secs;
        int tmTo = m_Times[i].to_hour * 3600 + m_Times[i].to_min * 60;
        
        // 返回
        if (now >= tmFrom && now < tmTo)
        {
            int today = Utils::mktimeFromToday(0);
            int ret = today + tmFrom;
            return ret;
        }
    }
    
    return -1;
}
///////////////////////////////
// 定时器检测
void ActTimerItem::Check(int actNumber, int tmFrom, int tmNow, int tmTo)
{
    // 一天的秒数
    static int daysecs = 24 * 3600;

    /*
    ActTimerType timer_type;
    int relative_secs;
    int exe_count;
    int interval_secs;
    void* timerfun;
    string params;*/

    int exec_tm = 0;

    // 算出执行时间
    if (timer_type == att_before_open)
    {
        exec_tm = tmFrom - relative_secs;
    }
    else if (timer_type == att_after_open)
    {
        exec_tm = tmFrom + relative_secs;
    }
    else if (timer_type == att_before_close)
    {
        exec_tm = tmTo - relative_secs;
    }
    else if (timer_type == att_after_close)
    {
        exec_tm = tmTo + relative_secs;
    }
    else return;

    // 头一天, 或下一天
    if (exec_tm < 0)
    {
        exec_tm += daysecs;
    }
    else if (exec_tm >= daysecs)
    {
        exec_tm -= daysecs;
    }

    // 检测是否该执行
    if (tmNow == exec_tm && tmLastExe != tmNow)
    {
        // 执行
        tmLastExe = tmNow;

        // 发送消息
        create_global(regActivityOnTimer, act);

        act->actid = actid;
        act->timerfun = timerfun;
        act->params = params;
        act->number = actNumber;
        
        if (strcmp(params.c_str(), "opensec") == 0) {
            act->params = Utils::makeStr("%d", SActMgr.GetOpenedSecs(actid));
        }
        
        sendGlobalMsg(Game::MQ, act);
    }
}

void ActTimerItem::OnActiveChange(int actNumber, bool IsOpen)
{
    if ( ( IsOpen && timer_type == att_open )
        || ( !IsOpen && timer_type == att_close ) )
    {
		//printf("notice act ipen %d\n ", actid);
		
        // 发送消息
        create_global(regActivityOnTimer, act);

        act->actid = actid;
        act->timerfun = timerfun;
        act->params = params;
        act->number = actNumber;

        if (strcmp(params.c_str(), "worldboss") == 0) {
            act->params = strFormat("%d", SActMgr.GetOpenedSecs(ae_world_boss));
        } else if (strcmp(params.c_str(), "guildboss") == 0) {
            act->params = Utils::makeStr("%d", SActMgr.GetOpenedSecs(ae_guild_boss));
        } else if (strcmp(params.c_str(), "guildTreasureOpenSec") == 0) {
            act->params = Utils::makeStr("%d", SActMgr.GetOpenedSecs(ae_guild_treasure_fight));
        } else if (strcmp(params.c_str(), "game_act") == 0) {
//            act->params = Utils::makeStr("%d", SActMgr.GetOpenedSecs(actid));
            act->params = Utils::makeStr("%d", SActMgr.getOpenTime(actid));
        } else if (strcmp(params.c_str(), "syncpvp_act") == 0) {
            act->params = Utils::makeStr("%d", SActMgr.getOpenTime(actid));
        }
     
        sendGlobalMsg(Game::MQ, act);
    }
}

///////////////////////////////////////////////////
ActivityManager::ActivityManager()
{
    for (size_t i=0; i < MAX_ACT_ID+1; i++)
    {
        m_Activities[i] = NULL;
    }
    m_Running = -1;
}

ActivityManager::~ActivityManager()
{
    for (size_t i=0; i < MAX_ACT_ID+1; i++)
    {
        if (m_Activities[i])
        {
            delete m_Activities[i];
            m_Activities[i] = NULL;
        }
    }
}

void ActivityManager::LoadConfig()
{
//    string activityItem = Cfgable::getFullFilePath("activity.ini");
    GameInifile ini("activity.ini");

    // 最多允许读取128个活动配置
    char secName[32];
    for (int i=0; i < MAX_ACT_ID; i++)
    {
        snprintf(secName, 32, "%d", i);
        string stat = ini.getValue(secName, "enable", "yes");

        // 活动被禁止
        if (stricmp(stat.c_str(), "yes") != 0
            && stricmp(stat.c_str(), "1") != 0
            && stricmp(stat.c_str(), "true") != 0)
        {
            continue;
        }

        string type = ini.getValue(secName,"type","");

        int actType = GetActType(type);
        if (actType == ACT_NONE)
        {
            continue;
        }

        string name = ini.getValue(secName,"name","");
        string day = ini.getValue(secName,"day","");
        string time = ini.getValue(secName,"time","");

        if ( m_Activities[i] ) delete m_Activities[i];

        m_Activities[i]= new ActivityItem;
        if (!m_Activities[i]->Create(i, actType, name, day, time))
        {
            delete m_Activities[i];
        }
    }

    // 创建活动定时器
    LoadTimer();
    LoadLoopBroadCast();

    // 发送消息
    create_global(onActivityInited, notive);
    sendGlobalMsg(Game::MQ, notive);
}

void ActivityManager::OnFrame(float sec)
{
    // 等待一秒
    if (m_Running == -1) return;
	
	// 
	m_Running ++;
	if (m_Running < 2) return;

    // 每秒执行5次
    static float timeCounter = 0.0f;
    timeCounter += sec;
    if (timeCounter < 0.20f)
    {
        return;
    }

    timeCounter = 0.0f;

    // 当前时间
    timeInfo info;
    GetCurTimePara(info);

    // 上次执行的天of年
    bool daychanges = false;
    static int lastdayofyear = -1;
    if (lastdayofyear != info.dayofyear
        && lastdayofyear != -1)
    {
        // 日期变更
        daychanges = true;
    }

    lastdayofyear = info.dayofyear;

    //printf("hb\n");
    for (size_t i=0; i < MAX_ACT_ID + 1; i++)
    {
        if (m_Activities[i])
        {
            if (daychanges)
            {
                m_Activities[i]->ResetTimer();
            }

            m_Activities[i]->Check(info);
        }
    }
}

void ActivityManager::OnDailyRefresh()
{

}

bool ActivityManager::IsActive(int actId)
{
    if (actId < 0 || actId > MAX_ACT_ID || !m_Activities[actId])
    {
        return false;
    }

    return m_Activities[actId]->m_IsActive;
}

void ActivityManager::SafeCreateTimer(int actid,
                 int type,
                 int relative_secs,
                 ACTTIMERFUN timerfun,
                 string params
                 )
{
    create_global(regActivityTimer, act);

    act->actid = actid;
    act->type = type;
    act->relative_secs = relative_secs;
    act->timerfun = (void*)timerfun;
    act->params = params;

    sendGlobalMsg(Global::MQ, act);
}

bool ActivityManager::IsActive(int actId, time_t tm)
{
    if (actId < 0 || actId > MAX_ACT_ID || !m_Activities[actId])
    {
        return false;
    }

    return m_Activities[actId]->IsInActive(tm);
}


bool ActivityManager::StartActiveByActID(int actId)
{
	
    if (actId < 0 || actId > MAX_ACT_ID )
    {
        
        return false;
    }

	if (!m_Activities[actId]) {
            m_Activities[actId]= new ActivityItem;
    }
    
    m_Activities[actId]->SetActive(0,true);
    return true;
}

int ActivityManager::GetOpenedSecs(int actId)
{
    if (actId < 0 || actId > MAX_ACT_ID || !m_Activities[actId])
    {
        return -1;
    }

    return m_Activities[actId]->GetOpenedSecs();
}

int ActivityManager::getOpenTime(int actId)
{
    if (actId < 0 || actId > MAX_ACT_ID || !m_Activities[actId])
    {
        return -1;
    }
    
    return m_Activities[actId]->getOpenTime();
}

void ActivityManager::SendActiveStats(int sessionid)
{
    ack_get_act_stats req;
    req.servertime = time( NULL );
    obj_actstat stat;

    for (size_t i=0; i < MAX_ACT_ID + 1; i++)
    {
        if (m_Activities[i])
        {
            stat.actid = i;
            stat.isopen = m_Activities[i]->m_IsActive ? 1 : 0;
            req.stats.push_back(stat);
        }
    }

    sendNetPacket(sessionid, &req);
}

void ActivityManager::CreateTimer(int actid,
                 int type,
                 int relative_secs,
                 void* timerfun,
                 string params
                 )
{
    if (!timerfun) return;

    for (size_t i=0; i < MAX_ACT_ID + 1; i++)
    {
        if (m_Activities[i] && i == actid)
        {
            m_Activities[i]->CreateTimer(i, type, relative_secs, timerfun, params);
        }
    }
}

void ActivityManager::LoadTimer()
{
//    string activityItem = Cfgable::getFullFilePath("activitynotice.ini");
    GameInifile ini("activitynotice.ini");

    // 最多允许读取1024个活动配置
    /*
    actId=1
    type=open
    secs=
    notice=1号活动已经开始*/
    char secName[32];
    for (int i=1; i < 512; i++)
    {
        snprintf(secName, 32, "%d", i);
        int actId = atoi( ini.getValue(secName, "actId", "").c_str() );
        string type = ini.getValue(secName, "type", "");
        int secs = atoi( ini.getValue(secName, "secs", "").c_str() );
        string notice = ini.getValue(secName, "notice", "");

        // 活动id 超出范围
        if (actId < 1 || actId > MAX_ACT_ID) continue;

        // 时间配置有错误
        if (secs < 0) continue;

        ActTimerType att = att_cnt;

        if (type == "open") att = att_open;
        else if (type == "close") att = att_close;
        else if (type == "afteropen") att = att_after_open;
        else if (type == "beforeopen") att = att_before_open;
        else if (type == "afterclose") att = att_after_close;
        else if (type == "beforeclose") att = att_before_close;
        else continue;

        // 执行定时器
        CreateTimer(actId, att, secs, (void*)act_notice, notice);
    }
}

void ActivityManager::LoadLoopBroadCast()
{
//    string activityItem = Cfgable::getFullFilePath("activitybroadcast.ini");
    GameInifile ini("activitybroadcast.ini");
    
    //string activityMlang = Cfgable::getFullFilePath("activitybroadcastMultiLanguage.ini");
    //IniFile iniml(activityMlang.c_str());

    // 最多允许读取1024个活动配置
    /*
        actId=2
        timeslice=(20:30-22:00)
        secs=900
        notice=夜幕降临《世界BOSS》暗之龙重生，团结起来一起抵抗它的入侵吧！
    */

    char secName[32];
    for (int i=1; i < 256; i++)
    {
        snprintf(secName, 32, "%d", i);
        int actId = atoi( ini.getValue(secName, "actId", "").c_str() );
        string timeslice = ini.getValue(secName, "timeslice", "");
        string day = ini.getValue(secName,"day","");
        int secs = atoi( ini.getValue(secName, "secs", "").c_str() );
        string notice = ini.getValue(secName, "notice", "");
        //string noticeid = ini.getValue(secName, "notice_id", "");
        //string notice = iniml.getValue(noticeid, "desc");

        // 活动id 超出范围
        if (actId < 0 || actId > MAX_ACT_ID || !m_Activities[actId])
        {
            continue;
        }

        // 时间配置有错误
        if (secs <= 0) continue;

        ActivityItem& avi = *m_Activities[actId];

        // 天限制
        if (day.size() > 0)
        {
            StringArray sa;
            split_path(sa, day.c_str(), ";");

            for (size_t i=0; i < sa.size(); i++)
            {
                int week = atoi( sa[i].c_str() );
                if (week < 0 || week > 6)
                {
                    continue;
                }
                // 添加到容器
                avi.m_WeekDays_br.push_back( week );
            }
        }

        // 添加这个广播
        StringArray sa;
        split_path(sa, timeslice.c_str(), "+");

        for(size_t ii=0; ii < sa.size(); ii++)
        {
            Timeslice slice;
            if (!slice.Parse(sa[ii]))
            {
                printf("activitybroadcast.ini: error time slice: %s\n", sa[ii].c_str());
                continue;
            }

            avi.m_BroadCastTimes.push_back( slice );
        }

        avi.m_loopSecs = secs;// 时间间隔
        avi.m_broadCastContents = notice;// 广播内容
    }
}
