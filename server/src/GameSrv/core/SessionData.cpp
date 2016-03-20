//
//  SessionData.cpp
//  GameSrv
//
//  Created by cxy on 13-1-24.
//
//
#include "SessionData.h"
#include "log.h"
#include "Singleton.h"
#include "Game.h"
#include "cmd_def.h"
#include "gate.h"

SessionDataMgr SSessionDataMgr;
extern void sendMessageToGate(int dest, int type, void* data, int size);

string SessionDataMgr::getProp(int sid, const string& key)
{
    map<int, SessionData*>::iterator iter = dataset.find(sid);
    if (iter == dataset.end())
    {
        return "";
    }
    
    return iter->second->properties.getProperty(key);
}

void SessionDataMgr::setProp(int sid, const string& key, const string& val)
{
    map<int, SessionData*>::iterator iter = dataset.find(sid);
    if (iter != dataset.end())
    {
        iter->second->properties.setProperty(key, val);
    }
}

void SessionDataMgr::newData(int sid)
{
    map<int, SessionData*>::iterator iter = dataset.find(sid);
    if (iter != dataset.end())
    {
        delete iter->second;
    }
    SessionData* data = new SessionData;
    data->lasttime = Game::tick;
    data->state = kSessionConnect;
    dataset[sid] = data;
}

void SessionDataMgr::delData(int sid)
{
    map<int, SessionData*>::iterator iter = dataset.find(sid);
    if (iter != dataset.end())
    {
        delete iter->second;
        dataset.erase(iter);
    }
}

int SessionDataMgr::getState(int sid)
{
    map<int, SessionData*>::iterator iter = dataset.find(sid);
    if (iter != dataset.end())
    {
        return iter->second->state;
    }
    
    return kSessionStateUnknown;
}

void SessionDataMgr::setState(int sid, int state)
{
    map<int, SessionData*>::iterator iter = dataset.find(sid);
    if (iter != dataset.end())
    {
        iter->second->state = state;
    }
}

void SessionDataMgr::update(){
    std::map<int, SessionData*>::iterator iter = dataset.begin();
    while (iter != dataset.end()){
        bool shouldkick = false;
        
        SessionData* data = iter->second;
        //if (data->state == kSessionConnect){
            if (data->lasttime + 300 < Game::tick){
                shouldkick = true;
            }
        //}
        
        if (shouldkick){
            create_cmd(kickSession, kickSession);
            kickSession->sid = iter->first;
            sendMessageToGate(0, CMD_MSG, kickSession, 0);
            //sendCmdMsg(Gate::MQ, kickSession);
        }
        
        ++iter;
    }
}

SessionData* SessionDataMgr::getData(int sid)
{
    SessionData* data = NULL;
    map<int, SessionData*>::iterator iter = dataset.find(sid);
    if (iter != dataset.end()) {
        data = iter->second;
    }
    
    return data;
}
