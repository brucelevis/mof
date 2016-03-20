#pragma once

#include <stdint.h>

typedef int64_t PvpRoleId;

inline PvpRoleId genPvpRoleId(int serverId, int roleId)
{
	return (((int64_t) serverId) << 32) + roleId;
}

inline int getPvpRoleServerId(PvpRoleId pvpRoleId)
{
	return (pvpRoleId >> 32);
}

inline int getPvpRoleRoleId(PvpRoleId pvpRoleId)
{
	return (pvpRoleId & 0xFFFFFFFF);
}
