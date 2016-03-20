//
//  DynamicConfig.h
//  GameSrv
//  动态配置，一个独立线程定时读配置文件，然后把最新配置发给主线程
//  改造成为一个定时器
//  Created by Huang Kunchao on 13-7-17.
//
//

#ifndef __GameSrv__DynamicConfig__
#define __GameSrv__DynamicConfig__
#include "thread.h"

const unsigned int FOREVER_EXECUTE = -1;
typedef void* (*TimerSubCb)();                          //子线程回调
typedef void (*TimerMainCb)(void * para);               //主线程回调

void addTimer(TimerMainCb mcb,                          //主线程回调,不能为空
              TimerSubCb scb ,                          //子线程回调
              unsigned int seconds ,                    //定时,秒             
              unsigned int exetimes );                  //执行次数

void remTimer(TimerMainCb mcb);

void startTimer();

#define addTimerDef(mcb,scb, seconds,exetimes) \
    struct tagAddTimer##mcb { \
        tagAddTimer##mcb(){ \
            addTimer( (mcb),(scb),(seconds),(exetimes) );\
        }\
    } tempObjtagAddTimer##mcb ;



struct message_queue;

class DynamicCfg : public Runnable
{
public:
    DynamicCfg():_stop(false){}
    ~DynamicCfg(){}
    void run();
    void stop(){_stop = true;}
    static message_queue *MQ;
protected:
    bool        _stop;
};

void mainThreadRefreshCfg(TimerMainCb mcb, void* param);




#endif /* defined(__GameSrv__DynamicConfig__) */
