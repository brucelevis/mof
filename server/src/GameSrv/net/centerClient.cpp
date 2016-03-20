#include "centerClient.h"
#include "MsgBuf.h"
#include "process.h"
#include "main.h"
#include "log.h"
#include "msg.h"

CenterClient* CenterClient::sInstance = NULL;

CenterClient* 
CenterClient::instance() {
	return sInstance;
}

bool
CenterClient::globalInit() {
	sInstance = new CenterClient;
	sInstance->mSession = 0;
	sInstance->mConnected = false;
	sInstance->mRegistered = false;
	return true;
}

void
CenterClient::doRegister() {
	req_register_server req;
	req.server_id = Process::env.getInt("server_id");
	req.server_name = Process::env.getProperty("server_name");
	sendNetPacket(mSession, &req);
}

void
CenterClient::doKeepAlive() {
	if (!mRegistered) {
		return;
	}
}

void
CenterClient::onConnect(int session) {
	mSession = session;
	mConnected = true;
	doRegister();
}

void
CenterClient::onDisconnect() {
	mSession = 0;
	mConnected = false;
	mRegistered = false;
}


void
CenterClient::onResponse(void* data, int len) {

}

void
CenterClient::sendMessage() {
    
}

void
CenterClient::update() {

}

void
CenterClient::sendNetPacket(int session, INetPacket *packet) {
    packet->setSession(session);
    ::sendNetPacket(getSession(), packet);
}

