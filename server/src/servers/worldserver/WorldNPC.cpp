//
//  WorldNPC.cpp
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#include "WorldNPC.h"
#include "WorldCreatureAI.h"

#include "DataCfg.h"


bool WorldNPC::globalInit(int maxNpcNum)
{
    g_WorldObjectMgr.init(eWorldObjectNPC, maxNpcNum);
    return true;
}


WorldNPC::WorldNPC() : WorldCreature(eWorldObjectNPC)
{
}

WorldNPC::~WorldNPC()
{
    deinit();
}

WorldNPC* WorldNPC::create(int npcId, void* extParam)
{
    MonsterCfgDef* monsterDef = MonsterCfg::getCfg(npcId);
    if (monsterDef == NULL) {
        return NULL;
    }
    
    WorldNPC* npc = new WorldNPC();
    npc->setModId(npcId);
    if (monsterDef->monsterType)
    
    if (!npc->init(monsterDef, extParam)) {
        delete npc;
        npc = NULL;
    }
    return npc;
}

bool WorldNPC::init(MonsterCfgDef* monsterDef, void* extParam)
{
    mResCfg = ResCfg::getCfg(monsterDef->res_id);
    if (mResCfg == NULL) {
        return false;
    }
    
    setGroup(eCreatureMonsterGroup);
    
    setMaxHp(monsterDef->getHp());
    setHp(monsterDef->getHp());
    setDodge(monsterDef->getDodge());
    setHit(monsterDef->getHit());
    setAtk(monsterDef->getAtk());
    setDef(monsterDef->getDef());
    
    setIsUnhurtState(monsterDef->isArmor);
    setActionMoveSpeed(monsterDef->getWalkSpeed());
    
    if (extParam != NULL) {
        Json::Value value;
        Json::Reader reader;
        if (reader.parse((char*)extParam, value) && value.isObject()) {
            mAIType = value["ai"].asString();
        }
    }
    
    mInitialed = true;
    
    return true;
}


bool WorldNPC::deinit()
{
    if (!mInitialed) {
        return false;
    }
    
    mInitialed = false;
    return true;
}
