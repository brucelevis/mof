#ifndef BAN_CHAT_H
#define BAN_CHAT_H

#include "GMTask.h"
#include "json/json.h"

#include <stdio.h>


class BanChat : public GmTask
{
public:
    BanChat() {}
    ~BanChat() {}

    static string formatId(int serverid, int roleid)
    {
        char buf[64];
        snprintf(buf, 32, "%d:banchat:%d", serverid, roleid);
        return buf;
    }

    virtual string getId();
    virtual bool create(int serverid, Json::Value& field);
    virtual bool update(int ts);

    virtual void insertDB();
    virtual void updateDB();
    virtual void deleteDB();

    int mServerId;
    int mRoleId;
    int mStartTime;
    int mEndTime;
    int mState;
};
#endif
