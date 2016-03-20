//
//  main.cpp
//  client
//
//  Created by xinyou on 14-5-19.
//
//

#include "main.h"
#include <unistd.h>
#include <deque>
#include <getopt.h>

#include "gate.h"
#include "ByteArray.h"
#include "NetPacket.h"
#include "msg.h"
#include "EasyEncrypt.h"
#include "base_state.h"
#include "lgameclient.h"
#include "clientmgr.h"

#ifdef __cplusplus
extern "C"
{
#endif
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#ifdef __cplusplus
}
#endif

using namespace std;

BaseState* curState;
Gate gate;
typedef void (*OnRecvData)(int sid, char* data, int len);

OnRecvData onRecvData;

void onFirstPacket(int sessionId, char* data, int len);
void onPacket(int sessionId, char* data, int len);
void sendRequest(INetPacket* packet);


class Session
{
public:
    
    typedef void (Session::*OnRecvData)(char* data, int len);
    
    int mState;
    int mSessionId;
    ClientEncrypt mEncryptor;
    
    OnRecvData mOnRecvData;
    
    Session(int sid)
    {
        mState = 0;
        mSessionId = sid;
        mOnRecvData = &Session::onFirstPacket;
    }
    
    virtual void onFirstPacket(char* data, int len)
    {
        int* keyinfo = (int*)data;
        string key = genEncryptKey(keyinfo[0], keyinfo[1], keyinfo[2], keyinfo[3]);
        mEncryptor.Connect(key.c_str());
        mOnRecvData = &Session::onPacket;
        
        GameClient* client = ClientMgr::instance()->getClient(mSessionId);
        if (client) {
            client->login();
        }
    }
    
    virtual void onPacket(char* data, int len)
    {
        
        ByteArray byteArray(data, len);
        int msgType = byteArray.read_int();
        int msgId = byteArray.read_int();
        
        INetPacket* packet = create_recvpacket(msgType, msgId);
        if (packet)
        {
            try
            {
                packet->decode(byteArray);
                NetPacketHander handler = hander_recvpacketfun(msgType, msgId);
                handler(packet, mSessionId);
            }
            catch (...)
            {
                log_error("decode packet failed " << msgType << " " << msgId);
                delete packet;
            }
        }
    }
};

class SessionService
{
public:
    Session* createSession(int sid)
    {
        Session* session = new Session(sid);
        addSession(session);
        return session;
    }
    void addSession(Session* session)
    {
        mSessions[session->mSessionId] = session;
    }
    void remSession(int sid)
    {
        map<int, Session*>::iterator iter = mSessions.find(sid);
        Session* session = iter->second;
        mSessions.erase(iter);
        delete session;
    }
    Session* getSession(int sid)
    {
        map<int, Session*>::iterator iter = mSessions.find(sid);
        if (iter != mSessions.end()) {
            return iter->second;
        }
        return NULL;
    }
    int getSessionCount()
    {
        return mSessions.size();
    }
    
    map<int, Session*> mSessions;
};

SessionService g_SessionService;

class NetReactor : public IoEventReactor
{
public:
    virtual void onEvent(int sid, int type, char* data, int len)
    {
        switch (type) {
            case IoEventConnect:
                if (data == 0)
                {
                    printf("connect fail\n");
                    break;
                }
                
                g_SessionService.createSession(sid);
                
                break;
                
            case IoEventRead:
            {
                Session* session = g_SessionService.getSession(sid);
                if (session) {
                    (session->*(session->mOnRecvData))(data, len);
                }
                break;
            }
            case IoEventClose:
                g_SessionService.remSession(sid);
                if (g_SessionService.getSessionCount() == 0) {
                    exit(0);
                }
                break;
                
            default:
                break;
        }
    }
    
    
};

NetReactor netReactor;

#include "base_state.h"

int main(int argc, char* argv[])
{
    string host = "192.168.160.200";
    int port = 18080;
    int count = 3000;
    
    static struct option long_options[] =
    {
        {"count",    no_argument,       0, 'c'},
        {"host",     no_argument,       0, 'h'},
        {"port",     no_argument,       0, 'p'},
        {"script",     no_argument,       0, 's'},
        {"version",     no_argument,       0, 'v'},
        {0, 0, 0, 0}
    };
    
    for (;;) {
        int optionIndex = 0;
        char opt = getopt_long(argc, argv, "h:p:c:v", long_options, &optionIndex);
        
        if (opt == -1) {
            break;
        }
        
        switch (opt) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'c':
                count = atoi(optarg);
                break;
            case '?':
                break;
            default:
                break;
        }
    }
    
    ClientMgr::globalInit();
    
    for (int i = 0; i < count; i++) {
        int sessionId = gate.create_connector(host.c_str(), port, &netReactor);
        GameClient* client = ClientMgr::instance()->createClient(sessionId);
        if (client) {
            client->mAccount = strFormat("bat-%d", i + 1);
        }
    }
    
    for (;;) {
        gate.processEvents();
    }
    
    return 0;
}

void sendRequest(int sessionId, INetPacket* packet)
{
    ByteArray byteArray;
    packet->build(byteArray);
    
    Session* session = g_SessionService.getSession(sessionId);
    if (session) {
        session->mEncryptor.SendEncrypt(byteArray.m_pContent, byteArray.m_nWrPtr);
    }
    gate.write(sessionId, (char*)&byteArray.m_nWrPtr, 4);
    gate.write(sessionId, byteArray.m_pContent, byteArray.m_nWrPtr);
}