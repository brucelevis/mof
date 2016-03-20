
#include <iostream>

#define RedisNameTableBegin() static const char* s_propname[] = {

#define RedisNameDef(name, value)  #value,

#define RedisNameTableEnd()  };

#include "RedisKeyName.h"

const char* g_GetRedisName(int type)
{
    if (type >= sizeof(s_propname) / sizeof(const char*) || type < 0)
    {
        return NULL;
    }
    
    return s_propname[type];
}
