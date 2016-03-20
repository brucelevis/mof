#include "NetPacket.h"


std::map<int, NetPacketFun>& get_Fun_CreateNetPacket()
{
    static std::map<int, NetPacketFun> Fun_CreateNetPacket;
    return Fun_CreateNetPacket;
}

std::map<int, NetPacketHander>& get_Fun_HanderNetPacket()
{
    static std::map<int, NetPacketHander> Fun_HanderNetPacket;
    return Fun_HanderNetPacket;
}

INetPacket* create_recvpacket(int type,int id)
{
    int index = type<<16|id;
    std::map<int, NetPacketFun>::iterator fit = get_Fun_CreateNetPacket().find(index);
    if (fit != get_Fun_CreateNetPacket().end()) {
        return (fit->second)();
    }
    return NULL;
}

NetPacketHander hander_recvpacketfun(int type,int id)
{
    int index = type<<16|id;
    std::map<int, NetPacketHander>::iterator fit = get_Fun_HanderNetPacket().find(index);
    if (fit != get_Fun_HanderNetPacket().end()) {
        return fit->second;
    }
    
    return NULL;
}

int
registerHandlerByName(const char* packName, const char* funcName) {
    
    
    return 1;
}

void regit_create(int index, NetPacketFun fun)
{
    assert(get_Fun_CreateNetPacket().find(index)==get_Fun_CreateNetPacket().end());
    get_Fun_CreateNetPacket().insert(std::make_pair(index, fun));
}

void regit_msg(int index,NetPacketHander fun)
{
    assert(get_Fun_HanderNetPacket().find(index)==get_Fun_HanderNetPacket().end());
    get_Fun_HanderNetPacket().insert(std::make_pair(index, fun));
}