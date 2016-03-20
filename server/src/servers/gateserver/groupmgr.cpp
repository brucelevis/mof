//
//  mod_session.cpp
//  GameSrv
//
//  Created by cxy on 13-1-15.
//
//

#include "groupmgr.h"

INSTANTIATE_SINGLETON(GroupMgr)

GroupMgr::GroupMgr()
{
}

GroupMgr::~GroupMgr()
{
    for (map<int, set<int>* >::iterator iter = groups.begin(); iter != groups.end(); iter++)
    {
        delete iter->second;
    }
}


void GroupMgr::enterGroup(int groupid, int sid)
{
    map<int, set<int>* >::iterator iter = groups.find(groupid);
    if (iter == groups.end() || iter->second == NULL)
    {
        iter = groups.insert(make_pair(groupid, new set<int>())).first;
    }
    
    iter->second->insert(sid);
}

void GroupMgr::leaveGroup(int groupid, int sid)
{
    map<int, set<int>* >::iterator iter = groups.find(groupid);
    if (iter == groups.end() || iter->second == NULL)
    {
        return;
    }
    
    set<int>* group = iter->second;
    group->erase(sid);
    if (group->size() == 0)
    {
        delete group;
        groups.erase(iter);
    }
}

set<int>* GroupMgr::getGroupMemeber(int groupid)
{
    map<int, set<int>* >::iterator iter = groups.find(groupid);
    if (iter == groups.end())
    {
        return NULL;
    }
    
    return iter->second;
}
