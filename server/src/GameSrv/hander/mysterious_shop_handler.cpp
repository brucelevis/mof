//
//  mysterious_shop_handler.cpp
//  GameSrv
//
//  Created by jin on 14-3-19.
//
//
#include "hander_include.h"
#include "daily_schedule_system.h"
#include "MysteriousMgr.h"
#include "datetime.h"

bool storeGoods(Role *role, GridArray &effgrids)
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
                xyerr("神秘商店 : role %d use item err %d", role->getInstID(), cfg->ReadInt("type"));
                return false;
            }
        }
        else
        {
            realEffgrids.push_back(effgrids[i]);
        }
        
    }

	// 添加物品到玩家
    if (realEffgrids.size()) {
        
        //wangzhigang 2014-11-19
//		BackBag::UpdateBackBag(role->getInstID(), realEffgrids);
        role->updateItemsChange(realEffgrids);
	}
	return true;
}

// 获取神秘商人物品列表
handler_msg(req_mysterious_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_mysterious_list ack;
    ack.errorcode = enMysteriousListReuslt_UnknowError;
    ack.nextTm = 0;
    ack.updateNum = 0;
    do
    {
		MysteriousMgr *mysMgr = role->getMysteriousMgr();
        if (!mysMgr->checkIsOpen()) {
            ack.errorcode = enMysteriousListReuslt_NoOpen;
            break;
        }
        // 更新列表
        mysMgr->checkUpdtaeList();
        // 获取列表
        StoreGoodsTd::const_iterator begin;
        for (begin = mysMgr->begin(); begin != mysMgr->end(); begin++) {
			// 去掉了筛选金钻或金币的判断，全部物品都返回 2014-10-28 by jin
            if (begin->second.isBuy) {
                continue;
            }
            obj_mysteriousInfo info;
            info.index = begin->second.index;
            info.goodsId = begin->second.goodsId;
            info.goodsNum = begin->second.goodsNum;
            info.type = begin->second.type;
            info.price = begin->second.price;
            info.isBuy = begin->second.isBuy;
            ack.info_arr.push_back(info);
        }
		ack.updateNum = mysMgr->getUpdateNum();
		ack.updateGoldNum = mysMgr->getUpdateGoldNum();
        ack.nextTm = mysMgr->getProcUpdateTime();
        ack.errorcode = enMysteriousListReuslt_Success;
		ack.time = mysMgr->getUpdateTime();
    } while (false);
    
    sendNetPacket(sessionid, &ack);
}}

// 手动刷新神秘商人列表
handler_msg(req_upMysterious_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_upMysterious_list ack;
    ack.errorcode = enManuallyMysteriousReuslt_UnknowError;
    ack.updateNum = 0;
	ack.updateGoldNum = 0;
    do
    {
		MysteriousMgr *mysMgr = role->getMysteriousMgr();
        if (!mysMgr->checkIsOpen()) {
            ack.errorcode = enManuallyMysteriousReuslt_NoOpen;
            break;
        }
        //判断刷新次数是否已经用完。
        if (mysMgr->isTimesUseUp(req.type, role->getVipLvl()))
        {
            ack.errorcode = enManuallyMysteriousReuslt_RefreshTimesIsUseUp;
            break;
        }
        // 检查手动更新消耗
		if (!mysMgr->checkDeductions(req.type)) {
			ack.errorcode = enManuallyMysteriousReuslt_NoRmb;
			break;
		}
        // 刷新列表
        mysMgr->manuallyUpdateList(req.type);
        ack.updateNum = mysMgr->getUpdateNum();
		ack.updateGoldNum = mysMgr->getUpdateGoldNum();
        ack.errorcode = enMysteriousListReuslt_Success;
    } while (false);
    sendNetPacket(sessionid, &ack);
}}

// 购买神秘商店物品
handler_msg(req_buyMysterious_goods, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_buyMysterious_goods ack;
    ack.errorcode = enBuyMysteriousReuslt_UnknowError;
    ack.indexId = req.indexId;
    do
    {
		MysteriousMgr *mysMgr = role->getMysteriousMgr();
        if (!mysMgr->checkIsOpen()) {
            ack.errorcode = enBuyMysteriousReuslt_NoOpen;
            break;
        }
        if (mysMgr->checkIsTimeout()) {
            ack.errorcode = enBuyMysteriousReuslt_Timeout;
            break;
        }
		// 时间戳不一至
		if (mysMgr->getUpdateTime() != req.time) {
            ack.errorcode = enBuyMysteriousReuslt_TimeErr;
            break;
        }
        StoreGoods *sg = mysMgr->getStoreGoods(req.indexId);
        if (sg == NULL) {
            xyerr("神秘商店 : role %d buy err index %d", roleid, req.indexId);
            ack.errorcode = enBuyMysteriousReuslt_ErrIndexId;
            break;
        }
        if (sg->isBuy) {
            ack.errorcode = enBuyMysteriousReuslt_Buy;
            break;
        }
        ItemCfgDef* cfg = ItemCfg::getCfg(sg->goodsId);
        if (NULL == cfg) {
            break;
        }
		//　检查背包空间
		ItemArray items;
		GridArray effgrids;
		items.push_back(ItemGroup(sg->goodsId, sg->goodsNum));
        
        //wangzhigang 2014-11-19
        if (role->preAddItems(items, effgrids)){
			ack.errorcode = enBuyMysteriousReuslt_BagFull;
			break;
		}
        // 扣费
        if ( role->CheckMoneyEnough(sg->price, sg->type, "神秘商店物品") != CE_OK) {
            ack.errorcode = enBuyMysteriousReuslt_NoCost;
            break;
        }
		
		if (!MysteriousMan::storeGoods(role, effgrids, "buyMysteriousGoods")) {
			break;
		}
        
        if (sg->type == enStoreGoodsType_RmbCost ||		// 金钻
            sg->type == enStoreGoodsType_ConsumeCost) {	// 金币)
            if (role->getDailyScheduleSystem()) {
                role->getDailyScheduleSystem()->onEvent(kDailyScheduleMysteriousShop, sg->goodsId, sg->goodsNum);
            }
        }

		Xylog log(eLogName_MysteriousBuy, roleid);
		log<<sg->index
		<<sg->goodsId \
		<<sg->goodsNum \
		<<sg->type \
		<<sg->price;
		
        // 记录购买标记
        mysMgr->setStoreGoodsBuyIndex(sg);
        ack.errorcode = enMysteriousListReuslt_Success;
    } while (false);
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_buyMysterious_special_goods_list, req)
{
	hand_Sid2Role_check(sessionid, roleid, role);
	ack_buyMysterious_special_goods_list ack;
	
	vector<MysteriousTimeGoodsDef*> vec;
	MysteriousCfgMgr::sCfg.getMysteriousTimeGoodsMap(vec);
    
    vector<MysteriousTimeGoodsDef*> result;
    int openServerTime = Process::env.getInt("starttime", 0);
    int now = time(NULL);
    for(vector<MysteriousTimeGoodsDef*>::iterator it = vec.begin(); it != vec.end(); it++){
        int benchmarkTime = openServerTime + (*it)-> after_openserver_days * SECONDS_PER_DAY;
        if(now > benchmarkTime){
            result.push_back(*it);
        }
    }
    
	for (vector<MysteriousTimeGoodsDef*>::iterator it = result.begin(); it != result.end(); it++) {
		obj_mysterious_special_goods obj;
		obj.index = (*it)->index;
		obj.id = (*it)->goodsId;
		obj.type = (*it)->type;
		obj.consume = (*it)->consume;
		obj.sale = (*it)->sale;
		obj.rank = (*it)->rank;
		obj.desc = (*it)->desc;
		obj.limit_num = (*it)->limit_num;
		ack.list.push_back(obj);
	}
	sendNetPacket(sessionid, &ack);
}}

handler_msg(req_buyMysterious_special_list, req)
{
	hand_Sid2Role_check(sessionid, roleid, role);
	ack_buyMysterious_special_list ack;
	MysteriousMgr *mysMgr = role->getMysteriousMgr();
	// 检查活动结束时间,　初始累计数
	mysMgr->checkclearSpecial();
	map<int, int>::const_iterator it;
	for (it = mysMgr->specialBegin(); it != mysMgr->specialEnd(); it++) {
		obj_mysterious obj;
		obj.indexId = it->first;
		obj.num = it->second;
		ack.list.push_back(obj);
	}
	ack.tm = Game::tick;
	sendNetPacket(sessionid, &ack);
}}


// 购买神秘商店限时物品
handler_msg(req_buyMysterious_special_goods, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
	MysteriousMgr *mysMgr = role->getMysteriousMgr();
	
	ack_buyMysterious_special_goods ack;
	ack.errorcode = enBuyMysteriousReuslt_UnknowError;
    ack.indexId = req.indexId;
    ack.goodNum = req.goodsNum;
    do
    {
        if (req.goodsNum <= 0) {
            break;
        }
		MysteriousTimeGoodsDef *def = MysteriousCfgMgr::sCfg.getMysteriousTimeGoodsDef(req.indexId);
		if (def == NULL) {
			break;
		}
		if (def->limit_num > 0 && !MysteriousCfgMgr::sCfg.checkSpecialTm())
		{
			ack.errorcode =  enBuyMysteriousReuslt_NoAction;
			break;
		}
		if (mysMgr->checkSpecialLimit(def->index, def->limit_num, req.goodsNum))
		{
			ack.errorcode =  enBuyMysteriousReuslt_Limit;
			break;
		}
		if (def->goodsId != req.goodsId) {  
			ack.errorcode =  enBuyMysteriousReuslt_NoMatch;
			break;
		}
		ItemCfgDef* cfg = ItemCfg::getCfg(def->goodsId);
        if (NULL == cfg) {
            break;
        }
		//　检查背包空间
		ItemArray items;
		GridArray effgrids;    
		items.push_back(ItemGroup(def->goodsId, req.goodsNum));
        
        //wangzhigang 2014-11-19
        if (role->preAddItems(items, effgrids) != CE_OK){
			ack.errorcode = enBuyMysteriousReuslt_BagFull;
			break;
		}
        // 扣费
		int needNum = def->consume * req.goodsNum;
		if (needNum <= 0 || def->consume <= 0 || needNum / def->consume != req.goodsNum) {
			break;
		}
        if (role->CheckMoneyEnough(needNum, eRmbCost, "神秘商店限时物品") != CE_OK) {
            ack.errorcode = enBuyMysteriousReuslt_NoCost;
            break;
        }
		
		if (!MysteriousMan::storeGoods(role, effgrids, "buyMysteriousGoods special")) {
			break;
		}
		
		// 记录购买次数
//		if (def->limit_num > 0) {
//			mysMgr->setSpecial(def->index);
//		}
        
        //记录物品购买数量
        if (def->limit_num > 0) {
            mysMgr->setGoodsNum(def->index, req.goodsNum);
        }
        
        if (role->getDailyScheduleSystem()) {
            role->getDailyScheduleSystem()->onEvent(kDailyScheduleMysteriousShop,
                                                    def->goodsId, req.goodsNum);
        }
        
		Xylog log(eLogName_MysteriousBuy, roleid);
		log<<def->index
		<<def->goodsId \
		<<req.goodsNum \
		<<def->type \
		<<def->consume \
		<<"限时" \
		<<MysteriousCfgMgr::sCfg.getSpecialEnd();
		
		ack.errorcode = enMysteriousListReuslt_Success;
	} while (false);
	
	sendNetPacket(sessionid, &ack);
}}

handler_msg(req_buyMysterious_exchange_goods_list, req)
{
	hand_Sid2Role_check(sessionid, roleid, role);
	ack_buyMysterious_exchange_goods_list ack;
	
	vector<MysteriousExchangeDef*> vec;
	MysteriousCfgMgr::sCfg.getExchangeGoods(vec);
    
    int now = time(NULL);
    int openServerTime = Process::env.getInt("starttime", 0);
    vector<MysteriousExchangeDef*> result;
    
    for (vector<MysteriousExchangeDef*>::iterator it = vec.begin(); it != vec.end(); it++) {
        int benchmarkTime = openServerTime + (*it)->after_openserver_days * SECONDS_PER_DAY;
        if(now > benchmarkTime){
            result.push_back(*it);
        }
    }
    
	for (vector<MysteriousExchangeDef*>::iterator it = result.begin(); it != result.end(); it++) {
		obj_mysterious_exchange_goods obj;
		obj.index = (*it)->index;
		obj.id = (*it)->goodsId;
		obj.lvlmin = (*it)->lvlMin;
		obj.lvlmax = (*it)->lvlMax;
		obj.exchange_id = (*it)->exchangeId;
		obj.num = (*it)->exchangeNum;
		obj.rank = (*it)->rank;
		obj.desc = (*it)->desc;
		obj.limit_num = (*it)->limit_num;
		ack.list.push_back(obj);
	}
	sendNetPacket(sessionid, &ack);
}}

handler_msg(req_buyMysterious_exchange_list, req)
{
	hand_Sid2Role_check(sessionid, roleid, role);
	ack_buyMysterious_exchange_list ack;
	MysteriousMgr *mysMgr = role->getMysteriousMgr();
	// 检查活动结束时间,　初始累计数
	mysMgr->checkclearExchange();
	map<int, int>::const_iterator it;
	for (it = mysMgr->exchangeBegin(); it != mysMgr->exchangeEnd(); it++) {
		obj_mysterious obj;
		obj.indexId = it->first;
		obj.num = it->second;
		ack.list.push_back(obj);
	}
	ack.tm = Game::tick;
	sendNetPacket(sessionid, &ack);
}}

// 兑换神秘商店道具
handler_msg(req_buyMysterious_exchange_goods, req)
{
	hand_Sid2Role_check(sessionid, roleid, role);
	ack_buyMysterious_exchange_goods ack;
	ack.indexId = req.indexId;
	ack.errorcode = enBuyMysteriousExchange_UnknowError;
    ack.goodNum = req.goodsNum;

	MysteriousMgr *mysMgr = role->getMysteriousMgr();
	
	do {
        if (req.goodsNum <= 0) {
            break;
        }
		if (role->getBackBag()->EmptyGridAmount() <= 0) {
			ack.errorcode = enBuyMysteriousExchange_BagFull;
			break;
		}
		const MysteriousExchangeDef *def = MysteriousCfgMgr::sCfg.getExchangeGoods(req.indexId);
		if (NULL == def) {
			break;
		}
		if (def->limit_num > 0 && !MysteriousCfgMgr::sCfg.checkExchangeTm())
		{
			ack.errorcode =  enBuyMysteriousExchange_NoAction;
			break;
		}
		if (mysMgr->checkExchangeLimit(def->index, def->limit_num, req.goodsNum))
		{
			ack.errorcode =  enBuyMysteriousExchange_Limit;
			break;
		}
		if (def->goodsId != req.goodsId) {
			ack.errorcode =  enBuyMysteriousExchange_NoMatch;
			break;
		}
		if (role->getLvl() < def->lvlMin || role->getLvl() > def->lvlMax) {
			ack.errorcode =  enBuyMysteriousExchange_LvlNoMatch;
			break;
		}
		// 扣除需求的物品
		int needNum = def->exchangeNum * req.goodsNum;
		if (needNum <= 0 || def->exchangeNum <= 0 || needNum / def->exchangeNum != req.goodsNum) {
			break;
		}
		ItemArray exchange_items;
		GridArray exchange_effgrids;
		exchange_items.push_back(ItemGroup(def->exchangeId, needNum));
		if (!role->getBackBag()->PreDelItems(exchange_items, exchange_effgrids))
		{
			ack.errorcode = enBuyMysteriousExchange_BagShortage;
			break;
		}
        

//        role->updateBackBag(exchange_effgrids, exchange_items, false, "buy_mysterious_exchange_goods");
        role->playerDeleteItemsAndStore(exchange_effgrids, exchange_items, "buy_mysterious_exchange_goods", true);
		
		// 兑换的物品, 预存储
		ItemArray items;
		GridArray effgrids;
		items.push_back(ItemGroup(def->goodsId, req.goodsNum));
		if (role->preAddItems(items, effgrids)) {
			ack.errorcode = enBuyMysteriousExchange_BagFull;
			break;
		}

//        role->updateBackBag(effgrids, items, true, "buyMysteriousGoodsexchange");
        role->playerAddItemsAndStore(effgrids, items, "buyMysteriousGoodsexchange", true);
		
		// 记录竞换物品数量
		if (def->limit_num > 0) {
			mysMgr->setExchange(def->index, req.goodsNum);
		}
        
        
        //if (role->getDailyScheduleSystem()) {
        //    role->getDailyScheduleSystem()->onEvent(kDailyScheduleMysteriousShop,
        //                                            def->goodsId, req.goodsNum);
        //}
        
		Xylog log(eLogName_MysteriousExchange, roleid);
		log<<def->index
		<<def->goodsId
		<<def->exchangeId
		<<def->exchangeNum      //* req.goodsNum  //?
		<<def->limit_num
		<<MysteriousCfgMgr::sCfg.getExchangeEnd();
		
		ack.errorcode = enBuyMysteriousExchange_Success;
	} while (false);
		
	sendNetPacket(sessionid, &ack);
}}