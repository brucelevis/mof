#ifndef BAN_ROLE_H
#define BAN_ROLE_H

#include "GMTask.h"
#include "json/json.h"
#include <stdio.h>


class BanRole : public GmTask
{
public:
    BanRole() {}
    ~BanRole() {}


    static string formatId(int serverid, int roleid)
    {
        char buf[64];
        snprintf(buf, 32, "%d:banrole:%d", serverid, roleid);
        return buf;
    }

    virtual string getId();
    virtual bool create(int serverid, Json::Value& field);
    virtual bool update(int ts);
    bool over();

    virtual void insertDB();
    virtual void updateDB();
    virtual void deleteDB();

    int mServerId;
    int mRoleId;
    int mStartTime;
    int mEndTime;
    int mState;
    int mDelpvprank;
};

#endif
