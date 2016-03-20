
#include "ActivationCodeMgr.h"
#include "main.h"
#include "Utils.h"
#include "GameLog.h"
#include "redis_helper.h"
#include "Role.h"
#include "HttpRequestApp.h"

bool ActivationCodeMgr::load(Role *role)
{
	mRole = role;
    mActivationMap.clear();

    RedisResult result(redisCmd("hgetall activation:%d", mRole->getInstID()));
    int elementnum = result.getHashElements();
    for (int i = 0; i < elementnum; i += 2)
    {
        int type = Utils::safe_atoi(result.readHash(i, "").c_str(), 0);
        int codeId = Utils::safe_atoi(result.readHash(i + 1, "").c_str(), 0);
        ActivationCode ac(type, codeId);
        mActivationMap.insert(std::make_pair(type, ac));
    }
    return true;
}

void ActivationCodeMgr::offlineAction()
{
	string ret = mRole->getActivation();
	if (ret.empty()) {
		return;
	}
	// 处理离线的激活码
	mRole->setActivation("");
	httpRequestCbActivation(mRole->getInstID(), mRole->getSessionId(), ret.c_str());
    mRole->saveNewProperty();
}

bool ActivationCodeMgr::add(ActivationCode &ac)
{
    if (NULL != this->getActivation(ac.getCodeType()))
    {
        return false;
    }
    mActivationMap.insert(std::make_pair(ac.getCodeType(), ac));
    return true;
}

bool ActivationCodeMgr::save(int roleId, ActivationCode &ac)
{
    doRedisCmd("hmset activation:%d %d %d", roleId, ac.getCodeType(), ac.getCodeId());
    return true;
}

ActivationCode * ActivationCodeMgr::getActivation(int type)
{
    ActivationTd::iterator it = mActivationMap.find(type);
    if (it == mActivationMap.end())
    {
        return NULL;
    }
    return &it->second;
}