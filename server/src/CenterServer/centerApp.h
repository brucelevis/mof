//
//  center_app.h
//  CenterServer
//
//  Created by 丁志坚 on 15/6/1.
//
//

#ifndef __CenterServer__center_app__
#define __CenterServer__center_app__

#include <stdio.h>
#include "server_app.h"
#include "centerServer.h"

struct skynet_message;
class GateMod;
class LogService;

class CenterApp : public ServerApp
{
public:
    virtual int init(int argc, char** argv);
    virtual int main(int argc, char** argv);
    virtual void deinit();

    void sendMessageToCenter(skynet_message* msg);
	void sendMessageToGate(skynet_message* msg);
	void sendMessageToLog(skynet_message* msg);

	GateMod* getGate() {return mGateMod;}
	CenterServer* getCenterServer() {return mCenterServer;}

private:
	GateMod* mGateMod;
	LogService* mLogService;
	CenterServer* mCenterServer;
};

extern CenterApp g_CenterApp;

#endif /* defined(__CenterServer__center_app__) */
