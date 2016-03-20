/*-------------------------------------------------------------------------
	created:	2010/10/09  13:45
	filename: 	e:\SVN\��������\Source\ClientTest\TestBattle.cpp
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
	// ����������Ϣ����½��Ϣ
	CMsgTyped msg;
	msg.SetInt(10);// 1 ��ʾϵͳ��Ϣ
	msg.SetInt(70);// ��ȡ��ɫ�б�

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
		int succ = retmsg.IntVal();// ��½�ɹ� 1 �ɹ� 0 ʧ��

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

				Log(LOG_NORMAL, "��Ҷ�NPC[%d] ����� %d ���˺���%d", id, life, GetTickCount());
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
	// ����������Ϣ����½��Ϣ
	CMsgTyped msg;
	msg.SetInt(10);// 1 ��ʾϵͳ��Ϣ
	msg.SetInt(71);// ��ȡ��ɫ�б�

	g_sock.SendMessage(&msg);
}

void ReturnOnNormalAttack2(CMsgTyped* pmsg)
{
	CMsgTyped& retmsg = *pmsg;
	try
	{
		int succ = retmsg.IntVal();// ��½�ɹ� 1 �ɹ� 0 ʧ��

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

				Log(LOG_NORMAL, "��Ҷ�NPC[%d] ����� %d ���˺�, Ŀ�껹ʣ�� %d ����ֵ",id, life, liferm);
			}
		}
	}
	catch (CExceptMsg e)
	{
		printf(e.GetMsg());
		printf("\n");
	}
}
