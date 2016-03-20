/****************************************************************************
 活动：单笔一次充值大返利
 【活动时间】：读配置表
 【活动范围】：读配置表
 【活动内容】：迎新服，单笔充值大返利。
 【活动目的】：增加新服充值次数和充值金额。
 
 奖励内容：
 单笔充值300-999金钻：300金钻充值礼包
 单笔充值1000-4999金钻：1000金钻充值礼包
 单笔充值5000金钻以上：5000金钻充值礼包
 【注意事项】
 1、活动期间内，单笔一次性充值金额满足相应活动条件即可获得相应返还，每个额度的奖励都只能获得一次。
 2、活动奖励以单笔支付最高额度计算，无法累计。
 3、活动需要达到的充值金额和奖励段数可以自由配置。
 4、所返还金钻不会增加VIP经验。
 5、充值金额满足高额度奖励，但该额度奖励已经领取过了，在保证每个额度奖励都只能领取一次的前提下，发放低于充值金额的最高额度奖励。

 ****************************************************************************/

#include "DynamicConfig.h"
#include "mail_imp.h"
#include "GameLog.h"
#include "process.h"
#include "Role.h"
#include <vector>
#include <string>
#include "RedisHashJson.h"
#include "Utils.h"
#include "curl_helper.h"
#include "OnceRechargeAwardAct.h"
#include "datetime.h"

static const int ONCERECHARDAWARD = 99999999;


OnceRechargeAwardActivity g_OnceRechargeAwardActivity;

void OnceRechargeAct::init(int index, int begTime , int endTime ,const vector<AwardItem>& awards)
{
    assert(endTime >= begTime);
    assert(index >= 0);
    mIndex = index;
    mBeginTime = begTime;
    mEndTime = endTime;
    mAwards = awards;
    
    mAwardHistory.load(g_GetRedisName(rnOnceRechardAwardAct), index);
}

void OnceRechargeAct::award(int roleid,const char* rolename , int rmb )
{
    vector<AwardItem>::reverse_iterator itAward ;
    for (itAward = mAwards.rbegin(); itAward != mAwards.rend(); ++itAward)
    {
        // 从后向上找出能奖励的
        if( itAward->rmb > rmb)
            continue;
        
        // 看这个奖过了没有
        string rmbstr = xystr("%d",itAward->rmb);
        bool alreadyAward = mAwardHistory[roleid][rmbstr.c_str()].asInt() > 0;
        if(alreadyAward)
            continue;

        // 没奖过就奖这个
        mAwardHistory[roleid][rmbstr.c_str()] = itAward->rmb;
        mAwardHistory.save();
        break;
    }
    
    if( itAward == mAwards.rend())
        return;
    
    //之前是通过邮件形式发送，现在要改成领取的方式，所以要先存入数据库
    //mail(roleid , rolename , rmb ,itAward->items);
    Json::Value value;
    std::string str;
    value["mIndex"] = mIndex;
    value["rmb"] = rmb;
    value["items"] = itAward->items;
    value["rolename"] = rolename;
    value["roleid"] = roleid;
    
    int keyNum = g_OnceRechargeAwardActivity.mAllRoldAwardHistory[roleid].size();
    value["time"] = Utils::makeStr("%d",keyNum+1);
    value["hasGet"]=0;
    value["needRmb"]=xystr("%d",itAward->rmb);
    str = Json::FastWriter().write(value);
    
    g_OnceRechargeAwardActivity.mAllRoldAwardHistory[roleid][Utils::makeStr("%d",keyNum+1)] = value;
    g_OnceRechargeAwardActivity.mAllRoldAwardHistory.save();
    
}

OnceRechargeAwardActivity::~OnceRechargeAwardActivity()
{
    for (int i = 0 ; i< mActs.size() ; ++i)
    {
        SafeDelete( mActs[i]);
    }
}

void OnceRechargeAwardActivity::init()
{
//	string fullpath = Cfgable::getFullFilePath("OperateActOnceRechargeAward.ini");
    GameInifile inifile("OperateActOnceRechargeAward.ini");
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
    {
        int index = inifile.getValueT(*iter, "index", 0);
        assert(index > 0);
        
        std::string server = inifile.getValue(*iter, "server_id");
        
        vector<string> serverids = StrSpilt(server.c_str(), ";");
        
        int thisServerId = Process::env.getInt("server_id");
        std::string thisServerIdStr = Utils::makeStr("%d",thisServerId);
        std::string AllServerIdStr = Utils::makeStr("%d",ALL_SERVER_ID);

        vector<string>::iterator it=find(serverids.begin(), serverids.end(), thisServerIdStr);
        vector<string>::iterator itAllServer=find(serverids.begin(), serverids.end(), AllServerIdStr);
        if( itAllServer == serverids.end() && it==serverids.end())
            continue;
        
       int beginTime = Utils::parseDate( inifile.getValue(*iter, "startdate").c_str() );
        int endTime = Utils::parseDate( inifile.getValue(*iter, "overdate").c_str() );
//        int openServerTime = Process::env.getInt("starttime", 0);
//        int after_openserver_days = inifile.getValueT(*iter, "after_openserver_days", 0);
//        int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
//        if(time(0) > endTime || time(0) < benchmarkTime)
//            continue;
        
        OnceRechargeAct* act = new OnceRechargeAct();
        assert(act);
        
        const int awardnum = inifile.getValueT(*iter, "awardnum", 0);
        
        
        vector<OnceRechargeAct::AwardItem> awards;
        
        for (int i = 0; i < awardnum; ++i)
        {
            OnceRechargeAct::AwardItem aw;
            aw.rmb = inifile.getValueT(*iter, xystr("needrecharge%d",i+1), -1);
            aw.items = inifile.getValue(*iter, xystr("itemawards%d",i+1));
            assert( aw.rmb >=0 );
            awards.push_back(aw);
        }
        
        act->init(index, beginTime, endTime, awards);
		act -> afterOpenServerDays = inifile.getValueT(*iter, "after_openserver_days", 0);
		act->mTitle = inifile.getValue(*iter, "title");
		act->mContent = inifile.getValue(*iter, "content");
		act->mAwardType = inifile.getValueT(*iter, "award_type", 0);
		act->mChannel = inifile.getValue(*iter, "Channel");
		act->mServerId = server;
		
        mActs.push_back(act);
	}
	mAllRoldAwardHistory.load(g_GetRedisName(rnOnceRechardAwardAct), ONCERECHARDAWARD);
}

// 发送奖励，成功返回true
bool OnceRechargeAwardActivity::sendAward(Role* role ,  const string& award)
{
    if (award.empty())
    {
        xyerr("单笔奖励出错%s",award.c_str());
        return false;
    }
    vector<string> awards = StrSpilt(award.c_str(), ";");
    ItemArray items;
    return role->addAwards(awards, items, "单笔奖励");
}
void OnceRechargeAwardActivity::getAward(int roleid, int time )
{
    Role* role = SRoleMgr.GetRole(roleid);
    xyassert(role);
    
    ack_once_recharge_get_award ack;
    ack.time = time;
    Json::Value value =g_OnceRechargeAwardActivity.mAllRoldAwardHistory[roleid][Utils::makeStr("%d",time)];
    
    send_if(value["hasGet"].asInt()!=0, eOnceRechargeAwardResult_AlreadyAwarded);
    bool isBagFull = ! sendAward(role , value["items"].asString() ) ;
    send_if(isBagFull,eOnceRechargeAwardResult_BagFull);
    
    Xylog log(eLogName_OperateActOnceRechargeAward,roleid);
    log<<value["roleid"].asInt()<<value["rmb"].asInt()<<value["items"].asString();
    
    
    //成功后将数据改成领取过状态
    g_OnceRechargeAwardActivity.mAllRoldAwardHistory[roleid][Utils::makeStr("%d",time)]["hasGet"]=1;
    //g_OnceRechargeAwardActivity.mAllRoldAwardHistory.erase(roleid);
    g_OnceRechargeAwardActivity.mAllRoldAwardHistory.save();
    
    send_if(true,eOnceRechargeAwardResult_Succ);
}

void OnceRechargeAwardActivity::sendStatus(int roleid)
{
    Json::Value jsonData = g_OnceRechargeAwardActivity.mAllRoldAwardHistory[roleid];
    /*
     value["mIndex"] = mIndex;
     value["rmb"] = rmb;
     value["items"] = itAward->items;
     value["rolename"] = rolename;
     value["roleid"] = roleid;
     value["time"] = Utils::makeStr("%d",t);
     */
    ack_once_recharge_status ack;
    Json::Value::Members m = jsonData.getMemberNames();
    for (std::vector<std::string>::iterator it = m.begin(); it != m.end(); it++) {
//        log_info(*it);
//    }
//    
//    log_info(xyJsonWrite(jsonData));
//    
//    for (int i = 0; i < jsonData.size(); i++)
//    {
        Json::Value value = jsonData[*it];
        
        obj_recharge_info reInfo;
        reInfo.actIndex = value["mIndex"].asInt();
        reInfo.rmb = value["rmb"].asInt();
        reInfo.item = value["items"].asString();
        reInfo.roleid = roleid;
        reInfo.time = Utils::safe_atoi(value["time"].asString().c_str());
        reInfo.hasGet = value["hasGet"].asInt();
        reInfo.needRmb = Utils::safe_atoi(value["needRmb"].asString().c_str());
        ack.recharges.push_back(reInfo);
    }
    sendRolePacket(roleid,&ack);
}
void OnceRechargeAwardActivity::award(int roleid,const char* rolename , int rmb )
{
    int currentTime = time(0);
    int openServerTime = Process::env.getInt("starttime", 0);
    
    for (vector<OnceRechargeAct*>::iterator it = mActs.begin(); it != mActs.end(); ++it)
    {
        int after_openserver_days = (*it)->afterOpenServerDays;
        int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
        if( currentTime < (*it)->getBeginTime() || (*it)->getEndTime() < currentTime)
            continue;
        
        if(currentTime < benchmarkTime){
            continue;
        }
        
        (*it)->award(roleid,rolename,rmb );
    }
}




#pragma mark -

void onceRechargeAwardActivity(int roleid ,const char* rolename, int rmb )
{
    xyassertf(rolename,"%d - %d",roleid, rmb);
    g_OnceRechargeAwardActivity.award(roleid,rolename, rmb);
}
// 玩家领奖
void GetAward_OnceRechargeAwardAct( int roleid , int rmb )
{
    g_OnceRechargeAwardActivity.getAward(roleid, rmb);
}

// 玩家获取状态
void GetStatus_OnceRechargeAwardAct( int roleid )
{
    g_OnceRechargeAwardActivity.sendStatus(roleid);
}

