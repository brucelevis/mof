#include "Notify.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ModGM.h"
#include "async_func.h"

Notify::Notify()
{
}

Notify::~Notify()
{

}

string Notify::getId()
{
    return formatId(mServerId, mNotifyId);
}

bool Notify::create(int serverid, Json::Value& field)
{
    mServerId = serverid;

    try
    {
        mNotifyId = field["notifyid"].asInt();
        mInterval = field["interval"].asInt();
        mTimes = field["times"].asInt();
        mContent = field["content"].asString();

        string starttimestr = field["starttime"].asString();
        string endtimestr = field["endtime"].asString();

        mStartTime = sscanftime((starttimestr.c_str()));
        mEndTime = sscanftime((endtimestr.c_str()));

        mNextTime = mStartTime;
        mExecTimes = 0;

    }
    catch (...)
    {
        return false;
    }

    return true;
}

bool Notify::update(int ts)
{
    if (trigger(ts))
    {
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(0);
        msg.SetInt(GM_NOTIFY);
        msg.SetString(mContent.c_str());
        g_pGame->SendMessage(mServerId, (int)eGmServer, &msg);
    }

    if (next(ts))
    {
        return true;
    }

    deleteDB();

    return false;
}


void Notify::insertDB()
{
    CMsgTyped msg;
    msg.SetInt(mServerId);
    msg.SetInt(mNotifyId);
    msg.SetString(mContent.c_str());
    msg.SetInt(mStartTime);
    msg.SetInt(mEndTime);
    msg.SetInt(mTimes);
    msg.SetInt(mInterval);

    int processorId = g_pGame->FindProcessor(1, eGmServer);
    g_pGame->Call(processorId, AsyncProc_SaveNotify, &msg);
}

void Notify::updateDB()
{

}

void Notify::deleteDB()
{
    CMsgTyped msg;
    msg.SetInt(mServerId);
    msg.SetInt(mNotifyId);
    int processorId = g_pGame->FindProcessor(1, eGmServer);
    g_pGame->Call(processorId, AsyncProc_DeleteNotify, &msg);
}

bool Notify::trigger(int nowtime)
{
    if (mNextTime < nowtime)
    {
        return true;
    }

    return false;
}

bool Notify::next(int nowtime)
{
    while (mNextTime < nowtime)
    {
        mNextTime += mInterval;
    }
    mExecTimes++;
    if (mExecTimes > mTimes && mNextTime > mEndTime)
    {
        return false;
    }

    return true;
}
