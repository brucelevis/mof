//
//  ServerInfoMgr.cpp
//  GameSrv
//
//  Created by jin on 14-8-12.
//
//

#include "ServerInfoMgr.h"
#include "DynamicConfig.h"
#include "cmd_def.h"
#include "Game.h"
#include "psmgr.h"


void sendServerInfoMsgToGame(void * param)
{
	create_cmd(update_server_info, request);
    sendCmdMsg(Game::MQ, request);
}

addTimerDef(sendServerInfoMsgToGame, NULL, 30, FOREVER_EXECUTE);


void updateServerInfo()
{
	{
		bool isCon = false;
		bool isReg = false;
		PublicServer *server = SPSMgr.getPublicServer(eMailServer);
		if (NULL != server) {
			isCon = server->isConnected();
			isReg = server->isRegistered();
		}
		doRedisCmd("hmset server_info %s %d %s %d", "mail_con", isCon, "mail_reg", isReg);
	}
	{
		bool isCon = false;
		bool isReg = false;
		PublicServer *server = SPSMgr.getPublicServer(eGmServer);
		if (NULL != server) {
			isCon = server->isConnected();
			isReg = server->isRegistered();
		}
		doRedisCmd("hmset server_info %s %d %s %d", "gm_con", isCon, "gm_reg", isReg);
	}
	{
		bool isCon = false;
		bool isReg = false;
		PublicServer *server = SPSMgr.getPublicServer(eLogServer);
		if (NULL != server) {
			isCon = server->isConnected();
			isReg = server->isRegistered();
		}
		doRedisCmd("hmset server_info %s %d %s %d", "log_con", isCon, "log_reg", isReg);
	}
	
	
	doRedisCmd("hset server_info %s %d", "time", time(NULL));
	
}