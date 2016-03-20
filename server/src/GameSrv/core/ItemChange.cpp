//
//  ItemChange.cpp
//  GameSrv
//
//  Created by jin on 13-10-24.
//
//

#include "ItemChange.h"
#include "DataCfgActivity.h"
#include "main.h"
#include "Utils.h"
#include "GameLog.h"
#include "DBRolePropName.h"
#include "json/json.h"
#include "hander_include.h"
#include "mail_imp.h"
#include "Role.h"

void ItemChangeMgr::itemChange(int roleId)
{
	Role *role = SRoleMgr.GetRole(roleId);
	if (NULL == role) {
		return;
	}
	itemChange(role);
}

void ItemChangeMgr::itemChange(Role *role)
{
	BackBag *bag = role->getBackBag();
	int roleId = role->getInstID();
	bool isUpdate = false;
	ItemChangeCfgMgr::Iterator it;
	for (it = ItemChangeCfgMgr::begin(); it != ItemChangeCfgMgr::end(); it++) {
		// 建壮性检测配置表
		if (it->second.proto_id == it->second.goal_id) {
			continue;
		}
		
		int itemNum = bag->GetItemNum(it->second.proto_id);
		if (itemNum <= 0) {
			continue;
		}
		
		// 计算能转换的目标数量, 与金钻数量
		int goalNum = ceil(itemNum * it->second.change);
		int rmb = ceil(itemNum * it->second.compensation_rmb);
		
		// 删除原item
		ItemArray proto_items;
		GridArray proto_effgrids;
		proto_items.push_back(ItemGroup(it->second.proto_id, itemNum));
		if (!bag->PreDelItems(proto_items, proto_effgrids))
		{
			continue;
		}
		
		bag->UpdateBackBag(proto_effgrids);
		bag->Store(roleId, proto_effgrids);
        
        { //use item log
            LogMod::addUseItemLog(role, proto_items, "item_change");
        }
		
		Xylog log(eLogName_ItemChange, roleId);
		// 添加目标item
		ItemArray goal_id_items;
		GridArray goal_id_effgrids;
		goal_id_items.push_back(ItemGroup(it->second.goal_id, goalNum));
		if (!bag->PreAddItems(goal_id_items, goal_id_effgrids))
		{
			log<<it->second.goal_id<<goalNum<<rmb<<"未成功";
			continue;
		}
		bag->UpdateBackBag(goal_id_effgrids);
		bag->Store(roleId, goal_id_effgrids);
		
		// 添加目标金钻
		role->addRmb(rmb, "物品转换补偿", false);
		
		isUpdate = true;
		
		// 写转换日志
		log<<it->second.proto_id \
		<<itemNum \
		<<it->second.goal_id \
		<<goalNum \
		<<rmb;
		
		// 写增加item日志
		LogMod::addLogGetItem(role, role->getRolename().c_str(), role->getSessionId(), goal_id_items, "itemchange");
	}
	
	// 整理背包
	if (isUpdate) {
		vector<int> effgrids;
		bag->Sort(effgrids);
		bag->Store(roleId, effgrids);
	}
}