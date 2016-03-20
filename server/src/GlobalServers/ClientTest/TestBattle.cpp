/*-------------------------------------------------------------------------
	created:	2010/10/09  13:45
	filename: 	e:\SVN\服务器端\Source\ClientTest\TestBattle.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#include <mmsystem.h>

extern CMyAsyncSock g_sock;

#pragma warning(disable: 4267)
#pragma warning(disable: 4305)

extern char recvBuffer[1024*55];

void OnNormalAttack(StringArray& sa)
{
	// 发送网络消息，登陆消息
	CMsgTyped msg;
	msg.SetInt(10);// 1 表示系统消息
	msg.SetInt(70);// 获取角色列表

	if (sa.size() > 1)
	{
		msg.SetInt(atoi(sa[1].c_str()));
	}
	else
	{
		msg.SetInt(1);
	}

	g_sock.SendMessage(&msg);
}

void ReturnOnNormalAttack(CMsgTyped* pmsg)
{
	CMsgTyped& retmsg = *pmsg;
	try
	{
		int succ = retmsg.IntVal();// 登陆成功 1 成功 0 失败

		if (succ == 0)
		{
			//Log(LOG_ERROR, "normal attack.");

			int nEffects = retmsg.IntVal();

			for (int i=0; i < nEffects; i++)
			{
				int id = retmsg.IntVal();
				int et = retmsg.IntVal();
				int life = retmsg.IntVal();
				int liferm=retmsg.IntVal();

				Log(LOG_NORMAL, "玩家对NPC[%d] 造成了 %d 点伤害。%d", id, life, GetTickCount());
			}
		}
	}
	catch (CExceptMsg e)
	{
		printf(e.GetMsg());
		printf("\n");
	}
}

void OnNormalAttack2(StringArray& sa)
{
	// 发送网络消息，登陆消息
	CMsgTyped msg;
	msg.SetInt(10);// 1 表示系统消息
	msg.SetInt(71);// 获取角色列表

	g_sock.SendMessage(&msg);
}

void ReturnOnNormalAttack2(CMsgTyped* pmsg)
{
	CMsgTyped& retmsg = *pmsg;
	try
	{
		int succ = retmsg.IntVal();// 登陆成功 1 成功 0 失败

		if (succ == 0)
		{
			Log(LOG_ERROR, "normal attack.");

			int nEffects = retmsg.IntVal();

			for (int i=0; i < nEffects; i++)
			{
				int id = retmsg.IntVal();
				int et = retmsg.IntVal();
				int life = retmsg.IntVal();
				int liferm=retmsg.IntVal();

				Log(LOG_NORMAL, "玩家对NPC[%d] 造成了 %d 点伤害, 目标还剩余 %d 生命值",id, life, liferm);
			}
		}
	}
	catch (CExceptMsg e)
	{
		printf(e.GetMsg());
		printf("\n");
	}
}
