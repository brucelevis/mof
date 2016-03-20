//
//  RoleMgr.cpp
//  GameSrv
//
//  Created by xinyou on 14-5-5.
//
//

#include "RoleMgr.h"
#include "Role.h"
#include "INetPacket.h"
#include "Pet.h"
#include "PetPvp.h"
#include "GameLog.h"
#include "cmd_def.h"
#include "TreasureFight.h"
#include "SyncTeamCopy.h"
#include "Retinue.h"
#include "cs_pvp/cs_pvp_battle.h"
#include "centerClient.h"

INSTANTIATE_SINGLETON(RoleMgr)

extern void broadcastSceneDisplayedPets(Scene* scene, int roleId);
extern void broadcastSceneDisplayRetinues(Scene* scene, int roleId);

Role* RoleMgr::createRole(int roleid)
{
    Role* role = Role::create(roleid, false);
    if (role)
    {
        //role->load();
        
        _Roles.insert(make_pair<int, Role*>(roleid, role));
        mRoleNameIdx.insert(make_pair<string, Role*>(role->getRolename(), role));
    }
    
    return role;
}
void RoleMgr::mapRoleSession(int roleid, int sessionid)
{
    Role* role = GetRole(roleid);
    session2role[sessionid] = role;
    if (role)
    {
        role->setSessionId(sessionid);
    }
}


void RoleMgr::disconnect(Role* role, DestroyRoleType def)
{
    assert(role);
    
    create_cmd(kickSession, kickSession);
    kickSession->sid = role->getSessionId();
    sendMessageToGate(0, CMD_MSG, kickSession, 0);
    
    destroyRole(role, def);
}


void RoleMgr::destroyRole(Role* role, DestroyRoleType def)
{
    role->tryLeaveWorld();
    
    int roleid = role->getInstID();
    int sessionid = role->getSessionId();
    
    role->saveNewProperty();
    
    //增加离开代码
    GlobalRoleOffline(roleid);
    
    //pvp处理
    role->stopPvp();
    
    SPetPvpMgr.safe_UpdatePlayerData(role, false);
    
    SPetPvpMgr.safe_endPlayerSearchEnemy(role);
    
    //离开场景
    Scene* roleScene = role->getScene();
    if (roleScene)
    {
        Pet* pet = role->mPetMgr->getActivePet();
        if (pet) {
            int displayPetNum = GlobalCfg::sCityPetNum;
            int rank = roleScene->removePet(pet);
            if (rank > 0 && rank <= displayPetNum) {
                broadcastSceneDisplayedPets(roleScene, role->getInstID());
            }
        }
        
        Retinue *retinue = role -> mRetinueMgr -> getActiveRetinue();
        if (retinue) {
            int displayRetinueNum = GlobalCfg::sCityRetinueNum;
            int rank = roleScene -> removeRetinue(retinue);
            if (rank > 0 && rank <= displayRetinueNum) {
                broadcastSceneDisplayRetinues(roleScene, role -> getInstID());
            }
        }
        
        notify_leave_city notify;
        notify.roleid = role->getInstID();
        notify.rolename = role->getRolename();
        roleScene->broadcast(&notify, role->getInstID());
        roleScene->removeRole(role);
        
    }
    
    if (role->getCsPvpState()->getState() != CsPvpState::kCsPvpNone) {
        req_cs_cancel_cspvp req;
        req.server_id = Process::env.getInt("server_id");
        req.role_id = role->getInstID();
        CenterClient::instance()->sendNetPacket(0, &req);
    }
    
    //通知好友
    notify_friendoffline offlinenotify;
    offlinenotify.roleid = role->getInstID();
    role->sendMessageToFriends(&offlinenotify);
    
    //离开世界多播组
    create_cmd(leaveGroup, leaveGroup);
    leaveGroup->sid = role->getSessionId();
    leaveGroup->groupid = WORLD_GROUP;
    sendMessageToGate(0, CMD_MSG, leaveGroup, 0);
    SSessionDataMgr.setState(role->getSessionId(), kSessionAuth);
    
    //上传新数据到排行榜
    //    role->CalcPlayerProp();
    
    //    Pet* pet = role->mPetMgr->getActivePet();
    //    if (pet) {
    //        role->UploadPetPaihangData(eUpdate, pet->petid, pet->mBattle);
    //    }
    
    int timeonline = time(NULL) - role->mLoginMoment;
    LogMod::addLogLogout(role->getInstID(), role->getRolename().c_str(), role->getSessionId(), timeonline, def);
    
    //退出公会宝藏战活动
    g_GuildTreasureFightMgr.playerDisconnect(role);
    
    //退出队伍
    g_SyncFightingTeamMgr.leaveTeam(role, role->getSyncTeamId());
    
    //退出组队副本的界面
    g_SyncTeamCopyMgr.leaveActivity(role->getSessionId());
    
    
    
    //回收角色资源
    string rolename = role->getRolename();
    delete role;
    
    //删除角色管理器的信息
    _Roles.erase(roleid);
    mRoleNameIdx.erase(rolename);
    session2role.erase(sessionid);
}

int RoleMgr::Session2RoleId(int sid)
{
    std::map<int, Role*>::iterator iter = session2role.find(sid);
    if (iter != session2role.end())
    {
        return iter->second->getInstID();
    }
    
    return 0;
}


int RoleMgr::RoleId2Session(int roleid)
{
    Role* role = GetRole(roleid);
    if (role)
    {
        return role->getSessionId();
    }
    
    return 0;
}

Role* RoleMgr::GetRole(int roleid)
{
    std::map<int, Role*>::iterator fit = _Roles.find(roleid);
    if (fit != _Roles.end()) {
        return fit->second;
    }
    return NULL;
}

Role* RoleMgr::GetRole(const char* rolename)
{
    std::map<string, Role*>::iterator fit = mRoleNameIdx.find(rolename);
    if (fit != mRoleNameIdx.end()) {
        return fit->second;
    }
    return NULL;
}


Role*
RoleMgr::getRoleBySessionId(int sessionId) {
    
    std::map<int, Role*>::iterator iter = session2role.find(sessionId);
    if (iter != session2role.end()) {
        return iter->second;
    }
    
    return NULL;
}

void RoleMgr::GetRolesId(std::vector<int>& rolesId)
{
    for (std::map<int, Role*>::iterator it = _Roles.begin(); it != _Roles.end(); ++it) {
        rolesId.push_back(it->first);
    }
}

void RoleMgr::updateAll(int nbeat)
{
    std::map<int, Role*>::iterator it;
    for (it = _Roles.begin(); it != _Roles.end(); it++) {
        it->second->OnHeartBeat(nbeat);
    }
    
}

void RoleMgr::traverseRoles(TraverseCallback callback, void* param)
{
    std::map<int, Role*>::iterator it;
    for (it = _Roles.begin(); it != _Roles.end(); it++)
    {
        if (!callback(it->second, param))
        {
            return;
        }
    }
}

void RoleMgr::sendPacketToRole(int roleid, INetPacket* packet)
{
    Role* role = GetRole(roleid);
    if (role)
    {
        sendNetPacket(role->getSessionId(), packet);
    }
}

void RoleMgr::addRoleNameIdx(const string &name, Role *role)
{
    mRoleNameIdx[name] = role;
}

void RoleMgr::delRoleNameIdx(const string &roleName)
{
    mRoleNameIdx.erase(roleName);
}