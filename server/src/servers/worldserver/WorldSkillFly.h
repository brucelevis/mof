//
//  WorldSkillFly.h
//  GameSrv
//
//  Created by prcv on 14-4-16.
//
//

#ifndef __GameSrv__WorldSkillFly__
#define __GameSrv__WorldSkillFly__

#include <iostream>
#include "WorldSceneObject.h"
#include "WorldObject.h"

class WorldSkillFly : public SceneObject
{
public:
    static bool globalInit(int maxNpcNum);
public:
    WorldSkillFly() : SceneObject(eWorldObjectSkillFly)
    {
    }
    
    ~WorldSkillFly()
    {
    }
    
    static WorldSkillFly* create()
    {
        WorldSkillFly* fly = new WorldSkillFly;
        return fly;
    }
    
    
};



#endif /* defined(__GameSrv__WorldSkillFly__) */
