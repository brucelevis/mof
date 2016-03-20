//
//  Pvp.cpp
//  GameSrv
//
//  Created by pireszhi on 13-4-17.
//
//

#include "Pvp.h"
#include "main.h"
#include "DBRolePropName.h"
#include "Global.h"
#include "Game.h"
#include "GameScript.h"
#include "ItemHelper.h"
#include "mail_imp.h"

INSTANTIATE_SINGLETON(PvpMgr);

//int64_t makePvpId(int roleid, int objtype)
//{
//    int64_t newId = roleid;
//    
//    return newId;
//}

void getPvpObjTypeAndObjId(int64_t inId, int& outId, int& outType)
{
//    ((uint64_t)type << 32) + objIdx;
    
    outId = (inId & 0xFFFFFFFF);
    
    outType = ((inId>>32) & 0xFFFFFFFF);
}

int calcPvpPointsAward(int rank)
{
    int getPoints = 0;
    try
    {
        CMsgTyped msg;
        msg.SetInt(rank);
        int ret = GameScript::instance()->call("pvpMod_pvpRankSettlement", msg);
        if ( ret >= 0)
        {
            int type = msg.TestType();
            if (type == TYPE_FLOAT)
            {
                getPoints = msg.FloatVal();
            }
        }
    }
    catch(...)
    {
    }
    
    return getPoints;
}

extern string awardToMailContentExplanation(string awardStr);
void sendPvpAwardMail(int  roleid, const char* roleName, int rank)
{
    PvpAward* award = PvpCfg::getRankAward(rank);
    
    ItemArray items;
    RewardStruct reward;
    reward.reward_consval = award->protoss;
    reward.reward_gold = award->gold;
    
    vector<string> attachs;
    
    std::vector<std::string> rewards = StrSpilt(award->itemAward, ";");
    rewardsCmds2ItemArray(rewards, items, reward);
    
    vector<string> contents;
    reward2MailFormat(reward, items, &contents, &attachs);
    string attach = StrJoin(attachs.begin(), attachs.end(), ";");
    string attachDesc = StrJoin(contents.begin(), contents.end(), ",");
    
    MailFormat* formate = NULL;
    string sendername = "";
    string title = "无";
    string content = "无";
    
    formate = MailCfg::getCfg("pvp_rank_award");
    
    if (formate) {
        sendername = formate->sendername;
        title = formate->title;
        content = formate->content;
        
        //獎勵：{:str:}
        find_and_replace(content, 2, Utils::itoa(rank).c_str(), attachDesc.c_str());
    }
    
    bool ret = sendMail(roleid, sendername.c_str(), roleName, title.c_str(), content.c_str(), attach.c_str(), "");
    
    if( false ==  ret ){
        StoreOfflineItem(roleid, attach.c_str());
    }
}

PvpMgr::PvpMgr()
{
//    for (int i = 0; i < 10; i++) {
//        mPvpRoles.push_back(i);
//    }
}

bool PvpMgr::loadDataFromDB()
{
    mPvpRanking.clear();
    mRankingIndexs.clear();
    mAwardIndex.clear();
    
    redisReply* rreply = redisCmd("zrange paihang:jjc 0 -1 withscores");
    int ranksize = rreply->elements;
    
    std::map<int, int64_t> rankData;
    rankData.clear();
    
    for (int i = 0; i < ranksize;) {
        rankData.insert(make_pair(atoi(rreply->element[i+1]->str), atoll(rreply->element[i]->str)));
        i += 2;
    }
    freeReplyObject(rreply);
    
    for (std::map<int, int64_t>::iterator iter = rankData.begin(); iter != rankData.end(); iter++) {
        
        int64_t roleid = iter->second;
        if (mRankingIndexs.find(roleid) == mRankingIndexs.end()) {
            mPvpRanking.push_back(roleid);
            mRankingIndexs.insert(make_pair(roleid, iter->first));
            setAward(mPvpRanking.size());
        }
    }
    
    RedisResult result(redisCmd("hget serverdata pvp_distrib_time pvp_settle_time"));
    mDistribAwardTime = result.readHash(0, 0);
    if (mDistribAwardTime == 0) {
        setNextDistribTime();
    }
    
    mRankSettleTime = result.readHash(1, 0);
    if (mRankSettleTime == 0) {
        setNextRankSettleTime();
    }
    
    if (mPvpRanking.size()) {
        SaveDataInDB();
        return true;
    }
    
    
    return true;
}

bool PvpMgr::SaveDataInDB()
{
    string wcmd = "zadd paihang:jjc";
    
    
    for (int i = 0; i < mPvpRanking.size(); i++) {
        string tmp = Utils::makeStr(" %d %lld", i+1, mPvpRanking[i]);
        wcmd.append(tmp);
    }
    
    //以事务形式执行
    doRedisCmd("multi");
    doRedisCmd("del paihang:jjc");
    doRedisCmd(wcmd.c_str());
    doRedisCmd("exec");
    
    return true;
}

void PvpMgr::setNextDistribTime()
{
    int tmp = Utils::mktimeFromToday(21);
    if ( time(NULL) < tmp ) {
        mDistribAwardTime = tmp;
    }
    else
    {
        mDistribAwardTime = tmp + 3600 * 24;
    }
    
    doRedisCmd("hset serverdata pvp_distrib_time %d", mDistribAwardTime);
}

bool PvpMgr::addRoleToRank(int64_t roleid)
{
    map<int64_t, int>::iterator iter;
    iter = mRankingIndexs.find(roleid);
    if (iter != mRankingIndexs.end()) {
        return false;
    }
    mPvpRanking.push_back(roleid);
    mRankingIndexs.insert(make_pair(roleid, mPvpRanking.size()));
    
    mAwardIndex.insert(make_pair(mPvpRanking.size(), true));
    
    int newRank = mPvpRanking.size();
    doRedisCmd("zadd paihang:jjc %d %lld", newRank, roleid);
    string tmp = "";
    
    tmp = Utils::makeStr("hset role:%d ranking %d", roleid, newRank);
    doRedisCmd(tmp.c_str());
    
    tmp = Utils::makeStr("hset role:%d %s 0", roleid, GetRolePropName(eRolePvpAward));
    doRedisCmd(tmp.c_str());
    
    create_global(onAddRoleToPvp, msg);
    msg->roleid = roleid;
    sendGlobalMsg(Game::MQ, msg);
    return true;
}

bool PvpMgr::changRolesRank(int64_t roleid, int64_t beChallengeRoleid)
{
    int preRanking = getRankingByRoleId(roleid);
    int newRanking = getRankingByRoleId(beChallengeRoleid);
    
    if ( preRanking > mPvpRanking.size() || newRanking > mPvpRanking.size() ) {
        return false;
    }

    mPvpRanking[newRanking - 1] = roleid;
    mPvpRanking[preRanking - 1] = beChallengeRoleid;
    
    mRankingIndexs[roleid] = newRanking;
    mRankingIndexs[beChallengeRoleid] = preRanking;
    
    setAward(newRanking);
    setAward(preRanking);
    
    string tmp = Utils::makeStr("zadd paihang:jjc %d %lld %d %lld", newRanking, roleid, preRanking, beChallengeRoleid);
    doRedisCmd(tmp.c_str());
    
    return true;
}

int PvpMgr::getRankingSize()
{
    return mPvpRanking.size();
}
int PvpMgr::getRankingByRoleId(int64_t roleid)
{
    map<int64_t, int>::iterator iter;
    iter = mRankingIndexs.find(roleid);
    if (iter != mRankingIndexs.end()) {
        
        if (!IsRealRanking(iter->second, roleid)) {
            iter->second = onGetPvpRealRank(roleid);
        }
        
        return iter->second;
    }
    return -1;
}

int64_t PvpMgr::getRoleIdByRank(int ranking)
{
    if (ranking < 1 || ranking > mPvpRanking.size()) {
        return -1;
    }
    return mPvpRanking[ranking - 1];
}

bool PvpMgr::IsRealRanking(int checkRank, int64_t checkRoleid)
{
//    int realRank = getRankingByRoleId(checkRoleid);
    if (checkRank > mPvpRanking.size() || checkRank < 1) {
        return false;
    }
    
    uint64_t realPlayer = mPvpRanking[checkRank - 1];
    
    return realPlayer == checkRoleid ? true : false;
}

bool PvpMgr::pushRoleInPvp(int64_t roleid)
{
    if (IsRoleInPvp(roleid)) {
        return true;
    }
    
    mPvpRoles.push_back(roleid);
    return true;
}

int PvpMgr::eraseRoleInPvp(int64_t roleid, bool myself)
{
    list<int64_t>::iterator it;
    for (it = mPvpRoles.begin(); it != mPvpRoles.end();) {

        int role = *it;
        if ( role && role == roleid) {
            it = mPvpRoles.erase(it);
            
            if (myself) {
                RedisResult result(redisCmd("hmget role:%d %s %s %s", roleid, GetRolePropName(eRolePropVipLvl), GetRolePropName(eRolePvpTimes), GetRolePropName(eRolePvpBuyTimes)));
                
                int vipLvl = result.readHash(0, 0);
                
                if ( !vipLvl ) {
                    
                    int pvptimes = result.readHash(1, 0);
                    int pvpbuytimes = result.readHash(2, 0);
                    
                    int freetime = PvpCfg::getfreeTimes();
                    int cooldown = 0;
                    int nextopen = Global::tick;
                    cooldown = PvpCfg::getCoolDownTime(freetime + pvpbuytimes - pvptimes - 1);
                    nextopen += cooldown;
                    doRedisCmd("hset role:%d %s %d", roleid, GetRolePropName(eRolePvpCoolDown), nextopen);
                    return cooldown;
                }
            }
            return 0;
        }
        else it++;
    }
    return 0;
}

bool PvpMgr::IsRoleInPvp(int64_t roleid)
{
    list<int64_t>::iterator it;
    for (it = mPvpRoles.begin(); it != mPvpRoles.end(); it++) {
        if (roleid == *it) {
            return true;
        }
    }
    return false;
}

bool PvpMgr::IsInCoolingDown(int64_t roleid)
{
    RedisResult result(redisCmd("hget role:%d %s", roleid, GetRolePropName(eRolePvpCoolDown)));
    int cooldown = result.readInt();
    return cooldown > time(NULL);
}

void PvpMgr::updatePvpLog(int64_t roleid, int64_t challengeid, int changeType, bool iswin, bool isBechallenge)
{
    int myRank = getRankingByRoleId(roleid);
    
    //超过5000名的按5001名，version2.2， 2014.7.28，by wzg
    check_max(myRank, PvpCfg::getRobotCount()+1);
    
    std::map<int64_t, struct PvpLog>::iterator iter;
    iter = mPvplog.find(roleid);
    if (iter == mPvplog.end()) {
        struct PvpLog pvpLog(challengeid, myRank, changeType, iswin, isBechallenge);
        mPvplog.insert(std::make_pair(roleid,pvpLog));
    }
    else
    {
        iter->second.updatePvpLog(challengeid, myRank, changeType, iswin, isBechallenge);
    }
}

bool PvpMgr::getPvpLog(int64_t roleid, vector<struct PvpLogData *> &outputData)
{
    std::map<int64_t, struct PvpLog>::iterator iter;
    iter = mPvplog.find(roleid);
    if (iter == mPvplog.end()) {
        return false;
    }
    
    iter->second.getPvpLog( outputData );
    return true;
}

bool PvpMgr::DeleteRoleInRank(int64_t roleid)
{
    vector<int64_t>::iterator iter;
    for (iter = mPvpRanking.begin(); iter != mPvpRanking.end();) {
        if (roleid == *iter) {
            
            //开始读取位置
            int index = iter - mPvpRanking.begin();
            
            //在内存中删除
            mPvpRanking.erase(iter);
            
            //在数据库排行列表中删除
            doRedisCmd("zrem paihang:jjc %lld", roleid);
            
            //在每日的奖励列表中删除
            doRedisCmd("hdel rolepvpAward %d", roleid);
            
            string wcmd = "zadd paihang:jjc";
            for (int i = index; i < mPvpRanking.size(); i++) {
                string tmp = Utils::makeStr(" %d %lld", i + 1, mPvpRanking[i]);
                wcmd.append(tmp);
                
            }
            doRedisCmd(wcmd.c_str());
            
            //更新index表
            std::map<int64_t, int>::iterator indexIter = mRankingIndexs.find(roleid);
            if (indexIter != mRankingIndexs.end()) {
                mRankingIndexs.erase(indexIter);
            }
            
            for (int i = index; i < mPvpRanking.size(); i++) {
                mRankingIndexs[mPvpRanking[i]] = i + 1;
            }
            
            return true;
        }
        else
        {
            iter++;
        }
    }
    return false;
}

//int PvpMgr::getAwardByRank(int roleid)
//{
//    map<int, PvpLog>::iterator iter;
//    iter = mPvplog.find(roleid);
//    if (iter == mPvplog.end()) {
//        return -1;
//    }
//    return iter->second.getTodayAward();
//}

void PvpMgr::SaveNewAwardRank()
{
    int counter = 0;
    RedisArgs args;
    
    string begin = "rolepvpAward";
    RedisHelper::beginHmset(args, begin.c_str());
    
    map<int, bool>::iterator iter;
    
    for (iter = mAwardIndex.begin(); iter != mAwardIndex.end(); iter++) {
        int rank = iter->first;
        int roleid = getRoleIdByRank(rank);
        if (roleid <= 0) {
            continue;
        }
        
        //超过5000名的按5001名给奖励，version2.2， 2014.7.5，by wzg
        check_max(rank, PvpCfg::getRobotCount()+1);
        
        RedisHelper::appendHmset(args, roleid, rank);
        counter++;
        
        if (counter%5000 == 0) {
            RedisHelper::commitHmset(get_DbContext(), args);
            
            args.clear();
            RedisHelper::beginHmset(args, begin.c_str());
        }
    }
    
    if (args.size() > 2) {
        RedisHelper::commitHmset(get_DbContext(), args);
    }
    
    mAwardIndex.clear();
}

void PvpMgr::setAward(int rank)
{
    mAwardIndex.insert(make_pair(rank, true));
}

void PvpMgr::sendPvpDayAward()
{
    MyPipeRedis pipeRedis(get_DbContext());
    
    vector<int> playerRoles;
    vector<int> playerRanks;
    vector<string> roleNames;
    
    int roleSize = mPvpRanking.size();
    for (int i = 0; i < roleSize; i++) {
        
        int roleid;
        int objType;
        getPvpObjTypeAndObjId(mPvpRanking[i], roleid, objType);
        
        if (objType == kObjectRobot) {
            continue;
        }
        
        if( REDIS_OK != pipeRedis.myRedisAppendCommand("hget role:%d rolename", roleid))
        {
            continue;
        }
        
        playerRoles.push_back(roleid);
        playerRanks.push_back(i + 1);
    }
    
    int effeRoleSize = playerRoles.size();
    for (int i = 0; i < effeRoleSize; i++) {
        redisReply* reply = pipeRedis.myRedisGetReply();
        
        if (reply == NULL) {
            playerRoles[i] = 0;
            roleNames.push_back("");
            continue;
        }
        
        RedisResult result(reply);
        
        string name = result.readStr();
        
        roleNames.push_back(name);
    }
    
    for (int i = 0; i < effeRoleSize; i++) {
        
        int rank = playerRanks[i];
        
        check_max(rank, PvpCfg::getRobotCount()+1);

        string rolename = roleNames[i];
        PvpAward* award = PvpCfg::getRankAward(rank);
        
        if (award == NULL) {
            continue;
        }
        sendPvpAwardMail(playerRoles[i], rolename.c_str(), rank);
    }
}

void PvpMgr::onHeartBeat(float dt)
{
    time_t nowtime = Global::tick;
    if (nowtime >= mDistribAwardTime) {
//        SaveNewAwardRank();
        
//        create_global(onSendPvpAward, msg);
//        msg->roles.clear();
//        msg->ranks.clear();
//        
//        int playerCount = mPvpRanking.size();
//        for (int i = 0; i < playerCount; i++) {
//            msg->ranks.push_back(i + 1);
//            msg->roles.push_back(mPvpRanking[i]);
//        }
//        
//        sendGlobalMsg(Game::MQ, msg);
        
        sendPvpDayAward();
        
        setNextDistribTime();
    }
    
//    if (PvpCfg::getAwardPointPeriod() > 0)
//    {
//        if (Global::tick >= mRankSettleTime)
//        {
//
//            
//            setNextRankSettleTime();
//            
//            log_info("异步竞技场每天积分发放");
//        }
//    }
}

int PvpMgr::safe_getNextDistribAwardTime()
{
    return mDistribAwardTime;
}
/*
 --------------------------------Pvp Log-------------------------------
 */
PvpLog::PvpLog(int64_t challengeid, int rank, int changeType, bool iswin, bool isBechallenge)
{
    mIndex = 0;
    mlogList[mIndex].mChallengeRoleId = challengeid;
    mlogList[mIndex].mIsWin = iswin;
    mlogList[mIndex].mRank = rank;
    mlogList[mIndex].mChangeType = changeType;
    mlogList[mIndex].mIsBeChallege = isBechallenge;
}
void PvpLog::updatePvpLog(int64_t challengeid, int rank, int changeType, bool iswin, bool isBechallenge)
{
    mIndex++;
    if (mIndex == PVP_LOG_COUNT) {
        mIndex = 0;
    }
    
    mlogList[mIndex].mChallengeRoleId = challengeid;
    mlogList[mIndex].mIsWin = iswin;
    mlogList[mIndex].mRank = rank;
    mlogList[mIndex].mChangeType = changeType;
    mlogList[mIndex].mIsBeChallege = isBechallenge;
}

void PvpLog::setTodayAward(int rank)
{
    mAwardRank = rank;
}

int PvpLog::getTodayAward()
{
    return mAwardRank;
}

void PvpLog::getPvpLog(vector<struct PvpLogData*> &outputData)
{
    outputData.clear();
    int tmp = mIndex;
    for (int i = 0; i < PVP_LOG_COUNT; i++) {
        tmp = mIndex - i;
        if (tmp < 0) {
            tmp += PVP_LOG_COUNT ;
        }
        outputData.push_back( mlogList + tmp );
    }
}

int PvpMgr::safe_GetRoleAwardRank(int roleid)
{
    RedisResult result(redisCmd("hget rolepvpAward %d", roleid));
    
    int rank = result.readInt(0);
    
    if (!rank) {
        result.setData(redisCmd("hget role:%d ranking", roleid));
        rank = result.readInt(0);
    }
    
    return rank;
}

void PvpMgr::safe_DeleteRoleInpvp(int roleid, ObjectType type)
{
    if (type != kObjectRole && type != kObjectRobot) {
        return;
    }
    
    create_global(deletRoleInPvp, pvp);
    pvp->roleid = MAKE_OBJ_ID(type, roleid);
    sendGlobalMsg(Global::MQ, pvp);
}

void PvpMgr::setRolePvpState(int64_t roleId, int64_t targetId)
{
    
    map<int64_t, PvpRoleState>::iterator iter = mPvpRoleStates.find(roleId);
    if (iter == mPvpRoleStates.end())
    {
        PvpRoleState state;
        state.mRoleid = roleId;
        state.isChallenge = true;
        mPvpRoleStates[roleId] = state;
        return;
    }
    
    iter->second.isChallenge = true;
    
    if (targetId <= 0) {
        return;
    }
    
    iter = mPvpRoleStates.find(targetId);
    if (iter == mPvpRoleStates.end()) {
        PvpRoleState targetState;
        
        targetState.mRoleid = targetId;
        targetState.isBeChallenge = true;
        mPvpRoleStates[targetId] = targetState;
        return;
    }
    iter->second.isBeChallenge = true;
}

void PvpMgr::resetRolePvpState(int64_t roleId, int64_t targetId)
{
    
    map<int64_t, PvpRoleState>::iterator iter = mPvpRoleStates.find(roleId);
    if (iter == mPvpRoleStates.end())
    {
        return;
    }
    
    iter->second.isChallenge = false;
    
    if (targetId <= 0) {
        return;
    }
    
    iter = mPvpRoleStates.find(targetId);
    if (iter != mPvpRoleStates.end()) {
        iter->second.isBeChallenge =false;
    }
}

bool PvpMgr::isRoleChallenging(int64_t roleId)
{
    map<int64_t, PvpRoleState>::iterator iter = mPvpRoleStates.find(roleId);
    if (iter == mPvpRoleStates.end())
    {
        return false;
    }
    
    return iter->second.isChallenge;
}
bool PvpMgr::isRoleChallenged(int64_t roleId)
{
    map<int64_t, PvpRoleState>::iterator iter = mPvpRoleStates.find(roleId);
    if (iter == mPvpRoleStates.end())
    {
        return false;
    }
    
    return iter->second.isBeChallenge;
}

int PvpMgr::onGetPvpRealRank(int64_t roleid)
{
    int pvpsize = mPvpRanking.size();
    
    for (int i = 0; i < pvpsize; i++) {
        if (roleid == mPvpRanking[i]) {
            return i + 1;
        }
    }
    return 0;
}

void PvpMgr::setNextRankSettleTime()
{
    int nowtime = Global::tick;
    
    int nextTime = 0;
    int pointTime = Utils::mktimeFromToday(21);
    
    if (nowtime >= pointTime) {
        nextTime = pointTime + 3600 * 24;
    }
    else
    {
        nextTime = pointTime;
    }
    
//    int addTime = (nowtime % PvpCfg::getAwardPointPeriod());
//    mRankSettleTime = nowtime - (nowtime % PvpCfg::getAwardPointPeriod()) + PvpCfg::getAwardPointPeriod();
    
    mRankSettleTime = nextTime;
}