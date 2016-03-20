//
//  WorldRole.cpp
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#include "WorldRole.h"
#include "WorldScene.h"
#include "WorldPet.h"
#include "WorldPlayer.h"
#include "msg.h"

WorldRoleMgr g_WorldRoleMgr;



void WorldRole::updateAlive(uint64_t ms)
{
    WorldCreature::updateAlive(ms);
}

void WorldRole::onDie(uint64_t ms)
{
    setValid(false);
}

void WorldRole::updateDead(uint64_t ms)
{
    
}


bool WorldRole::preEnterScene(WorldScene* scene)
{
    if (!WorldCreature::preEnterScene(scene))
    {
        return false;
    }
    return true;
}

void WorldRole::onEnterScene()
{    
    CCPoint vec = mScene->getRandPos(getAreaId());
    setOrient(eLeft);
    setPosition(vec);
    SceneObject::setSpeed(Vector2(0,0));
    
    setHp(getMaxHp());
    setGroup(mPlayer->mRoleId);
}

void WorldRole::onLeaveScene()
{
    WorldCreature::onLeaveScene();
}


float WorldRole::getAttackMoveTime()
{
    if(mTpltCfg)
        return mTpltCfg->mAttackMoveTime;
    else
        return SeparateValues::sDefaultAttackMoveTime;
}

int WorldRole::getAttackMoveDistance()
{
    if(mTpltCfg)
        return mTpltCfg->mAttackMoveDistance;
    else
        return SeparateValues::sDefaultAttackMoveDistance;
}

float WorldRole::getLastAttackHitBackTime()
{
    if(mTpltCfg)
        return mTpltCfg->mLastAttackHitBackTime;
    else
        return getAttackMoveTime();
}

int WorldRole::getLastAttackHitBackDistance()
{
    if(mTpltCfg)
        return mTpltCfg->mLastAttackHitBackDistance;
    else
        return getAttackMoveDistance();
}

