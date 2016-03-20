/*-------------------------------------------------------------------------
	created:	2010/09/19  17:58
	filename: 	e:\SVN\服务器端\Source\GamePlayer\PublicMessage.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#pragma once

enum GM_SYS_MESSAGE
{
    GM_REGISTER = 4,
    GM_UNREGISTER = 5,
    GM_HEARTBEAT = 6
};

enum GM_EXT_MESSAGE
{
    GM_LOGIN_XINYOU = 1000,
    GM_LOGIN_91 = 1001,
    GM_LOGIN_UC = 1002,
    GM_LOGIN_TB = 1003,
    GM_LOGIN_PP = 1004,

    // 邮件服务器
	GM_SEND_MAIL=2000,      // 发送邮件
	GM_MAIL_LIST,			// 获取邮件列表
	GM_READ_MAIL,			// 查看邮件
	GM_DELETE_MAIL,         // 删除邮件
	GM_GET_MAIL_ATTACHMENT, // 获取邮件附件
	GM_REGISTER_PLAYER,     // 注册玩家
	GM_UNREGPLAYER,         // 注销玩家
	GM_RENAME_INDEX,

	GM_LOG = 3000,

	// GM 服务器
	GM_INSERT_MAIL = 4000,
	GM_INSERT_GLOBALMAIL,
    GM_BAN_ROLE,
    GM_ALLOW_ROLE,
    GM_BAN_CHAT,
    GM_ALLOW_CHAT,
    GM_NOTIFY,
    GM_KICK_ROLE,
    GM_SET_ROLE_PROP,
	GM_APP_CMD,				// 指令应用
};

enum GLOBAL_ERROR
{
	GE_OK,                   // 0.成功
	GE_ERROR_ACCOUNT_PASS,   // 账号密码错误
	GE_ACCOUNT_INVALID,      // 账号不可用
	GE_CANNOT_ACCESS_DB,     // 无法操作数据库

	GE_PLAYER_NOT_FOUND,      // juese bu cunzai
};


enum GLOBAL_MODULE_DEF
{
    eLoginServer = 1,
    eMailServer = 2,
    eLogServer = 3,
    eGmServer = 4,
    eCenterServer = 5
};

