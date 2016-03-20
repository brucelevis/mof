//
//  MidastouchMgr.h
//  GameSrv
//
//  Created by jin on 15-6-11.
//
//

#ifndef __GameSrv__MidastouchMgr__
#define __GameSrv__MidastouchMgr__

#include <iostream>
#include "DataCfgActivity.h"

class Role;

class MidastouchMou
{
public:
	eMidastouchResult midastouch(Role *role);
	
	void getMidastouchInfo(Role *role, obj_midastouch_info &obj);
	
	void rechargeToReward(int roleid, int rmb);
	void rechargeToReward(Role *role, int rmb);
private:
	eMidastouchResult checkRechargeToReward(Role *role, int rmb);
	bool sendaward(Role *role, float ratio, int rmb, int award);

};
#define SMidastouchMou Singleton<MidastouchMou>::Instance()


class MidastouchMgr
{
public:
	MidastouchMgr(){}
	void load(Role *role);
	
	void online()
	{
		offlineMidastouchAction();
	}
	
	void rechargeSuessc();

	// 练金
	eMidastouchResult midastouch();
	
	// 能否练金
	bool isAlchemy();
	int getLastNum();
	void weeklyRefresh();
private:
	void offlineMidastouchAction();
	
	Role *mRole;
};

#endif /* defined(__GameSrv__MidastouchMgr__) */
