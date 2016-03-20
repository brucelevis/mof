//
//  GuildStore.cpp
//  GameSrv
//
//  Created by jin on 13-9-10.
//
//

#include "GuildStore.h"
#include "GuildRole.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "DynamicConfig.h"
#include "Role.h"

INSTANTIATE_SINGLETON(GuildStoreOpenMgr);

GuildStoreOpenMgr::GuildStoreOpenMgr()
{
	time_t now = time(NULL);
	mInterval = GuildCfgMgr::getGoodsCfg()->getInterval();
	
	int tmLen = now % mInterval;
	// 上一次刷新时间
	mUpdateTime = now - tmLen;
	mIsOpen = true;
	
	// 下一次刷新回调
	addTimer(OnRefresh, NULL, mInterval - tmLen, 1);
}

void GuildStoreOpenMgr::OnRefresh(void *param)
{
	SGuildStoreOpenMgr.refresh();
	addTimer(OnRefresh, NULL, SGuildStoreOpenMgr.getInterval(), 1);
}


void GuildStoreMgr::load(Role *role)
{
    mRole = role;
	mRgCfg = GuildCfgMgr::getGoodsCfg();
	mSgOpen = &SGuildStoreOpenMgr;

	mUpdateTime = mRole->getGoodsUpdateTime();
	mUpdateNum = mRole->getGoodsUpdateNum();
	
	insert();
}

void GuildStoreMgr::saveUpdateTime()
{
	mRole->setGoodsUpdateTime(mUpdateTime);
	mRole->saveNewProperty();
}

void GuildStoreMgr::saveUpdateNum()
{
	mRole->setGoodsUpdateNum(mUpdateNum);
	mRole->saveNewProperty();
}

int GuildStoreMgr::getLvl()
{
	Guild& guild = SGuildMgr.getRoleGuild(mRole->getInstID());
	return guild.getLevel();
}

//金钻刷新次数是否已经用完
bool GuildStoreMgr::isTimesUseUp(const int &viplvl)
{
    if (!mRgCfg->isUseGuildViplvlRefreshLimit())
    {
        return false;
    }
    
    int guildVipTimes = mRgCfg->getGuildVipRefreshTimes(viplvl);
    if (mUpdateNum > (guildVipTimes + mRgCfg->getFreeNum()))
    {
        return true;
    }
    return false;
}




