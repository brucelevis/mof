//
//  recharge_mod.cpp
//  GameSrv
//
//  Created by prcv on 13-12-4.
//
//

#include "recharge_mod.h"
#include "db_mgr.h"
#include "process.h"
#include "os.h"
#include "cmd_def.h"
#include "main.h"
#include "Game.h"
#include "GameLog.h"

void createNewOrderCmd(string platform, int rmb, int roleid, string billno)
{
    create_cmd(newOrder, newOrder);
    newOrder->platform = platform;
    newOrder->rmb = rmb;
    newOrder->roleid = roleid;
    newOrder->transaction = billno;
    sendCmdMsg(Game::MQ, newOrder);
}

bool RechargeMod::init()
{
    mContext = RedisHelper::createContext(Process::env.getString("db_host").c_str(),
                                          Process::env.getInt("db_port"),
                                          Process::env.getString("db_password").c_str(),
                                          Process::env.getInt("db_index"));
    
    return mContext != NULL;
}

bool RechargeMod::beforeRun()
{
    
    return true;
}

void RechargeMod::run()
{
    mRunning = true;
    
    string orderKey = strFormat("payment:orders:%d", Process::env.getInt("server_id"));
    while (mRunning)
    {        
        for (;;)
        {
            RedisResult result((redisReply*)redisCommand(mContext, "lpop %s", orderKey.c_str()));
            string orderStr = result.readStr();
            if (orderStr.empty()) {
                break;
            }
			
			Xylog log(eLogName_OrderPayment, 0);
			log << orderStr;
            
            Json::Value value;
            if (!xyJsonSafeParse(orderStr, value)) {
                continue;
            }
            
            try
            {
                // 新版充值，json格式串 {"platform": "123", "billno": "123", "roleid":12312, "amount":123}
                // platform  平台
                // billno    订单号
                // roleid    角色id
                // amount    金钻数量
				
                string platform = value["platform"].asString();
                string billno = value["billno"].asString();
                int roleid = xyJsonAsInt(value["roleid"]);
                int amount = xyJsonAsInt(value["amount"]);
                
                if (roleid > 0 && amount > 0)
                {
                    createNewOrderCmd(platform, amount, roleid, billno);
                }
            }
            catch(...)
            {
                
            }
        }
        
        
        if (mRunning)
        {
            Thread::sleep(1000);
        }
    }
}

void RechargeMod::afterRun()
{
    redisFree(mContext);
}

void RechargeMod::start()
{
    mThread.start(this);
}

void RechargeMod::stop()
{
    mRunning = false;
    mThread.join();
}

void RechargeMod::deinit()
{
}