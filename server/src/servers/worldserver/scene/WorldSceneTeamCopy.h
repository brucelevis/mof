//
//  WorldSceneTeamCopy.h
//  GameSrv
//
//  Created by xinyou on 14-7-17.
//
//

#ifndef __GameSrv__WorldSceneTeamCopy__
#define __GameSrv__WorldSceneTeamCopy__

#include "WorldScene.h"

class WorldPlayer;
class WorldCreature;
class WorldMonster;

class WorldSceneTeamCopy : public WorldScene
{
public:
    WorldSceneTeamCopy();
    ~WorldSceneTeamCopy();
    
    virtual void update(uint64_t ms);
    virtual void onCreatureDead(WorldCreature* creature, WorldCreature* killer);
    
    virtual void onInit();
    virtual void onDeinit();
    virtual void onEvent(int eventId, long lParam, long rParam);
    
protected:
    
    void onAddedPlayer(WorldPlayer* player);
    void beforePlayerLeave(WorldPlayer* player);
    void afterPlayerLeave();
    void onPlayerReady(WorldPlayer* player);
    
    void onAddMonster(WorldMonster* monster);
    void onMonsterDead(WorldMonster* monster, WorldCreature* killer);
    void onRoleDead(WorldRole* role, WorldCreature* killer);
    
    bool isLastArea();
    bool isMonsterAllDeadInCurrArea();
    bool isBattleFail();
    
    void startCountDown();
    void startBattle();
    void endBattle(bool success = true);
    
    void nextArea(uint64_t ms);
    
private:
    int mSubState;
    int mSubStateTime;
    
    int mWaitCountDownTime;
    int mCountDownTime;
    int mBattleTime;
    int mWaitEndTime;
    
    vector<WorldMonster*> mMonsters;
    
    int mPlayerCount;
    int mTeamId;
    int mLeaderId;
};

#endif /* defined(__GameSrv__WorldTeamCopy__) */
