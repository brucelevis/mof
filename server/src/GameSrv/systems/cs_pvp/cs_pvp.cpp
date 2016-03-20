#include "cs_pvp.h"
#include "DataCfg.h"
#include "process.h"
#include <time.h>

bool 
CsPvpSystem::canWorship() {
	time_t now = time(NULL);
	if (isClose(now)) {
		return false;
	}

	tm nowTm;
	localtime_r(&now, &nowTm);
	return CrossServiceWarWorshipCfg::canWorship(nowTm.tm_wday, nowTm.tm_hour, nowTm.tm_min);
}

bool 
CsPvpSystem::canBattle() {
	time_t now = time(NULL);
	if (isClose(now)) {
		return false;
	}

	tm nowTm;
	localtime_r(&now, &nowTm);
	return CrossServiceWarWorshipCfg::canBattle(nowTm.tm_wday, nowTm.tm_hour, nowTm.tm_min);
}

bool
CsPvpSystem::isClose(time_t testTime) {
	int serverId = Process::env.getInt("server_id");
	return (CsPvpGroupCfg::isClose(serverId, testTime));
}