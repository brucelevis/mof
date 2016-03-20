//
//  gate_mod.cpp
//  GameSrv
//
//  Created by prcv on 14-1-3.
//
//

#include "gate_mod.h"
#include "BaseType.h"
#include "Utils.h"
#include "cmd_helper.h"
#include "process.h"
#include "log.h"
#include "basedef.h"
#include "gate.h"
#include "groupmgr.h"
#include "basedef.h"



message_queue* GateMod::MQ = NULL;

GateMod::~GateMod()
{
    mApp->unRegisterServer(this);
    delete mGate;
}


void GateMod::registerIoReactor(const char* name, IoReactorCreator creator)
{
    mIoReactorCreators[name] = creator;
}

IoEventReactor* GateMod::createReactor(const char* name, const Json::Value& value)
{
    map<string, IoReactorCreator>::iterator iter = mIoReactorCreators.find(name);
    if (iter == mIoReactorCreators.end()) {
        return NULL;
    }
    IoEventReactor* ioReactor = (iter->second)(mGate, value);
    return ioReactor;
}


bool GateMod::createConnector(const Json::Value& config)
{
    string host = config["host"].asString();
    unsigned short port = config["port"].asInt();
    string reactorName = config["reactor"].asString();
    
    IoEventReactor* iReactor = createReactor(reactorName.c_str(), config);
    if (iReactor == NULL) {
        return false;
    }
    
    bool ret = mGate->create_connector(host.c_str(), port, iReactor);
    if (ret) {
        mReactors.push_back(iReactor);
    } else {
        delete iReactor;
    }
    
    return ret;
}

bool GateMod::createListener(const Json::Value& config)
{
    string host = config["host"].asString();
    unsigned short port = config["port"].asInt();
    string reactorName = config["reactor"].asString();
    int maxConn = config["max_conn"].asInt();
    
    IoEventReactor* iReactor = createReactor(reactorName.c_str(), config);
    if (NULL == iReactor) {
        return false;
    }
    
    int ret = mGate->create_server(host.c_str(), port, maxConn, iReactor);
    if (ret == -1) {
        delete iReactor;
    } else {
        mReactors.push_back(iReactor);
    }
    return ret != -1;
}

bool GateMod::init()
{
    mGate = new Gate;
    
    string confs = Process::env.getProperty("gate");
    Json::Value value;
    Json::Reader reader;
    
    reader.parse(confs, value);
    for (Json::Value::iterator iter = value.begin(); iter != value.end(); iter++) {
        string type = (*iter)["type"].asString();
        if (type == "connector") {
            createConnector(*iter);
        } else if (type == "listener") {
            if (!createListener(*iter)) {
                return false;
            }
            
            log_info("listening on: " << (*iter)["host"].asString() << ":" << (*iter)["port"].asInt());
        }
    }

    skynet_message msg;
    msg.session = 0;
    msg.data = 0;
    msg.source = 0;
    msg.sz = TIMER_MSG << 24;
    skynet_mq_push(MQ, &msg);
    
    return true;
}

void GateMod::onCommand(char* data)
{
    ICmdMsg* cmd = (ICmdMsg*)data;
    if (cmd) {
        cmd->handle(this);
    }
}

void GateMod::onTimer(int timerId)
{
    mGate->processEvents();
    
    skynet_message msg;
    msg.session = 0;
    msg.data = 0;
    msg.source = 0;
    msg.sz = (TIMER_MSG << 24);
    skynet_mq_push(MQ, &msg);
}
void GateMod::onNetMessage(int sid, char* data, int len)
{
    mGate->write(sid, data, len);
    delete[] data;
}
void GateMod::onMulticastMessage(char* data, int len)
{
    int sdatapos = Utils::alignNum(len, sizeof(int));
    int sessionnum = *((int*)(data + sdatapos));
    int *sessions = (int*)(data + sdatapos + sizeof(int));
    for (int i = 0; i < sessionnum; i++)
    {
        mGate->write(sessions[i], (char*)data, len);
    }
    delete[] data;
}

void GateMod::onBroadcastMessage(int groupId, char* data, int len)
{
    set<int>* sessions = SGroupMgr.getGroupMemeber(groupId);
    CheckCondition(sessions, return)
    set<int>::iterator iter;
    foreach(iter, *sessions)
    {
        mGate->write(*iter, data, len);
    }
    delete[] data;
}

void GateMod::onMessage(skynet_message *msg)
{
    int type = msg->sz >> 24;
    int dataLen = msg->sz & 0xFFFFFF;
    switch (type) {
        case NET_MSG:
            onNetMessage(msg->session, (char*)msg->data, dataLen);
            break;
        case TIMER_MSG:
            onTimer((long)msg->data);
            break;
        case CMD_MSG:
            onCommand((char*)msg->data);
            break;
        case BROADCAST_MSG:
            onBroadcastMessage(msg->session, (char*)msg->data, dataLen);
            break;
        case MULTICAST_MSG:
            onMulticastMessage((char*)msg->data, dataLen);
            break;
            
        default:
            break;
    }
}