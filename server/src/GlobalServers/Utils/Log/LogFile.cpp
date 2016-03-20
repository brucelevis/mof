/*------------------------------------------------------------------------------
  created:	2006/07/27 15:28
  filename: LogFile.cpp
  author:	tfl
  purpose:
--------------------------------------------------------------------------------
  Copyright (C) 2006 - All Rights Reserved
------------------------------------------------------------------------------*/
#include <time.h>
#include <stdio.h>
#ifdef LINUX_
#include <stdarg.h>
#include "../FileSys/SearchOps.h"
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <windows.h>
#endif // LINUX_
#include "../ThreadPool/ThreadPool.h"
#include "LogFile.h"
#pragma warning(disable:4996)

#ifdef LINUX_
long g_hConsole = NULL;
#else
HANDLE g_hConsole = NULL;
#endif

CriticalObject g_consoleLock;

void SetConsoleColor(int color)
{
    #ifdef LINUX_
    #else
	if (g_hConsole == NULL)
	{
		g_hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	// 使用亮色
	SetConsoleTextAttribute(g_hConsole, FOREGROUND_INTENSITY | color);
	#endif
}

struct tm* GetTmst()
{
	// 计算完成时间
	time_t curTime;
	::time(&curTime);

	struct tm* tmst = localtime(&curTime);
	return tmst;
}

InitLog::InitLog()
{
	m_hFile = NULL;

	struct tm* tmst = GetTmst();

	if (tmst)
	{
		char filename[128];
		#ifdef LINUX_
		sprintf(
        #else
		sprintf_s(
        #endif
            filename,
			"log/%04d-%02d-%02d.txt",
			tmst->tm_year + 1900,
			tmst->tm_mon+1,
			tmst->tm_mday);

		m_hFile = fopen(filename, "a+");

		if (!m_hFile)
		{
			CreateDirectory("log", NULL);
			chmod("log", 0755);
			m_hFile = fopen(filename, "a+");
		}
	}
}
InitLog::~InitLog()
{
	if (m_hFile)
	{
		fclose(m_hFile);
		m_hFile = NULL;
	}
}

char g_LogBuffer[8192];
void Log(LogLevel level, const char *Format, ...)
{
	if (!Format)
	{
		return;
	}

	// 锁定控制台
	g_consoleLock.Lock();

	InitLog InitLog;
	if (InitLog.m_hFile)
	{
		va_list args;
		va_start(args,Format);
		#ifdef LINUX_
		_vsnprintf(g_LogBuffer, sizeof(g_LogBuffer) - 4, Format, args);
		#else
		_vsnprintf_s(g_LogBuffer, sizeof(g_LogBuffer) - 4, Format, args);
		#endif
		va_end(args);

		struct tm* tmst = GetTmst();
		if (tmst)
		{
			if (level == LOG_ERROR)
			{
				fprintf(InitLog.m_hFile, "[%02d:%02d:%02d] [ErrorMessage] %s\n",
					tmst->tm_hour, tmst->tm_min, tmst->tm_sec, g_LogBuffer);
			}
			else
			{
				fprintf(InitLog.m_hFile, "[%02d:%02d:%02d] %s\n",
					tmst->tm_hour, tmst->tm_min, tmst->tm_sec, g_LogBuffer);
			}

            #ifdef LINUX_
            #else
			SetConsoleTextAttribute(g_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			#endif

			printf("[%02d-%02d %02d:%02d:%02d] ",
				tmst->tm_mon+1,
				tmst->tm_mday,
				tmst->tm_hour,
				tmst->tm_min,
				tmst->tm_sec);

            #ifdef LINUX_
            #else
			if (level == LOG_NORMAL)
			{
				SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			}
			else if (level == LOG_ERROR)
			{
				SetConsoleColor(FOREGROUND_RED);
			}
			else if (level == LOG_NOTICE)
			{
				SetConsoleColor(FOREGROUND_GREEN);
			}
			#endif

			printf("%s\n", g_LogBuffer);

            #ifdef LINUX_
            #else
			SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			#endif
		}
	}

	// 释放控制台
	g_consoleLock.Unlock();
}

void _Debug(const char *Format, ...)
{
#ifndef _DEBUG
	return;
#endif

	if (!Format)
	{
		return;
	}

	// 锁定控制台
	g_consoleLock.Lock();

	InitLog InitLog;
	if (InitLog.m_hFile)
	{
		va_list args;
		va_start(args,Format);
		#ifdef LINUX_
		_vsnprintf(g_LogBuffer, sizeof(g_LogBuffer) - 4, Format, args);
		#else
		_vsnprintf_s(g_LogBuffer, sizeof(g_LogBuffer) - 4, Format, args);
		#endif
		va_end(args);

		struct tm* tmst = GetTmst();
		if (tmst)
		{
			fprintf(InitLog.m_hFile, "[%02d:%02d:%02d] %s\n",
				tmst->tm_hour, tmst->tm_min, tmst->tm_sec, g_LogBuffer);

            #ifdef LINUX_
            #else
			SetConsoleTextAttribute(g_hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			#endif

			printf("[%02d-%02d %02d:%02d:%02d] ",
				tmst->tm_mon+1,
				tmst->tm_mday,
				tmst->tm_hour,
				tmst->tm_min,
				tmst->tm_sec);

            #ifdef LINUX_
            #else
			SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			#endif

			printf("%s\n", g_LogBuffer);
		}
	}

	// 释放控制台
	g_consoleLock.Unlock();
}
