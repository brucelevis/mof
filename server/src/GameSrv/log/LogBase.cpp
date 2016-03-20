//
//  LogBase.cpp
//  GameSrv
//
//  Created by prcv on 14-6-10.
//
//

#include "LogBase.h"
#include "GameLog.h"


void BaseLog::save()
{
    string now = Utils::currentDate();
    now.append(" ");
    now.append(Utils::currentTime().c_str()) ;
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(1);
    msg.SetInt(GM_LOG);
    msg.SetString(getName());
    msg.SetString(now.c_str());
    specSave();
    sendLogMsg(GameLog::MQ, msg);
}

void BaseLog::addField(const string& val)
{
    mMsg.SetString(val.c_str());
}
void BaseLog::addField(int val)
{
    mMsg.SetString(Utils::itoa(val).c_str());
}
void BaseLog::addField(unsigned int val)
{
    mMsg.SetString(Utils::itoa(val).c_str());
}
void BaseLog::addField(double val)
{
    char buf[64];
    snprintf(buf, 64, "%lf", val);
    mMsg.SetString(buf);
}
void BaseLog::addField(size_t val)
{
    char buf[64];
    snprintf(buf, 64, "%ld", val);
    mMsg.SetString(buf);
}
void BaseLog::addField(time_t val)
{
    char buf[64];
    snprintf(buf, 64, "%ld", val);
    mMsg.SetString(buf);
}