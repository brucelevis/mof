#include "BanChat.h"
#include <stdio.h>

#include "ModGM.h"
#include "async_func.h"

string BanChat::getId()
{
    return formatId(mServerId, mRoleId);
}

bool BanChat::create(int serverid, Json::Value& field)
{
    mServerId = serverid;
    try
    {
        mRoleId = field["roleid"].asInt();
        string starttimestr = field["starttime"].asString();
        string endtimestr = field["endtime"].asString();
        mStartTime = sscanftime((starttimestr.c_str()));
        mEndTime = sscanftime((endtimestr.c_str()));

        mState = 0;
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool BanChat::update(int ts)
{
    if (mState == 0)
    {
        if (ts > mStartTime)
        {
            CMsgTyped msg;
            msg.SetInt(0);
            msg.SetInt(0);
            msg.SetInt(GM_BAN_CHAT);
            msg.SetInt(mRoleId);
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
            msg.SetInt(GM_ALLOW_CHAT);
            msg.SetInt(mRoleId);
            g_pGame->SendMessage(mServerId, (int)eGmServer, &msg);

            return false;
        }
    }

    return true;
}

void BanChat::insertDB()
{
    CMsgTyped msg;
    msg.SetInt(mServerId);
    msg.SetInt(mRoleId);
    msg.SetInt(mStartTime);
    msg.SetInt(mEndTime);
    int processorId = g_pGame->FindProcessor(1, eGmServer);
    g_pGame->Call(processorId, AsyncProc_SaveBanChat, &msg);
}

void BanChat::updateDB()
{

}

void BanChat::deleteDB()
{
    CMsgTyped msg;
    msg.SetInt(mServerId);
    msg.SetInt(mRoleId);
    int processorId = g_pGame->FindProcessor(1, eGmServer);
    g_pGame->Call(processorId, AsyncProc_DeleteBanChat, &msg);
}
