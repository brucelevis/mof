//
//  PrivateChatHistoryMgr.h
//  GameSrv
//
//  Created by nothing on 14-4-9.
//
//

#ifndef __GameSrv__PrivateChatHistoryMgr__
#define __GameSrv__PrivateChatHistoryMgr__


#include <string>
#include <list>

#include "msg.h"
#include "ChatRoom.h"

using namespace std;

struct ChatItem
{
    int messageType;
    int date;           //时间戳
    string content;     //内容
    string senderName;        //自己-》好友，from为空
    string receiverName;          //好友-》自己，to为空
    
    int senderRoleType;    //发送者角色类型
    int receiverRoleType;      //接受者角色类型
    
    int senderLvl;      //
    int receiverLvl;        //
    
    int senderId;
    int receiverId;         //接收者的roleid
    int64_t mVoiceId;
    
    ChatItem(): date(0),
                content(""),
                senderName(""),
                senderRoleType(0),
                senderLvl(0),
                receiverName(""),
                receiverRoleType(0),
                receiverLvl(0),
                receiverId(0),
                mVoiceId(0)
    {}
};

class Role;
class PrivateChatHistoryMgr
{
public:
    typedef list<ChatItem> List;
    typedef List::iterator ListIterator;
    typedef List::reverse_iterator ListReverseIterator;

    PrivateChatHistoryMgr();
    ~PrivateChatHistoryMgr();
    
    static PrivateChatHistoryMgr *create();
    
    //load chat history from db
    void load(Role *role);
    void saveAll();
    
    //update chat history
    void update(req_privatechat &req, const VoiceInfoItem& voice);
    void update(const ChatItem &item, const VoiceInfoItem& voice);
    
    //只更新语音信息
    void updateVoiceInfo(const VoiceInfoItem& voice);
    
    void sendChatHistory(req_private_chat_history &req);
    time_t getVoiceCreateDate(int64_t voiceId);
    void removeVoiceInfo(int64_t voiceId);
    
    VoiceInfoItem getVoiceInfo(GameChatType type, int64_t voiceId);
    
private:
    //string to item
    bool __parse(ChatItem &item, VoiceInfoItem &voice, const string &str);
    //item to string
    bool __tostring(string &str, const ChatItem &item, const VoiceInfoItem& voice);
    //delete a key from db
    bool __deleteFromDB(const int &key, const int &roleid);
    //insert a key to db
    bool __insertToDB(const int &key, const string &value, const int &roleid);
    //update friend chat histroy, if friend is offline, fRole is NULL
    void __updateFriendChatHistory(Role *fRole, const ChatItem &item, const VoiceInfoItem& voice, const int &friendRoleid);
    
    Role *mRole;
    List mData;
    list<VoiceInfoItem> mVoiceInfo;
};



#endif /* defined(__GameSrv__PrivateChatHistoryMgr__) */
