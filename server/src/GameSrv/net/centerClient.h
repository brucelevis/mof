#pragma once
#include "MsgBuf.h"
class INetPacket;

class CenterClient
{
public:
	static CenterClient* instance();
	static bool globalInit();

	void doRegister();
	void doKeepAlive();

	void onConnect(int session);
	void onResponse(void* data, int len);
	void onDisconnect();

	void update();
    
    void sendMessage();
    void sendNetPacket(int session, INetPacket* packet);
    
    int getSession() {
        return mSession;
    }
    
    bool isRegistered() {
        return mRegistered;
    }

private:
	int mSession;
	bool mConnected;
	bool mRegistered;

	static CenterClient* sInstance;
};