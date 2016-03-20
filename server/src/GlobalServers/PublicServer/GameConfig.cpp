/*-------------------------------------------------------------------------
	created:	2010/09/27  17:42
	filename: 	e:\Project_SVN\Server\GlobalServer\GameConfig.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/


#include "GameConfig.h"
#include "../Interface/ShellHandle.h"

extern CAsyncProcMgr* _ASYNCPROC;

CGameConfig::CGameConfig()
{
}

CGameConfig::~CGameConfig(void)
{

}

bool CGameConfig::LoadConfig(const char* config)
{
	// 获取资源路径
	W_GetAppPath(m_szGamePath);
	fix_path(m_szGamePath);

	m_szResourcePath = m_szGamePath + "Resource/";
	m_szDllPath = m_szGamePath + "Module/";

	// 读取服务器配置
	string szConfigFile;
	if (config == NULL) {
		szConfigFile = m_szGamePath + "PublicConfig.ini";
	} else {
		szConfigFile = m_szGamePath + config;
	}
	CIniFile2 ini(szConfigFile.c_str());
	if (!ini.LoadFromFile())
	{
		return false;
	}

	// 读取ip, 端口
	m_szServerIP = ini.ReadString("system", "ServerIP", "");
	m_nPort = ini.ReadInteger("system", "Port", 0);

	if (m_nPort == 0)
	{
		Log(LOG_ERROR, "invalid port(0).");
		return false;
	}

	// 加载线程池
	LoadProcessor(ini);

	return true;
}

const char* CGameConfig::GetAppPath()
{
	return m_szGamePath.c_str();
}

const char* CGameConfig::GetResourcePath()
{
	return m_szResourcePath.c_str();
}

const char* CGameConfig::GetDllPath()
{
	return m_szDllPath.c_str();
}

UI32 CGameConfig::FindProcessor( UI32 RoleId, int mod)
{
	SMART_LOCK_THIS();

	vector<Pool_Entity>::iterator ItData;
	for( ItData = m_ConVector.begin(); ItData != m_ConVector.end(); ItData++ )
	{
		if ( RoleId >= (UI32)ItData->inrole_range_from && RoleId <= (UI32)ItData->role_range_to
			&& ItData->mod_id == mod)
		{
			return ItData->pool_id;
		}
	}

	return 0;
}

bool CGameConfig::GetIdPool( int mod, int* szIdPool, int nMaxSize )
{
	SMART_LOCK_THIS();

	if ( !szIdPool ) return false;

	//清空
	for ( int i = 0; i < nMaxSize; i++ )
	{
		*( szIdPool + i ) = 0;
	}

	int j = 0;
	for ( int i = 0; i < (int)m_ConVector.size(); i++ )
	{
		if ( m_ConVector[i].mod_id == mod )
		{
			*( szIdPool + j++ ) = m_ConVector[i].pool_id; //按顺序从0开始放进数组
		}
	}

	return true;
}

void CGameConfig::LoadProcessor(CIniFile2& ini)
{
	// 读取不了
	int i = 0;

	char section_name[128];
	while( (i ++) < 100 )
	{
		sprintf(section_name, "db_conn_pool%d", i);

		Pool_Entity TemData;

		TemData.mod_id = ini.ReadInteger(section_name, "mod_id",   -1);
		TemData.pool_id = ini.ReadInteger(section_name, "pool_id", -1);

		if (TemData.mod_id < 0 || TemData.pool_id < 1)
		{
			continue;
		}

		TemData.threads = ini.ReadInteger(section_name, "threads", 1);
		TemData.conn = ini.ReadString(section_name, "conn", "");
		TemData.inrole_range_from = ini.ReadInteger(section_name, "role_range_from", 0);
		TemData.role_range_to = ini.ReadInteger(section_name, "role_range_to", 0);

		if (TemData.conn.size() > 0 && TemData.threads > 0
			&& TemData.role_range_to > TemData.inrole_range_from && TemData.inrole_range_from >= 0)
		{
			m_ConVector.push_back(TemData);

			// 创建处理器
			if (!_ASYNCPROC->AddProcessor(TemData.pool_id, section_name, TemData.threads, TemData.conn.c_str()))
			{
				Log(LOG_ERROR, "creating pool faild.., pool_name:  %s", section_name);
				exit(0);
			}
		}
		else
		{
			Log(LOG_ERROR, "config error!!, exiting..., pool_name:  %s", section_name);
			exit(0);
		}
	}
}
