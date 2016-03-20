#pragma once

#include <map>
#include "Defines.h"
using namespace std;

class ServerEntity;

class Role
{
public:
	READWRITE(int, mRoleId, RoleId)
	READWRITE(string, mRoleName, RoleName)
	READWRITE(int, mBat, Bat)
	READWRITE(int, mLvl, Lvl)
	READWRITE(int, mRoleType, RoleType)

	string encode();
	bool decode(const char* str);
};


class RoleMgr
{
public:
	RoleMgr(ServerEntity* server) {
		mServer = server;
	}

	void load();

	Role* getRole(int roleId);
	bool addRole(Role* role);
	void delRole(int roleId);

	map<int, Role*> mRoles;

	ServerEntity* mServer;
};