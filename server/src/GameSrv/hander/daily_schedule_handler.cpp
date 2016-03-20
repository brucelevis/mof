//
//  daily_schedule_handler.cpp
//  GameSrv
//
//  Created by 丁志坚 on 11/24/14.
//
//

#include "hander_include.h"
#include "daily_schedule_system.h"
#include "daily_schedule.h"


inline bool isInOpenInterval(int value, int start, int end)
{
    return value >= start && value <= end;
}

inline bool isInCloseInterval(int value, int start, int end)
{
    return value > start && value < end;
}

handler_msg(req_daily_schedule_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_daily_schedule_list ack;
    ack.errorcode = CE_UNKNOWN;
    DailyScheduleSystem* system = role->getDailyScheduleSystem();
    if (system == NULL) {
        role->send(&ack);
        return;
    }
    
    for (DailyScheduleList::Iterator iter = system->getScheduleList().begin();
         iter != system->getScheduleList().end(); iter++) {
        DailySchedule* schedule = *iter;
        if (schedule == NULL) {
            continue;
        }
        obj_dailyschedule_info info;
        info.id = schedule->mId;
        info.progress = schedule->mProgress;
        info.type = schedule->mType;
        ack.dailyschedules.push_back(info);
    }
    
    for (DailyScheduleAwardState::Iterator iter = system->getAwardState().begin();
         iter != system->getAwardState().end(); iter++) {
        ack.gotaward.push_back(*iter);
    }
    
    ack.errorcode = CE_OK;
    ack.point = system->getPoint();
    role->send(&ack);
}}


handler_msg(req_get_daily_schedule_award, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_get_daily_schedule_award ack;
    ack.errorcode = CE_UNKNOWN;
    ack.awardid = req.awardid;
    DailyScheduleSystem* system = role->getDailyScheduleSystem();
    if (system == NULL) {
        role->send(&ack);
        return;
    }
    
    int awardId = req.awardid;
    do {
        DailyScheduleAwardCfgDef* awardCfgDef = DailyScheduleCfg::getAwardCfg(awardId);
        if (awardCfgDef == NULL) {
            ack.errorcode = CE_CAN_NOT_FIND_CFG;
            break;
        }
        
        int awardIndex = awardCfgDef->mIndex;
        if (system->getAwardState().getHaveGotAward(awardIndex)) {
            ack.errorcode = CE_HAVE_GOT_AWARD;
            break;
        }
        
        int point = awardCfgDef->mPoint;
        string awardStr = awardCfgDef->mAward;
        if (system->getPoint() < point) {
            ack.errorcode = CE_POINT_NOT_ENOUGH;
            break;
        }
        
        if (!isInOpenInterval(role->getLvl(), awardCfgDef->mStartLvl, awardCfgDef->mEndLvl)) {
            ack.errorcode = CE_UNKNOWN;
            break;
        }
        
        vector<string> awards = StrSpilt(awardStr, ";");
        ItemArray items;
        RewardStruct reward;
        
        if (!role->addAwards(awards, items, "daily_schedule_award")) {
            ack.errorcode = CE_BAG_FULL;
            break;
        }
        
        system->getAwardState().setHaveGotAward(awardId, awardIndex);
        system->save();
        
        ack.errorcode = CE_OK;
        ack.award = awardStr;
        
        Xylog log(eLogName_DailyScheduleGetAward, roleid);
        log << awardId << awardIndex << awardStr;
    }
    while (0);
    
    role->send(&ack);
}}
