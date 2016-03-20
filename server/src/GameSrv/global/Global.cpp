//
//  File.cpp
//  GameSrv
//
//  Created by pireszhi on 13-7-1.
//
//

#include "Global.h"
#include "GlobalMsg_helper.h"
#include "MQ.h"
#include "Game.h"
#include "Utils.h"
#include "process.h"
#include "log.h"
#include <stdio.h>
#include "Pvp.h"
#include "Activity.h"
#include "PetPvp.h"
#include "RealPvpMgr.h"
// 活动测试代码
/*
void actfun1(int actId, int number, string& params)
{
    printf("actfun1, %d: %s\n", number, params.c_str());
}

void actfun2(int actId, int number, string& params)
{
    printf("actfun2, %d: %s\n", number, params.c_str());
}

void actfun3(int actId, int number, string& params)
{
    printf("actfun3, %d: %s\n", number, params.c_str());
}

void actfun4(int actId, int number, string& params)
{
    printf("actfun4, %d: %s\n", number, params.c_str());
}*/


message_queue* Global::MQ = NULL;
time_t Global::tick = 0;

int Global::init()
{
    //connect db
//    struct timeval timeout = {2, 0};
//    db_connector = redisConnectWithTimeout((char*)(Process::env["db_host"].c_str()), Process::env.getInt("db_port"), timeout);



    // 活动测试代码
    /*
    SActMgr.SafeCreateTimer(2, att_before_open, 10, actfun1, "att_before_open");
    SActMgr.SafeCreateTimer(2, att_after_open, 10, actfun2, "att_after_open");
    SActMgr.SafeCreateTimer(2, att_before_close, 10, actfun3, "att_before_close");
    SActMgr.SafeCreateTimer(2, att_after_close, 10, actfun4, "att_after_close");
    */

    return 0;
}

bool Global::beforeRun()
{
    Global::tick = time(NULL);
    
    usleep(100 * 1000);//0.1s
    
    redisContext* redisdb = (redisContext*)_thread->getData();
    g_DBMgr.registerDB(_thread->getThreadId(), redisdb);
    
    redisReply* rreply;
    rreply = redisCmd("get globalmodfreshtime");
    
    if (rreply->type == REDIS_REPLY_NIL) {
        time_t nowtime = time(NULL);
        struct tm date;
        localtime_r(&nowtime, &date);
        
        date.tm_hour = 0;
        date.tm_min = 0;
        date.tm_sec = 0;
        
        time_t nextfreshtime = mktime(&date) + 3600*24;
        char tmp[128];
        sprintf(tmp, "set globalmodfreshtime %d", (int)nextfreshtime);
        doRedisCmd(tmp);
        
        mEverydayFreshTimes = nextfreshtime;
    }
    else
    {
        mEverydayFreshTimes = Utils::safe_atoi(rreply->str);
    }
    freeReplyObject(rreply);
    
    SPvpMgr.loadDataFromDB();
    SSortMgr.init();
    SActMgr.LoadConfig();
//    SPetPvpMgr.loadDataInDB();
    SPetPvpMgr.load();
    g_fameHallMgr.init();
    
    //同步竞技场管理器初始化；
    g_RealpvpMgr.initialize();

    return true;
}
void Global::run()
{
    while (!_stop) {
        int remain=FRAME_TIME_MILLSEC - _ts.elapsed();
        if(remain > 0){
            ::Thread::sleep(remain);
            update(FRAME_TIME_SEC);
        }else{
            update((-remain+FRAME_TIME_MILLSEC)/1000);
        }

        _ts.update();
    }
}

void Global::update(float dt)
{
    Global::tick = time(NULL);
    time_t nowtime = Global::tick;
    
    bool isEveryDayFresh = false;

    if ( nowtime  >= mEverydayFreshTimes ) {

        struct tm date;
        localtime_r(&nowtime, &date);

        date.tm_hour = 0;
        date.tm_min = 0;
        date.tm_sec = 0;

        time_t nextfreshtime = mktime(&date) + 3600*24;

        char tmp[128];
        sprintf(tmp, "set globalmodfreshtime %d", (int)nextfreshtime);
        doRedisCmd(tmp);
        mEverydayFreshTimes = nextfreshtime;
        isEveryDayFresh = true;
    }

    
    SPvpMgr.onHeartBeat(dt);
    SSortMgr.onHeartBeat(dt);
    
    //全局刷新时间，每天12点
    if (isEveryDayFresh) {

        SActMgr.OnDailyRefresh();
    }

    // 活动对象跑循环
    SActMgr.OnFrame(dt);

    skynet_message m;
    while( 0 == skynet_mq_pop(MQ, &m) )
    {
        IGlobalMsg* msg = (IGlobalMsg*)m.data;
        if (!msg) {
            return;
        }
        msg->handle();
        destroy_global(msg);
    }
    
    SPetPvpMgr.onHeartBeat();
    g_fameHallMgr.onHeartBeat();
    g_RealpvpMgr.update(dt);
}

