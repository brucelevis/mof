//
//  operact_handler.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 14-1-1.
//
//  运营活动

#include "hander_include.h"
#include "gift_activity.h"
#include "RechargeAwardAct.h"
#include "OnceRechargeAwardAct.h"
#include "AccumulateRechargeAwardAct.h"
#include "AccumulateConsumeAwardAct.h"
#include "AccumulateFatAwardAct.h"
#include "InviteCodeAct.h"
#include "datetime.h"

extern void GetAward_OnceRechargeAwardAct( int roleid , int rmb );
extern void GetStatus_OnceRechargeAwardAct( int roleid );

hander_msg(req_once_recharge_status, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetStatus_OnceRechargeAwardAct(roleid);
}}

hander_msg(req_once_recharge_get_award, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetAward_OnceRechargeAwardAct(roleid,req.time);
}}

extern void GetAward_ReRechargeAwardAct( int roleid , int time );
extern void GetStatus_ReRechargeAwardAct( int roleid );

hander_msg(req_re_recharge_status, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetStatus_ReRechargeAwardAct(roleid);
}}

hander_msg(req_re_recharge_get_award, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetAward_ReRechargeAwardAct(roleid,req.time);
}}

extern void GetAward_MonthRechargeAwardAct( int roleid , int index );
extern void GetStatus_MonthRechargeAwardAct( int roleid );
extern void GetRmb_MonthRechargeAwardAct(int roleid);

hander_msg(req_month_recharge_status, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetStatus_MonthRechargeAwardAct(roleid);
}}

hander_msg(req_month_recharge_get_award, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetAward_MonthRechargeAwardAct(roleid,req.index);
}}

hander_msg(req_month_recharge_get_rmb, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetRmb_MonthRechargeAwardAct(roleid);
}}



extern void GetAward_AccuRechargeAwardAct( int roleid , int rmb );
extern void GetStatus_AccuRechargeAwardAct( int roleid );

hander_msg(req_acc_recharge_status, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetStatus_AccuRechargeAwardAct(roleid);
}}

hander_msg(req_acc_recharge_get_award, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetAward_AccuRechargeAwardAct(roleid,req.rmb);
}}



extern void GetAward_AccuConsumeAwardAct( int roleid , int rmb );
extern void GetStatus_AccuConsumeAwardAct( int roleid );

hander_msg(req_acc_consume_status, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetStatus_AccuConsumeAwardAct(roleid);
}}

hander_msg(req_acc_consume_get_award, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetAward_AccuConsumeAwardAct(roleid,req.rmb);
}}


extern void shareAwardAll(Role *role, ack_shareaward_state &ack);

// 分享奖励
hander_msg(req_shareaward_state, req)
{
	hand_Sid2Role_check(sessionid, roleid, role);
	ack_shareaward_state ack;
	shareAwardAll(role, ack);
}}

hander_msg(req_invite_status,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SInviteCodeMgr.sendStatus(roleid);
}}

hander_msg(req_invite_get_award,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SInviteCodeMgr.getAward(roleid,req.index);
}}

hander_msg(req_invite_input_code,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SInviteCodeMgr.inputCode(roleid, req.code);
}}

hander_msg(req_gift_activity,req)
{
	ack_gift_activity ack;
	ack.errorcode = 0;
	
	map<int, GiftActivity*>::iterator it;
	for (it = g_GiftActivityMgr.mActivities.begin();
		 it != g_GiftActivityMgr.mActivities.end(); it++) {
		obj_gift_activity obj;
		obj.index = it->second->mActId;
		obj.startdate = it->second->mStartTime;
		obj.overdate = it->second->mEndTime;
        
        int now = time(NULL);
        int openServerTime = Process::env.getInt("starttime", 0);
        int benchmarkTime = openServerTime + it -> second -> afterOpenServerDays * SECONDS_PER_DAY;
        if( now > it -> second -> mEndTime || now < it -> second -> mStartTime){
            continue;
        }
        
        if(now < benchmarkTime){
            continue;
        }
        
		obj.title = it->second->mTitle;
		obj.content = it->second->mContent;
		ack.giftActivity.push_back(obj);
	}
	sendNetPacket(sessionid, &ack);
}}

hander_msg(req_server_time,req)
{
	ack_server_time ack;
	ack.time = Game::tick;
	sendNetPacket(sessionid, &ack);
}}

hander_msg(req_operate_act_recharge_award,req)
{
	ack_operate_act_recharge_award ack;
	ack.errorcode = 0;
	
	vector<RechargeAct> &temp = g_act.getActs();
	vector<RechargeAct>::iterator it;
	for (it = temp.begin(); it != temp.end(); it++) {
		obj_operate_recharge obj;
		
		obj.index = it->mIndex;
		obj.server_id = it->mServerId;
		obj.Channel = it->mChannel;
		obj.startdate = it->beginTime;
		obj.overdate = it->endTime;
		obj.award_type = it->mAwardType;
		obj.awardnum = it->awards.size();
        
        int now = time(NULL);
        int openServerTime = Process::env.getInt("starttime", 0);
        int benchmarkTime = openServerTime + it -> afterOpenServerDays * SECONDS_PER_DAY;
        if( now > it -> endTime || now < it -> beginTime){
            continue;
        }
        
        if(now < benchmarkTime){
            continue;
        }

		
		Json::Value result;
		for(int i = 0; i < it->awards.size(); i++)
		{
			Json::Value value;
			value["need"] = it->awards[i].rmbNeed;
			value["award"] = it->awards[i].items;
			result.append(value);
		}
		
		obj.configJson = xyJsonWrite(result);
		
		obj.title = it->mTitle;
		obj.content = it->mContent;
		
		ack.operateRecharge.push_back(obj);
	}
	sendNetPacket(sessionid, &ack);
}}

hander_msg(req_operate_act_once_recharge_award,req)
{
	ack_operate_act_once_recharge_award ack;
	ack.errorcode = 0;
	
	vector<OnceRechargeAct*> &temp = g_OnceRechargeAwardActivity.getActs();
	vector<OnceRechargeAct*>::iterator it;
	for (it = temp.begin(); it != temp.end(); it++) {
		obj_operate_once_recharge obj;
		OnceRechargeAct *act = *it;
		
		obj.index = act->mIndex;
		obj.server_id = act->mServerId;
		obj.Channel = act->mChannel;
		obj.startdate = act->mBeginTime;
		obj.overdate = act->mEndTime;
		obj.award_type = act->mAwardType;
		obj.awardnum = act->mAwards.size();
        
        int now = time(NULL);
        int openServerTime = Process::env.getInt("starttime", 0);
        int benchmarkTime = openServerTime + act -> afterOpenServerDays * SECONDS_PER_DAY;
        
        if( now > act -> mEndTime || now < act -> mBeginTime){
            continue;
        }
        
        if(now < benchmarkTime){
            continue;
        }
		
		Json::Value result;
		for(int i = 0; i < act->mAwards.size(); i++)
		{
			Json::Value value;
			value["need"] = act->mAwards[i].rmb;
			value["award"] = act->mAwards[i].items;
			result.append(value);
		}
		
		obj.configJson = xyJsonWrite(result);
		
		obj.title = act->mTitle;
		obj.content = act->mContent;
		
		ack.operateOnceRecharge.push_back(obj);
	}
	sendNetPacket(sessionid, &ack);

}}

hander_msg(req_accumulate_recharge_awardact,req)
{
	ack_accumulate_recharge_awardact ack;
	ack.errorcode = 0;
	
	vector<AccumulateRechargeAwardAct::Activity*> &temp = g_AccumulateRechargeAwardAct.getActs();
	vector<AccumulateRechargeAwardAct::Activity*>::iterator it;
	for (it = temp.begin(); it != temp.end(); it++) {
		obj_accumulate_recharge obj;
		AccumulateRechargeAwardAct::Activity *act = *it;
		
		obj.index = act->mIndex;
		obj.server_id = act->mServerId;
		obj.Channel = act->mChannel;
		obj.startdate = act->mBeginTime;
		obj.overdate = act->mEndTime;
		obj.award_type = act->mAwardType;
		obj.awardnum = act->mAwards.size();

        
        int now = time(NULL);
        int openServerTime = Process::env.getInt("starttime", 0);
        int benchmarkTime = openServerTime + act -> afterOpenServerDays * SECONDS_PER_DAY;
        if( now > act -> mEndTime || now < act -> mBeginTime){
            continue;
        }
        
        if(now < benchmarkTime){
            continue;
        }
		
		Json::Value result;
		for(int i = 0; i < act->mAwards.size(); i++)
		{
			Json::Value value;
			value["need"] = act->mAwards[i].rmb;
			value["award"] = act->mAwards[i].items;
			result.append(value);
		}
		
		obj.configJson = xyJsonWrite(result);

		obj.title = act->mTitle;
		obj.content = act->mContent;
		
		ack.accumulateRecharge.push_back(obj);
	}
	sendNetPacket(sessionid, &ack);
}}



hander_msg(req_accumulate_consume_awardact,req)
{
	ack_accumulate_consume_awardact ack;
	ack.errorcode = 0;
	
	vector<AccumulateConsumeAward::Activity*> &temp = g_AccumulateConsumeAward.getActs();
	vector<AccumulateConsumeAward::Activity*>::iterator it;
	for (it = temp.begin(); it != temp.end(); it++) {
		obj_accumulate_consume obj;
		AccumulateConsumeAward::Activity *act = *it;
		
		obj.index = act->mIndex;
		obj.server_id = act->mServerId;
		obj.Channel = act->mChannel;
		obj.startdate = act->mBeginTime;
		obj.overdate = act->mEndTime;
		obj.award_type = act->mAwardType;
		obj.awardnum = act->mAwards.size();
        
        int now = time(NULL);
        int openServerTime = Process::env.getInt("starttime", 0);
        int benchmarkTime = openServerTime + act -> afterOpenServerDays * SECONDS_PER_DAY;
        if( now > act -> mEndTime || now < act -> mBeginTime){
            continue;
        }
        
        if(now < benchmarkTime){
            continue;
        }
		
		Json::Value result;
		for(int i = 0; i < act->mAwards.size(); i++)
		{
			Json::Value value;
			value["need"] = act->mAwards[i].rmb;
			value["award"] = act->mAwards[i].items;
			result.append(value);
		}
		
		obj.configJson = xyJsonWrite(result);
		
		obj.title = act->mTitle;
		obj.content = act->mContent;
		
		ack.accumulateConsume.push_back(obj);
	}
	sendNetPacket(sessionid, &ack);
}}
hander_msg(req_fat_award_activity, req)
{
    ack_fat_award_activty ack;
    ack.errorcode = 0;
    
    vector<AccumulateFatAwardAct::FatActivity*> &temp = g_AccumulateConsumeFatAward.getActs();
    vector<AccumulateFatAwardAct::FatActivity*>::iterator it;
    for (it = temp.begin(); it != temp.end(); ++it) {
        obj_fat_award_activity obj;
        AccumulateFatAwardAct::FatActivity *act = *it;
        
        obj.index = act->mIndex;
        obj.server_id = act->mServerId;
        obj.Channel = act->mChannel;
        obj.startdate = act->mBeginTime;
        obj.overdate = act->mEndTime;
        obj.award_type = act->mAwardType;
        obj.awardnum = act->mAwards.size();
        
        int now = time(NULL);
        int openServerTime = Process::env.getInt("starttime", 0);
        int benchmarkTime = openServerTime + act -> afterOpenServerDays * SECONDS_PER_DAY;
        if( now > act -> mEndTime || now < act -> mBeginTime){
            continue;
        }
        
        if(now < benchmarkTime){
            continue;
        }
        
		Json::Value result;
		for(int i = 0; i < act->mAwards.size(); i++)
		{
			Json::Value value;
			value["need"] = act->mAwards[i].fat;
			value["award"] = act->mAwards[i].items;
			result.append(value);
		}
		
		obj.configJson = xyJsonWrite(result);
        
        obj.title = act->mTitle;
        obj.content = act->mContent;
        
        ack.fat_activity.push_back(obj);
    }
    sendNetPacket(sessionid, &ack);
}}
extern void GetAward_AccuConsumeFatAwardAct(int roleid , int fat);
extern void GetStatus_AccuConsumeFatAwardAct(int roleid );
hander_msg(req_consumefat_award_status, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetStatus_AccuConsumeFatAwardAct(roleid);
}}
hander_msg(req_consumefat_get_award, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    GetAward_AccuConsumeFatAwardAct(roleid, req.fat);
}}
