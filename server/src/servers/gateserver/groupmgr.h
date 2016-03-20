//
//  mod_session.h
//  GameSrv
//
//  Created by cxy on 13-1-15.
//
//

#ifndef __GameSrv__mod_session__
#define __GameSrv__mod_session__

#include <iostream>
#include <map>
#include "INetPacket.h"
#include "Singleton.h"
#include <list>
#include <set>
using namespace std;

#define WORLD_GROUP 0

class GroupMgr
{
public:
    GroupMgr();
    ~GroupMgr();
    
    bool addGroup(int groupid);
    bool delGroup(int groupid);
    void enterGroup(int groupid, int sid);
    void leaveGroup(int groupid, int sid); 
    
    set<int>* getGroupMemeber(int groupid);
private:

    static const int MAX_SESSION_NUM = 8192;

    map<int, set<int>* > groups;
    std::list<int> freesessions;
    int _sessions[MAX_SESSION_NUM];

    unsigned long _gen_session;
};

#define SGroupMgr Singleton<GroupMgr>::Instance()

#endif /* defined(__GameSrv__mod_session__) */
