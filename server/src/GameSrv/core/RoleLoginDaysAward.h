//
//  RoleAward.h
//  GameSrv
//
//  Created by wind on 15-1-6.
//
//

#ifndef __GameSrv__RoleAward__
#define __GameSrv__RoleAward__

#include "RedisHashJson.h"
class Role;

typedef RedisHashJson<int> RoleAwardHash;
class RoleLoginDaysAwardMgr
{
public:
    RoleLoginDaysAwardMgr();
    ~RoleLoginDaysAwardMgr();
    
    //
    void load(Role* role);
    
    // 刷新角色奖励的状态
    void refreshLoginDays();
    // 根据登陆天数刷新角色奖励
    void refreshRoleAwardState();
    // 获取可领取奖励
    string getRoleAwardCanGet(int days);
    // 获取已经领取的奖励
    string getRoleAwardHaveGet(int days);
    // 判断某个奖励是否已经领取
    bool isRoleAwardHaveGet(int days, int itemid);
    // 设置可领取
    void setRoleAwardCanGet(int days, const string& canGets);
    // 设置不可领取
    void setRoleAwardHaveGet(int days, const string& haveGets);
    // 发放指定id角色奖励
    void ackRoleAwardGet(int days);
    // 发送角色奖励状态
    void ackRoleAwardState();
private:
    RoleAwardHash mRoleAwardHash;
    Role* mRole;
};
#endif /* defined(__GameSrv__RoleAward__) */
