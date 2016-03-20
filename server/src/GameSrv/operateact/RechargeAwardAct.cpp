/****************************************************************************
 活动一：单笔充值大返利
 【活动时间】：开服前5日
 【活动范围】：新服
 【活动内容】：迎新服，单笔充值大返利。
 奖励内容：
 单笔充值300-999金钻：返还15%金钻,普通攻击技能书*3
 单笔充值1000-4999金钻：返还18%金钻，2级宝石袋*3、紫色技能书*3
 单笔充值5000金钻以上：返还20%金钻
 【注意事项】
 1、活动期间内，单笔一次性充值金额满足相应活动条件即可获得相应返还，多次充值可以多次获得返还！
 2、活动奖励以单笔支付最高额度计算，无法累计。
 3、返还金钻会自动增加到角色所拥有的金钻中。
 4、所返还金钻不会增加VIP经验。
 ****************************************************************************/

#include "DynamicConfig.h"
#include "mail_imp.h"
#include "GameLog.h"
#include "process.h"
#include "Role.h"
#include "OperateActUtils.h"
#include <vector>
#include <string>
#include "curl_helper.h"
#include "RechargeAwardAct.h"
#include "datetime.h"
static const int RECHARDAWARD = 88888888;


RechargeAwardActivity g_act;

void RechargeAwardActivity::init()
{
//    string fullpath = Cfgable::getFullFilePath("OperateActRechargeAward.ini");
    GameInifile inifile("OperateActRechargeAward.ini");
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
    {
		RechargeAct act;
        act.mIndex = inifile.getValueT(*iter, "index", 0);
        act.beginTime = Utils::parseDate( inifile.getValue(*iter, "startdate").c_str() );
        act.endTime = Utils::parseDate( inifile.getValue(*iter, "overdate").c_str() );
//		int openServerTime = Process::env.getInt("starttime", 0);
//        int after_openserver_days = inifile.getValueT(*iter, "after_openserver_days", 0);
//        int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
//        if(time(0) > act.endTime || time(0) < benchmarkTime)
//            continue;
		act.afterOpenServerDays = inifile.getValueT(*iter, "after_openserver_days", 0);
        const int awardnum = inifile.getValueT(*iter, "awardnum", 0);
        
        std::string server = inifile.getValue(*iter, "server_id");
        
        vector<string> serverids = StrSpilt(server.c_str(), ";");
        
        int thisReServerId = Process::env.getInt("server_id");
        std::string thisServerIdStr = Utils::makeStr("%d",thisReServerId);
        std::string AllServerIdStr = Utils::makeStr("%d",ALL_SERVER_ID);

        vector<string>::iterator it=find(serverids.begin(), serverids.end(), thisServerIdStr);
        vector<string>::iterator itAllServer=find(serverids.begin(), serverids.end(), AllServerIdStr);
        if( itAllServer == serverids.end() && it==serverids.end())
            continue;
		
		
        
        for (int i = 0; i < awardnum; ++i)
        {
            RechardAwardItem award;
            award.rmbNeed = inifile.getValueT(*iter, xystr("needrecharge%d",i+1), -1);
            award.rmbWard = 0.0;
            award.items = inifile.getValue(*iter, xystr("itemawards%d",i+1));
            
            assert( award.rmbNeed >=0 );
            act.awards.push_back(award);
        }
		act.mTitle = inifile.getValue(*iter, "title");
		act.mContent = inifile.getValue(*iter, "content");
		act.mAwardType = inifile.getValueT(*iter, "award_type", 0);
		act.mChannel = inifile.getValue(*iter, "Channel");
		act.mServerId = server;
		
        mActs.push_back(act);
	}
    mAllRoldAwardHistory.load(g_GetRedisName(rnOnceRechardAwardAct), RECHARDAWARD);
    
}
// 发送奖励，成功返回true
bool RechargeAwardActivity::sendAward(Role* role ,  const string& award)
{
    if (award.empty())
    {
        xyerr("累计充值奖励出错%s",award.c_str());
        return false;
    }
    vector<string> awards = StrSpilt(award.c_str(), ";");
    ItemArray items;
    return role->addAwards(awards, items, "累计充值奖励");
}
void RechargeAwardActivity::getAward(int roleid, int time )
{
    
    Role* role = SRoleMgr.GetRole(roleid);
    xyassert(role);
    
    ack_re_recharge_get_award ack;
    ack.time = time;
    Json::Value value = mAllRoldAwardHistory[roleid][Utils::makeStr("%d",time)];
    send_if(value["hasGet"].asInt()==1, eOnceRechargeAwardResult_AlreadyAwarded);
    bool isBagFull = ! sendAward(role , value["items"].asString() ) ;
    send_if(isBagFull,eOnceRechargeAwardResult_BagFull);
    
    Xylog log(eLogName_OperateActOnceRechargeAward,roleid);
    log<<value["roleid"].asInt()<<value["rmb"].asInt()<<value["items"].asString();
    mAllRoldAwardHistory[roleid][Utils::makeStr("%d",time)]["hasGet"]=1;
    //mAllRoldAwardHistory.erase(roleid);
    mAllRoldAwardHistory.save();
    send_if(true,eOnceRechargeAwardResult_Succ);
}


void RechargeAwardActivity::sendStatus(int roleid)
{
    /*
     value["mIndex"]=it->mIndex；
     value["rmb"] = rmb;
     value["items"] = rit->items;
     value["rolename"] = rolename;
     value["roleid"] = roleid;
     value["time"] = Utils::makeStr("%d",t);
     str = Json::FastWriter().write(value);
     */
    
    Json::Value JsonValue = mAllRoldAwardHistory[roleid];

    ack_re_recharge_status ack;
    Json::Value::Members m = JsonValue.getMemberNames();
    for (std::vector<std::string>::iterator it=m.begin();it!=m.end();++it){
//    for (int i=0;i!=JsonValue.size() ;++i ) {
        Json::Value value = JsonValue[*it];
        obj_recharge_info reInfo;
        reInfo.actIndex = value["mIndex"].asInt();
        reInfo.rmb = value["rmb"].asInt();
        reInfo.item = value["items"].asString();
        reInfo.roleid = roleid;
        reInfo.time = Utils::safe_atoi(value["time"].asString().c_str());
        reInfo.hasGet=value["hasGet"].asInt();
        reInfo.needRmb=Utils::safe_atoi(value["needRmb"].asString().c_str());
        ack.recharges.push_back(reInfo);

    }
    sendRolePacket(roleid,&ack);
}

void RechargeAwardActivity::award(int roleid,const char* rolename , int rmb , bool isOnline)
{
    int currentTime = time(0);
    int openServerTime = Process::env.getInt("starttime", 0);
    
    for (RechargeActIter it = mActs.begin(); it != mActs.end(); ++it)
    {
        int after_openserver_days = (*it).afterOpenServerDays;
        int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
        
        if(currentTime < benchmarkTime){
            continue;
        }
        
        if( currentTime < it->beginTime || it->endTime < currentTime)
            continue;
        
        for (AwardItemRiter rit = it->awards.rbegin(); rit != it->awards.rend(); ++ rit)
        {
            if( rit->rmbNeed <= rmb )
            {
                //之前是通过邮件形式发送，现在要改成领取的方式，所以要先存入数据库
                //mail(roleid , rolename , rmb  , *rit);
                Json::Value value;
                std::string str;
                value["mIndex"] = it->mIndex;
                value["rmb"] = rmb;
                value["items"] = rit->items;
                value["rolename"] = rolename;
                value["roleid"] = roleid;
                int keyNum = g_act.mAllRoldAwardHistory[roleid].size();
                value["time"] = Utils::makeStr("%d",keyNum+1);
                value["needRmb"]=Utils::makeStr("%d",rit->rmbNeed);
                value["hasGet"]=0; 
                
                mAllRoldAwardHistory[roleid][Utils::makeStr("%d",keyNum+1)]=value;
                mAllRoldAwardHistory.save();                
                break;
            }
        }
    }
}


// item 30034*1;item 30035*2 转换成邮件内容
std::string parseItemMailContent(const char* str)
{
    if(NULL == str)
        return "";
    
    stringstream ss;
    
    vector<std::string> items = StrSpilt(str, ";");
    for (int i = 0;i< items.size(); ++i)
    {
        int id= 0 , num = 0;
        sscanf(items[i].c_str(), "item %d*%d",&id,&num);
        
        if(id == 0 || num == 0)
            continue;
        
        ItemCfgDef* itemcfg = ItemCfg::getCfg(id);
        if(NULL == itemcfg)
        {
            xyerr("parseItemMailContent , %s , id = %d", str , id);
            continue;
        }
        ss<<itemcfg->ReadStr("name")<<"*"<<num;
        if(i != items.size() -1 )
            ss<<";";
    }
    
    return ss.str();
}

#pragma mark -

void rechargeAwardActivity(int roleid ,const char* rolename, int rmb , bool isOnline)
{
    xyassertf(rolename,"%d - %d",roleid, rmb);
    g_act.award(roleid,rolename, rmb , isOnline);
}

// 玩家领奖
void GetAward_ReRechargeAwardAct( int roleid , int time )
{
    g_act.getAward(roleid, time);
}

// 玩家获取状态
void GetStatus_ReRechargeAwardAct( int roleid )
{
    g_act.sendStatus(roleid);
}

