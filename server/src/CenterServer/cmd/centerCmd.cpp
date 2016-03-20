#include <stdint.h>
#include "centerCmd.h"
#include "serverList.h"

handler_cmd(ServerDisconnect)
{
	getServerList()->onServerDisconnect(sessionId);
}
