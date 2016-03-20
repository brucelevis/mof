//
//  WorldRetinue.h
//  GameSrv
//
//  Created by ZengHongru on 15/6/28.
//
//

#ifndef GameSrv_WorldRetinue_h
#define GameSrv_WorldRetinue_h

#include "WorldCreature.h"
#include "world_cmdmsg.h"

class WorldRetinueMgr;

extern WorldRetinueMgr g_WorldRetinueMgr;

class WorldRetinue : public WorldCreature
{
public:
    WorldRetinue();
    static WorldRetinue* create(const WorldRetinueInfo& retinueInfo, WorldRole* owner = NULL);
    bool init(const WorldRetinueInfo& retinueInfo, WorldRole* owner);
    virtual void update(uint64_t ms);
    WorldRetinueInfo getRetinueInfo();
    WorldPlayer* mPlayer;
private:
    WorldRetinueInfo mRetinueInfo;
};

class WorldRetinueMgr
{
public:
    bool init();
    WorldRetinue* createRetinue(const WorldRetinueInfo& retinueInfo, WorldRole* owner);
    WorldRetinue* getRetinueByRetinueId(int retinueId);
    void destroyRetinue(WorldRetinue* retinue);
private:
    map<int, WorldRetinue*> mRetinues;
};



#endif
