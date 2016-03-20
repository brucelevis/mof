
#ifndef _TRACE_H
#define _TRACE_H

bool __cdecl _AbortIgnore(const char * info);
void __cdecl _TraceInfo(const char * info, ...);
void __cdecl _EchoInfo(const char * info, ...);
void __cdecl LogW(const wchar_t * info);

#ifdef _ERROR_HALT
#include <stdlib.h>
extern bool g_bErrorHalt;
#define ERROR_HALT(p) { if (g_bErrorHalt) _AbortIgnore(p); }
#else
#define ERROR_HALT(p) (void(0))
#endif // _ERROR_HALT

#ifndef NDEBUG

#define LogMessage(p)	\
	_EchoInfo("Message, %s, %d, %s", __FILE__, __LINE__, p); \
	_TraceInfo("Message, %s, %d, %s", __FILE__, __LINE__, p)
#define LogError(p)		\
	_EchoInfo("Error, %s, %d, %s", __FILE__, __LINE__, p); \
	_TraceInfo("Error, %s, %d, %s", __FILE__, __LINE__, p); ERROR_HALT(p)
#define LogWarning(p)	\
	_EchoInfo("Warning, %s, %d, %s", __FILE__, __LINE__, p); \
	_TraceInfo("Warning, %s, %d, %s", __FILE__, __LINE__, p)

#else

#define LogMessage(p)	_TraceInfo("Message, %s", p)
#define LogError(p)		_TraceInfo("Error, %s", p)
#define LogWarning(p)	_TraceInfo("Warning, %s", p)

#endif	// NDEBUG

#endif // _TRACE_H
