//
//  db_mgr.cpp
//  GameSrv
//
//  Created by prcv on 13-8-16.
//
//

#include "db_mgr.h"

DBMgr g_DBMgr;

__thread redisContext* t_DbContext = NULL;

redisContext* DBMgr::getDBContext(pthread_t threadid)
{
    return t_DbContext;
}

bool DBMgr::registerDB(pthread_t threadid, redisContext* context)
{
    t_DbContext = context;
    return true;
}

void DBMgr::unregisterDB(pthread_t threadid)
{
    t_DbContext = NULL;
}