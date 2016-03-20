//
//  FamesHall.cpp
//  GameSrv
//
//  Created by pireszhi on 13-11-22.
//
//

#include "FamesHall.h"
#include <vector>
#include "DBRolePropName.h"
#include "EnumDef.h"
#include "Global.h"
using namespace std;
FameHallMgr g_fameHallMgr;

FameHallMgr::FameHallMgr()
{
    mRunning = false;
    mActiveId = ae_fameshall;
    mFamesVersion = 1;
}

void FameHallMgr::release()
{
    mPlayerDataMap.release();
}
void FameHallMgr::init()
{
    mBattleIndex = 0;
    mPlayerBattleIndex.clear();
    
    RedisResult result(redisCmd("hmget serverdata nextfreshfameslisttime nextfreshfamesranktime"));
    int nextfresh = result.readHash(0, 0);
    int nextFreshRank = result.readHash(1, 0);
    if (nextfresh == 0) {
        setNextFreshFamesListTime();
    }
    else{
        mNextFreshFamesListTime = nextfresh;
        //mNextFreshFamesListTime = time(NULL) + 180;
    }

    if (nextFreshRank == 0) {
        setNextFreshRankTime();
    }
    else{
        mNextFreshRankTime = nextFreshRank;
        //mNextFreshRankTime = time(NULL) + 180;
    }

    //加载即时挑战的情况
    loadChallData();

    //加载排行榜
    loadRankFromDB();

    //加载名人列表
    //loadFamesList();
    loadFamesListByRank();
    
    mActiveId = ae_fameshall;
}

void FameHallMgr::loadFamesList()
{
    //从数据库加载名人堂的人物id
    RedisResult famesInhall(redisCmd("hget serverdata famehall"));
    string hallDataStr = famesInhall.readStr();
    if (hallDataStr.empty()) {
        hallDataStr = FamousDataCfg::mData.fameHallList;
        onSaveFameHallList(hallDataStr);
    }

    vector<ObjJob> jobs;
    jobs.push_back(eWarrior);
    jobs.push_back(eMage);
    jobs.push_back(eAssassin);

    StringTokenizer token(hallDataStr, ";");
    if (token.count() < jobs.size()) {
        log_error("fames size error");
        assert(0);
    }

    for (int i = 0; i < token.count() && i < jobs.size(); i++) {
        FamesList list;
        int position = 1;
        ObjJob currJob = jobs[i];
        StringTokenizer fames(token[i], ",");
        
        for (int j = 0; j < fames.count(); j++)
        {
            int fameId = Utils::safe_atoi(fames[j].c_str());
            if (fameId <= 0) {
                continue;
            }

            FamesInfo fameInfo = loadFameInfo(fameId);
            if (fameInfo.name.empty()) {
                continue;
            }
            fameInfo.position = position;
            position++;

            list.push_back(fameInfo);
        }
        mFamesLists[(int)(currJob)] = list;
    }
}

void FameHallMgr::loadFamesListByRank()
{
    string hallDataStr =  FamousDataCfg::mData.fameHallList;
    
    vector<ObjJob> jobs;
    jobs.push_back(eWarrior);
    jobs.push_back(eMage);
    jobs.push_back(eAssassin);
    
    StringTokenizer token(hallDataStr, ";");
    if (token.count() < jobs.size()) {
        log_error("fames size error");
        assert(0);
    }
    
    for (int i = 0; i < token.count() && i < jobs.size(); i++) {
        FamesList list;
        int position = 1;
        ObjJob currJob = jobs[i];
        StringTokenizer fames(token[i], ",");
        
        map<int, FamesHallRank>::iterator rankIter = mRanks.find((int)jobs[i]);
        if (rankIter != mRanks.end())
        {
            FamesHallRank::iterator itrFind = rankIter->second.begin();
            for (; itrFind != rankIter->second.end(); ++itrFind)
            {
                if(position > FamousDataCfg::mData.famesNeedPassLayers)
                    break;
                int fameId = (*itrFind)->roleid;
                if(fameId <= 0)
                    continue;
                FamesInfo fameInfo = loadFameInfo(fameId);
                if (fameInfo.name.empty()) {
                    continue;
                }
                fameInfo.position = position;
                position++;
                
                list.push_back(fameInfo);
            }
        }
        
        for (int j = position - 1; j < fames.count(); j++)
        {
            if(position > FamousDataCfg::mData.famesNeedPassLayers)
                break;
            int fameId = Utils::safe_atoi(fames[j].c_str());
            if (fameId <= 0) {
                continue;
            }
            
            FamesInfo fameInfo = loadFameInfo(fameId);
            if (fameInfo.name.empty()) {
                continue;
            }
            fameInfo.position = position;
            position++;
            
            list.push_back(fameInfo);
        }
        mFamesLists[(int)(currJob)] = list;
    }

}

FamesInfo FameHallMgr::loadFameInfo(int roleid)
{
    // 等级不应该现在赋值，因为不同玩家挑战的名人堂等级不同，先不去掉，只是不用
    FamesInfo info;
    //FamousCfgDef* def = FamousCfg::getFamousCfg(roleid);        //先从配置表中找
    FamousHeroBaseDef* def = FamousDataCfg::getFamousHeroBaseDef(roleid);
    if (def) {
        info.personId = roleid;
//        info.name = def->default_name;
//        info.lvl = def->default_lvl;
//        info.roleType = def->default_roletplt;
//        info.weaponQua = def->default_weapon;
        info.name = def->_name;
        info.lvl = 0;
        info.roleType = def->_roleTplt;
        info.weaponQua = def->_weaponQue;
    }
    else        //如果没找到再去数据库找
    {
        RedisResult result(redisCmd("hmget role:%d %s %s %s %s %s %s", roleid,
                                    GetRolePropName(eRolePropRoleName),
                                    GetRolePropName(eRolePropLvl),
                                    GetRolePropName(eRolePropRoleType),
                                    GetRolePropName(eRolePropWeapQua),
                                    "enchantid",
                                    "enchantlvl",
                                    GetRolePropName(eRoleAwakeLvl)));
        info.personId = roleid;
        info.name = result.readHash(0, "");
        info.lvl = result.readHash(1, 0);
        info.roleType = result.readHash(2, 0);
        info.weaponQua = result.readHash(3, 0);
        info.enchantId = result.readHash(4, 0);
        info.enchantLvl = result.readHash(5, 0);
        info.awakeLvl = result.readHash(6, 0);
    }
    return info;
}

void FameHallMgr::loadRankFromDB()
{
    vector<ObjJob> roleJobs;
    roleJobs.push_back(eWarrior);
    roleJobs.push_back(eMage);
    roleJobs.push_back(eAssassin);

    for (int job = 0; job < roleJobs.size(); job++) {
        string tmp = "paihang:fame_";
        string jobName = RoleTpltCfg::objJobToString(roleJobs[job]);
        if (jobName.empty()) {
            continue;
        }

        tmp.append(jobName);
        FamesHallRank hallRank;

        RedisResult result(redisCmd("zrevrange %s 0 100 withscores", tmp.c_str()));
        int keysCount = result.getSortedSetsCounts();
        int playerRank = 1;

        for (int i = 0; i < keysCount; i++) {
            int points = 0;
            string roleIdStr = "";

            if ( !result.readSortedSets(i, points, roleIdStr) ) {
                break;
            }
            int roleid = Utils::safe_atoi(roleIdStr.c_str());
            const char* props[] =
            {
                GetRolePropName(eRolePropRoleName),
                GetRolePropName(eRolePropRoleType),
                GetRolePropName(eRolePropLvl),
                GetRolePropName(eRolePropWeapQua),
                "enchantid",
                "enchantlvl",
                GetRolePropName(eRoleAwakeLvl),
                NULL,
            };
            string allprops = StrJoin(props, &props[sizeof(props) / sizeof(const char*) - 1], " ");
            string cmd = "hmget role:";
            cmd.append(Utils::itoa(roleid));
            cmd.append(" ");
            cmd.append(allprops.c_str());

            RedisResult playerinfo(redisCmd(cmd.c_str()));

            string name = playerinfo.readHash(0, "");
            int roletype = playerinfo.readHash(1, 0);
            int lvl = playerinfo.readHash(2, 0);
            int weaponqua = playerinfo.readHash(3, 0);
            int chantId = playerinfo.readHash(4, 0);
            int chantLvl = playerinfo.readHash(5, 0);
            int awakeLvl = playerinfo.readHash(6, 0);

            if (name.empty()) {
                doRedisCmd("zrem %s %d", tmp.c_str(), roleid);
                continue;
            }

            FamesRankData* rankdata = new FamesRankData;

            FamesChallData playerData = getPlayerchallData(roleid);

            rankdata->roleid = roleid;
            rankdata->name = name;
            rankdata->roleType = roletype;
            rankdata->lvl = lvl;
            rankdata->points = playerData.currPoints;
            rankdata->topLayerPassed = playerData.topLayer;
            rankdata->rank = playerRank;
            rankdata->weaponQua = weaponqua;
            rankdata->job = RoleType2Job(roletype);
            rankdata->chantid = chantId;
            rankdata->chantLvl = chantLvl;
            rankdata->awakeLvl = awakeLvl;
            
            playerRank++;

            hallRank.push_back(rankdata);

            mPlayerRankData.insert(make_pair(roleid, rankdata));
        }

        mRanks[(int)roleJobs[job]] = hallRank;
    }
}

void FameHallMgr::loadChallData()
{
    mPlayerDataMap.load( g_GetRedisName(rnFameChallData), 0);
}

void FameHallMgr::loadReplaceLog()
{

}

void FameHallMgr::onHeartBeat()
{
    checkRankFresh();

    checkFamesListFresh();
}

void FameHallMgr::checkFamesListFresh()
{
    time_t nowtime = Global::tick;

    if (nowtime >= mNextFreshFamesListTime) {
        freshFamesList();
        // 玩家挑战数据不再清空，因为名人堂刷新后要继续上次的挑战
        // 排名也不应该删除
        //mPlayerDataMap.clear();
        setNextFreshFamesListTime();
        //clearRankInDB();
        
        checkChallDataFresh();
    }
}

void FameHallMgr::checkRankFresh()
{
    time_t nowtime = Global::tick;
    if (nowtime >= mNextFreshRankTime) {
        clearRank();
        loadRankFromDB();
        testRank();
        setNextFreshRankTime();
    }
}

void FameHallMgr::clearRank()
{
    mRanks.clear();
    map<int, FamesRankData*>::iterator iter = mPlayerRankData.begin();
    for (; iter != mPlayerRankData.end(); iter++) {
        FamesRankData* tmp = iter->second;
        delete tmp;
    }
    mPlayerRankData.clear();
}

void FameHallMgr::clearRankInDB()
{
    vector<ObjJob> roleJobs;
    roleJobs.push_back(eWarrior);
    roleJobs.push_back(eMage);
    roleJobs.push_back(eAssassin);

    for (int job = 0; job < roleJobs.size(); job++) {
        string tmp = "paihang:fame_";
        string jobName = RoleTpltCfg::objJobToString(roleJobs[job]);
        if (jobName.empty()) {
            continue;
        }

        tmp.append(jobName);

        doRedisCmd("del %s", tmp.c_str());
    }
}

void FameHallMgr::setNextFreshFamesListTime()
{
//    mNextFreshFamesListTime += 180;
//    return;
    if (Process::env.getInt("testmode") == 1) {
        mNextFreshFamesListTime = Global::tick - Global::tick%300 + 300;
        printf("名人榜替换\n");
    }else{
        time_t tmp = Utils::mktimeFromToday(0);
        time_t now = Global::tick;
        if ( now < tmp ) {
            mNextFreshFamesListTime = tmp;
        }
        else
        {
            mNextFreshFamesListTime = tmp + 3600 * 24;
        }
    }

    doRedisCmd("hset serverdata nextfreshfameslisttime %d", mNextFreshFamesListTime);
}

void FameHallMgr::setNextFreshRankTime()
{
//    mNextFreshRankTime += 180;
//    return;
    if (Process::env.getInt("testmode") == 1) {
        mNextFreshRankTime = Global::tick - Global::tick%300 + 300;
    }else{
        time_t tmp = Utils::mktimeTheNextOnhour();
        mNextFreshRankTime = tmp;
    }

    doRedisCmd("hset serverdata nextfreshfamesranktime %d", mNextFreshRankTime);
}

void FameHallMgr::freshFamesList()
{
    //    map<int, FamesList>::iterator iter;
    //    for (iter = mFamesLists.begin(); iter != mFamesLists.end(); iter++) {
    //        int job = iter->first;
    //        famesReplace((ObjJob)job);
    //    }
    // 修改了名人堂排序规则 by wangck
    famesRefresh();
    saveFameHallList();

    //广播刷新列表
    notify_fresh_fameshall_list notify;
    broadcastPacket(WORLD_GROUP, &notify);
}

int FameHallMgr::getPositionInHall(int roleid, ObjJob job)
{
    map<int, FamesList>::iterator iter = mFamesLists.find((int)job);

    if ( iter == mFamesLists.end()) {
        return-1;
    }

    for (int i = 0; i < iter->second.size(); i++) {
        if (iter->second[i].personId == roleid) {
            return i;
        }
    }
    return -1;
}
bool FameHallMgr::famesReplace(ObjJob job)
{
    map<int, FamesList>::iterator listIter = mFamesLists.find((int)job);
    if (listIter == mFamesLists.end()) {
        return false;
    }

    int fameHallSize = mFamesLists[job].size();
    int hallIndex = 0;
    int rankIndex = 0;

//    if (fameHallSize < 3) {
//        return false;
//    }
//
    map<int, FamesHallRank>::iterator rankIter = mRanks.find((int)job);
    if (rankIter == mRanks.end()) {
        return false;
    }

    int rankSize = mRanks[job].size();

    while (hallIndex < fameHallSize && rankIndex < rankSize) {

        FamesRankData* playerData = mRanks[job][rankIndex];
        if (playerData == NULL) {
            log_error("fameHall freshList:not find playerdata");
            return false;
        }

        int toplayer = playerData->topLayerPassed;

        if (hallIndex < (fameHallSize - toplayer)) {
            hallIndex = fameHallSize - toplayer;
            continue;
        }
        else
        {
            int ret = getPositionInHall(playerData->roleid, (ObjJob)playerData->job);
            if (ret < 0) {

                setFameData(mFamesLists[job][hallIndex], playerData);
                mFamesLists[job][hallIndex].position = hallIndex + 1;

                hallIndex++;
                rankIndex++;
            }
            else if (ret < (fameHallSize - toplayer)) {
                rankIndex++;
                continue;
            }
            else{
                if (ret > (fameHallSize - toplayer)) {
                    mFamesLists[job][ret] = mFamesLists[job][hallIndex];
                    mFamesLists[job][ret].position = ret + 1;
                }

                setFameData(mFamesLists[job][hallIndex], playerData);
                mFamesLists[job][hallIndex].position = hallIndex + 1;

                hallIndex++;
                rankIndex++;
            }
        }
    }
    return true;
}

bool FameHallMgr::famesRefresh()
{
    
    map<int, FamesList>::iterator famelistItr = mFamesLists.begin();
    for (; famelistItr != mFamesLists.end(); ++famelistItr)
    {
        int pos = 0;
        int job = famelistItr->first;
        
        vector<FamesInfo>::iterator infobeginItr = mFamesLists[job].begin();
        vector<FamesInfo>::iterator infoendItr = mFamesLists[job].end();
        for(;infobeginItr != infoendItr; ++infobeginItr)
        {
            if(pos >= mRanks[job].size())
                break;
            FamesInfo info;
            setFameData(info, mRanks[job][pos]);
            
            info.position = pos + 1;
            mFamesLists[job][pos] = info;
            ++pos;
        }
    }
    return true;
}

void FameHallMgr::checkChallDataFresh()
{
    //mFamesVersion += 1;
    int currLayer = 1;
    int topLayer = 0;
    int startLayer = 1;
    PlayerDataMap::iterator itrFind = mPlayerDataMap.begin();
    PlayerDataMap::iterator itrEnd = mPlayerDataMap.end();
    
    int maxStarLayer = FamousDataCfg::mData.famesHallLayers - FamousDataCfg::mData.famesNeedPassLayers + 1;
    
    for (; itrFind != itrEnd; ++itrFind)
    {
        currLayer = itrFind->second["currlayer"].asInt();
        topLayer = itrFind->second["toplayer"].asInt();
        startLayer = itrFind->second["startlayer"].asInt();
        
        check_max(startLayer, maxStarLayer);
        
        int tempLayer = FamousDataCfg::mData.famesHallLayers - (topLayer + FamousDataCfg::mData.famesNextAddLayers);
        if(topLayer == 0)
            continue;
        // 已经挑战过全部
        if(topLayer >= startLayer + FamousDataCfg::mData.famesNeedPassLayers - 1
           && tempLayer >= 0
           && currLayer <= FamousDataCfg::mData.famesHallLayers)
        {
            currLayer = startLayer = startLayer + FamousDataCfg::mData.famesNextAddLayers;
        }
        else
        {
            currLayer = startLayer;
        }
        topLayer = 0;
        itrFind->second["currlayer"] = currLayer;
        //itrFind->second["toplayer"] = topLayer;
        itrFind->second["startlayer"] = startLayer;
        mPlayerDataMap[itrFind->first]["toplayer"] = topLayer;
    }
    
    mPlayerDataMap.save();
    
    mBattleIndex++;
}

void FameHallMgr::setFameData(FamesInfo &fame, FamesRankData *playerdata)
{
    fame.personId = playerdata->roleid;
    fame.name = playerdata->name;
    fame.roleType = playerdata->roleType;
    fame.weaponQua = playerdata->weaponQua;
    fame.lvl = playerdata->lvl;
    fame.enchantId = playerdata->chantid;
    fame.enchantLvl = playerdata->chantLvl;
    fame.awakeLvl = playerdata->awakeLvl;
}

void FameHallMgr::safe_getFamesHallFameList(Role *role)
{
    create_global(getFamesHallList, msg);
    role->getRoleInfo(msg->playerinfo);
    msg->sessionid = role->getSessionId();
    sendGlobalMsg(Global::MQ, msg);
}

void FameHallMgr::safe_getFamesHallPlayerData(Role *role, int versionid)
{
    create_global(getFamesHallPlayerData, msg);
    role->getRoleInfo(msg->playerinfo);
    msg->sessionid = role->getSessionId();
    msg->versionid = versionid;
    sendGlobalMsg(Global::MQ, msg);
}

void FameHallMgr::safe_beginFamesHallBattle(Role *role, int checkLayer, int sceneid)
{
    create_global(beginFamesHallBattle, msg);
    msg->checkLayer = checkLayer;
    role->getRoleInfo(msg->playerinfo);
    msg->sceneid = sceneid;
    sendGlobalMsg(Global::MQ, msg);
}

void FameHallMgr::safe_battleHandle(Role *role, int result, int layer, int sceneid, FamesHallPointFactor factor)
{
    create_global(famesHallBattleHandle, msg);
    role->getRoleInfo(msg->playerinfo);
    msg->result = result;
    msg->layer = layer;
    msg->sceneid = sceneid;
    msg->factor = factor;
    sendGlobalMsg(Global::MQ, msg);
}

void FameHallMgr::onClientGetFamesHallFameList(int roleid, int sessionid)
{
    ack_fameshall_fameslist ack;

    vector<ObjJob> jobs;
    jobs.clear();

    jobs.push_back(eWarrior);
    jobs.push_back(eMage);
    jobs.push_back(eAssassin);

    FamesChallData chalData = getPlayerchallData(roleid);
    
    for (int i = 0; i < jobs.size(); i++) {

        switch(jobs[i])
        {
            case eWarrior:
            {
                //onGetFameInfo(eWarrior, ack.warriorFames);
                onGetFameInfoNew(eWarrior, chalData.startLayer, ack.warriorFames);
                break;
            }
            case eMage:
            {
                //onGetFameInfo(eMage, ack.mageFames);
                onGetFameInfoNew(eMage, chalData.startLayer, ack.mageFames);
                break;
            }
            case eAssassin:
            {
                //onGetFameInfo(eAssassin, ack.AssassinFames);
                onGetFameInfoNew(eAssassin, chalData.startLayer, ack.AssassinFames);
                break;
            }
            default:
                break;
        }
    }
    
    sendNetPacket(sessionid, &ack);
}

int FameHallMgr::onGetFameInfo(ObjJob job, vector<obj_fames_info> &outdata)
{
    map<int, FamesList>::iterator iter = mFamesLists.find((int)job);
    if (iter == mFamesLists.end()) {
        return 1;
    }

    for (int i = 0; i < mFamesLists[job].size(); i++) {
        obj_fames_info info;
        info.name = mFamesLists[job][i].name;
        info.roleType = mFamesLists[job][i].roleType;
        info.lvl = mFamesLists[job][i].lvl;
        info.weaponQua = mFamesLists[job][i].weaponQua;

        outdata.push_back(info);
    }

    return 0;
}

int FameHallMgr::onGetFameInfoNew(ObjJob job, int currentLayer, vector<obj_fames_info>& outdata)
{
    map<int, FamesList>::iterator iter = mFamesLists.find((int)job);
    if (iter == mFamesLists.end()) {
        return 1;
    }
    
    for (int i = 0; i < mFamesLists[job].size(); i++)
    {
        FamousCfgDef* _cfg = FamousCfg::getFamousCfgNew((int)job, currentLayer, mFamesLists[job][i].position);
        if(_cfg == NULL)
            continue;
        obj_fames_info info;
        info.name = mFamesLists[job][i].name;
        info.roleType = mFamesLists[job][i].roleType;
        //info.lvl = mFamesLists[job][i].lvl;
        info.lvl = _cfg->default_lvl;
        info.weaponQua = mFamesLists[job][i].weaponQua;
        info.chantid = mFamesLists[job][i].enchantId;
        info.chantlvl = mFamesLists[job][i].enchantLvl;
        info.awakeLvl = mFamesLists[job][i].awakeLvl;
        outdata.push_back(info);
    }
    
    return 0;
}

int FameHallMgr::onGetPlayerData(int roleid, int sessionid, int versionId)
{
    ack_fameshall_player_data ack;

    FamesChallData data;
    data = getPlayerchallData(roleid);
    
    ack.currLayer = data.currtLayer;
    ack.topLayer = data.topLayer;
    ack.startlayer = data.startLayer;
    ack.famesVersion = this->mFamesVersion;
    
    
    sendNetPacket(sessionid, &ack);
    return 0;
}

FamesChallData FameHallMgr::getPlayerchallData(int roleid)
{
    FamesChallData data;
    data.topLayer = 0;
    data.currtLayer = 1;
    data.startLayer = 1;
    data.currPoints = 0;
    if (mPlayerDataMap.exist(roleid)) {
        try {
            
            int maxStarLayer = FamousDataCfg::mData.famesHallLayers - FamousDataCfg::mData.famesNeedPassLayers + 1;
            
            data.topLayer = mPlayerDataMap[roleid]["toplayer"].asInt();
            data.currtLayer = mPlayerDataMap[roleid]["currlayer"].asInt();
            data.currPoints = mPlayerDataMap[roleid]["currpoints"].asInt();
            data.startLayer = mPlayerDataMap[roleid]["startlayer"].asInt();
            
            if(data.currtLayer == 0 || data.startLayer == 0 || data.currtLayer < data.topLayer)
            {
                data.currtLayer = 1;
                data.startLayer = 1;
                data.topLayer = 0;
                commitPlayerChallData(roleid, data);
            }
            
            if (data.startLayer > maxStarLayer) {
                int diffLayer = data.startLayer - maxStarLayer;
                data.currtLayer = data.currtLayer - diffLayer;
                data.startLayer = maxStarLayer;
                
                if (data.topLayer > 0) {
                    data.topLayer = data.topLayer - diffLayer;
                    check_min(data.topLayer, 0);
                }
                
                commitPlayerChallData(roleid, data);
            }
        } catch (...) {

        }
    }
    return data;
}

bool FameHallMgr::commitPlayerChallData(int roleid, FamesChallData newdata)
{
    try {
        mPlayerDataMap[roleid]["toplayer"] = newdata.topLayer;
        mPlayerDataMap[roleid]["currlayer"] = newdata.currtLayer;
        mPlayerDataMap[roleid]["currpoints"] = newdata.currPoints;
        mPlayerDataMap[roleid]["startlayer"] = newdata.startLayer;
        mPlayerDataMap.save();
    } catch (...) {
        return false;
    }
    return true;
}

bool FameHallMgr::updatePlayerPointsInRanking(int roleid, ObjJob job, int points)
{
    string jobname = RoleTpltCfg::objJobToString(job);
    if (jobname.empty()) {
        return false;
    }
    string tmp = "paihang:fame_";
    tmp.append(jobname);

    doRedisCmd("zadd %s %d %d", tmp.c_str(), points, roleid);
    return true;
}

bool FameHallMgr::CheckActiveOpen()
{
    return SActMgr.IsActive(mActiveId);
}

bool FameHallMgr::saveFameHallList()
{
    string famesList = "";
    map<int, FamesList>::iterator iter;
    for (iter = mFamesLists.begin(); iter != mFamesLists.end(); iter++) {
        stringstream list;
        for (int i = 0; i < iter->second.size(); i++) {
            int fameid = iter->second[i].personId;
            if (i) {
                list<<",";
            }
            list<<fameid;
        }
        list<<";";
        famesList.append(list.str());
    }

    return onSaveFameHallList(famesList);
}

bool FameHallMgr::onSaveFameHallList(string list)
{
    if (list.empty()) {
        return false;
    }

    doRedisCmd("hset serverdata famehall %s", list.c_str());
    return true;
}

int FameHallMgr::onBeginBattle(int roleid, int checkLayer)
{
    //暂时不限制时间
//    if (!CheckActiveOpen()) {
//        return CE_ACTIVE_IS_NOT_OPEN;
//    }

    FamesChallData realdata;
    realdata = getPlayerchallData(roleid);
    if(realdata.topLayer && checkLayer > realdata.startLayer + FamousDataCfg::mData.famesNeedPassLayers - 1)
        return CE_FAMEHALL_THIS_CHECKLAYER_IS_NOT_RIGHT;
    if (checkLayer > FamousDataCfg::mData.famesHallLayers ) {
        return CE_FAMEHALL_YOU_HAVE_PASS_ALLLAYERS;
    }
    if (checkLayer < realdata.currtLayer) {
        return CE_FAMEHALL_YOU_HAVE_PASS_THIS_LAYER;
    }

    if (checkLayer > (realdata.currtLayer + 1)) {
        return CE_FAMEHALL_THIS_CHECKLAYER_IS_NOT_RIGHT;
    }

    setPlayerBattleIndex(roleid);
    
    return CE_OK;

}

int FameHallMgr::onBattleHandle(int roleid, ObjJob job, int result, int layer, FamesHallPointFactor factor)
{
    if (result == eDungeNext || result == eDungeFinish) {
        
        //第二天0点之后完成的不算积分什么的，add by wangzhigang 2015-3-17
        int playerBattleIndex = getPlayerBattleIndex(roleid);
        if (playerBattleIndex != mBattleIndex) {
            return CE_OK;
        }
        
        FamesChallData lastInfo;
        int endLayer = 0;
        //处理即时数据
        if (mPlayerDataMap.exist(roleid)) {
            
            lastInfo = getPlayerchallData(roleid);
            // 开始层不可能为0
            if(lastInfo.startLayer == 0)
            {
                return CE_UNKNOWN;
            }
            
            endLayer = lastInfo.startLayer + FamousDataCfg::mData.famesNeedPassLayers - 1;
            
            // 结束层不应该小于等于第一层
            if(endLayer <= lastInfo.startLayer)
            {
                return CE_FAMEHALL_YOU_HAVE_PASS_ALLLAYERS;
            }
            // 挑战完当天的最大层
            if(lastInfo.topLayer >= endLayer)
            {
                return CE_FAMEHALL_YOU_HAVE_PASS_ALLLAYERS;
            }
            
            //该层及低级层已通关
            if (layer < lastInfo.currtLayer) {
                return CE_FAMEHALL_YOU_HAVE_PASS_THIS_LAYER;
            }
            // 挑战前做判断，挑战后不做判断
            if (layer > (lastInfo.currtLayer + 1)) {
                //return CE_FAMEHALL_THIS_CHECKLAYER_IS_NOT_RIGHT;
            }
            
            if (layer > (lastInfo.currtLayer + 1)) {
                //return CE_UNKNOWN;
            }
            
            lastInfo.currtLayer = layer + 1;
            
            if (lastInfo.topLayer < layer) {
                lastInfo.topLayer = layer;
            }
            
        }else
        {
            if (lastInfo.currtLayer < layer) {
                lastInfo.currtLayer = layer + 1;
            }
            
            if (lastInfo.topLayer < layer) {
                lastInfo.topLayer = layer;
            }
            
            // 必然是从第一层开始
            lastInfo.startLayer = 1;
        }
        
        //处理排行榜
        //        int getPoints = layer * 10000000 + factor.roleBattleForce;
        //        if (getPoints > lastInfo.currPoints) {
        //
        //            lastInfo.currPoints = getPoints;
        //            updatePlayerPointsInRanking(roleid, job, getPoints);
        //        }
       
       int maxLayer = FamousDataCfg::mData.famesHallLayers;
       
        // 新的得分处理，挑战完当天的所有层数才可以获得积分
        if(lastInfo.topLayer >= endLayer || maxLayer <= lastInfo.topLayer)
        {
            int getPoints = FamousDataCfg::mData.famesNeedPassLayers * 10000000 + factor.roleBattleForce;
            lastInfo.currPoints = getPoints;
            updatePlayerPointsInRanking(roleid, job, getPoints);
        }
        commitPlayerChallData(roleid, lastInfo);
    }
    return CE_OK;
}


void FameHallMgr::onGetRankData(ObjJob job, int beginRank, int endRank, vector<FamesRankData>& outData)
{
    check_min(beginRank, 1);
    map<int, FamesHallRank>::iterator rankIter = mRanks.find((int)job);
    if (rankIter == mRanks.end()) {
        return;
    }

    FamesHallRank& session = mRanks[job];

    for (int i = beginRank - 1; i < session.size() && i < endRank; i++) {
        outData.push_back(*session[i]);
    }
}

void FameHallMgr::testRank()
{
    return;
    bool testMode = Process::env.getInt("testmode") == 1;
    if (testMode) {

        vector<FamesRankData*> data;

        vector<ObjJob> jobs;
        jobs.push_back(eWarrior);
        jobs.push_back(eMage);
        jobs.push_back(eAssassin);
        for (int job = 0; job < 3; job++) {
            vector<FamesRankData> data;
            onGetRankData(jobs[job], 1 , 10, data);
            stringstream content;

            string jobName = RoleTpltCfg::objJobToString(jobs[job]);
            content<<jobName<<":"<<"\n";
            for (int i = 0; i < data.size(); i++) {
                content<<data[i].name<<",分数"<<data[i].points<<"通关层数:"<<data[i].topLayerPassed<<"\n";
            }
            notify_worldchat noty;
            noty.senderId = 0;
            noty.senderSex = 0;
            noty.senderName = "名人堂";
            noty.message = content.str();

            broadcastPacket(WORLD_GROUP, &noty);
        }
    }
}

void FameHallMgr::testMyPoints(obj_roleinfo roleinfo, int sessionid)
{
    return;
    bool testMode = Process::env.getInt("testmode") == 1;
    if (testMode) {

        FamesChallData mydata = getPlayerchallData(roleinfo.roleid);
        stringstream content;
        content<<"名人堂分数:";

        content<<mydata.currPoints<<"通关层："<<mydata.currtLayer;
        notify_privatechat noty;
        noty.senderId = roleinfo.roleid;
        noty.senderSex = roleinfo.roletype;
        noty.senderName = roleinfo.rolename;
        noty.message = content.str();

        sendNetPacket(sessionid, &noty);
    }
}

void FameHallMgr::setPlayerBattleIndex(int roleid)
{
    map<int, int>::iterator iter = mPlayerBattleIndex.find(roleid);
    
    if (iter == mPlayerBattleIndex.end()) {
        mPlayerBattleIndex.insert(make_pair(roleid, mBattleIndex));
        return;
    }
    
    iter->second = mBattleIndex;
}

int FameHallMgr::getPlayerBattleIndex(int roleid)
{
    map<int, int>::iterator iter = mPlayerBattleIndex.find(roleid);
    if (iter == mPlayerBattleIndex.end()) {
        return 0;
    }
    
    return iter->second;
}

void getFamesHallData(GETFAMESHALLDATAFUNC cbfunc)
{
    create_global(GetFamesHallData, msg);
    msg->cb = (void*)cbfunc;
    sendGlobalMsg(Global::MQ, msg);
}
