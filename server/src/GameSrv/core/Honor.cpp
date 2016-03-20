//
//  Honor.cpp
//  GameSrv
//
//  Created by jin on 13-9-27.
//
//

#include "Honor.h"
#include "main.h"
#include "Utils.h"
#include "GameLog.h"
#include "DBRolePropName.h"
#include "json/json.h"
//#include "hander_include.h"
#include "mail_imp.h"
#include "DataCfgActivity.h"
#include "GameLog.h"
#include "GuildRole.h"
#include "Constellation.h"

#include "Game.h"
#include "BroadcastNotify.h"
#include "Role.h"
#include "MysteriousMgr.h"

#define HONOR_COND_DATA_SEP "_"

INSTANTIATE_SINGLETON(HonorMou);


int getHonorCondValue(Role *role, HonorDef *def)
{
	int num = 0;
	std::set<string>::iterator it;
	for (it = def->condSet.begin(); it != def->condSet.end(); it++) {
		num += role->getHonorMgr()->getHonorCondValue(def->key, *it);
	}
	return num;
}

// =======================计算条件值回调===========================
static int HonorNum_Gold(Role *role, HonorDef *def, string param)
{
	return role->getGold();
}

static int HonorNum_Lvl(Role *role, HonorDef *def, string param)
{
	return role->getLvl();
}

static int HonorNum_Kill(Role *role, HonorDef *def, string param)
{
	return getHonorCondValue(role, def);
}

static int HonorNum_EquipQua(Role *role, HonorDef *def, string param)
{
	return role->getPlayerEquip()->getQuality();;
}

static int HonorNum_EquipLvl(Role *role, HonorDef *def, string param)
{
	return role->getPlayerEquip()->getDeepen();
}

static int HonorNum_EquipStone(Role *role, HonorDef *def, string param)
{
	return role->getPlayerEquip()->GetStoneNumByLvl(Utils::safe_atoi(def->cond.c_str()));
}

static int HonorNum_Cons(Role *role, HonorDef *def, string param)
{
	ConstellationMgr *mgr = role->getConstellationMgr();
	ConstellData *data = mgr->getConstell(Utils::safe_atoi(def->cond.c_str()));
	if (NULL == data) {
		return 0;
	}
	return data->step;
}

static int HonorNum_Power(Role *role, HonorDef *def, string param)
{
	return role->getBattleForce();
}

static int HonorNum_Copy(Role *role, HonorDef *def, string param)
{
	return getHonorCondValue(role, def);
}


static int HonorNum_Ill(Role *role, HonorDef *def, string param)
{
	int type = Utils::safe_atoi(def->cond.c_str());
	IllustrationsMgr *mgr = role->getIllustrationsMgr();
	bool bet = false;
	if (type > 0) {
		bet = mgr->checkComIllustrations((IllustrationsType)type);
	} else {
		bet = mgr->checkComIllustrations();
	}
	return bet ? 1 : 0;
}

static int HonorNum_Constrib(Role *role, HonorDef *def, string param)
{
	RoleGuildProperty& property = SRoleGuild(role->getInstID());
	return ceil(property.getConstrib());
}

static int HonorNum_Exploit(Role *role, HonorDef *def, string param)
{
	RoleGuildProperty& property = SRoleGuild(role->getInstID());
	return ceil(property.getExploit());
}

static int HonorNum_Prestige(Role *role, HonorDef *def, string param)
{
	return role->getPrestige();
}

static int HonorNum_BossRankOne(Role *role, HonorDef *def, string param)
{
	return role->getHonorMgr()->getHonorCondValue(def->key, "");
}

static int HonorNum_BossLast(Role *role, HonorDef *def, string param)
{
	return role->getHonorMgr()->getHonorCondValue(def->key, "");
}

static int HonorNum_PvpRankOne(Role *role, HonorDef *def, string param)
{
	return role->getHonorMgr()->getHonorCondValue(def->key, "");
}

static int HonorNum_PvpKill(Role *role, HonorDef *def, string param)
{
	return role->getHonorMgr()->getHonorCondValue(def->key, "");
}

static int HonorNum_Gambler(Role *role, HonorDef *def, string param)
{
	return role->getHonorMgr()->getHonorCondValue(def->key, "");
}

static int HonorNum_PetPvpRankOne(Role *role, HonorDef *def, string param)
{
	return getHonorCondValue(role, def);
}

static int HonorNum_PetPvpKill(Role *role, HonorDef *def, string param)
{
	return role->getHonorMgr()->getHonorCondValue(def->key, "");
}

static int HonorNum_UseItem(Role *role, HonorDef *def, string param)
{
	return getHonorCondValue(role, def);
}

static int HonorNum_GuildMasterTreasurecopy(Role *role, HonorDef *def, string param)
{
	return getHonorCondValue(role, def);
}

static int HonorNum_GuildTreasurecopy(Role *role, HonorDef *def, string param)
{
	return getHonorCondValue(role, def);
}

static int HonorNum_SynPvpRank(Role *role, HonorDef *def, string param)
{
	return getHonorCondValue(role, def);
}
// ========================需要做记录值回调==========================


static void HonorarCond_Kill(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, Utils::safe_atoi(param.c_str()));
}

static void HonorarCond_Copy(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, 1);
}

static void HonorarCond_PvpKill(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, Utils::safe_atoi(param.c_str()));
}

static void HonorarCond_Gambler(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, 1);
}

static void HonorarCond_PvpRankOne(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, 1);
}

static void HonorarCond_BossLast(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, 1);
}

static void HonorarCond_BossRankOne(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, 1);
}

static void HonorarCond_PetPvpKill(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, Utils::safe_atoi(param.c_str()));
}

static void HonorarCond_PetPvpRankOne(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, 1);
}

static void HonorarCond_UseItem(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, 1);
}

static void HonorarCond_GuildMasterTreasurecopy(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, 1);
}

static void HonorarCond_GuildTreasurecopy(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, 1);
}

static void HonorarCond_SynPvpRank(eHonorKey key, Role *role, string cond, string param)
{
	role->getHonorMgr()->addHonorCond(key, cond, 1);
}

/*
 ========================有应用功能的称号回调==========================
 HonorMou::procAppHonor 函数处理会处理到这
 */

static void HonorarApp_WorldTalk(eHonorApp key, Role *role, HonorDef *def)
{
	role->getHonorMgr()->setIsTalk(true);
}

static void HonorarApp_OnlineTips(eHonorApp key, Role *role, HonorDef *def)
{
	SystemBroadcast bc;
	string str = SystemBroadcastCfg::getValue("honor_online", "content");
    bc << str.c_str() << def->name.c_str() << role->getRolename().c_str();
    bc.send(1);
}


HonorMou::HonorMou()
{
	addCallback(eHonorKey_Gold, HonorNum_Gold, NULL);
	addCallback(eHonorKey_Lvl, HonorNum_Lvl, NULL);
	addCallback(eHonorKey_Kill, HonorNum_Kill, HonorarCond_Kill);
	addCallback(eHonorKey_EquipQua, HonorNum_EquipQua, NULL);
	addCallback(eHonorKey_EquipLvl, HonorNum_EquipLvl, NULL);
	addCallback(eHonorKey_EquipStone, HonorNum_EquipStone, NULL);
	addCallback(eHonorKey_Cons, HonorNum_Cons, NULL);
	addCallback(eHonorKey_Power, HonorNum_Power, NULL);
	addCallback(eHonorKey_Copy, HonorNum_Copy, HonorarCond_Copy);
	addCallback(eHonorKey_Ill, HonorNum_Ill, NULL);
	addCallback(eHonorKey_Constrib, HonorNum_Constrib, NULL);
	addCallback(eHonorKey_Exploit, HonorNum_Exploit, NULL);
	addCallback(eHonorKey_Prestige, HonorNum_Prestige, NULL);
	addCallback(eHonorKey_BossRankOne, HonorNum_BossRankOne, HonorarCond_BossRankOne);
	addCallback(eHonorKey_BossLast, HonorNum_BossLast, HonorarCond_BossLast);
	addCallback(eHonorKey_PvpRankOne, HonorNum_PvpRankOne, HonorarCond_PvpRankOne);
	addCallback(eHonorKey_PvpKill, HonorNum_PvpKill, HonorarCond_PvpKill);
	addCallback(eHonorKey_Gambler, HonorNum_Gambler, HonorarCond_Gambler);
	addCallback(eHonorKey_PetPvpRankOne, HonorNum_PetPvpRankOne, HonorarCond_PetPvpRankOne);
	addCallback(eHonorKey_PetPvpKill, HonorNum_PetPvpKill, HonorarCond_PetPvpKill);
	addCallback(eHonorKey_UseItem, HonorNum_UseItem, HonorarCond_UseItem);
	addCallback(eHonorKey_GuildMasterTreasurecopy, HonorNum_GuildMasterTreasurecopy, HonorarCond_GuildMasterTreasurecopy);
	addCallback(eHonorKey_GuildTreasurecopy, HonorNum_GuildTreasurecopy, HonorarCond_GuildTreasurecopy);
	addCallback(eHonorKey_SynPvpRank, HonorNum_SynPvpRank, HonorarCond_SynPvpRank);
	
	
	addAppCallback(eHonorApp_WorldTalk, HonorarApp_WorldTalk);
	addAppCallback(eHonorApp_OnlineTips, HonorarApp_OnlineTips);
}

void HonorMou::addCallback(eHonorKey key, KeyNumCallback ncb, KeyCondCallback ccb)
{
	assert(ncb);
	
	KeyTd::iterator it = mKeyMap.find(key);
	if (it != mKeyMap.end()) {
		xyerr("[称号][key函数] eHonorKey = %d 重复添加回调函数", key);
        return;
	}
	HonorCb cb;
	cb.ncb = ncb;
	cb.ccb = ccb;
	
	mKeyMap.insert(make_pair(key, cb));
}

void HonorMou::addAppCallback(eHonorApp key, KeyAppCallback acb)
{
	assert(acb);
	
	AppTd::iterator it = mAppMap.find(key);
	if (it != mAppMap.end()) {
		xyerr("[称号][key函数] eHonorApp = %d 重复添加回调函数", key);
        return;
	}
	
	mAppMap.insert(make_pair(key, acb));
}


int HonorMou::handleNum(eHonorKey key, Role *role, HonorDef *def, string param)
{
	KeyTd::iterator it = mKeyMap.find(key);
	if (it == mKeyMap.end()) {
		xyerr("[称号][key函数] eHonorKey = %d 不存在", key);
        return 0;
	}
	return it->second.ncb(role, def, param);
}

void HonorMou::handleCond(eHonorKey key, Role *role, string cond, string param)
{
	KeyTd::iterator it = mKeyMap.find(key);
	if (it == mKeyMap.end()) {
		xyerr("[称号][key函数] eHonorKey = %d 不存在", key);
        return;
	}
	// 允许空函数,　不作处理
	if (NULL == it->second.ccb) {
		return;
	}
	it->second.ccb(key, role, cond, param);
}

void HonorMou::handleApp(eHonorApp key, Role *role, HonorDef *def)
{
	AppTd::iterator it = mAppMap.find(key);
	if (it == mAppMap.end()) {
		xyerr("[称号][key函数] eHonorApp = %d 不存在", key);
        return;
	}
	// 允许空函数,　不作处理
	if (NULL == it->second) {
		return;
	}
	it->second(key, role, def);
}

bool HonorMou::isCondCb(eHonorKey key)
{
	KeyTd::iterator it = mKeyMap.find(key);
	if (it == mKeyMap.end()) {
		return false;
	}
	return it->second.ccb != NULL;
}

void HonorMou::procHonor(eHonorKey key, Role *role, vector<int> &vec)
{
	vector<int>::iterator it;
	for (it = vec.begin(); it != vec.end(); it++) {
		if (*it <= 0) {
			continue;
		}
		this->procHonor(key, role, xystr("%d", *it), "1");
	}
}

void HonorMou::procHonor(eHonorKey key, int roleId, string cond, string param)
{
	Role *role = SRoleMgr.GetRole(roleId);
	if (NULL == role) {
		// 离线动作
		NewRoleProperty property;
		property.load(roleId);
		RoleHonorAction &honorAction = property.getHonorAction();
		honorAction.add(key, cond, param);
		property.save();
		// 写动作存储日志
		Xylog log(eLogName_HonorAction, roleId);
		log<<key<<cond<<param;
		
		return;
	}
	this->procHonor(key, role, cond, param);
}

void HonorMou::procHonor(eHonorKey key, Role *role, string cond, string param, int procTime)
{
	if (NULL == role) {
		xyerr("[称号]　procHonor　函数　出现role为NULL");
		return;
	}
	
	if (NULL == role->getHonorMgr()) {
		xyerr("[称号]　procHonor　函数　出现称号管理类为NULL");
		return;
	}
	
	// 检查处理时间是否有传递,　因为有离线处理,　离线有自己的逻辑存储时间
	if (procTime == 0) {
		procTime = time(NULL);
	}
	
	// 日志记录
	Xylog log(eLogName_HonorProcLog, role->getInstID());
	log << key << cond << param << procTime;
	
	// 从系统ini得到符合称号key的称号
	vector<HonorDef*> vec;
	HonorCfgMgr::getHonor(key, cond, vec);
	
	// 称号key添加条件记录值
	handleCond(key, role, cond, param);

	// 处理称号
	vector<HonorDef*>::iterator it;
	for (it = vec.begin(); it != vec.end(); it++) {
		HonorDef *def = *it;
		// 已获得, 并且没有时间限制
		if (role->getHonorMgr()->isHonor(def->id) && def->effectiveTime == 0) {
			continue;
		} 
		// 得到数量值
		int num = handleNum(key, role, def, param);
		if (def->needNum <= num) {
			role->getHonorMgr()->addHonor(def->id, def->effectiveTime, procTime);
		}
	}
}

void HonorMou::procAppHonor(eHonorApp key, Role *role)
{
	if (NULL == role) {
		xyerr("[称号]　procHonor　函数　出现role为NULL");
		return;
	}
	if (NULL == role->getHonorMgr()) {
		xyerr("[称号]　procHonor　函数　出现称号管理类为NULL");
		return;
	}
	
	vector<HonorDef*> vec;
	HonorCfgMgr::getHonorApp(key, vec);
	
	vector<HonorDef*>::iterator it;
	for (it = vec.begin(); it != vec.end(); it++) {
		// 是当前使用称号
		if (role->getUseHonor() != (*it)->id) {
			continue;
		}
		this->handleApp(key, role, *it);
	}
}

Honor::Honor()
{
	mId = 0;
	mReaded = 0;
}

string Honor::str() const
{
    Json::Value value;
    value["id"] = mId;
    value["readed"] = mReaded;
	value["endtime"] = mEndTime;
	value["isexpiration"] = mIsExpiration;
    return Json::FastWriter().write(value);
}

// 获取剩余时间
int Honor::getRemainingTime()
{
	if (mEndTime == 0) {
		return 0;
	}
	// 过期了,直接返回
	if (isExpiration()) {
		return 0;
	}
	return mEndTime - Game::tick;
}

void Honor::load(const char* str)
{
    if(NULL == str) return;
    
    Json::Value retval;
	if (!xyJsonSafeParse(str, retval)) {
		xyerr("Honorary::load , %s ",str);
        return;
	}
		
    mId = xyJsonAsInt(retval["id"]);
    mReaded = xyJsonAsInt(retval["readed"]);
	mEndTime = xyJsonAsInt(retval["endtime"]);
	mIsExpiration = xyJsonAsInt(retval["isexpiration"]);
}

std::ostream& operator<<(std::ostream& out, const Honor& obj) 
{
    return out << obj.str();
}

std::istream& operator>>(std::istream& in, Honor& obj)
{
    string tmp;
    in>>tmp;
    obj.load(tmp.c_str());
    return in;
}


HonorCond::HonorCond()
{
	value = 0;
}

string HonorCond::str() const
{
    Json::Value jsonValue;
    jsonValue["num"] = value;
    return Json::FastWriter().write(jsonValue);
}

void HonorCond::load(const char* str)
{
    if(NULL == str) return;
    
    Json::Reader reader;
    Json::Value retval;
    
    if (!reader.parse(str, retval)) {
        xyerr("HonoraryCond::load , %s ",str);
        return;
    }
	
    if (retval["num"].type() == Json::intValue) {
		value = retval["num"].asInt();
	}
}

std::ostream& operator<<(std::ostream& out, const HonorCond& obj)
{
    return out << obj.str();
}

std::istream& operator>>(std::istream& in, HonorCond& obj)
{
    string tmp;
    in>>tmp;
    obj.load(tmp.c_str());
    return in;
}


HonorMgr::HonorMgr()
{
	mRole = NULL;
	mIsTalk = false;
}

void HonorMgr::load(Role *role)
{
	mRole = role;
	int roleid = mRole->getInstID();
	
	mHonor.load(roleid);
	mHonorCond.load(roleid);
}

//　过期检查
void HonorMgr::expirationCheck()
{
	RoleHonor::iterator it;
	for (it = mHonor.begin(); it != mHonor.end(); it++) {
		// 永久有效的
		if (it->second.mEndTime == 0) {
			continue;
		}
		// 已过期
		if (it->second.isExpiration()) {
			continue;
		}
		// 未过期
		if (Game::tick < it->second.mEndTime) {
			continue;
		}
		// 过期了,　打标记
		it->second.setExpiration();
		it->second.setReaded();
		mHonor.set(it->first, it->second);
		
		// 移除称号的条件值
		this->removeHonorCond(it->second.mId);
		
		// 发送过期通知
		this->sendExpirationHonorNotify(it->second.mId);
		
		// 检查当前称号是否在使用,　取消掉
		if (mRole->getUseHonor() == it->second.mId) {
			this->cancelHonor();
			// 重算属性加成
			mRole->CalcPlayerProp();
		}
	}
}

//　清空该称号的条件值
void HonorMgr::removeHonorCond(int honorId)
{
	HonorDef &def = HonorCfgMgr::getHonor(honorId);
	if (def.id == 0) {
		xyerr("数据库存有配置表没有配置的称号id");
		return;
	}
	RoleHonorCond::reverse_iterator it;
	for (it = mHonorCond.rbegin(); it != mHonorCond.rend(); it++) {
		StringTokenizer st(it->first, HONOR_COND_DATA_SEP);
		if (st.count() == 0) {
			continue;
		}
		if (Utils::safe_atoi(st[0].c_str()) != def.key) {
			continue;
		}
		// 清空并存储
		it->second.clearNum();
		mHonorCond.set(it->first, it->second);
	}
}

bool HonorMgr::isHonor(int honorId)
{
	Honor *h = this->getHonor(honorId);
	if (NULL == h) {
		return false;
	}
	if (h->isExpiration()) {
		return false;
	}
	return true;
}

bool HonorMgr::useHonor(int honorId, int &code)
{
	Honor *h = this->getHonor(honorId);
	if (NULL == h) {
		code = enUseHonroReuslt_NoGet;
		return false;
	}
	// 取消称号新状态
	this->cancelReaded(honorId);
	
	// 检查是否过期
	if (h->isExpiration()) {
		code = enUseHonroReuslt_Expire;
		return false;
	}
	
	// 使用称号
	if (!this->useHonor(honorId)) {
		return false;
	}
	
	// 用了称号后的处理
	this->useHonorLater();
	return true;
}



bool HonorMgr::useHonor(int honorId)
{
	mRole->setUseHonor(honorId);
	mRole->saveNewProperty();
	return true;
}

bool HonorMgr::cancelHonor()
{
	mRole->setUseHonor(0);
	mRole->saveNewProperty();
	return true;
}

void HonorMgr::cancelReaded(int honorId)
{
	Honor *h = this->getHonor(honorId);
	if (NULL == h) {
		return;
	}
	// 已是取消新状态的, 跳出
	if (h->mReaded == 1) {
		return;
	}
	h->setReaded();
	mHonor.set(honorId, *h);
}

void HonorMgr::addHonor(int id, int effectiveTime, int procTime)
{
	// 若有持续时间,　则计算过期时间存储
	int endtime = effectiveTime > 0 ? procTime + effectiveTime : 0;
	
	// 结束时间已是过期, 移掉条件值
	if (endtime != 0 && endtime < time(NULL)) {
		this->removeHonorCond(id);
		
		Xylog log(eLogName_HonorOfflineEffectiveTimeoutLog, mRole->getInstID());
		log << id << effectiveTime << procTime << endtime;
		return;
	}
	
	Honor tmp(id, endtime);
	mHonor.set(id, tmp);
	
	// 写获得称号日志
	Xylog log(eLogName_HonorLog, mRole->getInstID());
	log << tmp.mId << tmp.mEndTime;
	
	// 发送称号获得消息
	sendHonorNotify(tmp.mId, tmp.isLimited(), tmp.getRemainingTime() + 5);
}

void HonorMgr::rewardCopyPrestige(int roleId, int copyId)
{
	SceneCfgDef *scenecfg = SceneCfg::getCfg(copyId);
    if (NULL == scenecfg) {
        return;
    }
	if (scenecfg->sceneType != stTeamCopy && scenecfg->sceneType != stFriendDunge) {
		return;
	}
    int incr = scenecfg->incr_prestige;
	HonorMgr::addPrestige(roleId, incr); 
}

void HonorMgr::addPrestige(int roleId, int val)
{
	Role *role = SRoleMgr.GetRole(roleId);
	if (NULL != role) {
		role->getHonorMgr()->addPrestige(val);
	} else {
		// 离线处理
		NewRoleProperty property;
		property.load(roleId);
		property.setPrestige(property.getPrestige() + val);
		property.save();
		
		// 称号检查
		SHonorMou.procHonor(eHonorKey_Prestige, roleId);
	}
}

bool HonorMgr::addPrestige(int val)
{
	if (mRole->getPrestige() + val < 0) {
		return false;
	}
	mRole->setPrestige(mRole->getPrestige() + val);
	mRole->saveNewProperty();
	
	// 发送同步消息
	notify_edit_prestige notify;
    notify.prestige = mRole->getPrestige();
    sendNetPacket(mRole->getSessionId(), &notify);

	// 称号检查
	SHonorMou.procHonor(eHonorKey_Prestige, mRole);
	
	return true;
}

void HonorMgr::outHonorByType(eHonorType type, vector<Honor*> &vec)
{
	RoleHonor::iterator it;
	for (it = mHonor.begin(); it != mHonor.end(); it++) {
		HonorDef &def = HonorCfgMgr::getHonor(it->second.mId);
		if (def.type == type) {
			vec.push_back(&it->second);
		}
	}
}

int HonorMgr::getHonorCondValue(eHonorKey key, string cond)
{
	stringstream ss;
	ss<<key<<HONOR_COND_DATA_SEP<<cond;
	return getHonorCondValue(ss.str());
}

int HonorMgr::getHonorCondValue(string key)
{
	RoleHonorCond::iterator it = mHonorCond.find(key);
	if (it != mHonorCond.end()) {
		return it->second.value;
	}
	return 0;
}

void HonorMgr::sendHonorNotify(int id, int limited, int remainingTime)
{
	notify_honor_get notify;
	notify.honorId = id;
	notify.isLimited = limited;
	notify.remainingTime = remainingTime;
	sendNetPacket(mRole->getSessionId(), &notify);
}

void HonorMgr::sendExpirationHonorNotify(int id)
{
	notify_honor_expiration notify;
	notify.honorId = id;
	sendNetPacket(mRole->getSessionId(), &notify);
}

void HonorMgr::addHonorCond(eHonorKey key, string cond, int num)
{
	stringstream ss;
	ss<<key<<HONOR_COND_DATA_SEP<<cond;
	addHonorCond(ss.str(), num);
}

void HonorMgr::addHonorCond(string key, int num)
{
	if (num <= 0) {
		return;
	}
	
	RoleHonorCond::iterator it = mHonorCond.find(key);
	if (it == mHonorCond.end()) {
		HonorCond tmp;
		tmp.value = num;
		mHonorCond.set(key, tmp);
	} else {
		it->second.addNum(num);
		mHonorCond.set(key, it->second);
	}
}

Honor * HonorMgr::getHonor(int id)
{
	RoleHonor::iterator it = mHonor.find(id);
	if (it != mHonor.end()) {
		return &it->second;
	}
	return NULL;
}


void HonorMgr::offlineHonorAction()
{
	RoleHonorAction &honorAction = mRole->getHonorAction();
	RoleHonorAction::Iterator it;
	for (it = honorAction.begind(); it != honorAction.end(); it++) {
		SHonorMou.procHonor((eHonorKey)it->key, mRole, it->cond, it->param);
	}
	honorAction.clear();
	mRole->saveNewProperty();
}


void HonorMgr::CalcProperty(BattleProp &batprop, BaseProp &baseprop)
{
	if (NULL == mRole) {
		return;
	}
	
	int honorId = mRole->getUseHonor();
	HonorDef &def = HonorCfgMgr::getHonor(honorId);
	if (!def.isAtt) {
		return;
	}
	
	baseprop.mStre += def.getStre();
	baseprop.mInte += def.getInte();
	baseprop.mPhys += def.getPhys();
	baseprop.mCapa += def.getCapa();
	
	batprop.mAtk += def.getAtk();
	batprop.mDef += def.getDef();
	batprop.mMaxHp += def.getAddMaxHp();
	batprop.mHit += def.getHit();
	batprop.mDodge += def.getDodge();
	batprop.mCri += def.getCri();
}


int HonorMgr::CalHeraldryFatAddition()
{
	int honorId = mRole->getUseHonor();
	HonorDef &def = HonorCfgMgr::getHonor(honorId);
	return def.getAddMaxFat();
}


int HonorMgr::getTalkHonor()
{
	return mIsTalk ? mRole->getUseHonor() : 0;
}

void HonorMgr::useHonorLater()
{
	mIsTalk = false;
}