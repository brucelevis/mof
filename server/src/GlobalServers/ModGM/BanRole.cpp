#include "BanRole.h"

#include <stdio.h>

#include "ModGM.h"
#include "async_func.h"

string BanRole::getId()
{
    return formatId(mServerId, mRoleId);
}

bool BanRole::create(int serverid, Json::Value& field)
{
    mServerId = serverid;
    try
    {
        mRoleId = field["roleid"].asInt();
        string starttimestr = field["starttime"].asString();
        string endtimestr = field["endtime"].asString();
        mStartTime = sscanftime((starttimestr.c_str()));
        mEndTime = sscanftime((endtimestr.c_str()));

        mDelpvprank = field["delpvprank"].isNull() ? 0 : field["delpvprank"].asInt();

        mState = 0;
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BanRole::update(int ts)
{
    if (mState == 0)
    {
        if (ts > mStartTime)
        {
            CMsgTyped msg;
            msg.SetInt(0);
            msg.SetInt(0);
            msg.SetInt(GM_BAN_ROLE);
            msg.SetInt(mRoleId);
            msg.SetInt(mDelpvprank);
            g_pGame->SendMessage(mServerId, (int)eGmServer, &msg);

            mState = 1;
        }
    }
    else if (mState == 1)
    {
        if (ts > mEndTime)
        {
            CMsgTyped msg;
            msg.SetInt(0);
            msg.SetInt(0);
            msg.SetInt(GM_ALLOW_ROLE);
            msg.SetInt(mRoleId);
            g_pGame->SendMessage(mServerId, (int)eGmServer, &msg);

            return false;
        }
    }

    return true;
}


void BanRole::insertDB()
{
    CMsgTyped msg;
    msg.SetInt(mServerId);
    msg.SetInt(mRoleId);
    msg.SetInt(mStartTime);
    msg.SetInt(mEndTime);
    int processorId = g_pGame->FindProcessor(1, eGmServer);
    g_pGame->Call(processorId, AsyncProc_SaveBanRole, &msg);
}

void BanRole::updateDB()
{

}

void BanRole::deleteDB()
{
    CMsgTyped msg;
    msg.SetInt(mServerId);
    msg.SetInt(mRoleId);
    int processorId = g_pGame->FindProcessor(1, eGmServer);
    g_pGame->Call(processorId, AsyncProc_DeleteBanRole, &msg);
}

