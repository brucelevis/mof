//
//  serverList.h
//  CenterServer
//
//  Created by 丁志坚 on 15/6/9.
//
//

#ifndef __CenterServer__serverList__
#define __CenterServer__serverList__

#include <stdio.h>
#include <map>
#include <string>
#include "basedef.h"
#include "serverEntity.h"

using namespace std;

class ServerList
{
public:
	bool init();

    ServerEntity* getServer(int serverId);
    ServerEntity* getServerBySession(int session);

    void addServer(ServerEntity* server);
    bool onServerRegister(int serverId, int session);
    void onServerDisconnect(int session);
    void traverse(Traverser* traverser);
    
    void onHeartBeat();
    
private:
    map<int, ServerEntity*> mData;
    map<int, ServerEntity*>::iterator mIter;

    map<int, ServerEntity*> mSessionIndex;
};

extern ServerList g_ServerList;
extern ServerList* getServerList();


#endif /* defined(__CenterServer__serverList__) */
