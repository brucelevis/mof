//
//  serverList.cpp
//  CenterServer
//
//  Created by 丁志坚 on 15/6/9.
//
//

#include "serverList.h"
#include "configManager.h"
#include "serverCfg.h"
#include "serverGroupMgr.h"
#include "csPvp/csPvp.h"

ServerList g_ServerList;

ServerList* 
getServerList() {
	return &g_ServerList;
}


bool
ServerList::init() {
	ServerCfg* cfg = g_ConfigManager.getServerCfg();
	vector<int> serverIds = cfg->getAllRunId();

	for (vector<int>::iterator iter = serverIds.begin(); iter != serverIds.end(); iter++) {
		int serverId = *iter;
		ServerCfgDef* serverCfgDef = cfg->getServerCfgDef(serverId);
		if (serverCfgDef == NULL) {
			continue;
		}
		ServerEntity* server = ServerEntity::create(serverId, serverCfgDef->mName.c_str());
		addServer(server);

		g_BatRankListGroupMgr->addServerEntity(server);
		g_ConsumeRankListGroupMgr->addServerEntity(server);
		g_RechargeRankListGroupMgr->addServerEntity(server);
		g_PetRankListGroupMgr->addServerEntity(server);
	}

	return true;
}


void 
ServerList::addServer(ServerEntity* server) {
	mData[server->getServerId()] = server;
}

bool 
ServerList::onServerRegister(int serverId, int session) {
	ServerEntity* server = getServer(serverId);
	if (server == NULL) {
		return false;
	} else if (server->getState() == ServerEntity::kConnect) {
		return false;
	}
	server->onConnect(session);
	mSessionIndex[session] = server;
	return true;
}

void
ServerList::onServerDisconnect(int session) {
	ServerEntity* server = getServerBySession(session);
	if (server) {
		server->onDisconnect();
		mSessionIndex.erase(session);
	}
    
    CsPvp* csPvp = getCsPvpMgr()->getCsPvpByServerId(server->getServerId());
    if (csPvp) {
        csPvp->getPvpStateMgr()->endPvpByServerId(server->getServerId());
    }
}

ServerEntity* 
ServerList::getServer(int serverId) {
	map<int, ServerEntity*>::iterator iter = mData.find(serverId);
	if (iter == mData.end()) {
		return NULL;
	}
	return iter->second;
}

ServerEntity* 
ServerList::getServerBySession(int session) {
	map<int, ServerEntity*>::iterator iter = mSessionIndex.find(session);
	if (iter == mSessionIndex.end()) {
		return NULL;
	}
	return iter->second;
}

void 
ServerList::traverse(Traverser* traverser) {
	map<int, ServerEntity*>::iterator iter = mData.begin();
	while (iter != mData.end()) {
		traverser->callback(iter->second);
		iter++;
	}
}

void
ServerList::onHeartBeat()
{
    map<int, ServerEntity*>::iterator iter = mData.begin();
	for (; iter != mData.end(); iter++) {
        iter->second->onHeartBeat();
    }
}
