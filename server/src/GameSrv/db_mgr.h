//
//  db_mgr.h
//  GameSrv
//
//  Created by prcv on 13-8-16.
//
//

#ifndef GameSrv_db_mgr_h
#define GameSrv_db_mgr_h

#include <map>
#include <pthread.h>
#include "hiredis.h"
#include "log.h"
using namespace std;

class DBMgr
{
public:
    redisContext* getDBContext(pthread_t threadid);
    bool registerDB(pthread_t threadid, redisContext* context);
    void unregisterDB(pthread_t threadid);
private:
    map<pthread_t, redisContext*> _dbContextes;
};

extern DBMgr g_DBMgr;

#endif
