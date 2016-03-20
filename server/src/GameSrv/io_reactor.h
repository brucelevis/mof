//
//  io_reactor.h
//  GameSrv
//
//  Created by prcv on 14-1-1.
//
//

#ifndef __GameSrv__io_reactor__
#define __GameSrv__io_reactor__

#include <iostream>
#include "INetPacket.h"
#include "gate.h"
#include "json/json.h"


class ClientIoReactor : public IoEventReactor
{
public:
    void onEvent(int sid, int type, char* data, int len);
    void dispatchClientPacket(int sid, int type, void* data, int len);
    
    Gate* mGate;
    
    static const char* getName();
    static IoEventReactor* creator(Gate* gate, const Json::Value& config);
};

class PublicServerIoReactor : public IoEventReactor
{
public:
    string name;
    int    id;
    
    PublicServerIoReactor(const char* servername, int serverid);
    void onEvent(int sid, int type, char* data, int len);
    
    static const char* getName();
    static IoEventReactor* creator(Gate* gate, const Json::Value& config);
};

#endif /* defined(__GameSrv__io_reactor__) */
