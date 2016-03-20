//
//  RedisKeyTable.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-8-19.
//


#include <iostream>
#include <cassert>
#include <string.h>
#include "RedisHash.h"
#include "RedisResultHelper.h"
#include "main.h"
#include "redis_helper.h"


//////////////////////////////////////
//init
#define begin_redis_hash_class(name,dbname)                         \
                                                                    \
name::name()                                                        \
{                                                                   \
    mDbname = g_GetRedisName(dbname);                               \
    mId = 0;                                                   \
    mActive = false;

#define redis_hash_int(name,dbname)        name.setName(#dbname);
#define redis_hash_float(name,dbname)      name.setName(#dbname);
#define redis_hash_double(name,dbname)     name.setName(#dbname);
#define redis_hash_string(name,dbname)     name.setName(#dbname);
#define redis_hash_obj(type,name,dbname)   name.setName(#dbname);

#define end_redis_hash_class()                                      \
};

#include "RedisHashDefine.h"

#undef begin_redis_hash_class
#undef end_redis_hash_class

#undef redis_hash_int
#undef redis_hash_float
#undef redis_hash_double
#undef redis_hash_string
#undef redis_hash_obj

//////////////////////////////////////
//load
#define begin_redis_hash_class(name,dbname)                         \
                                                                    \
void name::load(int id)                                       \
{                                                                   \
    active(id);                                            \
    stringstream ss;                                                \
    ss<<"HMGET "<<dbkey()<<" " <<joinAllKeyString();  \
    RedisHMGET result(ss.str().c_str());                            \
    RedisHMGET::iterator iter ;                                     \
    for( iter = result.begin(); iter != result.end() ; ++iter)      \
    {                                                               \
        const string& key = iter->first;                            \
        const string& val = iter->second;                           \

#define redis_hash_int(name,dbname)                                 \
        if( strcmp( key.c_str(), name.getName() ) == 0 )            \
            name.load(val.c_str());

#define redis_hash_float(name,dbname)                               \
        if( strcmp( key.c_str(), name.getName() ) == 0 )            \
            name.load(val.c_str());

#define redis_hash_double(name,dbname)                               \
        if( strcmp( key.c_str(), name.getName() ) == 0 )            \
            name.load(val.c_str());

#define redis_hash_string(name,dbname)                              \
        if( strcmp( key.c_str(), name.getName() ) == 0 )            \
            name.load(val.c_str());

#define redis_hash_obj(type,name,dbname)                            \
        if( strcmp( key.c_str(), name.getName() ) == 0 )            \
            name.load(val.c_str());

#define end_redis_hash_class()                                  \
    }                                                               \
}

#include "RedisHashDefine.h"

#undef begin_redis_hash_class
#undef end_redis_hash_class

#undef redis_hash_int
#undef redis_hash_float
#undef redis_hash_double
#undef redis_hash_string
#undef redis_hash_obj

//////////////////////////////////////
//save
#define begin_redis_hash_class(name,dbname)                             \
                                                                        \
void name::save()                                                 \
{                                                                       \
    if( ! mActive )                                                 \
        return;                                                         \
                                                                        \
    RedisArgs args;                                                     \
    RedisHelper::beginHmset(args,dbkey().c_str());

#define redis_hash_int(name,dbname)                                     \
    if( name.dirty() )                                                  \
    {                                                                   \
        RedisHelper::appendHmset(args,name.getName(),name.str().c_str());\
        name.dirty(false);                                              \
    }

#define redis_hash_float(name,dbname)                                   \
    if( name.dirty() )                                                  \
    {                                                                   \
        RedisHelper::appendHmset(args,name.getName(),name.str().c_str());\
        name.dirty(false);                                              \
    }

#define redis_hash_double(name,dbname)                                   \
    if( name.dirty() )                                                  \
    {                                                                   \
        RedisHelper::appendHmset(args,name.getName(),name.str().c_str());\
        name.dirty(false);                                              \
    }

#define redis_hash_string(name,dbname)                                  \
    if( name.dirty() )                                                  \
    {                                                                   \
        RedisHelper::appendHmset(args,name.getName(),name.str().c_str());\
        name.dirty(false);                                              \
    }

#define redis_hash_obj(type,name,dbname)                                \
    if( name.dirty() )                                                  \
    {                                                                   \
        RedisHelper::appendHmset(args,name.getName(),name.str().c_str());\
        name.dirty(false);                                              \
    }

#define end_redis_hash_class()                                      \
    if(args.size() <= 2){ return;}                                  \
    RedisHelper::commitHmset(get_DbContext(), args);                \
}

#include "RedisHashDefine.h"

#undef begin_redis_hash_class
#undef end_redis_hash_class

#undef redis_hash_int
#undef redis_hash_float
#undef redis_hash_double
#undef redis_hash_string
#undef redis_hash_obj

///////////////////////////////////////
//deleteFromDatabase
#define begin_redis_hash_class(name,dbname)                             \
                                                                        \
void name::deleteFromDatabase(){                                        \
    if(mActive)                                                         \
        doRedisCmd("DEL %s",dbkey().c_str());                           \
}

#define redis_hash_int(name,dbname)
#define redis_hash_float(name,dbname)
#define redis_hash_double(name,dbname)
#define redis_hash_string(name,dbname)
#define redis_hash_obj(type,name,dbname)

#define end_redis_hash_class()

#include "RedisHashDefine.h"

#undef begin_redis_hash_class
#undef end_redis_hash_class

#undef redis_hash_int
#undef redis_hash_float
#undef redis_hash_double
#undef redis_hash_string
#undef redis_hash_obj

//////////////////////////////////////
//dbkey
#define begin_redis_hash_class(name,dbname)                             \
                                                                        \
string name::dbkey(){                                        \
    stringstream ss;                                                    \
    ss<< mDbname<<":"<<mId;                               \
    return ss.str();                                                \
}

#define redis_hash_int(name,dbname)
#define redis_hash_float(name,dbname)
#define redis_hash_double(name,dbname)
#define redis_hash_string(name,dbname)
#define redis_hash_obj(type,name,dbname)  

#define end_redis_hash_class()  

#include "RedisHashDefine.h"

#undef begin_redis_hash_class
#undef end_redis_hash_class

#undef redis_hash_int
#undef redis_hash_float
#undef redis_hash_double
#undef redis_hash_string
#undef redis_hash_obj

//////////////////////////////////////
//joinAllKeyString
#define begin_redis_hash_class(name,dbname)                             \
                                                                        \
string name::joinAllKeyString()                              \
{                                                                       \
    stringstream ss;

#define redis_hash_int(name,dbname)   ss<<name.getName()<<" ";
#define redis_hash_float(name,dbname) ss<<name.getName()<<" ";
#define redis_hash_double(name,dbname) ss<<name.getName()<<" ";
#define redis_hash_string(name,dbname) ss<<name.getName()<<" ";
#define redis_hash_obj(type,name,dbname) ss<<name.getName()<<" ";

#define end_redis_hash_class()                                        \
    return ss.str();                                              \
}
    
#include "RedisHashDefine.h"

#undef begin_redis_hash_class
#undef end_redis_hash_class

#undef redis_hash_int
#undef redis_hash_float
#undef redis_hash_double
#undef redis_hash_string
#undef redis_hash_obj


