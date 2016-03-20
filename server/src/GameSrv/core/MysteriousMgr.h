//
//  MysteriousMgr.h
//  GameSrv
//
//  Created by jin on 13-8-23.
//
//

#ifndef __GameSrv__MysteriousMgr__
#define __GameSrv__MysteriousMgr__

#include "StoreGoodsBase.h"
#include "ItemBag.h"

using namespace std;

class Role;


// ###################################################

// 神秘商人管理类
class MysteriousMan : public StoreGoodsOpenBase
{
public:
	MysteriousMan();
    ~MysteriousMan(){}
	
	static bool storeGoods(Role *role, GridArray &effgrids, const char *desc);
	
	void init();
	
	static void OnPreviousRefresh(void *param);
	static void OnRefresh(void *param);
};
extern MysteriousMan g_MysteriousMan;


class MysteriousMgr : public StoreGoodsBase
{
public:
	
	
	
	void load(Role *role);
	
	void checkUpdtaeList() {
		if (!checkIsOpen()) {
			return;
		}
		
		if (!checkIsTimeout()) {
			return;
		}
		
		updateBuyList();
		mUpdateTime = mSgOpen->getUpdateTime();
		
		saveUpdateTime();
		saveStoreGoodsList();
	}

	void manuallyUpdateList(int type)
	{
		// 这里修改了，手动刷新时，两个种类的物品都刷 2014-10-28 by jin
        // 通用商品，不区分金钻和金币，二者共用显示数量，随机产生商品 2015-02-02
		updateBuyList();
		
		if (type == enStoreGoodsType_RmbCost) {
			mUpdateNum += 1;
		} else {
			mUpdateGoldNum += 1;
		}

		saveUpdateNum();
		saveStoreGoodsList();
	}
	
	void reFresh() {
		mUpdateGoldNum = 0;
		StoreGoodsBase::reFresh();
	}
	// 添加商品
	
	
	// 手动更新消耗检查与扣除
	bool checkDeductions(int type);
    
    //刷新次数用完了
    bool isTimesUseUp(int type, int viplvl);
    
	int getUpdateGoldNum() {return mUpdateGoldNum;}
	
	
	void checkclearExchange();
	void checkclearSpecial();
	bool checkExchangeLimit(int index, int max, int num);
	bool checkSpecialLimit(int index, int max, int num);
	
	void setExchange(int index, int num);
	void setSpecial(int index);
    void setGoodsNum(int index, int num);
	
	map<int, int>::const_iterator exchangeBegin() {return mExchange.begin();}
	map<int, int>::const_iterator exchangeEnd() {return mExchange.end();}
	map<int, int>::const_iterator specialBegin() {return mSpecial.begin();}
	map<int, int>::const_iterator specialEnd() {return mSpecial.end();}
private:
	void loadExchange();
	void saveExchange();
	void loadSpecial();
	void saveSpecial();
	
	// 金币刷新次数
	int mUpdateGoldNum;
		
	// 移除某种类型商品
	void removeMapType(int type);
	void updateBuyList();
	int getUpdateNeedGold() {return MysteriousCfgMgr::sCfg.getUpdateNeedGold();}
	// 是否有空闲金币刷新次数
	bool isGoldFreeUpdate() {return mUpdateGoldNum < MysteriousCfgMgr::sCfg.getFreeGoldNum();}
	
	int getVipLvl();
	virtual int getLvl();
	virtual const char * getStoreKey() {return "mysteriousStore";}
	virtual void saveUpdateTime();
	virtual void saveUpdateNum();
	
	map<int, int> mExchange;
	map<int, int> mSpecial;
};




// ####################################
class Illustrations
{
public:
    Illustrations(){}
    Illustrations(int illId, IllustrationsType type)
    {
        init();
        mIllId = illId;
        mType = type;
    }
    
    // 数据格式
    bool insertData(string data);
    string formatSaveData();
    
    
    int getIllId() const {return mIllId;}
    int getType() const {return mType;}
    int getQuaNum(IllustrationsQuality qua) const {
        return mQuaNum[qua];
    }
    IllustrationsState getRewardState(IllustrationsQuality qua) const {
        return mReward[qua];
    }
    void setRewardState(IllustrationsQuality qua, IllustrationsState state) {
        mReward[qua] = state;
    }
    void addQuaNum(IllustrationsQuality qua, int num) {
        mQuaNum[qua] += num;
    }
    // 添加品质数量, 开启奖励
    void addIllustrations(IllustrationsQuality qua);
private:
    void init();
private:
    // 图鉴ID
    int mIllId;
    // 宠物类型
    IllustrationsType mType;
    // 品质数量
    vector<int> mQuaNum;
    // 奖励标记
    vector<IllustrationsState> mReward;
};

typedef std::map<int, Illustrations> IllustrationsTd;
class IllustrationsMgr
{
public:
    void loadIllustrations(Role *role);

    // 刷玩家原有的宠物数据, 加入图鉴
    void updatePlayerPetData();
    
    // 增加图鉴
    void addIllustrations(int petmod);
    
    // 领取奖励
    void getReward(Illustrations *ill, IllustrationsQuality qua);
	
	// 检查是否收集完图鉴
	bool checkComIllustrations(IllustrationsType type);
	bool checkComIllustrations();
	
    IllustrationsTd::const_iterator begin() {return mIllustrationsMap.begin();}
    IllustrationsTd::const_iterator end() {return mIllustrationsMap.end();}
    Illustrations * getIllustrations(int illId);
	
	// 该图鉴是否全收集
	bool isIllustrationsAll(int illId);
private:
    // 检查发送奖励消息
    void checkSendRewardNotify(Illustrations *ill, IllustrationsQuality qua);
    
    void insertIllustrations(Illustrations &ill);
    void saveIllustrations(Illustrations *ill);
	
	
    IllustrationsTd mIllustrationsMap;
    Role *mRole;
};




#endif /* defined(__GameSrv__MysteriousMgr__) */
