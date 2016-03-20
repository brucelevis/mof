//
//  configManager.h
//  CenterServer
//
//  Created by 丁志坚 on 15/6/9.
//
//

#ifndef __CenterServer__configManager__
#define __CenterServer__configManager__

#include <stdio.h>
#include "serverCfg.h"
#include "EnumDef.h"

class ServerCfg;
class ServerGroupCfg;

class ConfigManager
{
public:
    void init();
    void load();
    
    ServerCfg* getServerCfg()
    {
    	return mServerCfg;
    }
    
    ServerGroupCfg* getRankListGroupCfg(int type);
    ServerGroupCfg*  mBatRankListGroupCfg;
    ServerGroupCfg*  mConsumeRankListGroupCfg;
    ServerGroupCfg*  mRechargeRankListGroupCfg;
    ServerGroupCfg*  mPetRankListGroupCfg;
    
    ServerCfg*       mServerCfg;
};

extern ConfigManager g_ConfigManager;

#endif /* defined(__CenterServer__configManager__) */
