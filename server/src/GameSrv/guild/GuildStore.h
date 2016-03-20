//
//  GuildStore.h
//  GameSrv
//
//  Created by jin on 13-9-10.
//
//

#ifndef GameSrv_GuildStore_h
#define GameSrv_GuildStore_h


#include "StoreGoodsBase.h"

// 公会商店管理类
class GuildStoreOpenMgr : public StoreGoodsOpenBase
{
public:
	GuildStoreOpenMgr();
    ~GuildStoreOpenMgr(){}
	
	static void OnRefresh(void *param);
};
#define SGuildStoreOpenMgr Singleton<GuildStoreOpenMgr>::Instance()

// 公会商店角色管理类
class GuildStoreMgr : public StoreGoodsBase
{
public:
	void load(Role *role);
    
    bool isTimesUseUp(const int &viplvl); //判断刷新次数是否已经用完

private:
    
	virtual int getLvl();
	virtual const char * getStoreKey() {return "guildStore";}
	virtual void saveUpdateTime();
	virtual void saveUpdateNum();
};

#endif
