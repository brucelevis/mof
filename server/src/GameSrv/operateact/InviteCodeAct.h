//
//  InviteCodeAct.h
//  GameSrv
//
//  Created by hekang on 14-7-14.
//
//

#ifndef __GameSrv__InviteCodeAct__
#define __GameSrv__InviteCodeAct__

/**************************************************
 功能描述：给好友一个邀请码。达到奖励要求后，双方都可以领取奖励。
 
 规则：
 ①	每个角色创建后，都会获得一个邀请码。
 ②	进入“好友邀请”界面后，就可以看到自己的邀请码了
 ③	每个角色只能输入一次邀请码
 ④	邀请码 可 复制
 **************************************************/

#include <iostream>

#include "DynamicConfig.h"
#include "mail_imp.h"
#include "GameLog.h"
#include "process.h"

#include <vector>
#include <string>
#include "RedisHashJson.h"
#include "Utils.h"

class Role;

//邀请码

class InviteCodeActivity
{
public:
    struct AwardItem
    {
        int NeedCount;//需要输入的激活码次数
        string items; //奖励物品
    };
    void init();    // 读配置
    void inputCode(int roleid,std::string & codeStr); // 输入激活码
    void getAward(int roleid, int index); // 领奖
    void sendStatus(int roleid); // 发送当前状态
protected:
    typedef RedisHashJson<int>  JsonInfoTypeKeyByInt;
    typedef RedisHashJson<string>  JsonInfoTypeKeyByString;
    JsonInfoTypeKeyByInt mRoleIDToInviteCode;  //玩家对应的激活码数据，这个是通过玩家找激活码用
    JsonInfoTypeKeyByString mInviteCodeToRoleID;  //玩家输入的激活码数据，这个是通过激活码找玩家
    //JsonInfoTypeKeyByString mDictInviteCodeToRoleID;//通过邀请码找到玩家，作为字典用
    bool sendAward(Role* role ,  const string& award);
};

#define SInviteCodeMgr Singleton<InviteCodeActivity>::Instance()

#endif /* defined(__GameSrv__InviteCodeAct__) */
