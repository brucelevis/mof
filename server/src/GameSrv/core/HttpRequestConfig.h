
#ifndef __GameSrv__HttpRequestConfig__
#define __GameSrv__HttpRequestConfig__

#include <string>
#include <stdio.h>
#include <assert.h>
#include "thread.h"
#include "MQ.h"
#include "cmd_def.h"
#include "cmdprocessor.h"
#include "threadpool.h"
#include "GameLog.h"
#include "curl_helper.h"
#include "Game.h"
#include "HttpRequestApp.h"

class HttpRequest
{
private:
	typedef void (*HttpRequestCallback)(int roleid, int sessionid, const char* param);
public:
	// 处理HTTP请求
    void request(const char* url, const char* param, const char* name, int roleid, int sessionid, bool ishttps = false);
    // 注册回调函数
    void addCallBack(const char* name, HttpRequestCallback hrcb);
    void handle(const char* name, int roleid, const char* param, int sessionid);
    static message_queue* MQ;
private:
	typedef std::map<string, HttpRequestCallback> HttpRequsetCB;
    HttpRequsetCB _hrcb;
};
extern HttpRequest _HttpRequest;

class HttpRequestProxy
{
public:
    bool init()
    {
        threadHttp.init(10);
        proxyHttp.init(HttpRequest::MQ);
        return true;
    }

    void start()
    {
        threadHttp.start(&proxyHttp);
    }

    void stop()
    {
        proxyHttp.stop();
        threadHttp.join();
    }
private:
    ThreadPool threadHttp;
    CmdProcessor proxyHttp;
};
#endif
