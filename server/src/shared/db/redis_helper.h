//
//  redis_helper.h
//  GameSrv
//
//  Created by prcv on 13-5-2.
//
//

#ifndef GameSrv_redis_helper_h
#define GameSrv_redis_helper_h

#include "hiredis.h"
#include <string>
#include <string.h>
#include <vector>
#include <map>
#include "Utils.h"
using namespace std;

typedef void (*RefPtrFreeFunc)(void* ptr);

class RefPtr
{
public:
    void* _data;
    int   _refCount;
    RefPtrFreeFunc _freeFunc;
    
public:
    RefPtr(void* ptr, RefPtrFreeFunc freefun)
    {
        _data = ptr;
        _refCount = 1;
        _freeFunc = freefun;
    }
    ~RefPtr()
    {
        if (_data) {
            _freeFunc(_data);
        }
    }

    void* operator->()
    {
        return _data;
    }

    void release()
    {
        _refCount--;
        if (_refCount <= 0)
        {
            delete this;
        }
    }

    void retain()
    {
        _refCount++;
    }
    
    void *getData()
    {
        return _data;
    }

};

class RedisResult
{
public:
    RedisResult() : ptr(NULL)
    {

    }

    explicit RedisResult(redisReply* reply) : ptr(NULL)
    {
        ptr = new RefPtr(reply, freeReplyObject);
    }

    virtual ~RedisResult()
    {
        if (ptr)
        {
            ptr->release();
        }
    }

    RedisResult(const RedisResult& other)
    {
        ptr = other.ptr;
        ptr->retain();
    }

    RedisResult& operator=(const RedisResult& other)
    {
        if (this != &other)
        {
            if (ptr)
            {
                ptr->release();
            }

            ptr = other.ptr;
            ptr->retain();
        }

        return *this;
    }

    void setData(redisReply* data)
    {
        if (ptr)
        {
            ptr->release();
        }

        ptr = new RefPtr(data, freeReplyObject);
    }

    long readInt(long def = 0)
    {
        long ret = def;

        redisReply* reply = (redisReply*)ptr->getData();
        if (reply->type == REDIS_REPLY_INTEGER)
        {
            ret = reply->integer;
        }
        else if (reply->type == REDIS_REPLY_STRING)
        {
            ret = atoi(reply->str);
        }

        return ret;
    }

    float readFloat(float def = 0.0)
    {
        float ret = def;

        redisReply* reply = (redisReply*)ptr->getData();
        if (reply->type == REDIS_REPLY_INTEGER)
        {
            ret = reply->integer;
        }
        else if (reply->type == REDIS_REPLY_STRING)
        {
            ret = atof(reply->str);
        }

        return ret;
    }

    std::string readStr(const char* def = "")
    {
        std::string ret = def;

        redisReply* reply = (redisReply*)ptr->getData();
        if (reply->type == REDIS_REPLY_STRING)
        {
            ret = reply->str;
        }
        else if (reply->type == REDIS_REPLY_INTEGER)
        {
            ret = Utils::itoa(reply->integer);
        }

        return ret;
    }

    int getHashElements()
    {
        redisReply* reply = (redisReply*)ptr->getData();
        return reply->elements;
    }

    std::string readHash(int index, const char* def = "")
    {
        std::string ret = def;

        redisReply* reply = (redisReply*)ptr->getData();
        if (reply->type == REDIS_REPLY_ARRAY && index < reply->elements)
        {
            if (reply->element[index] != NULL && reply->element[index]->str != NULL)
            {
                ret = reply->element[index]->str;
            }
        }

        return ret;
    }
    
    char* readHashBlob(int index, int& len)
    {
        char* ret = NULL;
        len = 0;
        
        redisReply* reply = (redisReply*)ptr->getData();
        if (reply->type == REDIS_REPLY_ARRAY && index < reply->elements)
        {
            if (reply->element[index] != NULL && reply->element[index]->str != NULL)
            {
                ret = reply->element[index]->str;
                len = reply->element[index]->len;
            }
        }
        
        return ret;
    }

    int readHash(int index, int def)
    {
        int ret = def;

        redisReply* reply = (redisReply*)ptr->getData();
        if (reply->type == REDIS_REPLY_ARRAY && index < reply->elements)
        {
            if (reply->element[index] != NULL && reply->element[index]->str != NULL)
            {
                ret = atoi(reply->element[index]->str);
            }
        }

        return ret;
    }

    float readHash(int index, float def)
    {
        float ret = def;

        redisReply* reply = (redisReply*)ptr->getData();
        if (reply->type == REDIS_REPLY_ARRAY && index < reply->elements)
        {
            if (reply->element[index] != NULL && reply->element[index]->str != NULL)
            {
                ret = atof(reply->element[index]->str);
            }
        }

        return ret;
    }

    int getSortedSetsCounts()
    {
        redisReply* reply = (redisReply*)ptr->getData();
        int count = reply->elements;
        return (count + 1)/2;
    }
    bool readSortedSets(int index, int& val, string& key)
    {
        val = 0;
        key = "";
        redisReply* reply = (redisReply*)ptr->getData();
        if (reply->type == REDIS_REPLY_ARRAY && (2 * index + 1) < reply->elements) {
            if (reply->element[2*index] != NULL
                && reply->element[2*index]->str != NULL
                && reply->element[2*index + 1] != NULL
                && reply->element[2*index + 1]->str != NULL)
            {
                key = reply->element[2*index]->str;
                val = Utils::safe_atoi(reply->element[2*index + 1]->str);
                return true;
            }
        }
        return false;
    }
    
    std::string readList(int index, const char* def = "")
    {
        return readHash(index , def);
    }
    
    
    bool isStatusSuccess()
    {
        redisReply* reply = (redisReply*)ptr->getData();
        if (reply->type == REDIS_REPLY_STATUS && reply->str != NULL)
        {
            return strcmp(reply->str, "OK") == 0;
        }

        return false;
    }

    operator redisReply*()
    {
        return (redisReply*)ptr->getData();
    }

private:
    RefPtr* ptr;
};

class RedisArgs
{
public:
    RedisArgs();
    ~RedisArgs();

    void add(int val);
    void add(int64_t);
    void add(const char* val);
    void add(float val);
    void clear();
    const char** data()
    {
        return args.data();
    }

    size_t size()
    {
        return args.size();
    }

private:
    std::vector<const char*> args;
};

class RedisHelper
{
public:
    static redisContext* createContext(const char* host, unsigned short port, const char* pwd, int index);
    static void destroyContext(redisContext* context);

    //setStr("playertest:100099", "testval")
    static bool set(redisContext* context, const char* key, int val);
    static bool set(redisContext* context, const char* key, const char* val);

    //string xval = getStr("player:100099")
    static std::string getStr(redisContext* context, const char* key, const char* def = "");
    static int getInt(redisContext* context, const char* key, int def = 0);

    //int roleid = getInteger("incr gen_roleid");
    static long getInteger(redisContext* context, const char* cmd, int def = 0);

    //string xval = getHash("role:100099", "x")
    static std::string hgetStr(redisContext* context, const char* key, const char* hkey, const char* def = "");
    static int hgetInt(redisContext* context, const char* key, const char* hkey, int def = 0);

    //setHash("role:100099", "x", "123"
    static bool hsetStr(redisContext* context, const char* key, const char* hkey, const char* val);
    static bool hsetInt(redisContext* context, const char* key, const char* hkey, int val);

    static void beginHmset(RedisArgs& args, const char* key);
    static void appendHmset(RedisArgs& args, const char* hkey, const char* val);
    static void appendHmset(RedisArgs& args, const char* hkey, int val);
    static void appendHmset(RedisArgs& args, int hkey, int val);
    static void appendHmset(RedisArgs& args, int hkey, const char* val);
    static void appendHmset(RedisArgs& args, const char* hkey, float val);
    static bool commitHmset(redisContext* context, RedisArgs& args);
    
    static void beginHdel(RedisArgs& args, const char* key);
    static void appendHdel(RedisArgs& args,const char* keyname);
    static void appendHdel(RedisArgs& args,int key);
    static void appendHdel(RedisArgs &args, int64_t key);
    static bool commitHdel(redisContext* ctx, RedisArgs& args);

    static void beginDelKeys(RedisArgs& args);
    static void appendKeyname(RedisArgs& args, const char* keyname);
    static bool commitDelKeys(redisContext* ctx, RedisArgs& args);
    
    static void beginHmget(RedisArgs& args, const char* key);
    static void appendHmget(RedisArgs& args, const char* hkey);
    static RedisResult commitHmget(redisContext* context, RedisArgs& args);
    ///
    // msetHashArgv("key", 2, hkey1, hkey2, val1, val2)
    ///
    static bool hmsetArgv(redisContext* context, const char* key, int hkeynum, ...);

    ///
    // char val1[32], val2[32];
    // mgetHashArgv("key", 2, hkey1, hkey2, val1, val2)
    ///
    static bool hmgetArgv(redisContext* context, const char* key, int hkeynum, ...);

    ///
    // const char* hkeys[2] = {"key1", "key2"};
    // const char *vals[2] = {"val1", "val2"};
    // msetHash("key", 2, hkeys, vals)
    ///
    static bool hmset(redisContext* context, const char* key, int hkeynum, const char** hkeys, const char** vals);

    ///
    // const char* hkeys[2] = {"key1", "key2"};
    // char val1[32], val2[32];
    // char *vals[2] = {val1, val2};
    // mgetHash("key", 2, hkeys, vals)
    ///
    static bool hmget(redisContext* context, const char* key, int hkeynum, const char** hkeys, char** vals);

    ///
    //vector<string> a, b;
    //a.push_back("x");
    //a.push_back("y");
    //a.push_back("lvl");
    //b.push_back("6");
    //b.push_back("7");
    //b.push_back("26");
    //helper.msetHash("role:100099", a, b);
    ///
    static bool hmset(redisContext* context, const char* key, const std::vector<std::string>& hkeys, const std::vector<std::string>& vals);

    ///
    //vector<string> a, b;
    //a.push_back("x");
    //a.push_back("y");
    //a.push_back("lvl");
    //helper.mgetHash("role:100099", a, b);
    ///
    static bool hmget(redisContext* context, const char* key, const std::vector<std::string>& hkeys, std::vector<std::string>& vals);

    //add set
    static bool addSetMember();

    //del set
    static bool delSetMember();

    //set exist
    static bool existSetMember();
};

class MyPipeRedis
{
public:
    MyPipeRedis(redisContext* conn):mCommandCnt(0),
                                    mIsCommited(false),
                                    mConn(conn)
    {
        
    }
    
    ~MyPipeRedis()
    {
        commitImmediately();
    }
    
    bool myRedisAppendCommand(const char *format, ...);
    
    redisReply* myRedisGetReply();
    
    void commitImmediately();
    
private:
    int mCommandCnt;    //命令的条数
    int mIsCommited;
    
    redisContext* mConn;
    
    vector<string> mCommands;
};

extern redisReply* myRedisGetReply(redisContext* conn);

////////////////////////////////////
#endif  //GameSrv_redis_helper_h
