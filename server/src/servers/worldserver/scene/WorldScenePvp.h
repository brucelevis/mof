//
//  WorldScenePvp.h
//  GameSrv
//
//  Created by prcv on 14-3-20.
//
//

#ifndef __GameSrv__WorldScenePvp__
#define __GameSrv__WorldScenePvp__

#include <iostream>

#include "WorldScene.h"
#include "WorldRole.h"


struct PvpPlayerDetail
{
    int mSessionId;
    int mRoleId;
    int mCurHp;
    int mState;
};

//
// pvp场景五个状态：场景初始化，战斗倒计时，战斗开始，战斗结束，场景收尾
//
enum WorldScenePvpState
{
    eWorldScenePvpInit,
    eWorldScenePvpCountDown,
    eWorldScenePvpBattleStart,
    eWorldScenePvpBattleEnd,
    eWorldScenePvpEnd,
};

class WorldScenePvp : public WorldScene
{
public:
    WorldScenePvp();
    
    ~WorldScenePvp();
    
    virtual void onInit();
    
    //添加玩家
    virtual bool preAddPlayer(WorldPlayer* player);
    virtual void onAddedPlayer(WorldPlayer* player);
    
    void update(uint64_t ms);
    void onCreatureDead(WorldCreature* victim, WorldCreature* killer);
    void beforePlayerLeave(WorldPlayer* player);
    
    void startCountDown();
    void startBattle();
    void endBattle(int reason);
    
    int             mMaxPlayerNum;
    int             mPlayerNum;
    PvpPlayerDetail mPlayerDetails[2];
    
    //在当前状态消耗的时间
    int64_t mSubStateTime;
    
    // 倒计时时间
    int64_t mCountDownTime;
    // 战斗时间
    int64_t mMaxFightTime;
    // 结束后缓冲时间
    int64_t mEndingTime;
    // pvp状态
    int mSubState;
};


#endif /* defined(__GameSrv__WorldScenePvp__) */
