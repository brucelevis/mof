//
//  OnLineNumberLog.c
//  GameSrv
//
//  Created by nothing on 14-2-22.
//
//

#include "GameLog.h"
#include "DynamicConfig.h"
#include "Role.h"

void updateOnLineNumber(void *param)
{
    Xylog log(eLogName_OnLineNumber, 0);
    log << SRoleMgr.getRoleNum();
}

addTimerDef(updateOnLineNumber, NULL, 60, FOREVER_EXECUTE);
