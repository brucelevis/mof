#include "main.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <cmath>
#include <map>
#include <signal.h>
#include <getopt.h>

#include "game_app.h"

using namespace std;

redisContext* get_DbContext()
{
    return g_DBMgr.getDBContext(pthread_self());
}

void sendMessage(message_queue* mq, int session, int type, void* data, int size)
{
    skynet_message m;
    m.sz = (type << 24) + size;
    m.data = data;
    m.session = session;
    m.source = 0;
    skynet_mq_push(mq, &m);
}

void sendMessageToGame(int session, int type, void* data, int size)
{
    skynet_message m;
    m.sz = (type << 24) + size;
    m.data = data;
    m.session = session;
    m.source = 0;
    skynet_mq_push(Game::MQ, &m);
}


void sendMessageToGate(int session, int type, void* data, int size)
{
    skynet_message m;
    m.sz = (type << 24) + size;
    m.data = data;
    m.session = session;
    m.source = 0;
    skynet_mq_push(GateMod::MQ, &m);
}

void sendMessageToWorld(int session, int type, void* data, int size)
{
    skynet_message m;
    m.sz = (type << 24) + size;
    m.data = data;
    m.session = session;
    m.source = 0;
    skynet_mq_push(World::sMQ, &m);
}


void sendNetPacket(int sessionid, INetPacket* packet)
{
    ByteArray bytearray;
    bytearray.write_int(10);
    try {
        packet->build(bytearray);
    } catch (...) {
        return;
    }
    int* plen = (int*)bytearray.m_pContent;
    *plen = bytearray.m_nWrPtr - 4;

    int size = bytearray.m_nWrPtr;
    char* data = new char[size];
    memcpy(data, bytearray.m_pContent, size);

    sendMessage(GateMod::MQ, sessionid, NET_MSG, data, size);
}

void sendNetData(int sessionid, char* data, int size)
{
    char* buf = new char[size + 4];
    memcpy(buf, &size, 4);
    memcpy(buf + 4, data, size);
    sendMessage(GateMod::MQ, sessionid, NET_MSG, buf, size + 4);
}

void sendCmdMsg(message_queue* mq, ICmdMsg* msg)
{
    skynet_message m;
    m.sz = (CMD_MSG << 24);
    m.data = msg;
    m.session = 0;
    m.source = 0;
    skynet_mq_push(mq, &m);
}

void sendLogMsg(message_queue* mq, CMsgTyped& msg)
{
    char* data = new char[msg.GetLength()];
    memcpy(data, msg.GetData(), msg.GetLength());
    skynet_message smsg;
    smsg.data = data;
    smsg.sz = msg.GetLength();
    skynet_mq_push(GameLog::MQ, &smsg);
}

void sendGlobalMsg(message_queue* mq, IGlobalMsg* msg)
{
    skynet_message m;
    m.sz = (GLOBAL_MSG << 24);
    m.data = msg;
    m.session = 0;
    m.source = 0;
    skynet_mq_push(mq, &m);
}

void broadcastPacket(int groupid, INetPacket* packet)
{
    ByteArray bytearray;
    bytearray.write_int(10);
    try {
        packet->build(bytearray);
    } catch (...) {
        return;
    }
    int* plen = (int*)bytearray.m_pContent;
    *plen = bytearray.m_nWrPtr - 4;

    int size = bytearray.m_nWrPtr;
    char* data = new char[size];
    memcpy(data, bytearray.m_pContent, size);

    sendMessage(GateMod::MQ, groupid, BROADCAST_MSG, data, size);
}


void multicastPacket(const vector<int>& sessions, INetPacket* packet)
{
    int sessionnum = sessions.size();
    if (sessionnum == 0)
    {
        return;
    }

    ByteArray bytearray;
    bytearray.write_int(10);
    try {
        packet->build(bytearray);
    } catch (...) {
        return;
    }
    int* plen = (int*)bytearray.m_pContent;
    *plen = bytearray.m_nWrPtr - 4;

    int intsize = sizeof(int);
    int size = bytearray.m_nWrPtr;
    int alignsize = Utils::alignNum(size, intsize);

    int sessionlen = intsize * (1 + sessionnum);
    int totallen = alignsize + sessionlen;
    char* data = new char[totallen];

    //数据包数据
    memcpy(data, bytearray.m_pContent, size);
    //sessionid数量
    memcpy(data + alignsize, &sessionnum, intsize);
    //所有玩家的sessionid
    memcpy(data + alignsize + intsize, sessions.data(), sessionnum * intsize);
    sendMessage(GateMod::MQ, 0, MULTICAST_MSG, data, size);
}