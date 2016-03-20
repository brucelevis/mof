#pragma once

#include <stdio.h>
#include <vector>
#include <map>
#include <unistd.h>
#include <string>
#include "thread.h"
#include "MQ.h"
#include "application.h"
using namespace std;

class IServer;

class ServerApp : public Application
{
    class ServerContext
    {
    public:
        message_queue* mMQ;
        IServer* mServer;
        Thread* mThread;
        bool mRunning;
        
        static void* threadProc(void* param);
    };
    
    ServerContext* createServerContext(IServer* server);
    void destroyServerContext(ServerContext* ctx);
    
public:
    ServerApp()
    {
    }
    
    int registerServer(IServer* server);
    void unRegisterServer(IServer* server);
    IServer* getServer(int serverId);
    
    void runBackground();
    void waitTerm();
    
    void send(int serviceId, skynet_message* msg);
    
    message_queue* getMQ(int serverId);
    
public:
    
    virtual void sendNetMessage(int sid, char* data, int len) {throw "not implemented";}
    
private:
    
    vector<ServerContext*> mServers;
};