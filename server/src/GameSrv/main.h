#ifndef __Main_H__
#define __Main_H__
#include "INetPacket.h"
#include "hiredis.h"
#include "groupmgr.h"
#include "MQ.h"
#include "SessionData.h"
#include "MsgBuf.h"

class ICmdMsg;
class LogMsg;
class IGlobalMsg;

//#define sendNetPacket SSession.sendNetMsg
#define sendRolePacket SRoleMgr.sendPacketToRole

redisContext* get_DbContext();

// 发送网络消息给客户端
void sendNetPacket(int sessionid, INetPacket* packet);
void sendNetData(int sessionid, char* data, int len);
// 发送服务器命令到消息队列
void sendCmdMsg(message_queue* mq, ICmdMsg* msg);

void sendMessageToGame(int dest, int type, void* data, int size);
void sendMessageToGate(int dest, int type, void* data, int size);
void sendMessageToWorld(int dest, int type, void* data, int size);


//发送日志模块消息队列
void sendLogMsg(message_queue* mq, CMsgTyped& msg);
// 发送广播消息，给一个广播组发送消息，目前只支持世界广播，groupid为WORLD_GROUP（0）
void broadcastPacket(int groupid, INetPacket* packet);
// 发送多播消息，给多个客户端发送消息
void multicastPacket(const vector<int>& sessions, INetPacket* packet);

void sendGlobalMsg(message_queue* mq, IGlobalMsg* msg);

#define redisCmd(...) (redisReply*)redisCommand(get_DbContext(),__VA_ARGS__)
//不需要自己释放
#define doRedisCmd(...) freeReplyObject(redisCmd(__VA_ARGS__))

#define MQ_GAME 0
#define MQ_GATE 1
#define MQ_LOGGER 2
#define MQ_PROXY_91 3
#define MQ_PROXY_PPS 4
#define MQ_PROXY_TB 5
#define MQ_XINYOU 6
#define MQ_PROXY_UC 7
#define MQ_PROXY_APPLE 8
#define MQ_PAIHANG 9
#define MQ_DYNAMIC_READ_CFG 10       //动态加载配置
#define MQ_HTTP_REQUEST 11           //HTTP请求
#define MQ_PROXY_360 12          
#define MQ_SDK_PROXY 15
#endif
