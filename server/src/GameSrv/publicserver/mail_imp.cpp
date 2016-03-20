//
//  mailinc.cpp
//  GameSrv
//
//  Created by prcv on 13-7-19.
//
//
#include "mail_imp.h"
#include "psmgr.h"
#include "mailserver.h"
#include "GameLog.h"
#include "Role.h"
#include "EnumDef.h"
extern void pet_casino_wager_compensate();

void onRegisterMailServerSuccess()
{
    static int times = 0;
    if (times == 0){
        //幻兽大比拼押金补偿
        pet_casino_wager_compensate();
        times++;
    }
}


/*
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 string	 sender	 发送者名字
 string	 receiver	 接收者名字
 string	 title	 标题
 string	 content	 内容
 string	 attach	 附件
 string	 ext_params	 自定义参数， 发送邮件
 的返回消息会带回来
 */
bool sendMail(int roleid, const char* sender, const char* receiver,
              const char* title, const char* content, const char* attach,
              const char* extparams)
{
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(roleid);
    msg.SetInt(GM_SEND_MAIL);
    msg.SetString(sender);
    msg.SetString(receiver);
    msg.SetString(title);
    msg.SetString(content);
    msg.SetString(attach);
    msg.SetString(extparams);
    
    return SPSMgr.sendMessage(eMailServer, msg);
}

bool sendMailWithName(int roleid, const char* sender, const char* receiver, const char* mailName,
              const char* attach)
{
    MailFormat *f = MailCfg::getCfg(mailName);
    if (f == NULL) {
        return false;
    }
    
    if (strcmp(sender, "") == 0) {
        sender = f->sendername.c_str();
    }
    
    return sendMail(roleid, sender, receiver, f->title.c_str(), f->content.c_str(), attach, "");
}

/*
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 int32	 errorCode	 错误码
 string  mailid
 string	 attach	 附件
 string	 sender	 发送者
 string	 receiver	 接收者
 string	 title	 标题
 string  sendtime 
 string	 ext_params	 自定义参数，发送邮件
 的返回消息会带回来
 */
void onSendMail(int roleid, CMsgTyped* msg)
{
    notify_new_mail notify;
    
    int errorcode = msg->IntVal();
    string mailId= msg->StringVal();
    string attach = msg->StringVal();
    string sender = msg->StringVal();
    string receiver = msg->StringVal();
    string title = msg->StringVal();
    string sendtime = msg->StringVal();
    string extparams = msg->StringVal();
    
    notify.mail.mailId = mailId;
    notify.mail.hasattach = (attach.size() > 0);
    notify.mail.mailtitle = title;
    notify.mail.IsReaded = 0;
    notify.mail.send_time = sendtime;
    notify.mail.sender = sender;
    
    if (errorcode == 0 && roleid == 0)
    {
        int receiverid = Utils::safe_atoi(extparams.c_str(), -1);
        if (receiverid == 0)
        {
            broadcastPacket(WORLD_GROUP, &notify);
        }
        else if (receiverid > 0)
        {
            Role* recvrole = SRoleMgr.GetRole(receiverid);
            if (recvrole)
            {
                sendNetPacket(recvrole->getSessionId(), &notify);
            }
        }
    }
    else
    {
        xyerr("invalid mail by role:%d", roleid);
    }
}

/*
邮件列表（2001）	 参数类型	 参数名称	 参数说明
int32	 reserv	 备用，填0
int32	 receiver	 填0
int32	 cmdId	 消息id
string	 roleName	 角色名字
int32	 from	 从哪封邮件开始，》=1
int32	 count	 邮件数量
 */
bool getMailList(int roleid, const char* owner, int from, int count)
{
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(roleid);
    msg.SetInt(GM_MAIL_LIST);
    msg.SetString(owner);
    msg.SetInt(from);
    msg.SetInt(count);
    
    return SPSMgr.sendMessage(eMailServer, msg);
}
/*
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 int32	 errorCode	 错误id,0表示无错
 int32	 totleCount	 邮件总数量
 int32   from        客户端参数
 int32	 mailCount	 返回邮件数量
 string	 mailId	 邮件GUID
 string	 title	 标题
 string	 sender	 发送者
 string	 receiver	 接收者
 int32	 isReaded	 是否已读
 string	 sendTime	 发送时间
 int32	 hasAttach	 错误id,0表示无错
 */
void onGetMailList(int roleid, CMsgTyped* msg)
{
    Role* role = SRoleMgr.GetRole(roleid);
    if (!role)
    {
        return;
    }
    
    ack_get_mail_list ack;
    
    ack.errorcode = msg->IntVal();
    ack.count = msg->IntVal();
    ack.from = msg->IntVal();
    
    int mailcount = msg->IntVal();
    for (int i = 0; i < mailcount; i++)
    {
        obj_mail mail;
        mail.mailId = msg->StringVal();
        mail.mailtitle = msg->StringVal();
        mail.sender = msg->StringVal();
        (void)msg->StringVal();
        mail.IsReaded = msg->IntVal();
        mail.send_time = msg->StringVal();
        mail.hasattach = msg->IntVal();
        ack.mails.push_back(mail);
    }
    
    sendNetPacket(role->getSessionId(), &ack);
}

/*
读取邮件（2002）	 参数类型	 参数名称	 参数说明
int32	 reserv	 备用，填0
int32	 receiver	 填0
int32	 cmdId	 消息id
string	 mailId	 邮件guid
string	 ownerName	 所有者名字
string     mailtitle        邮件标题
 */
bool getMail(int roleid, const char* mailid, const char* ownername, const char* mailtitle)
{
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(roleid);
    msg.SetInt(GM_READ_MAIL);
    msg.SetString(mailid);
    msg.SetString(ownername);
    msg.SetString(mailtitle);
    
    return SPSMgr.sendMessage(eMailServer, msg);
}
/*
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 int32	 errorCode	 错误id,0表示无错
 string	 guid	 邮件guid
 string	 content	 邮件内容
 string	 attachment	 邮件附件
 string    mailtitle         邮件标题

 */
void onGetMail(int roleid, CMsgTyped* msg)
{
    Role* role = SRoleMgr.GetRole(roleid);
    if (!role)
    {
        return;
    }
    
    int errorcode = msg->IntVal();
    string mailid = msg->StringVal();
    string content = msg->StringVal();
    string attach = msg->StringVal();
    string mailtitle = msg->StringVal();
    
    string waitmail = role->getProperty("waitmail");
    if (waitmail == mailid)
    {
        vector<string> itemstrs = StrSpilt(attach, ";");
        ItemArray items;
        RewardStruct reward;
        rewardsCmds2ItemArray(itemstrs, items, reward);
        GridArray grids;
        if(role->preAddItems(items, grids) == CE_OK)
        {
            if (getAttach(roleid, mailid.c_str(), role->getRolename().c_str(), mailtitle.c_str()))
            {
                return;
            }
        }
    
        ack_get_attach ack;
        ack.errorcode = CE_BAG_FULL;
        ack.mailid = mailid;
        sendNetPacket(role->getSessionId(), &ack);
        
        role->setProperty("waitmail", "");
    }
    else
    {
        ack_get_mail ack;
        ack.errorcode = errorcode;
        ack.mailid = mailid;
        ack.content = content;
        ack.attach = attach;
        sendNetPacket(role->getSessionId(), &ack);
    }
}

/*

删除邮件（2003）	 参数类型	 参数名称	 参数说明
int32	 reserv	 备用，填0
int32	 receiver	 填0
int32	 cmdId	 消息id
string	 mailId	 邮件guid
string	 ownerName	 所有者名字
 */

bool deleteMail(int roleid, const char* mailid, const char* ownername)
{
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(roleid);
    msg.SetInt(GM_DELETE_MAIL);
    msg.SetString(mailid);
    msg.SetString(ownername);
    
    return SPSMgr.sendMessage(eMailServer, msg);
}
/*
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 int32	 errorCode	 错误id,0表示无错
 string	 guid	 邮件guid
 */
void onDeleteMail(int roleid, CMsgTyped* msg)
{
    Role* role = SRoleMgr.GetRole(roleid);
    if (!role)
    {
        return;
    }
    
    ack_delete_mail ack;
    ack.errorcode = msg->IntVal();
    ack.mailid = msg->StringVal();
    sendNetPacket(role->getSessionId(), &ack);
}
/*
读取附件（2004）	 参数类型	 参数名称	 参数说明
int32	 reserv	 备用，填0
int32	 receiver	 填0
int32	 cmdId	 消息id
string	 mailId	 邮件guid
string	 ownerName	 所有者名字
string     mailtitle        邮件标题
 */

bool getAttach(int roleid, const char* mailid, const char* ownername, const char* mailtitle)
{
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(roleid);
    msg.SetInt(GM_GET_MAIL_ATTACHMENT);
    msg.SetString(mailid);
    msg.SetString(ownername);
    msg.SetString(mailtitle);
    
    return SPSMgr.sendMessage(eMailServer, msg);
}
/*
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 int32	 errorCode	 错误码
 string	 guid	 邮件id
 string	 attachment	 附件内容
 string    mailtitle        邮件标题
 */
void onGetAttach(int roleid, CMsgTyped* msg)
{
    ack_get_attach ack;
    ack.errorcode = msg->IntVal();
    ack.mailid = msg->StringVal();
    string attach = msg->StringVal();
    string mailtitle = msg->StringVal();
    
    Role* role = SRoleMgr.GetRole(roleid);
    if (!role)
    {
        xyerr("role:%d get attach %s fail", roleid, attach.c_str());
        return;
    }
    
    
    if (ack.errorcode == 0)
    {
        vector<string> itemstrs = StrSpilt(attach, ";");
        ItemArray items;
        string comefrom = Utils::makeStr("mail_attach:%s", mailtitle.c_str());
        
        if (role->addAwards(itemstrs, items, comefrom.c_str()))
        {
            LogMod::addLogGetMailAttach(role, ack.mailid.c_str(), mailtitle.c_str(), attach.c_str());
        }
    }
    
    if (role->getProperty("waitmail") == ack.mailid)
    {
        role->setProperty("waitmail", "");
    }
    
    sendNetPacket(role->getSessionId(), &ack);
}
/*
注册玩家（2005）	 参数类型	 参数名称	 参数说明
int32	 reserv	 备用，填0
int32	 receiver	 填0
int32	 cmdId	 消息id
string	 playerName	 角色名称
 */

bool mailAddRole(const char* rolename)
{
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(0);
    msg.SetInt(GM_REGISTER_PLAYER);
    msg.SetString(rolename);
    
    return SPSMgr.sendMessage(eMailServer, msg);
}

/*
 删除玩家（2006）	 参数类型	 参数名称	 参数说明
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 string	 playerName	 角色名称
 */

bool mailDelRole(const char* rolename)
{
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(0);
    msg.SetInt(GM_UNREGPLAYER);
    msg.SetString(rolename);
    
    return SPSMgr.sendMessage(eMailServer, msg);
}


bool mailChangeRoleName(int roleid, const char* prevName, const char* newName)
{
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(0);
    msg.SetInt(GM_RENAME_INDEX);
    msg.SetString(prevName);
    msg.SetString(newName);
    
    return SPSMgr.sendMessage(eMailServer, msg);
}

void onMailChangeRolename(int roleid, CMsgTyped* msg)
{
}

