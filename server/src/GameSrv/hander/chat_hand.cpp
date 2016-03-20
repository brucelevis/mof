//
//  chat_hand.cpp
//  GameSrv
//
//  Created by cxy on 13-1-10.
//
//

#include "hander_include.h"
#include "Scene.h"

#include "Utils.h"
#include "Robot.h"
#include "ChatRoom.h"
#include "GuildMgr.h"
#include "SyncTeamCopy.h"
#include "PrivateChatHistoryMgr.h"
#include "Honor.h"

extern bool gmCommand(Role* role, const char* str);
extern void sendSceneDisplayedPets(Role* role);
extern void sendSceneDisplayedRetinues(Role* role);

static void __strfilter(string &str)
{
    replace(str.begin(), str.end(), '\t', ' ');
	replace(str.begin(), str.end(), '\n', ' ');
    return ;
}

hander_msg(req_privatechat,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    Role* rRole = SRoleMgr.GetRole(req.receiverId);

    if (role->getBanChat())
    {
        return;
    }
    
    //__strfilter(req.message);

    ack_private_chat ack;
    ack.errorcode = 1;
    ack.receiverId = req.receiverId;
    ack.message = req.message;
    ack.messageType = req.messageType;
    ack.uuid = req.uuid;
    ack.voiceId = 0;
    ack.voiceDuration = req.voiceDuration;
    ack.senderRoleType = role->getRoleType();
    ack.translated = req.translated;
    
//    int objectType = getObjectType(req.receiverId);
    
    bool isChatFriend = role->getFriendMgr()->isFriend(req.receiverId);
    
    VoiceInfoItem voiceInfo;
    
    if (req.messageType == kChatContentTypeVoice) {
        
        VoiceDataStruct* voicedata = NULL;
        
        bool saveDB = false;
        if (isChatFriend) {
            saveDB =  true;
        }
        
        voicedata = g_VoiceChatMgr.createVoiceData(eGameChatPlayerPrivateChat,req.voiceData, Game::tick, saveDB);
            
        voiceInfo.mChatType = eGameChatPlayerPrivateChat;

        
        if( voicedata != NULL )
        {
            ack.voiceId = voicedata->mVoiceId;
            
            voiceInfo.mVoiceId = voicedata->mVoiceId;
            voiceInfo.mVoiceDuration = req.voiceDuration;
            voiceInfo.mTranslated = req.translated;
        }
    }
    
    if (rRole) {

        notify_privatechat noty;
        noty.senderId = roleid;
        noty.senderSex = role->getSex();
        noty.senderRoleType = role->getRoleType();
        noty.senderName = role->getRolename();
        noty.senderLvl = role->getLvl();
        
        noty.createDate = Game::tick;
        noty.message = req.message;
        noty.messageType = req.messageType;
        
        noty.voiceId = ack.voiceId;             //语音的编号
        noty.voiceDuration = ack.voiceDuration;
        noty.translated = ack.translated;
        
        sendNetPacket(rRole->getSessionId(), &noty);

        string recvAccount = SSessionDataMgr.getProp(rRole->getSessionId(), "account");

        LogMod::addLogPrivateChat(roleid, role->getRolename().c_str(), role->getSessionId(), rRole->getInstID(), rRole->getRolename().c_str(), recvAccount.c_str(), req.message.c_str());

        ack.errorcode = 0;
        
        if (RobotCfg::sFriendId != req.receiverId && !isChatFriend) {
            rRole->getChatHistoryMgr()->updateVoiceInfo(voiceInfo);
        }
    }
    
    //是和好友聊天，则刷新聊天记录和好友的聊天记录。
    if (RobotCfg::sFriendId != req.receiverId && isChatFriend)
    {
        ack.errorcode = 0;
        role->getChatHistoryMgr()->update(req, voiceInfo);
    }
    ack.createDate = Game::tick;
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_get_voice_chat_data, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_get_voice_chat_data ack;
    ack.channel = req.channel;
    ack.uuid = req.uuid;
    ack.errorcode = CE_OK;
    ack.roleId = req.roleId;
    
    VoiceDataStruct* voice = NULL;
    
    //私聊要验证一下
    if (req.channel == ePrivatechat)
    {

        VoiceInfoItem voiceInfo = role->getChatHistoryMgr()->getVoiceInfo(eGameChatPlayerPrivateChat, req.voiceId);
        
        voice = g_VoiceChatMgr.findVoiceData(voiceInfo.mChatType, voiceInfo.mVoiceId);
        
        if (voice == NULL) {
            ack.errorcode = CE_VOICECHAT_NOT_FIND_VOICEDATA;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        if (voiceInfo.mChatType == eGameChatPlayerPrivateChat) {
            
            //陌生人聊天，取了之后删除
            if(false == voice->mSave)
            {
                voice->mRemove = true;
            }
        }
    }
    else if (req.channel == eGuild)  //公会聊天
    {
        Guild& guild = SGuild(role->getGuildid());
        if (guild.isNull()) {
            ack.errorcode = CE_GUILD_NOT_EXIST;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        VoiceInfoItem voiceInfo = guild.getVoiceInfo(req.voiceId);
        if (voiceInfo.mVoiceId <= 0) {
            ack.errorcode = CE_VOICECHAT_NOT_FIND_VOICEDATA;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        voice = g_VoiceChatMgr.findVoiceData(eGameChatGuildChat, req.voiceId);
    }
    else if (req.channel == eWorld) //世界聊天
    {
        voice = g_VoiceChatMgr.findVoiceData(eGameChatWorldChat, req.voiceId);
    }
    else if (req.channel == eSyncTeam)  //同步组队副本聊天
    {
        FightingTeam* team = g_SyncFightingTeamMgr.findTeam(role->getSyncTeamId());
        
        if (NULL != team) {
            VoiceInfoItem voiceInfo = team->getTalkingVoiceInfo(req.voiceId);
            voice = g_VoiceChatMgr.findVoiceData(voiceInfo.mChatType, voiceInfo.mVoiceId);
        }
    }
    
    if (voice == NULL) {
        ack.errorcode = CE_VOICECHAT_NOT_FIND_VOICEDATA;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    ack.voiceData = voice->mVoiceData;
    ack.voiceId = voice->mVoiceId;
    ack.createDate = voice->mCreateDate;
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_worldchat,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (role->getBanChat())
    {
        return;
    }
    
	if (!role->checkWorldChatTime()) {
        ack_worldchat ack;
        ack.errorcode = CE_YOU_SEND_WORLD_MSG_TOO_OFTEN;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    notify_worldchat noty;
    noty.senderId = roleid;
    noty.senderSex = role->getSex();
    noty.senderName = role->getRolename();
    noty.senderRoleType = role->getRoleType();
    noty.senderLvl = role->getLvl();
    noty.message = req.message;
	noty.honorId = role->getHonorMgr()->getTalkHonor();
    noty.createDate = Game::tick;
    noty.translated = req.translated;
    noty.messageType = req.messageType;
    
    if (req.messageType == kChatContentTypeVoice) {
        
        VoiceDataStruct* voicedata = NULL;
        
        voicedata = g_VoiceChatMgr.createVoiceData(eGameChatWorldChat,req.voiceData, Game::tick, false);
        
        if (voicedata != NULL) {
            noty.voiceId = voicedata->mVoiceId;
            noty.voiceDuration = req.voiceDuration;
        }
        
    }
    
    LogMod::addLogWorldChat(roleid, role->getRolename().c_str(), role->getSessionId(), req.message.c_str());

    broadcastPacket(WORLD_GROUP, &noty);
}}

hander_msg(req_system_chat,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
	ack_system_chat ack;
	ack.errorcode = enSystemChatResult_No;
	
	do {
		if (req.message.substr(0, 2) != "  ") {
			ack.errorcode = enSystemChatResult_Matching;
			break;
		}
		
		if (!gmCommand(role, req.message.substr(2).c_str())){
            break;
        }
		
		ack.errorcode = enSystemChatResult_Success;
	} while (false);
	
	role->send(&ack);
}}

//好友聊天历史记录
hander_msg(req_private_chat_history, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    role->getChatHistoryMgr()->sendChatHistory(req);
}}


bool GetAllRolesInfoCb(void* obj, void* param)
{
    Role* role = (Role*)obj;
    ack_getsceneroles *infos = (ack_getsceneroles*)param;
    if (param == NULL)
    {
        return false;
    }

    if (infos->roleinfos.size() >= infos->maxnum)
    {
        return false;
    }

    obj_roleinfo info;    
    role->getRoleInfo(info);
    infos->roleinfos.push_back(info);

    return true;
}


hander_msg(req_getsceneroles, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    ack_getsceneroles ack;

    ack.errorcode = 0;
    ack.maxnum = req.maxnum;

    if (role->getScene())
    {
        role->getScene()->traverseRoles(GetAllRolesInfoCb, (void*)&ack);
    }
    sendNetPacket(sessionid, &ack);
    
    sendSceneDisplayedPets(role);
    sendSceneDisplayedRetinues(role);
}}

hander_msg(req_scene_displayed_pets, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    sendSceneDisplayedPets(role);
}}

hander_msg(req_scene_displayed_retinues, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    sendSceneDisplayedRetinues(role);
}}


hander_msg(req_getfriendlist, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    ack_getfriendlist ack;
    ack.errorcode = 0;
    do
    {
        FriendMgr::FriendMap& friendlist = role->getFriendMgr()->getFriends();

        for (FriendMgr::FriendMapIter iter = friendlist.begin(); iter != friendlist.end(); iter++)
        {
            obj_friendRoleinfo info;
            info.roleid = iter->first;
            info.intimacy = iter->second.mIntimacy;

//            Role* friendrole = SRoleMgr.GetRole(iter->first);
//            if (friendrole)
//            {
//                info.rolename = friendrole->getRolename();
//                info.lvl = friendrole->getLvl();
//                info.roletype = friendrole->getRoleType();
//                info.isonline = 1;
//                ack.roleinfos.push_back(info);
//                continue;
//            }

//            RedisResult result(redisCmd("hmget role:%d rolename lvl roletype", iter->first));
//
//            string rolename = result.readHash(0, "");
//            int rolelvl = result.readHash(1, 0);
//            int roletype = result.readHash(2, 0);
//
//            if (rolename.empty() || rolelvl == 0) {
//                continue;
//            }

            info.rolename = iter->second.rolename;
            info.lvl = iter->second.rolelvl;
            info.roletype = iter->second.roletype;
            info.isonline = iter->second.isonline;
            info.battleForce = iter->second.BattleForce;
            //info.isrobot = getObjectType(iter->first) == kObjectRobot;
            info.isrobot = iter->first == RobotCfg::sFriendId;
            
            Role* friendRole = SRoleMgr.GetRole(info.roleid);
            if (friendRole) {
                info.sceneid = friendRole->getCurrSceneId();
            } else {
                info.sceneid = 0;
            }
            ack.roleinfos.push_back(info);
        }
    }
    while (0);

    sendNetPacket(sessionid, &ack);
}}


hander_msg(req_addfriend, req)
{
    hand_Sid2Role_check(sessionid, roleid, self)

    ack_addfriend ack;
    do
    {
        if (self->getFriendMgr()->isFriend(req.roleid))
        {
            ack.errorcode = 2;
            break;
        }

        Role* tarrole = SRoleMgr.GetRole(req.roleid);
        if (tarrole == NULL)
        {
            ack.errorcode = 1;
            break;
        }

        VipPrivilegeCfgDef* vipData = VipPrivilegeCfg::getCfg(self->getVipLvl());
        if (vipData == NULL) {
            ack.errorcode = CE_READ_VIPCFG_ERROR;
            break;
        }

        if (self->getFriendMgr()->getFriendCount() > vipData->friendCount) {
            ack.errorcode = CE_YOUR_FRIENDCOUNT_IS_FULL;
            break;
        }

        vipData = VipPrivilegeCfg::getCfg(tarrole->getVipLvl());
        if (vipData == NULL) {
            ack.errorcode = CE_READ_VIPCFG_ERROR;
            break;
        }
        if (tarrole->getFriendMgr()->getFriendCount() > vipData->friendCount) {
            ack.errorcode = CE_THE_TARGETS_FRIENDCOUNT_IS_FULL;
            break;
        }
        notify_addfriend notify;
        notify.role.roleid = roleid;
        notify.role.rolename = self->getRolename();
        notify.role.roletype = self->getRoleType();
        notify.role.lvl = self->getLvl();
        notify.role.isonline = 1;
        notify.role.battleForce = self->getBattleForce();
        sendNetPacket(tarrole->getSessionId(), &notify);

        ack.errorcode = 0;
        ack.roleid = req.roleid;
    }
    while (0);

    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_agreefriend, req)
{
    hand_Sid2Role_check(sessionid, roleid, self)

    ack_agreefriend ack;
    ack.errorcode = 0;
    ack.agree = req.agree;
    do
    {
        Role* tarrole = SRoleMgr.GetRole(req.roleid);
        if (tarrole == NULL)
        {
            ack.errorcode = 1;
            break;
        }

        Friend fself(req.roleid);
        Friend ftar(roleid);

        if (req.agree == 1)
        {
            VipPrivilegeCfgDef* vipData = VipPrivilegeCfg::getCfg(self->getVipLvl());
            
            if (vipData == NULL) {
                ack.errorcode = CE_READ_VIPCFG_ERROR;
                break;
            }
            
            if (self->getFriendMgr()->getFriendCount() > vipData->friendCount) {
                ack.errorcode = CE_YOUR_FRIENDCOUNT_IS_FULL;
                break;
            }
            
            VipPrivilegeCfgDef* targetVipData = VipPrivilegeCfg::getCfg(tarrole->getVipLvl());
            if (targetVipData == NULL) {
                ack.errorcode = CE_READ_VIPCFG_ERROR;
                break;
            }
            
            if (tarrole->getFriendMgr()->getFriendCount() > targetVipData->friendCount) {
                ack.errorcode = CE_THE_TARGETS_FRIENDCOUNT_IS_FULL;
                break;
            }
            
            if (!self->getFriendMgr()->isFriend(req.roleid))
            {
                self->getFriendMgr()->addNewFriend(req.roleid);
                FriendMgr::saveFriend(roleid, fself);
            }

            if (!tarrole->getFriendMgr()->isFriend(roleid))
            {
                tarrole->getFriendMgr()->addNewFriend(roleid);
                FriendMgr::saveFriend(req.roleid, ftar);
            }
        }
		
        ack.role.roleid = req.roleid;
        ack.role.rolename = tarrole->getRolename();
        ack.role.lvl = tarrole->getLvl();
        ack.role.roletype = tarrole->getRoleType();
        ack.role.isonline = 1;
        ack.role.intimacy = fself.mIntimacy;
        ack.role.battleForce = tarrole->getBattleForce();
		
        notify_agreefriend notify;
        notify.agree = req.agree;
        notify.role.roleid = roleid;
        notify.role.rolename = self->getRolename();
        notify.role.lvl = self->getLvl();
        notify.role.roletype = self->getRoleType();
        notify.role.isonline = 1;
        notify.role.intimacy = ftar.mIntimacy;
        notify.role.battleForce = self->getBattleForce();
        sendNetPacket(tarrole->getSessionId(), &notify);
    }
    while (0);

    sendNetPacket(sessionid, &ack);
}}


handler_msg(req_del_friend, req)
{
    hand_Sid2Role_check(sessionid, roleid, self)

    ack_del_friend ack;
    ack.errorcode = 1;
    ack.friendid = req.friendid;
    
    //int objType = getObjectType(req.friendid);
    if (req.friendid != RobotCfg::sFriendId && self->getFriendMgr()->delFriend(req.friendid))
    {
        Role* target = SRoleMgr.GetRole(req.friendid);
        if (target != NULL)
        {
            notify_del_friend notify;
            notify.friendid = roleid;

            if (target->getFriendMgr()->delFriend(roleid))
            {
                sendNetPacket(target->getSessionId(), &notify);
            }
        }
        FriendMgr::removeFriend(req.friendid, roleid);
        FriendMgr::removeFriend(roleid, req.friendid);
		
		ack.errorcode = 0;
    }

    sendNetPacket(sessionid, &ack);
}}


handler_msg(req_queryrole, req)
{
    ack_queryrole ack;
    ack.rolename = req.rolename;
    
    Role* qrole = SRoleMgr.GetRole(req.rolename.c_str());
    if (qrole != NULL){
        
        obj_roleinfo info;
        qrole->getRoleInfo(info);
        
        ack.roleinfos.push_back(info);
    }

    sendNetPacket(sessionid, &ack);
}}
