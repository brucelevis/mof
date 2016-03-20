#include "LogTableMgr.h"
#include "FileSys/FileSys.h"
#include "Log/LogFile.h"
#include <string.h>
#include <stdio.h>
#include "MessageQueue/MsgBuf.h"
#include "async_func.h"
extern IGlobal* g_pGame;
bool CLogTableMgr::Init()
{
    m_running = false;
    cachenum = 500;
    string apppath;
    if (!W_GetAppPath(apppath))
    {
        //
        return false;
    }
    string configpath = apppath + "/ModLogRes/LogConfig.ini";
    CIniFile2 iniFile(configpath.c_str());
    if (iniFile.LoadFromFile())
    {
        cachenum = iniFile.ReadInteger("Params", "cachenum", 500);
    }

    CMsgTyped log;
    log.SetPointer(this);
    int processID = g_pGame->FindProcessor(1, LogServer);
    g_pGame->Call(processID, AsyncProc_LoadTableStruct, &log);

    return true;
}

bool CLogTableMgr::LoadTables()
{
    map<string, LogCache*>::iterator it;
    for (it = logs.begin(); it != logs.end(); it++)
    {
        delete it->second;
    }
    logs.clear();

    string sAppPath;
    if (!W_GetAppPath(sAppPath))
    {
        //
        return false;
    }
    string folderPath = sAppPath + "/ModLogRes/Tables/";

    logs.clear();

    CompareResult files;
	List_Files(folderPath.c_str(), files);

    for (size_t i = 0; i < files.size(); i++)
    {
        string& filestr = files[i].m_szFileName;
        if (filestr.size() < 5) continue;
        string tail = filestr.substr(filestr.size()-4, 4);
        if (tail != ".ini") continue;

        string szFileName = string(folderPath) + files[i].m_szFileName;
//        AddLogTable(szFileName.c_str());
    }

    Log(LOG_NORMAL, "Load %d tables.\n", logs.size());
    return true;
}

LogCache* CLogTableMgr::GetLogCache(const char* tbName)
{
    std::map<string, LogCache*>::iterator iter = logs.find(tbName);
    if (iter == logs.end())
    {
        return NULL;
    }
    return iter->second;
}

/*

*/
bool CLogTableMgr::AddLogTable(string tableName, LogCache* logcache)
{
    logcache->recordnum = 0;
    logcache->starttime = time(NULL);
//    logcache->sqlstr = "";
    logs.insert(make_pair<string, LogCache*>(tableName, logcache));

    return true;
}

void CLogTableMgr::TraverseLogCache(TraverseFunc func, void* param)
{
    map<string, LogCache*>::iterator it;
    for (it = logs.begin(); it != logs.end(); it++)
    {
        func(it->second, param);
    }
}

bool CLogTableMgr::isRunning()
{
    return m_running;
}
void CLogTableMgr::setRunning()
{
    m_running = true;
}
