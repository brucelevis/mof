//
//  TreasureFight.cpp
//  GameSrv
//
//  Created by pireszhi on 14-5-15.
//
//

#include "TreasureFight.h"
#include "Utils.h"
#include "world_cmdmsg.h"
#include "Game.h"
#include "WorldPlayer.h"
#include "lua_helper.h"
#include "DBRolePropName.h"
#include "mail_imp.h"
#include "BroadcastNotify.h"
#include "GameScript.h"
#include "GameLog.h"
#include "daily_schedule_system.h"
#include "Honor.h"
#include "Role.h"

GuildTreasureFightMgr g_GuildTreasureFightMgr;

bool  sortByPoints(GuildTreasureFightResult *first, GuildTreasureFightResult *second)
{
    if (second->points == first->points) {
        return first->ownerid < second->ownerid;
    }
    
    return second->points < first->points;
}

//根据活动开始的时间算出星期几，拼出dbkey的前缀
string makeGuildTreasureDBkeyPrefix(int weekday)
{
    string prefix = Utils::makeStr("guildTreasureFight_%d", weekday);
    
    return prefix;
}

void guildTreasureFighActOpen(int actId, int number, string& params)
{
    int openTime = time(NULL) - Utils::safe_atoi(params.c_str());
    
    g_GuildTreasureFightMgr.activityOpen(openTime);
}

void guildTreasureFighActClose(int actId, int number, string& params)
{
    g_GuildTreasureFightMgr.activityClose();
}

string guildFightGetCityName(int weekday)
{
    //获取城市的名称
    string cityname = "";
    try
    {
        CMsgTyped msg;
        msg.SetInt(weekday);

        if (GameScript::instance()->call("treasurefight_getManorCityName", msg) >= 0)
        {
            int type = msg.TestType();
            if (type == TYPE_STRING)
            {
                cityname = msg.StringVal();
            }
        }
    }
    catch(...)
    {
    }
    
    return cityname;
}

int TreasureCopyItem::enterTreasureCopy(Role* role, GuildTreasureFightResult* roleResult)
{
    if (!isOpen) {
        return CE_TREASURECOPY_COPY_NOT_OPEN;
    }
    
    if (playerResult.size() >= maxPlayerCount) {
        
        return CE_TREASURECOPY_PLAYER_FULL;
        
    }
    
    for (int i = 0; i < playerResult.size(); i++) {
        GuildTreasureFightResult* tmp = playerResult[i];
        if (tmp && tmp->ownerid == role->getInstID()) {
            return CE_TREASURECOPY_YOU_ARE_INCOPY_NOW;
        }
    }
    
    int errorcode = CE_OK;
    
    do {
        
        int guildId = SRoleGuild(role->getInstID()).getGuild();
        
        if (guildId == 0) {
            errorcode = CE_GUILD_NOT_EXIST;
            break;
        }
        
        map<int, TreasureGuildMember>::iterator iter = groups.find(guildId);
        
        if (iter == groups.end()) {
            
            TreasureGuildMember members;
            
            members.clear();
            
            members.insert(role->getInstID());
            
            groups.insert(make_pair(guildId, members));
            
            break;
        }
        
        if (iter->second.size() >= maxGuildMem) {
            
            errorcode = CE_TREASURECOPY_GUILD_MEM_FULL;
            break;
            
        }
        else
        {
            iter->second.insert(role->getInstID());
            break;
        }
        
    } while (false);
    
    if (errorcode == CE_OK) {
        
        playerResult.push_back(roleResult);
        
    }
    
    return errorcode;
}

void TreasureCopyItem::leaveTreasureCopy(Role* role)
{
    
    for (int i = 0; i < playerResult.size(); i++) {
        
        GuildTreasureFightResult* tmp = playerResult[i];
        
        if (tmp && tmp->ownerid == role->getInstID()) {
            
            playerResult.erase(playerResult.begin() + i);
            
            break;
            
        }
    }
    
    int guildId = SRoleGuild(role->getInstID()).getGuild();
    
    if (guildId == 0) {
        return;
    }
    
    map<int, TreasureGuildMember>::iterator iter = groups.find(guildId);
    if (iter == groups.end()) {
        return;
    }
    
    iter->second.erase(role->getInstID());
    
}

void TreasureCopyItem::getTreasurecopyInfo(obj_treasure_scene& obj, int guildid)
{
    obj.copyNum = copyNum;
    obj.sceneid = sceneid;
    obj.sceneMod = sceneMod;
    obj.maxnum = maxPlayerCount;
    obj.curnum = playerResult.size();
    obj.copyType = copyType;
    obj.isOpen = isOpen;
    obj.is_guild_member_full = false;
    obj.sceneName = sceneName;
    
    map<int, TreasureGuildMember>::iterator iter = groups.find(guildid);
    if (iter != groups.end())
    {
        if(iter->second.size() >= maxGuildMem)
        {
            obj.is_guild_member_full = true;
        }
    }
}

void TreasureCopyItem::getPlayers(vector<int>& outdata)
{
    int playercount = playerResult.size();
    
    for (int i = 0; i < playercount; i++) {
        outdata.push_back(playerResult[i]->ownerid);
    }
}

/******************************** TreasureCopyMgr ******************************/

TreasureCopyItem* TreasureCopyMgr::createTreasureCopy(int copyNum, int sceneid)
{

    TreasureCopyListCfgDef* cfg = TreasureListCfg::getTreasureCopyCfg(copyNum);
    if (cfg == NULL) {
        return NULL;
    }
    
    TreasureCopyItem* newcopy = new TreasureCopyItem;
    
    newcopy->copyNum = copyNum;
    newcopy->sceneid = sceneid;
    newcopy->sceneMod = cfg->sceneMod;
    newcopy->copyType = cfg->treasureType;
    newcopy->maxPlayerCount = cfg->maxPlayerCount;
    newcopy->maxGuildMem = cfg->maxGuildMemCount;
    newcopy->sceneName = cfg->sceneName;
    
    newcopy->groups.clear();
    newcopy->isOpen = true;
    mTreasureCopys.push_back(newcopy);
    
    return newcopy;
}

bool TreasureCopyMgr::closeAllTreasureCopy()
{
    //发CMD通知同步线程副本将要关掉
    
    int vsize = mTreasureCopys.size();
    
    for (int i = 0; i < vsize; i++) {
        
        TreasureCopyItem* item = mTreasureCopys[i];
        
        if (item) {
            create_cmd(DestroyWorldScene, cmd);
            cmd->sceneId = item->sceneid;
            sendMessageToWorld(0, CMD_MSG, cmd, 0);
        }
    }
    return true;
}

void TreasureCopyMgr::onCloseTreasureCopy(int copyid)
{    
    for (int i = 0; i < mTreasureCopys.size(); i++) {
        
        TreasureCopyItem* item = mTreasureCopys[i];
        
        if (item != NULL && item->sceneid == copyid) {
            
            delete item;
            
            mTreasureCopys.erase(mTreasureCopys.begin() + i);
            
            break;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
        }
    }
    
}

vector<TreasureCopyItem*>& TreasureCopyMgr::getValidTreasureCopyList()
{
    return mTreasureCopys;
}

TreasureCopyItem* TreasureCopyMgr::getTreasureCopy(int sceneid)
{
    int vsize = mTreasureCopys.size();
    
    for (int i = 0; i < vsize; i++) {
        if (mTreasureCopys[i]->sceneid == sceneid) {
            return mTreasureCopys[i];
        }
    }
    return NULL;
}

int TreasureCopyMgr::getCopyCount()
{
    return mTreasureCopys.size();
}

void TreasureCopyMgr::update()
{
    int copys = mTreasureCopys.size();
    
    for (int i = 0; i < copys; i++) {
        mTreasureCopys[i]->update();
    }
}

//vector<int> TreasureCopyMgr::clearTreasureCopys()
//{
//    vector<int> playersIncopy;
//    playersIncopy.clear();
//    
//    int vsize = mTreasureCopys.size();
//    
//    for (int i = 0; i < vsize; i++) {
//        TreasureCopyItem* item = mTreasureCopys[i];
//        
//        if (item != NULL) {
//            
//            item->getPlayers(playersIncopy);
//            
//            //发CMD通知同步线程副本将要关掉
//            
//            
//            
//            delete item;
//        }
//    }
//    
//    return playersIncopy;
//}

/**************************** GuildTreasureFightMgr ******************************/

void GuildTreasureFightMgr::guildTreasureFightInit()
{

    loadDataFromDB();
    
    SActMgr.SafeCreateTimer(ae_guild_treasure_fight, att_open, 0, guildTreasureFighActOpen, "guildTreasureOpenSec");
    
    SActMgr.SafeCreateTimer(ae_guild_treasure_fight, att_close, 0, guildTreasureFighActClose, "");
}

void GuildTreasureFightMgr::loadPlayerPoints(int weekday)
{
    string prefix = makeGuildTreasureDBkeyPrefix(weekday);
    
    
    
    //加载那些积分                           :player_result
    RedisResult result(redisCmd("hgetall %s:player_result", prefix.c_str()));
    
    for (int i = 0; i < result.getHashElements(); i += 2) {
        int roleid = result.readHash(i, 0);
        string resultData = result.readHash(i + 1, "");
        
        Json::Reader reader;
        Json::FastWriter writer;
        
        Json::Value jsonval;
        reader.parse(resultData.c_str(), jsonval);
        
        RedisResult getRoleInfo(redisCmd("hmget role:%d %s %s", roleid, GetRolePropName(eRolePropRoleName), GetRolePropName(eRolePropLvl)));
        
        string rolename = getRoleInfo.readHash(0, "");
        int rolelvl = getRoleInfo.readHash(1, 0);
        
        if (rolename.empty()) {
            doRedisCmd("hdel %s:player_result %d", prefix.c_str(), roleid);
            continue;
        }
        
        GuildTreasureFightResult* playerresult = createPlayerResult(weekday, roleid, rolename, rolelvl);
        playerresult->points = jsonval["points"].asInt();
        playerresult->monsterKilled = jsonval["monstercount"].asInt();
        playerresult->enemyKilled = jsonval["enemysKilled"].asInt();
        playerresult->assists = jsonval["assists"].asInt();
        playerresult->hasGetAward = jsonval["getaward"].asBool();
    }
    
}

void GuildTreasureFightMgr::loadGuildPoints(int weekday)
{
    string prefix = makeGuildTreasureDBkeyPrefix(weekday);
    
    RedisResult result(redisCmd("exists %s:guild_points", prefix.c_str()));
    
    if (!result.readInt()) {
        return;
    }
    
    result.setData(redisCmd("hgetall %s:guild_points", prefix.c_str()));
    for (int i = 0; i < result.getHashElements();) {
        int guildid = result.readHash(i, 0);
        int points = result.readHash(i + 1, 0);
        
        Guild& guild = SGuildMgr.getGuild(guildid);
        
        GuildTreasureFightResult* guildresult = createGuildResult(weekday, guildid);
        guildresult->points = points;
        guildresult->ownerName = guild.getGuildName();
        
        i += 2;
    }
    
    map<int, TreasureFightResultArray>::iterator iter = mGuildFightResult.find(weekday);
    
    if (iter != mGuildFightResult.end()) {
        ::sort(iter->second.begin(), iter->second.end(), sortByPoints);
    }
}

void GuildTreasureFightMgr::loadManors(int weekday)
{
    string prefix = makeGuildTreasureDBkeyPrefix(weekday);
    
    RedisResult result(redisCmd("exists %s:manors", prefix.c_str()));
    
    if (!result.readInt()) {
        return;
    }
    
    vector<ManorDataStruct> manors;
    manors.clear();
    
    result.setData(redisCmd("hgetall %s:manors", prefix.c_str()));
    
    for (int i = 0; i < result.getHashElements();) {
        int position = result.readHash(i, 0);
        string str = result.readHash(i + 1, "");
		
		ManorDataStruct data;
		data.position = position;
		
		// 以前的旧数据, 这里写兼容处理
		Json::Value val;
		if (!xyJsonSafeParse(str, val))
		{
			data.guildName = str;
		}
		else
		{
			data.guildId = xyJsonAsInt(val["guildId"]);
			data.guildName = xyJsonAsString(val["name"]);
		}
		
        manors.push_back(data);
        
        i += 2;
    }
    
    //更新内存信息
    map<int, vector<ManorDataStruct> >::iterator iter = mGuildsManor.find(weekday);
    
    if (iter == mGuildsManor.end()) {
        mGuildsManor.insert(make_pair(weekday, manors));
    }
    else{
        mGuildsManor.erase(iter);
        mGuildsManor.insert(make_pair(weekday, manors));
    }
}

void GuildTreasureFightMgr::loadDataFromDB()
{
    RedisResult result(redisCmd("hmget serverdata guildTreasureFightOpenTime guildTreasureFightState"));
    
    mOpenTime = (time_t)result.readHash(0, 0);
    mRunning = result.readHash(1, 0);
    
    //加载公会的积分
    for (int i = 0; i < 7; i++) {
        
        //加载玩家的积分
        loadPlayerPoints(i);
        
        loadGuildPoints(i);
        
        //加载领地
        loadManors(i);
    }
    
    bool isActicityOpenning = (Game::tick - mOpenTime) < GuildTreasureFightCfg::mCfg.activityDuration;
    
    //再次启动的时候活动已经结束了，处理一些活动关闭时的事情
    if (!isActicityOpenning && mRunning) {
        activityClose();
    }
    
}

void GuildTreasureFightMgr::activityOpen(int opentime)
{
    if (mRunning) {
        return;
    }
    
    SystemBroadcast bc;
    
    int weekday = Utils::getWeekDay(opentime);
    
    string cityName = guildFightGetCityName(weekday);
    
    string str = SystemBroadcastCfg::getValue("guildtreasure_start", "content");
    //assert(str.empty() != true);
    bc << str.c_str() << cityName.c_str();
    bc.send();

    
    mDbKeyPrefix = makeGuildTreasureDBkeyPrefix(weekday);
    
    //删除数据库里的积分
    doRedisCmd("del %s:player_result", mDbKeyPrefix.c_str());
    doRedisCmd("del %s:guild_points", mDbKeyPrefix.c_str());
    
    //清楚上周今天的数据
    clearPlayerPoints(weekday);
    clearGuildPoints(weekday);
    
    //清除上次活动玩家的临时状态数据
    mPlayerState.clear();
    
    //设置活动开始的时刻
    setOpenTime(opentime);
    
    //开启副本
    checkIncreaseCopy();
}

void GuildTreasureFightMgr::activityClose()
{
    mRunning = false;
    
    doRedisCmd("hset serverdata guildTreasureFightState 0");
    
    int weekday = Utils::getWeekDay(mOpenTime);
    
    //重排公会排名
    sortGuildResult(weekday);
    
    //发送公会奖励
    sendGuildWealthAward(weekday);
    
    //刷新领地
    updateManors(weekday);
    
    //关闭副本
    mTreasureCopyMgr.closeAllTreasureCopy();
    
    notify_treasurecopy_activity_result report;
    
    makeFightingReportOfGuilds(report);
    
    map<int, TreasurecopyPlayerState>::iterator iter = mPlayerState.begin();
    map<int, TreasurecopyPlayerState>::iterator endIter = mPlayerState.end();
    
    for (; iter != endIter; iter++) {
        sendFightingReportToPlayer(iter->first, report);
    }
    
	// 称号的检查
	this->checkGuildRankHonor(weekday);
	
    //发送奖励邮件
    sendPlayerAwardMail(weekday);
	
    //清掉玩家状态
    mPlayerState.clear();
}

bool GuildTreasureFightMgr::enterActivity(Role* role)
{
    setPlayerState(role, eTreasureInActivity, 0);
    
    checkIncreaseCopy();
    
    return true;
}

bool GuildTreasureFightMgr::checkIncreaseCopy()
{
    //上次还没创建完，这次不允许
    if (mNewCopyCountRemain) {
        return false;
    }
    
    int deno = GuildTreasureFightCfg::mCfg.copyCountDeno;
    
    check_min(deno, 1);
    
    int allPlayerCoount = mPlayerState.size();
    
    int needCopyCount = allPlayerCoount / deno;   //分母从配置表读
    
    check_min(needCopyCount, GuildTreasureFightCfg::mCfg.minCopyCount);        //最小值要配置表配
    
    int currentCount = mTreasureCopyMgr.getCopyCount();
    
    int newcount = needCopyCount - currentCount;
    
    if (newcount > 0) {
      
        for (int i = 1; i <= newcount; i++) {
            TreasureCopyListCfgDef* cfg = TreasureListCfg::getTreasureCopyCfg(currentCount+i);
            if (cfg == NULL) {
                break;
            }
            
            create_cmd(CreateWorldScene, createWorldScene);
            createWorldScene->sceneMod = cfg->sceneMod;
            string numstr = Utils::itoa(cfg->sceneNum);
            createWorldScene->extendStr = numstr;
            mNewCopyCountRemain++;
            sendMessageToWorld(0, CMD_MSG, createWorldScene, 0);
        }
    }
    
    return true;
}

bool GuildTreasureFightMgr::onCreateTreasureCopy(int copynum, int sceneid)
{

    TreasureCopyItem* newcopy = mTreasureCopyMgr.createTreasureCopy(copynum, sceneid);
    
    if (newcopy == NULL) {
        return false;
    }
    
    if (!mRunning) {
        mRunning = true;
        
        doRedisCmd("hset serverdata guildTreasureFightState 1");
    }
    
    mNewCopyCountRemain--;
    
    notifyTreasureNewCopyAdd(newcopy);
    
    return false;
}

bool GuildTreasureFightMgr::leaveActivity(Role* role)
{
    erasePlayerState(role);
    
    checkReduceCopy();
    
    return true;
}

bool GuildTreasureFightMgr::checkReduceCopy()
{
    return true;
}

extern void enterWorldScene(int sceneInstId, const WorldPlayerInfo& playerInfo);
extern WorldPlayerInfo getWorldPlayerInfo(Role* role);

int GuildTreasureFightMgr::enterTreasureCopy(Role *role, int sceneid)
{
    if (role->getGuildid() == 0) {
        return CE_GUILD_NOT_EXIST;
    }
    
    TreasureCopyItem* copy = mTreasureCopyMgr.getTreasureCopy(sceneid);
    
    if (copy == NULL) {
        return CE_TREASURECOPY_COPY_NOT_EXIST;
    }
    
    if (!copy->isOpen) {
        return CE_TREASURECOPY_COPY_NOT_OPEN;
    }
    
    GuildTreasureFightResult* playerResult = getPlayerFightResult(role->getInstID());
    
    if (playerResult == NULL) {
        int weekday = Utils::getWeekDay(mOpenTime);
        playerResult = createPlayerResult(weekday, role->getInstID(), role->getRolename(), role->getLvl());
    }
    
    int errorcode =  copy->enterTreasureCopy(role, playerResult);
    
    if (errorcode == CE_OK) {
        role->addTreasureFightBuff();
        WorldPlayerInfo playerInfo = getWorldPlayerInfo(role);
        if (copy->sceneid)
        {
            Json::Value value;
            Json::FastWriter writer;
            value["guild_id"] = SRoleGuild(role->getInstID()).getGuild();
            value["score"] = playerResult->points;
            playerInfo.mProps = writer.write(value);
            enterWorldScene(copy->sceneid, playerInfo);
        }
        
        //副本人数增加
        mPlayerCountIncopy++;
        
        notifyTreasureCopyChange(copy);
        
        setPlayerState(role, eTreasureIncopy, copy->sceneid);
    }
    
    return errorcode;
}

TreasureCopyItem* GuildTreasureFightMgr::quickEnterTreasureCopy(Role *role)
{
    vector<TreasureCopyItem*>& copylist = mTreasureCopyMgr.getValidTreasureCopyList();
    
    int vsize = copylist.size();
    
    for (int i = 0; i < vsize; i++) {
        
        GuildTreasureFightResult* playerResult = getPlayerFightResult(role->getInstID());
        
        if (playerResult == NULL) {
            int weekday = Utils::getWeekDay(mOpenTime);
            playerResult = createPlayerResult(weekday, role->getInstID(), role->getRolename(), role->getLvl());
        }
        
        int errorcode = copylist[i]->enterTreasureCopy(role, playerResult);
        
        if (errorcode == CE_OK) {
            
            role->addTreasureFightBuff();
            WorldPlayerInfo playerInfo = getWorldPlayerInfo(role);
            if (copylist[i]->sceneid)
            {
                Json::Value value;
                Json::FastWriter writer;
                value["guild_id"] = SRoleGuild(role->getInstID()).getGuild();
                value["score"] = playerResult->points;
                playerInfo.mProps = writer.write(value);
                enterWorldScene(copylist[i]->sceneid, playerInfo);
            }
            
            mPlayerCountIncopy++;
            
            notifyTreasureCopyChange(copylist[i]);
            
            setPlayerState(role, eTreasureIncopy, copylist[i]->sceneid);
            
            return copylist[i];
        }
    }
    
    return NULL;
}

int GuildTreasureFightMgr::leaveTreasureCopy(Role *role, int sceneid, bool isDeath)
{
    erasePlayerState(role);
    
    if (sceneid == 0) {
        return CE_TREASURECOPY_COPY_NOT_EXIST;
    }
    
    TreasureCopyItem* copy = mTreasureCopyMgr.getTreasureCopy(sceneid);
    if (copy == NULL) {
        return CE_TREASURECOPY_COPY_NOT_EXIST;
    }
    copy->leaveTreasureCopy(role);
    
    //副本人数减一
    mPlayerCountIncopy--;
    
    notifyTreasureCopyChange(copy);
    
    //设置苏醒时间
    int recoverSec = GuildTreasureFightCfg::getRecoverSec(isDeath);
    
    setPlayerRecoverSec(role->getInstID(), recoverSec);
    
    return CE_OK;
}

vector<TreasureCopyItem*>& GuildTreasureFightMgr::getTreasureCopyList()
{
    return mTreasureCopyMgr.getValidTreasureCopyList();
}

int GuildTreasureFightMgr::getActivityDuration()
{
    return (mOpenTime + GuildTreasureFightCfg::mCfg.activityDuration) - Game::tick;
}

void GuildTreasureFightMgr::setOpenTime(int opentime)
{
    mOpenTime = opentime;
    
    doRedisCmd("hmset serverdata guildTreasureFightOpenTime %d", mOpenTime);
}

void GuildTreasureFightMgr::notifyTreasureCopyChange(TreasureCopyItem *copy)
{
    map<int, TreasurecopyPlayerState>::iterator iter = mPlayerState.begin();
    
    map<int, TreasurecopyPlayerState>::iterator enditer = mPlayerState.end();
    
    //目测比较耗效率，可能要优化
    for (; iter != enditer; iter++) {
        
        int roleid = iter->first;
        
        if (iter->second.state == eTreasureInActivity) {
            
            int guildid = SRoleGuildMgr.getRoleGuild(roleid).getGuild();
            notify_guild_treasurecopy_change change;
            copy->getTreasurecopyInfo(change.copy, guildid);
            sendNetPacket(iter->second.sessionid, &change);
            
        }
    }
}

void GuildTreasureFightMgr::notifyTreasureNewCopyAdd(TreasureCopyItem *newcopy)
{
    map<int, TreasurecopyPlayerState>::iterator iter = mPlayerState.begin();
    
    map<int, TreasurecopyPlayerState>::iterator enditer = mPlayerState.end();
    
    //目测比较耗效率，可能要优化
    for (; iter != enditer; iter++) {
        int roleid = iter->first;
        
        if (iter->second.state == eTreasureInActivity) {
            
            int guildid = SRoleGuildMgr.getRoleGuild(roleid).getGuild();
            
            notify_guild_new_treasurecopy change;
            newcopy->getTreasurecopyInfo(change.copy, guildid);
            sendNetPacket(iter->second.sessionid, &change);
        }
    }
}

void GuildTreasureFightMgr::playerDisconnect(Role *role)
{
    if (role == NULL) {
        return;
    }
    
    int sceneid = getPlayerState(role).copyid;
    
    leaveTreasureCopy(role, sceneid, false);
    
    leaveActivity(role);
    
    erasePlayerState(role);
}

int GuildTreasureFightMgr::getTreasureCopyList(int beginIndex, int endIndex, int playerGuildid, vector<obj_treasure_scene> &outdata)
{
    vector<TreasureCopyItem*>& copylist = g_GuildTreasureFightMgr.getTreasureCopyList();
    int vsize = copylist.size();
    
    if (beginIndex >= endIndex || beginIndex >= vsize) {

        return CE_TREASURECOPY_COPY_NOT_EXIST;
    }
    
    for (int i = beginIndex; i < vsize && i <= endIndex; i++) {
        obj_treasure_scene sceneinfo;
        
        copylist[i]->getTreasurecopyInfo(sceneinfo, playerGuildid);
        
        outdata.push_back(sceneinfo);
    }
    
    return CE_OK;
}

void GuildTreasureFightMgr::setPlayerRecoverSec(int roleid, int recoverSec)
{
//    mPlayerRecoverCountdown[roleid] = Game::tick + GuildTreasureFightCfg::mCfg.recoverSec;
    mPlayerRecoverCountdown[roleid] = Game::tick + recoverSec;
}

int GuildTreasureFightMgr::getPlayerRecoverSec(int roleid)
{
    map<int, int>::iterator iter = mPlayerRecoverCountdown.find(roleid);
    
    if (iter != mPlayerRecoverCountdown.end()) {
        int cooldown = iter->second - Game::tick;
        
        check_min(cooldown, 0);
        
        return cooldown;
    }
    
    return 0;
}

GuildTreasureFightResult* GuildTreasureFightMgr::getPlayerFightResult(int roleid, int weekday)
{
    if (weekday < 0) {
        weekday = Utils::getWeekDay(mOpenTime);
    }
    
    map<int, TreasureFightResultArray>::iterator iter = mPlayerFightResult.find(weekday);
    
    map<int, TreasureFightResultArray>::iterator endIter = mPlayerFightResult.end();
    
    if (iter == endIter) {
        return NULL;
    }
    
    int vsize = iter->second.size();
    
    for (int i = 0; i < vsize; i++) {
        
        GuildTreasureFightResult* tmp = iter->second[i];
        
        if (tmp && tmp->ownerid == roleid) {
            return tmp;
        }
    }
    
    return NULL;
}

GuildTreasureFightResult* GuildTreasureFightMgr::createPlayerResult(int weekday, int roleid, string rolename, int rolelvl)
{
    map<int, TreasureFightResultArray>::iterator iter = mPlayerFightResult.find(weekday);
    
    if (iter == mPlayerFightResult.end()) {
        
        TreasureFightResultArray newarray;
        newarray.clear();
        
        iter = mPlayerFightResult.insert(make_pair(weekday, newarray)).first;
    }
    
    GuildTreasureFightResult* newresult = new GuildTreasureFightResult(roleid, rolelvl);
    
    if (newresult == NULL) {
        return NULL;
    }
    
    newresult->ownerName = rolename;
    iter->second.push_back(newresult);
    
    return newresult;
}

bool GuildTreasureFightMgr::addPlayerFightPoints(int roleid, int addPoints, int addEnemysKilled, int addMonstersKilled, int addAssists)
{
    int weekday = Utils::getWeekDay(mOpenTime);
    
    GuildTreasureFightResult* result = getPlayerFightResult(roleid, weekday);

    if (result == NULL) {
        return false;
    }
    
    result->points += addPoints;
    result->assists += addAssists;
    result->enemyKilled += addEnemysKilled;
    result->monsterKilled += addMonstersKilled;
    result->hasGetAward = false;
    
    //保存到数据库
    savePlayerResult(result, weekday);
    
    Guild guild = SGuildMgr.getRoleGuild(roleid );
    
    addGuildFightPoints(guild, addPoints);
    
    Role* role = SRoleMgr.GetRole(roleid);
    if (role && role->getDailyScheduleSystem()) {
        role->getDailyScheduleSystem()->onEvent(kDailyScheduleSyncTreasureFight, 0, 1);
    }
    
//    Json::Value value;
//    value["points"] = result->points;
//    value["monstercount"] = result->monsterKilled;
//    value["assists"] = result->assists;
//    value["boxeskilled"]= result->boxes;
//    value["getaward"] = result->hasGetAward;
//
//    string jsonStr = Json::FastWriter().write(value);
//    
//    //保存到DB,暂时不保存
//    doRedisCmd("hset %s:player_result %d %s", mDbKeyPrefix.c_str(), roleid, jsonStr.c_str());
    
    return true;
}


void GuildTreasureFightMgr::sortPlayersResult()
{
//    ::sort(mPlayerFightResult.begin(), mPlayerFightResult.end(), sortByPoints);
}

GuildTreasureFightResult* GuildTreasureFightMgr::getGuildFightResult(int weekday, int guildid)
{
    map<int, TreasureFightResultArray>::iterator iter = mGuildFightResult.find(weekday);
    
    if (iter == mGuildFightResult.end()) {
        return NULL;
    }
    
    int vsize = iter->second.size();
    
    for (int i = 0; i < vsize; i++) {
        GuildTreasureFightResult* tmp = iter->second[i];
        
        if(tmp && tmp->ownerid == guildid)
        {
            return tmp;
        }
    }
    
    return NULL;
}

void GuildTreasureFightMgr::clientGetGuildResult(int guildid, int &guildpoints, int &guildRank, int weekday)
{
    bool resort = false;
    
    if (weekday < 0) {
        weekday = Utils::getWeekDay(mOpenTime);
        resort = true;
    }
    
    map<int, TreasureFightResultArray>::iterator iter = mGuildFightResult.find(weekday);
    
    if (iter == mGuildFightResult.end()) {
        return;
    }
    
    //如果是获取非当天活动的不用排序
    if (resort) {
        ::sort(iter->second.begin(), iter->second.end(), sortByPoints);
    }
    
    int vsize = iter->second.size();
    
    for (int i = 0; i < vsize; i++) {
        GuildTreasureFightResult* tmp = iter->second[i];
        
        if(tmp && tmp->ownerid == guildid)
        {
            guildpoints = tmp->points;
            guildRank = i + 1;
            return;
        }
    }
}

GuildTreasureFightResult* GuildTreasureFightMgr::createGuildResult(int weekday, int guildid)
{
    map<int, TreasureFightResultArray>::iterator iter = mGuildFightResult.find(weekday);
    
    if (iter == mGuildFightResult.end()) {
        
        TreasureFightResultArray newarray;
        newarray.clear();
        
        iter = mGuildFightResult.insert(make_pair(weekday, newarray)).first;
    }
    
    GuildTreasureFightResult* newresult = new GuildTreasureFightResult(guildid, 0);
    
    if (newresult == NULL) {
        return NULL;
    }
    
    iter->second.push_back(newresult);
    
    return newresult;
}

bool GuildTreasureFightMgr::addGuildFightPoints(Guild& guild, int addPoints)
{
    if (guild.getId() == 0) {
        return false;
    }
    int weekday = Utils::getWeekDay(mOpenTime);
    
    GuildTreasureFightResult* result = getGuildFightResult(weekday, guild.getId());
    
    if (result == NULL) {
        result = createGuildResult(weekday, guild.getId());
        if (result == NULL) {
            return false;
        }
        
        result->ownerName = guild.getGuildName();
    }
    
    result->points += addPoints;
    
    //保存到DB, 暂时不保存
    doRedisCmd("hset %s:guild_points %d %d", mDbKeyPrefix.c_str(), guild.getId(), result->points);
    
    return true;
}

void GuildTreasureFightMgr::sortGuildResult(int weeekday)
{
    map<int, TreasureFightResultArray>::iterator iter = mGuildFightResult.find(weeekday);
    
    if (iter == mGuildFightResult.end()) {
        return;
    }
    ::sort(iter->second.begin(), iter->second.end(), sortByPoints);
}

TreasureCopyItem* GuildTreasureFightMgr::getTreasurecopy(int sceneid)
{
    return mTreasureCopyMgr.getTreasureCopy(sceneid);
}


void GuildTreasureFightMgr::clearGuildPoints(int weekday)
{
    map<int , TreasureFightResultArray>::iterator iter = mGuildFightResult.find(weekday);
    
    if (iter == mGuildFightResult.end()) {
        return;
    }
    
    int vsize = iter->second.size();
    
    for (int i = 0; i < vsize; i++) {
        GuildTreasureFightResult* tmp = iter->second[i];
        
        if (tmp) {
            delete tmp;
        }
    }
    
    mGuildFightResult.erase(iter);
}

void GuildTreasureFightMgr::clearPlayerPoints(int weekday)
{
    map<int, TreasureFightResultArray>::iterator iter = mPlayerFightResult.find(weekday);
    
    if (iter == mPlayerFightResult.end()) {
        return;
    }

    int vsize = iter->second.size();
    
    for (int i = 0; i < vsize; i++) {
        GuildTreasureFightResult* tmp = iter->second[i];
        
        if (tmp) {
            delete tmp;
        }
    }
    
    iter->second.clear();
}

void GuildTreasureFightMgr::updateManors(int weekday)
{
    map<int, TreasureFightResultArray>::iterator resultIter = mGuildFightResult.find(weekday);
    
    if (resultIter == mGuildFightResult.end()) {
        return;
    }
    
    vector<ManorDataStruct> vtmp;
    vtmp.clear();
    
    //系统公告的串
    string broadcastStr;
    
    //保存到数据库
    RedisArgs args;
    string cmdBegin = Utils::makeStr("%s:manors", mDbKeyPrefix.c_str());
    RedisHelper::beginHmset(args, cmdBegin.c_str());
    
    int guildcount = resultIter->second.size();
    
    for (int i = 0; i < guildcount; i++) {
        
        if (resultIter->second[i] == NULL) {
            continue;
        }
        
        ManorDataStruct data;
		data.guildId = resultIter->second[i]->ownerid;
        data.guildName = resultIter->second[i]->ownerName;
        data.position = vtmp.size() + 1;
        
        vtmp.push_back(data);
        
		Json::Value val;
		val["name"] = data.guildName;
		val["guildId"] = data.guildId;

        RedisHelper::appendHmset(args, data.position, xyJsonWrite(val).c_str());
        
        broadcastStr.append(data.guildName);
        broadcastStr.append(", ");
        
//        broadcastStr = broadcastStr.substr(0, broadcastStr.size() - 2);
        
        if (vtmp.size() >= 5) {
            break;
        }
    }
    
    //更新内存信息
    map<int, vector<ManorDataStruct> >::iterator iter = mGuildsManor.find(weekday);
    
    if (iter == mGuildsManor.end()) {
        mGuildsManor.insert(make_pair(weekday, vtmp));
    }
    else{
        mGuildsManor.erase(iter);
        mGuildsManor.insert(make_pair(weekday, vtmp));
    }
    
    //清掉原来的数据
    doRedisCmd("del %s", cmdBegin.c_str());
    
    //提交到数据库
    RedisHelper::commitHmset(get_DbContext(), args);
    
    //这里全服公告
    if (!broadcastStr.empty()) {
        broadcastStr = broadcastStr.substr(0, broadcastStr.size() - 2);
        broadcastStr.append(".");
        
        string cityname = guildFightGetCityName(weekday);
        
        SystemBroadcast bc;

        string str = SystemBroadcastCfg::getValue("guildtreasure_fight_report", "content");
        //assert(str.empty() != true);
        bc << str.c_str() << broadcastStr.c_str() << vtmp.size() << cityname.c_str();
        bc.send();
    }
}

void GuildTreasureFightMgr::clientGetManors(int roleid, ack_treasurecopy_get_manors &ack)
{
    map<int, vector<ManorDataStruct> >::iterator iter = mGuildsManor.begin();
    
    map<int, vector<ManorDataStruct> >::iterator endIter = mGuildsManor.end();
    
    for (; iter != endIter; iter++) {
        obj_treasurecopy_manor manor;
        
        manor.index = iter->first;
        
        GuildTreasureFightResult* playerData = getPlayerFightResult(roleid, iter->first);
        
        if (playerData) {
            manor.canGetAward = !(playerData->hasGetAward);
        }
        else{
            manor.canGetAward = false;
        }
        
        for (int i = 0; i < iter->second.size(); i++) {
            manor.guildsName.push_back(iter->second[i].guildName);
        }
        
        ack.manors.push_back(manor);
    }
}

void GuildTreasureFightMgr::savePlayerResult(GuildTreasureFightResult *playerresult, int weekday)
{
    if (playerresult == NULL) {
        return;
    }
    
    string prefix = makeGuildTreasureDBkeyPrefix(weekday);
    
    Json::Value value;
    value["points"] = playerresult->points;
    value["enemysKilled"] = playerresult->enemyKilled;
    value["monstercount"] = playerresult->monsterKilled;
    value["assists"] = playerresult->assists;
    value["getaward"] = playerresult->hasGetAward;
    
    string jsonStr = Json::FastWriter().write(value);
    
    //保存到DB,暂时不保存
    doRedisCmd("hset %s:player_result %d %s", prefix.c_str(), playerresult->ownerid, jsonStr.c_str());
}

int GuildTreasureFightMgr::getMyGuildManorAward(int weekday, Role *role, string &award)
{
    if (mRunning) {
        int nowweekday = Utils::getWeekDay(mOpenTime);
        
        if (nowweekday == weekday) {
        
            return CE_ACTIVITY_IS_OPENNING;
        }
    }
    
    if (role->getGuildid() == 0) {
        return CE_GUILD_NOT_EXIST;
    }
    
    GuildTreasureFightResult* playerResult = getPlayerFightResult(role->getInstID(), weekday);
    
    if (playerResult == NULL) {
        return CE_TREASURECOPY_YOU_NOT_TAKE_PARTIN_ACT;
    }
    
    if (playerResult->hasGetAward) {
        return CE_TREASURECOPY_YOU_CAN_NOT_GET_MANOR_AWARD;
    }
    
    int guildPoints = 0;
    int guildRank = 0;
    
    clientGetGuildResult(role->getGuildid(), guildPoints, guildRank, weekday);
    
    if (guildRank == 0 || guildPoints == 0) {
        return CE_TREASURECOPY_YOU_NOT_TAKE_PARTIN_ACT;
    }
    
    //根据排名积分发奖励
    award = "";
    
    try
    {
        CMsgTyped msg;
        msg.SetInt(guildRank);
        
        if (GameScript::instance()->call("treasurefight_getGuildManorAward", msg) >= 0)
        {
            int type = msg.TestType();
            if (type == TYPE_STRING)
            {
                award = msg.StringVal();
            }
        }
    }
    catch(...)
    {
    }
    
    vector<string> itemstrs = StrSpilt(award, ";");
    ItemArray items;
    RewardStruct reward;
    rewardsCmds2ItemArray(itemstrs, items, reward);
    GridArray grids;

    if(role->preAddItems(items, grids) != CE_OK)
    {
        return CE_BAG_FULL;
    }
    
    
    playerResult->hasGetAward = true;
    
    savePlayerResult(playerResult, weekday);
    
    role->addAwards(itemstrs, items, "guild_manor");
    
    return CE_OK;
}

string GuildTreasureFightMgr::getPlayerAward(GuildTreasureFightResult *playerResult)
{
    if (playerResult == NULL) {
        return "";
    }
    
    string award = "";
    
    try
    {
        CMsgTyped msg;
        msg.SetInt(playerResult->points);
        msg.SetInt(playerResult->ownerLvl);
        
        if (GameScript::instance()->call("treasurefight_getPlayerAward", msg) >= 0)
        {
            int type = msg.TestType();
            if (type == TYPE_STRING)
            {
                award = msg.StringVal();
            }
        }
    }
    catch(...)
    {
    }
    
    return award;
}

void GuildTreasureFightMgr::makeFightingReportOfGuilds(notify_treasurecopy_activity_result& report)
{
    int weekday = Utils::getWeekDay(mOpenTime);
    
    map<int, TreasureFightResultArray>::iterator iter = mGuildFightResult.find(weekday);
    
    if (iter == mGuildFightResult.end()) {
        return;
    }
    
//    ::sort(iter->second.begin(), iter->second.end(), sortByPoints);
    
    TreasureFightResultArray& guilds = iter->second;
    
    int guildsCount = guilds.size();
    
    for (int i = 0; i < guildsCount; i++) {
        GuildTreasureFightResult* tmp = guilds[i];
        
        if (tmp) {
            obj_treasurecopy_guildresult guild;
            guild.rank = i + 1;
            guild.guildName = tmp->ownerName;
            guild.points = tmp->points;
            report.datas.push_back(guild);
        }
        
        //报告的数目可以配置
        if (report.datas.size() >= 10) {
            break;
        }
    }
}

void GuildTreasureFightMgr::makeFightingReportOfPlayer(Role *role, notify_treasurecopy_activity_result &report)
{
    if (role == NULL ) {
        return;
    }
    
    GuildTreasureFightResult* myResult = getPlayerFightResult(role->getInstID());
    
    if (myResult == NULL) {
        return;
    }
    
    report.myPoints = myResult->points;
    report.assists = myResult->assists;
    report.enemyKilled = myResult->enemyKilled;
    
    do {
        
        if (role->getGuildid() == 0) {
            break;
        }
        
        int myGuildRank = 0;
        int myGuildPoints = 0;
        clientGetGuildResult(role->getGuildid(), myGuildPoints, myGuildRank);
        
        report.myGuildRank = myGuildRank;
        report.myGuildPoints = myGuildPoints;
        
    } while (false);
}

bool GuildTreasureFightMgr::closeAllTreasureCopy()
{
    return mTreasureCopyMgr.closeAllTreasureCopy();
}

bool GuildTreasureFightMgr::onCloseTreasureCopy(int sceneid)
{
    
    mTreasureCopyMgr.onCloseTreasureCopy(sceneid);
    
    return true;
}

void GuildTreasureFightMgr::sendFightingReportToPlayer(int roleid, notify_treasurecopy_activity_result &report)
{
    Role* role = SRoleMgr.GetRole(roleid);
    
    if (role == NULL) {
        return;
    }
    
    makeFightingReportOfPlayer(role, report);
    
    sendNetPacket(role->getSessionId(), &report);
}

int GuildTreasureFightMgr::setPlayerState(Role *role, TreasureCopyState state, int sceneid)
{
    if (role == NULL) {
        return 1;
    }
    
    map<int, TreasurecopyPlayerState>::iterator iter = mPlayerState.find(role->getInstID());
    
    if (iter != mPlayerState.end()) {
        
        iter->second.sessionid = role->getSessionId();
        iter->second.state = state;
        iter->second.copyid = sceneid;
        
        return 0;
    }
    else{
        TreasurecopyPlayerState newstate;
        newstate.sessionid = role->getSessionId();
        newstate.state = state;
        newstate.copyid = sceneid;
        mPlayerState.insert(make_pair(role->getInstID(), newstate));
        return 0;
    }
    
    return 1;
}

void GuildTreasureFightMgr::createPlayerState(Role *role, TreasureCopyState state)
{
    TreasurecopyPlayerState newstate;
    newstate.sessionid = role->getSessionId();
    newstate.state = state;
    mPlayerState.insert(make_pair(role->getInstID(), newstate));
    return;
}

TreasurecopyPlayerState GuildTreasureFightMgr::getPlayerState(Role *role)
{
    TreasurecopyPlayerState state;
    
    if (role == NULL) {
        return state;
    }
    
    map<int, TreasurecopyPlayerState>::iterator iter = mPlayerState.find(role->getInstID());
    
    if (iter != mPlayerState.end()) {
        return iter->second;
    }
    
    return state;
}

void GuildTreasureFightMgr::erasePlayerState(Role *role)
{
    if (role == NULL) {
        return;
    }
    
    map<int, TreasurecopyPlayerState>::iterator iter = mPlayerState.find(role->getInstID());
    
    if (iter != mPlayerState.end()) {
        mPlayerState.erase(iter);
    }
}

bool GuildTreasureFightMgr::checkTreasureFightResulPlayer(int weekday, Role* role)
{
	map<int, TreasureFightResultArray>::iterator playerIt = mPlayerFightResult.find(weekday);
	if (playerIt == mPlayerFightResult.end()) {
		return false;
	}
	
	TreasureFightResultArray& player = playerIt->second;
	for (int i = 0; i < player.size(); i++) {
		GuildTreasureFightResult* p = player[i];
		if (NULL == p) {
			continue;
		}
		if (p->ownerid == role->getInstID()) {
			return true;
		}
	}
	return false;
}

bool GuildTreasureFightMgr::checkTreasureFightResulGuild(vector<ManorDataStruct> &val, int guildId, int &rank)
{
	rank = 0;
	vector<ManorDataStruct>::iterator it;
	for (it = val.begin(); it != val.end(); it++) {
		if (it->guildId != guildId) {
			continue;
		}
		rank = it->position;
		return true;
	}
	return false;
}


int GuildTreasureFightMgr::getOccupyPoints(Role* role)
{
	int sum = 0;
	RoleGuildProperty &prop = SRoleGuild(role->getInstID());
	if (prop.noGuild()) {
		return sum;
	}
	
	map<int, vector<ManorDataStruct> > ::iterator it;
	for (it = mGuildsManor.begin(); it != mGuildsManor.end(); it++) {
		int rank = 0;
		// 所属公会是否在榜上, 返回对应排名
		if (!this->checkTreasureFightResulGuild(it->second, prop.getGuild(), rank)) {
			continue;
		}
		// 是否参与过该场活动
		if (!this->checkTreasureFightResulPlayer(it->first, role)) {
			continue;
		}
		// 获取排名对应的占据分
		sum += GuildChampaGoodsCfg::getOccupied(rank);
	}
	return sum;
}

string awardToMailContentExplanation(string awardStr)
{
    ItemArray items;
    RewardStruct reward;
    vector<string> itemstrs = StrSpilt(awardStr, ";");
    rewardsCmds2ItemArray(itemstrs,items,reward);
    
    vector<string> contents;
    reward2MailFormat(reward, items, &contents, NULL);
    return StrJoin(contents.begin(), contents.end(), ",");
}


void GuildTreasureFightMgr::checkGuildRankHonor(int weekday)
{
	map<int, TreasureFightResultArray>::iterator guildIt = mGuildFightResult.find(weekday);
    if (guildIt == mGuildFightResult.end()) {
        return;
    }
	
	map<int, TreasureFightResultArray>::iterator playerIt = mPlayerFightResult.find(weekday);
	if (playerIt == mPlayerFightResult.end()) {
		return;
	}
	
	int rankOneGuildMaster = 0;
	int rankOneGuildId = 0;
	
	TreasureFightResultArray& guildResults = guildIt->second;
    for (int i = 0; i < guildResults.size(); i++) {
        GuildTreasureFightResult* result = guildResults[i];
        
        if (result == NULL || result->ownerName.empty() || result->points <= 0) {
            continue;
        }
		
		Guild &guild = SGuild(result->ownerid);
		if (guild.isNull()) {
			continue;
		}
		
		// 取得第一名的公会id, 与会长角色id
		if (i == 0) {
			rankOneGuildMaster = guild.getMaster();
			
			RoleGuildProp &ownerProp = SRoleGuild(guild.getMaster());
			rankOneGuildId = ownerProp.getGuild();
		}
		
		TreasureFightResultArray& playerResults = playerIt->second;
		for (int j = 0; j < playerResults.size(); j++) {
			GuildTreasureFightResult* result = playerResults[j];
			
			if (result == NULL || result->ownerName.empty() || result->points <= 0) {
				continue;
			}
			
			if (result->ownerid == guild.getMaster()) {
				SHonorMou.procHonor(eHonorKey_GuildMasterTreasurecopy, guild.getMaster(), xystr("%d", i + 1));
				break;
			}
		}
	}
		
	TreasureFightResultArray& playerResults = playerIt->second;
	for (int i = 0; i < playerResults.size(); i++) {
		GuildTreasureFightResult* result = playerResults[i];
		
		if (result == NULL || result->ownerName.empty() || result->points <= 0) {
			continue;
		}
		
		if (rankOneGuildId == 0) {
			continue;
		}
		
		if (rankOneGuildMaster == result->ownerid)
		{
			continue;
		}
		
		if (SRoleGuild(result->ownerid).getGuild() == rankOneGuildId)
		{
			SHonorMou.procHonor(eHonorKey_GuildTreasurecopy, result->ownerid, "1");
		}
	}
}

void GuildTreasureFightMgr::sendPlayerAwardMail(int weekday)
{
    map<int, TreasureFightResultArray>::iterator iter = mPlayerFightResult.find(weekday);
    
    if (iter == mPlayerFightResult.end()) {
        return;
    }
    
    TreasureFightResultArray& results = iter->second;
    
    int count = results.size();
    
    for (int i = 0; i < count; i++) {
        GuildTreasureFightResult* result = results[i];
        
        if (result == NULL || result->ownerName.empty() || result->points <= 0) {
            continue;
        }
        
        string award = getPlayerAward(result);
        
        MailFormat* formate = MailCfg::getCfg("guild_treasurefight");
        
        if (award.empty() || formate == NULL) {
            continue;
        }
        
        string awardExplanation = awardToMailContentExplanation(award);
        string cityname = guildFightGetCityName(weekday);
        string mail_content = formate->content;
        
        //獎勵：{:str:}
        find_and_replace(mail_content, 2, cityname.c_str(), awardExplanation.c_str());
        
        bool ret = sendMail(0,
                            formate->sendername.c_str(),
                            result->ownerName.c_str(),
                            formate->title.c_str(),
                            mail_content.c_str(),
                            award.c_str(),
                            Utils::itoa(result->ownerid).c_str());
        
        if( false ==  ret )
        {
            StoreOfflineItem(result->ownerid, award.c_str());
        }
        
		Xylog log(eLogName_TreasureFightPlayer, result->ownerid);
		log << weekday << result->ownerLvl << result->points << award;
    }
}

void GuildTreasureFightMgr::clearPlayerDataInDB(int roleid, int guildid)
{
    int weekday = Utils::getWeekDay(mOpenTime);
    
    map<int, TreasureFightResultArray>::iterator iter = mPlayerFightResult.begin();
    
    map<int, TreasureFightResultArray>::iterator endIter = mPlayerFightResult.end();
    
    for (; iter != endIter; iter++) {
        
        TreasureFightResultArray& array = iter->second;
        
        for (int i = 0; i < array.size(); i++) {
            
            GuildTreasureFightResult* playerResult = array[i];
            
            if (playerResult == NULL) {
                continue;
            }
            
            if (playerResult->ownerid == roleid) {
                
                //如果在活动期间
                if (weekday == iter->first && mRunning) {
                    
                    //把公会的积分也删了
                    Guild& myguild = SGuildMgr.getGuild(guildid);
                    addGuildFightPoints(myguild, - playerResult->points);
                    
                }
                
                string prefix = makeGuildTreasureDBkeyPrefix(iter->first);
                
                doRedisCmd("hdel %s:player_result %d", prefix.c_str(), roleid);
                
                delete playerResult;
                
                array.erase(array.begin() + i);
                
                break;
                
            }
            
        }
        
    }
}

void treasureFightGuildAward(int rank, int& fortune, int& construction)
{
    try
    {
        CMsgTyped msg;
        msg.SetInt(rank);
        
        if (GameScript::instance()->call("treasurefight_getGuildWealthAward", msg) >= 0)
        {
            fortune = msg.FloatVal();
            construction = msg.FloatVal();
        }
    }
    catch(...)
    {
    }
}

void GuildTreasureFightMgr::sendGuildWealthAward(int weekday)
{
    map<int, TreasureFightResultArray>::iterator iter = mGuildFightResult.find(weekday);
    
    if (iter == mGuildFightResult.end()) {
        return;
    }
    
    TreasureFightResultArray& array = iter->second;
    
    int guildCount = array.size();
    
    for (int i = 0; i < guildCount; i++) {
        
        GuildTreasureFightResult* gResult = array[i];
        
        if (gResult == NULL) {
            continue;
        }
        
        int fortunAward = 0;
        int constructionAward = 0;
        
        
        treasureFightGuildAward(i+1, fortunAward, constructionAward);
        
        Guild& guild = SGuildMgr.getGuild(gResult->ownerid);
        
        if (guild.getId() == 0) {
            continue;
        }
        
        string allmembers_str = ""; //GameTextCfg::getString("1045");
        string treasureFight = GameTextCfg::getString("1050");
        
        guild.addFortuneConstruction(fortunAward, constructionAward, allmembers_str.c_str(), treasureFight.c_str());
    }
}

void GuildTreasureFightMgr::playerLeaveGuild(Role *role)
{
    
    if (role == NULL) {
        return;
    }
    int sceneid = getPlayerState(role).copyid;
    
    if (sceneid) {
        
        create_cmd(LeaveWorld, leaveWorld);
        leaveWorld->roleId = role->getInstID();
        leaveWorld->sessionId = role->getSessionId();
        sendMessageToWorld(0, CMD_MSG, leaveWorld, 0);
        
        leaveTreasureCopy(role, sceneid, false);
        
        leaveActivity(role);
    }
    
    //删除公会战的东西
    g_GuildTreasureFightMgr.clearPlayerDataInDB(role->getInstID(), role->getGuildid());
}
