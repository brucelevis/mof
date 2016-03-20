//
//  Monster.cpp
//  GameSrv
//
//  Created by pireszhi on 13-7-16.
//
//

#include "Monster.h"
#include "hander_include.h"
#include "Activity.h"

Monster::Monster():Obj(eMonster)
{
    mLvl = 0;
    mHp = 0;
    mMaxHp = 0;
    mDef = 0;
    mHit = 0;
    mAtk = 0;
    mDodge = 0;
}
void Monster::MonsterInit(MonsterCfgDef* monsterCfg)
{
    //初始化基本属性
    if (monsterCfg) {
        setLvl(monsterCfg->getLvl());
        setHp(monsterCfg->getHp());
        setMaxHp(monsterCfg->getHp());
        setHit(monsterCfg->getHit());
        setDef(monsterCfg->getDef());
        setAtk(monsterCfg->getAtk());
        setDodge(monsterCfg->getDodge());
    }
}


