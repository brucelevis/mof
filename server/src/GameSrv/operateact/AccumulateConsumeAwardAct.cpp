/****************************************************************************
 活动：累计消费返利
 说明：因为消费有可能离线，所以玩家的数据单独抽出来，不放到role身上了
      另外，这个活动有可能经常重开，所以用一个index表示某一期活动
 时间：2014-2-17 黄坤朝
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
#include "OperateActUtils.h"
#include "curl_helper.h"
#include "AccumulateConsumeAwardAct.h"
#include "inifile.h"
#include "datetime.h"

namespace AccumulateConsumeAward
{
    void Activity::init(int index, int begTime , int endTime ,const vector<AwardItem>& awards)
    {
        assert(endTime >= begTime);
        assert(index >= 0);
        mIndex = index;
        mBeginTime = begTime;
        mEndTime = endTime;
        mAwards = awards;
        
        mAwardHistory.load(g_GetRedisName(rnAccumulateConsumeAwardAct), index);
    }
    
    // 玩家消费
    void Activity::consume(int roleid, int rmb )
    {
        assert(roleid > 0 && rmb > 0);
        mAwardHistory[roleid][ACCU_RMB] = mAwardHistory[roleid][ACCU_RMB].asInt() + rmb;
        mAwardHistory.save();
    }
    
    // 玩家领取奖励
    void Activity::getAward(int roleid, int rmb )
    {
        Role* role = SRoleMgr.GetRole(roleid);
        xyassert(role);
        
        ack_acc_consume_get_award ack;
        ack.rmb = rmb;
        
        vector<AwardItem>::const_iterator it;
        for ( it = mAwards.begin() ; it != mAwards.end(); ++it)
        {
            if(it->rmb == rmb)
                break;
        }       
        send_if(it == mAwards.end(), eAccuRechargeAwardResult_RmbError);
		
		// 检查充值是否满足
		int accuRmb = mAwardHistory[roleid][ACCU_RMB].asInt();
		send_if(rmb > accuRmb, eAccuRechargeAwardResult_RmbError);
        
        string rmbstr = xystr("%d",rmb);
        bool isAlreadyAward = mAwardHistory[roleid][rmbstr.c_str()].asInt() > 0;
        send_if(isAlreadyAward,eAccuRechargeAwardResult_AlreadyAwarded);
        
        bool isBagFull = ! sendAward(role , it->items ) ;
        send_if(isBagFull,eAccuRechargeAwardResult_BagFull);
        
        mAwardHistory[roleid][rmbstr.c_str()] = rmb;
        mAwardHistory.save();
        
        Xylog log(eLogName_OperateActAccuConsumeAward,roleid);
        log<<mIndex<<rmb<<it->items;
        
        send_if(true,eAccuRechargeAwardResult_Succ);
    }
    
    // 发送奖励，成功返回true
    bool Activity::sendAward(Role* role ,  const string& award)
    {
        if (award.empty())
        {
            xyerr("累计消费奖励出错%s",award.c_str());
            return false;
        }
        vector<string> awards = StrSpilt(award.c_str(), ";");
        ItemArray items;
		return role->addAwards(awards, items, "累计消费奖励");
    }
    
    // 发送玩家状态
    void Activity::sendStatus(int roleid)
    {
        ack_acc_consume_status ack;
        ack.actindex = mIndex;
        ack.accrmb = mAwardHistory[roleid][ACCU_RMB].asInt();
        
        vector<AwardItem>::const_iterator it;
        for ( it = mAwards.begin(); it != mAwards.end(); ++it)
        {
            string rmbstr = xystr("%d",it->rmb);
            bool isAlreadyAward = mAwardHistory[roleid][rmbstr.c_str()].asInt() > 0;
            if(isAlreadyAward)
            {
               ack.awardrmb.push_back(it->rmb);
            }
        }
        sendRolePacket(roleid,&ack);
    }
    
    
    /////////////////////////////////////////////////////////////////////////
    
    
    ActivityMgr::~ActivityMgr()
    {
        for (int i = 0 ; i< mActs.size() ; ++i)
        {
            SafeDelete( mActs[i]);
        }
    }
    
    void ActivityMgr::init()
    {
//		string fullpath = Cfgable::getFullFilePath("AccumulateConsumeAwardAct.ini");
        
        GameInifile inifile("AccumulateConsumeAwardAct.ini");
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
			int index = inifile.getValueT(*iter, "index", 0);
            assert(index > 0);
            std::string server = inifile.getValue(*iter, "server_id");
            
            vector<string> serverids = StrSpilt(server.c_str(), ";");
            
            int thisServerIdConsum = Process::env.getInt("server_id");
            std::string thisServerIdStr = Utils::makeStr("%d",thisServerIdConsum);
            std::string AllServerIdStr = Utils::makeStr("%d",ALL_SERVER_ID);
            //        if( ALL_SERVER_ID != serverid && serverid != thisServerId)
            //            continue;
            vector<string>::iterator it=find(serverids.begin(), serverids.end(), thisServerIdStr);
            vector<string>::iterator itAllServer=find(serverids.begin(), serverids.end(), AllServerIdStr);
            if( itAllServer == serverids.end() && it==serverids.end())
                continue;
            //OperateActNotForThisServer_ThenContinue();
			
            int beginTime = Utils::parseDate( inifile.getValue(*iter, "startdate").c_str() );
           int endTime = Utils::parseDate( inifile.getValue(*iter, "overdate").c_str() );
//            int openServerTime = Process::env.getInt("starttime", 0);
//            int after_openserver_days = inifile.getValueT(*iter, "after_openserver_days", 0);
//            int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
//            if(time(0) > endTime || time(0) < benchmarkTime)
//                continue;
            
            Activity* act = new Activity();
            assert(act);
            
            const int awardnum = inifile.getValueT(*iter, "awardnum", 0);
            vector<Activity::AwardItem> awards;
            
            for (int i = 0; i < awardnum; ++i)
            {
                Activity::AwardItem aw;
                aw.rmb = inifile.getValueT(*iter, xystr("needrecharge%d",i+1), -1);
                aw.items = inifile.getValue(*iter, xystr("itemawards%d",i+1));
                assert( aw.rmb >=0 && aw.items.size()>0 );
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
		
	}
    
    
    void ActivityMgr::consume(int roleid, int rmb )
    {
        xyassertf(roleid>0&&rmb>0,"%d,%d",roleid,rmb);
        
        int currentTime = time(0);
        int openServerTime = Process::env.getInt("starttime", 0);
        
        for (vector<Activity*>::iterator it = mActs.begin(); it != mActs.end(); ++it)
        {
            int after_openserver_days = (*it)->afterOpenServerDays;
            int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
            
            if( currentTime < (*it)->getBeginTime() || (*it)->getEndTime() < currentTime)
                continue;
            
            if(currentTime < benchmarkTime){
                continue;
            }
            
            (*it)->consume(roleid,rmb );
        }
    }
    
    void ActivityMgr::getAward(int roleid, int rmb)
    {
        xycheck(roleid > 0 && rmb > 0);
        
        int currentTime = time(0);
        int openServerTime = Process::env.getInt("starttime", 0);
        
        for (vector<Activity*>::iterator it = mActs.begin(); it != mActs.end(); ++it)
        {
            int after_openserver_days = (*it)->afterOpenServerDays;
            int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
            
            if( currentTime < (*it)->getBeginTime() || (*it)->getEndTime() < currentTime)
                continue;
            
            if(currentTime < benchmarkTime){
                continue;
            }
            
            (*it)->getAward(roleid,rmb );
        }
    }
    
    void ActivityMgr::sendStatus(int roleid)
    {
        int currentTime = time(0);
        int openServerTime = Process::env.getInt("starttime", 0);

        for (vector<Activity*>::iterator it = mActs.begin(); it != mActs.end(); ++it)
        {
            int after_openserver_days = (*it)->afterOpenServerDays;
            int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
            
            if( currentTime < (*it)->getBeginTime() || (*it)->getEndTime() < currentTime)
                continue;
            
            if(currentTime < benchmarkTime){
                continue;
            }

            (*it)->sendStatus(roleid);
        }
    }
    

}//namespace AccumulateConsumeAward

AccumulateConsumeAward::ActivityMgr g_AccumulateConsumeAward;



#pragma mark -

// 玩家消费
void Consume_AccuConsumeAwardAct( int roleid , int rmb )
{
    g_AccumulateConsumeAward.consume(roleid, rmb);
}

// 玩家领奖
void GetAward_AccuConsumeAwardAct( int roleid , int rmb )
{
    g_AccumulateConsumeAward.getAward(roleid, rmb);
}

// 玩家获取状态
void GetStatus_AccuConsumeAwardAct( int roleid )
{
    g_AccumulateConsumeAward.sendStatus(roleid);
}
