/*-------------------------------------------------------------------------
	created:	2010/09/16  17:50
	filename: 	e:\Project_SVN\Server\GamesScene\GameScene.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/
#pragma  once

#include "../Interface/ShellHandle.h"
#include "../CustomDefine/PublicMessage.h"
#include "LogTableMgr.h"

extern CLogTableMgr g_tbManager;
#define GM_RELOAD 6000

DEF_PUBLIC_MESSAGE(GM_LOG);
DEF_PUBLIC_MESSAGE(GM_RELOAD);

