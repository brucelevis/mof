
/*-------------------------------------------------------------------------
	created:	2010/08/30  16:05
	filename: 	e:\Project_SVN\Server\ServerDemo\MessageEnum.h
	author:		Deeple
	
	purpose:	
---------------------------------------------------------------------------*/


#pragma once

#define SYSTEM_ID 0
#define SYSTEM "SYSTEM"

enum InnerMsgType
{
	INMSG_CLIENT_IO = 1,
	INMSG_CMD_INPUT,
	INMSG_COMPONENT, 
};

enum IO_MESSAGE
{
	IOMSG_READ,        // �յ�������Ϣ
	IOMSG_CONNECT,     // �ͻ���socket��������
	IOMSG_DISCONNECT,  // �ͻ���socket�ر�����
};

enum COMPONENT_MESSAGE
{
	COMPMSG_SERVER_QUIT = 50000, // �������ر�
};

enum GLOBAL_MESSAGE
{
	GLOBAL_ADD_PLAYER,           // ע�����
	GLOBAL_DELETE_PLAYER,        // ɾ�����
	GLOBAL_LOGIN_PLAYER,        // ��ҵ�½������������Ҫ��������ת�ƣ�
	GLOBAL_LOGOUT_PLAYER,       // ��ҵǳ�
	GLOBAL_REGISTER_SERVER,     // ������ע��
	GLOBAL_UNREGISTER_SERVER,   // ������ע��(������ע��ʱ, 
	GLOBAL_KEEP_ALIVE,          // ��������
};

enum GLOBAL_EXT_MESSAGE
{
	GLOBAL_SEND_MAIL,     // �����ʼ�
	GLOBAL_MAIL_LIST,     // ��ȡ�ʼ�����
	GLOBAL_READ_MAIL,     // ��ȡ�ʼ�
	GLOBAL_DELETE_MAIL,   // ɾ���ʼ�
	GLOBAL_GET_MAIL_ATTACHMENT, // ��ȡ�ʼ�����
};

