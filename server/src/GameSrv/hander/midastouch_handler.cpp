//
//  midastouch_handler.cpp
//  GameSrv
//
//  Created by jin on 15-6-11.
//
//

#include "hander_include.h"
#include "MidastouchMgr.h"

handler_msg(req_midastouch_info, req)
{
	hand_Sid2Role_check(sessionid, roleid, role)
    ack_midastouch_info ack;
	SMidastouchMou.getMidastouchInfo(role, ack.info);
    sendNetPacket(sessionid, &ack);
}}


handler_msg(req_midastouch, req)
{
	hand_Sid2Role_check(sessionid, roleid, role)
    ack_midastouch ack;
	ack.errorcode = SMidastouchMou.midastouch(role);
	SMidastouchMou.getMidastouchInfo(role, ack.info);
    sendNetPacket(sessionid, &ack);
}}