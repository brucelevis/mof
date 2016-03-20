//
//  WorldBase.cpp
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#include "WorldBase.h"
#include <math.h>


float pointDistance(const CCPoint& p1, const CCPoint& p2)
{
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    
    return sqrtf(dx * dx + dy * dy);
}