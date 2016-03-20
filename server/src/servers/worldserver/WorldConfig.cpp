//
//  WorldConfig.cpp
//  GameSrv
//
//  Created by prcv on 14-7-7.
//
//

#include "WorldConfig.h"

#include "inifile.h"
#include "GameIniFile.h"

WorldConfig g_WorldConfig;

bool WorldConfig::load(const char* file)
{
    GameInifile ini(file);
    
    mHistoryPositionTime = ini.getValueT("root", "history_position_ms", 1000);
    
    return true;
}