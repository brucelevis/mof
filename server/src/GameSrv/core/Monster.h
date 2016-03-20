//
//  Monster.h
//  GameSrv
//
//  Created by pireszhi on 13-7-16.
//
//

#ifndef __GameSrv__Monster__
#define __GameSrv__Monster__

#include <iostream>

#include "Obj.h"
#include "DataCfg.h"
#include "msg.h"

class Monster : public Obj
{
public:
    Monster();
    void MonsterInit(MonsterCfgDef* monsterCfg);
};

#endif