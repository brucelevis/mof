//
//  GuildShare.cpp
//  ;
//  
//  Created by Huang Kunchao on 13-8-10.
//
//

#include "GuildShare.h"
#include "DataCfgActivity.h"

bool isGuildPosHasManagerRight(int pos, int right)
{
    switch (pos) {
        case kGuildViceMaster:
            if(kGuidlViceMasterRightBegin < right &&
               right < kGuildViceMasterRightEnd)
                return true;
            break;
        case kGuildMaster:
            return true;
    }
    return false;
}


int calGuildLevel(float fortune, float construction)
{
    GuildLvlDef* pGuildLvlDef = GuildCfgMgr::getGuildLvlDef(fortune,construction);
    if(pGuildLvlDef){
        return pGuildLvlDef->lvl;
    }
    return 0;
}

int calGuildMaxMember(int level)
{
    GuildLvlDef* pGuildLvlDef = GuildCfgMgr::getGuildLvlDef(level);
    if(pGuildLvlDef){
        return pGuildLvlDef->memberNum;
    }
    return 0;
}

int calGuildBossLevel(int exp)
{
    GuildBossLvlDef* def = GuildCfgMgr::getGuildBossLvlDef(exp);
    if(def){
        return def->level;
    }
    return 0;
}