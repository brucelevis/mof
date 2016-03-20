#include "dbMgr.h"
#include "process.h"

__thread redisContext* tContext = NULL;

void 
registerRedisContext(redisContext* context) {
	tContext = context;
}

void
unregisterRedisContext() {
	tContext = NULL;
}

redisContext* 
getRedisContext() {
	return tContext;
}

redisContext*
createRedisContext() {
    return RedisHelper::createContext(Process::env.getString("db_host").c_str(),
                                      Process::env.getInt("db_port"),
                                      Process::env.getString("db_password").c_str(),
                                      Process::env.getInt("db_index"));
}