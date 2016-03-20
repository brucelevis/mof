#ifndef NOTIFY_H
#define NOTIFY_H

#include <map>
#include <string>
#include "json/json.h"
#include "GMTask.h"
#include <stdio.h>
using namespace std;

class Notify : public GmTask
{
public:
    Notify();
    ~Notify();

    static string formatId(int serverid, int notifyid)
    {
        char buf[64];
        snprintf(buf, 32, "%d:notify:%d", serverid, notifyid);
        return buf;
    }

    virtual string getId();
    virtual bool create(int serverid, Json::Value& field);
    virtual bool update(int ts);

    virtual void insertDB();
    virtual void updateDB();
    virtual void deleteDB();

    bool trigger(int nowtime);
    bool next(int nowtime);

    int     mServerId;
    int     mNotifyId;
    string  mContent;
    int     mStartTime;
    int     mEndTime;
    int     mTimes;
    int     mInterval;
    int     mNextTime;
    int     mExecTimes;

    friend class NotifyManager;
};


#endif
