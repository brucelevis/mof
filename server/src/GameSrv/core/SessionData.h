//
//  SessionData.h
//  GameSrv
//
//  Created by cxy on 13-1-22.
//
//

#ifndef __GameSrv__SessionData__
#define __GameSrv__SessionData__

#include <string>
#include <map>
#include <list>
#include "process.h"
using namespace std;


enum SessionState{
    kSessionStateUnknown = -1,
    kSessionConnect,
    kSessionAuthPending,
    kSessionAuth,
    kSessionInGame,
};

class SessionData
{
public:
    int sid;
    int state;
    int lasttime;
    Properties properties;
};

class SessionDataMgr
{
public:
    string getProp(int sid, const string& key);
    void setProp(int sid, const string& key, const string& val);
    SessionData* getData(int sid);
    
    void newData(int sid);
    void delData(int sid);
    
    int  getState(int sid);
    void setState(int sid, int state);
    
    void update();
    
private:
    std::map<int, SessionData*> dataset;
};

extern SessionDataMgr SSessionDataMgr;

#endif /* defined(__GameSrv__SessionData__) */
