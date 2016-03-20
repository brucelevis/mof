//
//  configManager.cpp
//  CenterServer
//
//  Created by 丁志坚 on 15/6/9.
//
//

#include "configManager.h"
#include "process.h"
#include "serverCfg.h"
#include "DataCfg.h"

ConfigManager g_ConfigManager;

void
ConfigManager::init() {
    mServerCfg = new ServerCfg;
    
    mBatRankListGroupCfg        = new ServerGroupCfg;
    mConsumeRankListGroupCfg    = new ServerGroupCfg;
    mRechargeRankListGroupCfg   = new ServerGroupCfg;
    mPetRankListGroupCfg        = new ServerGroupCfg;
    
    load();
}

void
ConfigManager::load() {

    Process::respath = Process::execpath + "/../res";
    
    string lang = Process::env.getString("lang");
    Cfgable::pathPrefix = Process::respath + "/" + lang + "/config/";

    g_ConfigTableMgr.addSearchPath(Process::respath + "/" + lang + "/system/");
    
    g_ConfigTableMgr.addSearchPath(Cfgable::pathPrefix);
    
    g_ConfigTableMgr.loadAllTable();

	string platform = Process::env.getString("platform");
    if (platform.empty()) {
        const char* url = "http://192.168.1.109:8080/servermgr/gdzh_serverlist.php?channel=&version=&platform=";
        mServerCfg->loadFromHttp(url);
    } else {
        const char* url = "http://gdzh.vxinyou.com/interface/platformServerList.php";
        string requestUrl = strFormat("%s?platform=%s", url, platform.c_str());
        mServerCfg->loadFromHttp(requestUrl.c_str());
    }
	mServerCfg->dump();
    
    mBatRankListGroupCfg->load(Cfgable::getFullFilePath("CrossServiceBattleRankListCfg.ini"), eSortBat);
    mConsumeRankListGroupCfg->load(Cfgable::getFullFilePath("CrossServiceConsumeRankListCfg.ini"), eSortConsume);
    mRechargeRankListGroupCfg->load(Cfgable::getFullFilePath("CrossServiceRechargeRankListCfg.ini"), eSortRecharge);
    mPetRankListGroupCfg->load(Cfgable::getFullFilePath("CrossServicePetRankListCfg.ini"), eSortPet);

    CsPvpGroupCfg::load(Cfgable::getFullFilePath("csPvpGroupCfg.ini").c_str());
    CrossServiceWarWorshipCfg::load(Cfgable::getFullFilePath("CrossServiceWorship.ini").c_str());
    CrossServiceWarRankingserviceCfg::load(Cfgable::getFullFilePath("rankingservice.ini"));
    CrossServiceWarRankingsimpleCfg::load(Cfgable::getFullFilePath("rankingsimple.ini"));
}

ServerGroupCfg*
ConfigManager::getRankListGroupCfg(int type)
{
    switch (type) {
        case eSortBat:
        {
            if (mBatRankListGroupCfg->isLoaded()) {
                return mBatRankListGroupCfg;
            }
            break;
        }
        case eSortConsume:
        {
            if (mConsumeRankListGroupCfg->isLoaded()) {
                return mConsumeRankListGroupCfg;
            }
            break;
        }
        case eSortRecharge:
        {
            if (mRechargeRankListGroupCfg->isLoaded()) {
                return mRechargeRankListGroupCfg;
            }
            break;
        }
        case eSortPet:
        {
            if (mPetRankListGroupCfg->isLoaded()) {
                return mPetRankListGroupCfg;
            }
            break;
        }
        default:
            break;
    }
    return NULL;
}