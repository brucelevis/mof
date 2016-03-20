 //
//  RealPvpMgr.cpp
//  GameSrv
//
//  Created by pireszhi on 14-3-21.
//
//

#include "RealPvpMgr.h"
#include "Global.h"
#include "EnumDef.h"
#include "Game.h"
#include "daily_schedule_system.h"
#include "GameLog.h"

RealPvpMgr g_RealpvpMgr;

bool matchSort(MatchModel* first, MatchModel* second)
{
    return second->battleForce < first->battleForce;
}

bool compareModel(MatchModel* first, MatchModel* second)
{
    return second->battleForce < first->battleForce;
}

bool MatchingWorker::receiveModel(MatchModel* model)
{
    model->inMatching = true;
    
    int now = Global::tick;
    
    model->enterTime = now;
    
    mQueue.push_back(model);
    
    mQueue.sort(compareModel);
    
    return true;
}

void addRealPvpMatchFailAward(MatchModel* model)
{
    if (model == NULL) {
        return;
    }
    
    int playerMedal = model->playdata.getMedal();
    int playerHonor = model->playdata.getHonour();
    
    //奖励勋章及荣誉
    SynPvpAward award = SynPvpFuntionCfg::getSynPvpMatchingFailAward(model->todayTimes);
    
    playerMedal += award.medal;
    playerHonor += award.honor;
    
    model->playdata.setMedal(playerMedal);
    model->playdata.setHonour(playerHonor);
    
    model->playdata.save();
    
    if (award.medal) {
        
        g_RealpvpMgr.sendRoleInfoChangeLog(model->roleid, model->roleName, model->playdata.getMedal(), award.medal, "medal", "synpvp_matchingfail");
    }
    
    //send message to player
    model->inMatching = false;
    model->inCombat = false;
    
    create_global(onNotifyRealpvpMatchingFail, msg);
    msg->roleid = model->roleid;
    msg->award = award;
    sendGlobalMsg(Game::MQ, msg);
}

bool MatchingWorker::matching()
{
    list<MatchModel*>::iterator beginIter = mQueue.begin();
    list<MatchModel*>::iterator targeIter = beginIter;
//    targeIter++;
    list<MatchModel*>::iterator endIter = mQueue.end();
    
    for (; beginIter != endIter;) {
        
        MatchModel* firstModel = *beginIter;
        
        //删掉没在匹配的
        if (firstModel->inMatching == false) {
            
            beginIter = mQueue.erase(beginIter);
            targeIter = beginIter;
            
            //重设enditer
            endIter = mQueue.end();
            
            continue;
        }
        
        //检查超时
        int inMatchingSec = Global::tick - firstModel->enterTime;
        int maxSec = SynPvpFuntionCfg::getMatchingSec();
        
        if (inMatchingSec > maxSec) {
            
            addRealPvpMatchFailAward(firstModel);
            
            beginIter = mQueue.erase(beginIter);
            targeIter = beginIter;
            
            //重设enditer
            endIter = mQueue.end();
            
            continue;
        }
        
        targeIter++;
        
        //target 到末尾，beginItem++;
        if (targeIter == endIter) {
            beginIter++;
            targeIter = beginIter;
            continue;
        }

        MatchModel* secondModel = *targeIter;
        
        //跳过没在等待匹配的
        if (secondModel->inMatching == false) {
            continue;
        }
        
        //检查是否符合条件
        if (checkMatchCondition(firstModel, secondModel)) {
            
            mCompetitors.push_back(make_pair(firstModel->roleid, secondModel->roleid));
            
            firstModel->inMatching = false;
            secondModel->inMatching = false;
            
            //其实本来应该是倒数完之后才算是在战斗中的，但由于这时候已相当于不能匹配了
            firstModel->inCombat = true;
            secondModel->inCombat = true;
            
            firstModel->playdata.setLastSecondEnemy(firstModel->playdata.getLastFirstEnemy());
            firstModel->playdata.setLastFirstEnemy(secondModel->roleid);
            
            secondModel->playdata.setLastSecondEnemy(secondModel->playdata.getLastFirstEnemy());
            secondModel->playdata.setLastFirstEnemy(firstModel->roleid);
            
            beginIter++;
            targeIter = beginIter;
        }
    }
    
    return true;
}

bool MatchingWorker::checkContinuousEnemy(MatchModel* model, int enemyid)
{
    int lastfirst = model->playdata.getLastFirstEnemy();
    int lastsecond = model->playdata.getLastSecondEnemy();
    
    if (lastfirst == lastsecond && lastfirst == enemyid) {
        return true;
    }
    return false;
}

bool MatchingWorker::checkMatchCondition(MatchModel* firstModel, MatchModel* secondModel)
{
    if (firstModel == NULL ||
        secondModel == NULL ||
        firstModel->roleid == secondModel->roleid) {
        return false;
    }
    
    //比较是否连续匹配到对方
    if (checkContinuousEnemy(firstModel, secondModel->roleid) || checkContinuousEnemy(secondModel, firstModel->roleid)) {
        return false;
    }
    
    int matchSec = Global::tick - firstModel->enterTime;
    
    int maxBattleGapPercent = 50;
    int maxWinRateGapPercent = 50;
    int maxHonorGapPercent = 50;
    
    SynPvpFuntionCfg::getBattleAndWinrateGapPercent(matchSec, maxBattleGapPercent, maxWinRateGapPercent, maxHonorGapPercent);
    
    //比较战斗力
    int myBattleGapPercent = abs(firstModel->battleForce - secondModel->battleForce) * 100 / (firstModel->battleForce + 1);
    
    //战斗力不超过规定的差距
    if (myBattleGapPercent > maxBattleGapPercent) {
        return false;
    }
    
    //比较胜率
    int firstRoleWinTimes = firstModel->playdata.getWinningTimes();
    int firstRoleLoseTimes = firstModel->playdata.getWinningTimes();
    
    //超过一定场次才比较胜率
    if ((firstRoleLoseTimes + firstRoleWinTimes) >= SynPvpFuntionCfg::mCfgDef.minFightingTimes) {
        
        int firstWinRate = firstModel->playdata.getWinRate();
        int secondWinRate = secondModel->playdata.getWinRate();
        
        int winRateGap = abs( firstWinRate - secondWinRate) * 100 / (firstWinRate + 1);
        
        if (winRateGap >= maxWinRateGapPercent) {
            return false;
        }
    }
    
    //比较荣誉
    int firstHonor = firstModel->playdata.getHonour();
    int secondHonor = secondModel->playdata.getHonour();
    
    int honorGapPercent = abs(firstHonor - secondHonor) * 100 / (firstHonor + 1);
    if (honorGapPercent >= maxHonorGapPercent ) {
        return false;
    }
    
    return true;
}

/*
 *
 *  RealPvpMgr
 *
 */

void RealPvpMgr::initialize()
{
    loadMgrDataFromDB();
    
    time_t nowtime = time(NULL);
    
    //判断启动服务器的时候活动是否开启
    bool tmp = SActMgr.IsActive(ae_synpvp, nowtime);
    
    if (!tmp) {
        close();
    }
    
    SActMgr.SafeCreateTimer(ae_synpvp, att_open, 0, activityOpen, "syncpvp_act");
    
    SActMgr.SafeCreateTimer(ae_synpvp, att_close, 0, activityClose, "");
    
}

//static
void RealPvpMgr::activityOpen(int actId, int number, string& params)
{
    int beginTime = Utils::safe_atoi(params.c_str());
    check_min(beginTime, 0);
    g_RealpvpMgr.open(number, beginTime);
}

//static
void RealPvpMgr::activityClose(int actId, int number, string& params)
{
    g_RealpvpMgr.close();
}

void RealPvpMgr::open(int actNum, int beginTime)
{
    if (mRunning) {
        return;
    }
    
    mRunning = true;
    
    mBeginTime = beginTime;
//    mActivityIndex = (mActivityIndex + 1) % 2;  //暂时只有两场 
    mActivityIndex = actNum;
    
    saveMgrData();
}

void RealPvpMgr::close()
{
    mRunning = false;
    
    saveMgrData();
}

int RealPvpMgr::getIndex()
{
    return mActivityIndex;
}

int RealPvpMgr::getBeginTimes()
{
    return mBeginTime;
}

void RealPvpMgr::sendRoleInfoChangeLog(int roleid, string roleName, int totalcount, int addcount, string paramType, string changeFrom)
{
    
    LogMod::addLogSynPvpMedal(roleid, roleName.c_str(), addcount, totalcount, changeFrom);
    
//    create_global(onRealpvpMedalChangeLog, msg);
//    
//    msg->roleid = roleid;
//    msg->totalcount = totalcount;
//    msg->changecount = addcount;
//    msg->type = paramType;
//    msg->changeFrom = changeFrom;
//    
//    sendGlobalMsg(Game::MQ, msg);
}

void RealPvpMgr::resetPlayerState(int roleid)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter != mAllPlayerData.end()) {
        iter->second->inCombat = false;
        iter->second->inMatching = false;
    }
}

MatchModel* RealPvpMgr::findModel(int roleid)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter != mAllPlayerData.end()) {
        return iter->second;
    }
    
    return NULL;
}
MatchModel* RealPvpMgr::CreateModel(int roleid, obj_realpvp_info& roleinfo)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter == mAllPlayerData.end()) {
        MatchModel* newModel = new MatchModel(roleid, roleinfo.battleForce);
        
        if (newModel == NULL) {
            return NULL;
        }
        
        newModel->roleName = roleinfo.rolename;
        newModel->roleType = roleinfo.roleType;
        newModel->lvl = roleinfo.roleLvl;
        
        mAllPlayerData.insert(make_pair(roleid, newModel));
        
        return newModel;
    }
    else
    {
        iter->second->battleForce = roleinfo.battleForce;
        iter->second->lvl = roleinfo.roleLvl;
        return iter->second;
    }
}

void RealPvpMgr::clearPlayerData()
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.begin();
    map<int, MatchModel*>::iterator iterend = mAllPlayerData.end();
    
    for (; iter != iterend; iter++) {
        MatchModel* model = iter->second;
        
        if (model != NULL) {
            delete model;
        }
    }
}

void RealPvpMgr::update(float dt)
{
    //匹配
    mWorker.matching();
    
    
    //处理匹配的结果
    
    //把这次的匹配结果放到Mgr里
    handleCompetition(mWorker.getCompetitors());
    
    //清理这次的匹配结果
    mWorker.clearCompetitors();
    
    delaySendBeginMsgToSynScene();
    
}

bool RealPvpMgr::throwToMatchingWorker(int roleid, obj_realpvp_info& roleinfo, int todayTimes)
{
    MatchModel* model = CreateModel(roleid, roleinfo);
    
    if (model == NULL) {
        return false;
    }
    
    if (model->inCombat || model->inMatching) {
        return false;
    }
    
    model->todayTimes = todayTimes;
    
    mWorker.receiveModel(model);
    
    return true;
}

bool RealPvpMgr::cancelMatching(int roleid)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter == mAllPlayerData.end())
    {
        return false;
    }
    
    iter->second->inMatching = false;
    
    return true;
}

////
void RealPvpMgr::safe_getMyRealpvpInfo(Role* role)
{
    create_global(GetRealpvpInfo, msg);
    msg->roleid = role->getInstID();
    msg->roleinfo.battleForce = role->getBattleForce();
    msg->roleinfo.roleLvl = role->getLvl();
    msg->roleinfo.rolename = role->getRolename();
    msg->roleinfo.roleType = role->getRoleType();
    msg->matchingIndex = role->getRealPvpMatchIndex();
    sendGlobalMsg(Global::MQ, msg);
}

void RealPvpMgr::safe_readyToRealpvpMatching(Role *role)
{
    if(role == NULL)
    {
        return;
    }
    
    create_global(ReadyToRealpvpMatching, msg);
    
    msg->roleid = role->getInstID();
    
    msg->roleinfo.battleForce = role->getBattleForce();
    
    msg->roleinfo.rolename = role->getRolename();
    
    msg->roleinfo.roleType = role->getRoleType();
    
    msg->roleinfo.roleLvl = role->getLvl();
    
//    int todayTimes = SynPvpFuntionCfg::getAwardTimes(role->getRealPvpMatchTimes());
    
    //判断是否还可以领取奖励
//    if (todayTimes) {
//        msg->canGetAward = true;
//    }
//    else
//    {
//        msg->canGetAward = false;
//    }
    
    msg->todayTimes = role->getRealPvpMatchTimes();
    
    sendGlobalMsg(Global::MQ, msg);
}

void RealPvpMgr::safe_cancelMatching(int roleid)
{
    create_global(CancelMatching, msg);
    
    msg->roleid = roleid;
    
    sendGlobalMsg(Global::MQ, msg);
}

void RealPvpMgr::safe_medalExchange(int roleid, int itemIndex, int itemCount)
{
    create_global(RealpvpMedalExchange, msg);
    msg->roleid = roleid;
    msg->itemIndex = itemIndex;
    msg->itemCount = itemCount;
    sendGlobalMsg(Global::MQ, msg);
}

void RealPvpMgr::safe_addMedal(int roleid, int addMedal)
{
    create_global(GmAddRealpvpMedal, msg);
    msg->roleid = roleid;
    msg->addmedal = addMedal;
    sendGlobalMsg(Global::MQ, msg);
}

void RealPvpMgr::safe_receiveFightingResult(int winner, int loser)
{
    create_global(HandleRealpvpFightingResult, msg);
    
    Role* winnerRole = SRoleMgr.GetRole(winner);
    Role* loserRole = SRoleMgr.GetRole(loser);
    
    if (winnerRole) {
        if (winnerRole->getDailyScheduleSystem()) {
            winnerRole->getDailyScheduleSystem()->onEvent(kDailyScheduleSyncPvp, 0, 1);
        }
    }
    
    msg->winnerid = winner;
    msg->loserid = loser;
    
    sendGlobalMsg(Global::MQ, msg);
}

void RealPvpMgr::safe_gmSetMilitary(int roleid, int Index)
{
    create_global(GmSetRealpvpMilitary, msg);
    
    msg->roleid = roleid;
    msg->index = Index;
    
    sendGlobalMsg(Global::MQ, msg);
}

void RealPvpMgr::safe_gmSetMedal(int roleid, int setMeds)
{
    create_global(GmSetRealpvpMeds, msg);
    
    msg->roleid = roleid;
    msg->setMeds = setMeds;
    
    sendGlobalMsg(Global::MQ, msg);
}

void RealPvpMgr::safe_gmSetLostTimes(int roleid, int setWinningTimes)
{
    create_global(GmSetRealpvpLostTimes, msg);
    
    msg->roleid = roleid;
    msg->setLostTimes = setWinningTimes;
    
    sendGlobalMsg(Global::MQ, msg);
}

void RealPvpMgr::safe_gmSetWinningTimes(int roleid, int setWinningTimes)
{
    create_global(GmSetRealpvpWinTimes, msg);
    
    msg->roleid = roleid;
    msg->setWinTimes = setWinningTimes;
    
    sendGlobalMsg(Global::MQ, msg);
}
void RealPvpMgr::safe_gmSetHons(int roleid, int setHons)
{
    create_global(GmSetRealpvpHons, msg);
    
    msg->roleid = roleid;
    msg->setHons = setHons;
    
    sendGlobalMsg(Global::MQ, msg);
}

void RealPvpMgr::safe_gmAddWins(int roleid, int addWins)
{
    
    create_global(GmAddRealpvpWins, msg);
    msg->roleid = roleid;
    msg->addWins = addWins;
    
    sendGlobalMsg(Global::MQ, msg);
}

bool RealPvpMgr::getMyRealpvpInfo(int roleid, int matchingIndex, obj_realpvp_info roleData, obj_realpvp_info& outData)
{
   
    MatchModel* model = g_RealpvpMgr.CreateModel(roleid, roleData);

    if (model == NULL) {
        return false;
    }
    
    //如果是不同场次，刷新敌人列表
    if (matchingIndex != getIndex()) {
        model->playdata.setLastFirstEnemy(0);
        model->playdata.setLastSecondEnemy(0);
        model->playdata.save();
    }
    
    //测试模式给 10000 medal
    if (Process::env.getInt("testmode") == 1) {
        if(model->playdata.getMedal() <= 0)
        {
            model->playdata.setMedal(10000);
            model->playdata.save();
        }
    }
    
    outData.medal = model->playdata.getMedal();
    outData.honour = model->playdata.getHonour();
    outData.winningstreak = model->playdata.getWinningStreak();
    outData.winningtimes = model->playdata.getWinningTimes();
    outData.losingtimes = model->playdata.getLosingTimes();
    outData.exittimes = model->playdata.getExitTimes();
    outData.recentgains = model->playdata.getRecentGains();
    
    return true;
}

bool RealPvpMgr::handleCompetition(vector<CompetitionMember> competitions)
{
    map<int, MatchModel*>::iterator iter;
    map<int, MatchModel*>::iterator iter2;
    map<int, MatchModel*>::iterator enditer = mAllPlayerData.end();
    
    int now = Global::tick;
    
    size_t membersize = competitions.size();
    
    if (membersize <= 0) {
        return false;
    }
    
    for (int i = 0; i < membersize; i++) {
        CompetitionMember &member = competitions[i];
        int firstRole = member.first;
        int secondRole = member.second;
        
        iter = mAllPlayerData.find(firstRole);
        iter2 = mAllPlayerData.find(secondRole);
        
        if (iter == enditer || iter2 == enditer) {
            continue;
        }
        
        create_global(onNotifyRealpvpMatchingResult, msg);
        msg->firstRoleid = firstRole;
        iter->second->getData(msg->firstRoleInfo);
        
        msg->secondeRoleid = secondRole;
        iter2->second->getData(msg->secondRoleInfo);
        
        sendGlobalMsg(Game::MQ, msg);
    }
    
    //延迟推送到同步场景
    CompetitionData data;
    data.matchTime = now;
    data.members = competitions;
    mCompetitions.push_back(data);
    
    return true;
}

void RealPvpMgr::delaySendBeginMsgToSynScene()
{
    int now = Global::tick;
    
    list<CompetitionData>::iterator iter = mCompetitions.begin();
    list<CompetitionData>::iterator enditer = mCompetitions.end();
    for (; iter != enditer;) {
        
        if ((*iter).matchTime == 0) {
            iter++;
            continue;
        }
        
        int secDelay = now - (*iter).matchTime;
        
        if (secDelay >= 3) {
            
            onSendBeginMsgToSynScene((*iter).members);
            
            iter = mCompetitions.erase(iter);
            
            enditer = mCompetitions.end();
        }
        else{
            iter++;
        }
        
    }
}

void RealPvpMgr::onSendBeginMsgToSynScene(vector<CompetitionMember>& members)
{
    int vsize = members.size();
    
    for (int i = 0; i < vsize; i++) {
        //这里还要通知同步场景线程
        create_global(onSendBeginMsgToSynScene, msg);
        msg->sceneid = 0;
        msg->sceneMod = 501227;
        msg->firstRole = members[i].first;
        msg->secondRole = members[i].second;
        
        sendGlobalMsg(Game::MQ, msg);
    }
}

void RealPvpMgr::handleFightingResult(int winnerid, int loserid, SynPvpAward& winaward, SynPvpAward& loseaward)
{
    map<int, MatchModel*>::iterator winnerIter;
    map<int, MatchModel*>::iterator loserIter;
    map<int, MatchModel*>::iterator endIter = mAllPlayerData.end();
    
    winnerIter = mAllPlayerData.find(winnerid);
    loserIter = mAllPlayerData.find(loserid);
    
    if (winnerIter == endIter && loserIter == endIter) {
        return;
    }
    
    //胜利者处理
    if (winnerIter != endIter) {
        
        MatchModel* winner = winnerIter->second;
        winner->inCombat = false;
        
        if (winner) {
            
            SynPvpAward award = SynPvpFuntionCfg::getSynPvpWinAward(winner->todayTimes);
                
            int winnerMedal = winner->playdata.getMedal();
            int winnerHonor = winner->playdata.getHonour();
            
            //奖励勋章及荣誉
            winnerMedal += award.medal;
            winnerHonor += award.honor;
            
            winner->playdata.setMedal(winnerMedal);
            winner->playdata.setHonour(winnerHonor);
            
            winaward = award;
            
            //重新设置胜利场次及胜率
            
            int winTimes = winner->playdata.getWinningTimes() + 1;
            int winStreak = winner->playdata.getWinningStreak() + 1;
            
            int winRate = 0;
            
            winner->playdata.setWinningTimes(winTimes);
            winner->playdata.setWinningStreak(winStreak);
            
            winRate = winTimes * 100 / (winTimes + winner->playdata.getLosingTimes());
            
            winner->playdata.setWinRate(winRate);
            
            //最近战绩
            int recentgains = winner->playdata.getRecentGains();
            
            recentgains = (recentgains << 1) | (0x00000001);
            winner->playdata.setRecentGains(recentgains);
            
            winner->playdata.save();
            
            if (award.medal) {
                
                sendRoleInfoChangeLog(winnerid, winner->roleName, winner->playdata.getMedal(), award.medal, "medal", "synpvp_win");
            }
            
        }
    }
    
    //失败者处理
    if (loserIter != endIter) {
        
        MatchModel* loser = loserIter->second;
        loser->inCombat = false;
        
        if (loser) {
            
            SynPvpAward award = SynPvpFuntionCfg::getSynPvpLoserAward(loser->todayTimes);
                
            int loserMedal = loser->playdata.getMedal();
            int loserHonor = loser->playdata.getHonour();
            
            loserMedal += award.medal;
            loserHonor += award.honor;
            
            loser->playdata.setMedal(loserMedal);
            loser->playdata.setHonour(loserHonor);
            
            loseaward = award;
            
            int loseTimes = loser->playdata.getLosingTimes() + 1;
            
            int winRate = 0;
            
            //重新设置胜利场次及胜率
            loser->playdata.setLosingTimes(loseTimes);
            loser->playdata.setWinningStreak(0);
            
            winRate = loser->playdata.getWinningTimes() * 100 / (loseTimes + loser->playdata.getWinningTimes());
            
            loser->playdata.setWinRate(winRate);
            
            int recentgains = loser->playdata.getRecentGains();
            
            //最近战绩
            recentgains = (recentgains << 1) & (0xfffffffe);
            loser->playdata.setRecentGains(recentgains);
            
            loser->playdata.save();
            
            if (award.medal) {
                
                sendRoleInfoChangeLog(loserid, loser->roleName, loser->playdata.getMedal(), award.medal, "medal", "synpvp_lose");
            }
        }
    }
}

int RealPvpMgr::medalExchange(int roleid, int itemIndex, int itemCount)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter == mAllPlayerData.end()) {
        return CE_REALPVP_EXCHANGE_MEDAL_NOT_ENOUGH;
    }
    
    MatchModel* playerData = iter->second;
    
    SynPvpStoreGoodsDef* itemdef = SynPvpStoreCfg::getGoodsDefByIdx(itemIndex);
    
    if (itemdef == NULL) {
        return CE_REALPVP_CAN_NOT_FIND_THIS_ITEM;
    }
    
    int myhonor = playerData->playdata.getHonour();
    
    if (itemdef->needHonor > myhonor) {
        return CE_REALPVP_EXCHANGE_HONOR_NOT_ENOUGH;
    }
    
    int myMedal = playerData->playdata.getMedal();
    int totalNeedMedal = itemdef->needMedal * itemCount;
    
    if (myMedal < totalNeedMedal) {
        return CE_REALPVP_EXCHANGE_MEDAL_NOT_ENOUGH;
    }
    
    myMedal += -totalNeedMedal;
    
    playerData->playdata.setMedal(myMedal);
    playerData->playdata.save();
    
    sendRoleInfoChangeLog(roleid, playerData->roleName, playerData->playdata.getMedal(), - totalNeedMedal, "medal", "exchange_item");
    
    return CE_OK;
}

void RealPvpMgr::addMedal(int roleid, int medalCount)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter == mAllPlayerData.end()) {
        return;
    }
    
    int mymedal = iter->second->playdata.getMedal();
    
    mymedal += medalCount;
    
    iter->second->playdata.setMedal(mymedal);
    
    iter->second->playdata.save();
}

void RealPvpMgr::gmSetMilitary(int roleid, int index)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter == mAllPlayerData.end()) {
        return;
    }
    
    MatchModel* player = iter->second;
    if (player == NULL) {
        return;
    }
    
    std::vector<int> i_military;
    
    MilitaryCfg::Iterator beg = MilitaryCfg::GetMilitaryCgfs().begin();
    MilitaryCfg::Iterator end = MilitaryCfg::GetMilitaryCgfs().end();
    for (;beg!=end;++beg){
        i_military.push_back(beg->first);
    }
    
    std::sort(i_military.begin(), i_military.end());
    player->playdata.setHonour(i_military[index]);
    player->playdata.save();
    
    /*
    Role *role = SRoleMgr.GetRole(roleid);
    obj_realpvp_info roleinfo;
    roleinfo.battleForce = role->getBattleForce();
    roleinfo.roleLvl = role->getLvl();
    roleinfo.rolename = role->getRolename();
    roleinfo.roleType = role->getRoleType();
    
    
    ack_get_realpvp_data ack;
    
    ack.errorcode = CE_OK;
    
    ack.awardTimes = SynPvpFuntionCfg::getAwardTimes(role->getRealPvpMatchTimes());
    
    ack.roleid = roleid;
    
    ack.pvpinfo = roleinfo;
    
    sendNetPacket(role->getSessionId(), &ack);
    */
}

void RealPvpMgr::gmSetLostTimes(int roleid, int setLostTimes)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter == mAllPlayerData.end()) {
        return;
    }
    
    MatchModel* player = iter->second;
    if (player == NULL) {
        return;
    }
    
    player->playdata.setLosingTimes(setLostTimes);
    player->playdata.save();
    
}

void RealPvpMgr::gmSetWinningTimes(int roleid, int setWinTimes)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter == mAllPlayerData.end()) {
        return;
    }
    
    MatchModel* player = iter->second;
    if (player == NULL) {
        return;
    }
    
    player->playdata.setWinningTimes(setWinTimes);
    player->playdata.save();
    
    /*
    Role *role = SRoleMgr.GetRole(roleid);
    obj_realpvp_info roleinfo;
    roleinfo.battleForce = role->getBattleForce();
    roleinfo.roleLvl = role->getLvl();
    roleinfo.rolename = role->getRolename();
    roleinfo.roleType = role->getRoleType();
    
    
    ack_get_realpvp_data ack;
    
    ack.errorcode = CE_OK;
    
    ack.awardTimes = SynPvpFuntionCfg::getAwardTimes(role->getRealPvpMatchTimes());
    
    ack.roleid = roleid;
    
    ack.pvpinfo = roleinfo;
    
    sendNetPacket(role->getSessionId(), &ack);
     */
}

void RealPvpMgr::gmSetMedal(int roleid, int setMedal)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter == mAllPlayerData.end()) {
        return;
    }
    
    MatchModel* player = iter->second;
    if (player == NULL) {
        return;
    }
    
    player->playdata.setMedal(setMedal);
    player->playdata.save();
    /*
    
    Role *role = SRoleMgr.GetRole(roleid);
    obj_realpvp_info roleinfo;
    roleinfo.battleForce = role->getBattleForce();
    roleinfo.roleLvl = role->getLvl();
    roleinfo.rolename = role->getRolename();
    roleinfo.roleType = role->getRoleType();
    
    
    ack_get_realpvp_data ack;
    
    ack.errorcode = CE_OK;
    
    ack.awardTimes = SynPvpFuntionCfg::getAwardTimes(role->getRealPvpMatchTimes());
    
    ack.roleid = roleid;
    
    ack.pvpinfo = roleinfo;
    
    sendNetPacket(role->getSessionId(), &ack);
    */
}

void RealPvpMgr::gmSetHonour(int roleid, int setHons)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter == mAllPlayerData.end()) {
        return;
    }
    
    MatchModel* player = iter->second;
    if (player == NULL) {
        return;
    }
    
    player->playdata.setHonour(setHons);
    player->playdata.save();
    
    /*
    Role *role = SRoleMgr.GetRole(roleid);
    obj_realpvp_info roleinfo;
    roleinfo.battleForce = role->getBattleForce();
    roleinfo.roleLvl = role->getLvl();
    roleinfo.rolename = role->getRolename();
    roleinfo.roleType = role->getRoleType();
    
    
    ack_get_realpvp_data ack;
    
    ack.errorcode = CE_OK;
    
    ack.awardTimes = SynPvpFuntionCfg::getAwardTimes(role->getRealPvpMatchTimes());
    
    ack.roleid = roleid;
    
    ack.pvpinfo = roleinfo;
    
    sendNetPacket(role->getSessionId(), &ack);
    */
}

void RealPvpMgr::gmAddWins(int roleid, int addWins)
{
    map<int, MatchModel*>::iterator iter = mAllPlayerData.find(roleid);
    
    if (iter == mAllPlayerData.end()) {
        return;
    }
    
    MatchModel* player = iter->second;
    if (player == NULL) {
        return;
    }
    
    
    int winTimes = player->playdata.getWinningTimes();
    
    int loseTimes = player->playdata.getLosingTimes();
    
    if (addWins > 0) {
        winTimes += addWins;
        player->playdata.setWinningTimes(winTimes);
    }
    else if(addWins < 0)
    {
        loseTimes += abs(addWins);
        player->playdata.setLosingTimes(loseTimes);
    }
    
    int fightTimes = winTimes + loseTimes;
    int winRate = 0;
    
    if (fightTimes >= 0) {
        winRate = winTimes * 100 / fightTimes;
    }
    
    player->playdata.setWinRate(winRate);
    
    player->playdata.save();
    
    /*
    Role *role = SRoleMgr.GetRole(roleid);
    obj_realpvp_info roleinfo;
    roleinfo.battleForce = role->getBattleForce();
    roleinfo.roleLvl = role->getLvl();
    roleinfo.rolename = role->getRolename();
    roleinfo.roleType = role->getRoleType();
    
    
    ack_get_realpvp_data ack;
    
    ack.errorcode = CE_OK;
    
    ack.awardTimes = SynPvpFuntionCfg::getAwardTimes(role->getRealPvpMatchTimes());
    
    ack.roleid = roleid;
    
    ack.pvpinfo = roleinfo;
    
    sendNetPacket(role->getSessionId(), &ack);
    */
}

void RealPvpMgr::loadMgrDataFromDB()
{
    RedisResult result( redisCmd("hmget serverdata realpvpstat realpvpindex realpvpbegintime") );
    
    mRunning = (bool)result.readHash(0, 0);
    
    mActivityIndex = result.readHash(1, 0);
    
    mBeginTime = result.readHash(2, 0);
    
}

void RealPvpMgr::saveMgrData()
{
    doRedisCmd("hmset serverdata realpvpstat %d realpvpindex %d realpvpbegintime %d", mRunning, mActivityIndex, mBeginTime);
}