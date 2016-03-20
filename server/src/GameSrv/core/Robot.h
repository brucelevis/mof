//
//  Robot.h
//  GameSrv
//
//  Created by prcv on 14-7-2.
//
//

#ifndef __GameSrv__Robot__
#define __GameSrv__Robot__

#include <iostream>

#include "Defines.h"
#include "ItemBag.h"
#include <vector>
#include <map>
#include <string>
using namespace std;

struct redisContext;

class RobotCfgDef
{
public:
    BaseProp baseprop;
    BattleProp batprop;
    
    vector<vector<int> > skills;
    vector<vector<int> > equips;
    
    vector<int> stones;
    int         star;
    
    int startId;
    int endId;
    string name;
    int roletype;
};

class RobotCfg
{
public:
    typedef pair<int, int> IdInterval;
    
    static int sFriendId;
    static RobotCfgDef* getCfg(int id);
    static bool load(const char* file);
    static RobotCfgDef* sDummyCfgData;
    static vector<pair<IdInterval, RobotCfgDef*> > sCfgDatas;
};

class Robot
{
public:
    int mId;
    string mRoleName;
    int mRoleType;
    int mBattle;
    
    vector<int> mSkills;
    BaseProp mBaseProp;
    BattleProp mBattleProp;
    ItemArray mEquips;
    
    static Robot* create(const char* info);
};

class RobotMgr
{
public:
    
    bool load();
    Robot* getData(int robotId);
    Robot* getData(const char* name);
    map<int, Robot*> mDatas;
    map<string, Robot*> mNameIndices;
};

extern RobotMgr g_RobotMgr;

#endif /* defined(__GameSrv__Robot__) */
