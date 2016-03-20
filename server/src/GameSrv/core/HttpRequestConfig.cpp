
#include "HttpRequestConfig.h"





message_queue* HttpRequest::MQ = NULL;

HttpRequest _HttpRequest;

void HttpRequest::request(const char* url, const char* param, const char* name, int roleid, int sessionid, bool ishttps)
{
    std::string ret;
    CURLcode curlcode = xyCurlCommonRequest(url, param, ret, 5, ishttps, 3);
    if(curlcode != CURLE_OK) {
        Xylog log(eLogName_HttpRequest, roleid);
        log << url << param;
        return;
    }
    create_cmd(http_request_ret, http_request_ret);
    http_request_ret->fun_name = name;
    http_request_ret->roleid = roleid;
    http_request_ret->ret = ret;
	http_request_ret->sessionid = sessionid;
    sendCmdMsg(Game::MQ, http_request_ret);
}

void HttpRequest::addCallBack(const char* name, HttpRequestCallback cb)
{
    assert(name && cb);
    HttpRequsetCB::iterator it = _hrcb.find(name);
    if(it != _hrcb.end()) {
        return;
    }
    _hrcb.insert(make_pair(name, cb));
}

void HttpRequest::handle(const char* name, int roleid, const char* param, int sessionid)
{
    HttpRequsetCB::iterator it = _hrcb.find(name);
    if(it == _hrcb.end() || NULL == it->second){
        xyerr(" HttpRequest name = %s 没有回调函数", name);
        return;
    }
    it->second(roleid, sessionid, param);
}
