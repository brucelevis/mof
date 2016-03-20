//
//  offline_action.cpp
//  GameSrv
//
//  Created by prcv on 13-12-12.
//
//

#include "offline_action.h"

#include "Role.h"
#include "gift_activity.h"
#include "GameLog.h"
#include "target.h"

#include <string>
#include <vector>
using namespace std;


static bool execOfflineAction(Role* role, const char* cmd, vector<string>& params);

void procRoleOfflineAction(Role* role)
{
    do
    {
        RedisResult result(redisCmd("lpop offlineaction:%d", role->getInstID()));
        
        string actionStr = result.readStr();
        if (actionStr.empty())
        {
            break;
        }
        
        Json::Reader reader;
        Json::Value value;
        if (!reader.parse(actionStr, value) || !value.isObject())
        {
            continue;
        }
        
        try
        {
            string cmd = value["cmd"].asString();
            Json::Value paramsValue = value["params"];
            if (!paramsValue.isArray())
            {
                continue;
            }
            
            vector<string> params;
            for (Json::Value::iterator iter = paramsValue.begin(); iter != paramsValue.end(); iter++)
            {
                params.push_back((*iter).asString());
            }
            execOfflineAction(role, cmd.c_str(), params);
        }
        catch(...)
        {
            
        }
    }
    while (true);
                           
}


void addRoleOfflineAction(int roleid, const char* cmd, vector<string>& params)
{
    Json::FastWriter writer;
    Json::Value value, paramsValue;
    
    value["cmd"] = cmd;
    for (int i = 0; i < params.size(); i++)
    {
        paramsValue[i] = params[i];
    }
    value["params"] = paramsValue;
    
    string actionStr = writer.write(value);
    doRedisCmd("lpush offlineaction:%d %s", roleid, actionStr.c_str());
}



typedef void (*OfflineActionHandler)(Role* role, vector<string>& params);
static map<string, OfflineActionHandler> sActionHandlers;

static int register_action(string str, OfflineActionHandler handler)
{
    sActionHandlers[str] = handler;
    return 0;
}

static bool execOfflineAction(Role* role, const char* cmd, vector<string>& params)
{
    assert(role);
    
    do
    {
        map<string, OfflineActionHandler>::iterator iter = sActionHandlers.find(cmd);
        if (iter == sActionHandlers.end()){
            break;
        }
        
        OfflineActionHandler handler = iter->second;
        handler(role, params);
    }
    while (0);
    
    return true;
}

#define OFFLINE_ACTION(action) \
static void offline_##action(Role* role, vector<string>& params);\
int offline_##action##ret = register_action(#action, offline_##action); \
static void offline_##action


OFFLINE_ACTION(AddItems)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    
    ItemArray items;
    StringArray itemStrs = StrSpilt(params[0], ";");
    role->addAwards(itemStrs, items, "offline_action");
}


OFFLINE_ACTION(GiftActivity)(Role* role, vector<string>& params)
{
    if (params.size() < 3)
    {
        return;
    }
    
    int type = getTargetTypeByName(params[0].c_str());
    int param1 = Utils::safe_atoi(params[1].c_str());
    int param2 = Utils::safe_atoi(params[2].c_str());
    g_GiftActivityMgr.check(type, param1, param2, role);
}