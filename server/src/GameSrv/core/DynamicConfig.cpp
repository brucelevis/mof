//
//  DynamicConfig.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-7-17.
//
//

#include "DynamicConfig.h"
#include "Game.h"
#include <stdio.h>
#include <vector>

#include "cmd_def.h"
#include <map>
#include "timestamp.h"
#include "GameLog.h"
#include <string>
#include <iostream>

using std::string;
using namespace std;
////////////////////////////////////////////////////////////////////////

message_queue* DynamicCfg::MQ = NULL;

enum enDynamicCfgAction
{
    enDynamicCfgAction_Add = 0,     //add
    enDynamicCfgAction_Rem          //rem
};

struct TimerNode
{
    TimerMainCb             mcb;
    TimerSubCb              scb;             
    unsigned int            seconds;     
    unsigned int            exetimes;
};

struct DynamicCfgMsg
{
    int             action;                 //enDynamicCfgAction
    TimerNode       timernode;
};

void sendDynamicCfgMsg(message_queue* mq, DynamicCfgMsg* msg)
{
    skynet_message m;
    m.sz = (DYNAMIC_CFG_MSG << 24);
    m.data = msg;
    m.session = 0;
    m.source = 0;
    skynet_mq_push(mq, &m);
}


////////////////////////////////////////////////////////////////////////

class SubThreadTimer
{
    
public:
    SubThreadTimer(){}
    
    ~SubThreadTimer()
    {
        for (TimerMap::iterator iter = _timers.begin(); iter != _timers.end(); ++iter) {
            SafeDelete(iter->second);
        }
    }
    
    void add(const TimerNode& node)
    {
        if(NULL == node.mcb || 0 == node.seconds){
            return;
        }
        
        for (TimerMap::iterator iter = _timers.begin(); iter != _timers.end(); ++iter) {
            if(node.mcb == iter->second->mcb){
                return;
            }
        }
        
        TimerNode*  p = new TimerNode;
        if(p == NULL){
            xyerr("Memory exhausted，DynamicConfig::add fail");
            return;
        }
        *p = node;
        
        long timeout = time(NULL) + p->seconds;
        _timers.insert(TimerMap::value_type(timeout,p));
    }
    
    void rem(const TimerNode& node)
    {
        if(node.mcb == NULL){
            return;
        }
        
        for (TimerMap::iterator iter = _timers.begin(); iter != _timers.end(); ) {
            if(node.mcb == iter->second->mcb){
                SafeDelete(iter->second);
                _timers.erase(iter++);
            }else{
                ++iter;
            }
        }
    }

    void handle(DynamicCfgMsg* msg)
    {
        if(NULL == msg)
            return;
        
        if(enDynamicCfgAction_Add == msg->action){
            add(msg->timernode);
        }else{
            rem(msg->timernode);
        }
    }
    
    void update()
    {
        long curtm = time(NULL);
        std::vector<TimerNode*> remvec;
        
        for (TimerMap::iterator iter = _timers.begin(); iter != _timers.end(); ) {
            if(curtm < iter->first)
                break;
            
            TimerNode* p = iter->second;
            
            void* ret = NULL;
            if(p->scb != NULL)
                ret = p->scb();
            
            create_cmd(config_refresh, config_refresh);
            config_refresh->mcbl = (long)p->mcb;
            config_refresh->param = ret;
            sendCmdMsg(Game::MQ, config_refresh);
            
            
            _timers.erase(iter++);
            remvec.push_back(p);
        }
        
        for (int i = 0 , ni = remvec.size(); i < ni; ++i) {
            TimerNode* p = remvec[i];
            
            if(FOREVER_EXECUTE != p->exetimes){
                if(0 == -- p->exetimes){
                    SafeDelete(p);
                    continue;
                }
            }
            
            long timeout = time(NULL) + p->seconds;
            _timers.insert(TimerMap::value_type(timeout,p));
        }
    }

    
protected:
    typedef std::multimap<long, TimerNode*>  TimerMap;
    TimerMap    _timers;        
};

static SubThreadTimer* getSubThreadTimer()
{
    static SubThreadTimer obj;
    return &obj;
}



//////////////////////////////////////////////////////////////////////////////////
//主线程更新回调

class MainThreadCallback
{
public:
    MainThreadCallback(){}
    ~MainThreadCallback(){}
    
    void add(const TimerNode& node)
    {
        if(NULL == node.mcb )
            return;
        
        _maincb.insert(node.mcb);
    }
    
    void rem(const TimerNode& node)
    {
        _maincb.erase(node.mcb);
    }
    
    void handle(TimerMainCb mcb , void * param)
    {
        if( _maincb.count(mcb))
            mcb(param);
    }
    
protected:
    typedef std::set<TimerMainCb> MainRefCB;
    MainRefCB   _maincb;
};

MainThreadCallback* getMainThreadCallback(){
    static MainThreadCallback obj;
    return &obj;
}


////////////////////////////////////////////////////////////////////////////////////
//main 函数未启动前暂存msg
class BeforMainFunctionHelper
{
public:
    BeforMainFunctionHelper(){
        _mainstart = false;
    }
    ~BeforMainFunctionHelper(){
        for (int i = 0; i< _msg.size(); ++i) {
            SafeDelete(_msg[i]);
        }
    }
    
    void setStart(){
        _mainstart = true;
        for (int i = 0; i < _msg.size(); ++i) {
            _postQueue(_msg[i]);
        }
        _msg.clear();
    }
    
    void post(DynamicCfgMsg* msg)
    {
        assert(msg);
        if(! _mainstart){
            _msg.push_back(msg);
            return;
        }
        _postQueue(msg);
    }
protected:
    void _postQueue(DynamicCfgMsg* msg)
    {
        //这个放前面,否则下面内存删了
        if(enDynamicCfgAction_Add == msg->action)
            getMainThreadCallback()->add(msg->timernode);
        else
            getMainThreadCallback()->rem(msg->timernode);
        
        //这个会删msg内存
        sendDynamicCfgMsg(DynamicCfg::MQ,msg);
    }
protected:
    std::vector<DynamicCfgMsg*> _msg;
    bool _mainstart;
};

static BeforMainFunctionHelper* getBeforMainFunctionHelper()
{
    static BeforMainFunctionHelper obj;
    return &obj;
}

////////////////////////////////////////////////////////////////////////////////////


void mainThreadRefreshCfg(TimerMainCb mcb, void* param)
{
    getMainThreadCallback()->handle(mcb, param);
}

void addTimer(TimerMainCb mcb,TimerSubCb scb,unsigned int seconds, unsigned int exetimes)
{    
    if(NULL == mcb || 0 ==  seconds ){
        xyerr("addTimer ERROR , mcb is null ,or second = 0");
        return ;
    }
    
    DynamicCfgMsg* msg = new DynamicCfgMsg;
    if(msg == NULL){
        xyerr("No Memory , addTimer fail");
        return;
    }
    
    msg->action = enDynamicCfgAction_Add;
    msg->timernode.mcb = mcb;
    msg->timernode.scb = scb;
    msg->timernode.seconds = seconds;
    msg->timernode.exetimes = exetimes;
    
    getBeforMainFunctionHelper()->post(msg);
}


void remTimer(TimerMainCb mcb)
{   
    if(NULL == mcb ){
        xyerr("remTimer fail , mcb == null");
        return ;
    }
    
    DynamicCfgMsg* msg = new DynamicCfgMsg;
    if(msg == NULL){
        xyerr("No Memory, remTimer fail");
        return;
    }
    
    msg->action = enDynamicCfgAction_Rem;
    msg->timernode.mcb = mcb;
    
    getBeforMainFunctionHelper()->post(msg);
}

void startTimer()
{
    getBeforMainFunctionHelper()->setStart();
}


void DynamicCfg::run()
{
    while (!_stop) {
        
        skynet_message m;
        while( 0 == skynet_mq_pop(MQ, &m) )
        {
            DynamicCfgMsg* msg = (DynamicCfgMsg*)m.data;
            if (!msg) {
                xyerr("DynamicCfg run ERROR ");
                continue;
            }
            getSubThreadTimer()->handle(msg);
            
            SafeDelete(msg);
        }
        
        try{
            getSubThreadTimer()->update();
        }catch(...){
            printf("ERROR DynamicCfg");
        }
        
        ::Thread::sleep(100);
    }
}




