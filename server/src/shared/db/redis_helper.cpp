//
//  redis_helper.cpp
//  GameSrv
//
//  Created by prcv on 13-5-2.
//
//

#include "redis_helper.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

RedisArgs::RedisArgs()
{

}

RedisArgs::~RedisArgs()
{
    for (int i = 0; i < args.size(); i++)
    {
        delete [] args[i];
    }
}

void RedisArgs::add(int val)
{
    char *buf = new char[32];
    sprintf(buf, "%d", val);
    args.push_back(buf);
}

void RedisArgs::add(int64_t val)
{
    char *buf = new char[64];
    sprintf(buf, "%lld", val);
    args.push_back(buf);
}

void RedisArgs::add(const char* val)
{
    int len = strlen(val);
    char* buf = new char[len + 1];
    memcpy(buf, val, len + 1);
    args.push_back(buf);
}

void RedisArgs::add(float val)
{
    char *buf = new char[32];
    sprintf(buf, "%f", val);
    args.push_back(buf);
}

void RedisArgs::clear()
{
    for (int i = 0; i < args.size(); i++)
    {
        delete [] args[i];
    }
    args.clear();
}

redisContext* RedisHelper::createContext(const char* host, unsigned short port, const char* pwd, int index)
{
    redisContext* redisContex = NULL;
    bool ret = false;
    do
    {
        //connect db
        struct timeval timeout = {5, 0};
        redisContex = redisConnectWithTimeout(host, port, timeout);
        if (redisContex->err)
        {
            log_error("please start redis db (error: "<< redisContex->errstr<<" )\n");
            break;
        }
        
        //auth db
        if (pwd && *pwd)
        {
            RedisResult result((redisReply*)redisCommand(redisContex, "auth %s", pwd));
            if (!result.isStatusSuccess())
            {
                log_error("db password error");
                break;
            }
        }
        
        RedisResult result((redisReply*)redisCommand(redisContex,"select %d", index));
        if (!result.isStatusSuccess()){
            break;
        }
        
        ret = true;
    }
    while (0);
    
    if (!ret){
        redisFree(redisContex);
        redisContex = NULL;
    }
    
    return redisContex;
}

void RedisHelper::destroyContext(redisContext* context) 
{
    redisFree(context);
}

bool RedisHelper::set(redisContext* ctx, const char* key, const char* val)
{
    int ret = 1;
    redisReply* reply = (redisReply*)redisCommand(ctx, "set %s %s", key, val);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        ret = strcasecmp("OK", reply->str);
    }
    freeReplyObject(reply);

    return ret == 0;
}

bool RedisHelper::set(redisContext* ctx, const char* key, int val)
{
    assert(key && ctx);
    
    int ret = 1;
    redisReply* reply = (redisReply*)redisCommand(ctx, "set %s %d", key, val);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        ret = strcasecmp("OK", reply->str);
    }
    freeReplyObject(reply);

    return ret == 0;
}

std::string RedisHelper::getStr(redisContext* ctx, const char* key, const char* def)
{
    std::string ret = def;
    redisReply* reply = (redisReply*)redisCommand(ctx,  "get %s", key);
    if (reply->type == REDIS_REPLY_STRING && reply->str != NULL)
    {
        ret = reply->str;
    }
    freeReplyObject(reply);

    return ret;
}

int RedisHelper::getInt(redisContext* ctx, const char* key, int def)
{
    int ret = def;
    redisReply* reply = (redisReply*)redisCommand(ctx,  "get %s", key);
    if (reply->type == REDIS_REPLY_STRING && reply->str != NULL)
    {
        ret = atoi(reply->str);
    }
    freeReplyObject(reply);

    return ret;
}

long RedisHelper::getInteger(redisContext* ctx, const char* cmd, int def)
{
    int ret = def;

    redisReply* reply = (redisReply*)redisCommand(ctx, cmd);
    if (reply->type == REDIS_REPLY_INTEGER)
    {
        ret = reply->integer;
    }
    freeReplyObject(reply);

    return ret;
}

std::string RedisHelper::hgetStr(redisContext* ctx, const char* key, const char* hkey, const char* def)
{
    std::string ret = def;
    redisReply* reply = (redisReply*)redisCommand(ctx, "hget %s %s", key, hkey);
    if (reply->type == REDIS_REPLY_STRING && reply->str != NULL)
    {
        ret = reply->str;
    }
    freeReplyObject(reply);

    return ret;
}

int RedisHelper::hgetInt(redisContext* ctx, const char* key, const char* hkey, int def)
{
    int ret = def;
    redisReply* reply = (redisReply*)redisCommand(ctx, "hget %s %s", key, hkey);
    if (reply->type == REDIS_REPLY_STRING && reply->str != NULL)
    {
        ret = atoi(reply->str);
    }
    freeReplyObject(reply);

    return ret;

}

bool RedisHelper::hsetInt(redisContext* ctx, const char* key, const char* hkey, int val)
{
    int ret = 1;
    redisReply* reply = (redisReply*)redisCommand(ctx, "hset %s %s %d", key, hkey, val);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        ret = strcasecmp(reply->str, "OK");
    }
    freeReplyObject(reply);

    return ret == 0;
}

bool RedisHelper::hsetStr(redisContext* ctx, const char* key, const char* hkey, const char* val)
{
    int ret = 1;
    redisReply* reply = (redisReply*)redisCommand(ctx, "hset %s %s %s", key, hkey, val);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        ret = strcasecmp(reply->str, "OK");
    }
    freeReplyObject(reply);

    return ret == 0;
}


void RedisHelper::beginHmset(RedisArgs& args, const char* key)
{
    args.add("hmset");
    args.add(key);
}
void RedisHelper::appendHmset(RedisArgs& args, const char* hkey, const char* val)
{
    args.add(hkey);
    args.add(val);
}
void RedisHelper::appendHmset(RedisArgs& args, const char* hkey, int val)
{
    args.add(hkey);
    args.add(val);
}

void RedisHelper::appendHmset(RedisArgs &args, int hkey, int val)
{
    args.add(hkey);
    args.add(val);
}

void RedisHelper::appendHmset(RedisArgs &args, int hkey, const char *val)
{
    args.add(hkey);
    args.add(val);
}

void RedisHelper::appendHmset(RedisArgs& args, const char* hkey, float val)
{
    args.add(hkey);
    args.add(val);
}
bool RedisHelper::commitHmset(redisContext* ctx, RedisArgs& args)
{
    int ret = 1;

    redisReply* reply = (redisReply*)redisCommandArgv(ctx, args.size(), args.data(), NULL);

    if (reply->type == REDIS_REPLY_STATUS)
    {
        ret = strcasecmp(reply->str, "OK");
    }

    freeReplyObject(reply);

    return ret == 0;
}

void RedisHelper::beginHdel(RedisArgs& args, const char* key)
{
    args.add("hdel");
    args.add(key);
}
void RedisHelper::appendHdel(RedisArgs& args,const char* keyname)
{
    args.add(keyname);
}

void RedisHelper::appendHdel(RedisArgs &args, int key)
{
    args.add(key);
}

void RedisHelper::appendHdel(RedisArgs &args, int64_t key)
{
    args.add(key);
}

bool RedisHelper::commitHdel(redisContext* ctx, RedisArgs& args)
{
    int ret = 1;
    
    redisReply* reply = (redisReply*)redisCommandArgv(ctx, args.size(), args.data(), NULL);
    
    if (reply->type == REDIS_REPLY_STATUS)
    {
        ret = strcasecmp(reply->str, "OK");
    }
    
    freeReplyObject(reply);
    
    return ret == 0;
}

void RedisHelper::beginDelKeys(RedisArgs& args)
{
    args.add("del");
}

void RedisHelper::appendKeyname(RedisArgs& args, const char* keyname)
{
    args.add(keyname);
}

bool RedisHelper::commitDelKeys(redisContext* ctx,RedisArgs& args)
{
    int ret = 1;
    
    redisReply* reply = (redisReply*)redisCommandArgv(ctx, args.size(), args.data(), NULL);
    
    if (reply->type == REDIS_REPLY_STATUS)
    {
        ret = strcasecmp(reply->str, "OK");
    }
    
    freeReplyObject(reply);
    
    return ret == 0;
}


void RedisHelper::beginHmget(RedisArgs& args, const char* key)
{
    args.add(key);
}

void RedisHelper::appendHmget(RedisArgs& args, const char* hkey)
{
    args.add(hkey);
}

RedisResult RedisHelper::commitHmget(redisContext* ctx, RedisArgs& args)
{
    redisReply* reply = (redisReply*)redisCommandArgv(ctx, args.size(), args.data(), NULL);
    return RedisResult(reply);
}

bool RedisHelper::hmsetArgv(redisContext* ctx, const char* key, int hkeynum, ...)
{
    int argc = hkeynum * 2 + 2;
    const char** argv = new const char*[argc];

    va_list ap;
    va_start(ap, hkeynum);
    argv[0] = "hmset";
    argv[1] = key;
    for (int i = 0; i < hkeynum; i++)
    {
        argv[i * 2 + 2] = va_arg(ap, const char*);
    }
    for (int i = 0; i < hkeynum; i++)
    {
        argv[i * 2 + 3] = va_arg(ap, const char*);
    }
    va_end(ap);

    int ret = 1;

    redisReply* reply = (redisReply*)redisCommandArgv(ctx, argc, argv, NULL);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        ret = strcasecmp(reply->str, "OK");
    }
    freeReplyObject(reply);

    return ret == 0;
}

bool RedisHelper::hmgetArgv(redisContext* ctx, const char* key, int hkeynum, ...)
{
    int argc = hkeynum + 2;
    const char** argv = new const char*[argc];
    char** vals = new char*[hkeynum];

    va_list ap;
    va_start(ap, hkeynum);
    argv[0] = "hmget";
    argv[1] = key;
    for (int i = 0; i < hkeynum; i++)
    {
        argv[i + 2] = va_arg(ap, const char*);
    }
    for (int i = 0; i < hkeynum; i++)
    {
        vals[i] = va_arg(ap, char*);
    }
    va_end(ap);

    int ret = 1;

    redisReply* reply = (redisReply*)redisCommandArgv(ctx, argc, argv, NULL);
    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (int i = 0; i < hkeynum; i++)
        {
            if (reply->element[i]->str != NULL)
            {
                strcpy(vals[i], reply->element[i]->str);
            }
        }
        ret = 0;
    }
    freeReplyObject(reply);

    free(vals);
    free(argv);

    return ret == 0;
}

bool RedisHelper::hmset(redisContext* ctx, const char* key, int hkeynum, const char** hkeys, const char** vals)
{
    int argc = 2 + hkeynum + hkeynum;
    const char** argv = new const char*[argc];

    argv[0] = "hmset";
    argv[1] = key;
    for (int i = 0; i < hkeynum; i++)
    {
        argv[i * 2 + 2] = hkeys[i];
        argv[i * 2 + 2] = vals[i];
    }

    int ret = 1;
    redisReply* reply = (redisReply*)redisCommandArgv(ctx, argc, argv, NULL);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        ret = strcasecmp(reply->str, "OK");
    }
    freeReplyObject(reply);
    free(argv);

    return ret == 0;
}
bool RedisHelper::hmget(redisContext* ctx, const char* key, int hkeynum, const char** hkeys, char** vals)
{
    int argc = 2 + hkeynum;
    const char** argv = new const char*[argc];

    argv[0] = "hmget";
    argv[1] = key;
    for (int i = 0; i < hkeynum; i++)
    {
        argv[i + 2] = hkeys[i];
    }

    int ret = 1;
    redisReply* reply = (redisReply*)redisCommandArgv(ctx, argc, argv, NULL);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        for (int i = 0; i < hkeynum; i++)
        {
            if (reply->element[i]->str != NULL)
            {
                strcpy(vals[i], reply->element[i]->str);
            }
        }
    }
    freeReplyObject(reply);
    free(argv);

    return ret == 0;
}

bool RedisHelper::hmset(redisContext* ctx, const char* key, const std::vector<std::string>& hkeys, const std::vector<std::string>& vals)
{
    std::vector<const char*> argv;
    argv.push_back("hmset");
    argv.push_back(key);
    for (int i = 0; i < argv.size(); i++)
    {
        argv.push_back(hkeys[i].c_str());
        argv.push_back(vals[i].c_str());
    }

    int ret = 1;
    redisReply* reply = (redisReply*)redisCommandArgv(ctx, argv.size(), argv.data(), NULL);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        ret = strcasecmp(reply->str, "OK");
    }
    freeReplyObject(reply);

    return ret == 0;
}

bool RedisHelper::hmget(redisContext* ctx, const char* key, const std::vector<std::string>& hkeys, std::vector<std::string>& vals)
{
    std::vector<const char*> argv;
    argv.push_back("hmget");
    argv.push_back(key);
    for (int i = 0; i < hkeys.size(); i++)
    {
        argv.push_back(hkeys[i].c_str());
    }

    int ret = 1;
    redisReply* reply = (redisReply*)redisCommandArgv(ctx, argv.size(), argv.data(), NULL);

    if (reply->type == REDIS_REPLY_ARRAY)
    {
        std::string val;
        for (int i = 0; i < hkeys.size(); i++)
        {
            if (reply->element[i]->str != NULL)
            {
                val = reply->element[i]->str;
            }
            vals.push_back(val);
        }
        ret = 0;
    }
    freeReplyObject(reply);

    return ret == 0;
}

//////////////////////////////////////

redisReply* myRedisGetReply(redisContext* conn)
{
    redisReply* reply = NULL;
    if ( REDIS_OK != redisGetReply(conn, (void**)&reply) ){
        log_error("commite appendcommand error");
        freeReplyObject(reply);
        return NULL;
    }
    
    return reply;
}

bool MyPipeRedis::myRedisAppendCommand(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    string command = strvFormat(format, args);

    mCommands.push_back(command);
    va_end(args);
    
    return false;
}

redisReply* MyPipeRedis::myRedisGetReply()
{
    if (mCommands.size() <= 0) {
        return NULL;
    }
    
    if (mIsCommited == false) {
        for (int i = 0; i < mCommands.size(); i++) {
            
            int ret = redisAppendCommand(mConn, mCommands[i].c_str());
            if (ret == REDIS_OK) {
                mCommandCnt++;
            }
        }
    }
    
    mIsCommited = true;
    redisReply* reply = NULL;
    if ( REDIS_OK != redisGetReply(mConn, (void**)&reply) ){
        log_error("commite appendcommand error");
        freeReplyObject(reply);
        return NULL;
    }
    
    mCommandCnt--;
    return reply;
}

void MyPipeRedis::commitImmediately()
{
    if (mCommands.size() <= 0 && mIsCommited ==false) {
        return;
    }
    
    while (mCommandCnt || mIsCommited == false) {
        redisReply* reply = myRedisGetReply();
        if (reply) {
            freeReplyObject(reply);
        }
    }
    
    mCommands.clear();
}
////////////////////////////////