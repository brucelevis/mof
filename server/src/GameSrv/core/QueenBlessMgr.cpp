//
//  QueenBlessMgr.cpp
//  GameSrv
//
//  Created by nothing on 14-3-24.
//
//

#include "QueenBlessMgr.h"
#include "DynamicConfig.h"
#include "DataCfg.h"
#include "msg.h"
#include "GameLog.h"
#include "main.h"
#include "BroadcastNotify.h"

static void broadcastGetFatStatus(void *param)
{
    notify_get_fat_status notify;
    broadcastPacket(WORLD_GROUP, &notify);
    int remaintime = QueenBlessCfg::getRemainTime();
    log_info("remain time: " << remaintime);
    addTimer(broadcastGetFatStatus, NULL, remaintime, 1);
}

QueenBlessMgr g_QueenBlessMgr;

void QueenBlessMgr::init()
{
    int remaintime = QueenBlessCfg::getRemainTime();
    log_info("remain time: " << remaintime);
    addTimer(broadcastGetFatStatus, NULL, remaintime, 1);
}



