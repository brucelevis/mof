//
//  WebStateMgr.cpp
//  GameSrv
//
//  Created by jin on 14-10-21.
//
//

#include "WebStateMgr.h"
#include "GameLog.h"
#include "hander_include.h"
#include "DynamicConfig.h"
#include "HttpRequestApp.h"
#include "Role.h"

#define WEB_STATE_UPDATE_INTERVAL 60

WebStateMou g_WebStateMou;

void WebStateMou::init()
{
	WebStateMou::onRefresh(NULL);
	addTimer(onRefresh, NULL, WEB_STATE_UPDATE_INTERVAL, FOREVER_EXECUTE);
	
	WebStateMou::onRefreshIpList(NULL);
	addTimer(onRefreshIpList, NULL, WEB_STATE_UPDATE_INTERVAL, FOREVER_EXECUTE);
	
	WebStateMou::onRefreshUpServer(NULL);
	addTimer(onRefreshUpServer, NULL, WEB_STATE_UPDATE_INTERVAL, FOREVER_EXECUTE);
}

void WebStateMou::onRefresh(void *param)
{
	create_cmd(http_request, http_request);
    http_request->fun_name = HTTP_WEB_STATE;
    http_request->url = xystr("%s?viewId=%d", UrlCfg::getUrl(HTTP_WEB_STATE).c_str(), Process::env.getInt("server_id"));
    http_request->post_param = "";
    http_request->is_https = false;
    sendCmdMsg(HttpRequest::MQ, http_request);
}

void WebStateMou::onRefreshIpList(void *param)
{
	create_cmd(http_request, http_request);
    http_request->fun_name = HTTP_IP_LIST;
    http_request->url = UrlCfg::getUrl(HTTP_IP_LIST);
    http_request->post_param = "";
    http_request->is_https = false;
    sendCmdMsg(HttpRequest::MQ, http_request);
}

void WebStateMou::onRefreshUpServer(void *param)
{
	create_cmd(http_request, http_request);
    http_request->fun_name = HTTP_UP_SERVER;
    http_request->url = UrlCfg::getUrl(HTTP_UP_SERVER);
    http_request->post_param = xystr("onlineNum=%d&serverId=%d",
									 SRoleMgr.getRoleNum(), Process::env.getInt("server_id"));
    http_request->is_https = false;
    sendCmdMsg(HttpRequest::MQ, http_request);
}

bool WebStateMou::isSafeIp(const char *ip)
{
    return find(mIpList.begin(), mIpList.end(), ip) == mIpList.end() ? false : true;
}

void WebStateMou::loadIpList(const char *ret)
{
	if (NULL == ret) {
		return;
	}
	
	Xylog log(eLogName_IpList, 0);
	log << ret;
	
	Json::Value val;
	if (!xyJsonSafeParse(ret, val)) {
		return;
	}
	
	Json::Value data = val["data"];
	if (!data.isArray()) {
		return;
	}
	
	mIpList.clear();
	for (int i = 0; i < data.size(); i++) {
		Json::Value &temp = data[i];
		if (!temp.isObject()) {
			continue;
		}
		if (temp["ip"].isNull()) {
			continue;
		}
		mIpList.push_back(xyJsonAsString(temp["ip"]));
	}
	
	log << mIpList.size();
}

void WebStateMou::load(const char* ret)
{
	if (NULL == ret) {
		return;
	}
	
	Xylog log(eLogName_WebState, 0);
	log << ret;
	
	Json::Value val;
	if (!xyJsonSafeParse(ret, val)) {
		return;
	}
	
	if (val["ret"].isNull()) {
		return;
	}
	
	if (xyJsonAsInt(val["ret"]) != 0) {
		return;
	}
	
	Json::Value data = val["data"];
	if (data.isNull() || !data.isObject()) {
		return;
	}
	
	mState = xyJsonAsInt(data["state"]);
	mAppCmdState = xyJsonAsInt(data["appcmd"]);
	// 带多点特殊的信息"server"
	mTips = "server:" + xyJsonAsString(data["desc"]);
	
	log << mState << mTips << mAppCmdState;
}