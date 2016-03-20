//
//  RedisResultHelper.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-8-20.
//
//

#include "RedisResultHelper.h"
#include "main.h"

//////////////////////////////////////
#pragma mark -
#pragma mark RedisHashResult Implment


void RedisHashResult::HMGET(const char* str)
{
    assert(str);
    clear();
    
    StringTokenizer token(str," ",StringTokenizer::TOK_TRIM);
    
    if(token.count() < 3)
    {
        throw "HMGET param number error";
        return;
    }
    
    for (int i = 2; i < token.count(); ++i) {
        mKeys.push_back(token[i]);
    }
    
    RedisResult result(redisCmd(str));
    
    for (int i = 0; i < result.getHashElements(); i++)
    {
        string key = mKeys[i];
        string val = result.readHash(i, "");
        mResults[key] = val;
    }
}

void RedisHashResult::HGETALL(const char* str)
{
    assert(str);
    clear();
    
    StringTokenizer token(str," ",StringTokenizer::TOK_TRIM);
    
    if(token.count() != 2)
    {
        throw "HGETALL param number error";
        return;
    }
    
    RedisResult result(redisCmd(str));
    
    for (int i = 0 , ni = result.getHashElements(); i < ni; i += 2)
    {
        string key = result.readHash(i, "");
        string val = result.readHash(i+1, "");
        mResults[key] = val;
    }
}

void RedisHashResult::HKEYS(const char* str)
{
    assert(str);
    clear();
    StringTokenizer token(str," ",StringTokenizer::TOK_TRIM);
    if(token.count() != 2)
    {
        throw "HKEYS param number error";
        return;
    }
    
    RedisResult result(redisCmd(str));
    
    for (int i = 0 , ni = result.getHashElements(); i < ni; ++i)
    {
        string key = result.readHash(i, "");
        mResults[key] = key;
    }
}

const string& RedisHashResult::readString(const char* key,const char* def)
{
    iterator iter = mResults.find(key);
    if(iter == mResults.end())
    {
        tmp = def;
        return tmp;
    }
    return iter->second;
}

int RedisHashResult::readInt(const char* key,int def)
{
    iterator iter = mResults.find(key);
    if( iter == mResults.end())
        return def;
    return Utils::safe_atoi(iter->second.c_str(),def);
}

float RedisHashResult::readFloat(const char* key,float def)
{
    iterator iter = mResults.find(key);
    if( iter == mResults.end())
        return def;
    
    return atof(iter->second.c_str());
}

void RedisHashResult::clear()
{
    mKeys.clear();
    mResults.clear();
    tmp.clear();
}

////////////////////////////////
#pragma mark -
#pragma mark RedisSetResult Implement

void RedisSetResult::SMEMBERS(const char* str)
{
    assert(str);
    mResult.clear();
    
    StringTokenizer token(str," ",StringTokenizer::TOK_TRIM);
    
    if(token.count() != 2)
    {
        throw "SMEMBERS param number error";
        return;
    }
    
    RedisResult result(redisCmd(str));
    
    for (int i = 0 , ni = result.getHashElements(); i < ni; ++i)
    {
        string val = result.readHash(i, "");
        mResult.insert(val);
    }
}

bool RedisSetResult::SISMEMBER(const char* str, ...)
{
    MAKE_VA_STR_DEF(str);
    
    StringTokenizer token(strstr," ",StringTokenizer::TOK_TRIM);
    
    if(token.count() != 3)
    {
        throw "SISMEMBER param number error";
        return false;
    }
    
    RedisResult result(redisCmd(strstr.c_str()));
    return (result.readInt() == 1);
}

void RedisSetResult::readInt(std::set<int>& out , int def )
{
    for (iterator it = begin(); it != end(); ++it) {
        out.insert(Utils::safe_atoi(it->c_str(),def));
    }
}

void RedisSetResult::readFloat(std::set<float>& out,float def)
{
    for (iterator it = begin(); it != end(); ++it) {
        out.insert( atof(it->c_str()));
    }
}

#pragma mark -
#pragma mark Redis List Result Implement

void RedisListResult::LRANGE(const char* str)
{
    assert(str);
    mResult.clear();
    
    StringTokenizer token(str," ",StringTokenizer::TOK_TRIM);
    
    if(token.count() != 4)
    {
        throw "LRANGE param number error";
        return;
    }
    
    RedisResult result(redisCmd(str));
    
    for (int i = 0 , ni = result.getHashElements(); i < ni; ++i)
    {
        mResult.push_back( result.readHash(i,"") );
    }
}

bool isRedisKeyExist(const char* str)
{
    if(NULL == str)
        return false;
    
    RedisResult result(redisCmd("EXISTS %s",str));
    return result.readInt() == 1;
}
