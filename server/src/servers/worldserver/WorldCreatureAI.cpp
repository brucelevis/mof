//
//  WorldCreatureAI.cpp
//  GameSrv
//
//  Created by xinyou on 14-7-10.
//
//

#include "WorldCreatureAI.h"
#include "WorldCreature.h"
#include "WorldMonster.h"
#include "WorldMonsterAI.h"
#include "WorldScriptMonsterAI.h"



WorldCreatureAI::WorldCreatureAI()
{
    mCreature = NULL;
}

WorldCreatureAI* WorldCreatureAI::create(WorldCreature* creature, const char *type)
{
    WorldCreatureAI* ai;
    
    if (strcmp(type, "monster") == 0) {
        ai = new WorldMonsterAI();
        ai->attach(creature);
        return ai;
    } else if (strcmp(type, "luascript") == 0 ) {
        ai = new WorldScriptMonsterAI();
        ai->attach(creature);
        return ai;
    } else {
        return NULL;
    }
}

void WorldCreatureAI::attach(WorldCreature *creature)
{
    mCreature = creature;
}

