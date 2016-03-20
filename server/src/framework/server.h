#pragma once

#include "server_app.h"

class ServerApp;

class IServer
{
public:
    IServer(ServerApp* app) : mApp(app)
    {
        mApp->registerServer(this);
    }
    
    bool init(const char* param) {return true;}
    
    virtual void beforeRun() {}
    virtual void onMessage(skynet_message* msg) = 0;
    virtual void afterRun() {}
    
    int getId() {return mId;}
    void setId(int id) {mId = id;}
    
protected:
    int mId;
    ServerApp* mApp;
};
