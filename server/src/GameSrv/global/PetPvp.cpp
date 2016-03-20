
//
//  PetPvp.cpp
//  GameSrv
//
//  Created by pireszhi on 13-9-27.
//
//

#include "PetPvp.h"
#include "main.h"
#include "DBRolePropName.h"
#include "Global.h"
#include "Game.h"
#include <time.h>
#include "EnumDef.h"
#include "GameLog.h"
using namespace std;

INSTANTIATE_SINGLETON(PetPvpMgr);

void getIntegersOutoforder(std::vector<int>& integers)
{
    std::vector<int> tmp = integers;
    int count = tmp.size();
    int cbCount = 0;
    int cbPosition = 0;
    
    for (int i = 0; i < count; i++) {
        int r = count - cbCount;
        cbPosition = rand()%r;
        
        integers[i] = tmp[cbPosition];
        tmp[cbPosition] = tmp[r - 1];
        
        cbCount++;
    }
}

void PetPvpMgr::load()
{
    RedisResult result(redisCmd("hmget serverdata petpvpversion petpvp_distrib"));
    mVersion = result.readHash(0, 0);
    
    if (mVersion < static_PetPvpVersion) {
        //版本更新，重设数据结构
        resetDataWithVersionChange();
        mVersion = static_PetPvpVersion;
        
        doRedisCmd("hset serverdata petpvpversion %d", mVersion);
    }
    else
    {
        loadDataInDB();
    }
    
    int nextTime = result.readHash(1, 0);
    if (!nextTime) {
        setDistribTime();
    }
    else
    {
        mDistribTime = nextTime;
    }
}

void PetPvpMgr::loadDataInDB()
{
    //加载排行榜
    mGlobalRankData.clear();
    
    for (int type = 3; type >= 1; type--) {
        
        PetPvpType pvptype = (PetPvpType)type;
        string typeName = petPvpType2String(pvptype);
        
        redisReply* rreply = redisCmd("zrevrange paihang:petpvp_%s 0 -1 withscores", typeName.c_str());
        if (rreply->type == REDIS_REPLY_ARRAY) {
            for (int j = 0; j < rreply->elements; j += 2) {
                int roleid = Utils::safe_atoi(rreply->element[j]->str);
                int val = Utils::safe_atoi(rreply->element[j+1]->str);
                
                PetPvpData* data = loadPlayerData(roleid);
                
                if (data == NULL) {
                    doRedisCmd("zrem paihang:petpvp_%s %d", typeName.c_str(), roleid);
                    continue;
                }
                
                data->mCups = val;
                
                addPlayerIntoSection(pvptype, data);
                
                mPlayersData.insert(make_pair(roleid, data));
                
                mGlobalRankData.push_back(data);
                data->mGlobalRank = mGlobalRankData.size();
                reSort(eGlobal, data->mGlobalRank, eForward);
            }
        }
        freeReplyObject(rreply);
    }
}

PetPvpData* PetPvpMgr::loadPlayerData(int roleid)
{
    const char* props[] =
    {
        GetRolePropName(eRolePetPvpFormSize),   //阵型大小，不必要
        GetRolePropName(eRolePetPvpForm),       //
        GetRolePropName(eRolePetPvpHistoryRank),//
        GetRolePropName(eRolePropLvl),          //
        GetRolePropName(eRolePropRoleName),     //
        GetRolePropName(eRolePropRoleType),     //
        GetRolePropName(eRolePetPvpWinTimes),   //
        GetRolePropName(eRolePetPvploseTimes),  //
        GetRolePropName(eRolePetPvpPeridAwardRank), //
        GetRolePropName(eRolePetPvpPoints),     //
        GetRolePropName(eRolePetPvpVersion),
        GetRolePropName(eRolePetPvpCups),
        NULL,
    };
    string allprops = StrJoin(props, &props[sizeof(props) / sizeof(const char*) - 1], " ");
    string cmd = "hmget role:";
    cmd.append(Utils::itoa(roleid));
    cmd.append(" ");
    cmd.append(allprops.c_str());
    
    RedisResult result(redisCmd(cmd.c_str()));
    
    int formsize = result.readHash(0, 0);
    string petform = result.readHash(1, "");
    string historyRank = result.readHash(2, "");
    int rolelvl = result.readHash(3, 0);
    string rolename = result.readHash(4, "");
    int roleType = result.readHash(5, 0);
    
    int wintimes = result.readHash(6, 0);
    int losetimes = result.readHash(7, 0);
    int awardRank = result.readHash(8, 0);
    int points = result.readHash(9, 0);
    int version = result.readHash(10, 0);
    int cups = result.readHash(11, 0);
    
    if (rolename.empty()) {
//        doRedisCmd("zrem paihang:petpvp_%s %d", typeName.c_str(), roleid);
        return NULL;
    }
    
    PetPvpData* data = new PetPvpData(roleid);
    
    if (data == NULL) {
        return NULL;
    }
    
    data->mRolename = rolename;
    data->mRoletype = roleType;
    data->mLvl = rolelvl;
    data->mWinTimes = wintimes;
    data->mLostTimes = losetimes;
    data->mAwardRank = awardRank;
    data->mPoints = points;
    data->mVersion = version;
    data->mCups = cups;
    
    //新的版本，把个人数据重置
    if (data->mVersion < static_PetPvpVersion) {
        
        //历史最高排名清掉
        
        data->mCups = 0;
        data->mWinTimes = 0;
        data->mLostTimes = 0;
        data->mPvpType = eBronze;
        data->resetHistory();
        data->mVersion = static_PetPvpVersion;
        
//        historyRank = "";
//        data->mWinTimes = 0;
//        data->mLostTimes = 0;
//        data->mVersion = static_PetPvpVersion;
    }
    
    int petFormSize = PetPvpCfg::getPetFormSize(data->mLvl);
    
    //加载宠物信息
    StringTokenizer pets(petform, ";");
    for (int i = 0; i < pets.count() && i < petFormSize; i++) {
        int petid = Utils::safe_atoi(pets[i].c_str());
        RedisResult petResult(redisCmd("exists pet:%d", petid));
        
        if (petid && petResult.readInt() == 0) {
            continue;
        }
        
        bool pushBack = true;
        //把有重复的去掉
        for (int j = 0; j < data->mPetsForPvp.mPets.size(); j++) {
            if (petid == data->mPetsForPvp.mPets[j]) {
                pushBack = false;
                break;
            }
        }
        if (pushBack == false) {
            continue;
        }
        data->mPetsForPvp.mPets.push_back(petid);
    }
    
    data->formationResize(petFormSize);
    
    if (data->mPetsForPvp.mPets.size() != pets.count()) {
        saveFormation(roleid, data->mPetsForPvp.mPets);
    }
    
    //加载历史最高排名
    StringTokenizer history(historyRank, ";");
    for (int i = 0; i < history.count() && i < data->mHistoryRank.size(); i++) {
        data->mHistoryRank[i] = Utils::safe_atoi(history[i].c_str());
    }
    
    return data;
}

PetPvpData* PetPvpMgr::onUpdatePlayerData(int roleid, bool iswin, int givePoints, int& getCups, int& enemyid, int& enemyGetPoints, int& enemySurplusPoints)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter == mPlayersData.end()) {
        return NULL;
    }
    
    PetPvpType pvptype = (PetPvpType)iter->second->mPvpType;
    PetPvpSectionCfg* cfg = PetPvpCfg::getPetPvpSectionCfg(pvptype);
    if (cfg == NULL) {
        return NULL;
    }
    
    if(iter->second->mEnemy == NULL)
    {
        return NULL;
    }
    
    getCups = 0;
    
    int preTotalCups = iter->second->mCups;
    
    if (iswin) {
        
        int difRank = iter->second->mGlobalRank - iter->second->mEnemy->mGlobalRank;
        
        check_min(difRank, 0);
        
        if (pvptype == iter->second->mEnemy->mPvpType) {
            
            getCups = cfg->mFixAwardCups;
            getCups += difRank;
        }
        else if (pvptype > iter->second->mEnemy->mPvpType)
        {
            getCups = cfg->mWinLowerCups + difRank;
        }
        else if (pvptype < iter->second->mEnemy->mPvpType)
        {
            getCups = cfg->mWinHigherCups + difRank;
        }
        
        
        iter->second->mWinTimes++;
        iter->second->mCups += getCups;
        iter->second->mPoints += givePoints;
        
        reSort(pvptype, iter->second->mRank, eForward);
        reSort(eGlobal, iter->second->mGlobalRank, eForward);
    }
    else
    {
        
        if (pvptype == iter->second->mEnemy->mPvpType) {
            getCups = cfg->mFixAwardCups;
            int difRank =  iter->second->mEnemy->mGlobalRank - iter->second->mGlobalRank;
            
            check_min(difRank, 0);
            getCups += difRank;
            
            getCups = -getCups;
        }
        else if (pvptype > iter->second->mEnemy->mPvpType)
        {
            getCups = -cfg->mLostCups;
        }
        else if (pvptype < iter->second->mEnemy->mPvpType)
        {
            getCups = -cfg->mLostCups;
        }
        
        iter->second->mLostTimes++;
        iter->second->mCups += getCups;
        iter->second->mPoints += givePoints;
        
        enemyGetPoints = PetPvpCfg::mCfgData.mEnemyGetPoints;
        if (enemyGetPoints) {
            
            iter->second->mEnemy->mPoints += enemyGetPoints;
            enemySurplusPoints = iter->second->mEnemy->mPoints;
            enemyid = iter->second->mEnemy->mRoleid;
            doRedisCmd("hset role:%d %s %d", iter->second->mEnemy->mRoleid, GetRolePropName(eRolePetPvpPoints), iter->second->mEnemy->mPoints);
        }

        check_min(iter->second->mCups, 0);
        reSort(pvptype, iter->second->mRank, eBackward);
        reSort(eGlobal, iter->second->mGlobalRank, eBackward);
    }
    string pvptype2string = petPvpType2String(pvptype);
    
    doRedisCmd("zadd paihang:petpvp_%s %d %d", pvptype2string.c_str(), iter->second->mCups, iter->second->mRoleid);
    
    savePlayerData(iter->second);
    
    iter->second->mEnemy->unLock();
    iter->second->mEnemy = NULL;
    
    if (getCups != 0) {
        
        string from = iswin ? "win" : "lose";
        
        LogMod::addLogPetPvpCupsChange(iter->second->mRoleid, iter->second->mRolename.c_str(), preTotalCups, getCups, iter->second->mCups, pvptype2string.c_str(), from.c_str());
    }
    
    return iter->second;
}

void PetPvpMgr::savePlayerData(PetPvpData *playerdata)
{
    RedisArgs args;
    
    string key = Utils::makeStr("role:%d", playerdata->mRoleid);
    
    RedisHelper::beginHmset(args, key.c_str());
    
    RedisHelper::appendHmset(args, GetRolePropName(eRolePetPvpForm), playerdata->mPetsForPvp.tostring().c_str());

    RedisHelper::appendHmset(args, GetRolePropName(eRolePetPvpCups), playerdata->mCups);
    RedisHelper::appendHmset(args, GetRolePropName(eRolePetPvpPoints), playerdata->mPoints);
    RedisHelper::appendHmset(args, GetRolePropName(eRolePetPvpHistoryRank), playerdata->historyRankToString().c_str());
    
    RedisHelper::appendHmset(args, GetRolePropName(eRolePetPvpWinTimes), playerdata->mWinTimes);
    RedisHelper::appendHmset(args, GetRolePropName(eRolePetPvploseTimes), playerdata->mLostTimes);
    RedisHelper::appendHmset(args, GetRolePropName(eRolePetPvpPeridAwardRank), playerdata->mAwardRank);
    RedisHelper::appendHmset(args, GetRolePropName(eRolePetPvpVersion), playerdata->mVersion);
    
    //保存历史战绩
    if(playerdata->updateHistoryRank(playerdata->mPvpType, playerdata->mRank))
    {
        std::ostringstream historyStr;
        for (int i = 0; i < playerdata->mHistoryRank.size(); i++) {
            historyStr<<playerdata->mHistoryRank[i];
            historyStr<<";";
        }
        
        RedisHelper::appendHmset(args, GetRolePropName(eRolePetPvpHistoryRank), historyStr.str().c_str());
    }
    
    RedisHelper::commitHmset(get_DbContext(), args);
}

bool PetPvpMgr::reSort(PetPvpType pvptype, int beginIndex, ReSortType sortType)
{
    if (pvptype == eGlobal) {
        
        if (beginIndex > mGlobalRankData.size() || beginIndex < 1) {
            return false;
        }
        
        PetPvpData* currentData = mGlobalRankData[beginIndex - 1];
        if(sortType == eForward)
        {
            for (int i = beginIndex - 2; i >= 0; i--) {
                
                if (mGlobalRankData[i]->mCups < currentData->mCups) {
                    mGlobalRankData[i]->mGlobalRank++;
                    currentData->mGlobalRank--;
                    mGlobalRankData[i+1] = mGlobalRankData[i];
                    mGlobalRankData[i] = currentData; 
                }
                else
                {
                    break;
                }
            }
        }
        else if(sortType == eBackward)
        {
            for (int i = beginIndex; i < mGlobalRankData.size(); i++) {
                
                if (mGlobalRankData[i]->mCups > currentData->mCups) {
                    mGlobalRankData[i]->mGlobalRank--;
                    currentData->mGlobalRank++;
                    mGlobalRankData[i-1] = mGlobalRankData[i];
                    mGlobalRankData[i] = currentData;
                }
                else
                {
                    break;
                }
            }
            
        }
    }
    else
    {
    
        std::map<int, PetPvpSectionData>::iterator iter = mPvpDatas.find((int)pvptype);
        if (iter == mPvpDatas.end()) {
            return false;
        }
        
        PetPvpSectionData &section = iter->second;
        
        if (beginIndex > section.size() || beginIndex < 1) {
            return false;
        }
        PetPvpData* currentData = section[beginIndex - 1];
        
        if(sortType == eForward)
        {
            for (int i = beginIndex - 2; i >= 0; i--) {
                
                if (section[i]->mCups < currentData->mCups) {
                    section[i]->mRank++;
                    currentData->mRank--;
                    section[i+1] = section[i];
                    section[i] = currentData;
                }
                else
                {
                    break;
                }
            }
        }
        else if(sortType == eBackward)
        {
            for (int i = beginIndex; i < section.size(); i++) {
                
                if (section[i]->mCups > currentData->mCups) {
                    section[i]->mRank--;
                    currentData->mRank++;
                    section[i-1] = section[i];
                    section[i] = currentData;
                }
                else
                {
                    break;
                }
            }
            
        }
    }
    return true;
}

//bool PetPvpMgr::onPetPvpCheckIn(int roleid, int lvl)
//{
//    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
//    if (iter != mPlayersData.end()) {
//        return true;
//    }
//    
//    PetPvpType type = PetPvpCfg::level2PetPvpType(lvl);
//    if (type == eUnknown) {
//        return false;
//    }
//    
//    //读取阵型信息
//    RedisResult result(redisCmd("hget role:%d %s", roleid, GetRolePropName(eRolePetPvpForm)));
//    string petform = result.readStr("");
//    
//    PetPvpSectionCfg* section = PetPvpCfg::getPetPvpSectionCfg(type);
//    if (section == NULL) {
//        return false;
//    }
//    
//    int formationsize = section->mFormationSize;  //要读配置表
//    PetList petlist;
//    petlist.clear();
//    
//    StringTokenizer pets(petform, ";");
//
//    for (int i = 0; i < pets.count() && i < formationsize; i++) {
//        petlist.push_back(Utils::safe_atoi(pets[i].c_str()));
//    }
//    
//    for (int i = pets.count(); i < formationsize; i++) {
//        petlist.push_back(0);
//    }
//    
//    return onAddPlayer(roleid, type, petlist);
//}

void PetPvpMgr::pushBackRole(int roleid)
{
    std::set<int>::iterator iter = mRolesInPetPvp.find(roleid);
    if (iter != mRolesInPetPvp.end()) {
        return;
    }
    else{
        mRolesInPetPvp.insert(roleid);
    }
}

bool PetPvpMgr::isPlayerInPetPvp(int roleid)
{
    std::set<int>::iterator iter = mRolesInPetPvp.find(roleid);
    
    return iter != mRolesInPetPvp.end();
}

void PetPvpMgr::eraseRole(int roleid)
{
    std::set<int>::iterator iter = mRolesInPetPvp.find(roleid);
    
    if (iter != mRolesInPetPvp.end()) {
        mRolesInPetPvp.erase(iter);
        return;
    }
}

void PetPvpMgr::safe_getPlayerData(Role *role)
{
    if (role == NULL) {
        return;
    }
    create_global(getPetPvpData, msg);
    role->getRoleInfo(msg->playerinfo);
//    msg->petpvptimes = role->getPetPvpTimes();
//    msg->sessionid = role->getSessionId();
    sendGlobalMsg(Global::MQ, msg);
}

void PetPvpMgr::safe_editFormation(Role *role, std::vector<int> newformation)
{
    for (int i = 0; i< newformation.size(); ++i)
    {
        if(newformation[i] == 0)
            continue;
        
        for (int j = i + 1; j < newformation.size(); ++j)
        {
            if( newformation[i] == newformation[j] )
            {
                ack_edit_petpvp_formation ack;
                ack.errorcode = CE_PVP_PET_ERROR;
                sendNetPacket(role->getSessionId(),&ack);
                return;
            }
        }
    }
    
    if (role == NULL) {
        return;
    }
    create_global(editPetPvpFormation, msg);
    role->getRoleInfo(msg->playerinfo);
    msg->newformation = newformation;
    msg->sessionid = role->getSessionId();
    msg->noAck = false;
    sendGlobalMsg(Global::MQ, msg);
}

void PetPvpMgr::safe_matchEnemy(Role *role, bool isFirstSearch)
{
    if (role == NULL) {
        return;
    }
    if (role->mPetPvpMatching) {
        return;
    }
    
    create_global(getPetPvpEnemy, msg);
    role->getRoleInfo(msg->playerinfo);
    msg->firstSearch = isFirstSearch;
    msg->gold = role->getGold();
    sendGlobalMsg(Global::MQ, msg);
    
}

void PetPvpMgr::safe_beginBattle(Role *role, int enemyid, int sceneid)
{
    if (role == NULL) {
        return;
    }
    create_global(beginPetPvpBattle, msg);
    msg->roleid = role->getInstID();
    msg->enemyid = enemyid;
    msg->sceneid = sceneid;
    sendGlobalMsg(Global::MQ, msg);
}

//void PetPvpMgr::safe_checkUpgrade(Role *role)
//{
//    if (role == NULL) {
//        return;
//    }
//    int openlvl = PetPvpCfg::mCfgData.mOpenLvl;
//    
//    if (openlvl > role->getLvl()) {
//        return;
//    }
//    
//    create_global(checkPetPvpLvlUpGrade, msg);
//    role->getRoleInfo(msg->playerinfo);
//    sendGlobalMsg(Global::MQ, msg);
//    
//}

void PetPvpMgr::safe_delPlayer(Role *role)
{
    if (role == NULL) {
        return;
    }
    create_global(delPlayerInPetPvp, msg);
    msg->roleid = role->getInstID();
    sendGlobalMsg(Global::MQ, msg);
    
}

void PetPvpMgr::safe_UpdatePlayerData(Role* role, bool iswin)
{
    if (role == NULL) {
        return;
    }
    create_global(petPvpBattleHandle, msg);
    role->getRoleInfo(msg->playerinfo);
    msg->iswin = iswin;
    msg->pvptimes = role->getPetPvpTimes();
    msg->sessionid = role->getSessionId();
    sendGlobalMsg(Global::MQ, msg);
}

void PetPvpMgr::safe_gmAddPetPvpPoints(int roleid, int addpoints)
{
    create_global(gmAddPetPvpPoints, msg);
    msg->roleid = roleid;
    msg->addpoints = addpoints;
    sendGlobalMsg(Global::MQ, msg);
}

void PetPvpMgr::safe_gmSetPetPvpPoints(int roleid, int setpoints)
{
    create_global(gmSetPetPvpPoints, msg);
    msg->roleid = roleid;
    msg->setpoints = setpoints;
    sendGlobalMsg(Global::MQ, msg);
}

void PetPvpMgr::safe_gmSetPetPvpCups(int roleid, int setcups)
{
    create_global(gmSetPetPvpCups, msg);
    msg->roleid = roleid;
    msg->setCups = setcups;
    sendGlobalMsg(Global::MQ, msg);
}

PetPvpData* PetPvpMgr::onGetPlayerData(int roleid)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter == mPlayersData.end()) {
        
        PetPvpData* newPlayer = loadPlayerData(roleid);
        
        if (newPlayer) {
            
            onAddNewPlayer(newPlayer);
            
            savePlayerData(newPlayer);
            
//            addPlayerIntoSection(eBronze, newPlayer);
//            
//            mPlayersData.insert(make_pair(roleid, newPlayer));
//            
//            mGlobalRankData.push_back(newPlayer);
//            
//            savePlayerData(newPlayer);
//            
//            string pvpTypeName = petPvpType2String(eBronze);
//            
//            //加到数据库排行榜
//            doRedisCmd("zadd paihang:petpvp_%s %d %d", pvpTypeName.c_str(), newPlayer->mCups, newPlayer->mRoleid);
        }
        
        return newPlayer;
    }
    
    return iter->second;
}

//
//void PetPvpMgr::onPlayerAdvance(int roleid, int lvl)
//{
//    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
//    if (iter == mPlayersData.end()) {
//        return;
//    }
//    
//    PetPvpType oldType = (PetPvpType)iter->second->mPvpType;
//    PetPvpType newType = PetPvpCfg::level2PetPvpType(lvl);
//    
//    if(newType == eUnknown || oldType == newType){
//        return;
//    }
//    else{
//        
//        std::map<int, PetPvpSectionData>::iterator olditer = mPvpDatas.find(iter->second->mPvpType);
//        if (olditer == mPvpDatas.end()) {
//            return;
//        }
//        
//        //在旧的段中删除
//        int oldIndex = iter->second->mRank - 1;
//        olditer->second.erase(olditer->second.begin() + oldIndex);
//        //删除后重新排列
//        for (int i = oldIndex; i < olditer->second.size(); i++) {
//            olditer->second[i]->mRank--;
//        }
//        
//        //在新的段中加入
//        std::map<int, PetPvpSectionData>::iterator newiter = mPvpDatas.find(newType);
//        
//        //没有这个段时，创建这个段
//        if (newiter == mPvpDatas.end()) {
//            
//            PetPvpSectionData newSection;
//            std::pair<std::map<int, PetPvpSectionData>::iterator, bool> ret;
//            ret = mPvpDatas.insert(make_pair(newType, newSection));
//            
//            if (!ret.second) {
//                return;
//            }
//            else{
//                newiter = ret.first;
//            }
//        }
//        
//        //重新设定阵型大小
//        PetPvpSectionCfg* newsection = PetPvpCfg::getPetPvpSectionCfg(newType);
//        if (newsection != NULL) {
//            iter->second->formationResize(newsection->mFormationSize);
//            PetList petlist;
//            iter->second->getFormation(petlist);
//            saveFormation(roleid, petlist); //保存阵型
//        }
//        
//        //放到新的段去
//        newiter->second.push_back(iter->second);
//        iter->second->mRank = newiter->second.size();
//        reSort(newType, newiter->second.size() - 1, eForward);
//        
//        iter->second->mPvpType = newType;
//        iter->second->mLvl = lvl;
//        
//        string pvpTypeName = petPvpType2String(newType);
//        string pvpTypeOldName = petPvpType2String(oldType);
//        
//        //在数据库增删
//        doRedisCmd("zrem paihang:petpvp_%s %d", pvpTypeOldName.c_str(),roleid);
//        doRedisCmd("zadd paihang:petpvp_%s %d %d", pvpTypeName.c_str(), iter->second->mCups, iter->second->mRoleid);
//        
//        //保存历史战绩
//        if(iter->second->updateHistoryRank(newType, iter->second->mRank))
//        {
//            std::ostringstream historyStr;
//            for (int i = 0; i < iter->second->mHistoryRank.size(); i++) {
//                historyStr<<iter->second->mHistoryRank[i];
//                historyStr<<";";
//            }
//            doRedisCmd("hset role:%d %s %s", roleid, GetRolePropName(eRolePetPvpHistoryRank), historyStr.str().c_str());
//        }
//    }
//}

void PetPvpMgr::updatePlayerData(int roleid, int rolelvl)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter == mPlayersData.end()) {
        return;
    }
    
    if (iter->second->mLvl == rolelvl) {
        return;
    }
    
    iter->second->mLvl = rolelvl;
    
    //重新设定阵型大小
    int newSize = PetPvpCfg::getPetFormSize(rolelvl);
    
    if (newSize == iter->second->mPetsForPvp.mPets.size()) {
        return;
    }
    
    iter->second->formationResize(newSize);
    PetList petlist;
    iter->second->getFormation(petlist);
    saveFormation(roleid, petlist); //保存阵型
}

//void PetPvpMgr::setPetFormSize(int roleid, int roleLvl)
//{
//    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
//    if (iter == mPlayersData.end()) {
//        return;
//    }
//    
//    //重新设定阵型大小
//    int newSize = PetPvpCfg::getPetFormSize(roleLvl);
//    
//    if (newSize == iter->second->mPetsForPvp.mPets.size()) {
//        return;
//    }
//    
//    iter->second->formationResize(newSize);
//    PetList petlist;
//    iter->second->getFormation(petlist);
//    saveFormation(roleid, petlist); //保存阵型
//
//}

//void PetPvpMgr::onCheckPlayerLvl(int roleid, int lvl)
//{
//    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
//    if (iter == mPlayersData.end()) {
//        return;
//    }
//    
//    PetPvpType oldType = (PetPvpType)iter->second->mPvpType;
//    PetPvpType newType = PetPvpCfg::level2PetPvpType(lvl);
//    
//    if (newType != eUnknown && oldType != newType) {
//        onPlayerAdvance(roleid, lvl);
//    }
//}

void PetPvpMgr::delayDelPalyer(int roleid)
{
    mDelPlayerList.push_back(roleid);
}

bool PetPvpMgr::onDelPlayer(int roleid)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter != mPlayersData.end()) {
        return true;
    }
    
    if (iter->second->isLock()) {
        return false;
    }
    
    int type = iter->second->mPvpType;
    
    std::map<int, PetPvpSectionData>::iterator it = mPvpDatas.find(type);
    if (it == mPvpDatas.end()) {
        return true;
    }
    
    string sectionName = petPvpType2String((PetPvpType)iter->second->mPvpType);
    int sectionDlIndex = iter->second->mRank - 1;
    int globalDlIndex = iter->second->mGlobalRank - 1;
    
    for (int i = sectionDlIndex; i < it->second.size(); i++) {
        it->second[i]->mRank--;
    }
    
    for (int i = globalDlIndex; i < mGlobalRankData.size(); i++) {
        mGlobalRankData[i]->mGlobalRank--;
    }
    
    PetPvpData* delplayer = iter->second;
    delete delplayer;
    
    it->second.erase(it->second.begin() + sectionDlIndex);
    mPlayersData.erase(roleid);
    mGlobalRankData.erase(mGlobalRankData.begin() + globalDlIndex);
    
    //在数据库增删
    doRedisCmd("zrem paihang:petpvp_%s %d", sectionName.c_str(),roleid);
    
    return true;
}

bool PetPvpMgr::onEditFormation(int roleid, int rolelvl, PetList newform)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    
    if (iter == mPlayersData.end()) {
        return false;
    }
    else{

        if (newform.size() > iter->second->mPetsForPvp.mPets.size()) {
            return false;
        }
        
        for (int i = 0; i < newform.size(); i++) {
            if (newform[i] == iter->second->mPetsForPvp.mPets[i]) {
                continue;
            }
            iter->second->mPetsForPvp.mPets[i] = newform[i];
        }
        
        saveFormation(roleid, newform);
    }
    
    return true;
}

bool PetPvpMgr::onAddNewPlayer(PetPvpData* newplayer)
{
    addPlayerIntoSection(eBronze, newplayer);
    
    mPlayersData.insert(make_pair(newplayer->mRoleid, newplayer));
    
    mGlobalRankData.push_back(newplayer);
    
    newplayer->mGlobalRank = mGlobalRankData.size();
    reSort(eGlobal, newplayer->mGlobalRank, eForward);
    
    string pvpTypeName = petPvpType2String(eBronze);
    
    doRedisCmd("zadd paihang:petpvp_%s %d %d", pvpTypeName.c_str(), newplayer->mCups, newplayer->mRoleid);
    
    return true;
}


bool PetPvpMgr::saveFormation(int roleid, PetList formation)
{
    if (!formation.size()) {
        return false;
    }
    std::ostringstream formationStr;
    for (int i = 0; i < formation.size(); i++) {
        formationStr<<formation[i];
        formationStr<<";";
    }
    
    doRedisCmd("hset role:%d %s %s", roleid, GetRolePropName(eRolePetPvpForm), formationStr.str().c_str());
    return true;
}

bool PetPvpMgr::onRemovePet(int roleid, std::vector<int> petlist)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    
    if (iter == mPlayersData.end()) {
        return false;
    }
    
    PetPvpData* data = iter->second;
    PetList list;
    list.clear();
    bool save = false;
    for (int i = 0; i < data->mPetsForPvp.mPets.size(); i++) {
        for (int j = 0; j < petlist.size(); j++) {
            int remPetid = petlist[j];
            if (remPetid == data->mPetsForPvp.mPets[i]) {
                data->mPetsForPvp.mPets[i] = 0;
                data->mPetsForPvp.mPets[i] = 0;
                save = true;
                break;
            }
        }
        list.push_back(data->mPetsForPvp.mPets[i]);
    }

    if (save) {
        saveFormation(roleid, list);
    }
    
    return true;
}

bool PetPvpMgr::initMatchEnemy(PetPvpData *playerdata)
{
    if (playerdata == NULL) {
        return false;
    }
    
    if (playerdata->mEnemy != NULL && playerdata->mEnemy->isLock()) {
        playerdata->mEnemy->unLock();
        playerdata->mEnemy = NULL;
    }
    
    int myRank = playerdata->mGlobalRank - 1;
    int head = myRank - PETPVP_MATCH_RANGE;
    int tail = myRank + PETPVP_MATCH_RANGE;
    
    check_min(head, 0);
    int ranksize = mGlobalRankData.size();
    check_max(tail, ranksize - 1);
    
    std::vector<int> rands;
    for (int i = head; i < myRank; i++) {
        if (i == myRank) {
            continue;
        }
        rands.push_back(i);
    }
    
    for (int i = tail; i > myRank; i--) {
        rands.push_back(i);
    }
    
    getIntegersOutoforder(rands);
    
    playerdata->mEnemyIndexs.clear();
    playerdata->mEnemyIndexs = rands;
    playerdata->mEnemyMatchIndex = 0;
    playerdata->mEnemy = NULL;
    return true;
}

bool PetPvpMgr::expandMatchEnemy(PetPvpData *playerdata)
{
    if (playerdata->mEnemy != NULL && playerdata->mEnemy->isLock()) {
        playerdata->mEnemy->unLock();
        playerdata->mEnemy = NULL;
    }
    
    int nowsize = playerdata->mEnemyIndexs.size();
    
    int myRank = playerdata->mGlobalRank - 1;
    int head = myRank - nowsize - PETPVP_MATCH_RANGE;
    int tail = myRank + nowsize + PETPVP_MATCH_RANGE;
    
    check_min(head, 0);
    int ranksize = mGlobalRankData.size();
    check_max(tail, ranksize - 1);
    
    std::vector<int> rands;
    for (int i = head; i < myRank; i++) {
        if (i == myRank) {
            continue;
        }
        rands.push_back(i);
    }
    
    for (int i = tail; i > myRank; i--) {
        if (i == myRank) {
            continue;
        }
        rands.push_back(i);
    }
    
    if (rands.size() == 0) {
        return false;
    }
    
    getIntegersOutoforder(rands);
    
    playerdata->mEnemyIndexs.clear();
    playerdata->mEnemyIndexs = rands;
    playerdata->mEnemyMatchIndex = 0;
    playerdata->mEnemy = NULL;
    
    return true;
}

PetPvpData* PetPvpMgr::matchEnemy(int roleid)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    
    if (iter == mPlayersData.end()) {
        return NULL;
    }
    
    if (!initMatchEnemy(iter->second))
    {
        return NULL;
    }
    
    PetPvpData* enemyData = NULL;
    int matchtime = 0;
    do
    {
        int index = iter->second->mEnemyMatchIndex;
        
        if (index >= iter->second->mEnemyIndexs.size()) {
            if(!expandMatchEnemy(iter->second))
            {
                return NULL;
            }
            index = iter->second->mEnemyMatchIndex;
        }
        
        iter->second->mEnemyMatchIndex++;
        matchtime++;
        
        int getindex = iter->second->mEnemyIndexs[index];
        
        if (getindex >= mGlobalRankData.size() || mGlobalRankData[getindex] == NULL) {
            continue;
        }
        
        if (mGlobalRankData[getindex]->mRoleid == iter->second->mRoleid || mGlobalRankData[getindex]->isFormationEmpty()) {
            continue;
        }
        
        if (!isPlayerInPetPvp(mGlobalRankData[getindex]->mRoleid) && mGlobalRankData[getindex]->isLock() == false) {
            
            if (mGlobalRankData[getindex]->isFormationEmpty()) {
                continue;
            }
            iter->second->mEnemy = mGlobalRankData[getindex];
            iter->second->mEnemy->lock();
            iter->second->mEnemyid = mGlobalRankData[getindex]->mRoleid;
            enemyData = mGlobalRankData[getindex];
        }
        
    }while(enemyData == NULL && matchtime < 100);
    
    return enemyData;
}

bool PetPvpMgr::onBeginBattle(int playerid, int enemyid)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(playerid);
    if (iter == mPlayersData.end()) {
        return false;
    }
    
    if (iter->second == NULL) {
        return false;
    }
    
    if (iter->second->mEnemy == NULL) {
        return false;
    }
    
    if (iter->second == NULL || iter->second->mEnemy->mRoleid != enemyid) {
        return false;
    }

    pushBackRole(playerid);

    return true;
}

void PetPvpMgr::onHeartBeat()
{
    std::vector<int>::iterator iter = mDelPlayerList.begin();
    for (; iter != mDelPlayerList.end();) {
        
        if (onDelPlayer(*iter)) {
            iter = mDelPlayerList.erase(iter);
        }
        else{
            iter++;
        }
    }
    time_t nowtime = Global::tick;
    if (nowtime >= mDistribTime) {
        distribAward();
        
        //每天重新设置分段
        setPlayerPvpType();
    }
}

void PetPvpMgr::safe_endPlayerSearchEnemy(Role *role)
{
    if (role == NULL) {
        return;
    }
    create_global(endSearchPetPvpEnemy, msg);
    role->getRoleInfo(msg->playerinfo);
    sendGlobalMsg(Global::MQ, msg);
    
}

void PetPvpMgr::safe_removePetsInFormation(int roleid, std::vector<int> petlist)
{
    create_global(removePetFromPetPvpFormation, msg);
    msg->roleid = roleid;
    msg->petlist = petlist;
    sendGlobalMsg(Global::MQ, msg);
}

void PetPvpMgr::safe_getPetPvpFormation(Role* role)
{
    if (role == NULL) {
        return;
    }
    create_global(getPetPvpPetList, msg);
    msg->roleid = role->getInstID();
    msg->sessionid = role->getSessionId();
    sendGlobalMsg(Global::MQ, msg);
}

void PetPvpMgr::safe_buyItemsUsingPoints(Role *role, int itemid, int count)
{
    create_global(buyItemsUsingPetpvpPoints, msg);
    msg->sessionid = role->getSessionId();
    msg->roleid = role->getInstID();
    msg->itemid = itemid;
    msg->count = count;
    sendGlobalMsg(Global::MQ, msg);
}

void PetPvpMgr::onPlayerEndSearchEnemy(int roleid)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter == mPlayersData.end()) {
        return;
    }
    
    if (iter->second->mEnemy) {
        iter->second->mEnemy->unLock();
        iter->second->mEnemy = NULL;
    }
}

void PetPvpMgr::distribAward()
{
    doRedisCmd("del PetpvpAward");
    
    int counter = 0;
    RedisArgs args;
    
    string begin = "PetpvpAward";
    RedisHelper::beginHmset(args, begin.c_str());
    
    for (int i = 0; i < mGlobalRankData.size(); i++) {
        int rank = mGlobalRankData[i]->mRank;
        int pvpType = mGlobalRankData[i]->mPvpType;
        
        string buf = Utils::makeStr("%d:%d", pvpType, rank);
        
        RedisHelper::appendHmset(args, mGlobalRankData[i]->mRoleid, buf.c_str());
        
        counter++;
        if (counter%5000 == 0) {
            RedisHelper::commitHmset(get_DbContext(), args);
            counter = 0;
            args.clear();
            RedisHelper::beginHmset(args, begin.c_str());
        }
    }
    
    if (args.size() > 2) {
        RedisHelper::commitHmset(get_DbContext(), args);
    }
    
    //重新设定下次发奖时间
    setDistribTime();
}

bool PetPvpMgr::onGetWeeklyAward(int roleid, PetPvpData* playerdata, PetPvpAwardData& outdata)
{
    RedisResult result(redisCmd("hget PetpvpAward %d", roleid));
    
    string awardRnak = result.readStr();
    
    StringTokenizer awardinfo(awardRnak, ":");
    
    if (awardinfo.count() < 2) {
        return false;
    }
    
    outdata.mPvpType = Utils::safe_atoi(awardinfo[0].c_str());
    outdata.mRank = Utils::safe_atoi(awardinfo[1].c_str());
    
    if (outdata.mRank) {
        
        doRedisCmd("hdel PetpvpAward %d", roleid);
        PetPvpSectionCfg* sectioncfg = PetPvpCfg::getPetPvpSectionCfg((PetPvpType)outdata.mPvpType);
        
        int awardPoint = PetPvpCfg::getRankAward(outdata.mRank);
        if (sectioncfg) {
            awardPoint *= sectioncfg->mRankAdditionCoef;
            awardPoint += sectioncfg->mfixAwardPoints;
        }
        playerdata->mPoints += awardPoint;

        outdata.mAwardPoint = awardPoint;
        
        if(awardPoint)
        {
            doRedisCmd("hset role:%d %s %d", roleid, GetRolePropName(eRolePetPvpPoints), playerdata->mPoints);
            return true;
        }
    }
    return false;
}

bool PetPvpMgr::isPlayerFormationEmpty(int roleid)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter != mPlayersData.end()) {
        return iter->second->isFormationEmpty();
    }
    return true;
}

void PetPvpMgr::onGetPvpPetList(int roleid, PetList &formation)
{
    formation.clear();
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter != mPlayersData.end()) {
        formation = iter->second->mPetsForPvp.mPets;
    }
}

int PetPvpMgr::onBuyItemsUsingPoints(int roleid, int itemid, int count , int& surplus)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter != mPlayersData.end()) {
        int consumpoint = PetPvpRedemptionCfg::consumPoints(itemid);
        if (consumpoint < 0) {
            return -1;
        }
        
        int needPoints = consumpoint * count;
        if (iter->second->mPoints < needPoints) {
            return 0;
        }
        
        iter->second->mPoints -= needPoints;
        doRedisCmd("hset role:%d %s %d", roleid, GetRolePropName(eRolePetPvpPoints), iter->second->mPoints);
        
        surplus = iter->second->mPoints;
        return needPoints;
    }
    return 0;
}

void PetPvpMgr::addPetPvpPoints(int roleid, int addpoints)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter == mPlayersData.end()) {
        return;
    }
    
    iter->second->mPoints += addpoints;
    doRedisCmd("hset role:%d %s %d", roleid, GetRolePropName(eRolePetPvpPoints), iter->second->mPoints);
    
    create_global(onGmAddPetPvpPoints, msg);
    msg->roleid = roleid;
    msg->surplusPoints = iter->second->mPoints;
    msg->getPoints = addpoints;
    sendGlobalMsg(Game::MQ, msg);
}

void PetPvpMgr::setPetPvpCups(int roleid, int setCups)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter == mPlayersData.end()) {
        return;
    }
    
    int cupdiff = iter->second->mCups - setCups;
    
    iter->second->mCups = setCups;
    check_min(iter->second->mCups, 0);
    reSort((PetPvpType)iter->second->mPvpType, iter->second->mRank, eForward);
    
    if (cupdiff < 0) {
        reSort(eGlobal, iter->second->mGlobalRank, eBackward);
    }
    else
    {
        reSort(eGlobal, iter->second->mGlobalRank, eForward);
    }
    
    string pvptype2string = petPvpType2String((PetPvpType)iter->second->mPvpType);
    
    doRedisCmd("zadd paihang:petpvp_%s %d %d", pvptype2string.c_str(), iter->second->mCups, iter->second->mRoleid);
    
    savePlayerData(iter->second);
    
}

void PetPvpMgr::setPetPvpPoints(int roleid, int setpoints)
{
    std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleid);
    if (iter == mPlayersData.end()) {
        return;
    }
    
    //借用已经有的“增加流程”
    addPetPvpPoints(roleid, setpoints - iter->second->mPoints);
}

void PetPvpMgr::setDistribTime()
{
//    if (Process::env.getInt("testmode") == 1) {
//        mDistribTime = Global::tick + 300;
//        return;
//    }
    
    time_t newtime = Utils::mktimeFromToday(12, 0, 0);
    mDistribTime = newtime + 3600*24;
    doRedisCmd("hset serverdata petpvp_distrib %d", mDistribTime);
}
void PetPvpMgr::testMode(obj_roleinfo roleinfo, int sessionid)
{
    return;
    bool testMode = Process::env.getInt("testmode") == 1;
    if (testMode) {
        
        std::map<int, PetPvpData*>::iterator iter = mPlayersData.find(roleinfo.roleid);
        if (iter == mPlayersData.end()) {
            return;
        }
        int cups = iter->second->mCups;
        char buf[512];
        snprintf(buf, 512, "竞技场等级：%d", cups);
        string content = "";
        content.append(buf);
        
        notify_privatechat noty;
        noty.senderId = roleinfo.roleid;
        noty.senderSex = roleinfo.roletype;
        noty.senderName = roleinfo.rolename;
        noty.message = content;
        
        sendNetPacket(sessionid, &noty);
    }
}


void PetPvpMgr::resetDataWithVersionChange()
{
    //加载排行榜
    mGlobalRankData.clear();
    
    //新版本所有玩家从青铜段开始
    PetPvpSectionData sectiondata;
    sectiondata.clear();
    
    mPvpDatas.insert(make_pair(eBronze, sectiondata));
    
    for (int type = 3; type >= 1; type--) {
        
        PetPvpType pvptype = (PetPvpType)type;
        string typeName = petPvpType2String(pvptype);
        
        redisReply* rreply = redisCmd("zrevrange paihang:petpvp_%s 0 -1 withscores", typeName.c_str());
        if (rreply->type == REDIS_REPLY_ARRAY) {
            for (int j = 0; j < rreply->elements; j += 2) {
                int roleid = Utils::safe_atoi(rreply->element[j]->str);
                
                PetPvpData* data = loadPlayerData(roleid);
                
                if (data == NULL) {
                    doRedisCmd("zrem paihang:petpvp_%s %d", typeName.c_str(), roleid);
                    continue;
                }
                
                mPlayersData.insert(make_pair(roleid, data));
                
                mGlobalRankData.push_back(data);
                
                data->mGlobalRank = mGlobalRankData.size();
                
                reSort(eGlobal, data->mGlobalRank, eForward);
                
                addPlayerIntoSection(eBronze, data);
                
                savePlayerData(data);
                
            }
        }
        freeReplyObject(rreply);
    }
    
    //保存新的数据
    saveSectionDataInDB();
}

void PetPvpMgr::setPlayerPvpType()
{
    mPvpDatas.clear();
    
    int affectCount = mGlobalRankData.size();
    if (affectCount <= 0) {
        return;
    }
    
    for (int i = affectCount - 1; i >= 0; i--) {
        
        int limitCups = PetPvpCfg::getLimitCups();
        check_min(limitCups, 0);
        
        if (mGlobalRankData[i]->mCups >= limitCups) {
            break;
        }
        affectCount--;
    }
    
    for (int i = 0; i < mGlobalRankData.size(); i++) {
        
        PetPvpData* playerData = mGlobalRankData[i];
        
        PetPvpType type = PetPvpCfg::calPlayerPetpvpType(playerData->mCups, playerData->mGlobalRank, affectCount);
        
        addPlayerIntoSection(type, playerData);
        
        savePlayerData(playerData); 
    }

    saveSectionDataInDB();
}

void PetPvpMgr::addPlayerIntoSection(PetPvpType type, PetPvpData *playerData)
{
    std::map<int, PetPvpSectionData >::iterator iter = mPvpDatas.find(type);
    
    if (iter == mPvpDatas.end()) {
        PetPvpSectionData sectiondata;
        sectiondata.clear();
        
        sectiondata.push_back(playerData);
        
        playerData->mRank = sectiondata.size();
        
        playerData->mPvpType = type;
        
        mPvpDatas.insert(make_pair(type, sectiondata));
    }
    else{
        
        iter->second.push_back(playerData);
        
        playerData->mRank = iter->second.size();
        
        playerData->mPvpType = type;
        
        reSort(type, playerData->mRank, eForward);
    }
}

string petpvpSessionDataToString(PetPvpMgr::PetPvpSectionData &datas)
{
    string wcmd = "";
    for (int i = 0; i < datas.size(); i++) {
        
        PetPvpData* data = datas[i];
        if (data == NULL) {
            continue;
        }
        string tmp = Utils::makeStr(" %d %d", data->mCups, data->mRoleid);
        wcmd.append(tmp);
    }
    
    return wcmd;
}

void PetPvpMgr::saveSectionDataInDB()
{
    std::map<int, PetPvpSectionData >::iterator iter = mPvpDatas.begin();
    std::map<int, PetPvpSectionData >::iterator endIter = mPvpDatas.end();
    
    vector<PetPvpType> types;
    types.clear();
    types.push_back(eBronze);
    types.push_back(eSilver);
    types.push_back(eGolden);
    string delcmd = "del";
    
    for (int i = 0; i < types.size(); i++) {
        string typeName = petPvpType2String(types[i]);
        string keys = Utils::makeStr("paihang:petpvp_%s", typeName.c_str());
        
        delcmd.append(" ");
        delcmd.append(keys);
    }
    
    vector<string> wcmds;
    wcmds.clear();
    
    for (; iter != endIter; iter++) {
        string typeName = petPvpType2String((PetPvpType)iter->first);
        if (typeName.empty()) {
            continue;
        }
        
        string wcmd = Utils::makeStr("zadd paihang:petpvp_%s", typeName.c_str());
        string data = petpvpSessionDataToString(iter->second);
        
        if (data.empty()) {
            continue;
        }
        
        wcmd.append(data);
        wcmds.push_back(wcmd);
    }
    
    //以事务形式执行
    doRedisCmd("multi");
    doRedisCmd(delcmd.c_str());
    
    for (int i = 0; i < wcmds.size(); i++) {
        doRedisCmd(wcmds[i].c_str());
    }
    
    doRedisCmd("exec");
}

int getBestHistoryRank(int pvpType, const std::vector<int>& inputData)
{
    if (pvpType < 1 || inputData.size() <= 0) {
        return -1;
    }
    return inputData[pvpType - 1];
}