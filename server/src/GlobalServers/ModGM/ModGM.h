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


class IHaveSafeQueue
{
public:
	IHaveSafeQueue(){}
	virtual ~IHaveSafeQueue(){}

public:
	void SendMessage(CMsgTyped* pmsg)
	{
		m_msgQueue.PushMessage(pmsg);
	}

	void SendMessageNoWait(CMsgTyped* pmsg)
	{
		m_msgQueue.PushMessageNoWait(pmsg);
	}

	void MsgWait()
	{
		m_msgQueue.MsgWait();
	}

	CMsgTyped* GetMessage()
	{
		return m_msgQueue.GetMessage();
	}

protected:
	CSafeQueue m_msgQueue;
};


DEF_PUBLIC_MESSAGE(GM_INSERT_MAIL);			 // 发送邮件
DEF_PUBLIC_MESSAGE(GM_INSERT_GLOBALMAIL);	 // 发送全局邮件

