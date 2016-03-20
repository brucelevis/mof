//
//  GuildChat.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-10-21.
//
//

#include "GuildChat.h"
#include "Guild.h"
#include "Role.h"
#include "RedisKeyName.h"
#include "EnumDef.h"

#define MSG_TYPE "messageType"
#define MSG_DATE "date"
#define MSG_CONTENT "content"
#define MSG_POSITION "pos"

#define MSG_ROLE_ID "roleid"
#define MSG_ROLE_NAME "rolename"
#define MSG_ROLETYPE "roletype"
#define MSG_ROLELVL "rolelvl"

#define MSG_VOICEID "voiceid"
#define MSG_VOICE_DURATION "voiceduration"
#define MSG_VOICE_TRANSLATED "voicetranslated"

void GuildChat::load(int guildid)
{
    mChats.load( g_GetRedisName(rnGuildChat), guildid);
    
    //要把语音信息分离出来,好像很没效率
    for (Chats::iterator iter = mChats.begin(); iter != mChats.end(); iter++) {
        const Json::Value& val = iter->second;
        
        int messageType = val[MSG_TYPE].asInt();
        
        if (messageType == kChatContentTypeVoice) {
            
            VoiceInfoItem voiceInfo;
            
            voiceInfo.mChatType = eGameChatGuildChat;
            voiceInfo.mVoiceId = atoll(val[MSG_VOICEID].asString().c_str());
            voiceInfo.mVoiceDuration = val[MSG_VOICE_DURATION].asInt();
            voiceInfo.mTranslated = val[MSG_VOICE_TRANSLATED].asBool();
            
            mVoiceInfo.insert(make_pair(voiceInfo.mVoiceId, voiceInfo));
        }
    }
}
void GuildChat::update()
{
    mChats.save();
}
void GuildChat::destroy()
{
    mChats.clear();
}

void GuildChat::append(GuildChatMsg& msg)
{
    int id = mChats.nextIntKey();
    
    while( mChats.size() >= GUILD_CHAT_MSG)
    {
        const Json::Value& val = mChats.begin()->second;
        int64_t voiceId = atoll(val[MSG_VOICEID].asString().c_str());
        removeVoiceInfo(voiceId);
        
        mChats.erase( mChats.begin()->first );
    }
    
    mChats[id][MSG_TYPE] = msg.messageType;
    mChats[id][MSG_DATE]= msg.createDate;
    mChats[id][MSG_CONTENT] = msg.msg;
    mChats[id][MSG_POSITION]= msg.position;
    
    mChats[id][MSG_ROLE_ID] = msg.roleid;
    mChats[id][MSG_ROLE_NAME] = msg.rolename;
    mChats[id][MSG_ROLETYPE]= msg.roleType;
    mChats[id][MSG_ROLELVL] = msg.roleLvl;
    
    string voiceId = Utils::makeStr("%lld", msg.voiceId);
    mChats[id][MSG_VOICEID]= voiceId;
    mChats[id][MSG_VOICE_DURATION]= msg.voiceDuration;
    mChats[id][MSG_VOICE_TRANSLATED]=msg.translated;
    
    VoiceInfoItem voiceInfo;
    voiceInfo.mVoiceId = msg.voiceId;
    voiceInfo.mVoiceDuration = msg.voiceDuration;
    voiceInfo.mChatType = eGameChatGuildChat;
    voiceInfo.mTranslated = msg.translated;
    addVoiceInfo(voiceInfo);
}

void GuildChat::send(int roleid,int start ,int num)
{
    std::pair<int,int> range = checkPageRange(mChats.size(), start, num);
    
    ack_past_guild_chat ack;
    ack.start = range.first;
    obj_guild_chat obj;
    
    Chats::reverse_iterator iter = mChats.rbegin() ;
    for (int i = 0; i<range.first; ++i)
    {
        ++iter;
    }
    
    for (int i = 0; i < range.second - range.first; ++i)
    {
        obj.id = iter->first;
        const Json::Value& val = iter->second;
        
        obj.messageType = val[MSG_TYPE].asInt();
        obj.date = val[MSG_DATE].asInt();
        obj.content = val[MSG_CONTENT].asString();
        obj.position = val[MSG_POSITION].asInt();
        
        obj.senderRoleid = val[MSG_ROLE_ID].asInt();
        obj.senderRoleName = val[MSG_ROLE_NAME].asString();
        obj.senderRoleType = val[MSG_ROLETYPE].asInt();
        obj.senderLvl = val[MSG_ROLELVL].asInt();

        string str = val[MSG_VOICEID].asString();
        
        obj.voiceid = atoll(val[MSG_VOICEID].asString().c_str());
        obj.voiceDuration = val[MSG_VOICE_DURATION].asInt();
        obj.translated = val[MSG_VOICE_TRANSLATED].asBool();
        
        ack.chats.push_back(obj);
        ++iter;
    }
    sendRolePacket(roleid,&ack);
}

VoiceInfoItem GuildChat::findVoiceInfo(int64_t voiceId)
{
    map<int64_t, VoiceInfoItem>::iterator iter = mVoiceInfo.find(voiceId);
    
    if (iter == mVoiceInfo.end()) {
        return VoiceInfoItem();
    }
    
    return iter->second;
}

void GuildChat::removeVoiceInfo(int64_t voiceId)
{
    map<int64_t, VoiceInfoItem>::iterator iter = mVoiceInfo.find(voiceId);
    
    if (iter != mVoiceInfo.end()) {
        mVoiceInfo.erase(iter);
    }
}

void GuildChat::addVoiceInfo(VoiceInfoItem &voiceInfo)
{
    mVoiceInfo.insert(make_pair(voiceInfo.mVoiceId, voiceInfo));
}


