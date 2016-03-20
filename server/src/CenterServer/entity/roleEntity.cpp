#include "roleEntity.h"
#include "json/json.h"
#include "dbMgr.h"
#include "serverEntity.h"

string 
Role::encode() {
	Json::Value value;
	value["lvl"] = getLvl();
	value["roletype"] = getRoleType();
	value["roleid"] = getRoleId();
	value["rolename"] = getRoleName();
	value["bat"] = getBat();
	return Json::FastWriter().write(value);
}
	
bool 
Role::decode(const char* str) {
	Json::Value value;
	Json::Reader reader;
	reader.parse(str, value);
	if (!value.isObject()) {
		return false;
	}

	setLvl(value["lvl"].asInt());
	setRoleType(value["roletype"].asInt());
	setRoleId(value["roleid"].asInt());
	setBat(value["bat"].asInt());
	setRoleName(value["rolename"].asString());

	return true;
}

void
RoleMgr::load() {
	RedisResult result(redisCmd("hgetall server:%d:role", mServer->getServerId()));
	int count = result.getHashElements();
	for (int i = 0; i < count; i += 2) {
		int roleId = result.readHash(i, 0);
		if (roleId == 0) {
			continue;
		} 
		string roleSave = result.readHash(i + 1);
		Role* role = new Role;
		role->decode(roleSave.c_str());
		addRole(role);
	}
}

Role*
RoleMgr::getRole(int roleId) {
	map<int, Role*>::iterator iter = mRoles.find(roleId);
	if (iter == mRoles.end()) {
		return NULL;
	}
	return iter->second;
}

bool
RoleMgr::addRole(Role* role) {
	if (getRole(role->getRoleId()) != NULL) {
		return false;
	}
	mRoles[role->getRoleId()] = role;
	string roleSave = role->encode();
	doRedisCmd("hmset server:%d:role %d %s", mServer->getServerId(), role->getRoleId(), roleSave.c_str());
	return true;
}

void
RoleMgr::delRole(int roleId) {
	Role* role = getRole(roleId);
	if (role == NULL) {
		return;
	}
	mRoles.erase(roleId);
	delete role;
}