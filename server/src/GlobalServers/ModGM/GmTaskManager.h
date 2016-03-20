#ifndef GM_TASK_MANAGER_H
#define GM_TASK_MANAGER_H

#include "GMTask.h"
#include "ModGM.h"
#include "TimeTick.h"

class GmTaskManager : public IHaveSafeQueue
{
public:
    GmTaskManager();
    ~GmTaskManager();
    void loadGmTasks();
    void loadGmTaskComplete();
    void addGmTask(GmTask* gmtask);
    void addNewGmTask(GmTask* gmtask);
    void delGmTask(string taskId);
    void cancelGmTask(string taskId);
    void sendGmTaskCmd(int serverid, const char* cmdstr);
    void createGmTask(int serverid, const char* cmdstr);
    void update();

    void dumpGmTask();
private:
    map<string, GmTask*> mGmTasks;
    int state;
    enum
    {
        INIT,
        LOADING,
        READY,
    };
    enum
    {
        NEW_GM_TASK,

    };
};

extern GmTaskManager g_GmTaskManager;

#endif
