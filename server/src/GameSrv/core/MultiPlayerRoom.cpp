//
//  MultiPlayerRoom.cpp
//  GameSrv
//
//  Created by tfl on 12-12-28.
//
//

#include "MultiPlayerRoom.h"
#include "Role.h"

/*
// 活动场景同步
// 服务器提醒角色已经进入场景
begin_msg(notify_enter_act, 25, 1)
    def_string(actname) //活动名称
    def_object_arr(obj_roleinfo, roleinfos) // 已经进入活动场景的角色列表
    def_object_arr(obj_posiinfo, posiinfos) // 每个对象的位置信息(坐标，方向)
end_msg()

// 服务器通知有玩家加入活动场景
begin_msg(notify_act_add_player, 25, 2)
    def_object(obj_roleinfo, roleinfo) // 进入场景的角色对象
    def_object(obj_posiinfo, posiinfo) // 对象的位置信息
end_msg()

// 服务器通知玩家离开了活动场景
begin_msg(notify_act_remove_player, 25, 3) //有角色离开了活动房间
    def_int(roleid)
end_msg()

// 服务器通知同步角色移动
begin_msg(notify_act_player_move,25, 4)
    def_object(obj_posiinfo, posiinfo)
end_msg()

// 发送角色同步消息
begin_msg(req_act_player_move, 25,5)
    def_object(obj_posiinfo, posiinfo)
end_msg()

// 角色同步消息回应, 当服务器认为客户端的移动有错误时发送
// 将角色拖回服务器上的位置
begin_msg(respon_act_player_move, 25, 6) //
    def_int(errorcode)
    def_object(obj_posiinfo, posiinfo)
end_msg()
*/

ActSceneManager ActRoomMGR;

// 多人场景基础类，提供角色同步机制（移动，技能）
BaseActRoom::BaseActRoom()
{
    m_bSyncMove = true;
    m_bBroadCastInOut = true;
}

BaseActRoom::~BaseActRoom()
{
    ClearAll();
}

bool BaseActRoom::Add(Obj* obj, float x, float y, bool autoDelete)
{
    if (!obj) return false;

    Role* role = (obj->getType() == eRole) ? (Role*)obj : NULL;

    // 玩家已经在房间内
    if (role && role->getCurrentActRoomName().size() > 0)
    {
        return false;
    }

    // 将玩家添加到容器
    for (size_t i=0; i < m_objs.size(); i++)
    {
        // 对象已经存在
        if (m_objs[i].first == obj)
        {
            return false;
        }
    }

    // 对象坐标信息
    OBJ_INFO info;
    info.posi.x = x;
    info.posi.y = y;
    info.posi.speed = 0.0f;
    info.posi.direction = 0.0f;
    info.posi.roleid = role ? role->getInstID() : 0;  // 怪物暂时不分配id,分配规则暂无
    info.autoDelete = autoDelete;

    m_objs.push_back( std::make_pair(obj,  info) );

    OnEnterRoom( obj );

    // 发消息通知
    // 通知角色自己
    // 暂时只处理角色事件， 对象的同步机制仍取决于对象id的规划机制， 分配合理的对象id, 能轻易与角色的id区分开
    if (obj->getType() == eRole)
    {
        // 角色记录活动名称
        role->setCurrentActRoomName(m_className);
        role->setCurrentActRoom(this);

        // 通知该玩家
        notify_enter_act notice_enter;
        notice_enter.actname = m_className;

        for (size_t i=0;i<m_objs.size() && m_bBroadCastInOut;i++)
        {
            // 加入广播列表
            if (m_objs[i].first && m_objs[i].first->getType() == eRole)
            {
                Role *otherrole = (Role*)m_objs[i].first;
                // 填充roleInfo
                obj_roleinfo info;                
                otherrole->getRoleInfo(info);
            
                notice_enter.roleinfos.push_back(info);

                // posiinfo
                notice_enter.posiinfos.push_back(m_objs[i].second.posi);
            }
        }

        // 通知
        sendNetPacket(role->getSessionId(), &notice_enter);


        if(m_bBroadCastInOut){ // 广播到其它玩家?世界boss不需要
            // 广播到其它玩家
            notify_act_add_player notice_add;

            // roleinfo
            notice_add.roleinfo.roleid = role->getInstID();
            notice_add.roleinfo.rolename = role->getRolename();
            notice_add.roleinfo.lvl = role->getLvl();
            notice_add.roleinfo.roletype = role->getRoleType();
            notice_add.roleinfo.viplvl = role->getVipLvl();

            // posiinfo
            notice_add.posiinfo = info.posi;

            // 广播
            BroadCast(&notice_add, obj);
        }
    }

    return true;
}

bool BaseActRoom::Remove(Obj* obj)
{
    if (!obj) return false;

    for (size_t i=0;i<m_objs.size();i++)
    {
        if (m_objs[i].first == obj)
        {
            OnLeaveRoom( obj );

            // 发消息通知
            if (obj->getType() == eRole)
            {
                Role* role = (Role*)obj;

                // 清除
                role->setCurrentActRoomName("");
                role->setCurrentActRoom(NULL);

                notify_act_remove_player notice;
                notice.roleid = role->getInstID();

                //世界boss不需要广播
                if( m_bBroadCastInOut)
                    BroadCast(&notice);
                // 通知
                sendNetPacket(role->getSessionId(), &notice);
            }

            // 删除这个对象
            if (m_objs[i].second.autoDelete)
            {
                delete m_objs[i].first;
            }

            m_objs.erase( m_objs.begin() + i);

            return true;
        }
    }

    return false;
}

int BaseActRoom::FindObj(Obj* obj)
{
    if (!obj) return false;
    for (size_t i=0;i<m_objs.size();i++)
    {
        if (m_objs[i].first == obj) return i;
    }
    return -1;
}

void BaseActRoom::ClearAll()
{
    for (size_t i=0;i<m_objs.size();i++)
    {
        if (m_objs[i].first && m_objs[i].first->getType() == eRole)
        {
            Role* r = (Role*)m_objs[i].first;
            r->setCurrentActRoomName("");
            r->setCurrentActRoom(NULL);
        }

        // 删除这个对象
        if (m_objs[i].second.autoDelete)
        {
            delete m_objs[i].first;
        }
    }

    m_objs.clear();
}

void BaseActRoom::BroadCast(INetPacket* packet, Obj* except)
{
    if (!packet) return;

    vector<int> sessions;
    for (size_t i=0;i<m_objs.size();i++)
    {
        // 过滤掉指定对象
        if (m_objs[i].first == except) continue;

        // 加入广播列表
        if (m_objs[i].first && m_objs[i].first->getType() == eRole)
        {
            Role* pRole = (Role*)m_objs[i].first;
            sessions.push_back(pRole->getSessionId());
        }
    }

    multicastPacket(sessions, packet);
}

void BaseActRoom::SyncMove(int index, Role* role, req_act_player_move& req)
{
    // 暂时不做验证， 直接覆盖玩家移动参数，并进行广播
    if (index < 0 || index >= m_objs.size()) return;
    if (m_objs[index].first != role) return;

    // 直接赋值
    m_objs[index].second.posi = req.posiinfo;

    // 广播
    notify_act_player_move notice;
    notice.posiinfo = req.posiinfo;
    BroadCast(&notice, role);
}

void BaseActRoom::_Update(float dt)
{
    for (size_t i=0; i < m_objs.size(); i++)
    {
        OnUpdate(m_objs[i].first, m_objs[i].second, dt);
    }
}

void BaseActRoom::_OnHeartBeat()
{
    for (size_t i=0; i < m_objs.size(); i++)
    {
        OnHeartBeat(m_objs[i].first, m_objs[i].second);
    }
}

// class ActSceneManager
//////////////////////////////////////
ActSceneManager::ActSceneManager()
{

}

ActSceneManager::~ActSceneManager()
{
    for (size_t i=0; i < m_Rooms.size(); i++)
    {
        if (m_Rooms[i]) delete m_Rooms[i];
    }
    m_Rooms.clear();
};

// 注册场景
void ActSceneManager::Reg(BaseActRoom* room)
{
    if (!room)
    {
        log_info("ActSceneManager::Register, null object\n");
    }

    room->m_bValid = true;

    // 重用原来的空间
    for (size_t i=0; i < m_Rooms.size(); i++)
    {
        if (!m_Rooms[i])
        {
            m_Rooms[i] = room;
            return;
        }
    }

    m_Rooms.push_back( room );

    //printf("%s\n", room->m_className.c_str());
};

// 更新各场景
void ActSceneManager::Update(float dt)
{
    static float counter = 0.0f;
    counter += dt;
    for (size_t i=0; i < m_Rooms.size(); i++)
    {
        if (!m_Rooms[i]) continue;

        // 无效的场景，直接删除
        if (!m_Rooms[i]->m_bValid)
        {
//            printf("remove Room: %s\n", m_Rooms[i]->m_className.c_str());
            delete m_Rooms[i];
            m_Rooms[i] = NULL;
            continue;
        }

        // 执行心跳和帧
        if (counter > 1.0f)
        {
            m_Rooms[i]->OnHeartBeat();
            m_Rooms[i]->_OnHeartBeat();
        }

        m_Rooms[i]->OnUpdate(dt);
        m_Rooms[i]->_Update(dt);
    }

    if (counter > 1.0f)
    {
        counter = 0.0f;
    }
}

BaseActRoom* ActSceneManager::find(const char* className)
{
    if (!className) return NULL;

    for (size_t i=0; i < m_Rooms.size(); i++)
    {
        if (m_Rooms[i]
            && strcmp(m_Rooms[i]->m_className.c_str(), className) == 0)
        {
            return m_Rooms[i];
        }
    }
    return NULL;
}

bool ActSceneManager::find(vector<BaseActRoom*>& result, const char* className)
{
    if (!className) return false;

    result.clear();

    for (size_t i=0; i < m_Rooms.size(); i++)
    {
        if (m_Rooms[i]
            && strcmp(m_Rooms[i]->m_className.c_str(), className) == 0)
        {
            result.push_back(m_Rooms[i]);
        }
    }

    return result.size() > 0;
}

BaseActRoom* ActSceneManager::find(const char* className, Obj* obj, int* indexFind)
{
    if (!className) return NULL;
    if (!obj) return NULL;                                                 

    for (size_t i=0; i < m_Rooms.size(); i++)
    {
        if (m_Rooms[i]
            && strcmp(m_Rooms[i]->m_className.c_str(), className) == 0)
        {
            int nFind = m_Rooms[i]->FindObj(obj);
            if (nFind != -1)
            {
                if (indexFind) { *indexFind = nFind; }
                return m_Rooms[i];
            }
        }
    }

    return NULL;
}

void ActSceneManager::RemovePlayer(Role* role)
{
    BaseActRoom* room = find(role->getCurrentActRoomName().c_str(), role);
    if (room)
    {
        room->Remove(role);
    }
}

void ActSceneManager::SyncMove(Role* role, req_act_player_move& req)
{
    if (!role) return;

    int index = -1;
    BaseActRoom* room = find(role->getCurrentActRoomName().c_str(), role, &index);
    if (room)
    {
        room->SyncMove(index, role, req);
    }
}
