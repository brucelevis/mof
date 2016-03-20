//
//  WorldCreatureAI.h
//  GameSrv
//
//  Created by xinyou on 14-7-10.
//
//

#ifndef __GameSrv__WorldCreatureAI__
#define __GameSrv__WorldCreatureAI__

#include <stdint.h>

class WorldCreature;


class WorldCreatureAI
{
public:
    WorldCreatureAI();
    virtual ~WorldCreatureAI() {}
    
    static WorldCreatureAI* create(WorldCreature* creature, const char* type);
    
    void attach(WorldCreature* creature);
    virtual void update(uint64_t ms) = 0;
    
    WorldCreature* mCreature;
};


#endif /* defined(__GameSrv__WorldCreatureAI__) */
