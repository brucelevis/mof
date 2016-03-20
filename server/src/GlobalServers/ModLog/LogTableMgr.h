#ifndef LOG_TABLE_MGR_H
#define LOG_TABLE_MGR_H


#include <map>
#include <string>
#include "DBTable.h"
using namespace std;

struct LogCache
{
    int recordnum;
    time_t starttime;
    string sqlstr;
    CDBTable table;
};

typedef bool (*TraverseFunc)(LogCache* log, void* param);

class CLogTableMgr
{
private:
    map<string, LogCache*> logs;
    string             m_sDirPath;
    int                cachenum;
    bool m_running;
private:

public:
    bool AddLogTable(string tableName, LogCache* logcache);
    CLogTableMgr(){}
    bool Init();
    bool LoadTables();
    LogCache* GetLogCache(const char* tbName);
    int GetCacheNum() {return cachenum;}
    void TraverseLogCache(TraverseFunc func, void* param);

    bool isRunning();
    void setRunning();
};

#endif
