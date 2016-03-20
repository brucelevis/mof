//
//  RoleAward.cpp
//  GameSrv
//
//  Created by wind on 15-1-6.
//
//

#include "RoleLoginDaysAward.h"
#include "GameLog.h"
#include "json/json.h"
#include "Game.h"
#include "DBRolePropName.h"
#include "main.h"
#include "RedisKeyName.h"
#include "Role.h"
#include "EnumDef.h"
RoleLoginDaysAwardMgr::RoleLoginDaysAwardMgr()
{
    
}
RoleLoginDaysAwardMgr::~RoleLoginDaysAwardMgr()
{
    
}
//
void RoleLoginDaysAwardMgr::load(Role* role)
{
    mRole = role;
    mRoleAwardHash.load(g_GetRedisName(rnRoleLoginDaysAward), role->getInstID());
}
// 刷新角色奖励的状态
void RoleLoginDaysAwardMgr::refreshLoginDays()
{
    if(mRole == NULL)
        return;
    
//    tm pretm;
//    tm nowtm;
//    long preLogin = mRole->getLastLogintm();
//    long nowLogin = Game::tick;
//    localtime_r(&preLogin, &pretm);
//    localtime_r(&nowLogin, &nowtm);
//    
//    if(nowtm.tm_year != pretm.tm_year
//       && nowtm.tm_mon != pretm.tm_mon
//       && nowtm.tm_mday != pretm.tm_mday)
//    {
//        int logindays = mRole->getLoginDays() + 1;
//        mRole->setLoginDays(logindays);
//        mRole->saveProp(GetRolePropName(eRoleLoginDays), Utils::itoa(logindays).c_str());
//        
//    }
    
    int logindays = mRole->getLoginDays() + 1;
    mRole->setLoginDays(logindays);
    mRole->saveProp(GetRolePropName(eRoleLoginDays), Utils::itoa(logindays).c_str());
    
    // 刷新角色奖励的状态
    refreshRoleAwardState();
    
}

// 根据登陆天数刷新角色奖励
void RoleLoginDaysAwardMgr::refreshRoleAwardState()
{    
    RoleAwardCfgMgr::Iterator itr;
    int logindays = mRole->getLoginDays();
    int viplvl = mRole->getVipLvl();
    for (itr = RoleAwardCfgMgr::Begin(); itr != RoleAwardCfgMgr::End(); ++itr)
    {
        RoleAwardCfgDef* def = (RoleAwardCfgDef*)(*itr);
        if(def == NULL)
            continue;
        if(def->getDays() > logindays)
            continue;
        string str = "";
        for (int a = 0;  a < def->items.size(); ++a)
        {
            RoleAwardItemDef* item = def->items[a];
            if(item == NULL)
                continue;
            if(isRoleAwardHaveGet(def->getCfgid(), def->items[a]->itemid))
                continue;
            if(viplvl < def->items[a]->viplv)
                continue;
            if(a > 0)
                str.append(",");
            str.append(Utils::itoa(item->itemid));
        }
        setRoleAwardCanGet(def->getCfgid(), str);
    }
    
    mRoleAwardHash.save();
}

// 获取可领取奖励
string RoleLoginDaysAwardMgr::getRoleAwardCanGet(int days)
{

    if(!mRoleAwardHash.exist(days))
    {
        xyerr("RoleAward not have %d", days);
        return "";
    }
    Json::Value& value = mRoleAwardHash[days];
    string str = xyJsonAsString(value["cangetlist"]);
    return str;
}
// 获取已经领取的奖励
string RoleLoginDaysAwardMgr::getRoleAwardHaveGet(int days)
{
    if(!mRoleAwardHash.exist(days))
    {
        xyerr("RoleAward not have %d", days);
        return "";
    }
    Json::Value&  value = mRoleAwardHash[days];
    string str = xyJsonAsString(value["havegetlist"]);
    return str;
}
// 判断某个奖励是否已经领取
bool RoleLoginDaysAwardMgr::isRoleAwardHaveGet(int days, int itemid)
{
    string str = getRoleAwardHaveGet(days);
    
    StringTokenizer tokens(str, ",");
    for (int i = 0; i < tokens.count(); ++i)
    {
        if(itemid == Utils::safe_atoi(tokens[i].c_str()))
        {
            return true;
        }
    }

    return false;
}
// 设置可领取
void RoleLoginDaysAwardMgr::setRoleAwardCanGet(int days, const string& canGets)
{
    if(!mRoleAwardHash.exist(days))
    {
        Json::Value value;
        mRoleAwardHash[days] = value;
    }
    Json::Value& value = mRoleAwardHash[days];
    value["cangetlist"] = canGets;
}
// 设置不可领取
void RoleLoginDaysAwardMgr::setRoleAwardHaveGet(int days, const string& haveGets)
{
    if(!mRoleAwardHash.exist(days))
    {
        Json::Value value;
        mRoleAwardHash[days] = value;
    }
    Json::Value& value = mRoleAwardHash[days];
    string str = xyJsonAsString(value["havegetlist"]);
    str.append(",");
    str.append(haveGets);
    value["havegetlist"] = str;
}

// 发放指定id角色奖励
void RoleLoginDaysAwardMgr::ackRoleAwardGet(int days)
{
    ack_role_award ack;
    ack.errorcode = CE_OK;
    if(!mRoleAwardHash.exist(days))
    {
        ack.errorcode = CE_ROLE_AWARD_NOT_EXIST;
        sendNetPacket(mRole->getSessionId(), &ack);
        return;
    }
    
    Json::Value& value = mRoleAwardHash[days];
    string awardids = xyJsonAsString(value["cangetlist"]);
    StringTokenizer tokens(awardids, ",");
    
    vector<string> awards;
    awards.clear();
    RoleAwardCfgDef* def = RoleAwardCfgMgr::getRoleAwardCfgDefByid(days);
    if(def == NULL)
    {
        ack.errorcode = CE_CAN_NOT_FIND_CFG;
        sendNetPacket(mRole->getSessionId(), &ack);
        return;
    }
    
    for (int i = 0; i < tokens.count(); ++i)
    {
        RoleAwardItemDef* item = def->getRoleAwardItemDefByid(Utils::safe_atoi(tokens[i].c_str()));
        if(item)
            awards.push_back(item->item);
    }
    
    if(awards.size() == 0)
    {
        ack.errorcode = CE_ROLE_AWARD_NOT_MATCH;
    }
    else
    {
        ItemArray itemArray;
        if(!mRole->addAwards(awards, itemArray, "角色奖励"))
        {
            ack.errorcode = CE_BAG_CAPACITY_LESS_THAN_N_CELL;
            sendNetPacket(mRole->getSessionId(), &ack);
            return;
        }
        
        setRoleAwardHaveGet(days, awardids);
        setRoleAwardCanGet(days, "");
        mRoleAwardHash.save();
    }
    
    sendNetPacket(mRole->getSessionId(), &ack);
}
// 发送角色奖励状态
void RoleLoginDaysAwardMgr::ackRoleAwardState()
{
    ack_role_award_state ack;
    ack.errorcode = CE_OK;
    ack.canget.clear();
    ack.alreadyget.clear();
    ack.days = 0;
    string cangets = "";
    string havegets = "";
    int days = 0;
    int i = 0;
    RoleAwardHash::iterator itr = mRoleAwardHash.begin();
    for (; itr != mRoleAwardHash.end(); ++itr)
    {
        
        RoleAwardCfgDef* def = RoleAwardCfgMgr::getRoleAwardCfgDefByid(itr->first);
        if(def == NULL)
            continue;
        Json::Value &value = itr->second;
        cangets = xyJsonAsString(value["cangetlist"]);
        havegets = xyJsonAsString(value["havegetlist"]);
        
        if(!cangets.empty())
        {
            if(def->days > days)
            {
                days = def->days;
                ack.days = def->getCfgid();
            }
        }
        if(i++ > 0)
        {
            ack.alreadyget.append(",");
            ack.canget.append(",");
        }
        ack.canget.append(cangets);
        ack.alreadyget.append(havegets);
    }
    
    sendNetPacket(mRole->getSessionId(), &ack);
}