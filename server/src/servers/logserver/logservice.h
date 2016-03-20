#ifndef LOGSERVER_LOGSERVICE_H
#define LOGSERVER_LOGSERVICE_H

#include "MQ.h"
#include "thread.h"
#include "LogFiles.h"
#include "timestamp.h"
#include "server.h"

class ServerApp;

class GameLog: public Runnable
{
public:
    GameLog(void):_stop(false)
    {
        
    }
    virtual ~GameLog(void){}
    void run();
    int init();
    void update(float dt);
    void stop(){_stop = true;}
    static message_queue *MQ;
    static LogFileMgr *mLogfile;
private:
    Timestamp _ts;
    bool _stop;
};

class LogService : public IServer
{
public:
    
    LogService(ServerApp* app) : IServer(app)
    {
    }
    
    bool init();
    
    virtual void onMessage(skynet_message* msg);

    void LogMsghandler(skynet_message& m);

private:
    LogFileMgr *mLogfile;
};

#endif