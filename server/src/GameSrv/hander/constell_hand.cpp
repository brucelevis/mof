//
//  constell_hand.cpp
//  GameSrv
//
//  Created by 志坚 on 13-3-12.
//  Copyright (c) 2013年 __MyCompanyName__. All rights reserved.
//

#include "hander_include.h"
#include "Constellation.h"
#include "Honor.h"
#include "Paihang.h"

// 获取星座系统状态
hander_msg(req_constellstate, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    role->getConstellationMgr()->sendConstellState();
}}


// 星座升级
hander_msg(req_constellupg, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
	ack_constellupg ack;
    ack.errorcode = eConstellupgResult_UnknowError;
	
    do {
		eConstellupgResult ret = role->getConstellationMgr()->UpgConstell(req.constellid, ack.constellstep);
		if (ret != eConstellupgResult_Ok) {
			break;
		}
		
		role->CalcPlayerProp();
		role->UploadPaihangData(eUpdate);
		
		// 称号检查
		SHonorMou.procHonor(eHonorKey_Cons, role);
        
        //点亮星灵任务更新
        UpdateQuestProgress("constellupg", req.constellid, 1, role, true);
		
		ack.constellval = role->getConstellVal();
		role->getConstellationMgr()->getFailtimes(ack.constellTimes);
        ack.errorcode = eConstellupgResult_Ok;
        ack.constellid = req.constellid;
    } while (false);
	
	sendNetPacket(sessionid, &ack);
}}