#ifndef GAMESRV_WORLD_SCRIPT_MONSTER_AI
#define GAMESRV_WORLD_SCRIPT_MONSTER_AI

#include "WorldCreatureAI.h"

class WorldScriptMonsterAI : public WorldCreatureAI
{
public:
    WorldScriptMonsterAI();
    
    virtual void update(uint64_t ms);
    
    int mUpdateCounter;
    int mState;
    int mSubState;
};


#endif