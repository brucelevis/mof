/*-------------------------------------------------------------------------
	created:	2010/09/16  17:50
	filename: 	e:\Project_SVN\Server\GamesScene\GameScene.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#include "async_func.h"
#include "agentsock.h"
#include "StringUtils/StringUtil.h"
#include <time.h>

IGlobal* g_pGame = NULL;
CLogTableMgr g_tbManager;

string strvFormat(const char* fmt, va_list ap)
{
    char buf[128];
    va_list tryap;
    va_copy(tryap, ap);
    int n = vsnprintf(buf, sizeof(buf), fmt, tryap);
    if (n < sizeof(buf)){
        return string(buf);
    }

    char* data = new char[n + 1];
    vsnprintf(data, n + 1, fmt, ap);

    string retstr = string(data);
    delete [] data;

    return retstr;
}

string strFormat(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    string ret = strvFormat(fmt, args);
    va_end(args);
    return ret;
}

string strFormat(string& str, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    str = strvFormat(fmt, args);
    va_end(args);
    return str;
}

bool InitModule()
{
    g_AgentSock.OpenConnect();
	if (!g_tbManager.Init())
	{
		return false;
	}
	return true;
}

void RegisterCallBack()
{
	REG_PUBLIC_MESSAGE(GM_LOG);
    //REG_PUBLIC_MESSAGE(GM_SYSTEM_LOG);

	//g_pGame->QueryPoolList()
}

void HandleLogCache(LogCache* logcache)
{
    logcache->sqlstr.append(";");

    CMsgTyped log;
    log.SetString(logcache->table.GetTableName().c_str());
    log.SetString(logcache->sqlstr.c_str());
    int processID = g_pGame->FindProcessor(1, LogServer);
    g_pGame->Call(processID, AsyncProc_Plaayer, &log);

    logcache->sqlstr = logcache->table.GetInsertSQL();
    logcache->starttime = time(NULL);
    logcache->recordnum = 0;
}

bool DetectDelayLog(LogCache* logcache, void* param)
{
    time_t now = time(NULL);
    if (now - logcache->starttime > 0)
    {
        if (logcache->recordnum > 0)
        {
            HandleLogCache(logcache);
        }
    }
}

extern "C"
{
    void OnHeartBeat(int nBeat)
    {
        //update table manager every five minutes.
        if (nBeat % 300 == 0)
        {
            Log(LOG_NORMAL, "OnHeartBeat TraverseLogCache.");
            g_tbManager.TraverseLogCache(DetectDelayLog, NULL);
        }
    }
}

void PUB_MSG(GM_LOG)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
    //_Debug("PUB_MSG(GM_LOG)");
    CMsgTyped compassmsg;
    compassmsg = *pmsg;
    compassmsg.SeekToBegin();
    compassmsg.SetAsReadBuf();
    g_AgentSock.SendMessage(&compassmsg, true);


    //Handle the raw message and reconstruct a new message.
    string tableName = pmsg->StringVal();
    LogCache *logcache = g_tbManager.GetLogCache(tableName.c_str());
    if (logcache == NULL)
    {
        _Debug("Can't find table %s.\n", tableName.c_str());
        return ;
    }

    int fieldNum = logcache->table.GetFieldNum();

    string sqlStr = "(";  //(now(), ..., ..)
    string value;
    for (int i = 0; i < fieldNum; i++)
    {
        switch (pmsg->TestType())
        {
        case TYPE_INT:
            {
                int ival = pmsg->IntVal();
                strFormat(value, "%d", ival);
            }
            break;
        case TYPE_FLOAT:
            {
                strFormat(value, "%f", pmsg->FloatVal());
            }
            break;
        case TYPE_BOOL:
            {
                strFormat(value, "%d", (pmsg->BoolValue() ? 1 : 0));
            }
            break;
        case TYPE_STRING:
            {
                string tmpStr = pmsg->StringVal();
                if(tmpStr.empty())
                {
                    strFormat(value, "%s", "NULL");
                }
                else
                {
                    strFormat(value, "'%s'", tmpStr.c_str());
                }
            }
            break;
        case TYPE_WORD:
            {
                string valueStr(WideStrAsUtf8(pmsg->WideStrVal()));
                strFormat(value, "'%s'", valueStr.c_str());
            }
            break;
        case TYPE_NONE:
            {
                strFormat(value, "%s", "NULL");
            }
            break;
        default:
            //unrecognized type
            _Debug("%s has unknown data type.\n", tableName.c_str());
            return;
        }
        if (i != 0)
        {
            sqlStr.append(", ");
        }
        sqlStr.append(value);
    }
    sqlStr.append(")");

    logcache->recordnum++;
    if (logcache->recordnum > 1)
    {
        logcache->sqlstr.append(",");
    }
    logcache->sqlstr.append(sqlStr);
//    HandleLogCache(logcache);

    if (logcache->recordnum >= g_tbManager.GetCacheNum())
    {
        HandleLogCache(logcache);
    }
}


void PUB_MSG(GM_RELOAD)::OnIoMessage(PersistID id, UI32 receiver, UI32 msgId, CMsgTyped* pmsg)
{
	g_tbManager.LoadTables();
}
