//
//  WorldPlayer.h
//  GameSrv
//
//  Created by xinyou on 14-4-1.
//
//

#ifndef __GameSrv__WorldPlayer__
#define __GameSrv__WorldPlayer__

#include <iostream>

#include "WorldObject.h"
#include "WorldPet.h"
#include "WorldRole.h"
#include "WorldRetinue.h"
#include "world_cmdmsg.h"

class WorldPlayer
{
    friend class WorldScene;
public:
    WorldPlayer()
    {
        mPrevSceneMod = 0;
        mScene = NULL;
        mSessionId = 0;
        mReady = false;
        mRetinue = NULL;
    }
    
    ~WorldPlayer()
    {
        if (mRole)
        {
            g_WorldRoleMgr.destroyRole(mRole);
        }
        
        if(mRetinue)
        {
            g_WorldRetinueMgr.destroyRetinue(mRetinue);
        }
        
        for (list<WorldPet*>::iterator iter = mPetList.begin(); iter != mPetList.end(); ++iter)
        {
            WorldPet* pet = *iter;
            g_WorldPetMgr.destroyPet(pet);
        }
    }
    
    bool init(const WorldPlayerInfo& info)
    {
        mPetList.clear();
        mRole = NULL;
        mRetinue = NULL;
        
        mSessionId = info.mSessionId;
        mRoleId = info.mRoleId;
        mExtendProp = info.mProps;
        
        
        WorldRole* role = NULL;
        if (info.mRoleInfo.mRoleId > 0)
        {
            role = g_WorldRoleMgr.getRoleByRoleId(info.mRoleInfo.mRoleId);
            if (role)
            {
                log_warn(__FILE__ << __FUNCTION__ << __LINE__);
                g_WorldRoleMgr.destroyRole(role);
            }
            role = g_WorldRoleMgr.createRole(info.mRoleInfo);
            mRole = role;
            mRole->mPlayer = this;
        }
        
        WorldRetinue *retinue = NULL;
        if (info.mRetinueInfo.mRetinueId > 0){
            retinue = g_WorldRetinueMgr.getRetinueByRetinueId(info.mRetinueInfo.mRetinueId);
            if(retinue){
                g_WorldRetinueMgr.destroyRetinue(retinue);
            }
            retinue = g_WorldRetinueMgr.createRetinue(info.mRetinueInfo, role);
            if(retinue){
                mRetinue = retinue;
                retinue -> mPlayer = this;
            }
        }
        
        for (int i = 0; i < info.mPetInfos.size(); i++)
        {
            WorldPet* pet = NULL;
            pet = g_WorldPetMgr.getPetByPetId(info.mPetInfos[i].mPetId);
            if (pet)
            {
                log_warn(__FILE__ << __FUNCTION__ << __LINE__);
                g_WorldPetMgr.destroyPet(pet);
            }
            pet = g_WorldPetMgr.createPet(info.mPetInfos[i], role);
            if (pet == NULL) {
                continue;
            }
            addPet(pet);
            
            pet->mPlayer = this;
        }
        
        return true;
    }
    
    static WorldPlayer* create(const WorldPlayerInfo& info)
    {
        WorldPlayer* player = new WorldPlayer;
        player->init(info);
        return player;
    }
    
    void addPet(WorldPet* pet)
    {
        mPetList.push_back(pet);
    }
    
    WorldPet* getPet(int instId)
    {
        for (WorldPetIter iter = mPetList.begin(); iter != mPetList.end(); ++iter)
        {
            WorldPet* pet = *iter;
            if (pet->getInstId() == instId)
            {
                return pet;
            }
        }
        
        return NULL;
    }
    
    WorldPet* getFirstPet()
    {
        if (mPetList.empty())
        {
            return NULL;
        }
        
        return mPetList.front();
    }
    
    WorldRole* getRole()
    {
        return mRole;
    }
    
    WorldRetinue* getRetinue()
    {
        return mRetinue;
    }
    
    int getSessionId()
    {
        return mSessionId;
    }
    
    WorldScene* getScene()
    {
        return mScene;
    }
    
    void setReady(bool isReady = true)
    {
        mReady = isReady;
        if (mScene) {
            mScene->onPlayerReady(this);
        }
    }
    
    bool getReady()
    {
        return mReady;
    }
    
    int getPlayerId()
    {
        return mRoleId;
    }
    
   
    
private:
    void enterScene(WorldScene* scene);
    void leaveScene();
    void changeScene(WorldScene* scene);
    
    
public:
    typedef list<WorldPet*> WorldPetList;
    typedef list<WorldPet*>::iterator WorldPetIter;
    
    int mPrevSceneMod;
    WorldScene* mScene;
    int mSessionId;
    bool mReady;
    WorldRole* mRole;
    list<WorldPet*> mPetList;
    WorldRetinue* mRetinue;
    int mRoleId;
    int mPlayerIndex;
    string mExtendProp;
};


class WorldPlayerMgr
{
public:
    bool init()
    {
        
        return true;
    }
    
    WorldPlayer* createPlayer(const WorldPlayerInfo& playerInfo)
    {
        WorldPlayer* player = WorldPlayer::create(playerInfo);
        addPlayer(player);
        
        return player;
    }
    
    void destroyPlayer(WorldPlayer* player)
    {
        CheckCondition(player, return)
        removePlayer(player->getSessionId());
        delete player;
    }
    
    void destroyPlayerBySessionId(int sessionId)
    {
        WorldPlayer* player = removePlayer(sessionId);
        CheckCondition(player, return)
        delete player;
    }
    
    void addPlayer(WorldPlayer* player)
    {
        mPlayers.insert(make_pair(player->getSessionId(), player));
    }
    
    WorldPlayer* removePlayer(int sessionId)
    {
        map<int, WorldPlayer*>::iterator iter = mPlayers.find(sessionId);
        if (iter == mPlayers.end())
        {
            return NULL;
        }
        
        WorldPlayer* player = iter->second;
        mPlayers.erase(iter);
        
        return player;
    }
    
    WorldPlayer* getPlayer(int sessionId)
    {
        
        map<int, WorldPlayer*>::iterator iter = mPlayers.find(sessionId);
        if (iter == mPlayers.end())
        {
            return NULL;
        }
        
        return iter->second;
    }
    
    WorldPlayer* getWorldRoleBySessionId(int sessionId)
    {
        
        map<int, WorldPlayer*>::iterator iter = mPlayers.find(sessionId);
        if (iter == mPlayers.end())
        {
            return NULL;
        }
        
        return iter->second;
    }
    
    //会话id对应角色
    map<int, WorldPlayer*> mPlayers;
};

extern WorldPlayerMgr g_WorldPlayerMgr;

#endif /* defined(__GameSrv__WorldPlayer__) */
