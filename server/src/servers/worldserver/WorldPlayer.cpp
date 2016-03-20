//
//  WorldPlayer.cpp
//  GameSrv
//
//  Created by xinyou on 14-4-1.
//
//

#include "WorldPlayer.h"
#include "WorldScene.h"

#include "main.h"
#include "msg.h"

#include "DataCfg.h"

WorldPlayerMgr g_WorldPlayerMgr;




extern obj_pos_info getSceneObjectPosInfo(SceneObject* obj);
extern obj_worldrole_info getRoleInfo(WorldRole* role);
extern obj_worldpet_info getPetInfo(WorldPet* pet);

void WorldPlayer::enterScene(WorldScene* scene)
{
    if (mRole)
    {
        mRole->enterScene(scene);
    }
    
    if (mRetinue)
    {
        mRetinue -> enterScene(scene);
    }
    
    for (WorldPetIter iter = mPetList.begin(); iter != mPetList.end(); ++iter)
    {
        WorldPet* pet = *iter;
        pet->enterScene(scene);
    }
    mScene = scene;
}


void WorldPlayer::leaveScene()
{
    setReady(false);
    
    notify_worldplayer_leave_scene notify;
    if (mRole)
    {
        notify.roleInstId = mRole->getInstId();
        mRole->leaveScene();
    }
    if(mRetinue)
    {
        notify.retinueInstId = mRetinue -> getInstId();
        mRetinue->leaveScene();
    }
    for (WorldPetIter iter = mPetList.begin(); iter != mPetList.end(); ++iter)
    {
        WorldPet* pet = *iter;
        notify.petInstIds.push_back(pet->getInstId());
        pet->leaveScene();
    }
    
    mScene->broadcastPacket(&notify);
    mScene = NULL;
}