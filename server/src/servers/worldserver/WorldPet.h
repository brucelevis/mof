//
//  WorldPet.h
//  GameSrv
//
//  Created by prcv on 14-2-26.
//
//

#ifndef __GameSrv__WorldPet__
#define __GameSrv__WorldPet__

#include <iostream>

#include "WorldMonster.h"
#include "world_cmdmsg.h"
#include "DataCfg.h"

class WorldRole;
class WorldPlayer;

// modified by jianghan
//class WorldPet : public WorldCreature
class WorldPet : public WorldMonster
// end modify
{
public:
    // modified by jianghan
    //WorldPet() : WorldCreature(eWorldObjectPet)
    //{
    //}
    WorldPet() : WorldMonster(eWorldObjectPet)
    {
    }
    
    static WorldPet* create(const WorldPetInfo& petInfo, WorldRole* owner = NULL)
    {
        WorldPet* pet = new WorldPet;
        if (!pet->init(petInfo, owner)) {
            delete pet;
            pet = NULL;
        }
        
        return pet;
    }
    
    
    bool init(const WorldPetInfo& petInfo, WorldRole* owner)
    {
        mPetInfo = petInfo;
        
        // modified by jianghan
        // 这里需要应用怪物配置先将宠物的各项数值设置好，然后才应用 petInfo 部分
        PetCfgDef* petDef = PetCfg::getCfg(mPetInfo.mPetMod);
        CheckCondition(petDef, return false);
        MonsterCfgDef* monsterDef = MonsterCfg::getCfg(petDef->getMonsterTpltID());
        CheckCondition(monsterDef, return false);
        mResCfg = ResCfg::getCfg(monsterDef->res_id);
        CheckCondition(mResCfg, return false);
        
        setSkills(petInfo.mSkills);
        
        // added by jianghan 需要初始化很多怪物设定参数，这两项必须要
        InitBasicAttributes(monsterDef);
        InitOtherAttributes(monsterDef);
        
        // 然后加载个性部分
        setLvl(petInfo.mLvl);
        setMaxHp(petInfo.mMaxHp);
        setAtk(petInfo.mAtk);
        setDef(petInfo.mDef);
        setHit(petInfo.mHit);
        setDodge(petInfo.mDodge);
        setCri(petInfo.mCri);
        setMoveSpeed(petInfo.mMoveSpeed);
        
        setHp(getMaxHp());
        mDeathState = eDeathStateAlive;
        // end modify
        
        return true;
    }
    
    virtual void update(uint64_t ms)
    {
        WorldCreature::update(ms);
    }
    
    //
    virtual void updateAlive(uint64_t ms);
    virtual void onDie(uint64_t ms);
    virtual void updateDead(uint64_t ms);
    
    virtual void onEnterScene();
    virtual void onLeaveScene();
    
    int mPetState;
    
    WorldPetInfo mPetInfo;
    WorldPlayer* mPlayer;
};


class WorldPetMgr
{
public:
    bool init()
    {
        if (!g_WorldObjectMgr.init(eWorldObjectPet, 100000))
        {
            return false;
        }
        
        return true;
    }
    
    
    WorldPet* createPet(const WorldPetInfo& petInfo, WorldRole* owner)
    {
        WorldPet* pet = WorldPet::create(petInfo, owner);
        if (pet == NULL) {
            return NULL;
        }

        pet->retain();
        mPets[petInfo.mPetId] = pet;
        return pet;
    }
    
    WorldPet* getPetByPetId(int petId)
    {
        map<int, WorldPet*>::iterator iter = mPets.find(petId);
        if (iter == mPets.end())
        {
            return NULL;
        }
        
        return iter->second;
    }
    
    void destroyPet(WorldPet* pet)
    {
        mPets.erase(pet->mPetInfo.mPetId);
        pet->release();
    }
    
    map<int, WorldPet*> mPets;
};

extern WorldPetMgr g_WorldPetMgr;

#endif /* defined(__GameSrv__WorldPet__) */
