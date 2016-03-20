//
//  WorldPet.cpp
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#include "WorldPet.h"
#include "WorldRole.h"
#include "WorldScene.h"
#include "WorldPlayer.h"

WorldPetMgr g_WorldPetMgr;

void WorldPet::updateAlive(uint64_t ms)
{
}

void WorldPet::onDie(uint64_t ms)
{
    mDeathState = eDeathStateDead;
}

void WorldPet::updateDead(uint64_t ms)
{
    setValid(false);
}

void WorldPet::onEnterScene()
{
}

void WorldPet::onLeaveScene()
{
    WorldCreature::onLeaveScene();
}