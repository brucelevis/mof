//
//  File.h
//  GameSrv
//
//  Created by pireszhi on 13-7-1.
//
//

#ifndef __GameSrv__File__
#define __GameSrv__File__

#include <iostream>
#include <iostream>
#include "GlobalMsg_def.h"
#include "thread.h"
#include "timestamp.h"
#include "hiredis.h"

struct message_queue;
class Global: public Runnable
{
public:
    Global(void):_stop(false)
    {
        
    }
    virtual ~Global(void){}
    bool beforeRun();
    void run();
    int init();
    void update(float dt);
    void stop(){_stop = true;}
    static message_queue *MQ;
    static time_t tick;
private:
    Timestamp _ts;
    bool _stop;
    int mEverydayFreshTimes;
};
//#define GlobalRedisCmd(...) (redisReply*)redisCommand(Global::getDBConnector(),__VA_ARGS__)
//#define DoGlobalRedisCmd(...) freeReplyObject(GlobalRedisCmd(__VA_ARGS__))
#endif /* defined(__GameSrv__File__) */
