//
//  RoleMgr.h
//  GameSrv
//
//  Created by xinyou on 14-5-5.
//
//

#ifndef __GameSrv__RoleMgr__
#define __GameSrv__RoleMgr__

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "basedef.h"
using namespace std;

class Role;
class RoleMgr;
class INetPacket;


enum DestroyRoleType
{
    eLogout,
    eClientCheat,
    eGMKick,
    eLoginOtherPlace,
    eVersionError,
    eOtherError,
};

//return false to interupt the traverse.
typedef bool (*TraverseRoleCb)(Role* role, void* param);

class RoleMgr
{
private:
    std::map<int, Role*> _Roles;
    std::map<int, Role*> session2role;
    std::map<string, Role*> mRoleNameIdx;
public:
    RoleMgr(){}
	typedef map<int, Role*>::iterator Iterator;
	Iterator begin() {return _Roles.begin();}
	Iterator end() {return _Roles.end();}
    
    Role* GetRole(int roleid);
    Role* GetRole(const char* rolename);
    Role* getRoleBySessionId(int sessionId);
    void GetRolesId(std::vector<int>& rolesId);
    void updateAll(int nbeat);
    void traverseRoles(TraverseCallback callback, void* param);
    void delRoleNameIdx(const string& roleName);
    void addRoleNameIdx(const string& name, Role* role);
    
    void sendPacketToRole(int roleid, INetPacket* packet);
    
    void sendPacketToRoles(vector<int>& rolesid, INetPacket* packet)
    {
        for (int i = 0; i < rolesid.size(); i++)
        {
            sendPacketToRole(rolesid[i], packet);
        }
    }
    
    int getRoleNum()
    {
        return _Roles.size();
    }
    
    Role* createRole(int roleid);
    void mapRoleSession(int roleid, int sessionid);
    int Session2RoleId(int sid);
    int RoleId2Session(int roleid);
    
    void destroyRole(Role* role, DestroyRoleType def=eLogout);
    void disconnect(Role* role, DestroyRoleType def=eOtherError);
    
    void saveRoleData(int tick); //定时回写全部玩家有修改的部分数据
    
    
    void gmAddTop50Person(int roleID, int num = 50);
    void safe_gmAddTop50Person(int roleID, int num = 50);
};

#define SRoleMgr Singleton<RoleMgr>::Instance()



#endif /* defined(__GameSrv__RoleMgr__) */
