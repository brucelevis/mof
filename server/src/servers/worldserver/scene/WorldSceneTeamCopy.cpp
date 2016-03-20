//
//  WorldTeamCopy.cpp
//  GameSrv
//
//  Created by xinyou on 14-7-17.
//
//

#include "WorldSceneTeamCopy.h"
#include "msg.h"

#include "WorldMonster.h"
#include "WorldPlayer.h"
#include "WorldCreatureAI.h"
#include "world_cmdmsg.h"
#include "main.h"

#define EVENT_WORLD_SCENE_NEXT_AREA 1
//
// pvp场景五个状态：场景初始化，战斗倒计时，战斗开始，战斗结束，场景收尾
//
enum WorldScenePvpState
{
    eWorldSceneTeamCopyInit,
    eWorldSceneTeamCopyCountDown,
    eWorldSceneTeamCopyBattleStart,
    eWorldSceneTeamCopyBattleEnd,
    eWorldSceneTeamCopyEnd,
};


WorldSceneTeamCopy::WorldSceneTeamCopy()
{
}

WorldSceneTeamCopy::~WorldSceneTeamCopy()
{
    
}

void WorldSceneTeamCopy::onDeinit()
{
    vector<WorldMonster*>::iterator iter;
    for (iter = mMonsters.begin(); iter != mMonsters.end(); iter++) {
        WorldMonster* monster = *iter;
        monster->release();
    }
    mMonsters.clear();
}

void WorldSceneTeamCopy::onInit()
{
    mWaitCountDownTime = 3000;
    mBattleTime = 3600000;
    mCountDownTime = 4000;
    mWaitEndTime = 0;
    mSubStateTime = 0;
    mSubState = eWorldSceneTeamCopyInit;
    
    mPlayerCount = 0;
    
    Json::Value value;
    Json::Reader reader;
    reader.parse(mExtendProp, value);
    if (value.isObject()) {
        mTeamId = value["team_id"].asInt();
        mLeaderId = value["leader"].asInt();
    }
}

void WorldSceneTeamCopy::onAddMonster(WorldMonster *monster)
{
    WorldCreatureAI* ai = WorldCreatureAI::create(monster, "monster");
    monster->setAI(ai);
    monster->setGroup(eCreatureMonsterGroup);
    
    if (mSubState == eWorldSceneTeamCopyBattleStart) {
        monster->startAI();
    }
    
    mMonsters.push_back(monster);
}

bool WorldSceneTeamCopy::isLastArea()
{
    return mActiveArea == mAreas.size() - 1;
}

void WorldSceneTeamCopy::onMonsterDead(WorldMonster* monster, WorldCreature *killer)
{
    if (killer == NULL) {
        return;
    }
    
    vector<WorldMonster*>::iterator iter = mMonsters.begin();
 
    if (monster->getMonsterType() == eBoss) {
        while (iter != mMonsters.end()) {
            WorldMonster* iterMonster = *iter;
            if (iterMonster != monster) {
                iterMonster->death(NULL);
            }
            
            iterMonster->stopAI();
            iterMonster->setValid(false);
            iterMonster->release();
            iter++;
        }
        mMonsters.clear();
    } else {
        while (iter != mMonsters.end()) {
            if (*iter == monster) {
                monster->stopAI();
                monster->setValid(false);
                monster->release();
                mMonsters.erase(iter);
                break;
            }
            iter++;
        }
    }
    
    if (mMonsters.empty()) {
        if (isLastArea()) {
            endBattle(true);
        } else {
            scheduleOnce(schedule_selector(WorldSceneTeamCopy::nextArea), 1);
            //nextArea();
        }
    }
}


void WorldSceneTeamCopy::nextArea(uint64_t ms)
{
    list<WorldPlayer*>::iterator iter;
    for (iter = mPlayers.begin(); iter != mPlayers.end(); iter++) {
        WorldPlayer* player = *iter;
        
        WorldRole *role = player->mRole;
        if (!role->isDead()) {
            role->removeAllSkillAction();
            role->clearState();
        }
        
        WorldPet* pet = player->getFirstPet();
        if (pet && !pet->isDead()) {
            pet->removeAllSkillAction();
            pet->clearState();
        }
    }
    
    activeArea(getActiveArea() + 1);
    
    SceneArea* area = getArea(getActiveArea());
    notify_next_scene_area notify;
    CCPoint landpos = area->getLandPos(1);
    notify.posx = landpos.x;
    notify.posy = landpos.y;
    broadcastPacket(&notify, false);
}

void WorldSceneTeamCopy::onPlayerReady(WorldPlayer *player)
{
    if (mSubState == eWorldSceneTeamCopyInit) {
        startCountDown();
    }
}

bool WorldSceneTeamCopy::isBattleFail()
{
    list<WorldPlayer*>::iterator iter;
    for (iter = mPlayers.begin(); iter != mPlayers.end(); iter++) {
        WorldPlayer* player = *iter;
        
        if (!player->mRole->isDead()) {
            break;
        }
    }
    
    if (iter == mPlayers.end()) {
        return true;
    }
    
    return false;
}

void WorldSceneTeamCopy::onRoleDead(WorldRole *role, WorldCreature *killer)
{
    WorldPet* pet = role->mPlayer->getFirstPet();
    if (pet && !pet->isDead()) {
        pet->death(NULL);
    }
    
    if (isBattleFail()) {
        endBattle(false);
    }
}

void WorldSceneTeamCopy::onCreatureDead(WorldCreature *creature, WorldCreature *killer)
{
    creature->removeAllSkillAction();
    
    int type = creature->getType();
    switch (type) {
        case eWorldObjectMonster:
            onMonsterDead(dynamic_cast<WorldMonster*>(creature), killer);
            break;
        case eWorldObjectRole:
            onRoleDead(dynamic_cast<WorldRole*>(creature), killer);
            break;
        default:
            break;
    }
}

void WorldSceneTeamCopy::startCountDown()
{
    mSubState = eWorldSceneTeamCopyCountDown;
    mSubStateTime = 0;
    activeArea(0);
}

void WorldSceneTeamCopy::startBattle()
{
    notify_sync_teamcopy_start notify;
    broadcastPacket(&notify, false);
    
    mSubState = eWorldSceneTeamCopyBattleStart;
    mSubStateTime = 0;
    
    for (int i = 0; i < mMonsters.size(); i++) {
        mMonsters[i]->startAI();
    }
}

void WorldSceneTeamCopy::endBattle(bool success)
{
    mSubState = eWorldSceneTeamCopyBattleEnd;
    mSubStateTime = 0;
    
    notify_sync_teamcopy_end notify;
    broadcastPacket(&notify);
    
    create_cmd(WorldTeamCopyResult, cmd);
    cmd->result = success ? 1 : 0;
    cmd->sceneId = getInstId();
    cmd->sceneMod = getSceneModId();
    cmd->teamId = mTeamId;
    sendMessageToGame(0, CMD_MSG, cmd, 0);
}

void WorldSceneTeamCopy::update(uint64_t ms)
{
    mSubStateTime += ms;
    switch (mSubState) {
        case eWorldSceneTeamCopyInit:
            if (mSubStateTime > mWaitCountDownTime) {
                startCountDown();
            }
            break;
        case eWorldSceneTeamCopyCountDown:
            if (mSubStateTime > mCountDownTime) {
                startBattle();
            }
            break;
        case eWorldSceneTeamCopyBattleStart:
            if (mSubStateTime > mBattleTime) {
                endBattle(false);
            } else {
                WorldScene::update(ms);
            }
            break;
        case eWorldSceneTeamCopyBattleEnd:
            pendEnd();
            break;
        default:
            break;
    }
    
}


void WorldSceneTeamCopy::onEvent(int eventId, long lParam, long rParam)
{
}

void WorldSceneTeamCopy::onAddedPlayer(WorldPlayer* player)
{
    WorldPet* pet = player->getFirstPet();
    WorldRole* role = player->getRole();
    WorldRetinue *retinue = player->getRetinue();
    if (role) {
        CCPoint pt = getArea(0)->getLandPos(1);
        role->setPosition(pt);
        role->setGroup(mLeaderId);
        role->setOrient(eRight);
    }
    
    if (pet) {
        pet->setGroup(mLeaderId);
        pet->setPosition(role->getPosition());
        pet->setOrient(eRight);
    }
    
    if(retinue){
        retinue->setGroup(mLeaderId);
        retinue->setOrient(eRight);
    }
}

void WorldSceneTeamCopy::beforePlayerLeave(WorldPlayer *player)
{
}

void WorldSceneTeamCopy::afterPlayerLeave()
{
    if (getValid() && !mWaitingEnd) {
        if (isBattleFail()) {
            endBattle(false);
        }
    }
}
