//
//  gate_mod.h
//  GameSrv
//
//  Created by prcv on 14-1-3.
//
//

#ifndef __GameSrv__gate_mod__
#define __GameSrv__gate_mod__

#include <iostream>

#include "thread.h"
#include "gate.h"
#include "json/json.h"
#include "MQ.h"
#include "server.h"

class ICmdMsg;
class Gate;

typedef IoEventReactor* (*IoReactorCreator)(Gate* gateMod, const Json::Value& value);

class GateMod : public IServer
{
public:
    void registerIoReactor(const char* name, IoReactorCreator creator);
    IoEventReactor* createReactor(const char* name, const Json::Value& value);
    
private:
    map<string, IoReactorCreator> mIoReactorCreators;
    
public:
    GateMod(ServerApp* app) : IServer(app)
    {
        MQ = mApp->getMQ(getId());
    }
    ~GateMod();
    
    bool init();
//    void start();
//    void stop();
    
    //virtual void run();
    
    Gate* getGate()
    {
        return mGate;
    }
    
    virtual void onMessage(skynet_message* msg);
    
    virtual void onCommand(char* data);
    virtual void onTimer(int timerId);
    virtual void onNetMessage(int sid, char* data, int len);
    virtual void onMulticastMessage(char* data, int len);
    virtual void onBroadcastMessage(int groupId, char* data, int len);
    
public:
    static message_queue* MQ;
    
private:
    Gate* mGate;
    Thread mThread;
    bool mRunning;
    
    vector<IoEventReactor*> mReactors;
    
    bool createConnector(const Json::Value& config);
    bool createListener(const Json::Value& config);
};

#endif /* defined(__GameSrv__gate_mod__) */
