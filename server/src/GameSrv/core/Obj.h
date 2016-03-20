//
//  Obj.h
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//  战斗模块做在客户端

#ifndef __GameSrv__Obj__
#define __GameSrv__Obj__

#include "Defines.h"
#include <vector>
#include <queue>
#include "process.h"

using namespace std;


enum ObjectType
{
    kObjectRole = 0,
    kObjectRobot = 1,
    kObjectPet = 2,
};

inline int64_t MAKE_OBJ_ID(int type, int objIdx)
{
    return ((int64_t)type << 32) + objIdx;
}

inline int getObjectType(int64_t objId)
{
    int type = (objId >> 32);
    if (type == kObjectRole && objId < 10000) {
        return kObjectRobot;
    }

    return type;
}

inline int getObjectLowId(int64_t objId)
{
    return objId & 0xFFFFFFFF;
}

class Scene;class SkillMod;class Skill;
/*
 Obj的id段划分规则：（不分模板id和实例id都一样划分，但两者数值不一样->instID不等于tpltID）
 100000－999999 6位数给玩家的角色 - 共90万，实例id数据库保存，每个不可重复
 10000－99999 5位数给怪物 - 共9万，模板id唯一，为所有场景服务，实例id不同场景中可以重复
 1000－9999 4位数给npc - 共9千，使用方法同怪物
 
 与服务器沟通一至采用instID
 */

class Obj : public BattleProp, public Properties{
    /*
     runtime dynamic values
     */
    READWRITE(int, mInstID, InstID)
    READWRITE(int, mTpltID, TpltID)
    READWRITE(ObjType, mType, Type)
    
    READWRITE(int, mState, State)           // 状态
    READWRITE(int, mWalkSpeed, WalkSpeed)
    READWRITE(Scene*, mScene, Scene)       //
    READWRITE(int, mSceneID, SceneID)
    //READONLY(CCPoint, mPos, Pos);
    
    READWRITE(SkillMod*, mSkillMod, SkillMod)
    READWRITE(BattleProp, mRealBattleProp, RealBattleProp)
    
    
    
/*

 configable values
 */
    READWRITE(float, mAtkRange, AtkRange)       //
    READWRITE(int, mMaxFat, MaxFat)
    READWRITE(int, mAtkSpeed, AtkSpeed)       //
    
public:
    Obj(ObjType t);
    virtual ~ Obj();
    virtual void OnHeartBeat(int nBeat);
    
    virtual void update(float dt);
    
    SkillOwner toSkillOwner();
    
    virtual void onEnterScene() {}
    virtual void onLeaveScene() {}
    
    pair<int, float> mHpBuf;
    pair<int, float> mAtkBuf;
    pair<int, float> mDefBuf;
    pair<int, float> mHitBuf;
    pair<int, float> mDodgeBuf;
    pair<int, float> mCriBuf;
private:
    Obj& operator=(const Obj& other);
    Obj(const Obj& other);
    
};




#endif /* defined(__GameSrv__Obj__) */
