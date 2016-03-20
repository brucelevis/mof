//
//  TwistEggMgr.cpp
//  GameSrv
//
//  Created by jin on 15-1-6.
//
//

#include "TwistEggMgr.h"
#include "DynamicConfig.h"
#include "Singleton.h"
#include "DataCfgActivity.h"
#include "main.h"
#include "Game.h"
#include "BroadcastNotify.h"
#include "GameLog.h"
#include "daily_schedule_system.h"
#include "EnumDef.h"
#include "Role.h"

INSTANTIATE_SINGLETON(TwistEggMgr);

void TwistEggMgr::init()
{
	this->addActivityTimer();
}

void TwistEggMgr::addActivityTimer()
{
	time_t now = time(NULL);
	
	vector<const TwistEggActivityDef*> vec;
	TwistEggCfgMgr::getNotOpenActivity(vec, now);

	for (vector<const TwistEggActivityDef*>::iterator it = vec.begin(); it != vec.end(); it++) {
		addTimer(this->OnActivityRefresh, NULL, (*it)->beginTm - now, 1);
	}
}

void TwistEggMgr::OnActivityRefresh(void *param)
{
	TwistEggMgr& mgr = Singleton<TwistEggMgr>::Instance();
	mgr.sendActivityNotify();
}

void TwistEggMgr::sendActivityNotify()
{
	notify_twistegg_activity notify;
	notify.tm = time(NULL);
	notify.index = 0;	// 客户端整个界面刷新，不用单独通哪个项
	broadcastPacket(WORLD_GROUP, &notify);
}

int TwistEggMgr::checkConsume(Role* role, const TwistEggCommonDef* def, bool isFree, bool isEven)
{
	// 检查使用免费次数的规则。(连扭不消耗免费次数, 硬性规则)
	if (!isEven && isFree) {
		RoleTwistEggDef& data = role->getTwistEgg().get(def->index);
		if (def->isTimesLimit && def->getFreeNum(data.free) <= 0) {
			return eTwisteggGetResult_FreeNumOver;
		}
		
		if (def->getFreeTime(Game::tick - data.freeTime) > 0) {
			return eTwisteggGetResult_FreeTime;
		}
		
		return eTwisteggGetResult_Ok;
	}
	
	// 检查消耗物品
	pair<unsigned int, unsigned int> needItem = def->getItemConsume(isEven);
	if (needItem.first > 0
		&& role->getBackBag()->GetItemNum(needItem.first) >= needItem.second) {
		return eTwisteggGetResult_Ok;
	}
	
	// 检查金钻
	int rmb = def->getRmbConsume(isEven);
	if (rmb > 0 && role->getRmb() >= rmb) {
		return eTwisteggGetResult_Ok;
	}
	
	// 检查金币
	int gold = def->getGoldConsume(isEven);
	if (gold > 0 && role->getGold() >= gold) {
		return eTwisteggGetResult_Ok;
	}
	
	// 若没有配任何消耗, 返回错误
	if (rmb <= 0 && gold <= 0 && def->needItemId <= 0) {
		return eTwisteggGetResult_NotConsume;
	}
	
	return eTwisteggGetResult_NeedNot;
}

int TwistEggMgr::useConsume(Role*role, const TwistEggCommonDef* def, bool isFree, bool isEven, int& consumeType)
{
	consumeType = 0;
	// 不是连扭, 用免费次数来扭的逻辑
	if (!isEven && isFree) {
		RoleTwistEggDef& data = role->getTwistEgg().get(def->index);
		data.addFree(1);
		data.setFreeTime(Game::tick);
		role->getTwistEgg().save();
		return eTwisteggGetResult_Ok;
	}
	
	// 不是连扭, 若配有物品规则, 先扣物品
	pair<unsigned int, unsigned int> needItem = def->getItemConsume(isEven);
	if (needItem.first > 0) {
		ItemArray items;
		GridArray effgrids;
		items.push_back(ItemGroup(needItem.first, needItem.second));
		if (role->getBackBag()->PreDelItems(items, effgrids))
		{
			role->playerDeleteItemsAndStore(effgrids, items, "TwistEgg", true);
			consumeType = 1;
			return eTwisteggGetResult_Ok;
		}
	}
	
	// 扣金钻
	int rmb = def->getRmbConsume(isEven);
	if (rmb > 0 && role->CheckMoneyEnough(rmb, eRmbCost, "TwistEgg") == CE_OK) {
		consumeType = 2;
		return eTwisteggGetResult_Ok;
    }

	// 扣金币
	int gold = def->getGoldConsume(isEven);
	if (gold > 0 && role->CheckMoneyEnough(gold, eGoldCost, "TwistEgg") == CE_OK) {
		consumeType = 3;
		return eTwisteggGetResult_Ok;
	}
	
	return eTwisteggGetResult_UseConsumeErr;
}

int TwistEggMgr::getTwistEgg(int index, Role* role, bool free,  bool isEven,
							 vector<const TwistEggGoodsDef*>& goods)
{
	const TwistEggCommonDef* def = TwistEggCfgMgr::getTwistEgg(index, Game::tick);
	if (NULL == def) {
		return eTwisteggGetResult_NotConfig;
	}
	RoleTwistEggDef& data = role->getTwistEgg().get(index);
	
	// 随机物品
	if (!def->randGoods(goods, data.isFirst(), isEven)) {
		return eTwisteggGetResult_NotGetGoods;
	}
	
	ItemArray items;
	ItemArray itemsTemp;
	GridArray effgrids;
	for (vector<const TwistEggGoodsDef*>::iterator it = goods.begin(); it != goods.end(); it++) {
		itemsTemp.push_back(ItemGroup((*it)->goodsId, (*it)->goodsNum));
	}
	// 必奖的物品
	if (def->rewardItem > 0) {
		itemsTemp.push_back(ItemGroup(def->rewardItem, def->getRewardItemNum(isEven)));
	}

	mergeItems(itemsTemp, items);
	// 添加到背包
	if (role->preAddItems(items, effgrids) != CE_OK){
		return eTwisteggGetResult_BagFull;
	}
	
	// 扣消耗
	int consumeType = 0;
	int state = this->useConsume(role, def, free, isEven, consumeType);
	if (state != eTwisteggGetResult_Ok) {
		return state;
	}
	
	// 不是连扭, 改掉第一次状态
	if (!isEven && data.isFirst()) {
		data.setFirst();
		role->getTwistEgg().save();
	}
	
	// 存储物品到玩家身上
	role->playerAddItemsAndStore(effgrids, items, "TwistEgg", true);
    
    int twistNum = isEven ? def->evenTwistNum : 1;
    role->getDailyScheduleSystem()->onEvent(kDailyScheduleCapsuleToy, 0, twistNum);
	
	// 检查发送跑马灯
	this->sendBroadcast(role, goods);
	
	
	Xylog log(eLogName_TwistEggGet, role->getInstID());
	log
	<< index
	<< free
	<< isEven
	<< def->activityId
	<< this->getGoodsLogStr(goods)
	<< consumeType;
	
	return eTwisteggGetResult_Ok;
}

string TwistEggMgr::getGoodsLogStr(vector<const TwistEggGoodsDef*>& goods)
{
	stringstream ss;
	for (vector<const TwistEggGoodsDef*>::iterator it = goods.begin(); it != goods.end(); it++) {
		ss << (*it)->goodsId << "," << (*it)->goodsNum << ";";
	}
	return ss.str();
}

void TwistEggMgr::sendBroadcast(Role* role, vector<const TwistEggGoodsDef*>& goods)
{
	vector<const TwistEggGoodsDef*>::iterator it;
	for (it = goods.begin(); it != goods.end(); it++) {
		if (!(*it)->broadcast) {
			continue;
		}
		
		ItemCfgDef* cfg = ItemCfg::getCfg((*it)->goodsId);
		if (NULL == cfg) {
			continue;
		}
		
		SystemBroadcast bc;
		string str = SystemBroadcastCfg::getValue("twistEgg_goodsView", "content");
		bc << str.c_str() << role->getRolename().c_str() << cfg->ReadInt("qua") << cfg->ReadStr("name").c_str();
		bc.send();
	}
}


int TwistEggMgr::checkTwist(int index, int activityId, Role* role, bool free, bool isEven)
{
	const TwistEggCommonDef* def = TwistEggCfgMgr::getTwistEgg(index, Game::tick);
	if (NULL == def) {
		return eTwisteggGetResult_NotConfig;
	}
	
	// 检查配置是否超时
	if (def->checkTimeout(Game::tick)) {
		return eTwisteggGetResult_IsTimeout;
	}
	
	// 检查客户端发过来的活动id是否一置
	if (def->activityId != activityId) {
		return eTwisteggGetResult_NotActivityId;
	}
	
	// 检查玩家vip与lvl
	if (role->getLvl() < def->lvl && role->getVipLvl() < def->viplvl) {
		return eTwisteggGetResult_LvlNot;
	}
	
	// 检查需消耗的条件
	int state = this->checkConsume(role, def, free, isEven);
	if (state != eTwisteggGetResult_Ok) {
		return state;
	}
	
	// 检查背包需要的格子数(加上必奖物品的格子)
	int needSpace = (isEven ? def->evenTwistNum : 1) + 1;
	if (role->getBackBag()->EmptyGridAmount() < needSpace) {
		return eTwisteggGetResult_NotBagSpace;
	}
	
	return eTwisteggGetResult_Ok;
}

void TwistEggMgr::getTwisteggInfo(Role* role, const TwistEggCommonDef& def, obj_twistegg_info& obj)
{
	this->getTwisteggInfo(role, def.index, obj);
}

void TwistEggMgr::getTwisteggInfo(Role* role, int index, obj_twistegg_info& obj)
{
	RoleTwistEggDef& data = role->getTwistEgg().get(index);
	const TwistEggCommonDef* def = TwistEggCfgMgr::getTwistEgg(index, Game::tick);
	obj.index = def->index;
	obj.functionSw = def->isTime;
	obj.functionTm = def->getFunctionTm(Game::tick);
	obj.isTimesLimit = def->isTimesLimit;
	obj.freeNum = def->getFreeNum(data.free);
	obj.freeTm = def->getFreeTime(Game::tick - data.freeTime);
	
	// 特殊处理, 有次数限制并且又用完了, 时间返回0
	if (def->isTimesLimit && obj.freeNum <= 0) {
		obj.freeTm = 0;
	}
}