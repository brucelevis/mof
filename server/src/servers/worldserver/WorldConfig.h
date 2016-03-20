//
//  WorldConfig.h
//  GameSrv
//
//  Created by prcv on 14-7-7.
//
//

#ifndef __GameSrv__WorldConfig__
#define __GameSrv__WorldConfig__

#include <iostream>
#include <stdint.h>
class WorldConfig
{
public:
    WorldConfig()
    {
        mHistoryPositionTime = 1000;
    }
    bool load(const char* file);
    int64_t mHistoryPositionTime;
};

extern WorldConfig g_WorldConfig;

#endif /* defined(__GameSrv__WorldConfig__) */
