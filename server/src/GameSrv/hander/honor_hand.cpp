//
//  honor_hand.cpp
//  GameSrv
//
//  Created by jin on 13-10-8.
//
//

#include "hander_include.h"
#include "Honor.h"

// 获取当前使用的称号
hander_msg(req_honor, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_honor ack;
	ack.honorId = role->getUseHonor();
	sendNetPacket(sessionid, &ack);
}}

// 获取已获得的称号
hander_msg(req_honor_info, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
	ack_honor_info ack;
	ack.type = req.type;
    vector<Honor*> vec;
	// 过期称号检查
	role->getHonorMgr()->expirationCheck();
	role->getHonorMgr()->outHonorByType((eHonorType)req.type, vec);
	for (vector<Honor*>::iterator it = vec.begin(); it != vec.end(); it++) {
		obj_honorInfo info;
		info.id = (*it)->mId;
		info.readed = (*it)->mReaded;
		info.isLimited = (*it)->isLimited();
		info.isExpire = (*it)->isExpiration();
		info.remainingTime = (*it)->getRemainingTime() + 5;	// 增加5秒延时时间
		ack.obj.push_back(info);
	}
	sendNetPacket(sessionid, &ack);
}}

// 获取未取得称号的条件值
hander_msg(req_honor_cond_info, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
	ack_honor_cond_info ack;
	vector<HonorDef*> vec;
	HonorCfgMgr::getHonorByType((eHonorType)req.type, vec);
	HonorMgr *mgr = role->getHonorMgr();
	for (vector<HonorDef*>::iterator it = vec.begin(); it != vec.end(); it++) {
		if (NULL != mgr->getHonor((*it)->id)) {
			continue;
		}
		obj_honorCondInfo info;
		info.id = (*it)->id;
		info.value = SHonorMou.handleNum((*it)->key, role, *it);
		ack.obj.push_back(info);
	}
	sendNetPacket(sessionid, &ack);
}}

// 使用称号
hander_msg(req_honor_use, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_honor_use ack;
	ack.errorcode = enUseHonroReuslt_UnknowError;
	do {
		if (!role->getHonorMgr()->useHonor(req.honorId, ack.errorcode)) {
			break;
		}
		// 属性加成处理
		role->CalcPlayerProp();
		ack.honorId = req.honorId;
		ack.errorcode = enUseHonroReuslt_Success;
		
		SHonorMou.procAppHonor(eHonorApp_WorldTalk, role);
	} while (false);
	sendNetPacket(sessionid, &ack);
}}

hander_msg(req_honor_readed, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_honor_readed ack;
	role->getHonorMgr()->cancelReaded(req.honorId);
	sendNetPacket(sessionid, &ack);
}}

hander_msg(req_honor_cancel, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_honor_cancel ack;
	role->getHonorMgr()->cancelHonor();
	// 取消属性加成
	role->CalcPlayerProp();
	
	ack.errorcode = 0;
	sendNetPacket(sessionid, &ack);
}}


