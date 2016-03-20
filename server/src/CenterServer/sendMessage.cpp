#include "sendMessage.h"
#include "centerApp.h"
#include "BaseType.h"
#include "MsgBuf.h"
#include "INetPacket.h"

void 
sendMessageToCenter(skynet_message* msg) {
	g_CenterApp.sendMessageToCenter(msg);
}

void 
sendMessageToGate(skynet_message* msg) {
    g_CenterApp.sendMessageToGate(msg);
}


void 
sendMessageToLog(skynet_message* msg) {
    g_CenterApp.sendMessageToLog(msg);
}


void
sendNetData(int session, void* data, int len) {
    char* sendData = new char[len + 4];
    memcpy(sendData, &len, 4);
    memcpy(sendData + 4, data, len);
    
    skynet_message netMsg;
    netMsg.sz = (NET_MSG << 24) + len + 4;
    netMsg.source = 0;
    netMsg.session = session;
    netMsg.data = sendData;
    sendMessageToGate(&netMsg);
}

void 
sendNetPacket(int session, INetPacket* packet) {
    ByteArray byteArray;
    try {
        packet->build(byteArray);
    } catch (...) {
        return;
    }
    sendNetData(session, byteArray.m_pContent, byteArray.m_nWrPtr);
}
