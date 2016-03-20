//
//  StoreGoodsBase.h
//  GameSrv
//
//  Created by jin on 13-9-11.
//
//

#ifndef __GameSrv__StoreGoodsBase__
#define __GameSrv__StoreGoodsBase__

#include <iostream>


#include <set>
#include <map>
#include <string>
#include "Defines.h"
#include "DataCfg.h"
#include "Singleton.h"
#include "../global/Activity.h"
#include "redis_helper.h"

using namespace std;

class Role;
class RandGoodsCfgBase;
class RandGoodsDef;

class StoreGoodsOpenBase
{
    // 刷新时间
    READONLY(int, mUpdateTime, UpdateTime);
public:
    StoreGoodsOpenBase(){
		mIsOpen = false;
		mInterval = 0;
		mUpdateTime = 0;
	}
    ~StoreGoodsOpenBase(){}
	
    // 检查是否开启
    bool checkIsOpen() {return mIsOpen;}
    // 检查是否需要刷新
    bool checkIsUpdate(int tm) {return tm >= mUpdateTime ? false : true;}
    // 获取刷新时间倒计时(秒)
    int getProcUpdateTime()
	{
		int len = time(NULL) - mUpdateTime;
		return len > mInterval ? 0 : mInterval - len;
	}
protected:	
	void refresh() {
		mUpdateTime = time(NULL);
	}
	void close() {
		mIsOpen = false;
		mUpdateTime = 0;
	}
	int getInterval() {return mInterval;}
	int mInterval;
    int mIsOpen;
};


enum StoreGoodsTypeRandType
{
    enBoth,    // 两者
    enRmd,     // 金钻
    enConsume, // 其它消耗
};

struct StoreGoods
{
	// 索引id
	int index;
	// 物品id
	int goodsId;
	// 物品数量
	int goodsNum;
	// 售卖类型
	enStoreGoodsType type;
	// 售卖价格
	int price;
	// 是否购买
	int isBuy;
};


typedef std::map<int, StoreGoods> StoreGoodsTd;
class StoreGoodsBase
{
	// 刷新时间
    READONLY(int, mUpdateTime, UpdateTime);
    // 刷新次数累计
    READONLY(int, mUpdateNum, UpdateNum);
public:
	
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
	
	void manuallyUpdateList()
	{
		updateBuyList();
		
		mUpdateNum += 1;
		
		saveUpdateNum();
		saveStoreGoodsList();
	}
	
	// 更新
	void reFresh() {
		mUpdateNum = 0;
		saveUpdateNum();
	}
	
	// 重置更新时间
	void reFreshTime() {
		mUpdateTime = 0;
	}
	
	// 是否开启
	bool checkIsOpen() {
		return mSgOpen->checkIsOpen();
	}
	
	// 是否超时
	bool checkIsTimeout() {
		return mSgOpen->checkIsUpdate(mUpdateTime);
	}
	
	// 刷新倒计时
	int getProcUpdateTime() {
		return mSgOpen->getProcUpdateTime();
	}
	
	// 是否免费刷新
	bool isFreeUpdate() {
		return mUpdateNum < mRgCfg->getFreeNum();
	}
	
	// 更新需要金钻
	int getUpdateNeedRmb() {
		return mRgCfg->getUpdateNeedRmb();
	}
	
	void setStoreGoodsBuyIndex(StoreGoods *sg);
	
	StoreGoodsTd::const_iterator begin() {return mStoreGoodsMap.begin();}
    StoreGoodsTd::const_iterator end() {return mStoreGoodsMap.end();}
    StoreGoods * getStoreGoods(int index);
protected:
	StoreGoodsBase()
	{
		mUpdateTime = 0;
		mUpdateNum = 0;
		mStoreGoodsMap.clear();
	}
	virtual int getLvl() {return 0;}
	virtual const char * getStoreKey() {return "";}
	virtual void saveUpdateTime(){}
	virtual void saveUpdateNum(){}
	bool insert();
	

	StoreGoodsOpenBase *mSgOpen;
	RandGoodsCfgBase *mRgCfg;
	Role *mRole;
	
	StoreGoodsTd mStoreGoodsMap;
	void saveStoreGoodsList();
private:
	void saveStoreGoods(StoreGoods *sg);
	void updateBuyList();
	void insertStoreGoods(RandGoodsDef *def, enStoreGoodsType type, int index);

};

#endif /* defined(__GameSrv__StoreGoodsBase__) */
