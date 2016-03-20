#include "msg.h"
#include "NetPacket.h"
#include "sendMessage.h"
#include "serverList.h"
#include "EnumDef.h"
#include "Defines.h"

handler_msg(req_cs_get_role_battle, req)
{
    int serverId = req.target_server_id;

    int errorcode = 1;
    do {
        ServerEntity* server = getServerList()->getServer(serverId);
        if (server == NULL) {
            break;
        }
		if (server->getState() == ServerEntity::kDisconnect) {
			break;
		}

        sendNetPacket(server->getSession(), &req);
        errorcode = 0;
    } while (0);
    
    if (errorcode) {
        ack_cs_get_role_battle ack;
        ack.errorcode = errorcode;
        ack.src_role_id = req.src_role_id;
        ack.src_server_id = req.src_server_id;
        ack.target_role_id = req.target_role_id;
        ack.target_server_id = req.target_server_id;
        ack.custom = req.custom;
        sendNetPacket(sessionid, &ack);
    }
}}


handler_msg(ack_cs_get_role_battle, ack)
{
	int serverId = ack.src_server_id;

	do {
		ServerEntity* server = getServerList()->getServer(serverId);
		if (server == NULL) {
			break;
		}
		if (server->getState() == ServerEntity::kDisconnect) {
			break;
		}

		sendNetPacket(server->getSession(), &ack);
	} while (0);
}}
