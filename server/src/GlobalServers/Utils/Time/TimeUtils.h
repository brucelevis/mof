/*-------------------------------------------------------------------------
	created:	2010/08/27  16:48
	filename: 	e:\Project_SVN\Server\Utils\Time\TimeUtils.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#pragma once

#include <time.h>
#ifdef LINUX_
#include <signal.h>
#include <sys/types.h>
#define LONGLONG    long long
typedef union _LARGE_INTEGER {
    struct {
        unsigned long LowPart;
        long  HighPart;
    };
    long long QuadPart;
} LARGE_INTEGER;
#else
#include <windows.h>
#endif
#include <string>
using namespace std;

#include <sys/time.h>

// 时间转换函数
extern string Time_tToString(time_t time);
extern time_t sscanftime(const char* ptszTime);

// 获取当前时间
extern string GetCurTimeStr();

// 初始化随机函数
extern void InitRandomFunc();

/*
    struct timeval tpstart,tpend;
    float timeuse;

    gettimeofday(&tpstart,NULL);

	g_Dispatcher.OnCommand(pScene, sender, self, msgType, pmsg);

    gettimeofday(&tpend,NULL);
    timeuse=1000000*(tpend.tv_sec-tpstart.tv_sec)+tpend.tv_usec-tpstart.tv_usec;
    timeuse/=1000;
*/

// 计时器
class TimeCounter
{
public:
	TimeCounter(bool begin = true);
	~TimeCounter(){ }
public:
	void Recount();
	float GetCounter();
	void PrintCounter(const char* strval = "", int nval = 0);

private:
    timeval tpstart;
    timeval tpend;

	float m_result;
	bool m_bHasResult;
};

inline TimeCounter::TimeCounter( bool begin ) : m_result(0.0f), m_bHasResult(false)
{
    gettimeofday(&tpstart,NULL);
    if (begin){ Recount(); }
}

inline void TimeCounter::Recount()
{
    m_result = 0.0f;
	m_bHasResult = false;
	gettimeofday(&tpstart,NULL);
}

inline float TimeCounter::GetCounter()
{
	if (m_bHasResult){ return m_result; }

	gettimeofday(&tpend,NULL);

    m_result = 1000000*(tpend.tv_sec-tpstart.tv_sec) + tpend.tv_usec-tpstart.tv_usec;
    m_result /= 1000;

	m_bHasResult = true;
	return m_result;
}

inline void TimeCounter::PrintCounter(const char* strval, int nval)
{
    if (!strval) strval = "";
    printf("%s[ %d ] %f ms\n", strval, nval, GetCounter());
}

