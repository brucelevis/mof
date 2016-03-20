//
//  TowerDefense.h
//  GameSrv
//
//  Created by pireszhi on 13-8-20.
//
//

#ifndef __GameSrv__TowerDefense__
#define __GameSrv__TowerDefense__
#include "Obj.h"
#include "MultiPlayerRoom.h"
#include "ItemHelper.h"
#include <iostream>
#include <set>
#include <map>
#include <stdio.h>

typedef std::set<int> FriendCount;

struct RoleTowerDefenseData
{
    RoleTowerDefenseData()
    {
        roleid = m_waves = m_RolePoint = 0;
        m_friendsInAct.clear();
    }
    
    int roleid;
    int m_RolePoint;
    int m_waves;
    std::set<int> m_friendsInAct;
    
    RewardStruct m_reward;
};

class TowerDefense: public BaseActRoom
{
    CloneAble(TowerDefense)
public:
    TowerDefense();
    bool IsOpen();
    
    //玩家进入/离开/怪物的进入不走这个
    void Enter(Obj* obj);
    void Leave(Obj* obj);
    void AddPlayerResult(int playerid, int points, RewardStruct& reward);
    void AddFriendsInAct(int playerid, int friendid);
//    int getTowerWaves(int playerid);
    RoleTowerDefenseData getRoleData(int roleid);
    void sendAward(Role* role);
    void onSendAward(Role* role, RoleTowerDefenseData& result);
protected:
    virtual void OnUpdate(float dt); // 场景帧
    virtual void OnHeartBeat();// 场景心跳
    virtual void OnUpdate(Obj* obj, OBJ_INFO& info, float dt);// 对象帧
    virtual void OnHeartBeat(Obj* obj, OBJ_INFO& info);// 对象心跳
    virtual void OnEnterRoom(Obj* obj);// 有对象加入场景
    virtual void OnLeaveRoom(Obj* obj);// 对象离开
private:
    vector<RoleTowerDefenseData> m_RolesDataInAct;
};

#endif /* defined(__GameSrv__TowerDefense__) */

