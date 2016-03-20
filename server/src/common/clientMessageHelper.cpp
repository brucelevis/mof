#include "clientMessageHelper.h"
#include "NetPacket.h"

bool 
c_handleClientMsg(int session, void* data, int len) {
	ByteArray byteArray((char*)data, len);
    int connId = byteArray.read_int();
    int type = byteArray.read_int();
    int id = byteArray.read_int();
    NetPacketHander fun = hander_recvpacketfun(type, id);
    if (fun == NULL) {
    	return false;
    }

    do {
        INetPacket* packet = create_recvpacket(type, id);
        if (packet == NULL) {
            break;
        }
        
        try {
            packet->setSession(connId);
            packet->decode(byteArray);
        } catch (...) {
            delete packet;
            break;
        }

        fun(packet, session);
        delete packet;
    } while (0);

    return true;
}