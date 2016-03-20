//
//  gamemanage.cpp
//  GameSrv
//
//  Created by prcv on 13-6-20.
//
//

#include "psmgr.h"
#include "gate.h"
#include "mailserver.h"
#include "log_server.h"
#include "gmserver.h"
#include "universe_server.h"
#include "Game.h"

void PublicServer::onConnect()
{
    isconnected = true;
    registerServer();
    
    mHeartBeatTimer.reset(Game::tick + 10);
}

void PublicServer::onReconnect()
{
    isconnected = true; 
    registerServer();
    
    mHeartBeatTimer.reset(Game::tick + 10);
}

void PublicServer::onDisconnect()
{
    isconnected = false;
    mIsRegistered = false;
}

void PublicServer::update(float dt)
{
    if (mHeartBeatTimer.passed(Game::tick))
    {
        serverHeartbeat();
        mHeartBeatTimer.reset(Game::tick + 10);
    }
}

void PublicServer::onResponse(CMsgTyped* msg)
{
    (void)msg->IntVal();
    int receiver = msg->IntVal();
    int cmdid = msg->IntVal();
    
    switch (cmdid)
    {
        case GM_REGISTER:
        {
            (void)receiver;
            int errorcode = msg->IntVal();
            log_info(name << " register " << errorcode);
            if (errorcode == 0)
            {
                mIsRegistered = true;
                onRegister();
            }
            break;
        }
        case GM_HEARTBEAT:
        {
            int errorcode = msg->IntVal();
            (void)errorcode;
            break;
        }
        case GM_UNREGISTER:
        {
            log_info(name << " unregister ");
            break;
        }
        default:
            onResponse(receiver, cmdid, msg);

    }
}

void PublicServer::registerServer()
{
    if (!isConnected())
    {
        return;
    }
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(0);
    msg.SetInt(GM_REGISTER);
    msg.SetInt(Process::env.getInt("server_id"));
    msg.SetString(Process::env.getString("host").c_str());
    msg.SetInt(Process::env.getInt("port"));
    msg.SetInt(id);
    sendNetData(sid, (char*)msg.GetData(), msg.GetLength());
}

void PublicServer::unregisterServer()
{
    if (!isConnected())
    {
        return;
    }
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(0);
    msg.SetInt(GM_UNREGISTER);
    msg.SetInt(Process::env.getInt("server_id"));
    sendNetData(sid, (char*)msg.GetData(), msg.GetLength());
}

void PublicServer::serverHeartbeat()
{
    if (!isConnected())
    {
        return;
    }
    
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(0);
    msg.SetInt(GM_HEARTBEAT);
    sendNetData(sid, (char*)msg.GetData(), msg.GetLength());
}

PSMgr SPSMgr;


void PSMgr::addPS(PublicServer* ps)
{
    sididx[ps->sid] = ps;
    psididx[ps->id] = ps;
}

void PSMgr::onConnect(int sid, int psid, string name)
{    
    PublicServer* ps = getServer(sid);
    if (ps) {
        ps->onReconnect();
        return;
    }
    
    if (name == "MailServer") {
        MailServer* ms = new MailServer;
        ms->sid = sid;
        ms->id = psid;
        ms->name = name;
        ms->mMsgProtocol = kMsgProtocolMsgTyped;
        ps = ms;
    } else if(name == "LogServer") {
        LogServer* ls = new LogServer;
        ls->sid = sid;
        ls->id = psid;
        ls->name = name;
        ls->mMsgProtocol = kMsgProtocolMsgTyped;
        ps = ls;
    } else if (name == "GmServer") {
        GmServer* gs = new GmServer;
        gs->sid = sid;
        gs->id = psid;
        gs->name = name;
        gs->mMsgProtocol = kMsgProtocolMsgTyped;
        ps = gs;
    } else if (name == "UniverseServer") {
        UniverseServer* bs = new UniverseServer;
        bs->sid = sid;
        bs->id = psid;
        bs->name = name;
        bs->mMsgProtocol = kMsgProtocolGeneric;
        ps = bs;
    }
    
    if (ps) {
        addPS(ps);
        ps->onConnect();
    }
}

bool PSMgr::sendMessage(int psid, char* data, int len)
{
    map<int, PublicServer*>::iterator iter = psididx.find(psid);
    if (iter == psididx.end()) {
        return false;
    }
    
    if (!iter->second->isRegistered()) {
        return false;
    }
    
    sendNetData(iter->second->sid, data, len);
    return true;
}

bool PSMgr::sendMessage(int psid, CMsgTyped& msg)
{
    map<int, PublicServer*>::iterator iter = psididx.find(psid);
    if (iter == psididx.end()) {
        return false;
    }
    
    if (!iter->second->isRegistered()) {
        return false;
    }
    
    sendNetData(iter->second->sid, (char*)msg.GetData(), msg.GetLength());
    return true;
}

PublicServer * PSMgr::getPublicServer(int psid)
{
	map<int, PublicServer*>::iterator iter = psididx.find(psid);
    if (iter == psididx.end())
    {
        return NULL;
    }
    return iter->second;
}

void PSMgr::traversePublicServer(TraverseCallback cb, void* param)
{
    map<int, PublicServer*>::iterator iter;
    for (iter = psididx.begin(); iter != psididx.end(); iter++) {
        PublicServer* server = iter->second;
        cb(server, param);
    }
}


