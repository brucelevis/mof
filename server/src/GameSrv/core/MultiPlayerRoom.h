//
//  MultiPlayerRoom.h
//  GameSrv
//
//  Created by tfl on 12-12-28.
//
//

#ifndef __MULTI_PLAYER_ROOM__
#define __MULTI_PLAYER_ROOM__

#include "Defines.h"
#include "DataCfg.h"

// 宏，用于场景创建自身实例
#define CloneAble(name) \
public: static name* Create()\
{\
    name* o = new name;\
    o->m_className = #name;\
    ActRoomMGR.Reg(o);\
    return o;\
}\

class Obj;
class Role;

// 对象坐标信息
class OBJ_INFO
{
public:
    obj_posiinfo posi;
    bool autoDelete;
};

// 多人场景基础类，提供角色同步机制（移动，技能）
class BaseActRoom
{
    friend class ActSceneManager;
public:
    BaseActRoom();
    virtual ~BaseActRoom();

public:
    bool Add(Obj* obj, float x, float y, bool autoDelete);
    bool Remove(Obj* obj);
    int FindObj(Obj* obj);
    void ClearAll();
    void BroadCast(INetPacket* packet, Obj* except = NULL);
    void Destroy(){ m_bValid = false; }
    const char* GetClassName(){return m_className.c_str();}
    void SyncMove(int index, Role* role, req_act_player_move& req);
    int getObjCount(){return m_objs.size();}
protected:
    virtual void OnUpdate(float dt){}; // 场景帧
    virtual void OnHeartBeat(){};// 场景心跳
    virtual void OnUpdate(Obj* obj, OBJ_INFO& info, float dt){};// 对象帧
    virtual void OnHeartBeat(Obj* obj, OBJ_INFO& info){};// 对象心跳
    virtual void OnEnterRoom(Obj* obj){};// 有对象加入场景
    virtual void OnLeaveRoom(Obj* obj){};// 对象离开

protected:
    // 执行子对象的心跳和帧
    void _Update(float dt);
    void _OnHeartBeat();

protected:
    vector< std::pair<Obj*, OBJ_INFO> >m_objs;  // 角色与其他物体
    string m_className; // 类名
    bool m_bValid;// 是否有效
    bool m_bSyncMove;// 是否同步角色坐标信息
    bool m_bBroadCastInOut;// 是或同步玩家进出
};

// 房间管理器
// 提供一个总的入口
class ActSceneManager
{
public:
    ActSceneManager();
    ~ActSceneManager();

public:
    // 注册场景
    void Reg(BaseActRoom* room);

    // 更新各场景
    void Update(float dt);

    // 查找第一个
    BaseActRoom* find(const char* className);

    // 查找
    bool find(vector<BaseActRoom*>& result, const char* className);

    // 查找
    BaseActRoom* find(const char* className, Obj* obj, int* indexFind = NULL);

    // 查找并删除玩家
    void RemovePlayer(Role* role);

    // 同步移动信息
    void SyncMove(Role* role, req_act_player_move& req);

private:
    vector<BaseActRoom*> m_Rooms;
};

extern ActSceneManager ActRoomMGR;

#endif /* defined(__MULTI_PLAYER_ROOM__) */

