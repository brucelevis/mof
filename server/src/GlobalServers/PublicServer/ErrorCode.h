/*-------------------------------------------------------------------------
	created:	2010/09/28  16:09
	filename: 	e:\Project_SVN\Server\GlobalServer\ErrorCode.h
	author:		Deeple
	
	purpose:	
---------------------------------------------------------------------------*/


#pragma once


enum GLOBAL_ERROR
{
	GE_OK = 0,                 // �޴�
	GE_PLAYER_EXIST,           // ����Ѿ�����
	GE_PLAYER_NOT_EXIST,       // ��Ҳ�����
	GE_CANNOT_ACCESS_DATABASE, // ���ݿ���� ����ʧ��
	GE_ROLE_BEING_USED,        // ��ɫ��ʹ��
	GE_SERVER_NOT_FOUND,       // ������������
	GE_SERVER_ALREADYEXIST,    // �������Ѿ�����
	GE_BAD_PLAYER_LIST,        // ��Ч������б�
	GE_SERVER_NOT_REGISTERD,   // ������δע��
};

