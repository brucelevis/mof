#include "cmd_helper.h"
#include "cmdmsg.h"
#include <string>
using namespace std;


begin_cmd(ServerDisconnect)
    cmd_member(uint32_t, sessionId)
end_cmd()