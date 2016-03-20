#ifndef GAMESRV_WORLD_MONSTER_AI
#define GAMESRV_WORLD_MONSTER_AI

#include "WorldCreatureAI.h"

class WorldMonsterAI : public WorldCreatureAI
{
public:
    WorldMonsterAI();
    
    virtual void update(uint64_t ms);
    
    int mUpdateCounter;
    int mState;
    int mSubState;
};


#endif