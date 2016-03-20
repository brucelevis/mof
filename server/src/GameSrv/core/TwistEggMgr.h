//
//  TwistEggMgr.h
//  GameSrv
//
//  Created by jin on 15-1-6.
//
//

#ifndef __GameSrv__TwistEggMgr__
#define __GameSrv__TwistEggMgr__

#include <iostream>
#include "DataCfgActivity.h"

class Role;

class TwistEggMgr {
public:
	void init();

	int getTwistEgg(int index, Role* role, bool free, bool isEven,
					vector<const TwistEggGoodsDef*>& goods);
	
	int checkTwist(int index, int activityId, Role* role, bool free, bool isEven);
	
	
	void getTwisteggInfo(Role* role, const TwistEggCommonDef& def, obj_twistegg_info& obj);
	void getTwisteggInfo(Role* role, int index, obj_twistegg_info& obj);
	
	
	void addActivityTimer();
	void sendActivityNotify();
private:
	string getGoodsLogStr(vector<const TwistEggGoodsDef*>& goods);
	int checkConsume(Role* role, const TwistEggCommonDef* def, bool isFree, bool isEven);
	int useConsume(Role* role, const TwistEggCommonDef* def, bool isFree, bool isEven, int& consumeType);
	void sendBroadcast(Role* role, vector<const TwistEggGoodsDef*>& goods);
	
	static void OnActivityRefresh(void *param);
};

#define STwistEggMgr Singleton<TwistEggMgr>::Instance()

#endif /* defined(__GameSrv__TwistEggMgr__) */
