//
//  StoreGoodsBase.cpp
//  GameSrv
//
//  Created by jin on 13-9-11.
//
//

#include "StoreGoodsBase.h"

#include "main.h"
#include "Utils.h"
#include "GameLog.h"
#include "DBRolePropName.h"
#include "json/json.h"
#include "hander_include.h"
#include "mail_imp.h"
#include "GuildRole.h"
#include "Guild.h"
#include "GuildMgr.h"


void StoreGoodsBase::updateBuyList()
{	
	vector<RandGoodsDef*> vec;
    vector<RandGoodsDef*>::iterator it;
    // 更新逻辑
    mRgCfg->randGoods(getLvl(), vec);
    if (vec.empty()) {
        return;
    }
	mStoreGoodsMap.clear();
    int index = 1;
    for (it = vec.begin(); it != vec.end(); it++, index++) {
        RandGoodsDef *def = *it;
		
		if (def->consumeMax <= 0) {
			insertStoreGoods(def, enStoreGoodsType_RmbCost, index);
			continue;
		}
		if (def->brmdMax <= 0) {
			insertStoreGoods(def, enStoreGoodsType_ConsumeCost, index);
			continue;
		}
		
		float rand = range_randf(0.0, 1.0);
		if (rand > def->consumePro) {
			insertStoreGoods(def, enStoreGoodsType_RmbCost, index);
		} else {
			insertStoreGoods(def, enStoreGoodsType_ConsumeCost, index);
		}
    }
}


void StoreGoodsBase::insertStoreGoods(RandGoodsDef *def, enStoreGoodsType type, int index)
{
	int price = 0;
    if (type == enStoreGoodsType_RmbCost) {
        price = range_rand(def->brmbMin, def->brmdMax);
    } else {
        price = range_rand(def->consumeMin, def->consumeMax);
    }
	
	StoreGoods sg;
	sg.index = index;
	sg.goodsId = def->goodsId;
	sg.type = type;
	sg.price = price;
	sg.goodsNum = 1;
	sg.isBuy = false;
	mStoreGoodsMap.insert(make_pair(sg.index, sg));
}

StoreGoods * StoreGoodsBase::getStoreGoods(int index)
{
    StoreGoodsTd::iterator it = mStoreGoodsMap.find(index);
    if (it == mStoreGoodsMap.end()) {
        return NULL;
    }
    return &it->second;
}


bool StoreGoodsBase::insert()
{
	RedisResult result(redisCmd("hvals %s:%d", getStoreKey(), mRole->getInstID()));
	int elementnum = result.getHashElements();
    for (int i = 0; i < elementnum; i++) {
        Json::Reader reader;
        Json::Value value;
        if (!reader.parse(result.readHash(i, ""), value)) {
            continue;
        }
        StoreGoods sg;
        sg.index = value["index"].asInt();
        sg.goodsId = value["id"].asInt();
		sg.goodsNum = value["num"].asInt();
        sg.type = (enStoreGoodsType)value["type"].asInt();
        sg.price = value["cost"].asInt();
        sg.isBuy = value["isBuy"].asBool();
		
		// 检查到有内容不一至,配置表有变动了
		if(!mRgCfg->checkIsIndex(sg.index, sg.goodsId))
		{
			mStoreGoodsMap.clear();
			return false;
		}
		
        mStoreGoodsMap.insert(make_pair(sg.index, sg));
    }
	return true;
}

void StoreGoodsBase::saveStoreGoodsList()
{
	doRedisCmd("del %s:%d", getStoreKey(), mRole->getInstID());
    StoreGoodsTd::iterator it;
    for (it = mStoreGoodsMap.begin(); it != mStoreGoodsMap.end(); it++) {
        saveStoreGoods(&it->second);
    }
	
    doRedisCmd("expire %s:%d %d", getStoreKey(), mRole->getInstID(), GuildCfgMgr::getGoodsCfg()->getInterval());
}

void StoreGoodsBase::saveStoreGoods(StoreGoods *sg)
{
	Json::Value value;
    value["index"] = sg->index;
    value["id"] = sg->goodsId;
    value["num"] = sg->goodsNum;
    value["type"] = sg->type;
    value["cost"] = sg->price;
    value["isBuy"] = sg->isBuy;
	
    doRedisCmd("hmset %s:%d %d %s", getStoreKey(), mRole->getInstID(), sg->index, Json::FastWriter().write(value).c_str());
}

void StoreGoodsBase::setStoreGoodsBuyIndex(StoreGoods *sg)
{
	sg->isBuy = true;
	saveStoreGoods(sg);
}


