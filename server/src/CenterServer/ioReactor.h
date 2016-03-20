#pragma once

#include "gate.h"
#include "json/json.h"

class CenterIoReactor : public IoEventReactor
{
public:
    void onEvent(int sid, int type, char* data, int len);

    void onClientConnect(int sid);
    void onClientDisconnect(int sid);
    void onRecvData(int sid, char* data, int len);


    void onRecvClientData(int sid, int connId, char* data, int len);
    
    static const char* getName();
    static IoEventReactor* creator(Gate* gate, const Json::Value& config);

    Gate* mGate;
};