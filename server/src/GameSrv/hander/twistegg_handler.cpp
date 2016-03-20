//
//  twistegg_handler.cpp
//  GameSrv
//
//  Created by jin on 15-1-7.
//
//

#include "hander_include.h"
#include "TwistEggMgr.h"

hander_msg(req_twistegg_info, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
	ack_twistegg_info ack;
	ack.tm = Game::tick;
	
	TwistEggCommonCfg::Iterator begin, end;
	TwistEggCfgMgr::getTwistEgg(begin, end, Game::tick);
	for (TwistEggCommonCfg::Iterator it = begin; it != end; it++) {
		if (it->second.checkTimeout(Game::tick)) {
			continue;
		}
		obj_twistegg_info obj;
		STwistEggMgr.getTwisteggInfo(role, it->second, obj);
		ack.infoArr.push_back(obj);
	}
	role->send(&ack);
}}

hander_msg(req_twistegg_get, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
	ack_twistegg_get ack;
	ack.errorcode = eTwisteggGetResult_UnknowError;
	do {
		// 检查
		ack.errorcode = STwistEggMgr.checkTwist(req.index, req.activityId, role, req.free, false);
		if (ack.errorcode != eTwisteggGetResult_Ok) {
			break;
		}
		// 获取
		vector<const TwistEggGoodsDef*> goods;
		ack.errorcode = STwistEggMgr.getTwistEgg(req.index, role, req.free, false, goods);
		if (ack.errorcode != eTwisteggGetResult_Ok) {
			break;
		}
		// 返回
		for (vector<const TwistEggGoodsDef*>::iterator it = goods.begin(); it != goods.end(); it++) {
			ack.result.goodsIndex = (*it)->index;
			ack.result.goodsId = (*it)->goodsId;
			ack.result.goodsNum = (*it)->goodsNum;
		}
		STwistEggMgr.getTwisteggInfo(role, req.index, ack.info);
		ack.errorcode = eTwisteggGetResult_Ok;
	}
	while (false);
	role->send(&ack);
}}

hander_msg(req_twistegg_eventwist, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
	ack_twistegg_eventwist ack;
	ack.errorcode = eTwisteggGetResult_UnknowError;
	do {
		// 检查
		ack.errorcode = STwistEggMgr.checkTwist(req.index, 0, role, false, true);
		if (ack.errorcode != eTwisteggGetResult_Ok) {
			break;
		}
		// 获取
		vector<const TwistEggGoodsDef*> goods;
		ack.errorcode = STwistEggMgr.getTwistEgg(req.index, role, false, true, goods);
		if (ack.errorcode != eTwisteggGetResult_Ok) {
			break;
		}
		// 返回
		for (vector<const TwistEggGoodsDef*>::iterator it = goods.begin(); it != goods.end(); it++) {
			obj_twistegg_result obj;
			obj.goodsIndex = (*it)->index;
			obj.goodsId = (*it)->goodsId;
			obj.goodsNum = (*it)->goodsNum;
			ack.resultArr.push_back(obj);
		}
		ack.errorcode = eTwisteggGetResult_Ok;
	}
	while (false);
	role->send(&ack);
}}

