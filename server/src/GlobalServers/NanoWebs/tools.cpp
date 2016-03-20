// GamesScene.cpp : 定义 DLL 应用程序的入口点。
//

#include "tools.h"
#include "http.h"
#include <sys/statfs.h>

// 发送文件
void SendFile(PersistID conn, const char* filepath)
{
    if (!filepath) return;
    if (strlen(filepath) < 1) return;

    string fileName = g_ResonseManager.m_szAppPath;

    if (filepath[0] != '/')
    {
        fileName += "/";
    }

    fileName += filepath;

    FILE* file = fopen(fileName.c_str(), "rb");
    if (!file) return;

    fseek(file, 0L, SEEK_END);
    size_t length = ftell(file);
    fseek(file, 0L, SEEK_SET);

    // 生成HTTP 头
    const char* sendfile =
    "HTTP/1.0 200 OK\r\n"
    "Content-Length: %d\r\n"
    "Content-Type: application/octet-stream\r\n"
    "Server: Nano web 1.0\r\n"
    "Connection: keep-alive\r\n\r\n";

    // 不发送超过30MB的文件，保护服务器
    if (length > 1024*1024*30)
    {
        length = 0;
    }

    char buf[1024];
    sprintf(buf, sendfile, length);

    // 发送http头
    WShellAPI::SendData(conn, buf, strlen(buf));

    // 发送文件内容
    while(length > 0)
    {
        size_t read = fread(buf, 1, 1024, file);

        // 发送read
        WShellAPI::SendData(conn, buf, read);

        length -= read;
    }

    fclose(file);
}

// 获取磁盘信息
void DiskInfo(string& out)
{
    out = "";

    long long freespace = 0;
    struct statfs disk_statfs;
    long long totalspace = 0;
    float freeSpacePercent = 0 ;
    if( statfs("/", &disk_statfs) >= 0 )
    {
        freespace = (((long long)disk_statfs.f_bsize  * (long long)disk_statfs.f_bfree)/(long long)(1024*1024) );
        totalspace = (((long long)disk_statfs.f_bsize * (long long)disk_statfs.f_blocks) /(long long)(1024*1024) );
    }

    char buf[32];

    out += "totalspace: ";
    out += itoa(totalspace, buf, 10); out += "mb <br/>";
    out += "freespace: ";
    out += itoa(freespace, buf, 10); out += "mb <br/>";

    freeSpacePercent = 100.0f * (1.0f -  (float)freespace / (float)totalspace );

    out += "used percent: ";

    sprintf(buf, "%.1f%%", freeSpacePercent);
    out += buf; out += "<br/>";
}

// 字符串关键字在其中
bool Inside(StringArray& sa, const char* str)
{
    for (size_t i=0; i < sa.size(); i++)
    {
        if (strstr(str, sa[i].c_str()) == str)
            return true;
    }
    return false;
}

// 读取
void CmdCall(bool isPipe, const char* cmd, const char* sections, string& out)
{
    out = "";

    if (!cmd) return;

    if (sections && strlen(sections) == 0)
    {
        sections = NULL;
    }

    StringArray sa;
    if (sections)
    {
        split_path(sa, sections, ';');
    }

    // buf
    char line[4096];

    // 读取CPU 信息
    FILE* file = isPipe ? popen(cmd, "r") : fopen(cmd, "rb");
    if (file)
    {
        while( fgets(line, sizeof (line), file) )
        {
            if ( !sections || Inside(sa, line) )
            {
                out += line;
                out += "<br/>";
            }
        }

        isPipe ? pclose(file) : fclose(file);
    }
}

void SysInfo(PersistID id)
{
    string str;

    CmdCall(false, "/etc/issue", NULL, str);
    g_ResonseManager.AddTableItem(id, "OS", str.c_str());

    CmdCall(false, "/proc/version", NULL, str);
    g_ResonseManager.AddTableItem(id, "kernel", str.c_str());

    CmdCall(false, "/proc/loadavg", NULL, str);
    g_ResonseManager.AddTableItem(id, "loadavg", str.c_str());

    CmdCall(false, "/proc/cpuinfo", "processor;model name;cpu MHz", str);
    g_ResonseManager.AddTableItem(id, "cpuinfo", str.c_str());

    CmdCall(false, "/proc/meminfo", "MemTotal;MemFree;Buffers;Cached;SwapTotal;SwapFree", str);
    g_ResonseManager.AddTableItem(id, "meminfo", str.c_str());

    CmdCall(false, "/proc/net/dev", NULL, str);
    g_ResonseManager.AddTableItem(id, "net", str.c_str());

    DiskInfo(str);
    g_ResonseManager.AddTableItem(id, "diskinfo", str.c_str());

    CmdCall(true, "ulimit -a", "core;open files", str);
    g_ResonseManager.AddTableItem(id, "uname", str.c_str());

    CmdCall(true, "find /usr/lib -name libmysqlcppconn*", "", str);
    g_ResonseManager.AddTableItem(id, "mysql", str.c_str());

    CmdCall(true, "ps -e | grep S", "", str);
    g_ResonseManager.AddTableItem(id, "Game", str.c_str());

    g_ResonseManager.Flush(id);
}

/////////////////////////////
CSafeQueue g_msgQueue;
#define SYNC_CMD_SENDFILE 1
#define SYNC_CMD_SYS_INFO 2

//异步发送文件
void SyncSendFile(PersistID conn, const char* filepath)
{
    CMsgTyped msg;
    msg.SetInt( SYNC_CMD_SENDFILE );
    msg.SetInt( conn.nIndex );
    msg.SetInt( conn.nIdent );
    msg.SetString(filepath);

    g_msgQueue.PushMessage(&msg);
}

// 异步生成信息
void SyncSysInfo(PersistID conn)
{
    CMsgTyped msg;
    msg.SetInt( SYNC_CMD_SYS_INFO );
    msg.SetInt( conn.nIndex );
    msg.SetInt( conn.nIdent );
    msg.SetString("");

    g_msgQueue.PushMessage(&msg);
}

// 异步处理函数
void SyncProc()
{
    for ( ; ; )
    {
        CMsgTyped * pmsg = g_msgQueue.GetMessage();
        if (!pmsg) return;

        int type = pmsg->IntVal();
        PersistID id;
        id.nIndex = pmsg->IntVal();
        id.nIdent = pmsg->IntVal();
        const char* cmd = pmsg->StringVal();

        if (type == SYNC_CMD_SENDFILE)
        {
            SendFile(id, cmd);
        }
        else if (type == SYNC_CMD_SYS_INFO)
        {
            SysInfo(id);
        }

        delete pmsg;
    }
}

