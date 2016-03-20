
#ifndef _DEBUG_H
#define _DEBUG_H

#ifndef LINUX_
#include <crtdbg.h>
#endif
#include <stddef.h>


#ifdef LINUX_
#define NEW ::new
#else
#ifndef NDEBUG
#define NEW ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW ::new
#endif // NDEBUG
#endif  // LINUX_

#ifndef NDEBUG
	void _AssertionFail(const char * strFile, int nLine);
	#define Assert(p)	((p) ? (void)0 : (void)_AssertionFail(__FILE__, __LINE__))
#else
	#define Assert(p)	(void(0))
#endif

#endif // _DEBUG_H
