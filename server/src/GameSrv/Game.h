//
//  Game.h
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#ifndef __GameSrv__Game__
#define __GameSrv__Game__

#include "Defines.h"
#include "DataCfg.h"
#include <string>
#include "timestamp.h"
#include "thread.h"
//#include "Global.h"
#include "db_mgr.h"


#define MAX_COMM_SCENE 1

using namespace std;

#define FPS 20
#define FRAME_TIME_MILLSEC 50
#define FRAME_TIME_SEC 0.05

struct message_queue;
class LuaState;
class Scene; class Role;
class  Game : public ::Runnable
{
public:
    Game(void):_stop(false)
    {
        mBeat = 0;
    }
    virtual ~Game(void){}
    
    virtual bool beforeRun();
    virtual void afterRun();
    
    void run(){
        while (!_stop)
        {
            int remain=FRAME_TIME_MILLSEC - _ts.elapsed();
            if(remain > 0){
                ::Thread::sleep(remain);
                update(FRAME_TIME_SEC);
            }else{
                update((-remain+FRAME_TIME_MILLSEC)/1000);
            }

            _ts.update();
        }
    }
    void stop(){_stop=true;}
    
    bool init();
    int cleanup(){return 1;}
    
    void update(float dt);
    READWRITE(int, mBeat, Beat)
    
    static time_t tick;
    static message_queue *MQ;
    
    //static LuaState* sLuaState;
    
private:
    bool _stop;
    ::Timestamp _ts;
    int mGlobalfreshtime;
    
    Thread mThread;
};

//****************************  各种活动相关的函数 *****************************//

void allActiveInit();               //所有活动的初始化放这里


#endif /* defined(__GameSrv__Game__) */
