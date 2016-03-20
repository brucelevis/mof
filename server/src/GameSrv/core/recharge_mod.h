//
//  recharge_mod.h
//  GameSrv
//
//  Created by prcv on 13-12-4.
//
//

#ifndef __GameSrv__recharge_mod__
#define __GameSrv__recharge_mod__

#include <iostream>

#include "thread.h"
#include "redis_helper.h"

class RechargeMod : public Runnable
{
public:
    virtual bool beforeRun();
    virtual void run();
    virtual void afterRun();
    
    bool init();
    void deinit();
    
    void start();
    void stop();
    
private:
    Thread mThread;
    
    redisContext* mContext;
    bool   mRunning;
};

extern void createNewOrderCmd(string platform, int rmb, int roleid, string billno);

#endif /* defined(__GameSrv__recharge_mod__) */
