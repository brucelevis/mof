
/*-------------------------------------------------------------------------
	created:	2010/08/30  16:05
	filename: 	e:\Project_SVN\Server\ServerDemo\MessageEnum.h
	author:		Deeple
	
	purpose:	
---------------------------------------------------------------------------*/


#pragma once

#define SYSTEM_ID 0
#define SYSTEM "SYSTEM"

enum InnerMsgType
{
	INMSG_CLIENT_IO = 1,
	INMSG_CMD_INPUT,
	INMSG_COMPONENT, 
};

enum IO_MESSAGE
{
	IOMSG_READ,        // 收到网络消息
	IOMSG_CONNECT,     // 客户端socket连接上来
	IOMSG_DISCONNECT,  // 客户端socket关闭连接
};

enum COMPONENT_MESSAGE
{
	COMPMSG_SERVER_QUIT = 50000, // 服务器关闭
};

enum GLOBAL_MESSAGE
{
	GLOBAL_ADD_PLAYER,           // 注册玩家
	GLOBAL_DELETE_PLAYER,        // 删除玩家
	GLOBAL_LOGIN_PLAYER,        // 玩家登陆（公共服务器要负责数据转移）
	GLOBAL_LOGOUT_PLAYER,       // 玩家登出
	GLOBAL_REGISTER_SERVER,     // 服务器注册
	GLOBAL_UNREGISTER_SERVER,   // 服务器注销(服务器注销时, 
	GLOBAL_KEEP_ALIVE,          // 保持连接
};

enum GLOBAL_EXT_MESSAGE
{
	GLOBAL_SEND_MAIL,     // 发送邮件
	GLOBAL_MAIL_LIST,     // 获取邮件附件
	GLOBAL_READ_MAIL,     // 读取邮件
	GLOBAL_DELETE_MAIL,   // 删除邮件
	GLOBAL_GET_MAIL_ATTACHMENT, // 获取邮件附件
};

