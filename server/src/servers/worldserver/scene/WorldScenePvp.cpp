//
//  WorldScenePvp.cpp
//  GameSrv
//
//  Created by prcv on 14-3-20.
//
//

#include "WorldScenePvp.h"
#include "WorldPlayer.h"
#include "main.h"
#include "world_cmdmsg.h"
#include "msg.h"
#include "RealPvpMgr.h"
#include <sys/errno.h>

#define PVP_SCENE_FULL 1

enum PvpPlayerState
{
    ePvpPlayerAlive,
    ePvpPlayerLeave,
    ePvpPlayerDead,
};

WorldScenePvp::WorldScenePvp() : WorldScene()
{
    
}

WorldScenePvp::~WorldScenePvp()
{
}

void WorldScenePvp::onInit()
{
    mCountDownTime = 3000;
    //五秒钟缓冲时间
    mMaxFightTime = SynPvpFuntionCfg::mCfgDef.maxFightTime * 1000 + 5;
    
    mMaxPlayerNum = 2;
    mPlayerNum = 0;
    
    mSubState = 0;
    mSubStateTime = 0;
    
    setActiveArea(0);
}


void WorldScenePvp::update(uint64_t ms)
{
    mSubStateTime += ms;
    switch (mSubState) {
    case eWorldScenePvpInit:
        startCountDown();
        break;
    case eWorldScenePvpCountDown:
        if (mSubStateTime > mCountDownTime) {
            startBattle();
        }
        break;
    case eWorldScenePvpBattleStart:
        if (mSubStateTime > mMaxFightTime) {
            endBattle(eSyncPvpEndTimeout);
        } else {
            WorldScene::update(ms);
        }
        break;
    case eWorldScenePvpBattleEnd:
        pendEnd();
        break;
    default:
        break;
    }
}

void WorldScenePvp::onCreatureDead(WorldCreature* victim, WorldCreature* killer)
{
    if (victim->getType() == eWorldObjectRole)
    {
        WorldRole* role = (WorldRole*)victim;
        if (role->mRoleInfo.mRoleId == mPlayerDetails[0].mRoleId)
        {
            mPlayerDetails[0].mState = ePvpPlayerDead;
        }
        else
        {
            mPlayerDetails[1].mState = ePvpPlayerDead;
        }
        
        endBattle(eSyncPvpEndPlayerDead);
    }
}

void WorldScenePvp::beforePlayerLeave(WorldPlayer* player)
{
    if (mSubState < eWorldScenePvpBattleEnd)
    {
        WorldRole* role = player->getRole();
        if (role->mRoleInfo.mRoleId == mPlayerDetails[0].mRoleId)
        {
            mPlayerDetails[0].mState = ePvpPlayerLeave;
        }
        else
        {
            mPlayerDetails[1].mState = ePvpPlayerLeave;
        }
        
        endBattle(eSyncPvpEndPlayerLeave);
    }
}

void WorldScenePvp::startCountDown()
{
    mSubState = eWorldScenePvpCountDown;
    mSubStateTime = 0;
    activeArea(0);
}

void WorldScenePvp::startBattle()
{
    mSubState = eWorldScenePvpBattleStart;
    mSubStateTime = 0;
    notify_sync_pvp_start notify;
    notify.battime = mMaxFightTime / 1000;
    broadcastPacket(&notify);
}

void WorldScenePvp::endBattle(int reason)
{
    if (mSubState == eWorldScenePvpBattleEnd) {
        return;
    }
    
    mSubState = eWorldScenePvpBattleEnd;
    
    notify_sync_pvp_end notify;
    notify.reason = reason;
    broadcastPacket(&notify);
    
    if (mPlayerNum < 2) {
        return;
    }
    
    int loser = 0;
    int winner = 1;
    if (mPlayerDetails[0].mState == ePvpPlayerAlive) {
        if (mPlayerDetails[1].mState == ePvpPlayerAlive) {
            list<WorldPlayer*>::iterator iter;
            
            for (int i = 0; i < mPlayerNum; i++) {
                for (iter = mPlayers.begin(); iter != mPlayers.end(); iter++) {
                    WorldPlayer* player = *iter;
                    if (player->getRole()->mRoleInfo.mRoleId == mPlayerDetails[i].mRoleId) {
                        mPlayerDetails[i].mCurHp = player->getRole()->getHp();
                        break;
                    }
                }
            }
            
            if (mPlayerDetails[0].mCurHp > mPlayerDetails[1].mCurHp) {
                loser = 1;
                winner = 0;
            }
        } else {
            loser = 1;
            winner = 0;
        }
    }

    create_cmd(WorldPvpResult, worldPvpResult);
    worldPvpResult->winner = mPlayerDetails[winner].mRoleId;
    worldPvpResult->loser = mPlayerDetails[loser].mRoleId;
    worldPvpResult->loserState = mPlayerDetails[loser].mState;
    worldPvpResult->extendStr = getExtendProp();
    sendMessageToGame(0, CMD_MSG, worldPvpResult, 0);
    
}

bool WorldScenePvp::preAddPlayer(WorldPlayer* player)
{
    if (mMaxPlayerNum <= mPlayerNum) {
        return false;
    }
    
    return true;
}

void WorldScenePvp::onAddedPlayer(WorldPlayer* player)
{
    WorldRole* role = player->getRole();
    WorldPet* pet = player->getFirstPet();
    WorldRetinue *retinue =player -> getRetinue();
    
    if (role)
    {
        int roleHp = role->mRoleInfo.mMaxHp;
        //roleHp *= SynPvpFuntionCfg::mCfgDef.roleHpMultiple;
        
        role->setHp(roleHp);
        role->setMaxHp(roleHp);
        role->setGroup(player->mRoleId);
    }
    
    if (pet)
    {
        int petHp = pet->mPetInfo.mMaxHp;
        //petHp *= SynPvpFuntionCfg::mCfgDef.petHpMultiple;
        pet->setMaxHp(petHp);
        pet->setHp(petHp);
        pet->setGroup(player->mRoleId);
    }
    
    if(retinue){
        retinue -> setGroup(player->mRoleId);
    }
    
    if (mPlayerNum < mCfg.mLandPoses.size())
    {
        Point pt = mCfg.mLandPoses[mPlayerNum];
        ObjOrientation orient = mPlayerNum == 0 ? eRight : eLeft;
        
        role->setPosition(CCPoint(pt.x, pt.y));
        role->setOrient(orient);
        if (pet)
        {
            pet->setPosition(CCPoint(pt.x, pt.y + 50));
            pet->setOrient(orient);
        }
    }
    
    PvpPlayerDetail& state = mPlayerDetails[mPlayerNum];
    mPlayerNum++;
    
    state.mRoleId = role->mRoleInfo.mRoleId;
    state.mCurHp = 0;
    state.mSessionId = player->getSessionId();
    state.mState = ePvpPlayerAlive;
}

