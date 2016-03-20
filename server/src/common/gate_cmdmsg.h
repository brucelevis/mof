//
//  gate_cmdmsg.h
//  GameSrv
//
//  Created by 丁志坚 on 15/1/7.
//
//

#ifndef GameSrv_gate_cmdmsg_h
#define GameSrv_gate_cmdmsg_h

#include "cmd_helper.h"
#include "cmdmsg.h"
#include <string>
using namespace std;


begin_cmd(SetWorldServer)
    cmd_member(uint32_t, sessionId)
    cmd_member(uint32_t, worldId)
end_cmd()

begin_cmd(CreateServer)
    cmd_member(string, host)
    cmd_member(int, port)
    cmd_member(int, maxConn)
    cmd_member(string, reactorName)
end_cmd()

begin_cmd(CreateConnector)
    cmd_member(string, host)
    cmd_member(int, port)
    cmd_member(string, reactorName)
    cmd_member(string, serverName)
    cmd_member(int, serverId)
end_cmd()

#endif
