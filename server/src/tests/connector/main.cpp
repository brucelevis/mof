#include <cstdio>
#include <iostream>
#include <string>
#include <cmath>
#include <map>

#include "net/gate.h"
#include "mq/MQ.h"
#include "thread/thread.h"

using namespace std;
#define LOG cout



class ConnectorEventReactor : public IoEventReactor
{
public:
    
    int count;
    ConnectorEventReactor(){
        count = 0;
    }
    
    static void* worker(void* param){
        ConnectorEventReactor* reactor = (ConnectorEventReactor*)param;
        
        reactor->mRun = true;
        while (reactor->mRun){
            reactor->count += 1;
            if (reactor->count % 10 == 0){
                usleep(1000);
            }
            
            //usleep(1000);
            if (!reactor->mConnected){
                continue;
            }
            
            char *buf = new char[48];
            int len = 8;
            memcpy(buf, &len, 4);
            memset(buf + 4, 0, 8);
            
            skynet_message msg;
            msg.data = buf;
            msg.session = reactor->mSid;
            msg.source = 0;
            msg.sz = 12;
            skynet_mq_push(Gate::MQ, &msg);
    
        }
        
        return NULL;
    }
    
    void onEvent(int sid, int type, char* data, int len){
        switch (type) {
            case IoEventConnect:
                LOG << "connected with sid: " << sid << endl;
                
                mConnected = true;
                mSid = sid;
                pthread_create(&mThreadId, NULL, worker, this);
                
                break;
            case IoEventRead:
                break;
            
            case IoEventClose:
                mConnected = false;
                
                LOG << "disconnected" << endl;
                
                break;
            default:
                break;
        }
    }
    
private:
    bool      mRun;
    int       mSid;
    pthread_t mThreadId;
    bool      mConnected;
};


const char* host = "192.168.160.101";
uint16_t port = 18010;


int main(int argc, char** argv)
{
    Gate gate;
    Thread thread;
    ConnectorEventReactor reactor;
    for (int i = 0; i < 10; i++)
    gate.create_connector(host, port, &reactor);
    
    //gate.create_connector(host, port, &reactor);
    
    //gate.create_connector(host, port, &reactor);
    
    Gate::MQ = skynet_mq_create(0);
    
    thread.start(&gate);
    
    for (;;){
        usleep(10000000);
    }
    
    gate.stop();
    
    return 0;
}
