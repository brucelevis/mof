//
//  Utf8Matcher.h
//  GameSrv
//
//  Created by prcv on 14-5-9.
//
//

#ifndef __GameSrv__Utf8Matcher__
#define __GameSrv__Utf8Matcher__

#include "utf8.h"

#include <stdint.h>
#include <set>
#include <assert.h>
#include <string>

#include "basedef.h"

using namespace std;

class Utf8Matcher
{
public:
    set<uint32_t> mFilters;
    
    void appendMatchString(const string& str);
    
    bool match(const string& str, bool& isUtf8);
};



#endif /* defined(__GameSrv__Utf8Matcher__) */
