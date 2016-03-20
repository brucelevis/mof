//
//  SyncTeamCopy.cpp
//  GameSrv
//
//  Created by pireszhi on 14-7-16.
//
//

#include "SyncTeamCopy.h"
#include "world_cmdmsg.h"
#include "EnumDef.h"
#include "GuildMgr.h"
#include "GameScript.h"
#include "flopcard.h"
#include "GameLog.h"
#include "CopyExtraAward.h"
#include "daily_schedule_system.h"
#include "Game.h"
#include "Honor.h"
#include "Role.h"

SyncFightingTeamMgr g_SyncFightingTeamMgr;
SyncTeamCopyMgr g_SyncTeamCopyMgr;
extern bool checkCurrSceneType(int sceneid,const eSceneType &type);

extern WorldPlayerInfo getWorldPlayerInfo(Role* role);
extern WorldRoleInfo getWorldRoleInfo(Role* role);
extern WorldPetInfo getWorldPetInfo(Pet* pet);

extern bool sendMailWithName(int roleid, const char* sender, const char* receiver, const char* mailName,const char* attach);

//extern int sendNormalCopyAward(Role* role, SceneCfgDef* scenecfg, int flopid, int friendId, int& fcindex, vector<int>& drops, RewardStruct& notifyReward, string );

//extern int sendNormalCopyAward(Role* role,
//							   SceneCfgDef* scenecfg,
//							   int flopid,
//							   int friendId,
//							   int& fcindex,
//							   vector<int>& drops,
//							   RewardStruct& notifyReward,
//							   string &awardExtra);

void getTeamMemberInfo(obj_fighting_team_member& info, const TeamMember& member)
{
    info.memberId = member.roleid;
    info.memberName = member.rolename;
    info.memberRoleType = member.roleType;
    info.isReady = member.beReady;
    info.isCaptain = member.isCaptain;
    info.weaponFasion = member.weaponFasion;
    info.battleForce = member.battleForce;
    info.bodyfashion = member.bodyfashion;
    info.weaponmodel = member.weaponmodel;
    info.cityBodyFashion = member.cityBodyFashion;
    info.enchantId = member.enchantId;
    info.enchantLvl = member.enchantLvl;
}

FightingTeam::FightingTeam(int captainid, string captainName, int scenemod, int memCount):mCaptainId(captainid),
                                                                            mCaptainName(captainName),
                                                                            mCurrMemberCount(0),
                                                                            mSceneMod(scenemod),
                                                                            mSceneInstId(0)
{
    check_max(memCount, 20);
    check_min(memCount, 1);
    TeamMember member;
    mMembers.resize(memCount, member);
}

bool FightingTeam::addMember(Role* newMember, bool iscaptain)
{
    int vsize = mMembers.size();
    int position = 0;
    
    for (int i = 0; i < vsize; i++) {
        
        TeamMember& member = mMembers[i];
        if (member.roleid == 0) {
            member.roleid = newMember->getInstID();
            member.rolename = newMember->getRolename();
            member.guildid = newMember->getGuildid();
            member.isCaptain = iscaptain;
            member.roleType = newMember->getRoleType();
            
            member.battleForce = newMember->getBattleForce();
            member.rolelvl = newMember->getLvl();
            member.sessionid = newMember->getSessionId();
            
            member.weaponFasion = newMember->getWeaponFashion();
            member.cityBodyFashion = newMember->getHomeFashion();
            member.weaponmodel = newMember->getWeaponQua();
            member.bodyfashion = newMember->getBodyFashion();
            member.enchantId = newMember->getEnchantId();
            member.enchantLvl = newMember->getEnchantLvl();
            mCurrMemberCount++;
            
            //告诉其他人，有个xxx加入队伍
            notifySomeOneJoin(member, i);
            
            return true;
        }
    }
    return false;
}

void FightingTeam::notifySomeOneJoin(TeamMember& member, int position)
{
    if (member.isCaptain) {
        return;
    }
    notify_fighting_team_someone_join notify;
    
    notify.member.position = position;
    
    getTeamMemberInfo(notify.member, member);
    
    sendNotifyToMember(&notify, member.roleid);
}

void FightingTeam::notifySomeOneLeave(TeamMember &member, int position)
{
    notify_fighting_team_someone_leave notify;
    notify.position = position;
    notify.leaveId = member.roleid;
    notify.leaveName = member.rolename;
    
    sendNotifyToMember(&notify);
}

void FightingTeam::sendNotifyToMember(INetPacket *packet, int filteredId)
{
    int vsize = mMembers.size();
    
    for (int i = 0; i < vsize; i++) {
        TeamMember& oldMember = mMembers[i];
        
        if ( oldMember.roleid == 0 || oldMember.roleid == filteredId) {
            continue;
        }
        
        sendNetPacket(oldMember.sessionid, packet);
    }
}

void FightingTeam::notifySetNewCaptain(TeamMember &newCaptain, int position)
{
    notify_fighting_team_new_captain notify;
    notify.newCaptain = newCaptain.roleid;
    notify.newCaptainName = newCaptain.rolename;
    notify.position = position;
    
    sendNotifyToMember(&notify);
}

void FightingTeam::notifyMemberState(TeamMember &member, int position)
{
    notify_fighting_team_change notify;
    getTeamMemberInfo(notify.member, member);
    notify.member.position = position;
    
    sendNotifyToMember(&notify);
}

bool FightingTeam::leaveTeam(int roleid)
{
    int vsize = mMembers.size();
    
    int index = -1;
    int leaveCaptainId = 0;
    
    for (int i = 0; i < vsize; i++) {
        TeamMember& member = mMembers[i];
        
        if (mMembers[i].roleid == roleid) {
            
            if (mMembers[i].isCaptain) {
                leaveCaptainId = mMembers[i].roleid;
            }
            
            //通知队员，有人离开了
            notifySomeOneLeave(member, i);
            
            //原来的位置变成空
            TeamMember nilmember;
            mMembers[i] = nilmember;
            
            mCurrMemberCount--;
            break;
        }
    }
    
    //重设队长    
    if (leaveCaptainId > 0) {
        
        for (int i = 0; i < vsize; i++) {
            if (mMembers[i].roleid) {
                mMembers[i].isCaptain = true;
                mMembers[i].beReady = true;
                mCaptainId = mMembers[i].roleid;
                mCaptainName = mMembers[i].rolename;
                notifySetNewCaptain(mMembers[i], i);
                break;
            }
        }
        
    }
    
    //如果是同步组队副本且队伍不在战斗副本中，刷新列表
    if (checkCurrSceneType(mSceneMod, stSyncTeamCopy) && mSceneInstId == 0) {
        
        //更新列表中的队伍信息
        notify_sync_teamcopy_listchange notify;
        notify.team.teamid = mTeamId;
        notify.team.captain = mCaptainId;
        notify.team.captainName = mCaptainName;
        notify.team.sceneMod = mSceneMod;
        notify.team.memberCount = mCurrMemberCount;
        g_SyncTeamCopyMgr.sendNotifyToPalyers(&notify);
    }
    
    return true;
}

bool FightingTeam::setReady(int roleid, bool ready)
{
    int vsize = mMembers.size();
    
    for (int i = 0; i < vsize; i++) {
        TeamMember& member = mMembers[i];
        if (member.roleid == roleid) {
        
            member.beReady = ready;
            
            //通知队友，该玩家状态变化
            notifyMemberState(member, i);
            
            return true;
        }
    }
    return false;
}

bool FightingTeam::isAllMemberReady()
{
    int vsize = mMembers.size();
    
    for (int i = 0; i < vsize; i++) {

        if (mMembers[i].roleid != 0 && mMembers[i].beReady == false) {
            return false;
        }
    }
    
    return true;
}

int FightingTeam::canBeginBattle()
{    
    int vsize = mMembers.size();
    
    for (int i = 0; i < vsize; i++) {
        
        if (mMembers[i].roleid != 0) {
            
            if(mMembers[i].beReady == false)
            {
                return CE_SYNC_TEAM_SOME_ONE_OF_THIS_TEAM_ARE_NOT_READY;
            }
            
            Role* teamMate = SRoleMgr.GetRole(mMembers[i].roleid);
            
            if (teamMate == NULL) {
                continue;
            }
            
            if (!checkCurrSceneType(teamMate->getCurrSceneId(), stTown)) {
                return CE_SYNC_TEAM_SOME_ONE_OF_THIS_TEAM_ARE_IN_COPY;
            }
        }
    }
    
    return CE_OK;
}

void FightingTeam::getFightingTeamMembers(vector<obj_fighting_team_member>& outdata)
{
    int membercount = mMembers.size();
    
    for (int i = 0; i < membercount; i++) {
        TeamMember& member = mMembers[i];
        obj_fighting_team_member info;
        
        getTeamMemberInfo(info, member);
        
        info.position = i;
        outdata.push_back(info);
    }
}

vector<TeamMember>& FightingTeam::getFightingTeamMembers()
{
    return mMembers;
}

int FightingTeam::canChangeSceneMod(int scenemod)
{
    SceneCfgDef* scene = SceneCfg::getCfg(scenemod);
    if (scene == NULL) {
        return CE_READ_SCENE_CFG_ERROR;
    }
    
    int membercount = mMembers.size();
    
    for (int i = 0; i < membercount; i++) {
        TeamMember& member = mMembers[i];
        if (member.roleid != 0 && member.rolelvl < scene->minLv) {
            return CE_SYNC_TEAM_TEAM_MATE_LVL_IS_TOO_LOW;
        }
    }
    
    return CE_OK;
}

void FightingTeam::changeSceneMod(int scenemod)
{
    mSceneMod = scenemod;
    
    notify_fighting_team_scenemod_change change;
    change.newscenemod = scenemod;
    
    sendNotifyToMember(&change);
}

void FightingTeam::enterFightingScene()
{
    int membercount = mMembers.size();
    
    create_cmd(CreateWorldScene, createWorldScene);
    createWorldScene->sceneMod = mSceneMod;
    
    for (int i = 0; i < membercount; i++) {
        TeamMember& member = mMembers[i];
        Role* role = SRoleMgr.GetRole(member.roleid);
        
        if (role) {
            WorldPlayerInfo playerInfo = getWorldPlayerInfo(role);
            createWorldScene->playerinfos.push_back(playerInfo);
        }
    }
    
    Json::Value value;
    Json::FastWriter writer;
    value["team_id"] = mTeamId;
    value["leader"] = mCaptainId;
    createWorldScene->extendStr = writer.write(value);
    
    sendMessageToWorld(0, CMD_MSG, createWorldScene, 0);
}

bool FightingTeam::isMemberFull()
{
    int vsize = mMembers.size();
    
    for (int i = 0; i < vsize; i++) {
        if (mMembers[i].roleid == 0) {
            return false;
        }
    }
    
    return true;
}

int FightingTeam::canJoin(Role *role)
{
    if (isMemberFull()) {
        return CE_SYNC_TEAM_MEMBER_OF_THIS_TEAM_FULL;
    }
    
    SceneCfgDef* scene = SceneCfg::getCfg(mSceneMod);
    
    if (scene == NULL) {
        return CE_READ_SCENE_CFG_ERROR;
    }
    
    if (scene->minLv > role->getLvl()) {
        return CE_ROLE_LVL_TOO_LOW;
    }
    
    return CE_OK;
}

void FightingTeam::sendTeamTalking(Role* sender, string content, VoiceInfoItem& voiceInfo)
{
    mVoiceInfo.push_back(voiceInfo);
    
    notify_fighting_team_teamtalking talking;
    talking.senderId = sender->getInstID();
    talking.senderName = sender->getRolename();
    talking.senderRoleType = sender->getRoleType();
    talking.message = content;
    
    talking.createDate = Game::tick;
    talking.voiceId = voiceInfo.mVoiceId;
    talking.voicdeDuration = voiceInfo.mVoiceDuration;
    talking.translated = voiceInfo.mTranslated;
    
    sendNotifyToMember(&talking);
}

VoiceInfoItem FightingTeam::getTalkingVoiceInfo(int64_t voiceId)
{
    list<VoiceInfoItem>::reverse_iterator iter = mVoiceInfo.rbegin();
    list<VoiceInfoItem>::reverse_iterator endIter = mVoiceInfo.rend();
    
    for (; iter != endIter; iter++) {
        if (voiceId == (*iter).mVoiceId) {
            return (*iter);
        }
    }
    
    return VoiceInfoItem();
}

void FightingTeam::removeVoiceInfo()
{
    list<VoiceInfoItem>::reverse_iterator iter = mVoiceInfo.rbegin();
    list<VoiceInfoItem>::reverse_iterator endIter = mVoiceInfo.rend();
    
    for (; iter != endIter; iter++) {
        if (0 != (*iter).mVoiceId) {
            g_VoiceChatMgr.setRemove((*iter).mChatType, (*iter).mVoiceId);
        }
    }
}

/******
 Mgr
 *****/

FightingTeam* SyncFightingTeamMgr::createTeam(Role* creater, int sceneMod, int memberNum)
{
    //条件判断
    FightingTeam* team = new FightingTeam(creater->getInstID(), creater->getRolename(), sceneMod, memberNum);
    
    if (team) {
        team->mTeamId = getGenTeamid();
        if(team->addMember(creater, true))
        {
            creater->setSyncTeamId(team->mTeamId);
            team->setReady(creater->getInstID(), true);
        }
    }
    
    int idx = team->mTeamId - 1;
    
    idx &= (mCapacity - 1);
    
    
    if (mTeams[idx] == NULL) {
        mTeams[idx] = team;
        mCurNum++;
    }
    else{
        delete team;
        return NULL;
    }
    
    return team;
}

FightingTeam* SyncFightingTeamMgr::findTeam(int teamid)
{
    if (teamid <= 0) {
        return NULL;
    }
    
    int idx = teamid - 1;
    
    idx &= (mCapacity - 1);
    
    return mTeams[idx];
}

int SyncFightingTeamMgr::applyToJoinTeam(Role* applycant, FightingTeam* team)
{    
    if (team == NULL) {
        return CE_SYNC_TEAM_CAN_NOT_FIND_THIS_TEAM;
    }
    
    if (applycant->getSyncTeamId()) {
        return CE_SYNC_TEAM_YOU_HAVE_IN_A_TEAM;
    }
    
    SceneCfgDef* scene = SceneCfg::getCfg(team->mSceneMod);
    if (scene) {
        if (applycant->getLvl() < scene->minLv) {
            return CE_YOUR_LVL_TOO_LOW;
        }
    }
    
    //各种条件判定
    if (team->addMember(applycant, false)) {
        applycant->setSyncTeamId(team->mTeamId);
        return CE_OK;
    }
    
    return CE_SYNC_TEAM_MEMBER_OF_THIS_TEAM_FULL;
}

void SyncFightingTeamMgr::leaveTeam(Role* role, int teamid)
{
    FightingTeam* team = findTeam(teamid);
    
    if (team) {
        team->leaveTeam(role->getInstID());
        role->setSyncTeamId(0);
        
        //队伍的人数变成0，做点什么
        if (!team->mCurrMemberCount) {
            dismissTeam(teamid);
        }
    }
    g_SyncTeamCopyMgr.notifyTeamInfoChange(teamid); 
    
    return;
}

bool SyncFightingTeamMgr::onTeamEnterSceneToFight(int teamid, int sceneInstId)
{
    FightingTeam* team = findTeam(teamid);
    if (team) {
        team->mSceneInstId = sceneInstId;
        
        //在组队副本里清掉
        g_SyncTeamCopyMgr.delTeamInList(team->mTeamId);
        
        return true;
    }
    return false;
}

void SyncFightingTeamMgr::cancelReadyToFight(Role *role, int teamid)
{
    FightingTeam* team = findTeam(teamid);
    if (team) {
        team->setReady(role->getInstID(), false);
    }
}

int SyncFightingTeamMgr::kickOutFromTeam(Role* roleBekicked, int teamid)
{
    FightingTeam* team = findTeam(teamid);
    if (team) {
        if (roleBekicked->getInstID() == team->mCaptainId) {
            return 1;
        }
        
        roleBekicked->setSyncTeamId(0);
        if(team->leaveTeam(roleBekicked->getInstID()))
        {
            notify_fighting_team_be_kick_out notify;
            notify.captainName = team->mCaptainName;
            notify.beKickId = roleBekicked->getInstID();
            notify.someoneBeKickName = roleBekicked->getRolename();
            sendNetPacket(roleBekicked->getSessionId(), &notify);
            return 0;
        }

    }
    
    return 1;
}

void SyncFightingTeamMgr::sendTeamTalking(Role *sender, string message, VoiceInfoItem& voiceInfo)
{
    if (sender->getSyncTeamId() == 0) {
        return;
    }
    
    FightingTeam* team = findTeam(sender->getSyncTeamId());
    if (team == NULL) {
        return;
    }
    
    team->sendTeamTalking(sender, message, voiceInfo);
}

void SyncFightingTeamMgr::dismissTeam(int teamid)
{
    int idx = teamid - 1;
    
    idx &= (mCapacity - 1);
    
    FightingTeam* team = mTeams[idx];
    if (team) {
        
        //清除语音信息
        team->removeVoiceInfo();
        delete team;
        mTeams[idx] = NULL;
        mCurNum--;
    }
}

/********
 副本相关
 *******/

void SyncTeamCopyMgr::getTeamList(vector<obj_sync_team_info> &teamList)
{
    list<int>::iterator iter = mTeams.begin();
    list<int>::iterator endIter = mTeams.end();
    
    for (; iter != endIter; iter++) {
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(*iter);
        if (team) {
            obj_sync_team_info info;
            info.teamid = team->mTeamId;
            info.captain = team->mCaptainId;
            info.captainName = team->mCaptainName;
            info.sceneMod = team->mSceneMod;
            info.memberCount = team->mCurrMemberCount;
            
            teamList.push_back(info);
        }
    }
}

bool SyncTeamCopyMgr::addNewTeam(int teamid)
{
    if (!isTeamInList(teamid)) {

        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(teamid);
        
        if (team == NULL) {
            return false;
        }
        
        mTeams.push_back(teamid);

        notify_sync_teamcopy_increase notify;
        notify.team.teamid = team->mTeamId;
        notify.team.captain = team->mCaptainId;
        notify.team.captainName = team->mCaptainName;
        notify.team.sceneMod = team->mSceneMod;
        notify.team.memberCount = team->mCurrMemberCount;
        
        sendNotifyToPalyers(&notify);
    }
    
    return true;
}

bool SyncTeamCopyMgr::isTeamInList(int teamid)
{
    list<int>::iterator iter = mTeams.begin();
    list<int>::iterator endIter = mTeams.end();
    
    for (; iter != endIter; iter++) {
        if (*iter == teamid) {
            return true;
        }
    }
    
    return false;
}

void SyncTeamCopyMgr::delTeamInList(int teamid)
{
    list<int>::iterator iter = mTeams.begin();
    list<int>::iterator endIter = mTeams.end();
    
    for (; iter != endIter; iter++) {
        if (*iter == teamid) {
            mTeams.erase(iter);
            notify_sync_teamcopy_remove remove;
            remove.teamid = teamid;
            g_SyncTeamCopyMgr.sendNotifyToPalyers(&remove);
            return;
        }
    }
}

void SyncTeamCopyMgr::enterActivity(int sessionid)
{
    if (!isOpenActivity(sessionid)) {
        mPlayersSessionId.push_back(sessionid);
        return;
    }
}

void SyncTeamCopyMgr::leaveActivity(int sessionid)
{
    list<int>::iterator iter = mPlayersSessionId.begin();
    list<int>::iterator endIter = mPlayersSessionId.end();
    
    for (; iter != endIter; iter++) {
        if (*iter == sessionid) {
            mPlayersSessionId.erase(iter);
            return;
        }
    }
    
}

bool SyncTeamCopyMgr::isOpenActivity(int sessionid)
{
    list<int>::iterator iter = mPlayersSessionId.begin();
    list<int>::iterator endIter = mPlayersSessionId.end();
    
    for (; iter != endIter; iter++) {
        if (*iter == sessionid) {
            return true;
        }
    }
    return false;
}

void SyncTeamCopyMgr::sendNotifyToPalyers(INetPacket* packet)
{
    list<int>::iterator iter = mPlayersSessionId.begin();
    list<int>::iterator endIter = mPlayersSessionId.end();
    
    for (; iter != endIter; iter++) {
        sendNetPacket(*iter, packet);
    }
}

void SyncTeamCopyMgr::notifyTeamInfoChange(int teamid)
{
    if (!isTeamInList(teamid)) {
        return;
    }
    
    FightingTeam* team = g_SyncFightingTeamMgr.findTeam(teamid);
    
    if (team == NULL) {
        
        delTeamInList(teamid);
        
//        notify_sync_teamcopy_remove remove;
//        remove.teamid = teamid;
//        sendNotifyToPalyers(&remove);
        return;
    }
    
    //更新列表中的队伍信息
    notify_sync_teamcopy_listchange notify;
    notify.team.teamid = team->mTeamId;
    notify.team.captain = team->mCaptainId;
    notify.team.captainName = team->mCaptainName;
    notify.team.sceneMod = team->mSceneMod;
    notify.team.memberCount = team->mCurrMemberCount;
    sendNotifyToPalyers(&notify);

}

int getSyncTeamMateIntimacy(Role* role, vector<TeamMember>& teamMates, int sceneid)
{
    if (role == NULL) {
        return 0;
    }
    
    int intimacySum = 0;
    int teamMateCount = teamMates.size();
    
    for (int i = 0; i < teamMateCount; i++) {
        int teamMateId = teamMates[i].roleid;
        
        //如果是好友加亲密度
        if (role->mFriendMgr->isFriend(teamMateId)) {
            role->getFriendMgr()->procAddIntimacy(role, teamMateId, sceneid);
        }
        
        if (teamMateId == 0) {
            continue;
        }
        
        intimacySum += role->getFriendMgr()->getFrinedIntimacy(teamMateId);
    }
    
    return intimacySum;
}


/**
 *	@brief	同步组队副本结算时统计公会奖励的倍数
 *
 *	@param 	guildid 	公会id
 *	@param 	increment 	增量
 *	@param 	awardMultiple 	存放队内公会倍数的map
 */
void syncTeamCopyAddgGuildAwardMultiple(int guildid, string rolename, int incomeTimes, map<int, vector<SyncTeamCopyGuildMemberIncomeState> >& guildAndMember)
{
    map<int, vector<SyncTeamCopyGuildMemberIncomeState> >::iterator iter = guildAndMember.find(guildid);
    if (iter == guildAndMember.end()) {
        vector<SyncTeamCopyGuildMemberIncomeState> members;
        members.clear();
        
        SyncTeamCopyGuildMemberIncomeState state;
        state.rolename =rolename;
        state.incomeTimes = incomeTimes;
        members.push_back(state);
        guildAndMember.insert(make_pair(guildid, members));
        return;
    }
    SyncTeamCopyGuildMemberIncomeState state;
    state.rolename =rolename;
    state.incomeTimes = incomeTimes;
    iter->second.push_back(state);
}

/**
 *	@brief	 发送同步组队副本公会部分的奖励
 *
 *	@param 	guildAwardReport 	公会id及公会奖励的倍数
 */
void sendSyncTeamCopyGuildAward(int sceneMod, map<int, vector<SyncTeamCopyGuildMemberIncomeState> >& guildAwardReport)
{
    map<int, vector<SyncTeamCopyGuildMemberIncomeState> >::iterator iter = guildAwardReport.begin();
    map<int, vector<SyncTeamCopyGuildMemberIncomeState> >::iterator endIter = guildAwardReport.end();
    
    SyncTeamCopyCfgDef* cfg = SyncTeamCopyCfg::getCfg(sceneMod);
    if (cfg == NULL) {
        return;
    }
    
    for (; iter != endIter; iter++) {
        
        if (iter->second.size() < 2) {
            continue;
        }
        
        Guild& guild = SGuildMgr.getGuild(iter->first);
        
        if (guild.getId() == 0 ) {
            continue;
        }
        
        for (int i = 0; i < iter->second.size(); i++) {
            
            if (iter->second[i].incomeTimes) {
                
                string syncTeamCopy = GameTextCfg::getString("1051");
                
                int fortune = cfg->fortuneAward;
                
                int constrution = cfg->constructionAward;
                
                guild.addFortuneConstruction(fortune, constrution, iter->second[i].rolename.c_str(), syncTeamCopy.c_str());
            }
        }
        
    }
}

int getSyncTeamCopyPlayerIntimacyAwardMultiple(int intimacysum)
{
    int multiple = 1;
    
    try
    {
        CMsgTyped msg;
        msg.SetInt(intimacysum);
        
        if (GameScript::instance()->call("syncteamcopy_intimacy_award_multiple", msg) >= 0)
        {
            int type = msg.TestType();
            if (type == TYPE_STRING)
            {
                multiple = msg.IntVal();
            }
        }
    }
    catch(...)
    {
    }
    
    return multiple;
}

extern void sendNotifyBagFullAndSendMail(Role* role);

/**
 *	@brief	发送同步组队副本的专用函数
 *
 *	@param 	role 	收奖励的对象
 *	@param 	scenecfg 	对应场景的配置指针
 *	@param 	intimacySum 	好友亲密度总和
 *	@param 	notifyReward 	返回给客户端的协议
 *
 *	@return	错误码
 */
int sendPlayerSyncTeamCopyAward(Role* role, SceneCfgDef* scenecfg, int intimacySum, notify_sync_teamcopy_award& notifyReward)

{
    if (scenecfg == NULL || scenecfg->sceneType != stSyncTeamCopy) {
        return CE_READ_SCENE_CFG_ERROR;
    }
    
    ItemArray items;
    RewardStruct reward;
    BackBag* bag= role->getBackBag();
    
    int fcindex = 0;
    vector<int> drops;
    drops.clear();
    
    string dropStr = "";
    string awardBase = SflopcardMgr.RandomCard(scenecfg->sceneId, fcindex, drops, dropStr, 0);
    string awardResult = awardBase;
	
	// 双倍药水
	SCopyExtraAward.doublePotion(role, scenecfg, awardBase, awardResult);
	
	// 副本活动奖励
	SCopyExtraAward.extraItemsAward(role->getInstID(), scenecfg->sceneId, awardBase, awardResult);

    std::vector<std::string> rewards = StrSpilt(awardResult, ";");
    rewardsCmds2ItemArray(rewards, items, reward);
    
    //getitem log
    char buf[32] = "";
    sprintf(buf, "syncTeamCopy");
    
    GridArray effgrids;
    //检查是否可以发放奖励
//    bool canAddItem = bag->PreAddItems(items, effgrids);
    int canAddItem = role->preAddItems(items, effgrids);
    
    int ret = CE_OK;

    if (canAddItem == CE_OK) {
        
        //BackBag::UpdateBackBag(role->getInstID(), effgrids);
//        role->updateBackBag(effgrids, items, true, buf);
        role->playerAddItemsAndStore(effgrids, items, buf, true);
    }
    else
    {
        sendMailWithName(0, "", role->getRolename().c_str(), "bag_full", awardResult.c_str());
//        ret = CE_BAG_FULL_AND_SEND_MAIL;
        sendNotifyBagFullAndSendMail(role);
    }
    
    if (scenecfg != NULL)
    {
        float multiple = SyncTeamCopyFunctionCfg::getIncomeMultiple(intimacySum);
        
        reward.reward_gold += scenecfg->copy_gold * multiple;
        reward.reward_batpoint += scenecfg->copy_battlepoint * multiple;
		
		// 好友副本，如果双倍药水交次数还存在，则所得奖励翻倍
		SCopyExtraAward.doublePotionBase(role, scenecfg, reward);
		
		// 副本活动奖励
		SCopyExtraAward.extraBaseAward(role->getInstID(), scenecfg, reward);
        
        role->addExp(reward.reward_exp, buf);
        role->addGold(reward.reward_gold, buf);
        role->addBattlePoint(reward.reward_batpoint, buf);
        role->addConstellVal(reward.reward_consval, buf);
        role->addPetExp(role->getActivePetId(), reward.reward_petexp, buf);
        role->addEnchantDust(reward.reward_enchantdust, buf);
        role->addEnchantGold(reward.reward_enchantgold, buf);
    }
    
    notifyReward.normalaward.findex = fcindex;
    notifyReward.normalaward.dropindices = drops;
    notifyReward.normalaward.flopid = scenecfg->sceneId;
    
    notifyReward.normalaward.gold = reward.reward_gold;
    notifyReward.normalaward.exp = reward.reward_exp;
    notifyReward.normalaward.battlepoint = reward.reward_batpoint;
    notifyReward.normalaward.petexp = reward.reward_petexp;
    notifyReward.normalaward.constell = reward.reward_consval;
	notifyReward.normalaward.awardItems = awardResult;
    
    LogMod::addLogFinishCopy(role, scenecfg->sceneId, true, awardResult, scenecfg->sceneType);
    return ret;
}

void SyncTeamCopyMgr::fightingResultHandle(int teamid, int result)
{
    FightingTeam* team = g_SyncFightingTeamMgr.findTeam(teamid);
    
    if (team == NULL) {
        return;
    }
    
    SceneCfgDef* scenecfg = SceneCfg::getCfg(team->mSceneMod);
    
    if (scenecfg == NULL) {
        return;
    }
    
    if (scenecfg->sceneType != stSyncTeamCopy) {
        return;
    }
    
    team->mSceneInstId = 0;
    
    //添加会列表
    addNewTeam(teamid);
    
    if (result == 1) {
        
        //统计公会奖励倍数的map
        map<int, vector<SyncTeamCopyGuildMemberIncomeState> > guildAndMembers;
        guildAndMembers.clear();
        
        //发个人奖励
        vector<TeamMember>& members = team->getFightingTeamMembers();
        int memCount = members.size();
        for (int i = 0; i < memCount; i++) {
            
            TeamMember& member = members[i];
            
            //普通队员需要重新准备
            if (member.isCaptain == false) {
                member.beReady = false;
            }
            
            Role* role = SRoleMgr.GetRole(member.roleid);
            
            if (role == NULL) {
                continue;
            }
            
            int leftIncomeTimes = role->getSyncTeamCopyLeftTimesOfAward();
            
            //扣次数
            if (role->getIncrSyncTeamCopyTimesForIncome() > 0)
            {
                role->setIncrSyncTeamCopyTimesForIncome(role->getIncrSyncTeamCopyTimesForIncome() - 1);
                role->saveNewProperty();
            }
            else
            {
                role->setSyncTeamCopyAttendTimes(role->getSyncTeamCopyAttendTimes() + 1);
            }
            
            {
                string addAction = Utils::makeStr("entercopy_%d", scenecfg->sceneId);
                role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesSyncTeamCopy, addAction.c_str());
            }
            
            notify_sync_teamcopy_award notify;
            notify.isPass = true;
            notify.errorcode = CE_OK;
            notify.hasAward = false;
            
            //所在公会奖励倍数+1
            if (member.guildid) {
                syncTeamCopyAddgGuildAwardMultiple(member.guildid, member.rolename, leftIncomeTimes, guildAndMembers);
            }
            
            //如果有收益次数
            if (leftIncomeTimes) {
            
                notify.hasAward = true;
                
                //功能相关的额外奖励
                int intimacySum = getSyncTeamMateIntimacy(role, members, team->mSceneMod);
                
                //基本奖励
                notify.errorcode = sendPlayerSyncTeamCopyAward(role, scenecfg, intimacySum, notify);
                
                notify.hasAward = true;
                
                notify.isPass = true;
                    
                // 称号检查
                SHonorMou.procHonor(eHonorKey_Copy, role, xystr("%d",scenecfg->sceneId));
                if (role->getDailyScheduleSystem()) {
                    role->getDailyScheduleSystem()->onEvent(kDailyScheduleSyncTeamCopy, 0, 1);
                }
                
            }
            
            sendNetPacket(role->getSessionId(), &notify);
        }
        
        //下面是发公会奖励
        sendSyncTeamCopyGuildAward(team->mSceneMod, guildAndMembers);
    }
    else{
        //或者发奖励
        vector<TeamMember>& members = team->getFightingTeamMembers();
        int memCount = members.size();
        for (int i = 0; i < memCount; i++) {
            
            TeamMember& member = members[i];
            
            //普通队员需要重新准备
            if (member.isCaptain == false) {
                member.beReady = false;
            }
            
            Role* role = SRoleMgr.GetRole(member.roleid);
            
            if (role == NULL) {
                continue;
            }
            
            notify_sync_teamcopy_award notify;
            notify.isPass = false;
            notify.hasAward = false;
            notify.errorcode = 1;       //没通关
            sendNetPacket(role->getSessionId(), &notify);
        }
    }
}
