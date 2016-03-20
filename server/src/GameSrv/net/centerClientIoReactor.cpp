#include "centerClientIoReactor.h"
#include "main.h"
#include "cmd_def.h"

void 
CenterClientIoReactor::onEvent(int sid, int type, char* data, int len) {
	switch (type) {
		case IoEventConnect: {
            create_cmd(connectCenterServer, cmd);
            cmd->sid = sid;
            sendMessageToGame(sid, CMD_MSG, cmd, 0);
            break;
        }
        case IoEventRead: {
            if (len < 12) {
                break;
            }
            
            ByteArray byteArray(data, len);
            int connId = byteArray.read_int();
            int type = byteArray.read_int();
            int id = byteArray.read_int(); (void)id;
            if (type < 200) {
                //log_info("router msg (" << type << ", " << id << ") to " << connId);
                mGate->write(connId, (char*)&len, 4);
                mGate->write(connId, data, len);
            } else {
                char* msgData = new char[len];
                memcpy(msgData, data, len);
                sendMessageToGame(sid, NET_MSG, msgData, len);
            }

            break;
        }
        case IoEventClose: {
            create_cmd(disconnectCenterServer, cmd);
            sendMessageToGame(sid, CMD_MSG, cmd, 0);
            break;
        }
        default:
            break;
	}
}

const char* 
CenterClientIoReactor::getName() {
	return "CenterClientIoReactor";
}

IoEventReactor* 
CenterClientIoReactor::creator(Gate* gate, const Json::Value& config) {
	CenterClientIoReactor* reactor = new CenterClientIoReactor;
    reactor->mGate = gate;
	return reactor;
}