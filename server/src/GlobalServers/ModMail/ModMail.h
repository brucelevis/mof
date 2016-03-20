/*-------------------------------------------------------------------------
	created:	2010/09/16  17:50
	filename: 	e:\Project_SVN\Server\GamesScene\GameScene.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/
#pragma  once

#include "../Interface/ShellHandle.h"
#include "../CustomDefine/PublicMessage.h"
#include <map>

//DEF_PUBLIC_MESSAGE(GM_LOGIN);
//DEF_PUBLIC_MESSAGE(GM_LOGOUT);



DEF_PUBLIC_MESSAGE(GM_SEND_MAIL);			 // 发送邮件
DEF_PUBLIC_MESSAGE(GM_MAIL_LIST);			 // 获取邮件列表
DEF_PUBLIC_MESSAGE(GM_READ_MAIL);			 // 查看邮件
DEF_PUBLIC_MESSAGE(GM_DELETE_MAIL);			 // 删除邮件
DEF_PUBLIC_MESSAGE(GM_GET_MAIL_ATTACHMENT);  // 获取邮件附件
DEF_PUBLIC_MESSAGE(GM_REGISTER_PLAYER);      // 邮件系统注册玩家
DEF_PUBLIC_MESSAGE(GM_UNREGPLAYER);          // 邮件系统注销玩家
DEF_PUBLIC_MESSAGE(GM_RENAME_INDEX);
DEF_PUBLIC_MESSAGE(GM_SEND_MAIL_PROPS);      // 邮件发送奖励


