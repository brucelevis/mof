#include "GuildLog.h"
#include "Guild.h"
#include "main.h"
#include "Role.h"


////////////////////////////////////////////////////////////////////////////////
#define LOG_CONTENT "log"
#define LOG_LEVEL "lvl"


void GuildLog::load(int guildid)
{
    mLogs.load( g_GetRedisName(rnGuildLog), guildid);
}

void GuildLog::expire()
{
    int deltime = time(0) - GUILD_LOG_EXPIRED_SECOND;

    vector<int> ids;
    for ( Logs::iterator i = mLogs.begin(); i != mLogs.end(); ++i)
    {
        if( i->first < deltime )
            ids.push_back(i->first);
        else
            break;
    }

    for (int i = 0; i < ids.size(); ++i)
    {
        mLogs.erase(ids[i]);
    }
}

void GuildLog::destroy()
{
    mLogs.clear();
}

void GuildLog::append(const char* log,int level)
{
    int id = nextId();
    mLogs[id][LOG_CONTENT] = log;
    mLogs[id][LOG_LEVEL] = level;
}

void GuildLog::send(int roleid,int start ,int num)
{
    std::pair<int,int> range = checkPageRange(mLogs.size(), start, num);

    ack_guild_log ack;
    ack.start = range.first;
    obj_guild_log_info obj;


    Logs::reverse_iterator iter = mLogs.rbegin() ;
    for (int i = 0; i<range.first; ++i)
    {
        ++iter;
    }

    vector<int> delOldLog;

    for (int i = 0; i < range.second - range.first; ++i , ++iter)
    {
        obj.date = iter->first;

        try{

            if( iter->second.type() !=  Json::objectValue)
                throw 0;
            
            obj.log = iter->second[LOG_CONTENT].asString();
            obj.level = iter->second[LOG_LEVEL].asInt();
            ack.logs.push_back(obj);

        }catch(...){
            delOldLog.push_back(iter->first);
            continue;
        }

    }

    sendRolePacket(roleid,&ack);

    for (int i = 0; i< delOldLog.size(); ++i)
    {
        mLogs.erase( delOldLog[i] );
    }
}

int GuildLog::nextId()
{
    int id = time(0);

    for (;;)
    {
        if( ! mLogs.exist(id))
            return id;
        ++id;
    }
}

void GuildLog::update()
{
    expire();

    mLogs.save();
}




