//
//  CopyExtraAward.h
//  GameSrv
//
//  Created by jin on 14-11-13.
//
//

#ifndef __GameSrv__CopyExtraAward__
#define __GameSrv__CopyExtraAward__

#include <iostream>
#include "std_includes.h"
class SceneCfgDef;
class RewardStruct;

using namespace std;
class Role;

class CopyExtraAwardMgr
{
public:
	void extraItemsAward(int roleId, int copyId,
					const string &baseItems, string &resultItems);
	
	void extraBaseAward(int roleId, SceneCfgDef* scenecfg, RewardStruct &award);
	
	// 双倍药水，只对好友副本有效
	void doublePotion(Role* role, SceneCfgDef* scenecfg,
					  const string &baseItems, string &resultItems);
	
	void doublePotionBase(Role* role, SceneCfgDef* scenecfg, RewardStruct &award);
private:
	void baseAward(SceneCfgDef* scenecfg, RewardStruct& award, int num);
};

#define SCopyExtraAward Singleton<CopyExtraAwardMgr>::Instance()

#endif /* defined(__GameSrv__CopyExtraAward__) */
