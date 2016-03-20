//
//  CopyExtraAward.cpp
//  GameSrv
//
//  Created by jin on 14-11-13.
//
//

#include "CopyExtraAward.h"
#include "Singleton.h"
#include "GameLog.h"
#include "Role.h"
#include <ctime>
#include "datetime.h"

INSTANTIATE_SINGLETON(CopyExtraAwardMgr);

void CopyExtraAwardMgr::extraItemsAward(int roleId, int copyId,
								   const string &baseItems, string &resultItems)
{
	CopyExtraAward *def = CopyExtraAwardCfg::getExtraAward(copyId);
	if (NULL == def) {
		return;
	}

	Xylog log(eLogName_CopyExtraAward, roleId);
	log << copyId << baseItems << resultItems << def->index << def->multiple;

	if (!baseItems.empty()) {
		for (int i = 0; i < def->multiple; i++) {
			resultItems += ";" + baseItems;
		}
	}
	
    int benchmarkTime = Process::env.getInt("starttime", 0) + SECONDS_PER_DAY * def -> afterOpenServerDays; //满足开服多少天，转换为秒数
    int now = time(NULL);
    
	if (!def->extraItems.empty() && now > benchmarkTime) { //现在的时间已经过了基准时间，才可以发额外奖励
		resultItems += ";" + def->extraItems;
	}
	
	log << def->extraItems << resultItems;
}


void CopyExtraAwardMgr::extraBaseAward(int roleId, SceneCfgDef* scenecfg, RewardStruct& award)
{
	if (NULL == scenecfg) {
		return;
	}
	
	CopyExtraAward *def = CopyExtraAwardCfg::getExtraAward(scenecfg->sceneId);
	if (NULL == def) {
		return;
	}
	this->baseAward(scenecfg, award, def->multiple + 1);
}

void CopyExtraAwardMgr::baseAward(SceneCfgDef* scenecfg, RewardStruct& award, int num)
{
	if (num <= 0) {
		return;
	}
	
	RewardStruct temp;
	
	temp.reward_exp = scenecfg->copy_exp * num;
	temp.reward_gold = scenecfg->copy_gold * num;
	temp.reward_batpoint = scenecfg->copy_battlepoint * num;
	temp.reward_consval = scenecfg->copy_protoss * num;
	temp.reward_petexp = scenecfg->copy_petExp * num;
	
	award = award + temp;
}

void CopyExtraAwardMgr::doublePotion(Role* role, SceneCfgDef* scenecfg,
				  const string &baseItems, string &resultItems)
{
	if (NULL == scenecfg || NULL == role) {
		return;
	}
	
	if (role->getDoublePotionNum() > 0 && scenecfg->sceneType == stTeamCopy) {
        resultItems += ";" + baseItems;
    }
}

void CopyExtraAwardMgr::doublePotionBase(Role* role, SceneCfgDef* scenecfg, RewardStruct &award)
{
	if (NULL == scenecfg || NULL == role) {
		return;
	}
	
	if (role->getDoublePotionNum() > 0 && scenecfg->sceneType == stTeamCopy) {
		this->baseAward(scenecfg, award, 2);
		role->setDoublePotionNum(role->getDoublePotionNum() - 1);
        
        string action = Utils::makeStr("entercopy_%d", scenecfg->sceneId);
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesFriendCopyDoubleAward, action.c_str());
	}
}