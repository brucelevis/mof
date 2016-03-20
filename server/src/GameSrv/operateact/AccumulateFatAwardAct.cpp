//
//  AccumulateFatAwardAct.cpp
//  GameSrv
//
//  Created by haifeng on 14-12-29.
//
//

#include "AccumulateFatAwardAct.h"

#include "DynamicConfig.h"
#include "mail_imp.h"
#include "GameLog.h"
#include "process.h"
#include "Utils.h"
#include "OperateActUtils.h"
#include "curl_helper.h"
#include "inifile.h"
#include "datetime.h"

namespace AccumulateFatAwardAct
{
    // FatActivity
    void FatActivity::init(int index, int beginTime, int endTime, const vector<AwardItem>& awards)
    {
        assert(endTime >= beginTime);
        assert(index >= 0);
        mIndex = index;
        mBeginTime = beginTime;
        mEndTime = endTime;
        mAwards = awards;
        
        mFatAwardHistory.load(g_GetRedisName(rnAccumulateFatAwardAct), index);
    }
    
    // 玩家消费
    void FatActivity::consume(int roleid, int fat)
    {
        assert(roleid > 0 && fat > 0);
        mFatAwardHistory[roleid][ACCU_FAT] = mFatAwardHistory[roleid][ACCU_FAT].asInt() + fat;
        mFatAwardHistory.save();
    }
    
    // 玩家领取奖励
    void FatActivity::getAward(int roleid, int fat)
    {
        Role* role = SRoleMgr.GetRole(roleid);
        xyassert(role);
        
        ack_consumefat_get_award ack;
        ack.fat = fat;
        
        vector<AwardItem>::const_iterator it;
        for (it = mAwards.begin(); it != mAwards.end(); ++it) {
            if (it->fat == fat) {
                break;
            }
        }
        
        send_if(it == mAwards.end(), eConsumeFatAwardResult_FatError);
        
        // 检查所消耗的精力是否满足
        int accuFat = mFatAwardHistory[roleid][ACCU_FAT].asInt();
        send_if(fat > accuFat, eConsumeFatAwardResult_FatError);
        
        // 是否已领过
        string fatstr = xystr("%d", fat);
        bool isAlreadyAward = mFatAwardHistory[roleid][fatstr.c_str()].asInt() > 0;
        send_if(isAlreadyAward, eConsumeFatAwardResult_AlreadyAwarded);
        
        // 背包是否已满
        bool isBagFull = ! sendAward(role, it->items);
        send_if(isBagFull, eConsumeFatAwardResult_BagFull);
        
        mFatAwardHistory[roleid][fatstr.c_str()] = fat;
        mFatAwardHistory.save();
        
        Xylog log(eLogName_OperateActConsumeFatAward, roleid);
        log<<mIndex<<fat<<it->items;
        
        send_if(true, eConsumeFatAwardResult_Succ);
    }
    
    // 发送奖励，成功返回true
    bool FatActivity::sendAward(Role* role, const string& award)
    {
        if (award.empty()) {
            xyerr("消耗精力领奖励出错%s", award.c_str());
            return false;
        }
        vector<string> awards = StrSpilt(award.c_str(), ";");
        ItemArray items;
        return role->addAwards(awards, items, "消耗精力奖励");
        
    }
    
    // 发送玩家状态
    void FatActivity::sendStatus(int roleid)
    {
        ack_consumefat_award_status ack;
        ack.activityid = mIndex;
        ack.consumefat = mFatAwardHistory[roleid][ACCU_FAT].asInt();
        
        vector<AwardItem>::const_iterator it;
        for (it = mAwards.begin(); it != mAwards.end(); ++it) {
            string fatstr = xystr("%d", it->fat);
            bool isAlreadyAward = mFatAwardHistory[roleid][fatstr.c_str()].asInt() > 0;
            if (isAlreadyAward) {
                ack.awardfat.push_back(it->fat);
            }
        }
        sendRolePacket(roleid, &ack);
    }
    
    // ActivityMgr
    ActivityMgr::~ActivityMgr()
    {
        for (int i = 0; i < mActs.size(); ++i) {
            SafeDelete(mActs[i]);
        }
    }
    
    void ActivityMgr::init()
    {
//        string fullpath = Cfgable::getFullFilePath("ConsumeFatAwardAct.ini");
        GameInifile inifile("ConsumeFatAwardAct.ini");
        list<string> sections;
        inifile.getSections(back_inserter(sections));
        
        for (list<string>::iterator iter = sections.begin(); iter != sections.end(); ++iter) {
            int index = inifile.getValueT(*iter, "index", 0);
            assert(index > 0);
            string server = inifile.getValue(*iter, "server_id");
            
            vector<string> serverids = StrSpilt(server.c_str(), ";");
            
            int thisSserverIdConsum = Process::env.getInt("server_id");
            string thisServerIdStr = Utils::makeStr("%d", thisSserverIdConsum);
            string AllServerIdStr = Utils::makeStr("%d", ALL_SERVER_ID);
            
            vector<string>::iterator it = find(serverids.begin(), serverids.end(), thisServerIdStr);
            vector<string>::iterator itAllServer = find(serverids.begin(), serverids.end(), AllServerIdStr);
            if (itAllServer == serverids.end() && it == serverids.end()) {
                continue;
            }
            
            int beginTime = Utils::parseDate(inifile.getValue(*iter, "startdate").c_str());
            int endTime = Utils::parseDate(inifile.getValue(*iter, "overdate").c_str());
//            int openServerTime = Process::env.getInt("starttime", 0);
//            int after_openserver_days = inifile.getValueT(*iter, "after_openserver_days", 0);
//            int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
//            if(time(0) > endTime || time(0) < benchmarkTime)
//                continue;
            
            FatActivity* act = new FatActivity();
            assert(act);
            
            const int awardnum = inifile.getValueT(*iter, "awardnum", 0);
            vector<FatActivity::AwardItem> awards;
            
            for (int i = 0; i < awardnum; ++i) {
                FatActivity::AwardItem aw;
                aw.fat = inifile.getValueT(*iter, xystr("needrecharge%d", i + 1), -1);
                aw.items = inifile.getValue(*iter, xystr("itemawards%d", i + 1));
                assert(aw.fat >= 0 && aw.items.size() > 0);
                awards.push_back(aw);
            }
            
            act->init(index, beginTime, endTime, awards);
            act->afterOpenServerDays = inifile.getValueT(*iter, "after_openserver_days", 0);
            act->mTitle = inifile.getValue(*iter, "title");
            act->mContent = inifile.getValue(*iter, "content");
            act->mAwardType = inifile.getValueT(*iter, "award_type", 0);
            act->mChannel = inifile.getValue(*iter, "Channel");
            act->mServerId = server;
            
            mActs.push_back(act);
        }
    }
    
    void ActivityMgr::consume(int roleid, int fat)
    {
        xyassertf(roleid > 0 && fat > 0, "%d,%d", roleid, fat);
        int currentTime = time(0);
        int openServerTime = Process::env.getInt("starttime", 0);
        
        for (vector<FatActivity*>::iterator it = mActs.begin(); it != mActs.end(); ++it) {
            
            int after_openserver_days = (*it)->afterOpenServerDays;
            int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
            
            if (currentTime < (*it)->getBeginTime() || (*it)->getEndTime() < currentTime) {
                continue;
            }
            
            if(currentTime < benchmarkTime){
                continue;
            }
            
            (*it)->consume(roleid, fat);
        }
    }
    
    void ActivityMgr::getAward(int roleid, int fat)
    {
        xycheck(roleid > 0 && fat > 0);
        int currentTime = time(0);
        int openServerTime = Process::env.getInt("starttime", 0);
        
        for (vector<FatActivity*>::iterator it = mActs.begin(); it != mActs.end(); ++it) {
            int after_openserver_days = (*it)->afterOpenServerDays;
            int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
            
            if (currentTime < (*it)->getBeginTime() || (*it)->getEndTime() < currentTime) {
                continue;
            }
            
            if(currentTime < benchmarkTime){
                continue;
            }
            
            (*it)->getAward(roleid, fat);
        }
    }
    
    void ActivityMgr::sendStatus(int roleid)
    {
        int currentTime = time(0);
        int openServerTime = Process::env.getInt("starttime", 0);

        for (vector<FatActivity*>::iterator it = mActs.begin(); it != mActs.end(); ++it) {
            int after_openserver_days = (*it)->afterOpenServerDays;
            int benchmarkTime = openServerTime + after_openserver_days * SECONDS_PER_DAY;
            
            if (currentTime < (*it)->getBeginTime() || (*it)->getEndTime() < currentTime) {
                continue;
            }
            
            if(currentTime < benchmarkTime){
                continue;
            }
            
            (*it)->sendStatus(roleid);
        }
    }

}

AccumulateFatAwardAct::ActivityMgr g_AccumulateConsumeFatAward;

#pragma mark -

// 玩家消费
void Consume_AccuConsumeFatAwardAct(int roleid, int fat)
{
    g_AccumulateConsumeFatAward.consume(roleid, fat);
}

// 玩家领奖
void GetAward_AccuConsumeFatAwardAct(int roleid, int fat)
{
    g_AccumulateConsumeFatAward.getAward(roleid, fat);
}

// 玩家获取状态
void GetStatus_AccuConsumeFatAwardAct(int roleid)
{
    g_AccumulateConsumeFatAward.sendStatus(roleid);
}





