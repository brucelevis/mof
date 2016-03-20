// GamesScene.cpp : 定义 DLL 应用程序的入口点。
//

#include "http.h"
#include "tools.h"

HTTP_REQ_FUN g_HttpFun = NULL;
HttpResponseManager g_ResonseManager;
void SysInfo(PersistID id);

const char* head =
    "HTTP/1.1 200 OK\r\n"
    "Server: Nano web 1.0 \r\n"
    "Content-Length: %d\r\n"
    "Connection: keep-alive\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n\r\n";

const char* table_head=
"<html><head>"
"<style type=\"text/css\">"
"table {border-collapse: collapse;}"
".center {text-align: center;}"
".center table { margin-left: auto; margin-right: auto; text-align: left;}"
"td, th { border: 1px solid #000000; font-size: 75%; vertical-align: baseline;}"
"h2 {font-size: 100%;}"
".e {background-color: #ccccff; font-weight: bold; color: #000000;}" //ccccff
".h {background-color: #9999cc; font-weight: bold; color: #000000;}" //9999cc
".v {background-color: #cccccc; color: #000000;}"
"</style>"
"</head>"
"<body><div class=\"center\">";

const char* table_title=
"<br />"
"<h2>%s</h2>"
"<table border=\"0\" cellpadding=\"3\" width=\"700\">"
"<tr class=\"h\"><th>%s</th><th>%s</th></tr>"
"<hr />";

const char* table_end =
"</table><br />"
"\r\n"
"</div></body></html>";

///////////////////////////////////////
HttpRequest::HttpRequest()
{
    //ctor
}

HttpRequest::~HttpRequest()
{
    //dtor
}

void HttpRequest::ReceiverBytes(char* pData, size_t nBytes)
{
    for (size_t i=0; i < nBytes; i++)
    {
        m_HttpLine += pData[i];
        if (pData[i] == '\n')
        {
            Readline();
        }
    }
}

void HttpRequest::OnReceiveCmd(string& cmd)
{
    if (cmd == "/favicon.ico")
    {
        //printf("i need a icon!! %s\n", cmd.c_str());
        SyncSendFile(m_ConnId, "favicon.ico");
        return;
    }

    if (cmd == "/sysinfo")
    {
        SyncSysInfo(m_ConnId);
        return;
    }

    if (g_HttpFun)
    {
        g_HttpFun(m_ConnId, cmd);
    }
}

void HttpRequest::OnReceiveCmd()
{
    for (size_t i=0; i < m_Headers.size(); i++)
    {
        string& line = m_Headers[i];

        if (line.find("GET ") == 0)
        {
            StringArray sa;
            split_path(sa, line.c_str(), ' ');

            if (sa.size() >= 2)
            {
                Trim2SideSpace( sa[1] );
                OnReceiveCmd( sa[1] );
            }

            break;
        }
    }
}

bool HttpRequest::Readline()
{
    size_t cnt = m_HttpLine.size();
    if (cnt < 2)
    {
        return false;
    }

    if (m_HttpLine[cnt-2] == '\r' || m_HttpLine[cnt-1] == '\n')
    {
        if (m_HttpLine.size() > 2)
        {
            // 收到一行
            m_Headers.push_back( m_HttpLine );
            m_HttpLine = "";
        }
        else
        {
            // 收到完整的http头
            OnReceiveCmd();
            m_HttpLine = "";
            m_Headers.clear();
        }
        return true;
    }

    return false;
}

////////////////////////////////////////////////////


void HttpResponse::Write(const char* str)
{
    if (m_tabItems.size() > 0) return;

    if (!str) str = "";
    m_HttpRespon += str;
}

void HttpResponse::WriteLine(const char* str)
{
    if (m_tabItems.size() > 0) return;

    if (!str) str = "";

    m_HttpRespon += str;
    m_HttpRespon += "<br/>";
}

void HttpResponse::SetTableTitle(const char* title, const char* key, const char* val)
{
    if (m_HttpRespon.size() > 0) return;

    m_tabTitle.title = title ? title : "";
    m_tabTitle.key = key ? key : "";
    m_tabTitle.val = val ? val : "";
}

void HttpResponse::AddTableItem(const char* key, const char* val)
{
    if (!key) key = "";
    if (!val) val = "";

    if (m_HttpRespon.size() > 0) return;

    m_tabItems.push_back( std::make_pair(key, val) );
}

void HttpResponse::Flush(string& http)
{
    if (m_HttpRespon.size() > 0)
    {
        char httphead[512];

        sprintf(httphead, head, m_HttpRespon.size());

        http = httphead;
        http += m_HttpRespon;

        m_HttpRespon = "";
    }
    else if (m_tabItems.size() > 0)
    {
        FlushTable(http);
        m_tabItems.clear();
        m_tabTitle.Reset();
    }
}

void HttpResponse::FlushTable(string& http)
{
    string html = table_head;

    char * table_title_ = new char[strlen(table_title) + m_tabTitle.title.size()
            + m_tabTitle.key.size() + m_tabTitle.val.size() + 10];

    sprintf(table_title_, table_title, m_tabTitle.title.c_str(),
            m_tabTitle.key.c_str(), m_tabTitle.val.c_str());

    html += table_title_;
    delete [] table_title_;

    for (size_t i=0; i < m_tabItems.size(); i++)
    {
        html += "<tr><td class=\"e\">";
        html += m_tabItems[i].first;
        html += "</td><td class=\"v\">";
        html += m_tabItems[i].second;
        html += "</td></tr>";
    }

    html += table_end;

    char httphead[512];
    sprintf(httphead, head, html.size());

    http = httphead;
    http += html;
}

/////////////////////////////////////////////

HttpResponseManager::HttpResponseManager()
{
    W_GetAppPath(m_szAppPath);
}

HttpResponseManager::~HttpResponseManager()
{
    // 删除所有
    CleanAll();
}

// 写入数据
void HttpResponseManager::Write(PersistID id, const char* str)
{
    SMART_LOCK(this);
    GetResponse(id)->Write(str);
}

void HttpResponseManager::WriteLine(PersistID id, const char* str)
{
    SMART_LOCK(this);
    GetResponse(id)->WriteLine(str);
}

void HttpResponseManager::SetTableTitle(PersistID id, const char* title, const char* name, const char* val)
{
    SMART_LOCK(this);
    GetResponse(id)->SetTableTitle(title, name, val);
}

void HttpResponseManager::AddTableItem(PersistID id, const char* name, const char* val)
{
    SMART_LOCK(this);
    GetResponse(id)->AddTableItem(name, val);
}

void HttpResponseManager::Flush(PersistID id)
{
    string http;

    Lock();
    GetResponse(id)->Flush(http);
    Unlock();

    WShellAPI::SendData(id, (void*)http.c_str(), http.size());
}

HttpResponse* HttpResponseManager::GetResponse(PersistID id)
{
    SMART_LOCK(this);

    HttpResponse* respon = m_Responses[id];
    if (!respon)
    {
        respon = new HttpResponse;
        m_Responses[id] = respon;
    }

    return respon;
}

void HttpResponseManager::DeleteResonse(PersistID id)
{
    SMART_LOCK(this);

    std::map<PersistID, HttpResponse*>::iterator it = m_Responses.find( id );
    if (it != m_Responses.end())
    {
        if (it->second)
        {
            delete it->second;
        }
        m_Responses.erase(it);
    }
}

void HttpResponseManager::CleanAll()
{
    SMART_LOCK(this);

    std::map<PersistID, HttpResponse*>::iterator it;

    for (it = m_Responses.begin(); it != m_Responses.end(); it++)
    {
        if (it->second)
        {
            delete it->second;
        }
    }

    m_Responses.clear();
}

