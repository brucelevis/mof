//
//  dbconnector.cpp
//  MergeDB
//
//  Created by pireszhi on 14-1-13.
//  Copyright (c) 2014年 pireszhi. All rights reserved.
//

#include "dbconnector.h"
#include "redis_helper.h"
#include "log.h"
#include <string.h>
#include "inifile.h"
DBManager g_DBMgr;

redisContext* createContext(const char* host, unsigned short port, const char* pwd, int index)
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
            printf("plase start redis db (error: %s )\n", redisContex->errstr);
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

bool DBManager::initMgr()
{
    if (!loadConfig()) {
        return false;
    }

    if (!initConnectors()) {
        return false;
    }
    return true;
}

bool DBManager::loadConfig()
{
    string exePath = GetAppPath();
    string fullpath = exePath + "/merge_db.ini";
    IniFile inifile(fullpath.c_str());
    mMaster.host = inifile.getValue("master", "host");
    mMaster.port = inifile.getValueT("master", "port", 0);
    mMaster.index = inifile.getValueT("master", "index", 0);
    mMaster.passWord = inifile.getValue("master", "password");
    mMaster.serverName = inifile.getValue("master", "servername");

    if ( !mMaster.isConfigLoad()) {
        printf("load masterDB error\n");
        return false;
    }

    for (int i = 1; i < 32; i++) {

        string sec_name = Utils::makeStr("slave%d", i);

        DBConnector conn;
        conn.host = inifile.getValue(sec_name.c_str(), "host");
        conn.port = inifile.getValueT(sec_name.c_str(), "port", 0);
        conn.index = inifile.getValueT(sec_name.c_str(), "index", 0);
        conn.passWord = inifile.getValue(sec_name.c_str(), "password");
        conn.serverName = inifile.getValue(sec_name.c_str(), "servername");

        if (!conn.isConfigLoad()) {
            continue;
        }

        if (checkAdressExist(conn)) {
            log_error("不能是用相同的数据库");
            return false;
        }

        mSlaves.push_back(conn);
    }

    if (mSlaves.size() <= 0) {
        log_error("至少需要一个分区才能进行合并");
        return false;
    }

    return true;
}

bool DBManager::checkAdressExist(const DBConnector &conn)
{
    if ( strcmp(mMaster.serverName.c_str(), conn.serverName.c_str()) == 0 ) {
        return true;
    }

    if (strcmp(mMaster.host.c_str(), conn.host.c_str()) == 0 && mMaster.port == conn.port && mMaster.index == conn.index) {
        return true;
    }

    for (int i = 0; i < mSlaves.size(); i++) {

        if (strcmp(mSlaves[i].serverName.c_str(), conn.serverName.c_str()) == 0) {
            return true;
        }

        if (strcmp(mSlaves[i].host.c_str(), conn.host.c_str()) == 0 && mSlaves[i].port == conn.port && mSlaves[i].index == conn.index) {
            return true;
        }
    }

    return false;
}

bool DBManager::initConnectors()
{
    redisContext* dbContext = RedisHelper::createContext(mMaster.host.c_str(), mMaster.port, mMaster.passWord.c_str(), mMaster.index);

    if (dbContext == NULL) {
        log_error("无法连接到目标数据库");
        return false;
    }

    mMaster._dbContextes = dbContext;

    log_info("connect to master DB success");

    for (int i = 0; i < mSlaves.size(); i++) {
        redisContext* slaveDbContext = RedisHelper::createContext(mSlaves[i].host.c_str(), mSlaves[i].port, mSlaves[i].passWord.c_str(), mSlaves[i].index);

        if (slaveDbContext == NULL) {
            log_error("无法连接到分区 ( error slave"<<(i + 1)<<" )");
            return false;
        }

        mSlaves[i]._dbContextes = slaveDbContext;

        log_info("connect to slave["<<(i+1)<<"] success");
    }
    return true;
}

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
