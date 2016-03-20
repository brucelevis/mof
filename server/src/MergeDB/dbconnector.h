//
//  dbconnector.h
//  MergeDB
//
//  Created by pireszhi on 14-1-13.
//  Copyright (c) 2014年 pireszhi. All rights reserved.
//

#ifndef __MergeDB__dbconnector__
#define __MergeDB__dbconnector__

#include <iostream>
#include <string>
#include <vector>
#include "hiredis.h"
#include "inifile.h"
#define redisCmd(dbConn,...) (redisReply*)redisCommand(dbConn,__VA_ARGS__)

//不需要自己释放
#define doRedisCmd(dbConn,...) freeReplyObject(redisCmd(dbConn,__VA_ARGS__))

using namespace std;
struct DBConnector
{
    DBConnector()
    {
        host = "";
        port = 0;
        index = 0;
        serverName = "";
        _dbContextes = NULL;
    }

    DBConnector operator=(const DBConnector& conn)
    {
        host = conn.host;
        port = conn.port;
        passWord = conn.passWord;
        index = conn.index;
        serverName = conn.serverName;
        _dbContextes = conn._dbContextes;
        return *this;
    }

    bool isConfigLoad()
    {
        return (host.size() > 0 && port > 0 && index >= 0 && serverName.size() > 0);
    }
    string host;
    int port;
    string passWord;
    int index;
    string serverName;

    redisContext* _dbContextes;
};

class DBManager
{
public:
    bool initMgr();
    bool loadConfig();
    bool checkAdressExist(const DBConnector& conn);
    bool initConnectors();
public:
    DBConnector mMaster;
    vector<DBConnector> mSlaves;
};

extern DBManager g_DBMgr;
redisContext* createContext(const char* host, unsigned short port, const char* pwd, int index);

redisReply* myRedisGetReply(redisContext* conn);

#endif /* defined(__MergeDB__dbconnector__) */
