//
//  DynamicCfgApp.cpp
//  GameSrv
//  动态配置应用
//  Created by Huang Kunchao on 13-7-19.
//  
//

#include "DynamicCfgApp.h"
#include "DataCfg.h"
#include <vector>
#include "ActivityMod.h"
#include "DynamicConfig.h"
using namespace std;

////////////////////////////////////////////////////////////////////
//抽奖配置
static void* readLotteryConfig()
{
    return LotteryCfgMgr::read();
}

static void refreshLotteryConfig(void* param)
{
    if(NULL == param){
        xyerr("解释抽奖配置失败");
        return;
    }
    
    LotteryCfg* cfg = (LotteryCfg*)param;
    
    Xylog log(eLogName_Lottery, 0);
    log<<cfg->dump();
    
    LotteryCfgMgr::replace( (LotteryCfg*)param );
}

addTimerDef(refreshLotteryConfig, readLotteryConfig, 10*60,  FOREVER_EXECUTE);

////////////////////////////////////////////////////////////////////
//幻兽大比拼配置
static void* readActivityPetCasinoConfig()
{
    return ActivityPetCosinoCfgMgr::readcfg();
}

static void replaceActivityPetCasinoConfig(void* param)
{
    if(NULL == param){
        xyerr("幻兽大比拼配置替换失败");
        return;
    }
    tagActivityPetCosinoCfg* cfg = (tagActivityPetCosinoCfg*)param;
    
    Xylog log(eLogName_PetCasion, 0);
    log<<cfg->dump();
    
    ActivityPetCosinoCfgMgr::replacecfg( (tagActivityPetCosinoCfg*)param );
}

addTimerDef(replaceActivityPetCasinoConfig,readActivityPetCasinoConfig, 10*60,FOREVER_EXECUTE);




////////////////////////////////////////////////////////////////////






