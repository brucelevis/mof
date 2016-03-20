//
//  WeaponEnchant.h
//  GameSrv
//
//  Created by haifeng on 14-11-4.
//
//

#ifndef __GameSrv__WeaponEnchant__
#define __GameSrv__WeaponEnchant__

#include <iostream>
#include <limits.h>
#include "RedisHash.h"
#include "msg.h"
#include "Defines.h"

using namespace std;

class Role;

class WeaponEnchantMgr : public EnchantDBData
{
public:
    static WeaponEnchantMgr* create(Role* role);
    
public:
    WeaponEnchantMgr(Role *role);
    ~WeaponEnchantMgr() {}
    
    bool loadEnchantData();
    WeaponEnchantData getCurrentEnchant();                // 获取最高等级

    

    // 计算属性加成
    void calcWeaponEnchantProperty();
    void accuProperty(BattleProp& battleProp);

    // 发送初始状态
    void SendWeaponEnchantState();
    // 随机附魔
    void SendRandomWeaponEnchantResult(int continueNum);
    // 指定附魔
    void SendSureWeaponEnchantResult(int enchantId, int continueNum, ack_sure_enchant &ack);
    // 自由选择满级附魔
    void ChooseWeaponEnchant(int enchantId);
    
    void addBatPropByRatio(float ratio);
private:
	
    bool getRandomEnchant(WeaponEnchantDataList::EnchantDataList& data, int& enchantId);
	
	// 经验满时加等级
    void upLvl(WeaponEnchantData& enchant);
	
    // 检查升级条件
    int checkUpgradCondition(int enchantId, int enchantLvl);
	// 升级
    int UpgradeEnchant(WeaponEnchantData& enchant);
	
	// 获取某种武器附魔当前花费的总货币
    int getTotalCost(int enchantId, int enchantLvl, int currentCoin);
	int getTotalCost(WeaponEnchantData& enchant);
	
	
    READWRITE(Role*, mOwner, Owner)
    int mOwnerId;
    BattleProp mBatProp;
};

#endif /* defined(__GameSrv__WeaponEnchant__) */
