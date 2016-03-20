//
//  cmd_world_handler.h
//  GameSrv
//
//  Created by prcv on 14-3-10.
//
//

#ifndef __GameSrv__cmd_world_handler__
#define __GameSrv__cmd_world_handler__

#include <iostream>
#include "cmdmsg.h"
#include "cmd_helper.h"
#include "stdint.h"


enum LeaveWorldSceneType
{
    eLeaveWorldSceneNone,
    eLeaveWorldSceneLeaveScene,
    eLeaveWorldSceneLeaveWorld,
    eLeaveWorldSceneDead,
    eLeaveWorldSceneDestroyScene,
};


struct WorldPetInfo
{
    int mPetId;
    int mPetMod;
    
    int mLvl;
    int mMaxHp;
    int mAtk;
    int mDef;
    float mHit;
    float mDodge;
    float mCri;
    
    float mMoveSpeed;
    
    vector<int> mSkills;
};

struct WorldRetinueInfo
{
    int mRetinueId;
    int mLvl;
    int mGroup;
    int mRetinuemod;
    int mOwnerid;
    
    vector<int> mSkills;
};


struct WorldRoleInfo
{
    int mRoleId;
    string mRoleName;
    int mLvl;
    int mRoleType;
    int mVipLvl;
    int mHonorId;
    string mGuildName;
    int mGuildPosition;
    int mWeaponQuality;
    int mBodyFashion;
    int mWeaponFashion;
    int mEnchantId;
    int mEnchantLvl;
    int mAwakeLvl;
    
    int mMaxHp;
    int mAtk;
    int mDef;
    float mHit;
    float mDodge;
    float mCri;
    int mBattle;
    vector<int> mSkills;
    int mTriggerSkill;
    float mTriggerSkillPro;
};

struct WorldPlayerInfo
{
    int mServerId;
    int mSessionId;
    int mRoleId;
    
    WorldRoleInfo mRoleInfo;
    vector<WorldPetInfo> mPetInfos;
    WorldRetinueInfo mRetinueInfo;
    string mProps;
};

begin_cmd(LeaveWorld)
    cmd_member(uint32_t, sessionId)
    cmd_member(uint32_t, roleId)
end_cmd()


begin_cmd(CreateWorldScene)
    cmd_member(uint32_t, sceneMod)
    cmd_member(string, extendStr)
    cmd_member(vector<WorldPlayerInfo>, playerinfos)
end_cmd()

begin_cmd(CreateWorldSceneRet)
    cmd_member(uint32_t, sceneMod)
    cmd_member(uint32_t, sceneInstId)
    cmd_member(string, extendStr)
    cmd_member(vector<int>, roleIds)
    cmd_member(vector<uint32_t>, roleRets)
end_cmd()

begin_cmd(EnterWorldScene)
    cmd_member(uint32_t, sceneInstId)
    cmd_member(vector<WorldPlayerInfo>, playerinfos)
end_cmd()



begin_cmd(EnterWorldSceneRet)
    cmd_member(uint32_t, sceneInstId)
    cmd_member(uint32_t, sceneMod)
    cmd_member(vector<int>, roleIds)
    cmd_member(vector<uint32_t>, roleRets)
end_cmd()

begin_cmd(QuickEnterWorldScene)
    cmd_member(uint32_t, sceneMod)
    cmd_member(WorldPlayerInfo, playerinfo)
end_cmd()


begin_cmd(QuickEnterWorldSceneRet)
    cmd_member(uint32_t, sceneInstId)
    cmd_member(uint32_t, sceneMod)
    cmd_member(int, roleId)
end_cmd()


begin_cmd(LeaveWorldScene)
    cmd_member(int, sessionId)
    cmd_member(int, roleId)

end_cmd()

begin_cmd(LeaveWorldSceneRet)
    cmd_member(int, ret)
    cmd_member(int, sceneId)
    cmd_member(int, sceneMod)
    cmd_member(string, extendStr)
    cmd_member(int, sessionId)
    cmd_member(int, roleId)
    cmd_member(int, reason)
    cmd_member(bool, isDead)
end_cmd()

begin_cmd(WorldPvpResult)
    cmd_member(int, sceneMod)
    cmd_member(string, extendStr)
    cmd_member(int, winner)
    cmd_member(int, loser)
    cmd_member(int, loserState)
end_cmd()

begin_cmd(WorldTeamCopyResult)
    cmd_member(int, sceneMod)
    cmd_member(int, sceneId)
    cmd_member(int, result)
    cmd_member(int, teamId)
end_cmd()


begin_cmd(DestroyWorldScene)
    cmd_member(int, sceneId)
end_cmd()

begin_cmd(DestroyWorldSceneRet)
    cmd_member(int, ret)
    cmd_member(int, sceneId)
end_cmd()




#endif /* defined(__GameSrv__cmd_world_handler__) */
