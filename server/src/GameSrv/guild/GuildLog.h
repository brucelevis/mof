#pragma once
#include "RedisHashJson.h"
#include "RedisKeyName.h"
#include "GuildShare.h"
using namespace std;

class GuildLog
{
    typedef RedisHashJson<int> Logs;
public:
    void load(int guildid);
    void update();
    void destroy();
    
    void append(const char* log,int level = eGuildLogLvl_D);
    void send(int roleid,int start ,int num);
protected:
    void expire();
    int nextId();
protected:
    Logs        mLogs;
};


