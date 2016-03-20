/*-------------------------------------------------------------------------
	created:	2010/08/24  15:28
	filename: 	e:\Project_SVN\Server\ServerDemo\GameConfig.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#pragma once

#include "../Interface/GameType.h"
#include "FileSys/FileSys.h"

// �̳߳�ʵ��
struct  Pool_Entity
{
	int mod_id;
	int pool_id;
	int threads;
	string conn;
	int inrole_range_from;
	int role_range_to;
};

// ��Ϸ����
class CGameConfig : public CriticalObject
{
public:
	CGameConfig();
	~CGameConfig(void);

public:
	bool LoadConfig(const char* config);
	const char* GetAppPath();
	const char* GetResourcePath();
	const char* GetDllPath();

	//������ҷ�Χ�������ӳ�ID
	UI32 FindProcessor( UI32 RoleId, int mod);

	//����mod��ȡ���ӳ�
	bool GetIdPool( int mod, int* szIdPool, int nMaxSize = 0 );

private:
	void LoadProcessor(CIniFile2& ini);

public:
	// ������ip, �˿�
	string m_szServerIP;
	unsigned short m_nPort;

private:
	// ��Ϸ����·��
	string m_szGamePath;


	// ��Դ�ļ�·��
	string m_szResourcePath;

	// ģ���ļ�·��
	string m_szDllPath;

	// �̳߳ع���
	vector<Pool_Entity> m_ConVector;
};

