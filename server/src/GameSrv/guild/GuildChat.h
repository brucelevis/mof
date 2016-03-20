//
//  GuildChat.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-10-21.
//
//

#pragma once
#include "RedisHashJson.h"
#include "ChatRoom.h"
#include <string>
using namespace std;

struct GuildChatMsg
{
    GuildChatMsg(): roleid(0),
                    roleType(0),
                    roleLvl(0),
                    messageType(0),
                    voiceId(0),
                    voiceDuration(0),
                    translated(0)
    {
        
    }
    int roleid;
    string rolename;
    int roleType;
    int roleLvl;
    
    int messageType;
    int createDate;
    int position;
    string msg;
    
    int64_t voiceId;
    int voiceDuration;
    bool translated;
};

class GuildChat
{
    typedef IntMapJson Chats;
public:
    void load(int guildid);
    void update();
    void destroy();
    
    void append(GuildChatMsg& msg);
    void send(int roleid,int start ,int num);
    
    VoiceInfoItem findVoiceInfo(int64_t voiceId);
private:
    void removeVoiceInfo(int64_t voiceId);
    void addVoiceInfo(VoiceInfoItem& voiceInfo);
protected:
    Chats   mChats;
    map<int64_t, VoiceInfoItem> mVoiceInfo;
};
