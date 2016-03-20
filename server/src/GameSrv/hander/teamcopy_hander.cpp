//
//  teamcopy_hander.cpp
//  GameSrv
//
//  Created by pireszhi on 13-5-6.
//
//

#include "hander_include.h"
#include "Scene.h"
#include "Activity.h"
#include "Utils.h"
#include "flopcard.h"
#include "GuildMgr.h"
#include "Robot.h"
#include "SyncTeamCopy.h"
#include "DBRolePropName.h"
#include "Retinue.h"

handler_msg(req_getTeamCopyData, req)
{
    hand_Sid2Role_check(sessionid, roleid, self)
    ack_getTeamCopyData ack;
    ack.finishedCopys.clear();
    ack.frightedfriends.clear();

    const set<int>& copylist = self->mTeamCopyMgr->getFinishCopy();
    for (set<int>::iterator it = copylist.begin(); it != copylist.end(); it++) {
        ack.finishedCopys.push_back(*it);
    }

    const set<int>& friends = self->mTeamCopyMgr->getFightedFriendList();
    for (set<int>::iterator it = friends.begin(); it != friends.end(); it++) {
        ack.frightedfriends.push_back(*it);
    }

    sendNetPacket(sessionid, &ack);
}}

// 好友副本
handler_msg(req_beginTeamCopy, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    ack_beginTeamCopy ack;
    do
    {
        //防止客户端重复发多条消息过来
        if (0 < self->getPreEnterSceneid()) {
            return;
        }
        
        if (!checkCurrSceneType(req.copyid, stTeamCopy)) {
            kickAwayRoleWithTip(self);
            return;
        }
        
        if (!checkCanEnterCopy(self, req.copyid)) {
            return;
        }
        
        if (!self->mFriendMgr->isFriend(req.friendid)) {
        
            Guild& guild = SGuildMgr.getRoleGuild(roleid);
            if (guild.isNull() || !guild.getMembers().exist(req.friendid) )
            {
                ack.errorcode = CE_GET_FRIEND_DATA_ERROR;
                break;
            }
        }
        
        if(self->mTeamCopyMgr->IsCopyFinish(req.copyid))
        {
            ack.errorcode = CE_TEAMCOPY_YOU_HAD_FINISH_THISCOPY;
            break;
        }

        if (self->mTeamCopyMgr->IsFriendFighted(req.friendid)) {
            ack.errorcode = CE_TEAMCOPY_YOU_HAD_EMPLOYED_THIS_FRIEND;
            break;
        }
        
        //int objectType = getObjectType(req.friendid);
        //if (objectType == kObjectRobot) {
        if (req.friendid == RobotCfg::sFriendId) {
            Robot* robot = g_RobotMgr.getData(req.friendid);
            if (robot == NULL) {
                ack.errorcode = CE_GET_FRIEND_DATA_ERROR;
                break;
            }
            
            obj_roleBattleProp info;
            
            info.atk = robot->mBattleProp.getAtk();
            info.def = robot->mBattleProp.getDef();
            info.cri = robot->mBattleProp.getCri();
            info.dodge = robot->mBattleProp.getDodge();
            info.hit = robot->mBattleProp.getHit();
            info.maxhp = robot->mBattleProp.getMaxHp();
            
            info.roleid = req.friendid;
            info.skills = robot->mSkills;
            info.weaponfashion = 0;
            info.awakeLvl = 0;
            
            ack.friendinfo = info;
            ack.errorcode = CE_OK;
            
            self->setFightedFriendInfo(robot);
        } else {

            RedisResult result(redisCmd("hget role:%d %s", req.friendid, GetRolePropName(eRolePropRoleName)));
            string employeeName = result.readStr();
            if( !employeeName.empty() )
            {
                Role* employee = Role::create(req.friendid,true);

                if (employee == NULL) {
                    ack.errorcode = CE_GET_FRIEND_DATA_ERROR;
                    break;
                }
              
                employee->onCalPlayerProp();
                employee->getPlayerBattleProp(ack.friendinfo);
                
                Pet* pet = employee->mPetMgr->getActivePet();
                int petBattle = 0;
                int activePetid = 0;
                obj_petBattleProp info;
                if (pet != NULL && pet->petid > 0)
                {
                    activePetid = pet->petid;
                    pet->getBattleProp(info);
                    petBattle = pet->mBattle;
                }
                else
                {
                    info.petid = 0;
                }
                ack.petinfo = info;
                
                Retinue *retinue = employee -> mRetinueMgr -> getActiveRetinue();
                obj_retinueBattleProp retinueBPInfo;
                if(retinue != NULL && retinue -> getId() > 0){
                    retinue -> getBattleProp(retinueBPInfo);
                    retinueBPInfo.skills = employee -> getRetinueMgr() -> mSkillMgr -> getEquipedSkills();
                }
                ack.retinueinfo = retinueBPInfo;
                
                self->setFightedFriendInfo(employee, petBattle, activePetid);
                delete employee;
                
                
                //self->setPreEnterSceneid(req.copyid);

            } else{
                ack.errorcode = CE_GET_FRIEND_DATA_ERROR;
                break;
            }
        }
        
        ack.copyid = req.copyid;
        self->setPreEnterSceneid(req.copyid);
        self->mTeamCopyMgr->addFightedFriend(roleid, self->getFightedFriendId());
        
        UpdateQuestProgress("enterfriendcopy", req.copyid, 1, self, true);

    }while (false);

    sendNetPacket(sessionid, &ack);
}}

//好友副本 通知客户端剩余双倍药水奖励次数
handler_msg(req_doublePotionNum, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_doublePotionNum ack;
    ack.nums = role->getDoublePotionNum();
    
   sendNetPacket(sessionid, &ack);
}}


/****************************** syncTeam 同步组队相关 *********************************/

handler_msg(req_fighting_team_create, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_fighting_team_create ack;
    
    do
    {
        //已经加入了队伍不能建
        if (role->getSyncTeamId()) {
            ack.errorcode = CE_SYNC_TEAM_YOU_HAVE_IN_A_TEAM;
            break;
        }
        
        SceneCfgDef* scenecfg = SceneCfg::getCfg(req.sceneMod);
        
        if (scenecfg == NULL) {
            ack.errorcode = CE_READ_SCENE_CFG_ERROR;
            break;
        }
        
        if (scenecfg->minLv > role->getLvl()) {
            ack.errorcode = CE_YOUR_LVL_TOO_LOW;
            break;
        }
        
        if (scenecfg->sceneType == stSyncTeamCopy) {
            
            FightingTeam* team = g_SyncFightingTeamMgr.createTeam(role, req.sceneMod, SyncTeamCopyFunctionCfg::getTeamMemberCount());
            
            if (team == NULL) {
                ack.errorcode = CE_SYNC_TEAM_CREATE_TEAM_ERROR;
                break;
            }
            
            //离开界面
            g_SyncTeamCopyMgr.leaveActivity(sessionid);
            
            //添加到组队副本的队伍列表
            g_SyncTeamCopyMgr.addNewTeam(team->mTeamId);
            
            ack.errorcode = CE_OK;
            ack.teamid = team->mTeamId;
            ack.mySyncCopyleftTimeOfAward = role->getSyncTeamCopyLeftTimesOfAward();
        }
        
    }while (false);

    
    sendNetPacket(sessionid, &ack);

}}

handler_msg(req_fighting_team_aplly_join, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_fighting_team_aplly_join ack;
    
    do {
        
        if (role->getSyncTeamId()) {
            ack.errorcode = CE_SYNC_TEAM_YOU_HAVE_IN_A_TEAM;
            break;
        }
        
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(req.teamid);
        
        if (team == NULL) {
            ack.errorcode = CE_SYNC_TEAM_CAN_NOT_FIND_THIS_TEAM;
            break;
        }
        
        SceneCfgDef* scene = SceneCfg::getCfg(team->mSceneMod);
        if (scene == NULL) {
            ack.errorcode = CE_READ_SCENE_CFG_ERROR;
            break;
        }
        
        if (scene->minLv > role->getLvl()) {
            ack.errorcode = CE_YOUR_LVL_TOO_LOW;
            break;
        }
        
        int ret = g_SyncFightingTeamMgr.applyToJoinTeam(role, team);
        
        ack.errorcode = ret;
        
        if (!ret) {
            ack.teamid = team->mTeamId;
            ack.sceneMod = team->mSceneMod;
            team->getFightingTeamMembers(ack.members);
            
            if (checkCurrSceneType(team->mSceneMod, stSyncTeamCopy)) {
                
                //离开界面
                g_SyncTeamCopyMgr.leaveActivity(sessionid);
                g_SyncTeamCopyMgr.notifyTeamInfoChange(team->mTeamId);
            }
        }
        
    } while (false);
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_fighting_team_leave, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (role->getSyncTeamId() == 0) {
        return;
    }
    
    g_SyncFightingTeamMgr.leaveTeam(role, role->getSyncTeamId());
    ack_fighting_team_leave ack;
    
    ack.errorcode = CE_OK;

    sendNetPacket(sessionid, &ack);
    
}}

handler_msg(req_fighting_team_kick_someone_out, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_fighting_team_kick_someone_out ack;
    
    do {
        if (role->getSyncTeamId() == 0) {
            ack.errorcode = CE_SYNC_TEAM_YOU_ARE_NOT_IN_A_TEAM;
            break;
        }
        
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(role->getSyncTeamId());
        if (team == NULL) {
            ack.errorcode = CE_SYNC_TEAM_CAN_NOT_FIND_THIS_TEAM;
            break;
        }
        
        if (team->mCaptainId != roleid) {
            ack.errorcode = CE_SYNC_TEAM_YOU_ARE_NOT_CAPTAIN_OF_THIS_TEAM;
            break;
        }
        
        Role* target = SRoleMgr.GetRole( req.someoneBeKick);
        if (target == NULL) {
            ack.errorcode = CE_TARGET_IS_DISONLINE;
            break;
        }
        
        if(g_SyncFightingTeamMgr.kickOutFromTeam(target, role->getSyncTeamId()))
        {
            ack.errorcode = CE_SYNC_TEAM_KICKOUT_ERROR;
            break;
        }
        ack.errorcode = CE_OK;
        
    } while (false);
    sendNetPacket(sessionid, &ack);
    
}}

handler_msg(req_fighting_team_set_ready, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_fighting_team_set_ready ack;
    ack.errorcode = CE_OK;
    do {
        
        if (!role->getSyncTeamId()) {
            ack.errorcode = CE_SYNC_TEAM_YOU_HAVE_IN_A_TEAM;
            break;
        }
        
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(role->getSyncTeamId());
        
        if (team == NULL) {
            ack.errorcode = CE_SYNC_TEAM_CAN_NOT_FIND_THIS_TEAM;
            break;
        }
        
        //如果是队长
        if (team->mCaptainId == roleid) {
            
            //如果是队长就判断是否所有人准备好，并开始
            
            int canbegin = team->canBeginBattle();
            
            if (canbegin == CE_OK) {
                //开始战斗
                team->enterFightingScene();
            }
            
            ack.errorcode = canbegin;

        }
        else{   //如果不是队长
            if (!team->setReady(roleid, true)) {
                ack.errorcode = CE_SYNC_TEAM_SET_READY_ERROR;
                break;
            }
        }
    } while (false);

    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_fighting_team_cancel_ready, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (role->getSyncTeamId() == 0) {
        return;
    }
    
    ack_fighting_team_cancel_ready ack;
    ack.errorcode = CE_OK;
    do {
        
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(role->getSyncTeamId());
        
        if (team == NULL) {
            ack.errorcode = CE_SYNC_TEAM_CAN_NOT_FIND_THIS_TEAM;
            break;
        }
        
        if (team->mCaptainId == roleid) {
            ack.errorcode = CE_SYNC_TEAM_YOU_ARE_THE_CAPTAIN;
            break;
        }
        
        if(!team->setReady(roleid, false))
        {
            ack.errorcode = CE_SYNC_TEAM_CANCEL_READY_ERROR;
            break;
        }
    } while (false);
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_fighting_team_get_teamInfo, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_fighting_team_get_teamInfo ack;
    
    do {
        
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(req.teamid);
        
        if (team == NULL) {
            ack.errorcode = CE_SYNC_TEAM_CAN_NOT_FIND_THIS_TEAM;
            break;
        }
        
        ack.teamid = team->mTeamId;
        ack.sceneMod = team->mSceneMod;
        ack.mySyncCopyleftTimeOfAward = role->getSyncTeamCopyLeftTimesOfAward();
        team->getFightingTeamMembers(ack.members);
        ack.errorcode = CE_OK;
    } while (false);

    sendNetPacket(sessionid, &ack);
}}


hander_msg(req_fighting_team_batch_invite_player, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_fighting_team_batch_invite_player ack;
    do {
        
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(role->getSyncTeamId());
        
        if (team == NULL) {
            ack.errorcode = CE_SYNC_TEAM_YOU_ARE_NOT_IN_A_TEAM;
            break;
        }
        
        for (int i = 0; i < req.targetsId.size(); i++) {
            
            Role* target = SRoleMgr.GetRole(req.targetsId[i]);
            if (target == NULL) {
                continue;
            }
            
            if (target->getSyncTeamId()) {
                continue;
            }
            
            ack.errorcode = team->canJoin(target);
            if (ack.errorcode == CE_OK) {
                notify_fighting_team_invitation invitation;
                invitation.teamid = team->mTeamId;
                invitation.inviterId = roleid;
                invitation.inviterName = role->getRolename();
                invitation.sceneMod = team->mSceneMod;
                
                sendNetPacket(target->getSessionId(), &invitation);
            }
            
            //批量的时候， 都显示发送成功；
            ack.errorcode = CE_OK;
        }
        
    } while (false);
    
    sendNetPacket(sessionid, &ack);
    
}}

handler_msg(req_fighting_team_single_invite_player, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_fighting_team_single_invite_player ack;
    
    do {
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(role->getSyncTeamId());
        
        if (team == NULL) {
            ack.errorcode = CE_SYNC_TEAM_YOU_ARE_NOT_IN_A_TEAM;
            break;
        }
                
        Role* target = SRoleMgr.GetRole(req.targetsId);
        if (target == NULL) {
            ack.errorcode = CE_TARGET_IS_DISONLINE;
            break;
        }
        
        if (target->getSyncTeamId()) {
            ack.errorcode = CE_SYNC_TEAM_TARGET_INVITED_IS_IN_A_TEAM;
            break;
        }
        
        ack.errorcode = team->canJoin(target);
        if (ack.errorcode == CE_OK) {
            notify_fighting_team_invitation invitation;
            invitation.teamid = team->mTeamId;
            invitation.inviterId = roleid;
            invitation.inviterName = role->getRolename();
            invitation.sceneMod = team->mSceneMod;
            
            sendNetPacket(target->getSessionId(), &invitation);
        }
        
        break;

    } while (false);
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_fighting_team_invitation_respond, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_fighting_team_invitation_respond ack;
    ack.accept = req.accept;
    
    if (req.accept == 0) {
        
        Role* inviter = SRoleMgr.GetRole(req.inviterId);
        
        if (inviter) {
            
            notify_fighting_team_invitation_respond respond;
            respond.accept = req.accept;
            respond.targetId = roleid;
            respond.targetName = role->getRolename();
            
            sendNetPacket(inviter->getSessionId(), &respond);
        }
        return;
    }
    
    do {
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(req.teamid);
        
        if (team == NULL) {
            ack.errorcode = CE_SYNC_TEAM_INVITATION_TARGET_TEAM_MAYBE_DISMISS;
            break;
        }
        
        if (team->mSceneInstId) {
            ack.errorcode = CE_SYNC_TEAM_INVITATION_TARGET_TEAM_MAYBE_IN_FIGHTING;
            break;
        }
    
        ack.errorcode = g_SyncFightingTeamMgr.applyToJoinTeam(role , team);
        
        if (ack.errorcode == CE_OK) {
            ack.teamid = team->mTeamId;
            ack.sceneMod = team->mSceneMod;
            team->getFightingTeamMembers(ack.members);
        }
        
    } while (false);
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_fighting_team_teamtalking, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if(role->getSyncTeamId() == 0)
    {
        return;
    }
    
    VoiceInfoItem voiceInfo;
    
    if (req.messageType == kChatContentTypeVoice) {
        VoiceDataStruct* voice = g_VoiceChatMgr.createVoiceData(eGameChatSyncTeamChat, req.voiceData, Game::tick, false);
        
        if (NULL != voice) {
            voiceInfo.mVoiceId = voice->mVoiceId;
            voiceInfo.mVoiceDuration = req.voicdeDuration;
            voiceInfo.mTranslated = req.translated;
            voiceInfo.mChatType = eGameChatSyncTeamChat;
        }
    }
    
    g_SyncFightingTeamMgr.sendTeamTalking(role, req.message, voiceInfo);
    
}}

/********
 SyncTeamCopy
 *******/

handler_msg(req_sync_teamcopy_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_sync_teamcopy_list ack;
    
    ack.leftTimeOfAward = role->getSyncTeamCopyLeftTimesOfAward();
    
    g_SyncTeamCopyMgr.getTeamList(ack.teams);
    
    g_SyncTeamCopyMgr.enterActivity(sessionid);
    
    sendNetPacket(sessionid, &ack);
    
}}

handler_msg(req_sync_teamcopy_close_interface, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    g_SyncFightingTeamMgr.leaveTeam(role, role->getSyncTeamId());
    
    g_SyncTeamCopyMgr.leaveActivity(sessionid);
    
}}

handler_msg(req_sync_teamcopy_creatable_sceneModList, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_sync_teamcopy_creatable_sceneModList ack;
    SyncTeamCopyCfg::getAllCopyList(ack.sceneMods);
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_sync_teamcopy_change_scenemod, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_sync_teamcopy_change_scenemod ack;
    
    do {
        //不是队长不可操作
        if (role->getSyncTeamId() == 0) {
            ack.errorcode = CE_SYNC_TEAM_YOU_ARE_NOT_IN_A_TEAM;
            break;
        }
        
        //检查副本id是否正确
        SyncTeamCopyCfgDef* SyncScene = SyncTeamCopyCfg::getCfg(req.scene);
        if (SyncScene == NULL) {
            ack.errorcode = CE_SYNC_TEAM_TARGET_SCENEMOD_IS_INVALID;
            break;
        }
        
        //检查所有队员的等级
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(role->getSyncTeamId());
        
        if (team == NULL) {
            ack.errorcode = CE_SYNC_TEAM_CAN_NOT_FIND_THIS_TEAM;
            break;
        }
        
        if (team->mCaptainId != roleid) {
            ack.errorcode = CE_SYNC_TEAM_YOU_ARE_NOT_CAPTAIN_OF_THIS_TEAM;
            break;
        }
        
        int ret = team->canChangeSceneMod(req.scene);
        if (ret) {
            ack.errorcode = ret;
            break;
        }
        
        team->changeSceneMod(req.scene);
        
        //更新列表中的队伍信息
        notify_sync_teamcopy_listchange notify;
        notify.team.teamid = team->mTeamId;
        notify.team.captain = team->mCaptainId;
        notify.team.captainName = team->mCaptainName;
        notify.team.sceneMod = team->mSceneMod;
        notify.team.memberCount = team->mCurrMemberCount;
        g_SyncTeamCopyMgr.sendNotifyToPalyers(&notify);
        
        ack.errorcode = CE_OK;
        ack.scene = team->mSceneMod;
        
    } while (false);
    
    sendNetPacket(sessionid, &ack);

}}
