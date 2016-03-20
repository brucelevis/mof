//
//  WorldSceneTreasureFight.h
//  GameSrv
//
//  Created by xinyou on 14-5-12.
//
//

#ifndef __GameSrv__WorldSceneTreasureFight__
#define __GameSrv__WorldSceneTreasureFight__

#include <iostream>

#include "WorldScene.h"
#include "WorldMonster.h"

enum WorldSceneTreasureFightState
{
    eWorldSceneTreasureFightStart,
    eWorldSceneTreasureFightBattle,
    eWorldSceneTreasureFightEnd,
};

class TreasureFightPlayer
{
public:
    WorldPlayer* mPlayer;
    
    int mKillPlayerNum;
    int mAssistPlayerNum;
    int mKillBoxNum;
    int mAssistBoxNum;
    int mScore;
    int mGuildId;
    
    TreasureFightPlayer()
    {
        mKillBoxNum = 0;
        mAssistBoxNum = 0;
        mScore = 0;
        mGuildId = 0;
        mKillPlayerNum = 0;
        mAssistPlayerNum = 0;
        mPlayer = NULL;
    }
    
    ~TreasureFightPlayer()
    {
        
    }
    
    void init(WorldPlayer* player)
    {
        mKillBoxNum = 0;
        mAssistBoxNum = 0;
        mScore = 0;
        mGuildId = 0;
        mKillPlayerNum = 0;
        mAssistPlayerNum = 0;
        mPlayer = player;
    }
};

class WorldSceneTreasureFight : public WorldScene
{
public:
    WorldSceneTreasureFight();
    ~WorldSceneTreasureFight();
    
    virtual void onInit();
    virtual void onDeinit();
    
    virtual void onAddedPlayer(WorldPlayer* player);
    virtual void onAddMonster(WorldMonster* npc);
    
    virtual void onCreatureDead(WorldCreature* victim, WorldCreature* killer);
    virtual void beforePlayerLeave(WorldPlayer* player);
    
    TreasureFightPlayer* getTreasureFightPlayer(int playerId);
    void removeTreasureFightPlayer(int playerId);
    
    //角色被杀积分处理
    void onRoleDead(WorldRole* player, WorldCreature* killer);
    //箱子被杀积分处理
    void onMonsterKilled(WorldMonster* monster, WorldCreature* killer);
    //初始化箱子
    void initMonsters();
    //复活已经死亡的箱子
    void refreshMonsters(uint64_t ms);
    //遍历战斗玩家数据
    void traverseFightPlayer(TraverseCallback cb, void* param);
    //获取助攻玩家
    void getKillAssists(WorldCreature* victim, WorldRole* killer, set<TreasureFightPlayer*>& players);

    //协助击杀玩家获取的积分比例
    float mPlayerAssistRate;
    //协助击杀箱子获取的积分比例
    float mMonsterAssistRate;
    
    //箱子模版
    int mMonsterMod;
    //箱子数量
    int mMonsterNum;
    //杀箱子能获取的积分
    int mMonsterScore;
    //杀人积分倍数
    float mPlayerScoreRate;
    //所有箱子
    vector<WorldMonster*> mMonsters;
    //公会夺宝战玩家相关数据
    map<int, TreasureFightPlayer*> mFightPlayers;
    
public:
    
    
    static int produceScoreByMonster(WorldMonster* monster, int def);
    static int produceScoreByRole(WorldRole* role, int def);
    static float getRoleReduce(WorldRole* killer, WorldRole* victim, float def);
};


#endif
