//
//  MidastouchMgr.cpp
//  GameSrv
//
//  Created by jin on 15-6-11.
//
//

#include "MidastouchMgr.h"
#include "Role.h"
#include "GameLog.h"
#include "mail_imp.h"


INSTANTIATE_SINGLETON(MidastouchMou);

eMidastouchResult MidastouchMou::midastouch(Role *role)
{
	if (role->getMidastouchMgr()->getLastNum() <= 0) {
		return eMidastouchResult_NumOver;
	}
	
	if (!role->getMidastouchMgr()->isAlchemy()) {
		return eMidastouchResult_NotUse;
	}
	
	eMidastouchResult ret = role->getMidastouchMgr()->midastouch();
	
	return ret;
}

void MidastouchMou::getMidastouchInfo(Role *role, obj_midastouch_info &obj)
{
	obj.curNum = role->getMidastouchCurNum();
	obj.lastNum = role->getMidastouchMgr()->getLastNum();
	int index = role->getMidastouchIndex();
	MidastouchDef *def = MidastouchCfgMgr::sMidastouchCfg.getItem(index);
	if (NULL != def) {
		obj.ration = def->chargeRatio;
		obj.needrmb = def->needCoin;
	}
	obj.isSend = role->getMidastouchIsSend();
	obj.isAlchemy = role->getMidastouchMgr()->isAlchemy();
}

void MidastouchMou::rechargeToReward(int roleid, int rmb)
{
	Role *role = SRoleMgr.GetRole(roleid);
	if (NULL == role) {
		// 离线动作
		NewRoleProperty property;
		property.load(roleid);
		RoleMidastouchAction &action = property.getMidastouchAction();
		action.add(rmb);
		property.save();
		// 写日志
		Xylog log(eLogName_MidastouchAction, roleid);
		log << rmb;
		return;
	}
	this->rechargeToReward(role, rmb);
}

eMidastouchResult MidastouchMou::checkRechargeToReward(Role *role, int rmb)
{
	if (role->getMidastouchIndex() <= 0) {
		return eMidastouchResult_NotMidastouch;
	}
	
	if (role->getMidastouchIsSend() != 0) {
		return eMidastouchResult_Use;
	}
	
	MidastouchDef *def = MidastouchCfgMgr::sMidastouchCfg.getItem(role->getMidastouchIndex());
	if (NULL == def) {
		return eMidastouchResult_CurNoConfig;
	}
	
	if (rmb < def->needCoin) {
		return eMidastouchResult_RmbNoMeet;
	}
	
	return eMidastouchResult_Ok;
}

bool MidastouchMou::sendaward(Role *role, float ratio, int rmb, int award)
{
	if (award <= 0) {
		return true;
	}
	
	string mail_content = "Midastouch";
    string mail_tile = "Midastouch";
	string mail_name = "Midastouch";
	
	MailFormat *formate = MailCfg::getCfg("MidasTouch");
	if (NULL != formate) {
		mail_content = formate->content;
        mail_tile = formate->title;
        mail_name = formate->sendername;
	}
    
    find_and_replace(mail_content, 3,
					 xystr("%.2f", ratio).c_str(),
					 xystr("%d", rmb).c_str(),
					 xystr("%d", award).c_str());
    
    bool ret = sendMail(0,
                        mail_name.c_str(),
                        role->getRolename().c_str(),
                        mail_tile.c_str(),
                        mail_content.c_str(),
                        xystr("rmb %d*1;", award).c_str(),
						xystr("%d", role->getInstID()).c_str());
    return ret;
}

void MidastouchMou::rechargeToReward(Role *role, int rmb)
{
	if (NULL == role) {
		xyerr("[点石成金]　rechargeToReward　函数　出现role为NULL");
		return;
	}
	
	eMidastouchResult ret = this->checkRechargeToReward(role, rmb);
	
	Xylog log(eLogName_Midastouch, role->getInstID());
	log << rmb << role->getMidastouchIndex() << role->getMidastouchIsSend() << ret;
	
	if (ret != eMidastouchResult_Ok) {
		return;
	}
	
	MidastouchDef *def = MidastouchCfgMgr::sMidastouchCfg.getItem(role->getMidastouchIndex());
	if (NULL == def) {
		xyerr("[点石成金]　rechargeToReward　函数　出现MidastouchDef为NULL");
		return;
	}
	
	int award = floor(rmb * def->chargeRatio) - rmb;
	
	// 奖励邮件发送
	bool mailbet = this->sendaward(role, def->chargeRatio, rmb, award);
	if (!mailbet) {
		RoleMidastouchAction &action = role->getMidastouchAction();
		action.add(rmb);
		role->saveNewProperty();
		Xylog log(eLogName_MidastouchAction, role->getInstID());
		log << rmb;
	}
	else
	{
		role->getMidastouchMgr()->rechargeSuessc();
	}

	log << def->chargeRatio << def->needNum << def->needCoin << award << mailbet;
}


void MidastouchMgr::load(Role *role)
{
	mRole = role;
}

void MidastouchMgr::weeklyRefresh()
{
	mRole->setMidastouchIndex(0);
	mRole->setMidastouchCurNum(0);
	mRole->setMidastouchIsSend(0);
	mRole->saveNewProperty();
}

int MidastouchMgr::getLastNum()
{
	return MidastouchCfgMgr::sMidastouchCfg.getMaxNum() - mRole->getMidastouchCurNum();
}

bool MidastouchMgr::isAlchemy()
{
	if (this->getLastNum() <= 0) {
		return false;
	}
	int index = mRole->getMidastouchIndex();
	return (index > 0 && mRole->getMidastouchIsSend() == 0) ? false : true;
}

void MidastouchMgr::rechargeSuessc()
{
	mRole->setMidastouchIndex(0);
	mRole->setMidastouchIsSend(1);
	mRole->saveNewProperty();
}

eMidastouchResult MidastouchMgr::midastouch()
{
	int curNum = mRole->getMidastouchCurNum();
	// 配置表从1开始
	MidastouchDef *def = MidastouchCfgMgr::sMidastouchCfg.randItem(curNum + 1);
	if (NULL == def) {
		return eMidastouchResult_CurNoConfig;
	}
	
	mRole->setMidastouchIndex(def->index);
	mRole->setMidastouchIsSend(0);
	mRole->setMidastouchCurNum(curNum + 1);
	
	mRole->saveNewProperty();
	return eMidastouchResult_Ok;
}

void MidastouchMgr::offlineMidastouchAction()
{
	RoleMidastouchAction &action = mRole->getMidastouchAction();
	RoleMidastouchAction::Iterator it;
	for (it = action.begind(); it != action.end(); it++) {
		SMidastouchMou.rechargeToReward(mRole, *it);
	}
	action.clear();
	mRole->saveNewProperty();
}