//
//  BossFactory.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-9-27.
//
//

#include "BossFactory.h"
#include "GameLog.h"
#include "WorldBossNew.h"
#include "GuildBoss.h"

BossFactory SBossFactory;



Boss* BossFactory::create(enumBossType type)
{
    Boss* boss = NULL;
    
    switch (type) {
        case eBossType_WorldBoss:
        {
            boss =  new WorldBossNew();
            break;
        }
        case eBossType_GuildBoss:
        {
            boss =  new GuildBoss();
            break;
        }
            
        default:
            break;
    }
    
    if(boss)
    {
        mBossMap[boss->GetBossId()] = boss;
    }
    
    return boss;
}

void BossFactory::release(int bossid)
{
    Boss* boss =  getBoss(bossid);
    if(boss)
    {
        boss->Destroy();//由ActSceneManager来destroy  
    }
    
    mBossMap.erase(bossid);
}

Boss* BossFactory::getBoss(int bossid)
{
    BossMap::iterator it = mBossMap.find(bossid);
    if(mBossMap.end() == it)
        return NULL;
    return it->second;
}
