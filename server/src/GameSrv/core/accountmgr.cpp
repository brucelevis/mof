//
//  accountmgr.cpp
//  GameSrv
//
//  Created by prcv on 13-9-23.
//
//

#include "accountmgr.h"
#include "main.h"
#include "datetime.h"
#include "redis_helper.h"


bool tryRegisterAccount(const char* account, const char* platform, time_t createTime)
{
    RedisResult result(redisCmd("exists player:%s", account));
    
    if (result.readInt() == 1){
        return false;
    }
    
    doRedisCmd("hmset player:%s platform %s create_time %s", account, platform, DateTime::time2Str(createTime).c_str());
    
    return true;
}
