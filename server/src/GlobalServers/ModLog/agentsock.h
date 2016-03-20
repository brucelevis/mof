#ifndef MOD_LOG_AGENT_SOCK_H
#define MOD_LOG_AGENT_SOCK_H

#include "../Interface/ShellHandle.h"

class AgentSock : public CAsyncSocket
{
public:

	AgentSock(){}
	~AgentSock(){}

    bool OpenConnect();
    bool KeepAlive();

	void OnConnected();
	void OnDisconnected();
};

extern AgentSock g_AgentSock;
#endif

