//
//  ShareAward.cpp
//  GameSrv
//
//  Created by jin on 14-4-11.
//
//


#include "GameLog.h"
#include "Utils.h"
#include "DataCfg.h"
#include "Role.h"
#include "hander_include.h"


bool shareAwardFirst(Role *role)
{
	if (role->getShareAwardFirst() == 0) {
		vector<string> awards = StrSpilt(ShareAwardCfg::sfirstAward.c_str(), ";");
		ItemArray items;
		if (!role->addAwards(awards, items, "第一次分享奖励"))
		{
			return false;
		}
		
		Xylog log(eLogName_ShareAward, role->getInstID());
		log<<ShareAwardCfg::sfirstAward
		<<"第一次分享奖励";
		
		role->setShareAwardFirst(1);
		role->saveNewProperty();
		return true;
	}
	return false;
}

bool shareAwardWeekly(Role *role)
{
	if (!ShareAwardCfg::sIsWeekly) {
		return false;
	}
	if (role->getShareAwardWeekly() == 0) {
		vector<string> awards = StrSpilt(ShareAwardCfg::sWeeklyAward.c_str(), ";");
		ItemArray items;
		if (!role->addAwards(awards, items, "周分享奖励"))
		{
			return false;
		}
		
		Xylog log(eLogName_ShareAward, role->getInstID());
		log<<ShareAwardCfg::sWeeklyAward
		<<"周分享奖励";
		
		role->setShareAwardWeekly(1);
		role->saveNewProperty();
		return true;
	}
	return false;
}

bool shareAwardDay(Role *role)
{
	if (ShareAwardCfg::sCycleTime <= 0) {
		return false;
	}
	
	time_t tm = time(NULL);
	time_t now = tm - (tm % 86400);
	
	int distance = now % (ShareAwardCfg::sCycleTime * 86400);
	if (role->getShareAwardTime() != 0)
	{
		time_t uptime = role->getShareAwardTime();
		if ((now - distance) <= uptime) {
			return false;
		}
	}
	
	vector<string> awards = StrSpilt(ShareAwardCfg::sDailyAward.c_str(), ";");
	ItemArray items;
	if (!role->addAwards(awards, items, "日分享奖励"))
	{
		return false;
	}
	
	Xylog log(eLogName_ShareAward, role->getInstID());
	log<<ShareAwardCfg::sWeeklyAward
	<<"日分享奖励";
	
	role->setShareAwardTime(now);
	role->saveNewProperty();
	return true;
}

void shareAwardAll(Role *role, ack_shareaward_state &ack)
{
	ack.first = shareAwardFirst(role);
	ack.weekly = shareAwardWeekly(role);
	ack.day = shareAwardDay(role);
	
	role->send(&ack);
}

