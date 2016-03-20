//
//  TowerDefense.cpp
//  GameSrv
//
//  Created by pireszhi on 13-8-20.
//
//

#include "TowerDefense.h"
#include "Activity.h"
#include "mail_imp.h"
#include "DBRolePropName.h"
#include "flopcard.h"
#include "GameLog.h"
#include "CopyExtraAward.h"
#include "Robot.h"
#include "Role.h"

TowerDefense::TowerDefense()
{
    m_bBroadCastInOut = false;
    m_bSyncMove = false;
}
bool TowerDefense::IsOpen()
{
    return SActMgr.IsActive(ae_tower_defense);
}

void TowerDefense::AddFriendsInAct(int playerid, int friendid)
{
    for (int i = 0; i < m_RolesDataInAct.size(); i++) {
        if (m_RolesDataInAct[i].roleid == playerid) {
            m_RolesDataInAct[i].m_friendsInAct.insert(friendid);
            return;
        }
    }
    
    RoleTowerDefenseData data;
    data.roleid = playerid;
    data.m_friendsInAct.insert(friendid);
    m_RolesDataInAct.push_back(data);
}

void TowerDefense::AddPlayerResult(int playerid, int points, RewardStruct& reward)
{
    for (int i = 0; i < m_RolesDataInAct.size(); i++) {
        if (m_RolesDataInAct[i].roleid == playerid) {
            m_RolesDataInAct[i].m_RolePoint += points;
            m_RolesDataInAct[i].m_waves++;
            m_RolesDataInAct[i].m_reward += reward;
            return;
        }
    }
    
    RoleTowerDefenseData data;
    data.roleid = playerid;
    data.m_RolePoint = points;
    data.m_waves++;
    data.m_reward += reward;
    m_RolesDataInAct.push_back(data);
}

void TowerDefense::sendAward(Role* role)
{
    for (int i = 0; i < m_RolesDataInAct.size(); i++) {
        if (m_RolesDataInAct[i].roleid == role->getInstID()) {
            onSendAward(role, m_RolesDataInAct[i]);
            m_RolesDataInAct.erase(m_RolesDataInAct.begin() + i);
            return;
        }
    }
}

//int TowerDefense::getTowerWaves(int playerid)
//{
//    for (int i = 0; i < m_RolesDataInAct.size(); i++) {
//        if (m_RolesDataInAct[i].roleid == playerid) {
//            return m_RolesDataInAct[i].m_waves;
//        }
//    }
//    return 0;
//}

RoleTowerDefenseData TowerDefense::getRoleData(int roleid)
{
    for (int i = 0; i < m_RolesDataInAct.size(); i++) {
        if (m_RolesDataInAct[i].roleid == roleid) {
            return m_RolesDataInAct[i];
        }
    }
    
    RoleTowerDefenseData data;
    return data;
}

void TowerDefense::onSendAward(Role* role, RoleTowerDefenseData &result)
{
    if (!result.m_waves || !result.m_RolePoint) {
        return;
    }
    
    int currSceneid = role->getCurrSceneId();
    SceneCfgDef* scenecfg = SceneCfg::getCfg(currSceneid);
    if (scenecfg == NULL || scenecfg->sceneType != stDefendStatue) {
        return;
    }
    
    int flopIndex = 0;
    vector<int> drops;
    string dropStr = "";
	int sceneId = currSceneid * 100 + result.m_waves;
    
    string awardBase = SflopcardMgr.RandomCard(sceneId, flopIndex, drops, dropStr, 0);
	string awardResult = awardBase;
	
	SCopyExtraAward.extraItemsAward(role->getInstID(), sceneId, awardBase, awardResult);
    
    //send act award mail to player
    float friendcoef = FriendAwardCoefCfg::getFriendsCountCoef(result.m_friendsInAct.size());
    int IntimacySum = 0;

    //add by wangzhigang 2014/5/28
    for (std::set<int>::iterator iter = result.m_friendsInAct.begin(); iter != result.m_friendsInAct.end(); iter++) {
        IntimacySum += role->getFriendMgr()->getFrinedIntimacy(*iter);
    }
    
    float friendIntimacyCoef = FriendAwardCoefCfg::getFriendsIntimacyCoef(IntimacySum);
    
    RewardStruct reward;
    ItemArray items;
    
    vector<string> itemstrs = StrSpilt(awardResult, ";");
    rewardsCmds2ItemArray(itemstrs,items,reward);
    
    reward = result.m_reward;
    
    reward *= (1 + friendcoef + friendIntimacyCoef);

    vector<string> attachs;
    
    MailFormat *f = MailCfg::getCfg("tower_defense");
    if (f)
    {
        std::vector<string> awardsDesc;
        
        reward2MailFormat(reward, items, &awardsDesc, &attachs);
        
        string mail_content = f->content;
        string friendsize_str = Utils::itoa(result.m_friendsInAct.size());
        
        string attach_desc = StrJoin(awardsDesc.begin(), awardsDesc.end(), ",");
        string attach = StrJoin(attachs.begin(), attachs.end(), ";");
        
        //感谢你拼死保卫神像，特奖励：{:str:}同时参与本次战斗的好友数为：%d。此活动期间参与好友数越多，你的奖励越丰富。
        if (3 != find_and_replace(mail_content, 3, scenecfg->name.c_str(), attach_desc.c_str(), friendsize_str.c_str()))
        {
            log_warn("mail content format error.[tower_defense]");
        }
        
        bool ret = sendMail(0,
                            f->sendername.c_str(),
                            role->getRolename().c_str(),
                            f->title.c_str(),
                            mail_content.c_str(),
                            attach.c_str(),
                            Utils::itoa(role->getInstID()).c_str());
        
        if( false ==  ret ){
            StoreOfflineItem(role->getInstID(),attach.c_str());
        }
        
        LogMod::addLogSendRoleMail(role->getInstID(), f->title.c_str(), attach.c_str(), ret);
        
        LogMod::addLogTowerDefenseAward(role, result.m_waves, result.m_friendsInAct.size(), IntimacySum, attach, scenecfg->sceneId);
    }
    else
    {
        log_warn("[tower_defense] not found.");
    }
}

void TowerDefense::OnUpdate(float dt)
{

}

void TowerDefense::OnHeartBeat()
{
    if (!getObjCount()) {
        Destroy();
    }
}

void TowerDefense::OnUpdate(Obj* obj, OBJ_INFO& info, float dt)
{
    
}

void TowerDefense::OnHeartBeat(Obj* obj, OBJ_INFO& info)
{
    
}

void TowerDefense::OnEnterRoom(Obj* obj)
{
    
}

void TowerDefense::OnLeaveRoom(Obj* obj)
{
    if (obj->getType() == eRole) {
        Role* role = (Role*)obj;
//        if (role->getIncrTowerdefenseTimes() > 0)
//        {
//            role->setIncrTowerdefenseTimes(role->getIncrTowerdefenseTimes() - 1);
//            role->saveNewProperty();
//        }
//        else
//        {
//            int currentimes = role->getTowerDefenseTimes();
//            role->setTowerDefenseTimes(currentimes + 1);
//            role->saveProp(GetRolePropName(eRoleTowerDefenseTimes), role->getTowerDefenseTimes());
//        }
//        sendAward(role);
    }
}

void TowerDefense::Enter(Obj* obj)
{
    Add(obj, 0, 0, false);
    Role* role = (Role*)obj;
    
    std::vector<int> friendlist;
    role->getFriendMgr()->getOnlineFriends(friendlist);
    
    for (int i = 0 ; i < friendlist.size(); i++) {
        if (friendlist[i] == RobotCfg::sFriendId) {
        //if (getObjectType(friendlist[i]) != kObjectRole) {
            continue;
        }
        int friendId = getObjectLowId(friendlist[i]);
        Role* friendonline = SRoleMgr.GetRole(friendId);
        
        if (friendonline == NULL) {
            continue;
        }
        
        TowerDefense* friendroom = (TowerDefense*)ActRoomMGR.find("TowerDefense", friendonline);
        if (friendroom) {
            friendroom->AddFriendsInAct(friendonline->getInstID(), role->getInstID());
            AddFriendsInAct(role->getInstID(), friendonline->getInstID());
        }
    }
    
}

void TowerDefense::Leave(Obj *obj)
{
    
}
