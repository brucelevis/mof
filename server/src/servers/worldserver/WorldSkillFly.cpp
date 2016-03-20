//
//  WorldSkillFly.cpp
//  GameSrv
//
//  Created by prcv on 14-4-16.
//
//

#include "WorldSkillFly.h"
WorldObjectMgr g_WorldSkillFlyMgr;



bool WorldSkillFly::globalInit(int maxNpcNum)
{
    g_WorldObjectMgr.init(eWorldObjectSkillFly, maxNpcNum);
    return true;
}
