//
//  WorldRole.h
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#ifndef __GameSrv__WorldRole__
#define __GameSrv__WorldRole__

#include <iostream>
#include <string.h>

#include "WorldBase.h"
#include "WorldCreature.h"
#include "WorldObject.h"
#include "world_cmdmsg.h"

class WorldSceneInst;
class WorldPet;
class WorldPlayer;




class WorldRole : public WorldCreature
{
public:
    WorldRole() : WorldCreature(eWorldObjectRole)
    {
        
    }
    
    bool init(const WorldRoleInfo& roleInfo)
    {
        mRoleInfo = roleInfo;
        setAreaId(0);
        setMaxHp(roleInfo.mMaxHp);
        setAtk(roleInfo.mAtk);
        setDef(roleInfo.mDef);
        setHit(roleInfo.mHit);
        setDodge(roleInfo.mDodge);
        setCri(roleInfo.mCri);
        setLvl(roleInfo.mLvl);
        
        setHp(getMaxHp());
        setMoveSpeed(ROLE_WALK_SPEED);
        setSkills(roleInfo.mSkills);
        setTriggerSkill(roleInfo.mTriggerSkill, roleInfo.mTriggerSkillPro);
        //setTriggerSkill(450101, 1.0f);
        
        mDeathState = eDeathStateAlive;
        
        RoleTpltCfg* tpltCfg = RoleTpltCfg::getCfg(mRoleInfo.mRoleType);
        assert(tpltCfg);
        
        mTpltCfg = tpltCfg;
        mResCfg = ResCfg::getCfg(tpltCfg->mResID);
        assert(mResCfg);
        
        
        return true;
    }
    
    void update(uint64_t ms)
    {
        WorldCreature::update(ms);
    }
    
    WorldRetinueInfo getWorldRetinueInfo()
    {
        return mRetinueInfo;
    }
    
    virtual bool preEnterScene(WorldScene* scene);
    virtual void onEnterScene();
    virtual void onLeaveScene();
    
    virtual void updateAlive(uint64_t ms);
    virtual void onDie(uint64_t ms);
    virtual void updateDead(uint64_t ms);
    
    virtual float getAttackMoveTime();
    virtual int getAttackMoveDistance();
    virtual float getLastAttackHitBackTime();
    virtual int getLastAttackHitBackDistance();
    
    WorldRetinueInfo mRetinueInfo;
    WorldRoleInfo mRoleInfo;
    RoleTpltCfg* mTpltCfg;
    WorldPlayer* mPlayer;
};

class WorldRoleMgr
{
public:
    bool init()
    {
        if (!g_WorldObjectMgr.init(eWorldObjectRole, 100000))
        {
            return false;
        }
        
        return true;
    }
    
    WorldRole* getRoleByRoleId(int roleId)
    {
        map<int, WorldRole*>::iterator iter = mRoles.find(roleId);
        if (iter == mRoles.end())
        {
            return NULL;
        }
        
        return iter->second;
    }
    
    WorldRole* createRole(const WorldRoleInfo& roleInfo)
    {
        WorldRole* role = new WorldRole;
        role->init(roleInfo);
        
        role->retain();
        mRoles[roleInfo.mRoleId] = role;
        return role;
    }
    
    void destroyRole(WorldRole* role)
    {
        CheckCondition(role, return);
        
        mRoles.erase(role->mRoleInfo.mRoleId);
        role->release();
    }
    
    //角色id对应角色
    map<int, WorldRole*> mRoles;
};

extern WorldRoleMgr g_WorldRoleMgr;

#endif /* defined(__GameSrv__WorldRole__) */
