#include "ioReactor.h"
#include "sendMessage.h"
#include "BaseType.h"
#include "MQ.h"
#include "log.h"
#include "socket.h"
#include "centerCmd.h"
#include <string.h>

#define SEND_BUF_SIZE (10 * 1024 * 1024)
#define RECV_BUF_SIZE (10 * 1024 * 1024)

void 
CenterIoReactor::onEvent(int sid, int type, char* data, int len) {
	switch (type) {
		case IoEventAccept: {
			onClientConnect(sid);
			break;
		}
		case IoEventRead: {
			onRecvData(sid, data, len); 
			break;
		}
		case IoEventClose: {
			onClientDisconnect(sid);
			break;
		}
		default: {
			break;
		}
	}
}

void
CenterIoReactor::onClientConnect(int sid) {
    log_info("on client connect");
    Socket* sock = mGate->getSocket(sid);
    if (sock) {
    	sock->setSendBufSize(SEND_BUF_SIZE);
    	sock->setRecvBufSize(RECV_BUF_SIZE);
    }
}

void
CenterIoReactor::onClientDisconnect(int sid) {
    log_info("on client disconnect");
    create_cmd(ServerDisconnect, cmd);
    cmd->sessionId = sid;

	skynet_message msg;
	msg.source = 0;
	msg.session = sid;
	msg.sz = ((CMD_MSG << 24) + 0);
	msg.data = (void*)cmd;
    sendMessageToCenter(&msg);
}

void
CenterIoReactor::onRecvData(int sid, char* data, int len) {
//    log_info("on recv data");
	skynet_message msg;
	msg.source = 0;
	msg.session = sid;
	msg.sz = ((NET_MSG << 24) + len);
	msg.data = new char[len];
	memcpy(msg.data, data, len);
	sendMessageToCenter(&msg);
}

const char* 
CenterIoReactor::getName() {
	return "CenterIoReactor";
}

IoEventReactor* 
CenterIoReactor::creator(Gate* gate, const Json::Value& config) {
	CenterIoReactor* reactor = new CenterIoReactor;
	reactor->mGate = gate;
	return reactor;
}
