#include "centerServer.h"
#include "ioReactor.h"
#include "BaseType.h"
#include "MsgBuf.h"
#include "sendMessage.h"
#include "clientMessageHelper.h"
#include "log.h"
#include "dbMgr.h"
#include "csPvp/csPvp.h"
#include "serverList.h"
#include "cmd_helper.h"
#include "centerScript.h"
#include "serverGroupMgr.h"

#include "centerLog.h"


bool
CenterServer::init() {
    
	return true;
}

void
CenterServer::beforeRun() {
	redisContext* context = createRedisContext();
    assert(context != NULL);
	registerRedisContext(context);

	getServerList()->init();
	getCsPvpMgr()->init();
	CenterScript::globalInit();

	skynet_message msg;
	msg.sz = (TIMER_MSG << 24);
	sendMessageToCenter(&msg);
}

void
CenterServer::afterRun() {
	redisContext* context = getRedisContext();
	unregisterRedisContext();
	redisFree(context);
}

void 
CenterServer::onMessage(skynet_message* msg) {
	int type = (msg->sz >> 24);
	int dataLen = (msg->sz & 0xFFFFFF);
	void* data = msg->data;
	int session = msg->session;

	switch (type) {
		case NET_MSG: {
			onNetMsg(session, data, dataLen);
            break;
		}
		case TIMER_MSG: {
			onTimer(0);
			break;
		}
		case CMD_MSG: {
			onCmd(data);
			break;
		}
	}
}

void
CenterServer::onNetMsg(int session, void* data, int len) {
	c_handleClientMsg(session, data, len);
	delete (char*)(data);
}

void
CenterServer::onTimer(int timerId) {
	runFrame();

	usleep(1000);
	skynet_message msg;
	msg.sz = (TIMER_MSG << 24);
	sendMessageToCenter(&msg);
}

void
CenterServer::onCmd(void* data) {
   ICmdMsg *cmdmsg = (ICmdMsg*)data;
    if (cmdmsg != NULL) {
        cmdmsg->handle();
        destroy_cmd(cmdmsg);
    }
}

void
CenterServer::runFrame() {
	static int prevTime = time(NULL);
	static int frameNum = 0;
    

	int curTime = time(NULL);
	if (curTime > prevTime + 1) {
		prevTime = curTime;
        
        getCsPvpMgr()->update();
        g_ServerList.onHeartBeat();
        
        g_BatRankListGroupMgr->onHeartBeat();
        g_ConsumeRankListGroupMgr->onHeartBeat();
        g_RechargeRankListGroupMgr->onHeartBeat();
        g_PetRankListGroupMgr->onHeartBeat();
        
//		log_info("run " << frameNum << " frame");
	}
}
