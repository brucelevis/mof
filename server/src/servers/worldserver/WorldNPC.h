//
//  WorldNPC.h
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#ifndef __GameSrv__WorldNPC__
#define __GameSrv__WorldNPC__

#include <iostream>
#include "WorldBase.h"
#include "WorldCreature.h"

struct MonsterCfgDef;

enum WorldNPCType
{
    eWorldBox,
    eWorldMonster,
    eWorldBoss
};

class WorldNPC : public WorldCreature
{
public:
    static bool globalInit(int maxNpcNum);
    
public:
    WorldNPC();
    ~WorldNPC();
    
    static WorldNPC* create(int npcId, void* extParam);
    
    virtual bool init(MonsterCfgDef* monsterDef, void* extParam);
    virtual bool deinit();
    
    int mInitialed;
    int mNpcType;
    
    string mAIType;
};

extern WorldObjectMgr g_WorldNPCMgr;


#endif /* defined(__GameSrv__WorldNPC__) */
