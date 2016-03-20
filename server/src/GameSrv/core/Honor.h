//
//  Honor.h
//  GameSrv
//
//  Created by jin on 13-9-27.
//
//

#ifndef __GameSrv__Honor__
#define __GameSrv__Honor__

#include "RedisDhash.h"
#include "Defines.h"
class Role;
class HonorDef;

// key达到的数量函数定义
typedef int (*KeyNumCallback)(Role *role, HonorDef *def, string param);
// key逻辑函数定义()
typedef void (*KeyCondCallback)(eHonorKey key, Role *role, string cond, string param);
// key执行的应用逻辑
typedef void (*KeyAppCallback)(eHonorApp key, Role *role, HonorDef *def);

class HonorCb
{
public:
	HonorCb()
	{
		ncb = NULL;
		ccb = NULL;
	}
	KeyNumCallback ncb;
	KeyCondCallback ccb;
};


class HonorMou
{
public:
	HonorMou();
    ~HonorMou(){}
	// 称号处理
	void procHonor(eHonorKey key, Role *role, vector<int> &vec);
	void procHonor(eHonorKey key, int roleid, string cond="", string param="");
	void procHonor(eHonorKey key, Role *role, string cond="", string param="", int procTime=0);

	// 取称号达到的数量值
	int handleNum(eHonorKey key, Role *role, HonorDef *def, string param="");
	
	// 称号逻辑应用功能处理
	void procAppHonor(eHonorApp key, Role *role);
private:
	typedef std::map<eHonorKey, HonorCb> KeyTd;
	typedef std::map<eHonorApp, KeyAppCallback> AppTd;
	
	// 条件回调是否存在
	bool isCondCb(eHonorKey key);
	void addCallback(eHonorKey key, KeyNumCallback ncb, KeyCondCallback ccb);
	void handleCond(eHonorKey key, Role *role, string cond, string param);
	// 应用逻辑函数
	void addAppCallback(eHonorApp key, KeyAppCallback acb);
	void handleApp(eHonorApp key, Role *role, HonorDef *def);
	
	KeyTd mKeyMap;
	AppTd mAppMap;
};
#define SHonorMou Singleton<HonorMou>::Instance()

// 称号
class Honor
{
public:
	Honor();
	Honor(int id, int endtime) : mId(id), mReaded(0), mIsExpiration(0), mEndTime(endtime) {}
	~Honor(){}
	
	// ID
	int mId;
	// 是否读过(新状态)
	int mReaded;
	// 是否过期
	int mIsExpiration;
	// 截至时间
	int mEndTime;
	
	void load(const char* str);
    string str() const;
	
	bool isLimited() {return mEndTime == 0 ? false : true;}
	void setExpiration() {mIsExpiration = 1;}
	void setReaded() {mReaded = 1;}
	bool isExpiration() {return mIsExpiration == 1 ? true : false;}
	// 获取剩余时间
	int getRemainingTime();
};

// 条件值
class HonorCond
{
public:
	HonorCond();
	~HonorCond(){}
	
	int value;
	
	void addNum(int num) {
		value += num;
	}
	
	void clearNum() {
		value = 0;
	}
	
	void load(const char* str);
    string str() const;
};


typedef RedisDhash<int, Honor, rnHonor> RoleHonor;
typedef RedisDhash<string, HonorCond, rnHonorCond> RoleHonorCond;
class HonorMgr
{
public:
	HonorMgr();
	void load(Role *role);
	
	void online()
	{
		offlineHonorAction();
	}
	
	// 验证称号存在并且未过期
	bool isHonor(int honorId);
	
	// 使用某称号, 同时取消称号新状态
	bool useHonor(int honorId, int &code);
	bool cancelHonor();
	
	// 取消息称号新状态
	void cancelReaded(int honorId);
	
	// 添加称号
	void addHonor(int id, int effectiveTime, int procTime);
	
	//　获取达到条件值数量
	int getHonorCondValue(eHonorKey key, string cond);
	int getHonorCondValue(string key);

	// 累计添加条件值
	void addHonorCond(eHonorKey key, string cond, int num);

	// 根据类型,　获取已得到的称号
	void outHonorByType(eHonorType type, vector<Honor*> &vec);
	
	// 奖励副本声望
	static void rewardCopyPrestige(int roleId, int copyId);
	// 添加声望
	static void addPrestige(int roleId, int val);
	bool addPrestige(int val);
	
	// 计算属性加成
	void CalcProperty(BattleProp& batprop, BaseProp& baseprop);
	// 精力加成
	int CalHeraldryFatAddition();
	// 处理离线动作的内容
	void offlineHonorAction();

	// 过期检查
	void expirationCheck();
	
	void setIsTalk(bool val) {mIsTalk = val;}
	int getTalkHonor();
	
	Honor * getHonor(int id);
private:

	// 发送称号消息
	void sendHonorNotify(int id, int limited, int remainingTime);
	// 发送过期称号消息
	void sendExpirationHonorNotify(int id);	
	bool useHonor(int honorId);
	//　清空该称号的条件值
	void removeHonorCond(int honorId);
	
	void addHonorCond(string key, int num);
	
	// 使用称号后的处理方法
	void useHonorLater();
	
	// 说话是否使用前辍
	bool mIsTalk;

	Role *mRole;
	RoleHonor mHonor;
	RoleHonorCond mHonorCond;
};

#endif /* defined(__GameSrv__Honor__) */
