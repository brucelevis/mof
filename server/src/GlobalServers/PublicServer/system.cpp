/*-------------------------------------------------------------------------
	created:	2010/09/27  12:01
	filename: 	e:\Project_SVN\Server\GlobalServer\Request.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#include "Game.h"

void SYSIOMSG(GLOBAL_REGISTER_SERVER)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
	int nServerId = pmsg->IntVal();
	const char* szServerName = pmsg->StringVal();
	int nPort = pmsg->IntVal();
	int mod = pmsg->IntVal();

	// ��Ӧֵ
	int nErrorCode = GE_OK;

	do
	{
		// ��ȡ����ip
		//SOCKADDR_IN addr; //windows
		//WShellAPI::GetAddr(id, &addr, NULL); //windows

		char ip[32];
		//WShellAPI::SockAddrAsStr(addr, ip, 32); //windows
		const char* pIp = WShellAPI::GetAddr( id ); //linux
		strcpy( ip, pIp );

		// ���÷������Ƿ��Ѿ���ע��
		ServerInfo* pServer = _SERVERLIST->FindServer(nServerId, mod);
		if (pServer)
		{
			if (strcmp(ip, pServer->ip.c_str()) != 0 ||
				strcmp(szServerName, pServer->servername.c_str()) != 0)
			{
				// ���ip �������ֲ�ͬ, ����ע��
				// �������Ѿ���ע��
				Log(LOG_ERROR, "GLOBAL_REGISTER_SERVER�� server id(%d) being used. \
					currentip(%s), newip(%s)", nServerId, pServer->ip.c_str(), ip);

				nErrorCode = GE_SERVER_ALREADYEXIST;
				break;
			}

			// ����ʹ�����������
			pServer->conn = id;
		}

		// ע�������
		if (!pServer)
		{
			_SERVERLIST->AddServer(nServerId, szServerName, ip, id, mod);
		}

		_SERVERLIST->Echo(id);

		Log(LOG_NORMAL, "[ %s ] Registered. sid:[%d], ip:[%s], mod:%d", szServerName, nServerId, ip, mod);

	} while (false);

	CMsgTyped msg;
	msg.SetInt(0);
	msg.SetInt(receiver);
	msg.SetInt(msgId);
	msg.SetInt(nErrorCode);

	_GAME->SendClientMessage(id, &msg);
}

void SYSIOMSG(GLOBAL_UNREGISTER_SERVER)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
	int serverid = pmsg->IntVal();

	int nRet = GE_OK;
//	if (!_SERVERLIST->RemoveServer(id))
//	{
//		nRet = GE_SERVER_NOT_FOUND;
//	}

    ServerInfo* pServer = _SERVERLIST->FindServer( id );
    if ( !pServer )
    {
        nRet = GE_SERVER_NOT_FOUND;
    }
    else
    {
        _IFMGR->UnRegisterServer( serverid );
        _SERVERLIST->RemoveServer( id );
    }

	CMsgTyped msg;
	msg.SetInt(0);
	msg.SetInt(receiver);
	msg.SetInt(msgId);
	msg.SetInt(nRet);

	_GAME->SendClientMessage(id, &msg);
}

void SYSIOMSG(GLOBAL_KEEP_ALIVE)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
	//int serverid = pmsg->IntVal();

	int nRet = GE_OK;
	if (!_SERVERLIST->Echo(id))
	{
		nRet = GE_SERVER_NOT_FOUND;
	}

	CMsgTyped msg;
	msg.SetInt(0);
	msg.SetInt(receiver);
	msg.SetInt(msgId);
	msg.SetInt(GE_OK);

	_GAME->SendClientMessage(id, &msg);
}

