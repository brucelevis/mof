//
//  MonthAwardAct.cpp
//  GameSrv
//
//  Created by he kang on 14-6-21.
//
//
/****************************************************************************
 活动：月卡充值大返利
 【活动时间】：读配置表
 【活动范围】：读配置表
 【活动内容】：奖励分为30天发放完成，每天玩家上线后领取当天的奖励。
 【活动目的】：月卡，一个鼓励玩家消费的活动，玩家花费金钻购买月卡后，即可获得一系列的奖励；
 
 ****************************************************************************/
#include "MonthAwardAct.h"
#include "Role.h"

void MonthRechargeAct::init(int index,int needRmb,int begTime , int endTime ,const vector<AwardItem>& awards,int totalCount)
{
    assert(endTime >= begTime);
    assert(index >= 0);
    mIndex = index;
    mBeginTime = begTime;
    mEndTime = endTime;
    mAwards = awards;
    mCount = totalCount;
    mNeedRmb = needRmb;
    mAwardHistory.load(g_GetRedisName(rnMonthRechardAwardAct), index);
}

void MonthRechargeAct::award(int roleid, int rmb, const char* rolename, int count, int time)
{
    mAwardHistory[roleid]["count"] = count;
    for (int i = 1; i <= count; ++i) {
        mAwardHistory[roleid][xystr("%d", mIndex * 1000 + i)] = 0;
    }
	
	// 取出上次保存的金钻
	int getrmb = mAwardHistory[roleid]["getrmb"].asInt();
	mAwardHistory[roleid]["getrmb"] = getrmb + rmb;
	
    if (mNeedRmb > (getrmb + rmb)) {
        mAwardHistory[roleid]["isopen"] = 0;
        mAwardHistory[roleid]["beginTime"]= 0;
    }
    else {
        mAwardHistory[roleid]["isopen"] = 1;
        mAwardHistory[roleid]["beginTime"] = time;
		
		this->sendNotifyMsg(roleid);
    }
    mAwardHistory.save();
	
	Xylog log(eLogName_OperateActMonthOpen, roleid);
	log << mIndex << mNeedRmb << rmb << mAwardHistory[roleid]["isopen"].asInt() << getrmb;
}

void MonthRechargeAct::sendNotifyMsg(int roleid)
{
	notify_month_recharge_open notify;
	Role *role = SRoleMgr.GetRole(roleid);
	if (NULL == role) {
		return;
	}
	role->send(&notify);
}

MonthRechargeAwardActivity::~MonthRechargeAwardActivity()
{
    for (int i = 0 ; i< mActs.size() ; ++i)
    {
        SafeDelete( mActs[i]);
    }
}

void MonthRechargeAwardActivity::init()
{
//    string fullpath = Cfgable::getFullFilePath("MonthCardActivity.ini");
    GameInifile inifile("MonthCardActivity.ini");
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
    {
        int index = inifile.getValueT(*iter, "index", 0);
        assert(index > 0);
        
        MonthRechargeAct* act = new MonthRechargeAct();
        assert(act);
        
        const int totalCount = inifile.getValueT(*iter, "totalcount",0);
        const int needRmb = inifile.getValueT(*iter,"needrmb",0 );
		
		int begin = Utils::parseDate(inifile.getValue(*iter,"startdate").c_str());
        int end = Utils::parseDate(inifile.getValue(*iter,"overdate").c_str());
		
		
        vector<MonthRechargeAct::AwardItem> awards;
        MonthRechargeAct::AwardItem aw;
        aw.items = inifile.getValue(*iter, "awarditems");
        awards.push_back(aw);
        
        act->init(index,needRmb, begin, end, awards, totalCount);
        mActs.push_back(act);
    }
}

// 发送奖励，成功返回true
bool MonthRechargeAwardActivity::sendAward(Role* role, const string& award)
{
    if (award.empty())
    {
        xyerr("月卡奖励出错%s",award.c_str());
        return false;
    }
    vector<string> awards = StrSpilt(award.c_str(), ";");
    ItemArray items;
    return role->addAwards(awards, items, "月卡领取奖励");
}

void MonthRechargeAwardActivity::getAward(int roleid, int index)
{
    Role* role = SRoleMgr.GetRole(roleid);
    xyassert(role);
	
	ack_month_recharge_get_award ack;
    ack.index = index;
	send_if(index < 0, eMonthRechargeAwardResult_IndexErr);
	
    vector<MonthRechargeAct*>::iterator beg;
    for (beg = mActs.begin(); beg != mActs.end(); beg++) {
        if ((*beg)->mIndex == index / 1000) {
            break;
        }
    }
    send_if(beg == mActs.end(), eMonthRechargeAwardResult_NoActivity);
	
	Json::Value value;
    std::string strData = (*beg)->getStatusByRoleID(roleid);

	send_if(!xyJsonSafeParse(strData, value), eMonthRechargeAwardResult_DataErr);

    int distendDays = xyDiffday(value["beginTime"].asInt(), time(NULL)) + 1;
    send_if(distendDays < index % 1000, eMonthRechargeAwardResult_GetAwardError);
    send_if(value["isopen"].asInt() == 0, eMonthRechargeAwardResult_NoGet);
    send_if(value[xystr("%d", index)].asInt() == 1, eMonthRechargeAwardResult_AlreadyAwarded);
	
    vector<string> awards = StrSpilt(value["items"].asString().c_str(), ";");
    int tmpIndex = index % 1000 - 1;
    int maxIndex = awards.size() - 1;
    check_max(tmpIndex, maxIndex);
	
    send_if(!sendAward(role , awards[tmpIndex]), eMonthRechargeAwardResult_BagFull);
    
    Xylog log(eLogName_OperateActMonthRechargeAward,roleid);
    log << value["roleid"].asInt() << value["rmb"].asInt() << index << awards[tmpIndex];
    
    // 领完后，标记数据库数据，若是全领完，则移除
    (*beg)->setStatusByRoleID(roleid, index);
    send_if(true, eMonthRechargeAwardResult_Succ);
}


void MonthRechargeAwardActivity::getRmbStatus(int roleid)
{
	Role* role = SRoleMgr.GetRole(roleid);
    xyassert(role);
	
	ack_month_recharge_get_rmb ack;
	
	int now = time(NULL);
	
    for (vector<MonthRechargeAct*>::iterator it = mActs.begin(); it != mActs.end(); ++it)
    {
		if (now < (*it)->getBeginTime() || now > (*it)->getEndTime()) {
			continue;
		}
		// 只找一个月卡活动
		ack.id = (*it)->mIndex;
        ack.rmb = (*it)->getRmbStatusByRoleID(roleid);
		break;
    }
	
	role->send(&ack);
}


void MonthRechargeAwardActivity::sendStatus(int roleid)
{
    Role* role = SRoleMgr.GetRole(roleid);
    xyassert(role);
    
    ack_month_recharge_status ack;
	ack.tm = time(NULL);
	ack.errorcode = eMonthRechargeAwardResult_NoGet;
    send_if(!isOpenNow(roleid), eMonthRechargeAwardResult_NoGet);
    
	vector<MonthRechargeAct*>::iterator it;
    for (it = mActs.begin(); it != mActs.end(); it++){
		if (!(*it)->IsRunningNow(roleid)) {
			continue;
		}
        string str = (*it)->getStatusByRoleID(roleid);
        
        Json::Value tmpJson;
		if (!xyJsonSafeParse(str, tmpJson)) {
			send_if(true, eMonthRechargeAwardResult_DataErr);
		}

        // 这个时间的计算要进行处理
        int totalCount = xyDiffday(tmpJson["beginTime"].asInt(), time(NULL)) + 1;
		check_max(totalCount, tmpJson["count"].asInt());
        
        for (int k = 1; k <= totalCount; k++)
		{
			int tempIndex = (*it)->mIndex * 1000 + k;
            if (tmpJson[xystr("%d", tempIndex)].asInt() == 0) {
                ack.canGet.push_back(tempIndex);
            }
            else{
                ack.alreadGet.push_back(tempIndex);
            }
        }
		ack.id = (*it)->mIndex;
		ack.errorcode = eMonthRechargeAwardResult_Succ;
		break;
    }
    role->send(&ack);
}

void MonthRechargeAwardActivity::award(int roleid, int rmb, const char* rolename, int actTime)
{
    for (vector<MonthRechargeAct*>::iterator it = mActs.begin(); it != mActs.end(); ++it)
    {
		if (actTime < (*it)->getBeginTime() || actTime > (*it)->getEndTime()) {
			continue;
		}
		// 只找一个月卡活动
        (*it)->award(roleid, rmb, rolename, (*it)->getTotalCount(), actTime);
		break;
    }
}

#pragma mark -

void Recharge_MonthRechargeAwardAct(int roleid, const char* rolename, int rmb, int actTime)
{
    xyassertf(rolename, "%d - %d", roleid, actTime);
    if (!g_act.isOpenNow(roleid)) {
        g_act.award(roleid, rmb, rolename, actTime);
    }
}

// 玩家领奖
void GetAward_MonthRechargeAwardAct(int roleid, int index)
{
    g_act.getAward(roleid, index);
}

// 玩家月卡获取状态
void GetStatus_MonthRechargeAwardAct(int roleid)
{
    g_act.sendStatus(roleid);
}

static void AfterServerStartInitMonthRechargeAward(void * param)
{
    g_act.init();
}

// 玩家当前活动期间的充值金钻
void GetRmb_MonthRechargeAwardAct(int roleid)
{
    g_act.getRmbStatus(roleid);
}


addTimerDef(AfterServerStartInitMonthRechargeAward, NULL, 5, 1);


