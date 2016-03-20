//
//  serverCfg.cpp
//  CenterServer
//
//  Created by 丁志坚 on 15/6/9.
//
//

#include "serverCfg.h"
#include "json/json.h"
#include "curl_helper.h"
#include "Utils.h"
void
ServerCfg::loadFromHttp(const char* url) {
    string result;
    xyCurlCommonRequest(url, "", result, 10, false, 3);
    load(result.c_str());
}

void
ServerCfg::load(const char *str) {
    Json::Value jsonValue;
    Json::Reader reader;
    reader.parse(str, jsonValue);
    if (!jsonValue.isObject()) {
        log_error("server info error");
        return;
    }
    
    clear();
    Json::Value list = jsonValue["serverList"];
    if (!list.isArray()) {
        return;
    }

    for (int i = 0; i < list.size(); i++) {
        Json::Value serverInfo = list[i];
        int cfgId = Utils::safe_atoi(serverInfo["v"].asString().c_str());
        int runId = Utils::safe_atoi(serverInfo["id"].asString().c_str());
        string name = serverInfo["n"].asString();
        addServer(cfgId, runId, name.c_str());
    }
}

void
ServerCfg::dump() {
    for (map<int, ServerCfgDef*>::iterator iter = mCfgIndex.begin(); iter != mCfgIndex.end(); iter++) {
        log_info(iter->second->mCfgId << " " << iter->second->mRunId << " " << iter->second->mName);
    }
}


vector<int> 
ServerCfg::getAllRunId() {
    std::vector<int> ids;;
    for (map<int, vector<ServerCfgDef*> >::iterator iter = mRunIndex.begin(); iter != mRunIndex.end(); iter++) {
        ids.push_back(iter->first);
    }   

    return ids;
}

bool
ServerCfg::existRunId(int serverId) {
    return mRunIndex.find(serverId) != mRunIndex.end();
}


ServerCfgDef* 
ServerCfg::getServerCfgDef(int serverId) {
    map<int, ServerCfgDef*>::iterator iter = mCfgIndex.find(serverId);
    if (iter == mCfgIndex.end()) {
        return NULL;
    }

    return iter->second;
}





