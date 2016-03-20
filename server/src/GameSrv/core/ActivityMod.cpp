//
//  ActivityMod.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-7-24.
//
//

#include "ActivityMod.h"
#include "DataCfgActivity.h"
#include "DynamicConfig.h"
#include "main.h"
#include "EnumDef.h"
#include "Activity.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include "DBRolePropName.h"
#include "GameLog.h"
#include "json/json.h"
#include "mail_imp.h"
#include <sstream>
#include "time.h"
#include "BroadcastNotify.h"
#include "Paihang.h"
#include "Game.h"
#include "Role.h"

#include "RedisDhash.h"
#include "RedisKeyName.h"
#include "flopcard.h"
#include "Honor.h"
#include "CopyExtraAward.h"
#include "Activity.h"

using namespace std;
class Role;

#pragma mark -
#pragma mark 好友地下城活动
#pragma mark -
std::map<int, time_t> GameActMgr::mOpenTime;
void GameActMgr::init()
{
    mOpenTime.clear();
    
    SActMgr.SafeCreateTimer(ae_friend_dunge, att_open, 0, GameActMgr::setOpen, "game_act");
    
    SActMgr.SafeCreateTimer(ae_print_copy, att_open, 0, GameActMgr::setOpen, "game_act");
    
    SActMgr.SafeCreateTimer(ae_tower_defense, att_open, 0, GameActMgr::setOpen, "game_act");
}

void GameActMgr::setOpen(int actId, int number, string &params)
{
//    int openTime = Game::tick - Utils::safe_atoi(params.c_str());
    int opentime = Utils::safe_atoi(params.c_str());
    
    mOpenTime[actId] = opentime;
}

time_t GameActMgr::getActOpenTime(int actId)
{
    map<int, time_t>::iterator iter = mOpenTime.find(actId);
    if (iter == mOpenTime.end()) {
        return 0;
    }
    
    return iter->second;
}

/************************  好友地下城活动  ********************************/

bool FriendDungeAct::loadFriendDungeData(int roleid)
{
    char buf[256];
    snprintf(buf, 256, "smembers dungefriends:%d", roleid);
    
    redisReply* reply = redisCmd(buf);
    if (reply == NULL)
    {
        return false;
    }
    
    for (int i = 0; i < reply->elements; i++)
    {
        mDungeFriends.insert(Utils::safe_atoi(reply->element[i]->str));
    }
    freeReplyObject(reply);
    
    RedisResult result(redisCmd("hmget role:%d %s %s %s %s", roleid, GetRolePropName(eRolePropFriendDungeAttendTimes), GetRolePropName(eRolePropFriendDungeFriendRecord), GetRolePropName(eRolePropFriendDungeRecord),GetRolePropName(eRolePropFriendDungeLastFreshTime)));
    int times = result.readHash(0, 0);
    string friendInvited = result.readHash(1, "");
    string record = result.readHash(2, "");
    int freshTime = result.readHash(3, 0);
    
    mAttendTimes = times;
    mFreshTime = freshTime;
    friendDungeFriendRecordInit(friendInvited);
    friendDungeRecordInit(record);
    
    return true;
}

bool FriendDungeAct::IsFriendFighted(int friendid)
{
    if (mDungeFriends.find(friendid) == mDungeFriends.end()) {
        return false;
    }
    return true;
}

bool FriendDungeAct::addFightedFriend(int roleid, int friendid)
{
    if (mDungeFriends.find(friendid) != mDungeFriends.end()) {
        return false;
    }
    
    mDungeFriends.insert(friendid);
    
    char buf[256];
    snprintf(buf, 256, "sadd dungefriends:%d %d", roleid, friendid);
    
    doRedisCmd(buf);
    return true;
}
bool FriendDungeAct::checkFresh()
{
    if (mFreshTime < GameActMgr::getActOpenTime(mActid)) {
        actFresh();
        return true;
    }
    
    return false;
}

void FriendDungeAct::setFreshTime()
{
    mFreshTime = GameActMgr::getActOpenTime(mActid);
    doRedisCmd("hset role:%d %s %d", mMaster->getInstID(), GetRolePropName(eRolePropFriendDungeLastFreshTime), mFreshTime);
}

void FriendDungeAct::actFresh()
{
    mDungeFriends.clear();
    mAttendTimes = 0;
    
    mFriendDungeRecord.clear();
    mFriendDungeFriendRecord.clear();
    
    string action = Utils::makeStr("daily_fresh");
    mMaster->addLogActivityCopyEnterTimesChange(eActivityEnterTimesFriendDunge, action.c_str());
    
    RedisArgs hdelargs;
    RedisArgs delargs;
    
    string kenName = Utils::makeStr("role:%d", mMaster->getInstID());
    string buf = Utils::makeStr("dungefriends:%d", mMaster->getInstID());
    
    RedisHelper::beginDelKeys(delargs);
    RedisHelper::appendKeyname(delargs, buf.c_str());
    RedisHelper::commitDelKeys(get_DbContext(), delargs);
    
    RedisHelper::beginHdel(hdelargs, kenName.c_str());
    RedisHelper::appendHdel(hdelargs, GetRolePropName(eRolePropFriendDungeAttendTimes));
    RedisHelper::appendHdel(hdelargs, GetRolePropName(eRolePropFriendDungeRecord));
    RedisHelper::appendHdel(hdelargs, GetRolePropName(eRolePropFriendDungeFriendRecord));
    RedisHelper::commitHdel(get_DbContext(), hdelargs);
    
    setFreshTime();
}
const set<int>& FriendDungeAct::getFightedFriendList()
{
    return mDungeFriends;
}

void FriendDungeAct::clearFightedFriends(int roleid)
{
    mDungeFriends.clear();
}

int FriendDungeAct::getactid()
{
    return mActid;
}

int FriendDungeAct::getAttendTimes()
{
    return mAttendTimes;
}

void FriendDungeAct::addAttendTimes(int roleid, int add)
{
    mAttendTimes += add;
    
    doRedisCmd("hset role:%d %s %d", roleid, GetRolePropName(eRolePropFriendDungeAttendTimes), mAttendTimes);
}

void FriendDungeAct::saveFriendDungeRecord(int index, int newrecord)
{
    map<int, int>::iterator endIter = mFriendDungeRecord.end();
    
    map<int, int>::iterator iter = mFriendDungeRecord.find(index);
    
    if (iter == endIter) {
        mFriendDungeRecord.insert(make_pair(index, newrecord));
    }
    else{
        if (newrecord == 0) {
            mFriendDungeRecord.erase(iter);
        }
        else
        {
            iter->second = newrecord;
        }
    }
    
    ostringstream ostr;
    
    iter = mFriendDungeRecord.begin();
    
    for (; iter != endIter; iter++) {
        ostr<<iter->first;
        ostr<<":";
        ostr<<iter->second;
        ostr<<";";
    }
    string record = ostr.str();
    
    doRedisCmd("hmset role:%d %s %s", mMaster->getInstID(), GetRolePropName(eRolePropFriendDungeRecord), record.c_str());
}

int FriendDungeAct::getFriendDungeRecord(int index)
{
    
    map<int, int>::iterator endIter = mFriendDungeRecord.end();
    
    map<int, int>::iterator iter = mFriendDungeRecord.find(index);
    
    if (iter == endIter) {
        return 0;
    }
    else{
        return iter->second;
    }
}

void FriendDungeAct::getFriendDungeRecord(vector<obj_friendDunge_record>& output)
{
    map<int, int>::iterator endIter = mFriendDungeRecord.end();
    map<int, int>::iterator iter = mFriendDungeRecord.begin();
    
    for (; iter != endIter; iter++) {
        obj_friendDunge_record record;
        record.index = iter->first;
        record.record = iter->second;
        output.push_back(record);
    }
}

void FriendDungeAct::friendDungeRecordInit(string str)
{
    StringTokenizer token(str, ";");
    
    for (int i = 0; i < token.count(); i++) {
        StringTokenizer tmp(token[i], ":");
        
        if (tmp.count() < 2) {
            continue;
        }
        
        int index = Utils::safe_atoi(tmp[0].c_str());
        int record = Utils::safe_atoi(tmp[1].c_str());
        
        mFriendDungeRecord.insert(make_pair(index, record));
    }
}

int FriendDungeAct::getFriendDungeFriendRecord(int index)
{
    map<int, int>::iterator iter = mFriendDungeFriendRecord.find(index);
    if (iter == mFriendDungeFriendRecord.end()) {
        return 0;
    }
    
    return iter->second;
}

void FriendDungeAct::saveFriendDungeFriendRecord(int index, int friendId)
{
    map<int, int>::iterator iter = mFriendDungeFriendRecord.find(index);
    if (iter == mFriendDungeFriendRecord.end()) {
        
        if (friendId <= 0) {
            return;
        }
        
        mFriendDungeFriendRecord.insert(make_pair(index, friendId));
    }
    else{
        if (friendId == 0) {
            mFriendDungeFriendRecord.erase(iter);
        }
        else
        {
            iter->second = friendId;
        }
    }
    
    ostringstream ostr;
    
    iter = mFriendDungeFriendRecord.begin();
    
    for (; iter != mFriendDungeFriendRecord.end(); iter++) {
        ostr<<iter->first;
        ostr<<":";
        ostr<<iter->second;
        ostr<<";";
    }
    string record = ostr.str();
    
    doRedisCmd("hmset role:%d %s %s", mMaster->getInstID(), GetRolePropName(eRolePropFriendDungeFriendRecord), record.c_str());
    
}

void FriendDungeAct::friendDungeFriendRecordInit(string str)
{
    StringTokenizer token(str, ";");
    
    for (int i = 0; i < token.count(); i++) {
        StringTokenizer tmp(token[i], ":");
        
        if (tmp.count() < 2) {
            continue;
        }
        
        int index = Utils::safe_atoi(tmp[0].c_str());
        int friendid = Utils::safe_atoi(tmp[1].c_str());
        
        mFriendDungeFriendRecord.insert(make_pair(index, friendid));
    }
}
/************************  图纸副本活动  ********************************/

bool PrintCopyAct::loadPrintCopyData(int roleid)
{
    mFinishCopy.clear();

    RedisResult result(redisCmd("smembers printcopy:%d", roleid));

    for (int i = 0; i < result.getHashElements(); i++)
    {
        int copyid = result.readHash(i, 0);
        mFinishCopy.insert(copyid);
    }

    result.setData(redisCmd("hgetall printcopyreset:%d", roleid));
    for (int i = 0; i < result.getHashElements();) {
        int copyid = result.readHash(i, 0);
        int resettimes = result.readHash(i + 1, 0);
        mCopyResetTimesList.insert(make_pair(copyid, resettimes));
        i += 2;
    }
    
    result.setData(redisCmd("hget role:%d %s", mMaster->getInstID(), GetRolePropName(eRolePropPrintCopyFreshTime)));
    
    mFreshTime = result.readInt();

    return true;
}

bool PrintCopyAct::IsCopyFinish(int copyid)
{
    if (mFinishCopy.find(copyid) == mFinishCopy.end()) {
        return false;
    }

    return true;
}

const set<int>& PrintCopyAct::getFinishCopy()
{
    return mFinishCopy;
}

bool PrintCopyAct::addFinishCopyid(int roleid, int copyid)
{
    if (mFinishCopy.find(copyid) != mFinishCopy.end()) {
        return false;
    }

    mFinishCopy.insert(copyid);

    char buf[256];
    snprintf(buf, 256, "sadd printcopy:%d %d", roleid, copyid);

    doRedisCmd(buf);
    return true;
}

void PrintCopyAct::PrintCopyReset(int roleid, int sessionid)
{
    mFinishCopy.clear();
    mCopyResetTimesList.clear();

    notify_sys_reset_printcopy notify;
    sendNetPacket(sessionid, &notify);
}

bool PrintCopyAct::DelCopyFromFinish(int roleid, int copyid)
{
    std::set<int>::iterator iter = mFinishCopy.find(copyid);

    if (iter == mFinishCopy.end()) {
        return false;
    }
    else{
        mFinishCopy.erase(iter);
        doRedisCmd("srem printcopy:%d %d", roleid, copyid);
        return true;
    }

    return false;
}

void PrintCopyAct::addCopyResetTimes(int roleid, int copyid, int times)
{
    std::map<int, int>::iterator iter = mCopyResetTimesList.find(copyid);
    int resettimes = 0;
    if (iter != mCopyResetTimesList.end()) {
        iter->second += times;
        resettimes = iter->second;
    }
    else{
        resettimes = times;
        mCopyResetTimesList.insert(make_pair(copyid, times));
    }

    doRedisCmd("hset printcopyreset:%d %d %d", roleid, copyid, resettimes);
}

int PrintCopyAct::getCopyResetTimes(int copyid)
{
    std::map<int, int>::iterator iter = mCopyResetTimesList.find(copyid);
    if (iter != mCopyResetTimesList.end()) {
        return iter->second;
    }

    return 0;
}

const std::map<int, int>& PrintCopyAct::getResetTimesList()
{
    return mCopyResetTimesList;
}

bool PrintCopyAct::checkFresh()
{
    if (mFreshTime < GameActMgr::getActOpenTime(mActid)) {
        actFresh();
        return true;
    }
    
    return false;
}

void PrintCopyAct::setFreshTime()
{
    mFreshTime = GameActMgr::getActOpenTime(mActid);
    doRedisCmd("hset role:%d %s %d", mMaster->getInstID(), GetRolePropName(eRolePropPrintCopyFreshTime), mFreshTime);
}

void PrintCopyAct::actFresh()
{
    mFinishCopy.clear();

    mCopyResetTimesList.clear();
    
    RedisArgs delargs;
    
    RedisHelper::beginDelKeys(delargs);
    
    string keyName = Utils::makeStr("printcopy:%d", mMaster->getInstID());
    RedisHelper::appendKeyname(delargs, keyName.c_str());
    
    keyName = Utils::makeStr("printcopyreset:%d", mMaster->getInstID());
    RedisHelper::appendKeyname(delargs, keyName.c_str());
    
    RedisHelper::commitDelKeys(get_DbContext(), delargs);
    
    setFreshTime();
    
}

/*****************************  tower defense  ****************************/

bool TowerDefenseAct::checkFresh()
{
    if (mFreshTime < GameActMgr::getActOpenTime(mActid)) {
        actFresh();
        return true;
    }
        
    return false;
}

void TowerDefenseAct::actFresh()
{
    mEnterTimes = 0;
    
    string action = Utils::makeStr("daily_fresh");
    mMaster->addLogActivityCopyEnterTimesChange(eActivityEnterTimesTowerDefense, action.c_str());
    
    RedisArgs args;
    string key = Utils::makeStr("role:%d", mMaster->getInstID());
    RedisHelper::beginHdel(args, key.c_str());
    RedisHelper::appendHmset(args, GetRolePropName(eRoleTowerDefenseTimes), mEnterTimes);
    
    RedisHelper::commitHdel(get_DbContext(), args);
    
    setFreshTime();
}

void TowerDefenseAct::setFreshTime()
{
    mFreshTime = GameActMgr::getActOpenTime(mActid);
    
    doRedisCmd("hset role:%d %s %d", mMaster->getInstID(), GetRolePropName(eRoleTowerDefenseTimesFresh), mFreshTime);
}

void TowerDefenseAct::loadData()
{
    RedisResult result(redisCmd("hmget role:%d %s %s", mMaster->getInstID(), GetRolePropName(eRoleTowerDefenseTimesFresh), GetRolePropName(eRoleTowerDefenseTimes)));
    
    mFreshTime = result.readHash(0, 0);
    mEnterTimes = result.readHash(1, 0);
}

void TowerDefenseAct::addEnterTimes(int addTimes)
{
    mEnterTimes++;
    RedisArgs args;
    
    string key = Utils::makeStr("role:%d", mMaster->getInstID());
    RedisHelper::beginHmset(args, key.c_str());
    RedisHelper::appendHmset(args, GetRolePropName(eRoleTowerDefenseTimes), mEnterTimes);
    RedisHelper::commitHmset(get_DbContext(), args);
}
/********************************  petcamp  ********************************/

bool PetCampAct::loadPetCampData(int roleid)
{
    mFinishCamp.clear();
    
    RedisResult result(redisCmd("smembers petcamp:%d", roleid));
    
    for (int i = 0; i < result.getHashElements(); i++)
    {
        mFinishCamp.insert(result.readHash(i, 0));
    }
    
    result.setData(redisCmd("hgetall petcampreset:%d", roleid));
    for (int i = 0; i < result.getHashElements();) {
        int copyid = result.readHash(i, 0);
        int resettimes = result.readHash(i + 1, 0);
        mCampResetTimesList.insert(make_pair(copyid, resettimes));
        i += 2;
    }
    
    return true;
}

bool PetCampAct::IsCampFinish(int copyid)
{
    if (mFinishCamp.find(copyid) == mFinishCamp.end()) {
        return false;
    }
    
    return true;
}

const set<int>& PetCampAct::getFinishCamp()
{
    return mFinishCamp;
}

bool PetCampAct::addPetCampid(int roleid, int copyid)
{
    if (mFinishCamp.find(copyid) != mFinishCamp.end()) {
        return false;
    }
    
    mFinishCamp.insert(copyid);
    
    char buf[256];
    snprintf(buf, 256, "sadd petcamp:%d %d", roleid, copyid);
    
    doRedisCmd(buf);
    return true;
}

void PetCampAct::PetCampReset(int roleid, int sessionid)
{
    mFinishCamp.clear();
    mCampResetTimesList.clear();
    
    notify_sys_reset_petcamp notify;
    sendNetPacket(sessionid, &notify);
}

bool PetCampAct::DelCampFromFinish(int roleid, int copyid)
{
    std::set<int>::iterator iter = mFinishCamp.find(copyid);
    
    if (iter == mFinishCamp.end()) {
        return false;
    }
    else{
        mFinishCamp.erase(iter);
        doRedisCmd("srem petcamp:%d %d", roleid, copyid);
        return true;
    }
    
    return false;
}

void PetCampAct::addCampResetTimes(int roleid, int copyid, int times)
{
    std::map<int, int>::iterator iter = mCampResetTimesList.find(copyid);
    int resettimes = 0;
    if (iter != mCampResetTimesList.end()) {
        iter->second += times;
        resettimes = iter->second;
    }
    else{
        resettimes = times;
        mCampResetTimesList.insert(make_pair(copyid, times));
    }
    
    doRedisCmd("hset petcampreset:%d %d %d", roleid, copyid, resettimes);
}

int PetCampAct::getCampResetTimes(int copyid)
{
    std::map<int, int>::iterator iter = mCampResetTimesList.find(copyid);
    if (iter != mCampResetTimesList.end()) {
        return iter->second;
    }
    
    return 0;
}

const std::map<int, int>& PetCampAct::getResetTimesList()
{
    return mCampResetTimesList;
}


/**************************** pet dunge ***********************************/


bool PetDungeAct::loadPetDungeData(int roleid)
{
    char buf[256];
    snprintf(buf, 256, "smembers petdungefriends:%d", roleid);
    
    redisReply* reply = redisCmd(buf);
    if (reply == NULL)
    {
        return false;
    }
    
    for (int i = 0; i < reply->elements; i++)
    {
        mDungeFriends.insert(Utils::safe_atoi(reply->element[i]->str));
    }
    freeReplyObject(reply);
    return true;
}

bool PetDungeAct::IsFriendFighted(int friendid)
{
    if (mDungeFriends.find(friendid) == mDungeFriends.end()) {
        return false;
    }
    return true;
}

bool PetDungeAct::addFightedFriend(int roleid, int friendid)
{
    if (mDungeFriends.find(friendid) != mDungeFriends.end()) {
        return false;
    }
    
    mDungeFriends.insert(friendid);
    
    char buf[256];
    snprintf(buf, 256, "sadd petdungefriends:%d %d", roleid, friendid);
    
    doRedisCmd(buf);
    return true;
}
void PetDungeAct::petDungeReset()
{
    mDungeFriends.clear();
}

const set<int>& PetDungeAct::getFightedFriendList()
{
    return mDungeFriends;
}

void PetDungeAct::clearFightedFriends(int roleid)
{
    mDungeFriends.clear();
}

int PetDungeAct::getactid()
{
    return mActid;
}

void saveFriendAward(SceneCfgDef* scene, Role* role)
{
    if (!role->getFriendMgr()->isFriend(role->getFightedFriendId())) {
        return;
    }
    
    RedisResult result(redisCmd("hmget role:%d %s %s %s %s", role->getFightedFriendId(), GetRolePropName(eRolePropRoleName), GetRolePropName(eRoleFriendAwardCount), GetRolePropName(eRoleFriendAwards), GetRolePropName(eRolePropVipLvl)));
    
    string rolename = result.readHash(0, "");
    int todayCount = result.readHash(1, 0);
    string todayaAwards = result.readHash(2, "");
    int friendVipLvl = result.readHash(3, 0);
    
    VipPrivilegeCfgDef* vipData = VipPrivilegeCfg::getCfg(friendVipLvl);
    if (vipData == NULL) {
        return;
    }
    
    if (rolename.empty() || todayCount >= vipData->friendCount) {
        return;
    }
    
    ItemArray items;
    RewardStruct reward;
    todayaAwards.append(";");
    todayaAwards.append(scene->friendaward);
    vector<string> itemstrs = StrSpilt(todayaAwards, ";");
    rewardsCmds2ItemArray(itemstrs,items,reward);
    
    reward.reward_prestige += scene->incr_prestige;
    
    string saveAwardStr = Utils::makeStr("exp %d*1;gold %d*1;prestige %d*1", reward.reward_exp, reward.reward_gold, reward.reward_prestige);
    
    doRedisCmd("hmset role:%d %s %d %s %s", role->getFightedFriendId(), GetRolePropName(eRoleFriendAwardCount), todayCount + 1,GetRolePropName(eRoleFriendAwards), saveAwardStr.c_str());
    
}

void sendFriendAwardMail(SceneCfgDef* scene, Role* role)
{
    if (!role->getFriendMgr()->isFriend(role->getFightedFriendId())) {
        return;
    }
    
    ItemArray items;
    RewardStruct reward;
    vector<string> itemstrs = StrSpilt(scene->friendaward, ";");
    rewardsCmds2ItemArray(itemstrs,items,reward);
    
    MailFormat* formate = NULL;
    string sendername = "";
    string title = "无";
    char content[1024] = "无";
    
    switch (scene->sceneType) {
        case stFriendDunge:
        {
            formate = MailCfg::getCfg("friend_dunge");
            break;
        }
        case stTeamCopy:
        {
            formate = MailCfg::getCfg("team_copy");
            break;
        }
        default:
            break;
    }
    
    if (formate) {
        sendername = formate->sendername;
        title = formate->title;
        
        char itoabuf[21];
        string exp = Utils::itoa(reward.reward_exp, 10, itoabuf);
        string gold = Utils::itoa(reward.reward_gold, 10, itoabuf);
		string prestige = Utils::itoa(scene->incr_prestige, 10, itoabuf);
        snprintf(content, sizeof(content), formate->content.c_str(), role->getRolename().c_str(), scene->name.c_str(), exp.c_str(), prestige.c_str(), gold.c_str());
    }
    
    string friendname = "";
    Role* fightedFrined = SRoleMgr.GetRole(role->getFightedFriendId());
    if (fightedFrined) {
        friendname = fightedFrined->getRolename();
    }
    else{
        RedisResult result(redisCmd("hget role:%d rolename", role->getFightedFriendId()));
        friendname = result.readStr();
    }
    
    if (!friendname.empty()) {

        bool ret = sendMail(0, sendername.c_str(), friendname.c_str(), title.c_str(), content, scene->friendaward.c_str(), Utils::itoa(role->getFightedFriendId()).c_str());
        
        if( false ==  ret ){
            StoreOfflineItem(role->getFightedFriendId(), scene->friendaward.c_str());
        }
    }
	
	// 添加声望
	HonorMgr::rewardCopyPrestige(role->getFightedFriendId(), scene->sceneId);
}
//ActMgr<FriendDungeAct> g_friendDungeActMgr;
//ActMgr<PrintCopyAct> g_PrintCopyActMgr;
//ActMgr<PetCampAct> g_PetCampActMgr;
//ActMgr<PetDungeAct> g_PetDunngeActMgr;

/*************************************  神秘副本  **************************************/

int MysticalCopyItem::calcNextOpenTime(int now)
{
    int weekDay = Utils::getWeekDay(now);
    
    if (weekDay == 0) {
        weekDay = 7;
    }
    
    int beginTime = 0;
    int nextDay = weekDay;
    for(int i = 1; i < 7; i++)
    {
        nextDay++;
        
        if(nextDay >= 8)
        {
            nextDay = 1;
        }
        
        for(int jj = 0; jj < openDays.size(); jj++)
        {
            if(nextDay == openDays[jj])
            {
                
                beginTime = Utils::mktimeFromWday(nextDay%7, 0, 0);
                
                if (weekDay > nextDay && weekDay != 7) {
                    beginTime += 3600*24*7;
                }
                
                return beginTime;
            }
        }
        
    }
    return 0;
}

bool MysticalCopyItem::checkOpen(int now)
{
    int vsize = openDays.size();
    
    if(vsize <= 0)
    {
        return false;
    }
    
    int weekDay = Utils::getWeekDay(now);
    if (weekDay == 0)
    {
        weekDay = 7;
    }
    
    int beginDayIndex = 0;
    for(int i = 0; i < openDays.size(); i++)
    {
        if(weekDay == openDays[i])
        {
            setOpen(true);
            beginDayIndex = i;
            break;
        }
    }
    int nextday = weekDay;
    if(isOpen())
    {
        for(int i = 1; i < 7; i++)
        {
            nextday++;
            
            if(nextday >= 8)
            {
                nextday = 1;
            }
            
            beginDayIndex = (++beginDayIndex)%vsize;
            
            if(nextday != (openDays[beginDayIndex]))
            {
                endTime = Utils::mktimeFromWday(nextday%7, 0, 0);
                if(nextday < weekDay || nextday == 7)
                {
                    endTime += (3600*24*7);
                }
                break;
            }
        }
        
        return true;
    }
    else
    {
        nextBeginTime = calcNextOpenTime(now);
    }
    
    return false;
}

MysticalCopyMgr g_MysticalCopyMgr;
void MysticalCopyMgr::init()
{
    m_actid = ae_mystical_copy;
    m_currentBeginIndex = 0;
    m_nextReplace = 0;
    mProcessWDay = -1;
    
    //静态版,copy列表不刷新
    if(GameFunctionCfg::getGameFunctionCfg().mysticalcopyType == eNoChangeMysticalCopy)
    {
        loadDataFromConfig();
        
        mBeginCycle = true;
        
        //清除旧数据
        doRedisCmd("hdel serverdata mysticalcopyindex mysticalcopynextreplace mysticalcopydata mysticalcopynextindex");
    }
    else //动态版，copy列表周期性刷新
    {
        loadDataFromDB();
        m_running = true;
    }
}

void MysticalCopyMgr::loadDataFromDB()
{
    int nowtime = Game::tick;
    
    RedisResult result(redisCmd("hmget serverdata mysticalcopyindex mysticalcopynextreplace mysticalcopydata mysticalcopynextindex"));
    m_currentBeginIndex = result.readHash(0, 1);
    m_nextReplace = result.readHash(1, 0);
    string copydataStr = result.readHash(2, "");
    m_nextBeginIndex = result.readHash(3, 0);
    
    //旧服务器更新后第一次加载旧数据
    if (!m_nextBeginIndex && m_nextReplace) {
        MysticalCopyGroup newgroup;
        
        for (int i = 0; i < MysticalCopyListCfg::copyCountPerGroup; i++) {
            
            int index = m_currentBeginIndex + i;
            if (m_nextBeginIndex > MysticalCopyListCfg::listcount) {
                index = index - MysticalCopyListCfg::listcount;
            }
            
            MysticalCopyItem copyitem;
            copyitem.index = index;
            copyitem.copyids = MysticalCopyListCfg::getCopyID(index);
            newgroup.copyIndexs.push_back(copyitem);
            
            newgroup.eraseTime = m_nextReplace;
            
            m_nextBeginIndex = index + 1;
        }
        m_currentGroup.push_back(newgroup);
        
        saveMysticalCopyData();
    }
    else    //新服务器或者是旧服务器更新后非首次启动
    {
        if (m_nextReplace == 0) {
            m_nextReplace = nowtime;
        }
        
        StringTokenizer token(copydataStr, ";");
        
        for (int i = 0; i < token.count(); i++) {
            
            MysticalCopyGroup newGroup;
            StringTokenizer groupInfo(token[i], ",");
            if (groupInfo.count() <= 0) {
                continue;
            }
            
            if (groupInfo.count() != (MysticalCopyListCfg::copyCountPerGroup + 1)) {
                resetMgr();
                return;
            }
            
            newGroup.eraseTime = Utils::safe_atoi(groupInfo[0].c_str());
            
            if (newGroup.eraseTime > m_nextReplace) {
                
                int spread = newGroup.eraseTime - m_nextReplace;
                int periodMutipl = spread / MysticalCopyListCfg::replacePeriod + 1;
                int maxMutiple = MysticalCopyListCfg::activeGroup - 1;
                
                check_range(periodMutipl, 1, maxMutiple);
                
                newGroup.eraseTime = m_nextReplace + periodMutipl * MysticalCopyListCfg::replacePeriod;
            }
            
            for (int index = 1; index < groupInfo.count(); index++) {
                
                int copyindex = Utils::safe_atoi(groupInfo[index].c_str());
                MysticalCopyItem copyitem;
                copyitem.index = copyindex;
                copyitem.copyids = MysticalCopyListCfg::getCopyID(copyindex);
                newGroup.copyIndexs.push_back(copyitem);
                
                if (newGroup.copyIndexs.size() >= MysticalCopyListCfg::copyCountPerGroup) {
                    break;
                }
            }
            m_currentGroup.push_back(newGroup);
            
            if (m_currentGroup.size() >= MysticalCopyListCfg::activeGroup) {
                break;
            }
        }
    }
}

void MysticalCopyMgr::loadDataFromConfig()
{
    MysticalCopyGroup newGroup;
    std::map<int, MysticalCopyListCfgDef*>::iterator iter = MysticalCopyListCfg::m_copylist.begin();
    for (; iter != MysticalCopyListCfg::m_copylist.end(); iter++) {
        vector<int> copylist = MysticalCopyListCfg::getCopyID(iter->first);
        if (copylist.size()) {
            MysticalCopyItem copyitem;
            copyitem.index = iter->first;
            copyitem.copyids = copylist;
            copyitem.openDays = iter->second->getOpenDays();
            copyitem.copyType = iter->second->copyType;
            copyitem.enterLvl = iter->second->openLvl;
            
            if (copyitem.copyType == eMysticalCopy_Cycle) {
                mCycleCopyList.push_back(copyitem);
            }
            else{
                copyitem.mIsOpen = true;
                newGroup.copyIndexs.push_back(copyitem);
            }
        }
    }
    m_currentGroup.push_back(newGroup);
}

void MysticalCopyMgr::getCopyListIndex(vector<obj_mysticalcopy>& outdata)
{
    MysticalCopyGroupData::iterator iter = m_currentGroup.begin();
    for (; iter != m_currentGroup.end();) {
        if ( isRunning() && iter->eraseTime < Game::tick) {
            iter = m_currentGroup.erase(iter);
        }
        else
        {
            iter->getIndexs(outdata);
            iter++;
        }
    }
}

int MysticalCopyMgr::checkCopyAndIndex(int index, int copy)
{
    for (int i = 0; i < m_currentGroup.size(); i++) {
        if (m_currentGroup[i].checkIndexAndCopyid(index, copy)) {
            return CE_OK;
        }
    }
    
    for (int i = 0; i < mCycleCopyList.size(); i++)
    {
        if (mCycleCopyList[i].index == index) {
            
            if(mCycleCopyList[i].isOpen() == false)
            {
                return CE_ACTIVE_IS_NOT_OPEN;
            }
            
            if (mCycleCopyList[i].checkCopyid(copy))
            {
                return CE_OK;
            }
        }
    }
    
    return CE_MYSTICALCOY_LIST_HAD_CHANGE;
}

int MysticalCopyMgr::getSurplusSecForFreshList()
{
    int secs = m_nextReplace - Game::tick;
    check_min(secs, 0);
    return secs;
}

void MysticalCopyMgr::processDay()
{
    if(mBeginCycle == false)
    {
        return;
    }
    
    int weekDay = Utils::getWeekDay(Game::tick);
    if ( mProcessWDay == weekDay) {
        return;
    }
    
    mProcessWDay = weekDay;
    
    int copySize = mCycleCopyList.size();
    
    for(int i = 0; i < copySize; i++)
    {
        MysticalCopyItem& copyItem = mCycleCopyList[i];
        if (copyItem.openDays.size() <= 0) {
            return;
        }
        
        if (copyItem.isOpen()) {
            
            if (Game::tick >= copyItem.endTime)
            {
                copyItem.setOpen(false);
                copyItem.endTime = 0;
                copyItem.nextBeginTime = copyItem.calcNextOpenTime(Game::tick);
            }
        }
        else{
            copyItem.checkOpen(Game::tick);
        }
    }
}

void MysticalCopyMgr::getCycleCopyListIndex(vector<obj_mysticalcopy> &outdata)
{
    int vsize = mCycleCopyList.size();
    for (int i = 0; i < vsize; i++) {
        obj_mysticalcopy obj;
        obj.index = mCycleCopyList[i].index;
        obj.copyType = mCycleCopyList[i].copyType;
        obj.isOpen = mCycleCopyList[i].isOpen();
        obj.enterLvl = mCycleCopyList[i].enterLvl;
        
        if (mCycleCopyList[i].isOpen()) {
            obj.existsec = mCycleCopyList[i].endTime - Game::tick;
            obj.nextOpenSec = 0;
        }
        else{
            obj.existsec = 0;
            obj.nextOpenSec = mCycleCopyList[i].nextBeginTime - Game::tick;
        }
        
        outdata.push_back(obj);
    }
}

void MysticalCopyMgr::update()
{
    //新增加的模式，version2.7
    processDay();
    
    if (!m_running) {
        return;
    }
    
    int nowtime = Game::tick;
    
    if(nowtime > m_nextReplace)
    {
        MysticalCopyGroupData::iterator iter = m_currentGroup.begin();
        
        for (;iter != m_currentGroup.end();) {
            if (nowtime >= iter->eraseTime) {
                iter = m_currentGroup.erase(iter);
            }
            else{
                iter++;
            }
        }
        
        if(m_currentGroup.size() < MysticalCopyListCfg::activeGroup) {
            
            MysticalCopyGroup newGroup;
            newGroup = createNewGroup();
            newGroup.eraseTime = m_nextReplace + MysticalCopyListCfg::existTime;
            m_currentGroup.push_back(newGroup);
        }
        
        if (Process::env.getInt("testmode") == 1) {
            printCurrCopysIndexs();
        }
        
        m_nextReplace += MysticalCopyListCfg::replacePeriod;

        saveMysticalCopyData();
    }
}


int handlerDungeResultAward(int sceneid, int result, Role* role)
{
    ItemArray items;
    int fcindex = 0;
    bool giveAward = false;
    int friendId = role->getFightedFriendId();
    
    std::vector<int> dropindices;
    string dropStr = "";
    string awardBase = SflopcardMgr.RandomCard(sceneid,fcindex, dropindices, dropStr);
    string awardResult = awardBase;
	
	SCopyExtraAward.extraItemsAward(role->getInstID(), sceneid, awardBase, awardResult);
	
    RewardStruct reward;
    std::vector<std::string> rewards = StrSpilt(awardResult, ";");
    rewardsCmds2ItemArray(rewards,items,reward);
    
    SceneCfgDef* _sceneCfg = SceneCfg::getCfg(sceneid);
    if (_sceneCfg == NULL) {
        return CE_SCENE_TYPE_ERROR;
    }
    
    do
    {
        if (result == eDungGetout)
        {
            //check record
            giveAward = false;
            break;
        }
        
        if (result == eDungeNext) {
            
//            struct DungeAward award;
//            award.sceneid = sceneid;
//            award.flopIndex = fcindex;
//            award.drops = dropStr;
//            role->mDungeAward.push_back(award);
            giveAward = true;
            break;
        }
        
        if (result == eDungeFail)
        {
            giveAward = false;
            break;
        }
        
        if (result == eDungeFinish)
        {
//            struct DungeAward award;
//            award.sceneid = sceneid;
//            award.flopIndex = fcindex;
//            award.drops = dropStr;
//            role->mDungeAward.push_back(award);
            giveAward = true;
            break;
        }
        
    }while(false);
    
    GridArray effgrids;
    //检查是否可以发放奖励
    int canAddItem = role->preAddItems(items, effgrids);
    
    string comeFrom = Utils::makeStr("finishcopy:%d",_sceneCfg->sceneId);
    if (canAddItem == CE_OK && giveAward) {
        //BackBag::UpdateBackBag(role->getInstID(), effgrids);
        //LogMod::addLogGetItem(role->getInstID(), role->getRolename().c_str(), role->getSessionId(), items, buf);
//        role->updateBackBag(effgrids, items, true, buf);
        role->playerAddItemsAndStore(effgrids, items, comeFrom.c_str(), true);
    }
    else if(giveAward)
    {
        sendMailWithName(0, "", role->getRolename().c_str(), "bag_full", awardResult.c_str());
    }
    
    if (giveAward) {
        // 亲密度加成
        role->getFriendMgr()->intimacyReward(friendId, _sceneCfg, reward);
		
        reward.reward_exp += _sceneCfg->copy_exp;
        reward.reward_gold += _sceneCfg->copy_gold;
        reward.reward_batpoint += _sceneCfg->copy_battlepoint;
        reward.reward_consval += _sceneCfg->copy_protoss;
        reward.reward_petexp += _sceneCfg->copy_petExp;
		
		// 副本活动奖励
		SCopyExtraAward.extraBaseAward(role->getInstID(), _sceneCfg, reward);
        
        role->addExp(reward.reward_exp, comeFrom.c_str());
        role->addGold(reward.reward_gold, comeFrom.c_str());
        role->addBattlePoint(reward.reward_batpoint, comeFrom.c_str());
        role->addConstellVal(reward.reward_consval, comeFrom.c_str());
        role->addPetExp(role->getActivePetId(), reward.reward_petexp, comeFrom.c_str());
        role->addEnchantDust(reward.reward_enchantdust, comeFrom.c_str());
        role->addEnchantGold(reward.reward_enchantgold, comeFrom.c_str());
        
        struct DungeAward award;
        award.sceneid = sceneid;
        award.flopIndex = fcindex;
        award.drops = dropStr;
        
        award.exp = reward.reward_exp;
        award.gold = reward.reward_gold;
        award.battlepoint = reward.reward_batpoint;
        award.constell = reward.reward_consval;
        award.petexp = reward.reward_petexp;
		award.awardItems = awardResult;
        
        role->mDungeAward.push_back(award);
    }
    LogMod::addLogFinishCopy(role, sceneid, true, awardResult, _sceneCfg->sceneType);
    return 0;
}

int dungeWipeOut(int dungeIndex, int beginsceneid, int endsceneId, Role* role, std::vector<obj_dungeawawrd>& outdata)
{
    outdata.clear();
    
    ItemArray allitems;
    allitems.clear();
    
    RewardStruct reward;
    stringstream szLogDungeonSweepingReward;
    
    for (int i = beginsceneid; i <= endsceneId; i++) {
        int fcindex = 0;
        RewardStruct tmpReward;
        int friendId = role->getFightedFriendId();
        
        std::vector<int> dropindices;
        string dropStr = "";
		string awardBase = SflopcardMgr.RandomCard(i,fcindex, dropindices, dropStr);
		string awardResult = awardBase;
		
		// 副本活动奖励
		SCopyExtraAward.extraItemsAward(role->getInstID(), i, awardBase, awardResult);
		
        std::vector<std::string> rewards = StrSpilt(awardResult, ";");
        
        //日志记录奖励
        szLogDungeonSweepingReward << awardResult;
        
        rewardsCmds2ItemArray(rewards,allitems,tmpReward);
        
        SceneCfgDef* _sceneCfg = SceneCfg::getCfg(i);
        if (_sceneCfg == NULL) {
            return CE_SCENE_TYPE_ERROR;
        }
        
        role->getFriendMgr()->intimacyReward(friendId, _sceneCfg, tmpReward);
        
        tmpReward.reward_exp += _sceneCfg->copy_exp;
        tmpReward.reward_batpoint += _sceneCfg->copy_battlepoint;
        tmpReward.reward_consval += _sceneCfg->copy_protoss;
        tmpReward.reward_gold += _sceneCfg->copy_gold;
        tmpReward.reward_petexp += _sceneCfg->copy_petExp;
		
		// 副本活动奖励
		SCopyExtraAward.extraBaseAward(role->getInstID(), _sceneCfg, tmpReward);
        
        reward.reward_exp += tmpReward.reward_exp;
        reward.reward_gold += tmpReward.reward_gold;
        reward.reward_consval += tmpReward.reward_consval;
        reward.reward_batpoint += tmpReward.reward_batpoint;
        reward.reward_petexp += tmpReward.reward_petexp;
        reward.reward_enchantdust += tmpReward.reward_enchantdust;
        reward.reward_enchantgold += tmpReward.reward_enchantgold;
        
        obj_dungeawawrd award;
        award.secneid = i;
        award.flopindex = fcindex;
        award.drops = dropStr;
        
        award.battlepoint = tmpReward.reward_batpoint;
        award.constell = tmpReward.reward_consval;
        award.gold = tmpReward.reward_gold;
        award.exp = tmpReward.reward_exp;
        award.petexp = tmpReward.reward_petexp;
		award.itemsAward = awardResult;
		
        outdata.push_back(award);
    
    }
    
    GridArray effgrids;
    //检查是否可以发放奖励
//        int canAddItem = role->preAddItems(items, effgrids);
    int canAddItem = role->preAddItems(allitems, effgrids);
    
    string comeFrom = Utils::makeStr("dunge_wipeout:%d~%d", beginsceneid, endsceneId);
    if (canAddItem == CE_OK) {
        role->playerAddItemsAndStore(effgrids, allitems, comeFrom.c_str(), true);
//        role->updateBackBag(effgrids, allitems, true, buf);
        //BackBag::UpdateBackBag(role->getInstID(), effgrids);
        //LogMod::addLogGetItem(role->getInstID(), role->getRolename().c_str(), role->getSessionId(), allitems, buf);
    }
    else
    {
        return CE_BAG_FULL;
    }
    
    
    role->addExp(reward.reward_exp, comeFrom.c_str());
    role->addGold(reward.reward_gold, comeFrom.c_str());
    role->addBattlePoint(reward.reward_batpoint, comeFrom.c_str());
    role->addConstellVal(reward.reward_consval, comeFrom.c_str());
    role->addPetExp(role->getActivePetId(), reward.reward_petexp, comeFrom.c_str());
    
    LogMod::addLogDungeonSweeping(role, beginsceneid, endsceneId, szLogDungeonSweepingReward.str(), true);

    
    return CE_OK;
}

MysticalCopyGroup MysticalCopyMgr::createNewGroup()
{
    MysticalCopyGroup newGroup;
   
    for (int i = 0; i <= MysticalCopyListCfg::listcount; i++) {
        
        if (m_nextBeginIndex > MysticalCopyListCfg::listcount) {
            m_nextBeginIndex = m_nextBeginIndex - MysticalCopyListCfg::listcount;
        }
        
        MysticalCopyGroupData::iterator iter = m_currentGroup.begin();
        
        //把已存在的组删掉
        for (;iter != m_currentGroup.end();) {
            if (iter->isIndexExist(m_nextBeginIndex)) {
                iter = m_currentGroup.erase(iter);
            }
            else{
                iter++;
            }
        }
        
        vector<int> copylist = MysticalCopyListCfg::getCopyID(m_nextBeginIndex);
        if (copylist.size()) {
            MysticalCopyItem copyitem;
            copyitem.index = m_nextBeginIndex;
            copyitem.copyids = copylist;
            newGroup.copyIndexs.push_back(copyitem);
        }
        
        m_nextBeginIndex++;
        
        if (newGroup.copyIndexs.size() >= MysticalCopyListCfg::copyCountPerGroup) {
            break;
        }
        
    }
    
    return newGroup;
}

void MysticalCopyMgr::saveMysticalCopyData()
{
    RedisArgs args;
    string cmdHead = Utils::makeStr("serverdata");
    RedisHelper::beginHmset(args, cmdHead.c_str());
    
    //下次刷新时间
    RedisHelper::appendHmset(args, "mysticalcopynextreplace", (int)m_nextReplace);
    
    //保存现在开放的副本列表和留存时间
    stringstream data;

    for (int i = 0; i < m_currentGroup.size(); i++) {
        int eraseTime = m_currentGroup[i].eraseTime;
        data<<eraseTime<<",";
        
        for (int itemcount = 0; itemcount < m_currentGroup[i].copyIndexs.size(); itemcount++) {
            data<<m_currentGroup[i].copyIndexs[itemcount].index<<",";
        }
        data<<";";
    }
    string copysData = data.str();
    RedisHelper::appendHmset(args, "mysticalcopydata", copysData.c_str());
    
    //下次刷新的开始的copy列表索引
    RedisHelper::appendHmset(args, "mysticalcopynextindex", m_nextBeginIndex);
    
    RedisHelper::commitHmset(get_DbContext(), args);
}

void MysticalCopyGroup::getIndexs(vector<obj_mysticalcopy>& outdata)
{
    for (int i = 0; i < copyIndexs.size(); i++) {
        obj_mysticalcopy copy;
        copy.index = copyIndexs[i].index;
        copy.existsec = eraseTime - (int)Game::tick;
        copy.copyType = copyIndexs[i].copyType;
        copy.nextOpenSec = 0;
        copy.isOpen = true;
        
        outdata.push_back(copy);
    }
}

void MysticalCopyMgr::resetMgr()
{
    m_currentBeginIndex = 0;
    m_nextBeginIndex = 0;
    m_nextReplace = Game::tick;
}

bool MysticalCopyMgr::isRunning()
{
    return m_running;
}

void MysticalCopyMgr::printCurrCopysIndexs()
{
    vector<obj_mysticalcopy> tmp;
    for (int i = 0; i < m_currentGroup.size(); i++) {
        m_currentGroup[i].getIndexs(tmp);
    }
    
    stringstream str;
    str<<"可用的大本营索引：\n";
    for (int i = 0; i < tmp.size(); i++) {
        str<<"\t"<<tmp[i].index<<"\t"<<tmp[i].existsec<<"\n";
    }
    log_info(str.str().c_str());
}