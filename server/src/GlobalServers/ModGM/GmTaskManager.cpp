#include "GmTaskManager.h"

#include "Notify.h"
#include "BanRole.h"
#include "BanChat.h"
#include "async_func.h"

GmTaskManager g_GmTaskManager;

GmTaskManager::GmTaskManager()
{
    state = INIT;
}

GmTaskManager::~GmTaskManager()
{
    map<string, GmTask*>::iterator iter = mGmTasks.begin();
    while (iter != mGmTasks.end())
    {
        GmTask* gmTask = iter->second;
        delete gmTask;
    }
}

void GmTaskManager::loadGmTasks()
{
    int processorId = g_pGame->FindProcessor(1, eGmServer);
    g_pGame->Call(processorId, AsyncProp_LoadGmTasks, NULL);
    state = LOADING;
}


void GmTaskManager::dumpGmTask()
{
    Log(LOG_NORMAL, "************************************************");
    map<string, GmTask*>::iterator iter = mGmTasks.begin();
    while (iter != mGmTasks.end())
    {
        GmTask* gmTask = iter->second;
        Log(LOG_NORMAL, "gm task %s", gmTask->getId().c_str());
        iter++;
    }
    Log(LOG_NORMAL, "************************************************");
}

void GmTaskManager::loadGmTaskComplete()
{
    dumpGmTask();
    state = READY;
}

void GmTaskManager::addGmTask(GmTask* gmtask)
{
    map<string, GmTask*>::iterator iter = mGmTasks.find(gmtask->getId());
    if (iter != mGmTasks.end())
    {
        iter->second->deleteDB();
        delete iter->second;
    }
    mGmTasks[gmtask->getId()] = gmtask;
}

void GmTaskManager::delGmTask(string taskId)
{
    map<string, GmTask*>::iterator iter = mGmTasks.find(taskId);
    if (iter != mGmTasks.end())
    {
        delete iter->second;
        mGmTasks.erase(iter);
    }
}

void GmTaskManager::sendGmTaskCmd(int serverid, const char* cmdstr)
{
    CMsgTyped msg;
    msg.SetInt(NEW_GM_TASK);
    msg.SetInt(serverid);
    msg.SetString(cmdstr);
    SendMessage(&msg);
}

void GmTaskManager::addNewGmTask(GmTask* gmtask)
{
    addGmTask(gmtask);
    gmtask->insertDB();

    dumpGmTask();
}


void GmTaskManager::cancelGmTask(string taskId)
{
    map<string, GmTask*>::iterator iter = mGmTasks.find(taskId);
    if (iter != mGmTasks.end())
    {
        iter->second->deleteDB();
        delete iter->second;
        mGmTasks.erase(iter);
    }

    dumpGmTask();
}

void GmTaskManager::createGmTask(int serverid, const char* cmdstr)
{
    Json::Value value;
    Json::Reader reader;
    if (!reader.parse(cmdstr, value))
    {
        return;
    }

    try
    {
        string type = value["type"].asString();
        Json::Value fields = value["field"];

        GmTask* task = NULL;
        if (type == "banrole")
        {
            task = (GmTask*)new BanRole;
        }
        else if (type == "banchat")
        {
            task = (GmTask*)new BanChat;
        }
        else if (type == "allowchat")
        {
            int roleid = fields["roleid"].asInt();
            string idstr = BanChat::formatId(serverid, roleid);
            cancelGmTask(idstr);

            CMsgTyped msg;
            msg.SetInt(0);
            msg.SetInt(0);
            msg.SetInt(GM_ALLOW_CHAT);
            msg.SetInt(roleid);
            g_pGame->SendMessage(serverid, (int)eGmServer, &msg);
        }
        else if (type == "addnotify")
        {
            task = (GmTask*)new Notify;
        }
        else if (type == "allowrole")
        {
            int roleid = fields["roleid"].asInt();
            string idstr = BanRole::formatId(serverid, roleid);
            cancelGmTask(idstr);

            CMsgTyped msg;
            msg.SetInt(0);
            msg.SetInt(0);
            msg.SetInt(GM_ALLOW_ROLE);
            msg.SetInt(roleid);
            g_pGame->SendMessage(serverid, (int)eGmServer, &msg);
        }
        else if (type == "delnotify")
        {
            int notifyid = fields["notifyid"].asInt();
            string idstr = Notify::formatId(serverid, notifyid);
            cancelGmTask(idstr);
        }
        else if (type == "roleprop")
        {
            int roleid = fields["roleids"].asInt();
            string propname = fields["name"].asString();
            string propvalue = fields["value"].asString();

            CMsgTyped msg;
            msg.SetInt(0);
            msg.SetInt(0);
            msg.SetInt(GM_SET_ROLE_PROP);
            msg.SetInt(roleid);
            msg.SetString(propname.c_str());
            msg.SetString(propvalue.c_str());
            g_pGame->SendMessage(serverid, (int)eGmServer, &msg);
        }
        else if (type == "appcmd")
        {
            CMsgTyped msg;
            msg.SetInt(0);
            msg.SetInt(0);
            msg.SetInt(GM_APP_CMD);
            msg.SetString(Json::FastWriter().write(fields).c_str());
            g_pGame->SendMessage(serverid, (int)eGmServer, &msg);
        }

        if (task && task->create(serverid, fields))
        {
            addNewGmTask(task);
        }
    }
    catch(...)
    {

    }
}

void GmTaskManager::update()
{
    if (state != READY)
    {
        return;
    }

    while (true)
    {
        CMsgTyped *pMsg = GetMessage();
        if (!pMsg)
        {
            break;
        }

        int type = pMsg->IntVal();
        switch (type)
        {
            case NEW_GM_TASK:
            {
                int serverid = pMsg->IntVal();
                string cmdstr = pMsg->StringVal();
                createGmTask(serverid, cmdstr.c_str());
            }
        }

        delete pMsg;
    }

    list<string> deltasks;
    map<string, GmTask*>::iterator iter = mGmTasks.begin();
    while (iter != mGmTasks.end())
    {
        GmTask* gmTask = iter->second;
        if (!gmTask->update(TimeTick::cur_time))
        {
            deltasks.push_back(gmTask->getId());
        }
        iter++;
    }

    for (list<string>::iterator iter = deltasks.begin(); iter != deltasks.end(); iter++)
    {
        cancelGmTask(*iter);
    }
}
