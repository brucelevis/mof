/*-------------------------------------------------------------------------
	created:	2010/08/25  15:01
	filename: 	e:\Project_SVN\Server\ServerDemo\Game4.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#include "Game.h"

#pragma warning(disable:4311)
#pragma warning(disable:4312)

// 验证消息的有效性
// 如果连接未验证, 则验证其是否是登陆消息，
// 如果连接已验证， 则验证其是否是有效消息
bool VerifyMessage(bool bConnVerified,
				   char* pData,
				   size_t nBytes)
{
	// 验证消息是否有效
	if (!pData || nBytes == 0)
	{
		return false;
	}

	int iIndex = 0;

	// 传递到CMsgTyped 类准备用来验证消息
	CMsgTyped msg(nBytes, pData, false);

	bool bRet = true;
	try
	{
		if (bConnVerified)
		{
			while (1)
			{
				// 读取数据类型
				DATA_TYPES type = msg.TestType();

				// 如果是消息尾，验证成功退出，
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

					// 读取到不识别类型,退出
					Log(LOG_ERROR, "Verify Message, error data type:%d, index:%d.", type, iIndex);
					bRet = false;
					break;
				}

				iIndex ++;
			}
		}
		else
		{
			// 读取登陆消息
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

// 客户端消息输入(IO线程组输入)
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
			// 发送消息通知逻辑
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
			// 发送消息通知逻辑
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
