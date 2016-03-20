// GamesScene.cpp : 定义 DLL 应用程序的入口点。
//

#include "main.h"
#include "http.h"
#include "tools.h"

extern void BeginServer(unsigned short port);
extern void ShutDownServer();
extern void CloseConn(PersistID id, int secs);

extern "C"
{
    void Open(unsigned short port)
    {
        // 开启
        BeginServer(port);
    }

    void Close()
    {
        // 关闭
        ShutDownServer();

        // 清理所有数据
        g_ResonseManager.CleanAll();
    }

    void SetCallBack(HTTP_REQ_FUN fun)
    {
        g_HttpFun = fun;
    }

    // 以下函数线程安全， 在任何线程皆可调用
    // 写入数据
    void Write(PersistID id, const char* str)
    {
        g_ResonseManager.Write(id, str);
    }

    // 写入行
    void WriteLine(PersistID id, const char* str)
    {
        g_ResonseManager.WriteLine(id, str);
    }

    // 设置table title 和 列名
    void SetTableTitle(PersistID id, const char* title, const char* name, const char* val)
    {
        g_ResonseManager.SetTableTitle(id, title, name, val);
    }

    // 添加table行
    void AddTableItem(PersistID id, const char* name, const char* val)
    {
        g_ResonseManager.AddTableItem(id, name, val);
    }

    // 发送HTTP 回应
    void Flush(PersistID id)
    {
        g_ResonseManager.Flush(id);
    }

    // 发送文件
    void SendFile(PersistID id, const char* path)
    {
        SyncSendFile(id, path);
    }

    // 发送HTTP 回应
    void Disconnect(PersistID id, int secs)
    {
        CloseConn(id, secs);
    }

    // 获取ip
    const char* GetClientIP(PersistID id)
    {
        return WShellAPI::GetAddr(id);
    }
}



