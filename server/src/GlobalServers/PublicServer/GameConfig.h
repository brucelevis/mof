/*-------------------------------------------------------------------------
	created:	2010/08/24  15:28
	filename: 	e:\Project_SVN\Server\ServerDemo\GameConfig.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#pragma once

#include "../Interface/GameType.h"
#include "FileSys/FileSys.h"

// 线程池实例
struct  Pool_Entity
{
	int mod_id;
	int pool_id;
	int threads;
	string conn;
	int inrole_range_from;
	int role_range_to;
};

// 游戏配置
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

	//根据玩家范围查找连接池ID
	UI32 FindProcessor( UI32 RoleId, int mod);

	//根据mod获取连接池
	bool GetIdPool( int mod, int* szIdPool, int nMaxSize = 0 );

private:
	void LoadProcessor(CIniFile2& ini);

public:
	// 服务器ip, 端口
	string m_szServerIP;
	unsigned short m_nPort;

private:
	// 游戏运行路径
	string m_szGamePath;


	// 资源文件路径
	string m_szResourcePath;

	// 模块文件路径
	string m_szDllPath;

	// 线程池管理
	vector<Pool_Entity> m_ConVector;
};

