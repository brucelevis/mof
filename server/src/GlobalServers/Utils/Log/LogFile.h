/*------------------------------------------------------------------------------
  created:	2006/07/27 15:28
  filename: LogFile.h
  author:	tfl
  purpose:
--------------------------------------------------------------------------------
  Copyright (C) 2006 - All Rights Reserved
------------------------------------------------------------------------------*/
#pragma once

#define LogFilePos()\
	Log(LOG_ERROR, "%s:%d", __FILE__, __LINE__);

#ifdef LINUX_
extern long g_hConsole;
#else
extern HANDLE g_hConsole;
#endif
extern void SetConsoleColor(int color);

enum LogLevel
{
	LOG_NORMAL,
	LOG_ERROR,
	LOG_NOTICE,
};

class InitLog
{
public:
	InitLog();
	~InitLog();
	FILE* m_hFile;
};

extern void Log(LogLevel level, const char *Format, ...);
extern void _Debug(const char *Format, ...);
