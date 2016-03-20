//
//  MysticalCopy.h
//  GameSrv
//
//  Created by pireszhi on 13-9-4.
//
//

#ifndef __GameSrv__MysticalCopy__
#define __GameSrv__MysticalCopy__

#include <iostream>
#include "Obj.h"
#include "MultiPlayerRoom.h"
#include <set>
#include <map>
#include <stdio.h>

struct MysticalResult
{
    MysticalResult()
    {
        isPass = false;
        bossid = battleTime = 0;
        m_killMonsters.clear();
    }
    
    bool isPass;
    int bossid;
    int battleTime;
    std::vector<int> m_killMonsters;
};

class MysticalCopy: public BaseActRoom
{
    CloneAble(MysticalCopy)
public:
    MysticalCopy()
    {
        m_Copyid = 0;
        m_Monsters.clear();
        
        m_bBroadCastInOut = false;
        m_bSyncMove = false;
    };
    bool IsOpen();
    void setCopyId(int copyid);
    void CreateMonsters(int copyid, int times);
    //百分百出现隐藏 BOSS 与哥布林
    void CreateMonsters(int copyid);

    const std::vector<MysticalMonsterDef>& getMonsters();
    const MysticalMonsterDef& getBoss();
    bool getBattleResult(int roleid, MysticalResult& outdata);
    bool setBattleResult(int roleid, MysticalResult indata);
    bool setBattleKillBoss(int roleid, int bossid);
    bool checkMonsterExist(int monsterid);
    bool checkBossExist(int bossid);
protected:
    virtual void OnHeartBeat();// 场景心跳
    virtual void OnHeartBeat(Obj* obj, OBJ_INFO& info);// 对象心跳
    virtual void OnEnterRoom(Obj* obj);// 有对象加入场景
    virtual void OnLeaveRoom(Obj* obj);// 对象离开
private:
    int m_Copyid;
    std::vector<MysticalMonsterDef> m_Monsters;
    MysticalMonsterDef m_Boss;
    
    std::map<int, MysticalResult> m_RolesResult;
};

#endif /* defined(__GameSrv__MysticalCopy__) */
