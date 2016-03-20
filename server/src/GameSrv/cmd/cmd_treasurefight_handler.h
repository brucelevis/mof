//
//  cmd_treasurefight_handler.h
//  GameSrv
//
//  Created by xinyou on 14-5-29.
//
//

#ifndef __GameSrv__cmd_treasurefight_handler__
#define __GameSrv__cmd_treasurefight_handler__

#include <iostream>

#include "cmd_def.h"

begin_cmd(TreasureFightKillNpc)
    cmd_member(int, killerid)
    cmd_member(int, killerscore)
    cmd_member(vector<int>, assistids)
    cmd_member(vector<int>, assistscores)
end_cmd()

begin_cmd(TreasureFightKillPlayer)
    cmd_member(int, victimid)
    cmd_member(int, killerid)
    cmd_member(int, killerscore)
    cmd_member(vector<int>, assistids)
    cmd_member(vector<int>, assistscores)
end_cmd()

begin_cmd(TreasureFightPlayerLeave)
    cmd_member(vector<int>, ids)
    cmd_member(vector<int>, scores)
end_cmd()

#endif /* defined(__GameSrv__cmd_treasurefight_handler__) */
