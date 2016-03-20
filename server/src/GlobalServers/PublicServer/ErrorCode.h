/*-------------------------------------------------------------------------
	created:	2010/09/28  16:09
	filename: 	e:\Project_SVN\Server\GlobalServer\ErrorCode.h
	author:		Deeple
	
	purpose:	
---------------------------------------------------------------------------*/


#pragma once


enum GLOBAL_ERROR
{
	GE_OK = 0,                 // 无错
	GE_PLAYER_EXIST,           // 玩家已经存在
	GE_PLAYER_NOT_EXIST,       // 玩家不存在
	GE_CANNOT_ACCESS_DATABASE, // 数据库错误， 操作失败
	GE_ROLE_BEING_USED,        // 角色被使用
	GE_SERVER_NOT_FOUND,       // 服务器不存在
	GE_SERVER_ALREADYEXIST,    // 服务器已经存在
	GE_BAD_PLAYER_LIST,        // 无效的玩家列表
	GE_SERVER_NOT_REGISTERD,   // 服务器未注册
};

