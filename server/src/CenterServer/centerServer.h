#pragma once

#include "server.h"

class NetMessage;
class ISystem;
class CMsgTyped;
class ServerMgr;

class CenterServer : public IServer
{
public:
	CenterServer(ServerApp* app) : IServer(app) {

	}

	virtual void beforeRun();
	virtual void afterRun();
	virtual bool init();
    virtual void onMessage(skynet_message* msg);

private:
    void onClientConnect(int session);
    void onNetMsg(int session, void* data, int size);
    void onClientDisconnect(int session);

    void onTimer(int timerId);
    void onCmd(void* data);

    void runFrame();
};