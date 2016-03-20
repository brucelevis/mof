#pragma once

#include "INetPacket.h"
#include "gate.h"
#include "json/json.h"

class CenterClientIoReactor : public IoEventReactor
{
public:

    void onEvent(int sid, int type, char* data, int len);
    static const char* getName();
    static IoEventReactor* creator(Gate* gate, const Json::Value& config);

private:
	Gate* mGate;
};