
#ifndef _MACROS_H
#define _MACROS_H

#pragma warning(disable: 4786)
#pragma warning(disable: 4996)

#define SAFE_DELETE(p)       { if(p) { ::delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { ::delete[] (p);   (p)=NULL; } }

#define WORD1(p)		(p & 0xFFFF)
#define WORD2(p)		((p >> 16) & 0xFFFF)
#define BYTE1(p)		(p & 0xFF)
#define BYTE2(p)		((p >> 8) & 0xFF)
#define BYTE3(p)		((p >> 16) & 0xFF)
#define BYTE4(p)		((p >> 24) & 0xFF)

typedef unsigned char	UI08;
typedef unsigned short	UI16;
typedef unsigned int	UI32;
typedef unsigned int	DWORD;
#ifdef LINUX_
#define __int64 long long
#include <stddef.h>
#define UINT    unsigned int
#define CP_ACP  0
#define CP_UTF8 65001
#endif
typedef unsigned __int64	UI64;

typedef char			SI08;
typedef short			SI16;
typedef int				SI32;
typedef __int64			SI64;

// SOCKET¾ä±ú
typedef size_t			SOCK_HANDLE;

#define	UI32_MAX		0xFFFFFFFF
#define UI16_MAX		0xFFFF
#define UI08_MAX		0xFF

#include "Debug.h"

#endif // _MACROS_H

