//
//  MysteriousMgr.cpp
//  GameSrv
//
//  Created by jin on 13-8-23.
//
//

#include "MysteriousMgr.h"
#include "main.h"
#include "Utils.h"
#include "GameLog.h"
#include "DBRolePropName.h"
#include "json/json.h"
#include "hander_include.h"
#include "mail_imp.h"
#include "DynamicConfig.h"
#include "Role.h"
#include "Honor.h"
// #####################################

INSTANTIATE_SINGLETON(MysteriousMan);

MysteriousMan g_MysteriousMan;
MysteriousMan::MysteriousMan()
{

}

void MysteriousMan::init()
{
	time_t now = time(NULL);
    mInterval = MysteriousCfgMgr::sCfg.getInterval();
    assert(mInterval > 0);
	
	int tmLen = now % mInterval;
	// 上一次刷新时间
	mUpdateTime = now - tmLen;
	mIsOpen = true;
	
	// 下一次刷新回调
	addTimer(OnPreviousRefresh, NULL, mInterval - tmLen, 1);
}

void MysteriousMan::OnPreviousRefresh(void *param)
{
	g_MysteriousMan.refresh();
	addTimer(OnRefresh, NULL, g_MysteriousMan.getInterval(), FOREVER_EXECUTE);
}

void MysteriousMan::OnRefresh(void *param)
{
	g_MysteriousMan.refresh();
}


bool MysteriousMan::storeGoods(Role *role, GridArray &effgrids, const char *desc)
{
    GridArray realEffgrids;
    
    for (int i = 0; i < effgrids.size(); i++) {
        
        ItemCfgDef *cfg = ItemCfg::getCfg(effgrids[i].item.item);
        
        // 是直接使用的物品
        bool isOpen = false;
        
        if (cfg->ReadInt("type") == kItemConsumable) {
            
            string affect_type1 = cfg->ReadStr("affect_type1");
            if (strcmp(affect_type1.c_str(), "incr_exp") == 0 ||
                strcmp(affect_type1.c_str(), "incr_batpoint") == 0 ||
                strcmp(affect_type1.c_str(), "incr_constell") == 0)
            {
                isOpen = true;
            }
        }
        
        if (isOpen)
        {
            if (useConsumeItem(cfg, role, effgrids[i].item.count)) {
                xyerr("商店 : role %d use item err %d %d",
					  role->getInstID(), effgrids[i].item.item, effgrids[i].item.count);
                return false;
            }
        }
        else
        {
            realEffgrids.push_back(effgrids[i]);
        }
        
    }
	
	// 添加物品到玩家
	ItemArray items;
    if (realEffgrids.size() > 0) {
//		role->updateBackBag(realEffgrids, items, true, desc);
        role->playerAddItemsAndStore(realEffgrids, items, desc, true);
	}
	return true;
}

void MysteriousMgr::load(Role *role)
{
    mRole = role;
	mSgOpen = &g_MysteriousMan;
	mRgCfg = &MysteriousCfgMgr::sCfg;
	
	
	string num = mRole->loadProp(GetRolePropName(eRoleMysteriousNum));
	string goldNum = mRole->loadProp(GetRolePropName(eRoleMysteriousGoldNum));
    string tm = mRole->loadProp(GetRolePropName(eRoleMysteriousTm));
    if (!num.empty()) {
        mUpdateNum = atoi(num.c_str());
    }
	if (!goldNum.empty()) {
        mUpdateGoldNum = atoi(goldNum.c_str());
    }
    if (!tm.empty()) {
        mUpdateTime = atoi(tm.c_str());
    }

	// 检查加载数据有没有成功, 不成功时, 让列表可以刷新
	if (!insert()) {
		mUpdateTime = 0;
	} 
	
	loadExchange();
	loadSpecial();
}

void MysteriousMgr::loadExchange()
{
	StringTokenizer token(mRole->getMysteriousExchange(), ";");
	for (StringTokenizer::Iterator it = token.begin(); it != token.end(); it++) {
		int key = 0;
		int val = 0;
        if(sscanf(it->c_str(), "%d-%d", &key, &val) != 2)
		{
			continue;
		}
		mExchange[key] = val;
	}
}

void MysteriousMgr::saveExchange()
{
	vector<string> temp;
	for (map<int, int>::iterator it = mExchange.begin(); it != mExchange.end(); it++) {
		temp.push_back(xystr("%d-%d", it->first, it->second));
	}
	mRole->setMysteriousExchange(StrJoin(temp.begin(), temp.end(), ";"));
	mRole->saveNewProperty();
}

void MysteriousMgr::loadSpecial()
{
	StringTokenizer token(mRole->getMysteriousSpecial(), ";");
	for (StringTokenizer::Iterator it = token.begin(); it != token.end(); it++) {
		int key = 0;
		int val = 0;
        if(sscanf(it->c_str(), "%d-%d", &key, &val) != 2)
		{
			continue;
		}
		mSpecial[key] = val;
	}
}

void MysteriousMgr::saveSpecial()
{
	vector<string> temp;
	for (map<int, int>::iterator it = mSpecial.begin(); it != mSpecial.end(); it++) {
		temp.push_back(xystr("%d-%d", it->first, it->second));
	}
	mRole->setMysteriousSpecial(StrJoin(temp.begin(), temp.end(), ";"));
	mRole->saveNewProperty();
}

void MysteriousMgr::checkclearExchange()
{
	if (MysteriousCfgMgr::sCfg.exchangeIsDelay(mRole->getMysteriousExchangeTm())) {
		
		mRole->setMysteriousExchangeTm(MysteriousCfgMgr::sCfg.getExchangeEnd());
		
		mExchange.clear();
		saveExchange();
	}
}
void MysteriousMgr::checkclearSpecial()
{
	if (MysteriousCfgMgr::sCfg.specialIsDelay(mRole->getMysteriousSpecialTm())) {

		mRole->setMysteriousSpecialTm(MysteriousCfgMgr::sCfg.getSpecialEnd());
		
		mSpecial.clear();
		saveSpecial();
	}
}

bool MysteriousMgr::checkExchangeLimit(int index, int max, int num)
{
	if (max <= 0) {
		return false;
	}
	map<int, int>::iterator it = mExchange.find(index);
	if (it == mExchange.end()) {
		return false;
	}
	if (it->second + num > max) {
		return true;
	}
	return false;
}

bool MysteriousMgr::checkSpecialLimit(int index, int max, int num)
{
	if (max <= 0) {
		return false;
	}
	map<int, int>::iterator it = mSpecial.find(index);
	if (it == mSpecial.end()) {
		return false;
	}
	if (it->second + num > max) {
		return true;
	}
	return false;
}

void MysteriousMgr::setExchange(int index, int num)
{
	mExchange[index] += num;
	saveExchange();
}

void MysteriousMgr::setSpecial(int index)
{
	mSpecial[index] += 1;
	saveSpecial();
}

void MysteriousMgr::setGoodsNum(int index, int num)
{
    mSpecial[index] += num;
    saveSpecial();
}


bool MysteriousMgr::checkDeductions(int type)
{
	int cost = 0;
	switch (type) {
		case enStoreGoodsType_RmbCost:
			if (!isFreeUpdate()) {
				cost = getUpdateNeedRmb();
			}
			break;
		case enStoreGoodsType_ConsumeCost:
			if (!isGoldFreeUpdate()) {
				cost = getUpdateNeedGold();
			}
			break;
	}
	// 不需要消耗
	if (cost == 0) {
		return true;
	}
	if (mRole->CheckMoneyEnough(cost, (CostType)type, "刷新神秘商店") != CE_OK) {
		return false;
	}
    return true;
}

bool  MysteriousMgr::isTimesUseUp(int type, int viplvl)
{
    if (!MysteriousCfgMgr::sCfg.isUseViplvlRefreshLimit())
    {
        return false;
    }
    //获取最大刷新次数
    int times = mRgCfg->getFreeNum() + mRgCfg->getVipRefreshTimes(viplvl);
    int roleRefreshTimes = 0;
    switch (type) {
		case enStoreGoodsType_RmbCost:
            roleRefreshTimes = getUpdateNum();
			break;
		case enStoreGoodsType_ConsumeCost:
            roleRefreshTimes = getUpdateGoldNum();
			break;
	}
    if (roleRefreshTimes < times)
    {
        return false;
    }
    return true;
}

void MysteriousMgr::updateBuyList() {
	vector<MysteriousGoodsDef*> vec;
	vector<MysteriousGoodsDef*>::iterator it;
	
	mStoreGoodsMap.clear();
	// 更新逻辑
	MysteriousCfgMgr::sCfg.randGoods(getLvl(), getVipLvl(), vec);
	if (vec.empty()) {
		return;
	}
	
	for (it = vec.begin(); it != vec.end(); it++) {
		MysteriousGoodsDef *def = *it;
		StoreGoods sg;
		sg.index = def->index;
		sg.goodsId = def->goodsId;
		sg.goodsNum = 1;
		sg.type = (enStoreGoodsType)def->type;
		sg.price = range_rand(def->consumeMin, def->consumeMax);
		sg.isBuy = false;
		
		mStoreGoodsMap.insert(make_pair(sg.index, sg));
	}
}

void MysteriousMgr::removeMapType(int type)
{
	StoreGoodsTd::iterator it;
	for (it = mStoreGoodsMap.begin(); it != mStoreGoodsMap.end();) {
		if (it->second.type == type) {
			mStoreGoodsMap.erase(it++);
		} else {
			it++;
		}
	}
}


void MysteriousMgr::saveUpdateTime()
{
    mRole->saveProp(GetRolePropName(eRoleMysteriousTm), Utils::itoa(mUpdateTime).c_str());
}

void MysteriousMgr::saveUpdateNum()
{
    mRole->saveProp(GetRolePropName(eRoleMysteriousNum), Utils::itoa(mUpdateNum).c_str());
	mRole->saveProp(GetRolePropName(eRoleMysteriousGoldNum), Utils::itoa(mUpdateGoldNum).c_str());
}

int MysteriousMgr::getLvl()
{
	return mRole->getLvl();
}

int MysteriousMgr::getVipLvl()
{
    return mRole->getVipLvl();
}
//////////////////////////////////////////////////////////////

void Illustrations::init()
{
    mIllId = 0;
    mType = eIllType_General;
    mQuaNum.resize(eIllQua_Count, 0);
    mReward.resize(eIllQua_Count + 1, eIllState_Non);
}


bool Illustrations::insertData(string data)
{
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(data, value)) {
        return false;
    }
    
    init();
    
    mIllId = value["id"].asInt();
    mType = (IllustrationsType)value["type"].asInt();
    mQuaNum[eIllQua_Green] = value["green"].asInt();
    mQuaNum[eIllQua_Blue] = value["blue"].asInt();
    mQuaNum[eIllQua_Purple] = value["purple"].asInt();
    mReward[eIllQua_Green] = (IllustrationsState)value["greenS"].asInt();
    mReward[eIllQua_Blue] = (IllustrationsState)value["blueS"].asInt();
    mReward[eIllQua_Purple] = (IllustrationsState)value["purpleS"].asInt();
    mReward[eIllQua_Count] = (IllustrationsState)value["allS"].asInt();

    return true;
}

string Illustrations::formatSaveData()
{
    Json::Value value;
    value["id"] = mIllId;
    value["type"] = mType;
    value["green"] = mQuaNum[eIllQua_Green];
    value["blue"] = mQuaNum[eIllQua_Blue];
    value["purple"] = mQuaNum[eIllQua_Purple];
    value["greenS"] = mReward[eIllQua_Green];
    value["blueS"] = mReward[eIllQua_Blue];
    value["purpleS"] = mReward[eIllQua_Purple];
    value["allS"] = mReward[eIllQua_Count];
    
    return Json::FastWriter().write(value);
}

void Illustrations::addIllustrations(IllustrationsQuality qua)
{
    addQuaNum(qua, 1);
    if (getRewardState(qua) != eIllState_Non) return;
    
    setRewardState(qua, eIllState_NotReceive);
    
    if (getRewardState(eIllQua_Count) == eIllState_Non
        && getRewardState(eIllQua_Green) != eIllState_Non
        && getRewardState(eIllQua_Blue) != eIllState_Non
        && getRewardState(eIllQua_Purple) != eIllState_Non)
    {
        setRewardState(eIllQua_Count, eIllState_NotReceive);
    }
}

void IllustrationsMgr::loadIllustrations(Role *role)
{
    mRole = role;
    mIllustrationsMap.clear();
    
    RedisResult result(redisCmd("hvals illustrations:%d", mRole->getInstID()));
    int elementnum = result.getHashElements();
    for (int i = 0; i < elementnum; i++) {
        Illustrations ill;
        if (!ill.insertData(result.readHash(i, ""))) {
            continue;
        }
        mIllustrationsMap.insert(make_pair(ill.getIllId(), ill));
    }
    
    // 注意 在加载完宠物才调用的方法
    updatePlayerPetData();
}

void IllustrationsMgr::saveIllustrations(Illustrations *ill)
{
    doRedisCmd("hmset illustrations:%d %d %s", mRole->getInstID(), ill->getIllId(), ill->formatSaveData().c_str());
}

void IllustrationsMgr::getReward(Illustrations *ill, IllustrationsQuality qua)
{
    ill->setRewardState(qua, eIllState_Receive);
     saveIllustrations(ill);
}

void IllustrationsMgr::updatePlayerPetData()
{
    if (!IllustrationsCfg::isBagUpdateVer(mRole->getPetCollectVer())) {
        return;
    }
    
    vector<Pet*> pets = mRole->mPetMgr->getPets();
    for (vector<Pet*>::iterator it = pets.begin(); it != pets.end(); it++) {
        Pet* pet = *it;
        if (pet){
            addIllustrations(pet->petmod);
        }
    }
}



void IllustrationsMgr::addIllustrations(int petmod)
{
	Xylog log(eLogName_IllustrationsAdd, mRole->getInstID());
	log << petmod;
	
    PetCfgDef *petDef = PetCfg::getCfg(petmod);
    if (NULL == petDef) {
        return;
    }
	
    int monId = petDef->getCollectID();
	if (monId == 0) {
		monId =	petDef->getMonsterTpltID();
	}
    IllustrationsQuality qua = (IllustrationsQuality)petDef->getCollectqua();
    
	log << monId << qua;
	
	if (qua < 0 || qua > 2) {
		return;
	}
	
    IllustrationsDef *illDef = IllustrationsCfg::getIllustrationsDef(monId);
    if (NULL == illDef) {
        return; 
    }
    Illustrations *ill = getIllustrations(monId);
    if (NULL == ill) {
        Illustrations tempIll(monId, illDef->type);
        insertIllustrations(tempIll);
        ill = getIllustrations(monId);
    }
    if (NULL == ill) {
        xyerr("[图鉴] 获取图鉴失败 %d", monId);
        return;
    }
	
	log << ill->getIllId() << ill->getQuaNum(qua) << ill->getRewardState(qua);
	
	// 发送奖励消息
    checkSendRewardNotify(ill, qua);
    // 数量, 状态
    ill->addIllustrations(qua);

	if (ill->getRewardState(eIllQua_Count) == eIllState_NotReceive) {
		SHonorMou.procHonor(eHonorKey_Ill, mRole);
	}
	
    saveIllustrations(ill);
	
	log << ill->getQuaNum(qua) << ill->getRewardState(qua);
}

bool IllustrationsMgr::checkComIllustrations(IllustrationsType type)
{
	vector<IllustrationsDef*> vec = IllustrationsCfg::getIllustrationsDefsbyType(type);
	for (vector<IllustrationsDef*>::iterator it = vec.begin(); it != vec.end(); it++) {
		if (!isIllustrationsAll((*it)->templetId)) {
			return false;
		}
	}
	return true;
}

bool IllustrationsMgr::checkComIllustrations()
{
	IllustrationsCfg::Iterator it;
	for (it = IllustrationsCfg::begin(); it != IllustrationsCfg::end(); it++) {
		if (!isIllustrationsAll(it->second->templetId)) {
			return false;
		}
	}
	return true;
}


void IllustrationsMgr::checkSendRewardNotify(Illustrations *ill, IllustrationsQuality qua)
{
    if (ill->getRewardState(qua) != eIllState_Non) {
        return;
    }
    
    notify_illustrations_reward notify;
    notify.illId = ill->getIllId();
    notify.qua = qua;
    sendNetPacket(mRole->getSessionId(), &notify);
}

Illustrations * IllustrationsMgr::getIllustrations(int illId)
{
    IllustrationsTd::iterator it = mIllustrationsMap.find(illId);
    if (it == mIllustrationsMap.end()) {
        return NULL;
    }
    return &it->second;
}

void IllustrationsMgr::insertIllustrations(Illustrations &ill)
{
    mIllustrationsMap.insert(make_pair(ill.getIllId(), ill));
}

bool IllustrationsMgr::isIllustrationsAll(int illId)
{
	Illustrations *ill = getIllustrations(illId);
	if (NULL == ill) {
		return false;
	}
	// 检查全奖励未开启,　返回false
    return ill->getRewardState(eIllQua_Count) != eIllState_Non;
}
