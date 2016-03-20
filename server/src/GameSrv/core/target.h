//
//  action.h
//  GameSrv
//
//  Created by prcv on 13-12-2.
//
//

#ifndef __GameSrv__target__
#define __GameSrv__target__

#include <iostream>
#include "EnumDef.h"


struct TargetProgress
{
    bool  mIsDone;
    int  mProgress;
};

class BaseTarget
{
public:
    int mActionType;
    virtual bool trigger(int param1, int param2) { return 0;}
    
    virtual void update(int param1, int param2, TargetProgress& progress){}
    
    virtual ~BaseTarget() {}
};


BaseTarget* createTarget(int type, int param1, int param2);



#endif /* defined(__GameSrv__action__) */
