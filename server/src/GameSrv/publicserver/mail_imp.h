//
//  mail_imp.h
//  GameSrv
//
//  Created by prcv on 13-7-19.
//
//

#ifndef GameSrv_mail_imp_h
#define GameSrv_mail_imp_h

class CMsgTyped;

void onRegisterMailServerSuccess();

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
              const char* extparams);

bool sendMailWithName(int roleid, const char* sender, const char* receiver, const char* mailName,
              const char* attach);

void onSendMail(int roleid, CMsgTyped* msg);

/*
 邮件列表（2001）	 参数类型	 参数名称	 参数说明
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 string	 roleName	 角色名字
 int32	 from	 从哪封邮件开始，》=1
 int32	 count	 邮件数量
 */
bool getMailList(int roleid, const char* owner, int from, int count);

void onGetMailList(int roleid, CMsgTyped* msg);

/*
 读取邮件（2002）	 参数类型	 参数名称	 参数说明
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 string	 mailId	 邮件guid
 string	 ownerName	 所有者名字
 string    mailtitle        邮件标题
 */
bool getMail(int roleid, const char* mailid, const char* ownername, const char* mailtitle);

void onGetMail(int roleid, CMsgTyped* msg);
/*
 
 删除邮件（2003）	 参数类型	 参数名称	 参数说明
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 string	 mailId	 邮件guid
 string	 ownerName	 所有者名字
 */
bool deleteMail(int roleid, const char* mailid, const char* ownername);

void onDeleteMail(int roleid, CMsgTyped* msg);
/*
 读取附件（2004）	 参数类型	 参数名称	 参数说明
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 string	 mailId	 邮件guid
 string	 ownerName	 所有者名字
 string    mailtitle       邮件标题
 */
bool getAttach(int roleid, const char* mailid, const char* ownername, const char* mailtitle);

void onGetAttach(int roleid, CMsgTyped* msg);
/*
 注册玩家（2005）	 参数类型	 参数名称	 参数说明
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 string	 playerName	 角色名称
 */
bool mailAddRole(const char* role);

/*
 删除玩家（2006）	 参数类型	 参数名称	 参数说明
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 string	 playerName	 角色名称
 */
bool mailDelRole(const char* role);

/*
 修改玩家名字   
 int32	 reserv	 备用，填0
 int32	 receiver	 填0
 int32	 cmdId	 消息id
 string playerName  角色名称
 string newName  新角色名称
 */
bool mailChangeRoleName(int roleid, const char* prevName, const char* newName);

void onMailChangeRolename(int roleid, CMsgTyped* msg);



#endif
