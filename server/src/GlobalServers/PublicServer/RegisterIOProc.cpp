/*-------------------------------------------------------------------------
	created:	2010/09/27  12:01
	filename: 	e:\Project_SVN\Server\GlobalServer\Request.cpp
	author:		Deeple
	
	purpose:	
---------------------------------------------------------------------------*/



#include "Game.h"


void CGame::OnRegisterIoMsgProc()
{
	// 注册处理类
	// 系统
	REGISTER_SYS_CLASS(GLOBAL_REGISTER_SERVER);
	REGISTER_SYS_CLASS(GLOBAL_UNREGISTER_SERVER);
	REGISTER_SYS_CLASS(GLOBAL_KEEP_ALIVE);
}

