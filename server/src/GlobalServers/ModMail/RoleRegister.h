
#pragma once

#include "../Interface/ShellHandle.h"
#include "../CustomDefine/PublicMessage.h"


struct MapWStringComp
{
	bool operator()(const std::wstring& _X, const std::wstring& _Y) const
	{
		if(_X.c_str() == NULL || _Y.c_str() == NULL)
			return false;
		return (wcsicmp(_X.c_str() , _Y.c_str()) < 0);
	}
};

struct MapStringComp
{
	bool operator()(const std::string& _X, const std::string& _Y) const
	{
		if(_X.c_str() == NULL || _Y.c_str() == NULL)
			return false;
		return (strcmp(_X.c_str() , _Y.c_str()) < 0);
	}
};


// 角色位置管理器
class RoleList : public CriticalObject
{
public:
	RoleList();
	~RoleList();

public:
	// 查找玩家所在的位置，如果没有位置，就分配一个人数最少的位置
	void CreateRole(const char* playerName);
	bool FindRole(const char* playerName);

	// 删除索引
	int DeleteRole(const char* playerName);

	// 删除过期邮件
	int CheckDeleteRubish();

	// 载入位置信息
	void LoadRoleList(IGlobal* pGame);

	// 添加位置
	void AddRole(const char* playerName);

	// 重名
	bool Rename(const char* playerName, const char* newName);

private:
	std::map<string, int, MapStringComp> m_roleMap; //wstring //MapWStringComp
};

extern RoleList g_RoleList;
extern int g_poolId;

