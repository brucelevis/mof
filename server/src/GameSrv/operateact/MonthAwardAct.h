//
//  MonthAwardAct.h
//  GameSrv
//
//  Created by mac on 14-6-21.
//
//

#ifndef __GameSrv__MonthAwardAct__
#define __GameSrv__MonthAwardAct__

#include <iostream>

#include "DynamicConfig.h"
#include "mail_imp.h"
#include "GameLog.h"
#include "process.h"

#include <vector>
#include <string>
#include "RedisHashJson.h"
#include "Utils.h"

class Role;

// 一场活动
class MonthRechargeAct
{
public:
    int mIndex;      // 活动唯一标记
    struct AwardItem
    {
        string items;   // 奖励物品
    };
    
    void init(int index, int needRmb,int begTime , int endTime,const vector<AwardItem>& awards,int totalCount);
    
    int getBeginTime() { return mBeginTime;}
    int getEndTime() { return mEndTime; }
    
    // 尝试奖励
    void award(int roleid,int rmb,const char* rolename ,int count,int time);
    int getTotalCount(){return this->mCount;}
    string getStatusByRoleID(int roleid)
	{
        Json::Value value;
        
        value["mIndex"] = mIndex;
        value["roleid"] = roleid;
        value["items"] = mAwards.begin()->items;
		value["count"] = mCount;
        
        for (int i = 1; i <= mCount; i++) {
			string index = xystr("%d", mIndex * 1000 + i);
			
            value[index] = mAwardHistory[roleid][index];
            value["isopen"] = mAwardHistory[roleid]["isopen"];
            value["beginTime"] = mAwardHistory[roleid]["beginTime"];
        }
        return xyJsonWrite(value);
    }
	
	int getRmbStatusByRoleID(int roleid)
	{
		return mAwardHistory[roleid]["getrmb"].asInt();
	}
    
    void setStatusByRoleID(int roleid, int index)
	{
        mAwardHistory[roleid][xystr("%d", index)] = 1;
        mAwardHistory.save();
        
        int actIndex = index / 1000;
        int k = 0;
        for (; k != mCount; ++k) {
            int indexKey = actIndex * 1000 + k + 1;
            if(mAwardHistory[roleid][xystr("%d", indexKey)].asInt() == 0){
                break;
            }
        }
        if (k == mCount) {
            mAwardHistory.erase(roleid);
			mAwardHistory.save();
			
			Xylog log(eLogName_OperateActMonthClose, roleid);
			log << actIndex;
        }
    }
	
    bool IsRunningNow(int roleid)
	{
        if (mAwardHistory.exist(roleid)) {
            if (mAwardHistory[roleid]["isopen"].asInt() == 1) {
                return true;
            }
        }
        return false;
    }
    
    void cleanRoldDate(int roleid){
        mAwardHistory.erase(roleid);
        mAwardHistory.save();
    }
	
	void sendNotifyMsg(int roleid);
protected:
    
    int mBeginTime;  // 开始时间
    int mEndTime;    // 结束时间
    int mCount;         //奖励总共返回天数
    int mNeedRmb;      //要求的充值数
    vector<AwardItem>   mAwards; // 奖励内容
    
    typedef RedisHashJson<int>  AwardHistory;
    AwardHistory mAwardHistory; // 玩家领过奖历史
};


class MonthRechargeAwardActivity
{
public:
    ~MonthRechargeAwardActivity();
    void init();    // 读配置
    void award(int roleid,int rmb, const char* rolename, int actTime); // 发奖
    void getAward(int roleid, int time); // 领奖
    void sendStatus(int roleid); // 发送当前状态
	void getRmbStatus(int roleid);
    void cleanRoldData(int roleid)
	{
		for (vector<MonthRechargeAct*>::iterator it = mActs.begin(); it != mActs.end(); ++it)
		{
			(*it)->cleanRoldDate(roleid);
		}
    }
	
    bool isOpenNow(int roleid){
		for (vector<MonthRechargeAct*>::iterator it = mActs.begin(); it != mActs.end(); ++it)
		{
			if ((*it)->IsRunningNow(roleid)) {
				return true;
			}
		}
		return false;
    }
	
protected:
    vector<MonthRechargeAct*> mActs;  // 该服不同时间段的活动
    bool sendAward(Role* role ,  const string& award);
};

static MonthRechargeAwardActivity g_act;

void gm_cleanRoleData(int roleid)
{
    g_act.cleanRoldData(roleid);
}
#endif /* defined(__GameSrv__MonthAwardAct__) */
