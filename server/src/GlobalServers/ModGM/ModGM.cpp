/*-------------------------------------------------------------------------
	created:	2010/09/16  17:50
	filename: 	e:\Project_SVN\Server\GamesScene\GameScene.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#include "ModGM.h"
#include "async_func.h"
#include "TimeTick.h"
#include "GmTaskManager.h"

IGlobal* g_pGame = NULL;

extern "C"
{

void OnHeartBeat(int nBeat)
{
    TimeTick::cur_time = time(NULL);
    g_GmTaskManager.update();

    if (nBeat % 5 == 0)
    {
        int processid = g_pGame->FindProcessor(1, eGmServer);
        g_pGame->Call(processid, AsyncProc_HandleGmTask, NULL);
    }
}

}

void RegisterCallBack()
{
    REG_PUBLIC_MESSAGE(GM_INSERT_MAIL);			 // 发送邮件
    REG_PUBLIC_MESSAGE(GM_INSERT_GLOBALMAIL);	 // 发送全局邮件
}

// 发送邮件
void PUB_MSG(GM_INSERT_MAIL)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{

}

// 发送全局邮件
void PUB_MSG(GM_INSERT_GLOBALMAIL)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{

}
