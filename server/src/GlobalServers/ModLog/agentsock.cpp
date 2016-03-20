#include "agentsock.h"


AgentSock g_AgentSock;

void* AgentProc(void* p)
{
    AgentSock* agentsock = (AgentSock*)p;
    if (agentsock == NULL)
    {
        return NULL;
    }

    while ( 1 )
    {
        agentsock->KeepAlive();
        usleep(1000000);
    }

    return NULL;
}

bool AgentSock::OpenConnect()
{
    string gamePath;

    W_GetAppPath(gamePath);
    fix_path(gamePath);

    string szConfigFile = gamePath + "PublicConfig.ini";

    CIniFile2 ini(szConfigFile.c_str());
    if (!ini.LoadFromFile())
	{
        return false;
    }

    string ip = ini.ReadString("AgentServer", "ip", "");
	int port = ini.ReadInteger("AgentServer", "port", 0);

	if (Open(ip.c_str(), port))
	{
        pthread_t m_hThread;
        if (pthread_create(&m_hThread, NULL, AgentProc, this) != 0)
        {
            return false;
        }

	    return true;
	}

	return false;
}


bool AgentSock::KeepAlive()
{
	if (!IsConnected())
	{
		return false;
	}

	CMsgTyped msg;
	msg.SetInt(0);
	msg.SetInt(0);
	msg.SetInt(6);

	return SendMessage(&msg);
}

void AgentSock::OnConnected()
{
	Log(LOG_NORMAL, "Connect to tx compass agent %s:%d.", m_szIP.c_str(), m_uPort);
}

void AgentSock::OnDisconnected()
{
	Log(LOG_NORMAL, "Disconnect from tx compass agent %s:%d.", m_szIP.c_str(), m_uPort);
}
