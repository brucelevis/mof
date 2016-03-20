//
//  AntiCheating.cpp
//  GameSrv
//
//  Created by huangkunchao on 13-12-23.
//
//

#include "AntiCheating.h"
#include "GameLog.h"
#include "Role.h"
#include "DynamicConfig.h"
#include "Game.h"

HeatBeatAnitCheating g_HeatBeatAnitCheating;

void HeatBeatAnitCheating::onDisconnect(int roleid)
{
    mHeartbeats.erase(roleid);
}

void HeatBeatAnitCheating::heartBeat(Role* role, req_client_heart_beat& req)
{
    assert(role);
    
    int current = time(0);
    role->setLastHeartbeatTime(current);
    
    map<int, HeartBeat>::iterator iter = mHeartbeats.find(role->getInstID());
    if (iter == mHeartbeats.end())
    {
        mHeartbeats.insert(make_pair(role->getInstID(), HeartBeat()));
    }
    
    // 已标为kick的
    if( handleMarkClient(current,role) )
        return;
    
    if( mHeartbeats.count(role->getInstID()) == 0)
    {
        mHeartbeats.insert(make_pair(role->getInstID(), HeartBeat()));
    }
    
    if(req.clientTime < current - AntiClientCheatCfg::sFoceClientSyncSecond )
    {
        Xylog log(eLogName_HeartBeatAntiCheating, role->getInstID());
        log<<"sync"<<req.clientTime<<current;
        syncClinetTime(role);
        return;
    }
    
    if(req.clientTime >= current + AntiClientCheatCfg::sAcclerateKickSecond)
    {
        ack_client_cheat ack;
        sendNetPacket(role->getSessionId(), &ack);
        
        markDisconnect(current+2,role->getInstID());
        
        Xylog log(eLogName_HeartBeatAntiCheating, role->getInstID());
        log<<"kick"<<req.clientTime-current << req.clientTime<<current;
    }
}


void HeatBeatAnitCheating::markDisconnect(int disconTime , int roleid)
{
    HeartBeatMap::iterator it = mHeartbeats.find(roleid);
    if( it == mHeartbeats.end() )
        return;
    
    if( it->second.willDisconnect )
        return;
    
    it->second.willDisconnect = true;
    it->second.disconnectTime = disconTime;
}

bool HeatBeatAnitCheating::handleMarkClient( int currTime , Role* role)
{
    HeartBeatMap::iterator it = mHeartbeats.find(role->getInstID());
    if( it == mHeartbeats.end() )
        return false;
    
    HeartBeat& hb = it->second;
    
    if( hb.willDisconnect &&  currTime >=  hb.disconnectTime )
    {
        SRoleMgr.disconnect(role,eClientCheat);
    }
    
    return hb.willDisconnect;
}


// 同步客户端时间
void syncClinetTime(Role* sendto)
{
    syn_server_time pak;
    pak.time = time(0);
    
    if(sendto)
    {
        sendto->send(&pak);
    }else{
        broadcastPacket(WORLD_GROUP, &pak);
    }
}

// 定时同步客户端 
static void HeatBeatAnitCheating_SyncClientTime(void* param)
{
    syncClinetTime(NULL);
}


bool getOfflineRolesCb(void* obj, void* param)
{
    vector<Role*>* roles = (vector<Role*>*)param;
    
    Role* role = (Role*)obj;
    int curTick = Game::tick;
    
    int lastHeartbeatTime = role->getLastHeartbeatTime();
    int interval = curTick - lastHeartbeatTime;
    if (interval > AntiClientCheatCfg::sHeatBeatIntervalTime + AntiClientCheatCfg::sCommunicationTime)
    {
        roles->push_back(role);
    }
    
    return true;
}



static void CheckClientOnline(void* param)
{
    vector<Role*> roles;
    SRoleMgr.traverseRoles(getOfflineRolesCb, &roles);
    
    for (int i = 0; i < roles.size(); i++)
    {
        SRoleMgr.disconnect(roles[i]);
    }
}

void HeatBeatAnitCheating::addSyncClientTimer()
{
    int time = AntiClientCheatCfg::sSyncClientInterval;
    addTimer(HeatBeatAnitCheating_SyncClientTime, NULL, time, FOREVER_EXECUTE);
    
    if (Process::env.getInt("testmode") == 0)
    {
        //int checkInterval = 10;
        //addTimer(CheckClientOnline, NULL, checkInterval, FOREVER_EXECUTE);
    }
}

void HeatBeatAnitCheating::addCheckClientConnection()
{
    
}


