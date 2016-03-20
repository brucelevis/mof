//
//  ActivationCode.h
//  GameSrv
//
//  Created by prcv on 13-8-5.
//
//  激活码管理


#ifndef __GameSrv__ActivationCodeMgr__
#define __GameSrv__ActivationCodeMgr__

#include <set>
#include <map>
#include <string>
#include "Defines.h"
#include "Singleton.h"


using namespace std;

class Role;

class ActivationCode
{
    // 激活码类型(唯一,一个玩家只能领取一个类型CODE)
    READONLY(int, mCodeType, CodeType);
    // 激活码ID
    READONLY(int, mCodeId, CodeId);
public:
    ActivationCode(){};
    ActivationCode(int codeType, int codeId)
            :mCodeType(codeType), mCodeId(codeId){};
};

typedef std::map<int, ActivationCode> ActivationTd;

class ActivationCodeMgr
{
public:
    bool load(Role *role);
    bool add(ActivationCode &ac);
	
    static bool save(int roleId, ActivationCode &ac);
	
	// 处理离线动作的内容
	void offlineAction();
private:

    ActivationCode * getActivation(int type);
    ActivationTd mActivationMap;
	Role *mRole;
};

#endif /* defined(__GameSrv__ActivationCodeMgr__) */
