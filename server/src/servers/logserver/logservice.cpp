#include "logservice.h"
#include "Utils.h"
#include "MsgBuf.h"

#define FRAME_TIME_MILLSEC 50
#define FRAME_TIME_SEC (FRAME_TIME_MILLSEC / 1000)

message_queue* GameLog::MQ = NULL;
LogFileMgr* GameLog::mLogfile = NULL;

int GameLog::init()
{
    mLogfile = new LogFileMgr;
    return mLogfile->pathInit();
}

void GameLog::run()
{
    while (!_stop) {
        int remain = FRAME_TIME_MILLSEC - _ts.elapsed();
        if(remain > 0){
            ::Thread::sleep(remain);
            update(FRAME_TIME_SEC);
        }else{
            update((-remain + FRAME_TIME_MILLSEC)/1000);
        }
        
        _ts.update();
    }
}

void LogMsghandler(skynet_message m)
{
    CMsgTyped msg(m.sz, m.data, false);
    
    (void)msg.IntVal();
    (void)msg.IntVal();
    (void)msg.IntVal();
    string filename = msg.StringVal();
    filename.append(".txt");
    
    (void)msg.StringVal();
    
    std::ostringstream ostr;
    ostr<<Utils::currentTime();
    int type;
    while ( ( type = msg.TestType() ) != TYPE_NONE)
    {
        ostr<<"\t";
        if (type == TYPE_INT)
        {
            ostr<<msg.IntVal();
        }
        else if (type == TYPE_STRING)
        {
            ostr<<msg.StringVal();
        }
        else if (type == TYPE_FLOAT)
        {
            ostr<<msg.FloatVal();
        }
        else if (type == TYPE_BOOL)
        {
            ostr<<msg.BoolValue();
        }
        else break;
    }
    ostr<<"\n";
    
    FILE *fout = GameLog::mLogfile->openFile(filename.c_str());
    fputs(ostr.str().c_str(), fout);
    fflush(fout);
    
}
void GameLog::update(float dt)
{
    skynet_message m;
    while( 0 == skynet_mq_pop(MQ, &m) )
    {
        LogMsghandler(m);
        delete[] (char*)m.data;
    }
}

bool
LogService::init() {
    mLogfile = new LogFileMgr;
    return mLogfile->pathInit() == 0;
}

void 
LogService::LogMsghandler(skynet_message& m) {
    CMsgTyped msg(m.sz, m.data, false);
    
    (void)msg.IntVal();
    (void)msg.IntVal();
    (void)msg.IntVal();
    string filename = msg.StringVal();
    filename.append(".txt");
    
    (void)msg.StringVal();
    
    std::ostringstream ostr;
    ostr<<Utils::currentTime();
    int type;
    while ( ( type = msg.TestType() ) != TYPE_NONE)
    {
        ostr<<"\t";
        if (type == TYPE_INT)
        {
            ostr<<msg.IntVal();
        }
        else if (type == TYPE_STRING)
        {
            ostr<<msg.StringVal();
        }
        else if (type == TYPE_FLOAT)
        {
            ostr<<msg.FloatVal();
        }
        else if (type == TYPE_BOOL)
        {
            ostr<<msg.BoolValue();
        }
        else break;
    }
    ostr<<"\n";
    
    FILE *fout = mLogfile->openFile(filename.c_str());
    fputs(ostr.str().c_str(), fout);
    fflush(fout);
    
}

void
LogService::onMessage(skynet_message *msg) {
    LogMsghandler(*msg);
    delete[] (char*)msg->data;
}

