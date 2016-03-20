//
//  Totem.h
//  GameSrv
//
//  Created by jin on 14-5-19.
//
//

#ifndef __GameSrv__Totem__
#define __GameSrv__Totem__

#include <iostream>
#include <string.h>
#include <map>
#include <vector>
#include "Defines.h"
#include "cyclemsg.h"

using namespace std;

class Role;
class Totem;

class TotemMon
{
public:
	// 可选择的宠物合法性检查
	eTotemImmolation petCheck(Role *role, vector<int> &pets);
	// 宠物符合图腾献祭条件检查
	eTotemImmolation totemCheck(const Totem &totem, Role *role, vector<int> &pets);
	
	void updateAddition(int roleId);
	BattleProp & getAddition(int roleId);
	
	void sendAllPet(Role *role);
	
	void refresh();
private:
	// 计算图腾加成
	void totemCalculate(int roleId, BattleProp &battleProp);
	
	// 角色图腾战斗力加成缓存
	map<int, BattleProp> mBatAddition;
};
#define STotemMon Singleton<TotemMon>::Instance()


class Totem
{
	READONLY(int, mId, Id);
	READONLY(int, mLvl, Lvl);
	READONLY(int, mNum, Num);
	
public:
	Totem() {
		mId = 0;
		mLvl = 0;
		mNum = 0;
	}
	Totem(int id) : mId(id) , mLvl(0), mNum(0) {}
	
	bool addTotemNum(int num);
	
	// 更新图腾数据,计算升级,升级后数量恢复0
	bool updateUp();

	inline bool load(const char *str);
	inline string format() const;
private:
	bool totemLvlUp();
};

class TotemMgr
{
public:
	typedef map<int, Totem>::const_iterator Iterator;
	TotemMgr(){}
	void init(int roleId)
	{
		mRoleId = roleId;
	}
	static TotemMgr* create(int roleId)
	{
		TotemMgr *totem = new TotemMgr();
        totem->init(roleId);
        return totem;
	}
	void load();

public:
	// 添加图腾数量
	bool addTotem(int totemId, int num);
	
	// 计算属性加成
	void calculateProperty(BattleProp& batprop);
	
	const Totem & getTotemConst(int totemId);
	Iterator begin() const {return mTotem.begin();}
	Iterator end() const {return mTotem.end();}
private:
	// 检查配置表改动,更新旧的图腾数据,计算是否升级
	void updateOldData();
	
	Totem & getTotem(int totemId);
	void save(const Totem& totem);
	
	int mRoleId;
	map<int, Totem> mTotem;
};

#endif /* defined(__GameSrv__Totem__) */
