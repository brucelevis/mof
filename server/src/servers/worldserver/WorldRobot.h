//
//  WorldRobot.h
//  GameSrv
//
//  Created by prcv on 14-3-14.
//
//

#ifndef __GameSrv__WorldRobot__
#define __GameSrv__WorldRobot__

#include <iostream>

#include "WorldCreature.h"

class WorldRobot : public WorldCreature
{
public:
    WorldRobot() : WorldCreature(eWorldObjectRobot)
    {
        
    }
    
    virtual void update(uint64_t ms)
    {
        WorldCreature::update(ms);
    }
};


#endif /* defined(__GameSrv__WorldRobot__) */
