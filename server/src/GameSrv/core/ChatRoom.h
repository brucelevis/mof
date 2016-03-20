//
//  ChatRoom.h
//  GameSrv
//
//  Created by pireszhi on 14-7-16.
//
//

#ifndef __GameSrv__ChatRoom__
#define __GameSrv__ChatRoom__

#include <iostream>
#include <vector>
#include <list>
#include <stdint.h>
#include <map>

using namespace std;
#define VOICE_DATA_PRIVATECHAT_MAX_COUNT (100000)
#define VOICE_DATA_WORLDCHAT_MAX_COUNT (100)
#define VOICE_DATA_GUILDCHAT_MAX_COUNT (100000)
#define VOICE_DATA_SYNCTEAM_MAX_COUNT (10000)

#define VOICE_DATA_PRIVATECHAT_EXIST_SEC (24 * 3600)
#define VOICE_DATA_WORLDCHAT_EXIST_SEC (0)
#define VOICE_DATA_GUILDCHAT_EXIST_SEC (24 * 3600)
#define VOICE_DATA_SYNCTEAM_EXIST_SEC (10 * 60)

#define VOICECHAT_UPDATE_INTERVAL (5)

enum GameChatType
{
    eGameChatWorldChat = 1,         //世界聊天
    eGameChatPlayerPrivateChat,     //私聊
    eGameChatGuildChat,             //公会聊天
    eGameChatSyncTeamChat,          //同步组队队内聊天
    eGameChatTypeMax,               //聊天种类总数
};

static int getVoiceExistSec(GameChatType type);
static int getVoiceDataMaxCount(GameChatType type);
static string getVoiceDataKeyName(GameChatType type);
static string getVoiceIndexKeyName(GameChatType type);

class GenIdMgr
{
public:
    void genIdMgrInit(int maxNum)
    {
        mCurNum = 0;
        mCurIdx = 0;
        mMaxNum = maxNum;
        mCapacity = 1;
        while (mCapacity < maxNum)
        {
            mCapacity <<= 1;
        }
    }
    
    
private:
    int mCurIdx;
    int mCapacity;
    int mCurNum;
    int mMaxNum;
};

struct VoiceInfoItem
{
    VoiceInfoItem():mVoiceId(0),
                    mVoiceDuration(0),
                    mChatType(0)
    {
        
    }
    int64_t mVoiceId;
    int mVoiceDuration;
    int mChatType;      //区分好友私聊还是非好友的私聊
    int mTranslated;
};

struct VoiceDataStruct
{
    VoiceDataStruct():mRemove(false),
                      mSave(false),
                      mCreateDate(0),
                      mVoiceId(0)
    {
        mVoiceData.clear();
    }
    
    void setData(const char* str, int len)
    {
        mVoiceData.insert(mVoiceData.begin(), str, str + len);
    }
    
    VoiceDataStruct(const vector<char>& data):mRemove(false)
    {
        mVoiceData = data;
    }
    
    vector<char> mVoiceData;
    
//    int mSenderId;              //用于非好友的私聊
//    int mReceiverId;            //用于非好友的私聊
    bool mRemove;               //用于非好友的私聊
    bool mSave;
    
    time_t mCreateDate;
    int64_t mVoiceId;
};

typedef list<VoiceDataStruct*> VoiceDataList;
typedef map<int64_t, VoiceDataStruct*> VoiceDataMap;

class VoiceChat
{
public:
    VoiceChat(GameChatType type):mType(type),
                                 mGenId(0)
    {
        
    }
    
    void load();
    
    void update();
    
    VoiceDataStruct* createVoiceData(const vector<char>& voiceData, time_t createDate, bool save);
    
    VoiceDataStruct* findData(int64_t voiceId);
    
    void saveVoiceDataInDB(VoiceDataStruct* obj);
    
    void removeVoiceData(int64_t voiceId);
    
    GameChatType getType()
    {
        return mType;
    }
private:
    int64_t getGenId();
    
    void cleanVoiceData(int64_t voiceId);
    
private:
    
    VoiceDataMap mVoiceData;
    VoiceDataList mVoiceIndex;
    
    int64_t mGenId;
    GameChatType mType;
    
    time_t mUpdateSec;
};

class VoiceChatMgr
{
public:
    bool init();
    
    bool loadVoiceData();
    
    void update();
    
    bool addData(int type, VoiceDataStruct* obj);
    
    VoiceDataStruct* findVoiceData(int type, int64_t voiceId);
    
    VoiceDataStruct* createVoiceData(int type, const vector<char>& voiceData, time_t createDate, bool save);
    
    void setRemove(int type, int64_t voiceId);
    
private:
    VoiceChat* findVoiceType(int type);
    
    void removeVoiceData(int type, int64_t voiceId);
private:
    
    vector<VoiceChat*> mVoiceData;
};

extern VoiceChatMgr g_VoiceChatMgr;

#endif /* defined(__GameSrv__ChatRoom__) */
