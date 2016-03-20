//
//  PrivateChatHistoryMgr.cpp
//  GameSrv
//
//  Created by nothing on 14-4-9.
//
//

#include "PrivateChatHistoryMgr.h"
#include "Role.h"
#include "json/json.h"
#include "Game.h"
#include "DBRolePropName.h"
#include "EnumDef.h"

static const string KEY_NAME = "chathistory";
static const int MAX_HISTORY_SIZE = 60;

PrivateChatHistoryMgr::PrivateChatHistoryMgr()
{
    mRole = NULL;
}

PrivateChatHistoryMgr::~PrivateChatHistoryMgr()
{
}

PrivateChatHistoryMgr *PrivateChatHistoryMgr::create()
{
    //保证一定不会申请内存失败
    PrivateChatHistoryMgr *mgr = new PrivateChatHistoryMgr();
    return mgr;
}

struct __ListComp
{
    bool operator()(const ChatItem &item1, const ChatItem &item2)
    {
        if (item1.date < item2.date)
        {
            return true;
        }
        return false;
    }
};

void PrivateChatHistoryMgr::load(Role *role)
{
    mRole = role;
    RedisResult result(redisCmd("hvals %s:%d", KEY_NAME.c_str(), mRole->getInstID()));
	int elementnum = result.getHashElements();
    
    vector<int> delItems;
    delItems.clear();
    
    for (int i = 0; i < elementnum; i++)
    {
        ChatItem item;
        VoiceInfoItem voice;
        if (!__parse(item, voice, result.readHash(i, "")))
        {
            continue;
        }
        
        if (item.senderId == 0 || item.senderRoleType == 0 || item.receiverId == 0) {
            delItems.push_back(item.date);
            continue;
        }
        
        mData.push_back(item);
        
        if (voice.mVoiceId) {
            mVoiceInfo.push_back(voice);
        }
    }
    
    //貌似redis hash存放不是顺序的，待验证
    mData.sort(__ListComp());
    
    if(delItems.size())
    {
        RedisArgs args;
        RedisHelper::beginHdel(args, KEY_NAME.c_str());
        
        for (int i = 0; i < delItems.size(); i++) {
            RedisHelper::appendHdel(args, delItems[i]);
        }
        
        RedisHelper::commitDelKeys(get_DbContext(), args);
    }

}


void PrivateChatHistoryMgr::saveAll()
{
    if (mData.empty())
    {
        return ;
    }
    for (ListIterator iter = mData.begin(); iter != mData.end(); ++iter)
    {
        string tmp;
        VoiceInfoItem voice = getVoiceInfo(eGameChatPlayerPrivateChat, (*iter).mVoiceId);
        if(__tostring(tmp, *iter, voice))
        {
            doRedisCmd("hmset %s:%d %d %s", KEY_NAME.c_str(), mRole->getInstID(), iter->date, tmp.c_str());
        }
    }
    
}

void PrivateChatHistoryMgr::update(req_privatechat &req, const VoiceInfoItem& voice)
{
//    int date = time(NULL);
    int date = Game::tick;
    
    //好友Role
    Role* fRole = SRoleMgr.GetRole(req.receiverId);
    
    ChatItem item;
    item.messageType = req.messageType;
    item.date = date;
    item.content = req.message;
    item.mVoiceId = voice.mVoiceId;
    
    item.senderId = mRole->getInstID();
    item.senderRoleType = mRole->getRoleType();
    item.senderLvl = mRole->getLvl();
    
    item.receiverId = req.receiverId;   //说定保存的roleid总是接收者的roldid
    
    //获取朋友的名字
    if (fRole)
    {
        item.receiverName = fRole->getRolename();
        item.receiverRoleType = fRole->getRoleType();
        item.receiverLvl = fRole->getLvl();
    }
    else
    {
        RedisResult result(redisCmd("hmget role:%d %s %s %s", req.receiverId, GetRolePropName(eRolePropRoleName), GetRolePropName(eRolePropRoleType), GetRolePropName(eRolePropLvl)));
        item.receiverName = result.readHash(0, "");
        item.receiverRoleType = result.readHash(1, 0);
        item.receiverLvl = result.readHash(2, 0);
    }
    
    update(item, voice);
    
    //更新好友聊天历史记录
    __updateFriendChatHistory(fRole, item, voice, req.receiverId);
}

void PrivateChatHistoryMgr::update(const ChatItem &item, const VoiceInfoItem& voice)
{
    //记录已经满了，删除前面
    if (mData.size() >= MAX_HISTORY_SIZE)
    {
        ChatItem item = *(mData.begin());
        mData.pop_front();
        //从数据库删除之前的数据
        __deleteFromDB(item.date, mRole->getInstID());
        removeVoiceInfo(item.mVoiceId);
    }
    mData.push_back(item);
    mVoiceInfo.push_back(voice);
    
    //插入新数据到数据库
    string str;
    __tostring(str, item, voice);
    __insertToDB(item.date, str, mRole->getInstID());
}

void PrivateChatHistoryMgr::sendChatHistory(req_private_chat_history &req)
{
    ListReverseIterator iter = mData.rbegin();
    
    ack_private_chat_history ack;
    
    if (mData.size() <= req.start)
    {
        ack.errorcode = CE_PRIVATE_FRIEND_CHAT_DATA_OVER;
        sendNetPacket(mRole->getSessionId(), &ack);
        return ;
    }
    
    int i = req.start;
    while (i > 0)
    {
        if (iter == mData.rend())
        {
            break;
        }
        ++iter;
        --i;
    }
    
    ack.errorcode = CE_OK;
    ack.start = req.start;
    ack.have_offline_msg = mRole->getIsHaveChatMsg();
    
    int num = req.num;
    for (; iter != mData.rend(); ++iter)
    {
        if (num <= 0)
        {
            break;
        }
        --num;
        
        obj_private_chat item;
        item.messageType = iter->messageType;
        item.date = iter->date;
        item.content = iter->content;
        
        item.sendRoleName = iter->senderName;
        item.sendRoleType = iter->senderRoleType;
        item.sendRoleid = iter->senderId;
        item.sendRoleLvl = iter->senderLvl;
        
        item.receiveRoleName = iter->receiverName;
        item.receiveRoleType = iter->receiverRoleType;
        item.receiveRoleid = iter->receiverId;
        item.receiveRoleLvl = iter->receiverLvl;
        
        item.voiceid = iter->mVoiceId;
        
        VoiceInfoItem voice;
        if (iter->mVoiceId) {
            voice = getVoiceInfo(eGameChatPlayerPrivateChat, iter->mVoiceId);
        }
        item.voiceDuration = voice.mVoiceDuration;
        item.translated = voice.mTranslated;
        
        ack.chats.push_back(item);
    }
    
    //已经没有数据了
    if (ack.chats.size() < req.num) {
        ack.errorcode = CE_PRIVATE_FRIEND_CHAT_DATA_OVER;
    }
    
    mRole->send(&ack);
    
    //设置聊天记录为已读
    if (mRole->getIsHaveChatMsg() == 1)
    {
        mRole->setIsHaveChatMsg(0);
    }
}

time_t PrivateChatHistoryMgr::getVoiceCreateDate(int64_t voiceId)
{
    ListReverseIterator iter = mData.rbegin();
    
    for (; iter != mData.rend(); ++iter)
    {
        if (iter->mVoiceId == voiceId) {
            return iter->date;
        }
    }
    
    return 0;
}

bool PrivateChatHistoryMgr::__parse(ChatItem &item, VoiceInfoItem &voice, const string &str)
{
    Json::Value value;
    Json::Reader reader;
    if (!reader.parse(str, value) || value.type() != Json::objectValue)
    {
        return false;
    }
    item.messageType = value["messageType"].asInt();
    item.date = value["date"].asInt();
    item.content = value["msg"].asString();
    
    item.senderId = value["senderId"].asInt();
    item.senderName = value["from"].asString();
    item.senderRoleType = value["fromRoleType"].asInt();
    item.senderLvl = value["senderLvl"].asInt();
    
    item.receiverName = value["to"].asString();
    item.receiverId = value["receiverId"].asInt();
    item.receiverRoleType = value["toRoleType"].asInt();
    item.receiverLvl = value["receiverLvl"].asInt();
    
    item.mVoiceId = atoll(value["voiceid"].asString().c_str());

    voice.mVoiceId = item.mVoiceId;
    voice.mVoiceDuration = value["voiceDuration"].asInt();
    voice.mChatType = value["chatType"].asInt();
    voice.mTranslated = value["translated"].asInt();
    
    return true;
}

bool PrivateChatHistoryMgr::__tostring(string &str, const ChatItem &item, const VoiceInfoItem& voice)
{
    Json::Value value;
    value["messageType"] = item.messageType;
    value["date"] = item.date;
    value["msg"] = item.content;
    
    value["from"] = item.senderName;
    value["fromRoleType"] = item.senderRoleType;
    value["senderLvl"] = item.senderLvl;
    value["senderId"] = item.senderId;
    
    value["to"] = item.receiverName;
    value["toRoleType"] = item.receiverRoleType;
    value["receiverLvl"] = item.receiverLvl;
    value["receiverId"] = item.receiverId;
    
    string voiceId = Utils::makeStr("%lld", voice.mVoiceId);
    value["voiceid"] = voiceId;
    value["voiceDuration"] = voice.mVoiceDuration;
    value["chatType"] = voice.mChatType;
    value["translated"] = voice.mTranslated;
    
    str = Json::FastWriter().write(value);
    if (str.size() > 0)
    {
        str.resize(str.size() - 1);
    }
    return true;
}

bool PrivateChatHistoryMgr::__deleteFromDB(const int &key, const int &roleid)
{
    doRedisCmd("hdel %s:%d %d", KEY_NAME.c_str(), roleid, key);
    return true;
}

bool PrivateChatHistoryMgr::__insertToDB(const int &key, const string &value, const int &roleid)
{
    doRedisCmd("hset %s:%d %d %s", KEY_NAME.c_str(), roleid, key, value.c_str());
    return true;
}

void PrivateChatHistoryMgr::__updateFriendChatHistory(Role *fRole, const ChatItem &item, const VoiceInfoItem& voice, const int &friendRoleid)
{
    //更新朋友记录，则from和to要互换
    ChatItem fItem;
    fItem.messageType = item.messageType;
    fItem.date = item.date;
    fItem.content = item.content;
    
    fItem.senderName = mRole->getRolename();
    fItem.senderRoleType = item.senderRoleType;
    fItem.senderId = item.senderId;
    fItem.senderLvl = item.senderLvl;
    
    fItem.receiverId = item.receiverId;
    fItem.receiverRoleType = item.receiverRoleType;
    fItem.receiverLvl = item.receiverLvl;
    
    fItem.mVoiceId = item.mVoiceId;
    
    VoiceInfoItem fVoice;
    fVoice.mVoiceId = voice.mVoiceId;
    fVoice.mVoiceDuration = voice.mVoiceDuration;
    fVoice.mChatType = voice.mChatType;
    
    //朋友在线
    if (fRole)
    {
        fRole->getChatHistoryMgr()->update(fItem, fVoice);
    }
    else //朋友不在线，直接操作数据库？
    {
        doRedisCmd("hset role:%d ishavechatmsg 1", friendRoleid);
        RedisResult result(redisCmd("hlen %s:%d", KEY_NAME.c_str(), friendRoleid));
        int lognum = result.readInt();
        //数据库记录放太多啦，要删除掉前面的
        if (lognum >= MAX_HISTORY_SIZE)
        {
            RedisResult res(redisCmd("hkeys %s:%d", KEY_NAME.c_str(), friendRoleid));
            int num = res.getHashElements();
            set<int> times;
            for (int i = 0; i < num; ++i)
            {
                //得到时间
                string itemstr = res.readHash(i, "");
                int tmp = Utils::safe_atoi(itemstr.c_str());
                times.insert(tmp);
            }
            if (!times.empty())
            {
                int mydate = *times.begin();
                __deleteFromDB(mydate, friendRoleid);
            }
        }
        
        //插入新数据到数据库
        string str;
        __tostring(str, fItem, fVoice);
        __insertToDB(fItem.date, str, friendRoleid);
    }
}

VoiceInfoItem PrivateChatHistoryMgr::getVoiceInfo(GameChatType type, int64_t voiceId)
{
    list<VoiceInfoItem>::reverse_iterator iter = mVoiceInfo.rbegin();
    list<VoiceInfoItem>::reverse_iterator endIter = mVoiceInfo.rend();
    
    for (; iter != endIter; iter++) {
        if ((*iter).mVoiceId == voiceId && type == (*iter).mChatType) {
            return (*iter);
        }
    }
    
    return VoiceInfoItem();
}

void PrivateChatHistoryMgr::updateVoiceInfo(const VoiceInfoItem& voice)
{
    mVoiceInfo.push_back(voice);
}

void PrivateChatHistoryMgr::removeVoiceInfo(int64_t voiceId)
{
    list<VoiceInfoItem>::iterator iter = mVoiceInfo.begin();
    list<VoiceInfoItem>::iterator endIter = mVoiceInfo.end();
    
    for (; iter != endIter; iter++) {
        if ((*iter).mVoiceId == voiceId) {
            mVoiceInfo.erase(iter);
            return;
        }
    }
}
