//
//  WorldRetinue.cpp
//  GameSrv
//
//  Created by ZengHongru on 15/6/28.
//
//

#include "WorldRetinue.h"
#include "DataCfg.h"

WorldRetinueMgr g_WorldRetinueMgr;

WorldRetinue::WorldRetinue():WorldCreature(eWorldObjectRetinue){}

WorldRetinue* WorldRetinue::create(const WorldRetinueInfo& retinueInfo, WorldRole* owner)
{
    WorldRetinue* retinue = new WorldRetinue;
    if (!retinue->init(retinueInfo, owner)) {
        delete retinue;
        retinue = NULL;
    }
    return retinue;
}

 bool WorldRetinue::init(const WorldRetinueInfo& retinueInfo, WorldRole* owner)
{
    mRetinueInfo = retinueInfo;
    setSkills(retinueInfo.mSkills);
    
    RetinueModDef* modDef = RetinueCfg::getRetinueModCfg(mRetinueInfo.mRetinuemod);
    if (modDef == NULL) {
        return false;
    }
    RetinueOutline* outline = modDef->getOutlineData(mLvl);
    if (outline == NULL) {
        return false;
    }
    
    mResCfg = ResCfg::getCfg(outline->resID);
    if (mResCfg == NULL) {
        return false;
    }
    return true;
}

void WorldRetinue::update(uint64_t ms)
{
    WorldCreature::update(ms);
}

WorldRetinueInfo WorldRetinue::getRetinueInfo()
{
    return mRetinueInfo;
}

bool WorldRetinueMgr::init()
{
    if (!g_WorldObjectMgr.init(eWorldObjectRetinue, 100000))
    {
        return false;
    }
    
    return true;
}

WorldRetinue* WorldRetinueMgr::createRetinue(const WorldRetinueInfo& retinueInfo, WorldRole* owner)
{
    WorldRetinue* retinue = WorldRetinue::create(retinueInfo, owner);
    if (retinue == NULL) {
        return NULL;
    }
    
    retinue->retain();
    mRetinues[retinueInfo.mRetinueId] = retinue;
    return retinue;
}

WorldRetinue* WorldRetinueMgr::getRetinueByRetinueId(int retinueId)
{
    map<int, WorldRetinue*>::iterator iter = mRetinues.find(retinueId);
    if (iter == mRetinues.end())
    {
        return NULL;
    }
    
    return iter -> second;
}

void WorldRetinueMgr::destroyRetinue(WorldRetinue* retinue)
{
    mRetinues.erase(retinue->getRetinueInfo().mRetinueId);
    retinue -> release();
}








