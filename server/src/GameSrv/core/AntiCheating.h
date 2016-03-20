//
//  AntiCheating.h
//  GameSrv
//
//  Created by huangkunchao on 13-12-23.
//
//  防作弊

#pragma once
#include <map>
#include <list>
#include "msg.h"


class Role;

class HeatBeatAnitCheating
{
    struct HeartBeat
    {
        bool willDisconnect; // 是否要踢掉 ，这两个用于延迟5秒关闭客户端
        int disconnectTime; // 踢掉时间
        int mLastBeatTime;
        
        HeartBeat()
        {
            willDisconnect = false;
            disconnectTime = 0;
        }
    };
    
public:
    // 加入同步定时
    void addSyncClientTimer();
    
    void addCheckClientConnection();
    
    void onConnect(Role* role);
    
    // 当客户端被断开后清理操作
    void onDisconnect(int roleid);
    
    // 处理心跳包
    void heartBeat(Role* role, req_client_heart_beat& req);
    
protected:
    // 处理被标记的客户端
    bool handleMarkClient(int currTime , Role* role);
    
    // 标记要断线的客户端
    void markDisconnect(int disconTime , int roleid);
    
protected:
    typedef  std::map<int,HeartBeat> HeartBeatMap;
    HeartBeatMap mHeartbeats;
};

extern HeatBeatAnitCheating g_HeatBeatAnitCheating;
void syncClinetTime(Role* sendto);




