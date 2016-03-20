#include "WorldScriptMonsterAI.h"
#include "WorldMonster.h"
#include "WorldGeometry.h"
#include "msg.h"

WorldScriptMonsterAI::WorldScriptMonsterAI()
{
}

void WorldScriptMonsterAI::update(uint64_t ms)
{
    mUpdateCounter++;
    
    WorldMonster* monster = dynamic_cast<WorldMonster*>(mCreature);
    if (monster->isDead()) {
        return;
    }
 
    // added by jianghan
    monster->DealMonsterAiState();
    // end add
}
