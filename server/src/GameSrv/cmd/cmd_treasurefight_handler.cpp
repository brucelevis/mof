//
//  cmd_treasurefight_handler.cpp
//  GameSrv
//
//  Created by xinyou on 14-5-29.
//
//

#include "cmd_treasurefight_handler.h"
#include "log.h"
#include "TreasureFight.h"

handler_cmd(TreasureFightKillNpc)
{
    g_GuildTreasureFightMgr.addPlayerFightPoints(this->killerid, this->killerscore, 0, 1, 0);
    
    int assistsCount = this->assistids.size();
    int resultCount = this->assistscores.size();
    
    if (assistsCount != resultCount) {
        log_error("角色的数目跟结果数目不相等");
        return;
    }
    
    for (int i = 0; i < assistsCount; i++) {
        g_GuildTreasureFightMgr.addPlayerFightPoints(this->assistids[i], this->assistscores[i], 0, 0, 1);
    }
}

handler_cmd(TreasureFightKillPlayer)
{
    g_GuildTreasureFightMgr.addPlayerFightPoints(this->killerid, this->killerscore, 0, 1, 0);
    
    int assistsCount = this->assistids.size();
    int resultCount = this->assistscores.size();
    
    if (assistsCount != resultCount) {
        log_error("角色的数目跟结果数目不相等");
        return;
    }
    
    for (int i = 0; i < assistsCount; i++) {
        g_GuildTreasureFightMgr.addPlayerFightPoints(this->assistids[i], this->assistscores[i], 0, 0, 1);
    }
}

handler_cmd(TreasureFightPlayerLeave)
{
    int assistsCount = this->ids.size();
    int resultCount = this->scores.size();
    
    if (assistsCount != resultCount) {
        log_error("角色的数目跟结果数目不相等");
        return;
    }
    
    for (int i = 0; i < assistsCount; i++) {
        g_GuildTreasureFightMgr.addPlayerFightPoints(this->ids[i], this->scores[i], 0, 0, 0);
    }
}