#pragma once
#include "ByteArray.h"
#include "BaseType.h"
#include "INetPacket.h"
#include <string>
#include <vector>
#include <map>
#include "assert.h"
#include "log.h"

//创建接受包
INetPacket* create_recvpacket(int type,int id);

typedef INetPacket* (*NetPacketFun)();
typedef void (*NetPacketHander)(INetPacket*,int);

INetPacket* create_recvpacket(int type,int id);
NetPacketHander hander_recvpacketfun(int type,int id);
void regit_create(int index, NetPacketFun fun);
void regit_msg(int index,NetPacketHander fun);

template <typename T>
class creator {
public:
    static INetPacket* create()
    {
        INetPacket* tmp = new T;
        return tmp;
    }
};

#define hander_msg(msg,var) \
    void hander_##msg(INetPacket* impl_##var,int sessionid); \
    struct __reg_##msg{  \
        __reg_##msg(){   \
        regit_create(msg::_proto_type()<<16|msg::_msgid(),creator<msg>::create); \
        regit_msg(msg::_proto_type()<<16|msg::_msgid(),hander_##msg);  \
    }}__tmp_##msg;    \
    void hander_##msg(INetPacket* impl_##var,int sessionid){msg& var = *(msg*)impl_##var;(void)var; \
    //log_info("hand_msg("<<#msg<<")\n");

#ifndef handler_msg
#define handler_msg hander_msg
#endif