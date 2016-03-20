//
//  RoleAwake.h
//  GameSrv
//
//  Created by pireszhi on 15-4-1.
//
//

#ifndef __GameSrv__RoleAwake__
#define __GameSrv__RoleAwake__

#include "Defines.h"
class Role;
class BattleProp;
class BaseProp;
class RoleAwakeAddtionInfo;

class RoleAwake
{
public:
    
    static RoleAwake* Create(Role* role);
    
    RoleAwake(Role* role):mMaster(role),
                          mFashionCollectAdd(0.0f),
                          mWeaponEnchantsAdd(0.0f)
    {
        
    }
    
    void init();
    int skillReplace(int preSkillId);
    int allSkillReplace();
    int awakeLvlUp();
    
    //计算觉醒自有得附加属性
    void calcAwakeAddProp(BattleProp& bat, BaseProp& base);
    
//    void weaponEnchantsAddRatio(BattleProp& bat);
//    void fashionCollectAddRatio(BattleProp& bat);
    
    float getFashionPropAddRatio();
    float getEnchantsAddRatio();
    
private:
    int onSkillReplace(int preSkillId, int newSkillId);
    void setBatPropAdd();
    void onSetBatPropAdd(RoleAwakeAddtionInfo& info);
    void setRatioAdd(RoleAwakeAddtionInfo& info);
private:
    Role* mMaster;
    BattleProp mBatProp;
    
    float mFashionCollectAdd;    //时装收集属性加成
    float mWeaponEnchantsAdd;    //武器附魔加成
};

#endif