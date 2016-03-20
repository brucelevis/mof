/*-------------------------------------------------------------------------
	created:	2010/09/15  10:51
	filename: 	e:\Project_SVN\Server\ServerDemo\Interface\ObjectHandle.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#pragma once

#include "../Interface/ShellHandle.h"

typedef void* HMODULE;  //windows 2 linux

class CInterfaceManager
{
public:
	CInterfaceManager()
	{
		FreeModules();
	}

	~CInterfaceManager()
	{
		FreeModules();
	}

	void LoadModules(const char* szFolderPath, IGlobal* pGameInterface)
	{
		assert(szFolderPath);
		if (!szFolderPath)
		{
			return;
		}

		CompareResult files;
		List_Files(szFolderPath, files);

		for (size_t i=0; i < files.size(); i++)
		{
			string szFileName = string(szFolderPath) + files[i].m_szFileName;

			if (szFileName.size() <= 3
				|| szFileName.find(".so") != szFileName.size() - 3)
			{
				continue;
			}

			// ¼ÓÔØÄ£¿é,²¢Ö´ÐÐ
			bool bFindInterface = false;
			HMODULE hMod = dlopen( szFileName.c_str(), RTLD_LAZY);

			if (hMod)
			{
				typedef bool (*FUNC)(IGlobal* pInterface);

				FUNC func = (FUNC)dlsym(hMod, "Initialise");
				if (func)
				{
					if (func(pGameInterface))
					{
						bFindInterface = true;
					}
				}

				if (!bFindInterface)
				{
					dlclose(hMod);
					Log(LOG_ERROR, "%s, no interface found.", files[i].m_szFileName.c_str());
				}
				else
				{
					m_hModules.push_back(hMod);
					Log(LOG_NORMAL, "%s loaded.", files[i].m_szFileName.c_str());
				}
			}
			else
			{
				dlclose(hMod);
				Log(LOG_ERROR, "%s, invalid module.", files[i].m_szFileName.c_str());
			}
		}
	}

	void OnHeartBeat(int nBeat)
	{
		for (size_t i=0; i < m_hModules.size(); i++)
		{
			typedef void (*pfHeartBeat)(int nBeat);
            pfHeartBeat pFunc = (pfHeartBeat)dlsym(m_hModules[i], "OnHeartBeat");

			if ( pFunc )
			{
				pFunc(nBeat);
			}
		}
	}

	void UnRegisterServer(int ServerId)
	{
	    for ( size_t i = 0; i < m_hModules.size(); i ++ )
	    {
	        typedef void (*pfUnRegSvr)(int);
	        pfUnRegSvr pFunc = (pfUnRegSvr)dlsym( m_hModules[i],  "UnRegisterServer" );

	        if ( pFunc )
	        {
                pFunc( ServerId );
	        }
	    }
	}

	void FreeModules()
	{
		for (size_t i=0; i < m_hModules.size(); i++)
		{
			dlclose(m_hModules[i]);
		}

		m_hModules.clear();
	}

private:
	vector<HMODULE> m_hModules;
};

