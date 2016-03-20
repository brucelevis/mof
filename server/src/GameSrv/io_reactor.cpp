//
//  io_reactor.cpp
//  GameSrv
//
//  Created by prcv on 14-1-1.
//
//

#include "io_reactor.h"

#include <iostream>

#include "gate.h"
#include "cmd_def.h"
#include "tcp_connection.h"
#include "BaseType.h"
#include "basedef.h"
#include "main.h"
#include "NetPacket.h"
#include "EasyEncrypt.h"
#include "gate_mod.h"
#include "centerClient.h"

std::map<int, ServerEncrypt> g_ServerEncrypt;

bool decrypt(ServerEncrypt& srv ,char* data, int len)// , char* & buffer , int & outLen)
{
    assert(data && len > 0);
    return srv.RecvDecrypt(data, len);
}


const char* ClientIoReactor::getName()
{
    return "ClientIoReactor";
}

const char* PublicServerIoReactor::getName()
{
    return "PublicServerIoReactor";
}

IoEventReactor* ClientIoReactor::creator(Gate* gate, const Json::Value& config)
{
    ClientIoReactor* reactor = new ClientIoReactor;
    reactor->mGate = gate;
    return reactor;
}

IoEventReactor* PublicServerIoReactor::creator(Gate* gate, const Json::Value& config)
{
    string serverName = config["server_name"].asString();
    int serverId = config["server_id"].asInt();
    PublicServerIoReactor* reactor = new PublicServerIoReactor(serverName.c_str(), serverId);
    return reactor;
}


void ClientIoReactor::dispatchClientPacket(int sid, int type, void* data, int len)
{
    //GameSrv's message
    if (type < 100) {
        sendMessageToGame(sid, NET_MSG, data, len);
    } else if (type < 200) {
        sendMessageToWorld(sid, NET_MSG, data, len);
    } else if (type < 300) {
        //send message to center
        //send header, send data
        mGate->write(CenterClient::instance()->getSession(), (char*)&len, 4);
        memcpy(data, &sid, 4);
        mGate->write(CenterClient::instance()->getSession(), (char*)data, len);
    }
}

void ClientIoReactor::onEvent(int sid, int type, char* data, int len)
{
    switch (type) {
        case IoEventAccept:
        {
            create_cmd(connect, connect);
            connect->sid = sid;
            connect->ip = data;
            sendMessageToGame(0, CMD_MSG, connect, 0);
            
            
            int keyInfo[5];
            keyInfo[0] = 16;
            keyInfo[1] = Process::env.getInt("server_id");
            keyInfo[2] = sid;
            timeval tv;
            gettimeofday(&tv, NULL);
            keyInfo[3] = tv.tv_sec;
            keyInfo[4] = tv.tv_usec;
            
            mGate->write(sid, (char*)keyInfo, sizeof(keyInfo));
            
            g_ServerEncrypt.insert(make_pair(sid, ServerEncrypt() ));
            std::map<int, ServerEncrypt>::iterator iter = g_ServerEncrypt.find(sid);
            iter->second.Connect(genEncryptKey(keyInfo[1], keyInfo[2], keyInfo[3], keyInfo[4]).c_str());
            
            break;
        }
        case IoEventRead:
        {
            // 收到的是密文，先解密
            CheckCondition(data && len > 0, break)
            std::map<int, ServerEncrypt>::iterator it  = g_ServerEncrypt.find(sid);
            CheckCondition( it != g_ServerEncrypt.end(), break)
        
            try{
                decrypt(it->second,data,len);
            } catch(const char* e) {
                cout<< "有非法包 ," << e << endl; // 有非法包
                break;
            } catch(...) {
                break;
            }
            
            CheckCondition(len >= 12, break)
            ByteArray bytearray((char*)data, len);
            (void)bytearray.read_int();
            int type = bytearray.read_int();
            char* dispatchData = new char[len];
            memcpy(dispatchData, data, len);
            dispatchClientPacket(sid, type, dispatchData, len);
     
            break;
        }
        case IoEventClose:
        {
            create_cmd(disconnect, disconnect);
            disconnect->sid = sid;
            sendMessageToGame(0, CMD_MSG, disconnect, 0);
            
            g_ServerEncrypt.erase(sid);
            break;
        }
        default:
            break;
    }
}

PublicServerIoReactor::PublicServerIoReactor(const char* servername, int serverid)
{
    name = servername;
    id = serverid;
}

void PublicServerIoReactor::onEvent(int sid, int type, char* data, int len)
{
    switch (type) {
        case IoEventConnect:
        {
            if (data == NULL)
            {
                log_error("connect to " << name << " failed");
            }
            else
            {
                create_cmd(connectserver, connectserver);
                connectserver->sid = sid;
                connectserver->name = name;
                connectserver->psid = id;
                sendMessageToGame(sid, CMD_MSG, connectserver, 0);
            }
            break;
        }
        case IoEventRead:
        {
            char* buffer = new char[len];
            memcpy(buffer, data, len);
            sendMessageToGame(sid, PS_MSG, buffer, len);
            break;
        }
        case IoEventClose:
        {
            create_cmd(disconnectserver, disconnectserver);
            disconnectserver->sid = sid;
            sendMessageToGame(sid, CMD_MSG, disconnectserver, 0);
            break;
        }
        default:
            break;
    }
}
