//
//  PushNotifyMgr.cpp
//  GameSrv
//
//  Created by prcv on 14-10-31.
//
//

#include "PushNotifyMgr.h"

#include "cmd_def.h"
#include "HttpRequestConfig.h"


void pushNotifyToRole(int roleId, const char* ticker, const char* title, const char* text)
{
    Json::Value dataValue;
    dataValue["ticker"] = ticker;
    dataValue["title"] = title;
    dataValue["text"] = text;
    dataValue["serverid"] = Process::env.getInt("server_id");
    dataValue["roleid"] = roleId;
    string postData = Json::FastWriter().write(dataValue);
    
    create_cmd(http_request, http_request);
    http_request->fun_name = "";
    http_request->url = UrlCfg::get_pushnotify_url();
    http_request->post_param = "";
    http_request->roleid = 0;
	http_request->sessionid = 0;
    http_request->is_https = false;
    sendCmdMsg(HttpRequest::MQ, http_request);
}

void pushNotifyToAllRoles(const char* ticker, const char* title, const char* text)
{
    Json::Value dataValue;
    dataValue["ticker"] = ticker;
    dataValue["title"] = title;
    dataValue["text"] = text;
    dataValue["serverid"] = Process::env.getInt("server_id");
    string postData = Json::FastWriter().write(dataValue);
    
    create_cmd(http_request, http_request);
    http_request->fun_name = "";
    http_request->url = UrlCfg::get_pushnotify_url();
    http_request->post_param = postData;
    http_request->roleid = 0;
	http_request->sessionid = 0;
    http_request->is_https = false;
    sendCmdMsg(HttpRequest::MQ, http_request);
}