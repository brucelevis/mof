//
//  ChatRoom.cpp
//  GameSrv
//
//  Created by pireszhi on 14-7-16.
//
//

#include "ChatRoom.h"
#include "Game.h"
#include "redis_helper.h"
#include "main.h"

VoiceChatMgr g_VoiceChatMgr;

int getVoiceExistSec(GameChatType type)
{
    switch (type) {
        case eGameChatPlayerPrivateChat:
            return VOICE_DATA_PRIVATECHAT_EXIST_SEC;
            break;
            
        case eGameChatGuildChat:
            return VOICE_DATA_GUILDCHAT_EXIST_SEC;
            break;
            
        case eGameChatWorldChat:
            return VOICE_DATA_WORLDCHAT_EXIST_SEC;
        
        case eGameChatSyncTeamChat:
            return VOICE_DATA_SYNCTEAM_EXIST_SEC;
            break;
            
        default:
            break;
    }
    
    return 0;
}

int getVoiceDataMaxCount(GameChatType type)
{
    switch (type) {
        case eGameChatPlayerPrivateChat:
            return VOICE_DATA_PRIVATECHAT_MAX_COUNT;
            break;
            
        case eGameChatGuildChat:
            return VOICE_DATA_GUILDCHAT_MAX_COUNT;
            break;
            
        case eGameChatWorldChat:
            return VOICE_DATA_WORLDCHAT_MAX_COUNT;
            
        case eGameChatSyncTeamChat:
            return VOICE_DATA_SYNCTEAM_MAX_COUNT;
            break;
            
        default:
            break;
    }
    
    return 0;
}

string getVoiceDataKeyName(GameChatType type)
{
    switch (type) {
        case eGameChatPlayerPrivateChat:
            return "friendchat_voice";
            break;
        case eGameChatGuildChat:
            return "guildchat_voice";
            break;
        default:
            break;
    }
    return "";
}

string getVoiceIndexKeyName(GameChatType type)
{
    switch (type) {
        case eGameChatPlayerPrivateChat:
            return "friendchat_voice_index";
            break;
        case eGameChatGuildChat:
            return "guildchat_voice_index";
            break;
        default:
            break;
    }
    return "";
}

void VoiceChat::load()
{
    mVoiceData.clear();
    
    string indexKeyName = getVoiceIndexKeyName(mType);
    
    string dataKeyName = getVoiceDataKeyName(mType);
    
    if (dataKeyName.empty() || indexKeyName.empty()) {
        return;
    }
    
    RedisResult result(redisCmd("hgetall %s", indexKeyName.c_str()));
    
    int elementnum = result.getHashElements();
    for (int i = 0; i < elementnum; i += 2)
    {
        int64_t voiceId = atoll(result.readHash(i, "").c_str());
        int createDate = result.readHash(i + 1, 0);
        
        if (voiceId <= 0) {
            continue;
        }
        
        VoiceDataStruct* voice = new VoiceDataStruct();
        voice->mVoiceId = voiceId;
        voice->mCreateDate = createDate;
        voice->mSave = true;
        
        mVoiceIndex.push_back(voice);
        mVoiceData.insert(make_pair(voiceId, voice));
    }
    
    result.setData(redisCmd("hgetall %s", dataKeyName.c_str()));
    elementnum = result.getHashElements();
    
    vector<int64_t> delData;
    
    for (int i = 0; i < elementnum; i += 2) {
        
        int64_t voiceId = atoll(result.readHash(i, "").c_str());
        
        VoiceDataStruct* voice = findData(voiceId);
        
        if (voice == NULL) {
            delData.push_back(voiceId);
            continue;
        }
        
        int len = 0;
        char* data = result.readHashBlob(i + 1, len);
        if (data != NULL && len >= 0) {
            voice->setData(data, len);
            mVoiceData.insert(make_pair(voice->mVoiceId, voice));
        }
    }
    
    result.setData(redisCmd("get %s_genid", dataKeyName.c_str()));
    mGenId = atoll(result.readStr().c_str());
    
    RedisArgs delArgs;
    RedisHelper::beginHdel(delArgs, dataKeyName.c_str());
    
    for (int i = 0; i < delData.size(); i++) {
        RedisHelper::appendHdel(delArgs, delData[i]);
    }
    
    if (delData.size()) {
        RedisHelper::commitDelKeys(get_DbContext(), delArgs);
    }
}

void VoiceChat::update()
{
    int interval = Game::tick - mUpdateSec;
    
    //如果超时为零就不遍历了
    if (getVoiceExistSec(mType) <= 0) {
        return;
    }
    
    if (interval < VOICECHAT_UPDATE_INTERVAL) {
        return;
    }
    
    mUpdateSec = Game::tick - (Game::tick % VOICECHAT_UPDATE_INTERVAL);
    
    VoiceDataList::iterator iter = mVoiceIndex.begin();
    VoiceDataList::iterator endIter = mVoiceIndex.end();
    
    vector<int64_t> delKeys;
    
    bool commit = false;
    
    for (; iter != endIter;) {
        VoiceDataStruct* voice = (*iter);
        
        if(voice != NULL)
        {
            if( (voice->mCreateDate + getVoiceExistSec(mType) < Game::tick) || voice->mRemove)
            {
                
                commit = true;
                delKeys.push_back(voice->mVoiceId);

                cleanVoiceData(voice->mVoiceId);
                delete voice;
              
                iter = mVoiceIndex.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
    
    string dataKeyName = getVoiceDataKeyName(mType);
    string indexKeyName = getVoiceIndexKeyName(mType);
    
    if (dataKeyName.empty()) {
        return;
    }
    
    RedisArgs delDataArg;
    RedisArgs delIndexArg;
    
    RedisHelper::beginHdel(delDataArg, dataKeyName.c_str());
    RedisHelper::beginHdel(delIndexArg, indexKeyName.c_str());
    
    for (int i = 0; i < delKeys.size(); i++) {
        RedisHelper::appendHdel(delDataArg, delKeys[i]);
        RedisHelper::appendHdel(delIndexArg, delKeys[i]);
    }
    
    if (commit) {
        
        doRedisCmd("multi");
        RedisHelper::commitHdel(get_DbContext(), delDataArg);
        RedisHelper::commitHdel(get_DbContext(), delIndexArg);
        doRedisCmd("exec");
        
    }
}

VoiceDataStruct* VoiceChat::createVoiceData(const vector<char>& voiceData, time_t createDate, bool save)
{
    int maxCount = getVoiceDataMaxCount(mType);
    
    if (maxCount > 0 && maxCount <= mVoiceData.size()) {
        VoiceDataStruct* oldVoice =  *(mVoiceIndex.begin());
        
        if (NULL != oldVoice) {
            cleanVoiceData(oldVoice->mVoiceId);
            delete oldVoice;
        }
        
        mVoiceIndex.pop_front();
    }
    
    VoiceDataStruct* voice = new VoiceDataStruct(voiceData);
    
    if (voice == NULL) {
        return NULL;
    }
    
    voice->mSave = save;
    voice->mCreateDate = createDate;
    voice->mVoiceId = getGenId();
    mVoiceIndex.push_back(voice);
    mVoiceData.insert(make_pair(voice->mVoiceId, voice));
    
    saveVoiceDataInDB(voice);
    
    return voice;
}

VoiceDataStruct* VoiceChat::findData(int64_t voiceId)
{
    VoiceDataMap::iterator iter = mVoiceData.find(voiceId);
    
    for (; iter != mVoiceData.end(); iter++) {
        VoiceDataStruct* voice = iter->second;
        
        if ( voice != NULL && voiceId == voice->mVoiceId) {
            return voice;
        }
    }
    
    return NULL;
}

void VoiceChat::saveVoiceDataInDB(VoiceDataStruct* obj)
{
    if (!obj->mSave) {
        return;
    }
    
    string indexKeyName = getVoiceIndexKeyName(mType);
    string dataKenName = getVoiceDataKeyName(mType);
    
    if (indexKeyName.empty() || dataKenName.empty()) {
        return;
    }
    
    int dataLen = obj->mVoiceData.size() * sizeof(char);
    
    doRedisCmd("multi");
    doRedisCmd("hset %s %lld %d", indexKeyName.c_str(), obj->mVoiceId, obj->mCreateDate);
    doRedisCmd("hset %s %lld %b", dataKenName.c_str(), obj->mVoiceId, obj->mVoiceData.data(), dataLen);
    doRedisCmd("exec");
    
}

void VoiceChat::cleanVoiceData(int64_t voiceId)
{
    VoiceDataMap::iterator iter = mVoiceData.find(voiceId);
    
    if (iter == mVoiceData.end()) {
        return;
    }
    mVoiceData.erase(iter);
}

void VoiceChat::removeVoiceData(int64_t voiceId)
{
    VoiceDataList::iterator iter = mVoiceIndex.begin();
    VoiceDataList::iterator endIter = mVoiceIndex.end();
    
    bool save = false;
    for (; iter != endIter; iter++) {
        VoiceDataStruct* voice = (*iter);
        
        if ( voice != NULL && voiceId == voice->mVoiceId) {
            
            save = voice->mSave;
            
            mVoiceIndex.erase(iter);
            cleanVoiceData(voiceId);
            
            delete voice;
            break;
        }
    }
    
    if (!save) {
        return;
    }
    
    string indexKeyName = getVoiceIndexKeyName(mType);
    string dataKenName = getVoiceDataKeyName(mType);
    
    if (indexKeyName.empty() || dataKenName.empty()) {
        return;
    }
    
    doRedisCmd("multi");
    doRedisCmd("hdel %s %lld %d", dataKenName.c_str(), voiceId);
    doRedisCmd("hdel %s %lld %b", indexKeyName.c_str(), voiceId);
    doRedisCmd("exec");
}

int64_t VoiceChat::getGenId()
{
    ++mGenId;
    
    string keyName = getVoiceDataKeyName(mType);
    
    if ( !keyName.empty() ) {
        doRedisCmd("set %s_genid %lld", keyName.c_str(), mGenId);
    }

    return mGenId;
}





bool VoiceChatMgr::init()
{
    for (int i = 1; i < eGameChatTypeMax; i++) {
        
        VoiceChat* newVoiceChat = new VoiceChat((GameChatType)i);
        
        if (newVoiceChat == NULL) {
            return false;
        }
        
        newVoiceChat->load();
        
        mVoiceData.push_back(newVoiceChat);
    }
    return true;
}

bool VoiceChatMgr::loadVoiceData()
{
    return true;
}

void VoiceChatMgr::update()
{
    int typeSize = mVoiceData.size();
    for (int i = 0; i < typeSize; i++) {
        VoiceChat* voiceChat = mVoiceData[i];
        if (voiceChat != NULL) {
            voiceChat->update();
        }
    }
}

VoiceChat* VoiceChatMgr::findVoiceType(int type)
{
    int typeSize = mVoiceData.size();
    for (int i = 0; i < typeSize; i++) {
        VoiceChat* chatData = mVoiceData[i];
        if (chatData != NULL && chatData->getType() == type) {
            return chatData;
        }
    }
    
    return NULL;
}

VoiceDataStruct* VoiceChatMgr::createVoiceData(int type, const vector<char> &voiceData, time_t createDate, bool save)
{
    VoiceChat* voiceChat = findVoiceType(type);
    
    if (voiceChat == NULL) {
        return NULL;
    }
    
    VoiceDataStruct* voice = voiceChat->createVoiceData(voiceData, createDate, save);
    
    return voice;
}

bool VoiceChatMgr::addData(int type, VoiceDataStruct* obj)
{
    return true;
}

VoiceDataStruct* VoiceChatMgr::findVoiceData(int type, int64_t voiceId)
{
    VoiceChat* voiceChat = findVoiceType(type);
    
    if (voiceChat  != NULL) {
        return voiceChat->findData(voiceId);
    }
    
    return NULL;
}

void VoiceChatMgr::removeVoiceData(int type, int64_t voiceId)
{
    VoiceChat* voiceChat = findVoiceType(type);
    
    if (NULL == voiceChat) {
        return;
    }
    
    voiceChat->removeVoiceData(voiceId);
}

void VoiceChatMgr::setRemove(int type, int64_t voiceId)
{
    VoiceDataStruct* voice = findVoiceData(type, voiceId);
    
    if (NULL != voice) {
        voice->mRemove = true;
    }
}
