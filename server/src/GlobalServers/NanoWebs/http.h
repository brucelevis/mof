
#pragma once

#include "Utils.h"

class HttpRequest;
typedef void (*HTTP_REQ_FUN)(PersistID id, string& cmd);

//表头
struct TableTilte
{
public:
    TableTilte() { Reset(); }
    void Reset()
    {
        title = "Server Information";
        key = "Description";
        val = "Value";
    }
    string title;
    string key;
    string val;
};

// 数据管道
class HttpRequest
{
public:
    HttpRequest();
    virtual ~HttpRequest();

public:
    void ReceiverBytes(char* pData, size_t nBytes);

private:
    void OnReceiveCmd(string& cmd);
    void OnReceiveCmd();
    bool Readline();
    void FlushTable();

public:
    PersistID m_ConnId;
    string m_szIP;

protected:
    string m_HttpLine;
    vector<string> m_Headers;
};

class HttpResponse
{
public:
    HttpResponse(){};
    ~HttpResponse(){};

    void Write(const char* str);
    void WriteLine(const char* str);
    void SetTableTitle(const char* title, const char* key, const char* val);
    void AddTableItem(const char* key, const char* val);
    void Flush(string& http);

private:
    void FlushTable(string& http);

public:
    // 普通页面
    string m_HttpRespon;
    PersistID m_ConnId;

    // 表
    TableTilte m_tabTitle;
    vector< std::pair<string, string> > m_tabItems;
};

// 回应管理器
class HttpResponseManager : public CriticalObject
{
public:
    HttpResponseManager();
    ~HttpResponseManager();
public:
    // 写入数据
    void Write(PersistID id, const char* str);
    void WriteLine(PersistID id, const char* str);
    void SetTableTitle(PersistID id, const char* title, const char* name, const char* val);
    void AddTableItem(PersistID id, const char* name, const char* val);
    void Flush(PersistID id);

public:
    HttpResponse* GetResponse(PersistID id);
    void DeleteResonse(PersistID id);
    void CleanAll();

private:
    std::map<PersistID, HttpResponse*> m_Responses;
public:
    string m_szAppPath;
};

extern HttpResponseManager g_ResonseManager;
extern HTTP_REQ_FUN g_HttpFun;
