//
//  WeaponEnchant.cpp
//  GameSrv
//
//  Created by haifeng on 14-11-4.
//
//

#include "WeaponEnchant.h"

#include "DataCfg.h"
#include "basedef.h"
#include "EnumDef.h"

#include "Role.h"
#include "msg.h"
#include "GameLog.h"
#include <stdlib.h>
#include "Paihang.h"
#include "RoleAwake.h"
enum costEnchantType
{
    enchantdust = 1,
    enchantgold = 2,
};

WeaponEnchantMgr::WeaponEnchantMgr(Role *role)
{
    mOwner = role;
    mOwnerId = 0;
}

WeaponEnchantMgr* WeaponEnchantMgr::create(Role* role)
{
    WeaponEnchantMgr* weaponEnchant = new WeaponEnchantMgr(role);
    weaponEnchant->mOwner = role;
    weaponEnchant->mOwnerId = role->getInstID();
    return weaponEnchant;
}

bool WeaponEnchantMgr::loadEnchantData()
{
    if (mOwnerId == 0) {
        return false;
    }
    
    EnchantDBData::load(mOwnerId);
    calcWeaponEnchantProperty();
    return true;
}

// 获取某种武器附魔当前消耗的总货币
int WeaponEnchantMgr::getTotalCost(int enchantId, int enchantLvl, int currentCoin)
{
    int totalCoin = 0;
    for (int i = 1; i <= enchantLvl; ++i) {
        totalCoin += WeaponEnchantCfg::getCost(enchantId, i);
    }
    totalCoin += currentCoin;
    return totalCoin;
}

int WeaponEnchantMgr::getTotalCost(WeaponEnchantData& enchant)
{
	return this->getTotalCost(enchant.mEnchantId, enchant.mEnchantLvl, enchant.mEnchantExp);
}

// 获取最高等级附魔
WeaponEnchantData WeaponEnchantMgr::getCurrentEnchant()
{

//    WeaponEnchantData enchant;
//    enchant.mEnchantId = mOwner->getEnchantId();
//    enchant.mEnchantLvl = mOwner->getEnchantLvl();
//    
//    WeaponEnchantData *enchantDb = getEnchantDataList().find(mOwner->getEnchantId());
//    if (enchantDb == NULL) {
//        enchant.mEnchantExp = 0;
//    } else {
//        enchant.mEnchantExp = enchantDb->mEnchantExp;
//    }
//    
//    WeaponEnchantDataList::Iterator iter;
//    for (iter = getEnchantDataList().begin(); iter != getEnchantDataList().end(); ++iter) {
//        if (enchant.mEnchantLvl < (iter->second).mEnchantLvl) {
//            enchant = iter->second;
//        } else if (enchant.mEnchantLvl == (iter->second).mEnchantLvl) {
//            enchant = enchant.mEnchantExp >= (iter->second).mEnchantExp ? enchant : iter->second;
//        }
//    }
	
    WeaponEnchantData data;
    data.mEnchantId = mOwner->getEnchantId();
    data.mEnchantLvl = mOwner->getEnchantLvl();
    WeaponEnchantData *enchantdb = getEnchantDataList().find(mOwner->getEnchantId());
    if (enchantdb != NULL) {
        data.mEnchantExp = enchantdb->mEnchantExp;
    } else {
        data.mEnchantExp = 0;
    }
    
    WeaponEnchantData* enchant = &data;

	WeaponEnchantDataList::Iterator it;
	for (it = getEnchantDataList().begin(); it != getEnchantDataList().end(); ++it) {
		if (NULL == enchant
			|| enchant->mEnchantLvl < it->second.mEnchantLvl
			|| (enchant->mEnchantLvl == it->second.mEnchantLvl
				&& enchant->mEnchantExp < it->second.mEnchantExp)) {
			enchant = &it->second;
		}
	}

	if (NULL == enchant) {
		WeaponEnchantData temp;
		return temp;
	}
	
	return *enchant;
}

struct EnchantRandom
{
	EnchantRandom(int id, int cost, int max)
	: enchantId(id), enchantCost(cost), isMax(max) {}
	
	int enchantId;
	int enchantCost;
	bool isMax;
};

/* 
 * 附魔种类 typeNum; 生效数值 effectValue; 最低概率 lowestPro;
 * 设 每种附魔当前总经验这 EXP(i), i = 1~typeNum;
 * 记 A = MAX(EXP(1), EXP(2), ..., EXP(typeNum), effectValue, 1),
 * B(i) = A / MAX(EXP(i), effectValue, 1);
 * C = SUM(B(1), B(2), ..., B(typeNum));
 * D(i) = B(i) / C * (1 - typeNum * lowestPro) + lowestPro;
 * D(i) 为所求概率
 */
// 产生随机附魔类型
bool WeaponEnchantMgr::getRandomEnchant(WeaponEnchantDataList::EnchantDataList& data, int& enchantId)
{
    int typeNum = WeaponEnchantCfg::mTypeNum;
    int effectValue = WeaponEnchantCfg::mEffectValue;
    float lowestPro = WeaponEnchantCfg::mLowestPro;
    
	check_min(effectValue, 1);
	
	// 求A与EXP
	int A = effectValue;
    vector<EnchantRandom> EXP;
    map<int,WeaponEnchantCfgDef *>& enchantCfg = WeaponEnchantCfg::getEnchantCfgData();
    for (map<int,WeaponEnchantCfgDef *>::iterator it = enchantCfg.begin(); it != enchantCfg.end(); ++it) {
		
		int enchant_cost = effectValue;
		bool isMax = false;
		
		WeaponEnchantDataList::Iterator enchantIt = data.find(it->first);
        if (enchantIt != data.end()) {
            enchant_cost = getTotalCost(enchantIt->second);
			check_min(enchant_cost, effectValue);
			isMax = it->second->isMax(enchantIt->second.mEnchantLvl);
        }
		
		EXP.push_back(EnchantRandom(it->first, enchant_cost, isMax));
		
		// 找到最大的经验给A;
		check_min(A, enchant_cost);
    }
	
	// 求B与C
    vector<float> B;
	float C = 0.0;
    for (vector<EnchantRandom>::iterator it = EXP.begin(); it != EXP.end(); ++it) {
        float tmp = (float)A / it->enchantCost;
		C += tmp;
        B.push_back(tmp);
    }
    
	// 求D
    vector<int> D;
    for (vector<float>::iterator it = B.begin(); it != B.end(); ++it) {
        float tmp = *it / C * (1 - typeNum * lowestPro) + lowestPro;
        D.push_back(tmp * 10000);
    }
	
	// 满级的类型,权值赋值为0
	int maxCount = 0;
	for (int i = 0; i < EXP.size(); i++) {
		if (EXP[i].isMax) {
			D[i] = 0;
			maxCount++;
		}
	}
	
	// 全满级,返回
	if (maxCount >= EXP.size()) {
		return false;
	}
	
	// 按权值随机
	int index = getRand(D.data(), D.size());
    enchantId = EXP[index].enchantId;

	return true;
}

// 检查升级条件
int WeaponEnchantMgr::checkUpgradCondition(int enchantId, int enchantLvl)
{
    WeaponEnchantCfgDef* cfg = WeaponEnchantCfg::getWeaponEnchantCfgDef(enchantId);
    if (NULL == cfg) {
        LogMod::addLogWeaponEnchantErr(mOwner, enchantId, enchantLvl, "配置不存在");
        return CE_ENCHANT_UNKNOWN;
    }

	if (enchantLvl >= cfg->maxLvl) {
		return CE_ENCHANT_CUR_FULL;
	}
	
    if (cfg->getCost(enchantLvl + 1) == INT_MAX) {
        LogMod::addLogWeaponEnchantErr(mOwner, enchantId, enchantLvl, "已达最高等级");
        return CE_ENCHANT_CUR_FULL;
    }
    
    return CE_ENCHANT_OK;
}

//如果经验满，则加等级
void WeaponEnchantMgr::upLvl(WeaponEnchantData& enchant)
{
	WeaponEnchantCfgDef* cfg = WeaponEnchantCfg::getWeaponEnchantCfgDef(enchant.mEnchantId);
    if (NULL == cfg) {
		return;
	}
	
    int nextLvlCost = cfg->getCost(enchant.mEnchantLvl + 1);
    if (enchant.mEnchantExp >= nextLvlCost) {
        if (enchant.mEnchantLvl < cfg->maxLvl) {
            enchant.mEnchantLvl++;
            enchant.mEnchantExp -= nextLvlCost;
        }
    }
}

// 升级
int WeaponEnchantMgr::UpgradeEnchant(WeaponEnchantData& enchant)
{
    int preLvl = enchant.mEnchantLvl;
    int preExp = enchant.mEnchantExp;

    int code = checkUpgradCondition(enchant.mEnchantId, enchant.mEnchantLvl);
	if (code != CE_ENCHANT_OK) {
		return code;
	}

    enchant.mEnchantExp++;
    upLvl(enchant);
    
    LogMod::addLogWeaponEnchantUpgrade(mOwner,
                                        enchant.mEnchantId,
                                        preLvl,
                                        enchant.mEnchantLvl,
                                        preExp,
                                        enchant.mEnchantExp,
                                        "随机附魔");
    
    
    return CE_ENCHANT_OK;
}

void WeaponEnchantMgr::calcWeaponEnchantProperty()
{
    BattleProp tmpBatProp;
    map<int,WeaponEnchantCfgDef *>::iterator iter;
    map<int,WeaponEnchantCfgDef *> enchantCfg = WeaponEnchantCfg::getEnchantCfgData();
    for (iter = enchantCfg.begin(); iter != enchantCfg.end(); ++iter) {
        WeaponEnchantData *enchant = getEnchantDataList().find((iter->second)->typeId);
        
        BattleProp battleProp;
        if (enchant != NULL) {
            int enchant_id = enchant->mEnchantId;
            int enchant_lvl = enchant->mEnchantLvl;
            EnchantPropertyAddDef *propertyAdd;
            propertyAdd = WeaponEnchantCfg::getEnchantPropertyAddDef(enchant_id, enchant_lvl);
            if (propertyAdd != NULL) {
                battleProp.mMaxHp += propertyAdd->getHp();
                battleProp.mAtk += propertyAdd->getAtk();
                battleProp.mDef += propertyAdd->getDef();
                battleProp.mDodge += propertyAdd->getDodge();
            }
        }
        
        tmpBatProp += battleProp;
    }
    
    mBatProp = tmpBatProp;
    
    mOwner->CalcPlayerProp();
}

void WeaponEnchantMgr::accuProperty(BattleProp& battleProp)
{
    battleProp += mBatProp;
    
    //觉醒附加值
    if (mOwner->getRoleAwake()) {
        battleProp *= ( 1 + mOwner->getRoleAwake()->getEnchantsAddRatio());
    }
}

// 发送初始状态
void WeaponEnchantMgr::SendWeaponEnchantState()
{
    ack_enchant_list ack;
    ack.errorcode = -1;
    
    map<int,WeaponEnchantCfgDef *>::iterator iter;
    map<int,WeaponEnchantCfgDef *>& enchantCfg = WeaponEnchantCfg::getEnchantCfgData();
    for (iter = enchantCfg.begin(); iter != enchantCfg.end(); ++iter) {
	    obj_enchant_info enchantInfo;
		
        WeaponEnchantData *enchant = getEnchantDataList().find((iter->second)->typeId);
        enchantInfo.enchantId = (iter->second)->typeId;
        if (enchant == NULL) {
            enchantInfo.enchantLvl = 0;
            enchantInfo.currentExp = 0;
        } else {
            enchantInfo.enchantLvl = enchant->mEnchantLvl;
            enchantInfo.currentExp = enchant->mEnchantExp;
        }
		ack.enchantList.push_back(enchantInfo);
    }
    
    ack.errorcode = 0;
    sendNetPacket(mOwner->getSessionId(), &ack);
}

// 随机附魔
void WeaponEnchantMgr::SendRandomWeaponEnchantResult(int continueNum)
{
    ack_random_enchant ack;
    ack.errorcode = CE_ENCHANT_UNKNOWN;
    int enchant_dust = mOwner->getEnchantDust();
	
	// 不能超过enchant_dust
	check_max(continueNum, enchant_dust);
    
    WeaponEnchantData preEnchant = getCurrentEnchant();
    set<WeaponEnchantData*> upSet;		// 记录升级的附魔
    
    // 取数据库中的数据，存到dbData
	WeaponEnchantDataList::EnchantDataList dbData = getEnchantDataList().copy();
    
	int loopNum = 0;
    while (loopNum < continueNum)
    {
		int enchant_id = 0;
        if (!getRandomEnchant(dbData, enchant_id)) {
			break;
		}
		
		WeaponEnchantData& wed = dbData[enchant_id];
		wed.mEnchantId = enchant_id;
        
		ack.errorcode = UpgradeEnchant(wed);
        if (ack.errorcode != CE_ENCHANT_OK) {
			break;
		}
		
		upSet.insert(&wed);
		loopNum++;
    }
    
    if (loopNum > 0) {
        mOwner->addEnchantDust(-loopNum, "随机附魔");

		for (set<WeaponEnchantData*>::iterator it = upSet.begin(); it != upSet.end(); it++) {
			WeaponEnchantData* temp = *it;
            getEnchantDataList().add(temp->mEnchantId, *temp);
			
            //给客户端发的数据
            obj_enchant_info enchantInfo;
            enchantInfo.enchantId = temp->mEnchantId;
            enchantInfo.enchantLvl = temp->mEnchantLvl;
            enchantInfo.currentExp = temp->mEnchantExp;
            ack.enchantList.push_back(enchantInfo);
        }
		save();
    
        // 若最高等级发生变化，则给客户端发通知;
        WeaponEnchantData curEnchant = getCurrentEnchant();
        if (preEnchant.mEnchantId != curEnchant.mEnchantId || preEnchant.mEnchantLvl != curEnchant.mEnchantLvl) {
            
            mOwner->setEnchantId(curEnchant.mEnchantId);
            mOwner->setEnchantLvl(curEnchant.mEnchantLvl);
            mOwner->saveNewProperty();
            
            notify_current_enchant notify;
            notify.enchantId = curEnchant.mEnchantId;
            notify.enchantLvl = curEnchant.mEnchantLvl;
            sendNetPacket(mOwner->getSessionId(), &notify);
        }
        
        // 属性加成
        calcWeaponEnchantProperty();
        mOwner->sendRolePropInfoAndVerifyCode();
        mOwner->UploadPaihangData(eUpdate);
		ack.errorcode = CE_ENCHANT_OK;
    }
    
    sendNetPacket(mOwner->getSessionId(), &ack);
    
}

// 指定附魔
void WeaponEnchantMgr::SendSureWeaponEnchantResult(int enchant_id, int continueNum, ack_sure_enchant &ack)
{
    ack.errorcode = CE_ENCHANT_UNKNOWN;
    int enchant_lvl = 0;
    int enchant_exp = 0;
    
    if (continueNum <= 0) {
        ack.errorcode = CE_ENCHANT_UNKNOWN;
        return;
    }
	
    // 附魔尘不足
    if (mOwner->getEnchantDust() < continueNum) {
        ack.errorcode = CE_ENCHANT_DUST_LACK;
        return;
    }
    
    // 附魔钻不足
    if (mOwner->getEnchantGold() < continueNum) {
        ack.errorcode = CE_ENCHANT_GOLD_LACK;
        return;
    }
    
    // 获取升级前的，当前附魔；
    WeaponEnchantData preEnchant = getCurrentEnchant();
    WeaponEnchantCfgDef *cfg = WeaponEnchantCfg::getWeaponEnchantCfgDef(enchant_id);
	if (NULL == cfg) {
		return;
	}
    
    // 取数据库数据
    WeaponEnchantData *enchant = getEnchantDataList().find(enchant_id);
    if (enchant != NULL) {
        enchant_lvl = enchant->mEnchantLvl;
        enchant_exp = enchant->mEnchantExp;
    }
    
    int preLvl = enchant_lvl;   // 升级前的等级
    int preExp = enchant_exp;   // 升级前的经验
    if (enchant_lvl >= cfg->maxLvl) {
        ack.errorcode = CE_ENCHANT_UNKNOWN;
        LogMod::addLogWeaponEnchantErr(mOwner, enchant_id, enchant_lvl, "达到最高等级");
		return;
    }
    
    // 加经验和等级
    enchant_exp += (2 * continueNum);
	int returnDust = 0;
    while (true) {
        int needExp = WeaponEnchantCfg::getCost(enchant_id, enchant_lvl + 1);
		if (enchant_exp < needExp || needExp == INT_MAX) {
			break;
		}
		
		enchant_exp -= needExp;
		enchant_lvl++;
		
		// 升至满级，还有多剩经验，返还附魔尘
		if (enchant_lvl >= cfg->maxLvl && enchant_exp > 0)
		{
			enchant_exp = 0;
			returnDust = 1;
			break;
		}
    }
    
    // 扣钱
    mOwner->addEnchantDust(-(continueNum - returnDust), "指定附魔");
    mOwner->addEnchantGold(-continueNum, "指定附魔");
	
	// 保存数据库
	WeaponEnchantData data;
	data.mEnchantId = enchant_id;
	data.mEnchantLvl = enchant_lvl;
	data.mEnchantExp = enchant_exp;
	getEnchantDataList().add(enchant_id, data);
	save();
    
	// 属性加成
	if (preLvl < enchant_lvl) {
		calcWeaponEnchantProperty();
		mOwner->sendRolePropInfoAndVerifyCode();
	}
    
	// 给客户端的数据
	obj_enchant_info enchantInfo;
	enchantInfo.enchantId = enchant_id;
	enchantInfo.enchantLvl = enchant_lvl;
	enchantInfo.currentExp = enchant_exp;
	ack.enchantList.push_back(enchantInfo);
    
	// 若最高等级发生变化，则给客户端发通知;
	WeaponEnchantData curEnchant = getCurrentEnchant();
	if (preEnchant.mEnchantId != curEnchant.mEnchantId || preEnchant.mEnchantLvl != curEnchant.mEnchantLvl) {
		
		mOwner->setEnchantId(curEnchant.mEnchantId);
		mOwner->setEnchantLvl(curEnchant.mEnchantLvl);
		mOwner->saveNewProperty();
		
		notify_current_enchant notify;
		notify.enchantId = curEnchant.mEnchantId;
		notify.enchantLvl = curEnchant.mEnchantLvl;
		sendNetPacket(mOwner->getSessionId(), &notify);
	}

    LogMod::addLogWeaponEnchantUpgrade(mOwner,
                                       enchant_id,
                                       preLvl,
                                       enchant_lvl,
                                       preExp,
                                       enchant_exp,
                                       "指定附魔");

    mOwner->UploadPaihangData(eUpdate);
	ack.errorcode = CE_ENCHANT_OK;
}

// 自由选择满级附魔
void WeaponEnchantMgr::ChooseWeaponEnchant(int enchantId)
{
    ack_choose_enchant ack;
    ack.errorcode = -1;
    int preEnchantId = mOwner->getEnchantId();
    
    do {
        WeaponEnchantData *enchantdb = getEnchantDataList().find(enchantId);
        WeaponEnchantCfgDef* enchantcfg = WeaponEnchantCfg::getWeaponEnchantCfgDef(enchantId);
        
        if (enchantdb == NULL || enchantcfg == NULL) {
            break;
        }
        
        // 如果已达到最高等级
        if (enchantdb->mEnchantLvl == enchantcfg->maxLvl) {
            ack.errorcode = 0;
            ack.enchantId = enchantId;
            mOwner->setEnchantId(enchantId);
            mOwner->setEnchantLvl(enchantdb->mEnchantLvl);
        }
        
        // 最高等级变化，发通知
        if (preEnchantId != enchantId) {
            notify_current_enchant notify;
            notify.enchantId = enchantId;
            notify.enchantLvl = enchantdb->mEnchantLvl;
            sendNetPacket(mOwner->getSessionId(), &notify);
        }
        
    } while (0);
    
    sendNetPacket(mOwner->getSessionId(), &ack);
}

void WeaponEnchantMgr::addBatPropByRatio(float ratio)
{
    mBatProp *= (1 + ratio);
}
