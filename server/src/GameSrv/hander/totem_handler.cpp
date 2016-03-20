//
//  totem_handler.cpp
//  GameSrv
//
//  Created by jin on 14-5-19.
//
//

#include "hander_include.h"
#include "PetPvp.h"

hander_msg(req_totem_group, req)
{
	hand_Sid2Role_check(sessionid, roleid, role)
	ack_totem_group ack;
	TotemMgr *mgr = role->getTotemMgr();
	for (TotemMgr::Iterator it = mgr->begin(); it != mgr->end(); it++) {
		obj_totem_info obj;
		obj.totemId = it->second.getId();
		obj.totemLvl = it->second.getLvl();
		obj.totemNum = it->second.getNum();
		ack.info.push_back(obj);
	}
	role->send(&ack);
}}

hander_msg(req_totem_immolation, req)
{
	hand_Sid2Role_check(sessionid, roleid, role)
	ack_totem_immolation ack;
	
	do {
		SysTotemInfo *info = SysTotemCfgMgr::sCfg.getInfo(req.totemId);
		if (NULL == info) {
			ack.errorcode = eTotemImmolation_TotemErr;
			break;
		}
		const Totem& totem = role->getTotemMgr()->getTotemConst(req.totemId);
		// 1: 检查宠物选择的合法性
		ack.errorcode = STotemMon.petCheck(role, req.pets);
		if (ack.errorcode != eTotemImmolation_Success) {
			break;
		}
		
		// 2: 检查宠物是否满足献祭
		ack.errorcode = STotemMon.totemCheck(totem, role, req.pets);
		if (ack.errorcode != eTotemImmolation_Success) {
			break;
		}
		
		// 3: 献祭宠物
		bool isUp = role->getTotemMgr()->addTotem(totem.getId(), req.pets.size());
		
		// 4: 删除宠物
		for (vector<int>::const_iterator it = req.pets.begin(); it != req.pets.end(); it++) {
			role->mPetMgr->removePet(*it);
		}
		role->mPetMgr->save();
		PetMgr::delPetsFromDB(req.pets);
		
		// 5: 删除幻兽竞技场宠物
		SPetPvpMgr.safe_removePetsInFormation(roleid, req.pets);
		
		// 6: 记录日志
		Xylog log(eLogName_TotemImmolation, roleid);
		log << StrJoin(req.pets.begin(), req.pets.end());
		
		// 7: (等级变化)更新加成数据,同步宠物加成数据, 同步人物加成数据
		if (isUp) {
			STotemMon.updateAddition(roleid);
			STotemMon.sendAllPet(role);
			
			role->onCalPlayerProp();
			role->sendRolePropInfoAndVerifyCode();
		}
		
		// 8: 返回结果
		ack.info.totemId = totem.getId();
		ack.info.totemLvl = totem.getLvl();
		ack.info.totemNum = totem.getNum();
		ack.pets = req.pets;
		ack.errorcode = eTotemImmolation_Success;
	} while (false);
	
	role->send(&ack);
}}