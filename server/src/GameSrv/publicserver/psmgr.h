
//
//  gamemaster.h
//  GameSrv
//
//  Created by prcv on 13-6-19.
//
//

#ifndef GameSrv_gamemanage_h
#define GameSrv_gamemanage_h

#include <string>
#include <list>
#include <map>
#include "MsgBuf.h"
#include "main.h"
#include "CustomDefine/PublicMessage.h"
#include "basedef.h"
#include "define_helper.h"
using namespace std;

enum MsgProtocolType
{
    kMsgProtocolMsgTyped,
    kMsgProtocolGeneric,
};

class SecTimer
{
public:
    SecTimer() : mTime(0){}
    ~SecTimer() {}
    
    bool passed(time_t dt)
    {
        if (mTime == 0)
        {
            return false;
        }
        
        if (dt > mTime)
        {
            mTime = 0;
            return true;
        }
        
        return false;
    }
    
    void reset(time_t dt)
    {
        mTime = dt;
    }

private:
    time_t mTime;
};

struct PublicServer
{
public:
    int sid;
    string name;
    int id;
    bool isconnected;
    bool mIsRegistered;
    
    int mMsgProtocol;
    
    bool isRegistered() {return mIsRegistered; }
    bool isConnected() {return isconnected;}
    virtual void onResponse(char* data, int len) {}
    virtual void onResponse(CMsgTyped* msg);
    virtual void onResponse(int receiver, int cmdId, CMsgTyped* msg) {}
    virtual void onConnect();
    virtual void onReconnect();
    virtual void onDisconnect();
    virtual void onRegister() {}
    virtual void update(float dt);
    virtual ~PublicServer() {}
    
    void registerServer();
    void unregisterServer();
    void serverHeartbeat();
    
private:
    SecTimer mHeartBeatTimer;
};

class PSMgr
{
public:
    int getSid(int psid)
    {
        map<int, PublicServer*>::iterator iter = psididx.find(psid);
        if (iter != psididx.end())
        {
            return iter->second->sid;
        }
        
        return -1;
    }
    
    PublicServer* getServer(int sid)
    {
        map<int, PublicServer*>::iterator iter = sididx.find(sid);
        if (iter == sididx.end())
        {
            return NULL;
        }
        
        return iter->second;
    }
    
    void addPS(PublicServer* ps);
    
    void delPS(int sid)
    {
        map<int, PublicServer*>::iterator iter = sididx.find(sid);
        if (iter != sididx.end())
        {
            delete iter->second;
            psididx.erase(iter->second->id);
            sididx.erase(iter);
        }
    }
    
    void onResponse(int sid, CMsgTyped* msg)
    {
        PublicServer* server = getServer(sid);
        if (server)
        {
            server->onResponse(msg);
        }
    }
    
    void onResponse(int sid, char* data, int len)
    {
        PublicServer* server = getServer(sid);
        if (server == NULL) {
            return;
        }
        
        if (server->mMsgProtocol == kMsgProtocolMsgTyped) {
            CMsgTyped msg(len, data);
            msg.SeekToBegin();
            server->onResponse(&msg);
        } else {
            //server->onResponse();
        }
        
    }
    
    void onConnect(int sid, int psid, string name);
    
    void onDisconnect(int sid)
    {
        PublicServer* server = getServer(sid);
        if (server)
        {
            server->onDisconnect();
        }
    }
    
    void update(float dt)
    {
        map<int, PublicServer*>::iterator iter = sididx.begin();
        while (iter != sididx.end())
        {
            if (iter->second->isConnected())
            {
                iter->second->update(dt);
            }
            iter++;
        }
    }
    
    bool sendMessage(int psid, char* data, int len);
    bool sendMessage(int psid, CMsgTyped& msg);
	PublicServer * getPublicServer(int psid);
    
    void traversePublicServer(TraverseCallback cb, void* param);
    
private:
    map<int, PublicServer*> sididx;
    map<int, PublicServer*> psididx;
};

extern PSMgr SPSMgr;

#endif
