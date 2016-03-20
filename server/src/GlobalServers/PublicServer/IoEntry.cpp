/*-------------------------------------------------------------------------
	created:	2010/08/25  15:01
	filename: 	e:\Project_SVN\Server\ServerDemo\Game4.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#include "Game.h"

#pragma warning(disable:4311)
#pragma warning(disable:4312)

// ��֤��Ϣ����Ч��
// �������δ��֤, ����֤���Ƿ��ǵ�½��Ϣ��
// �����������֤�� ����֤���Ƿ�����Ч��Ϣ
bool VerifyMessage(bool bConnVerified,
				   char* pData,
				   size_t nBytes)
{
	// ��֤��Ϣ�Ƿ���Ч
	if (!pData || nBytes == 0)
	{
		return false;
	}

	int iIndex = 0;

	// ���ݵ�CMsgTyped ��׼��������֤��Ϣ
	CMsgTyped msg(nBytes, pData, false);

	bool bRet = true;
	try
	{
		if (bConnVerified)
		{
			while (1)
			{
				// ��ȡ��������
				DATA_TYPES type = msg.TestType();

				// �������Ϣβ����֤�ɹ��˳���
				if (type == TYPE_NONE)
				{
					break;
				}

				if (type == TYPE_INT){
					msg.IntVal();
				}
				else if (type == TYPE_STRING){
					msg.StringVal();
				}
				else if (type == TYPE_FLOAT){
					msg.FloatVal();
				}
				else if (type == TYPE_WIDESTR){
					msg.WideStrVal();
				}
				else if (type == TYPE_BINARY)
				{
					void* pData;
					size_t nLen;
					msg.BinaryVal(pData, nLen);
				}
				else
				{
					string out;
					BinaryToText(out, pData, nBytes);

					Log(LOG_ERROR, out.c_str());

					// ��ȡ����ʶ������,�˳�
					Log(LOG_ERROR, "Verify Message, error data type:%d, index:%d.", type, iIndex);
					bRet = false;
					break;
				}

				iIndex ++;
			}
		}
		else
		{
			// ��ȡ��½��Ϣ
			int sysCmd = msg.IntVal();
			int cmdType = msg.IntVal();
			const char* account = msg.StringVal();
			const char* password = msg.StringVal();
		}
	}
	catch (CExceptMsg e)
	{
		bRet = false;
		Log(LOG_ERROR, "Verify Message, %s\n", e.GetMsg());
	}

	return bRet;
}

// �ͻ�����Ϣ����(IO�߳�������)
void NETEVENTCALLBACK(const PersistID& id,
					  IoAction action,
					  char* pData,
					  size_t nBytes,
					  char* pUserData)
{
	switch(action)
	{
	case IoRead:
		{
			CMsgTyped msg;
			msg.SetInt(INMSG_CLIENT_IO);
			msg.SetInt(IOMSG_READ);
			msg.SetInt((int)id.nIndex);
			msg.SetInt((int)id.nIdent);
			msg.SetBinary(pData, nBytes);
			_GAME->SendMessage(&msg);

			break;
		}
//	case IoAfterRead:
//		{
//			_GAME->MsgWait();
//
//			break;
//		}
	case IoAccept:
		{
			// ������Ϣ֪ͨ�߼�
			CMsgTyped msg;
			msg.SetInt(INMSG_CLIENT_IO);
			msg.SetInt(IOMSG_CONNECT);
			msg.SetInt((int)id.nIndex);
			msg.SetInt((int)id.nIdent);
			_GAME->SendMessage(&msg);

			break;
		}
	case IoEnd:
		{
			// ������Ϣ֪ͨ�߼�
			CMsgTyped msg;
			msg.SetInt(INMSG_CLIENT_IO);
			msg.SetInt(IOMSG_DISCONNECT);
			msg.SetInt((int)id.nIndex);
			msg.SetInt((int)id.nIdent);
			_GAME->SendMessage(&msg);

			_SERVERLIST->ServerDisconnected((PersistID &)id);

			break;
		}
	default:
		break;
	}
}
