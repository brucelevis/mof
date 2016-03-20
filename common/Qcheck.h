//
//  Qcheck.h
//  GoldOL
//
//  Created by cxy on 12-11-29.
//  快速排错 debug 定位 release 版本保留代码
//

#ifndef GoldOL_Qcheck_h
#define GoldOL_Qcheck_h
#include <assert.h>
#if DEBUG
#define Qcheck(code) assert(code)
#define Qassert(code) assert(code)
#else
#define Qcheck(code) (code)
#define Qassert(code) 
#endif

#endif
