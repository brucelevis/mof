//  Created by Huang Kunchao on 13-8-20.
//
//  Redis Result 读取辅助

#ifndef __GameSrv__RedisResultHelper__
#define __GameSrv__RedisResultHelper__

#include <iostream>
#include "redis_helper.h"
#include "Utils.h"
#include <list>
#include <set>

//////////////////////////////////////
#pragma mark -
#pragma mark RedisHashResult

class RedisHashResult
{
    typedef std::map<string,string> ResultMap;
    typedef std::vector<string> KeyVec;
public:
    typedef ResultMap::iterator iterator;
    
    RedisHashResult(){}
    virtual ~RedisHashResult(){}
    
    void HMGET(const char* str);
    void HGETALL(const char* str);
    void HKEYS(const char* str);
    
    const string& readString(const char* key,const char* def="");
    int readInt(const char* key,int def=0);
    float readFloat(const char* key,float def=0.0f);
    
    iterator begin(){return mResults.begin();}
    iterator end(){return mResults.end();}
    int size() const {return mResults.size();}
protected:
    void clear();
    
protected:
    KeyVec          mKeys;
    ResultMap       mResults;
    string          tmp;
};

class RedisHMGET : public RedisHashResult
{
public:
    RedisHMGET(const char* str,...)
    {
        MAKE_VA_STR_DEF(str);
        HMGET(strstr.c_str());
    }
};

class RedisHGETALL : public RedisHashResult
{
public:
    RedisHGETALL(const char* str,...)
    {
        MAKE_VA_STR_DEF(str);
        HGETALL(strstr.c_str());
    }
};

class RedisHKEYS : public RedisHashResult
{
public:
    RedisHKEYS(const char* str,...)
    {
        MAKE_VA_STR_DEF(str);
        HKEYS(strstr.c_str());
    }
};



#pragma mark -
#pragma mark RedisSetResult

class RedisSetResult
{
    typedef std::set<string> ResultSet;
public:
    typedef ResultSet::iterator iterator;
    
    RedisSetResult(){}
    virtual ~RedisSetResult(){}
    
    void SMEMBERS(const char* str);
    static bool SISMEMBER(const char* str, ...);
    
    iterator begin(){return mResult.begin();}
    iterator end(){return mResult.end();}
    int size() const {return mResult.size();}
    
    void readInt(std::set<int>& out , int def = 0);
    void readFloat(std::set<float>& out,float def= 0.0f);
    
protected:
    ResultSet mResult;
};

class RedisSMEMBERS : public RedisSetResult
{
public:
    RedisSMEMBERS(const char* str,...)
    {
        MAKE_VA_STR_DEF(str);
        SMEMBERS(strstr.c_str());
    }
};

#pragma mark -
#pragma mark RedisListResult

class RedisListResult
{
    typedef std::list<std::string> Result;
public:
    typedef Result::iterator iterator;
    
    RedisListResult(){}
    virtual ~RedisListResult(){}
    
    void LRANGE(const char* str);
    
    iterator begin() {return mResult.begin();}
    iterator end(){return mResult.end();}
protected:
    Result       mResult;
};

class RedisLRANGE : public RedisListResult
{
public:
    RedisLRANGE(const char* str,...)
    {
        MAKE_VA_STR_DEF(str);
        LRANGE(strstr.c_str());
    }
};

bool isRedisKeyExist(const char* str);

#endif /* defined(__GameSrv__RedisResultHelper__) */
