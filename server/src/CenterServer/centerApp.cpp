//
//  center_app.cpp
//  CenterServer
//
//  Created by 丁志坚 on 15/6/1.
//
//

#include "centerApp.h"
#include "gate_mod.h"
#include "inifile.h"
#include "process.h"
#include "ioReactor.h"
#include "configManager.h"
#include "serverGroupMgr.h"
#include "logservice.h"

CenterApp g_CenterApp;

ServerApp* 
getApp() {
    return &g_CenterApp;
}


CServerGroupMgr* g_BatRankListGroupMgr      = NULL;
CServerGroupMgr* g_ConsumeRankListGroupMgr  = NULL;
CServerGroupMgr* g_RechargeRankListGroupMgr = NULL;
CServerGroupMgr* g_PetRankListGroupMgr      = NULL;

void
CenterApp::sendMessageToGate(skynet_message* msg) {
    send(mGateMod->getId(), msg);
}

void
CenterApp::sendMessageToCenter(skynet_message* msg) {
    send(mCenterServer->getId(), msg);
}

void 
CenterApp::sendMessageToLog(skynet_message* msg) {
    send(mLogService->getId(), msg);
}

int
CenterApp::init(int argc, char** argv) {
    Process::execpath = os::getAppPath();
    IniFile inifile(Process::execpath + "/center.ini");

    Process::env.setInt("server_id", inifile.getValueT("root", "server_id", 0));
    Process::env.setString("server_name", inifile.getValue("root", "server_name", ""));
    Process::env.setString("logpath", inifile.getValue("root", "logpath"));

    Process::env.setString("db_host",     inifile.getValue ("root", "db_host",  "127.0.0.1"));
    Process::env.setInt   ("db_port",     inifile.getValueT("root", "db_port",  6379));
    Process::env.setString("db_password", inifile.getValue ("root", "db_password"));
    Process::env.setInt   ("db_index",    inifile.getValueT("root", "db_index", 10));
    
    Process::env.setInt("testmode", inifile.getValueT("root", "testmode", 0));
    Process::env.setString("lang", inifile.getValue("root", "lang", "zh_cn"));
    Process::env.setString("platform", inifile.getValue("root", "platform"));

    Json::Value gateConfig;
    string host = inifile.getValue("root", "host", "0.0.0.0");
    unsigned short port = inifile.getValueT("root", "port", 18090);
    int maxConn = inifile.getValueT("root", "maxconn", 1024);
    Json::Value listenerConfig;
    listenerConfig["type"] = "listener";
    listenerConfig["host"] = host;
    listenerConfig["port"] = port;
    listenerConfig["reactor"] = CenterIoReactor::getName();
    listenerConfig["max_conn"] = maxConn;
    gateConfig.append(listenerConfig);
    Process::env.setProperty("gate", Json::FastWriter().write(gateConfig).c_str());
    
    g_ConfigManager.init();
    
    g_BatRankListGroupMgr = new CServerGroupMgr;
    g_BatRankListGroupMgr->init(eSortBat);
    
    g_ConsumeRankListGroupMgr = new CServerGroupMgr;
    g_ConsumeRankListGroupMgr->init(eSortConsume);
    
    g_RechargeRankListGroupMgr = new CServerGroupMgr;
    g_RechargeRankListGroupMgr->init(eSortRecharge);
    
    g_PetRankListGroupMgr = new CServerGroupMgr;
    g_PetRankListGroupMgr->init(eSortPet);

    mGateMod = new GateMod(this);
    mGateMod->registerIoReactor(CenterIoReactor::getName(),
                                (IoReactorCreator)CenterIoReactor::creator);

    mCenterServer = new CenterServer(this);

    mLogService = new LogService(this);
    //get server list

    return 0;
}

int
CenterApp::main(int argc, char** argv) {
    log_info("init log service");
    if (!mLogService->init()) {
        log_info("init log service fail");
    }

    log_info("init gate module");
    if (!mGateMod->init()) {
        return 1;
    }
    
    log_info("init center module");
    if (!mCenterServer->init()) {
        return 1;
    }

    runBackground();
    waitForTerminationRequest();
    return 0;
}

void
CenterApp::deinit() {
    
}