//
//  action.cpp
//  GameSrv
//
//  Created by prcv on 13-12-2.
//
//


#include "target.h"

class RechargeTarget : public BaseTarget
{
public:
    RechargeTarget(int val, int param2)
    {
        mActionType = eTargetType_Recharge;
        mVal = val;
    }
    
    virtual bool trigger(int val, int param2)
    {
        return (mVal <= val);
    }
    
    virtual void update(int val, int param2, TargetProgress& progress)
    {
        if (val >= mVal)
        {
            progress.mIsDone = true;
        }
    }
    
    int mVal;
};


class ComposeLvlStoneTarget : public BaseTarget
{
public:
    ComposeLvlStoneTarget(int lvl, int param2)
    {
        mActionType = eTargetType_ComposeLvlStone;
        mLvl = lvl;
    }
    
    bool trigger(int lvl, int param2)
    {
        return (mLvl == lvl);
    }
    
    virtual void update(int val, int param2, TargetProgress& progress)
    {
        if (val >= mLvl)
        {
            progress.mIsDone = true;
        }
    }
    
    int mLvl;
};

BaseTarget* createTarget(int type, int param1, int param2)
{
    BaseTarget* target = NULL;
    switch (type) {
        case eTargetType_ComposeLvlStone:
            target = new ComposeLvlStoneTarget(param1, param2);
            break;
        case eTargetType_Recharge:
            target = new RechargeTarget(param1, param2);
            break;
            
        default:
            break;
    }
    
    return target;
}
