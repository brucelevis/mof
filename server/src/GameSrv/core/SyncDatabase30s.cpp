//
//  SyncDatabaseFiveMinute.cpp
//  GameSrv
//
//  Created by huangkunchao on 13-11-13.
//
//  五分钟定时回写数据库

#include "DynamicConfig.h"
#include "GameLog.h"
#include "Pet.h"
#include "Role.h"

static int tick = 0;

void writeDatabase30s(void* param)
{
    
    
    
    
    

    ++ tick;
}


//addTimerDef(writeDatabase30s,NULL, 30 , FOREVER_EXECUTE);

