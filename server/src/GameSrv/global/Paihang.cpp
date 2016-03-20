//
//  Paihang.cpp
//  GameSrv
//
//  Created by pireszhi on 13-7-1.
//
//
#include "Paihang.h"
#include "EnumDef.h"
#include "Defines.h"
#include "Global.h"
#include "cmd_def.h"
#include "main.h"
#include <algorithm>
#include <set>
#include "Robot.h"
#include "GameLog.h"
#include "centerClient.h"
//INSTANTIATE_SINGLETON(SortManager);

SortManager SSortMgr;

bool  sortByLvl(SortInfo *first, SortInfo *second)
{
    if (first->battle == second->battle)
    {
        return first->lvl < second->lvl;
    }
    return second->battle < first->battle;
}

bool sortByBattle(SortInfo *first, SortInfo *second)
{
    if (first->lvl == second->lvl) {
        return second->battle < first->battle;
    }
    else return second->lvl < first->lvl;
}

bool  rechargeSort(SortInfo *first, SortInfo *second)
{
    if (first->recharge == second->recharge)
    {
        return first->lvl < second->lvl;
    }
    return second->recharge < first->recharge;
}

bool  consumeSort(SortInfo *first, SortInfo *second)
{
    if (first->consume == second->consume)
    {
        return first->lvl < second->lvl;
    }
    return second->consume < first->consume;
}

SortManager::SortManager()
{
    mRunning = false;
    mNextFreshTime = 0;
}

void SortManager::init()
{
    RedisResult result(redisCmd("hget serverdata paihangFreshTime"));

    int nexttime = result.readInt();

    if (nexttime) {
        mNextFreshTime = nexttime;
    }
    else{
        setNextFreshTime();
    }

    loadSortData();

    mRunning = true;
}

void SortManager::updateDB(int roleid, int lvl, int bat, int consume, int recharge, int optionType)
{
    if (optionType == eUpdate) {

        doRedisCmd("zadd paihang:lvl %d %d", lvl, roleid);

        doRedisCmd("zadd paihang:bat %d %d", bat, roleid);
        
        doRedisCmd("zadd paihang:consume %d %d", consume, roleid);

        doRedisCmd("zadd paihang:recharge %d %d", recharge, roleid);
        
        SortInfo* roleinfo = loadRoleInfoFromDB(roleid);
        if (NULL == roleinfo) {
            return;
        }
        vector<SortInfo*> ret(1, roleinfo);
        //暂时只知道 在 global 线程里  这样拿 centerserver 的sessionid 以后设计框架完善了 再改, esortend 用来标示这次发送是 update
        sendPaihangRoleDbData2CenterServer(CenterClient::instance()->getSession(), eSortEnd, ret);

    }
    else{
        
        int64_t targetid = MAKE_OBJ_ID(kObjectRole, roleid);
        
        std::map<int64_t, SortInfo*>::iterator iter = m_allroledata.find(targetid);

        if (iter == m_allroledata.end()) {
            return;
        }

        int lvlRank      = iter->second->lvlRank;
        int batRank      = iter->second->battleRank;
        int pvpRank      = iter->second->pvpRank;
        int consumeRank  = iter->second->consumeRank;
        int rechargeRank = iter->second->rechargeRank;

        //等级排行榜
        if (lvlRank >0 && lvlRank <= m_sort_lvl.size()) {
            if (roleid == m_sort_lvl[lvlRank - 1]->roleid) {

                m_sort_lvl.erase(m_sort_lvl.begin() + lvlRank - 1);

                for (int i = lvlRank - 1; i < m_sort_lvl.size(); i++) {
                    m_sort_lvl[i]->lvlRank--;
                }
            }
        }

        //战斗力排行榜
        if (batRank >0 && batRank <= m_sort_bat.size()) {
            if (roleid == m_sort_bat[batRank - 1]->roleid) {

                m_sort_bat.erase(m_sort_bat.begin() + batRank - 1);

                for (int i = batRank - 1; i < m_sort_bat.size(); i++) {
                    m_sort_bat[i]->battleRank--;
                }
            }
        }
        
        //竞技场排行榜
        if (pvpRank >0 && pvpRank <= m_sort_pvp.size()) {
            if (roleid == m_sort_pvp[pvpRank - 1]->roleid) {
                
                m_sort_pvp.erase(m_sort_pvp.begin() + pvpRank - 1);
                
                for (int i = pvpRank - 1; i < m_sort_pvp.size(); i++) {
                    m_sort_pvp[i]->pvpRank--;
                }
            }
        }
        
        //累计充值排行榜
        if (rechargeRank >0 && rechargeRank <= m_sort_recharge.size()) {
            if (roleid == m_sort_recharge[rechargeRank - 1]->roleid) {
                
                m_sort_recharge.erase(m_sort_recharge.begin() + rechargeRank - 1);
                
                for (int i = rechargeRank - 1; i < m_sort_recharge.size(); i++) {
                    m_sort_recharge[i]->rechargeRank--;
                }
            }
        }
        
        //累计消费排行榜
        if (consumeRank >0 && consumeRank <= m_sort_consume.size()) {
            if (roleid == m_sort_consume[consumeRank - 1]->roleid) {
                
                m_sort_consume.erase(m_sort_consume.begin() + consumeRank - 1);
                
                for (int i = consumeRank - 1; i < m_sort_consume.size(); i++) {
                    m_sort_consume[i]->consumeRank--;
                }
            }
        }

        SortInfo* info = iter->second;
        delete info;
        m_allroledata.erase(iter);
        doRedisCmd("zrem paihang:lvl %d", roleid);

        doRedisCmd("zrem paihang:bat %d", roleid);
        
        doRedisCmd("zrem paihang:recharge %d", roleid);

        doRedisCmd("zrem paihang:consume %d", roleid);

    }

}

void SortManager::updatePetDB(int petid, int battle, int optionType)
{
    if (optionType == eRemove) {
        doRedisCmd("zrem paihang:pet %d", petid);
    }
    else if(optionType == eUpdate)
    {
        doRedisCmd("zadd paihang:pet %d %d", battle, petid);
        
        SortPetInfo* petinfo = loadPetInfoFromDB(petid);
        if (NULL == petinfo) {
            return;
        }
        vector<SortPetInfo*> ret(1, petinfo);
        //暂时只知道 在 global 线程里  这样拿 centerserver 的sessionid 以后设计框架完善了 再改
        sendPaihangPetDbData2CenterServer(CenterClient::instance()->getSession(), eSortPet, ret);
    }
}

void SortManager::getSortData(int type, int job, int begingRank, int endRank, vector<SortInfo *> &outdata)
{
    if (type == eSortLvl) {
        if (job == eWarrior) {

        }
        else if(job == eMage)
        {

        }
        else if (job == eAssassin)
        {

        }
        else
        {
            check_min(begingRank, 1);
            for (int i = begingRank - 1; i < m_sort_lvl.size() && i < endRank; i++) {
                outdata.push_back(m_sort_lvl[i]);
            }
        }
    }
    else if(type == eSortBat)
    {
        if (job == eWarrior) {

        }
        else if(job == eMage)
        {

        }
        else if (job == eAssassin)
        {

        }
        else
        {
            check_min(begingRank, 1);
            for (int i = begingRank - 1; i < m_sort_bat.size() && i < endRank; i++) {
                outdata.push_back(m_sort_bat[i]);
            }
        }
    }

    else if(type == eSortPvp)
    {
        if (job == eWarrior) {

        }
        else if(job == eMage)
        {

        }
        else if (job == eAssassin)
        {

        }
        else
        {
            check_min(begingRank, 1);
            for (int i = begingRank - 1; i < m_sort_pvp.size() && i < endRank; i++) {
                outdata.push_back(m_sort_pvp[i]);
            }
        }
    }
    else if(type == eSortPet)
    {

    }
    else if(type == eSortConsume)
    {
        if (job == eWarrior) {
            
        }
        else if(job == eMage)
        {
            
        }
        else if (job == eAssassin)
        {
            
        }
        else
        {
            check_min(begingRank, 1);
            for (int i = begingRank - 1; i < m_sort_consume.size() && i < endRank; i++) {
                outdata.push_back(m_sort_consume[i]);
            }
        }
    }
    else if(type == eSortRecharge)
    {
        if (job == eWarrior) {
            
        }
        else if(job == eMage)
        {
            
        }
        else if (job == eAssassin)
        {
            
        }
        else
        {
            check_min(begingRank, 1);
            for (int i = begingRank - 1; i < m_sort_recharge.size() && i < endRank; i++) {
                outdata.push_back(m_sort_recharge[i]);
            }
        }
    }
}

void SortManager::getPetSortData(int beginRank, int endRank, vector<SortPetInfo*> &outdata)
{
    check_min(beginRank, 1);
    for (int i = beginRank - 1; i < m_sort_pet.size() && i < endRank; i++) {
        outdata.push_back(m_sort_pet[i]);
    }
}

SortInfo* SortManager::getRoleInfoFromDB(int64_t roleid)
{
    int64_t targetId = MAKE_OBJ_ID(kObjectRole, roleid);
    
    map<int64_t, SortInfo*>::iterator iter = m_allroledata.find(targetId);

    if (iter == m_allroledata.end())
    {
        SortInfo* info = loadRoleInfoFromDB(roleid);
        m_allroledata.insert(make_pair(targetId, info));
        return info;
    }
    else
    {
        return iter->second;
    }
}

SortInfo* SortManager::loadRoleInfoFromDB(int roleid)
{
    RedisResult result( redisCmd("hmget role:%d rolename lvl bat roletype rechargerecord rmbconsumedrecord", roleid) );
    string rolename             = result.readHash(0, "");
    int rolelvl                 = Utils::safe_atoi(result.readHash(1, "").c_str());
    int battle                  = Utils::safe_atoi(result.readHash(2, "").c_str());
    int job                     = Utils::safe_atoi(result.readHash(3, "").c_str());
    string sRechargeRecord      = result.readHash(4, "");
    string sRmbconsumedrecord   = result.readHash(5, "");
    
    if (rolename.empty() || rolelvl <= 0) {
        doRedisCmd("zrem paihang:lvl %d", roleid);
        return NULL;
    }
    
    Json::Reader reader;
    Json::Value value;
    int recharge = 0;
    if (reader.parse(sRechargeRecord, value) && value.type() == Json::arrayValue) {
        for (int i = 0 ; i < value.size(); i++) {
            if (0 < value[i]["Sum"].asInt()) {
                recharge = value[i]["Sum"].asInt();
                break;
            }
        }
    }
    
    
    int consume = 0;
    if (reader.parse(sRmbconsumedrecord, value) && value.type() == Json::arrayValue) {
        for (int i = 0 ; i < value.size(); i++) {
            if (0 < value[i]["Sum"].asInt()) {
                consume = value[i]["Sum"].asInt();
                break;
            }
        }
    }
    
    
    SortInfo *info  = new SortInfo;
    info->roleid    = roleid;
    info->rolename  = rolename;
    info->lvl       = rolelvl;
    info->battle    = battle;
    info->job       = job;
    info->objType   = kObjectRole;
    info->recharge  = recharge;
    info->consume   = consume;
    
    return info;

}

SortPetInfo* SortManager::loadPetInfoFromDB(int petid)
{
    SortPetInfo *pet = new SortPetInfo;

    redisReply* petdata = redisCmd("hmget pet:%d owner petmod growth battle", petid);
    
    if (petdata->element[0]->str == NULL) {
        return NULL;
    }
    
    int masterid = Utils::safe_atoi(petdata->element[0]->str);
    redisReply* master = redisCmd("hget role:%d rolename", masterid);
    if (master->str == NULL) {
        return NULL;
    }
    
    pet->masterid   = masterid;
    pet->petid      = petid;
    pet->mastername = master->str;
    pet->petmod     = Utils::safe_atoi( petdata->element[1]->str );
    pet->growth     = Utils::safe_atoi( petdata->element[2]->str );
    pet->battle     = Utils::safe_atoi( petdata->element[3]->str );
    
    freeReplyObject(master);
    freeReplyObject(petdata);
    
    return pet;
}

SortInfo* SortManager::getRobotInfo(int64_t robotid)
{
    map<int64_t, SortInfo*>::iterator iter = m_allroledata.find(robotid);
    
    SortInfo* info = NULL;
    
    if (iter == m_allroledata.end())
    {
        info = new SortInfo;

        m_allroledata.insert(make_pair(robotid, info));
        
        return info;
    }
    else{
        return iter->second;
    }
}

void SortManager::loadSortData()
{
    redisReply* rreply;
    clearSort();

    //加载等级排行榜
    rreply = redisCmd("zrevrange paihang:lvl 0 %d withscores", PAIHANG_LOAD_COUNT);
    if (rreply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < rreply->elements; i += 2) {

            int roleid = Utils::safe_atoi(rreply->element[i]->str);
            int val = Utils::safe_atoi(rreply->element[i+1]->str);
            SortInfo *info = getRoleInfoFromDB(roleid);
            if (info == NULL) {
                continue;
            }
            info->lvl = val;
            m_sort_lvl.push_back(info);
        }
        ::sort(m_sort_lvl.begin(), m_sort_lvl.end(), sortByBattle);

        for (int i = 0; i < m_sort_lvl.size(); i++) {
            m_sort_lvl[i]->lvlRank = i+1;
        }
    }
    freeReplyObject(rreply);

    //加载战斗力排行榜
    rreply = redisCmd("zrevrange paihang:bat 0 %d withscores", PAIHANG_LOAD_COUNT);
    if (rreply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < rreply->elements; i += 2) {

            int roleid = Utils::safe_atoi(rreply->element[i]->str);
            int val = Utils::safe_atoi(rreply->element[i+1]->str);
            SortInfo *info = getRoleInfoFromDB(roleid);
            if (info == NULL) {
                continue;
            }
            info->battle = val;
            m_sort_bat.push_back(info);
        }
        ::sort(m_sort_bat.begin(), m_sort_bat.end(), sortByLvl);

        for (int i = 0; i < m_sort_bat.size(); i++) {
            m_sort_bat[i]->battleRank = i+1;
        }
    }
    freeReplyObject(rreply);

    //加载pvp排行榜
    rreply = redisCmd("zrange paihang:jjc 0 %d withscores", PAIHANG_LOAD_COUNT);
    if (rreply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < rreply->elements; i += 2){
            
            const char* str= rreply->element[i]->str;
            
            if (str == NULL || *str == 0) {
                continue;
            }
            
            int64_t target64Id = atoll(rreply->element[i]->str);
            
            int targetid = 0;
            int targetType = 0;
            
            getPvpObjTypeAndObjId(target64Id, targetid, targetType);
            
            SortInfo *info = NULL;
            
            if (targetType == kObjectRole) {
                
                info = getRoleInfoFromDB(targetid);

            }
            else if(targetType == kObjectRobot)
            {
                Robot* robot = g_RobotMgr.getData(targetid);
                if (robot) {
                    
                    info = getRobotInfo(target64Id);
                    info->objType = targetType;
                    info->lvl = robot->mBattleProp.mLvl;
                    info->job = robot->mRoleType;
                    info->battle = robot->mBattle;
                    info->rolename = robot->mRoleName;
                    info->roleid = robot->mId;
                    
                    m_allroledata.insert(make_pair(target64Id, info));
                }
            }
            
            if (info == NULL) {
                continue;
            }
            
            int val = Utils::safe_atoi(rreply->element[i+1]->str);

            info->pvp = val;
            m_sort_pvp.push_back(info);
        }

        for (int i = 0; i < m_sort_pvp.size(); i++) {
            m_sort_pvp[i]->pvpRank = i+1;
        }
    }
    freeReplyObject(rreply);

    //加载宠物排行榜
    rreply = redisCmd("zrevrange paihang:pet 0 %d withscores", PAIHANG_LOAD_COUNT);
    if (rreply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < rreply->elements; i += 2){
            int petid = Utils::safe_atoi(rreply->element[i]->str);
            int val = Utils::safe_atoi(rreply->element[i+1]->str);

            redisReply* petdata = redisCmd("hmget pet:%d owner petmod growth", petid);
            if (petdata->element[0]->str != NULL) {
                int masterid = Utils::safe_atoi(petdata->element[0]->str);
                redisReply* master = redisCmd("hget role:%d rolename", masterid);
                if (master->str != NULL) {
                    SortPetInfo *pet = new SortPetInfo;
                    pet->masterid = masterid;
                    pet->petid = petid;
                    pet->mastername = master->str;
                    pet->petmod = Utils::safe_atoi( petdata->element[1]->str );
                    pet->growth = Utils::safe_atoi( petdata->element[2]->str );
                    pet->battle = val;
                    m_sort_pet.push_back(pet);
                    m_role_pet.insert(make_pair(masterid, m_sort_pet.size()));
                }

                freeReplyObject(master);
            }
            freeReplyObject(petdata);
        }
    }
    freeReplyObject(rreply);
    
    //暂时写成这样 以后有时间 把它们抽象出来
    //以后准备 把所有的榜 抽象为一个基类(load 数据库函数 和 refresh vector的函数)  具体的榜继承基类 每个榜单独管理自己的数据
    //加载累计消费排行榜
    
    do
    {
        ServerGroupCfgDef*  def = g_ConsumeRankListGroupCfg.getServerGroupCfgDefByServerid(Process::env.getInt("server_id"));
            
            if (def == NULL) {
                break;
            }
            
            int beginTime = def->mBeginTime;
            int endTime   = def->mEndTime;

            int currentTime = time(NULL);
            if (beginTime <= currentTime && currentTime <= endTime) {
                
                rreply = redisCmd("zrevrange paihang:consume 0 %d withscores", PAIHANG_LOAD_COUNT);
                if (rreply->type == REDIS_REPLY_ARRAY) {
                    for (int i = 0; i < rreply->elements; i += 2) {
                        
                        int roleid  = Utils::safe_atoi(rreply->element[i]->str);
                        //            int val     = Utils::safe_atoi(rreply->element[i+1]->str);
                        SortInfo *info = getRoleInfoFromDB(roleid);
                        if (info == NULL) {
                            continue;
                        }
                        m_sort_consume.push_back(info);
                    }
                    ::sort(m_sort_consume.begin(), m_sort_consume.end(), consumeSort);
                    
                    for (int i = 0; i < m_sort_consume.size(); i++) {
                        m_sort_consume[i]->consumeRank = i+1;
                    }
                }
                freeReplyObject(rreply);
            }
        
    }while (false);


    
    //加载累计充值排行榜
    
    do
    {
        ServerGroupCfgDef*  def = g_RechargeRankListGroupCfg.getServerGroupCfgDefByServerid(Process::env.getInt("server_id"));
        
        if (def == NULL) {
            break;
        }
        
        int beginTime = def->mBeginTime;
        int endTime   = def->mEndTime;
        
        int currentTime = time(NULL);
        if (beginTime <= currentTime && currentTime <= endTime) {
            
            rreply = redisCmd("zrevrange paihang:recharge 0 %d withscores", PAIHANG_LOAD_COUNT);
            if (rreply->type == REDIS_REPLY_ARRAY) {
                for (int i = 0; i < rreply->elements; i += 2) {
                    
                    int roleid  = Utils::safe_atoi(rreply->element[i]->str);
                    //            int val     = Utils::safe_atoi(rreply->element[i+1]->str);
                    SortInfo *info = getRoleInfoFromDB(roleid);
                    if (info == NULL) {
                        continue;
                    }
                    m_sort_recharge.push_back(info);
                }
                ::sort(m_sort_recharge.begin(), m_sort_recharge.end(), rechargeSort);
                
                for (int i = 0; i < m_sort_recharge.size(); i++) {
                    m_sort_recharge[i]->rechargeRank = i+1;
                }
            }
            freeReplyObject(rreply);
        }
        
    }while (false);
    
    notify_sync_fresh_paihang notify;
    broadcastPacket(WORLD_GROUP, &notify);
}

void SortManager::clearSort()
{
    m_sort_lvl.clear();
    m_sort_bat.clear();
    m_sort_pvp.clear();
    m_sort_recharge.clear();
    m_sort_consume.clear();

    for (int i = 0; i < m_sort_pet.size(); i++) {
        SortPetInfo* petinfo = m_sort_pet[i];
        if (petinfo) {
            delete petinfo;
        }
    }
    m_sort_pet.clear();
    m_role_pet.clear();

    std::map<int64_t, SortInfo*>::iterator iter = m_allroledata.begin();
    for (;iter != m_allroledata.end(); iter++) {
        SortInfo* info = iter->second;
        if (info) {
            delete info;
        }
    }
    m_allroledata.clear();
}

void SortManager::getMyRank(int64_t roleid, int type, int &pvp, int &rank)
{
    std::map<int64_t, SortInfo*>::iterator iter;
    
    int64_t targetId = MAKE_OBJ_ID(kObjectRole, roleid);
    
    iter = m_allroledata.find(targetId);
    if (iter == m_allroledata.end()) {
        rank = -1;
        pvp = -1;
        return;
    }

    if (iter != m_allroledata.end() && iter->second->pvp < 100) {
        pvp = iter->second->pvp;
    }
    else pvp = -1;

    switch (type) {
        case eSortBat:
            rank = iter->second->battleRank;
            break;
        case eSortLvl:
            rank = iter->second->lvlRank;
            break;
        case eSortRecharge:
            rank = iter->second->rechargeRank;
            break;
        case eSortConsume:
            rank = iter->second->consumeRank;
            break;
        default:
            break;
    }
}

void SortManager::getPaihangRoleDbDataByType(int type, vector<SortInfo*> &output)
{
    set<int> obj_ids;
    
    redisReply* rreply = NULL;
    
    //暂时写成这样 以后有时间 把它们抽象出来
    //取 bat 排行榜 roleid
    switch (type) {
        case eSortBat:
        {
            rreply = redisCmd("zrevrange paihang:bat 0 %d withscores", CROSSSERVICE_PAIHANG_LOAD_COUNT);
            if (rreply->type == REDIS_REPLY_ARRAY) {
                for (int i = 0; i < rreply->elements; i += 2) {
                    obj_ids.insert(Utils::safe_atoi(rreply->element[i]->str));
                }
            }
        }
            break;
        case eSortRecharge:
        {
            //取 recharge 排行榜 roleid
            rreply = redisCmd("zrevrange paihang:recharge 0 %d withscores", CROSSSERVICE_PAIHANG_LOAD_COUNT);
            if (rreply->type == REDIS_REPLY_ARRAY) {
                for (int i = 0; i < rreply->elements; i += 2) {
                    obj_ids.insert(Utils::safe_atoi(rreply->element[i]->str));
                }
            }
        }
        case eSortConsume:
        {
            //取 consume 排行榜 roleid
            rreply = redisCmd("zrevrange paihang:consume 0 %d withscores", CROSSSERVICE_PAIHANG_LOAD_COUNT);
            if (rreply->type == REDIS_REPLY_ARRAY) {
                for (int i = 0; i < rreply->elements; i += 2) {
                    obj_ids.insert(Utils::safe_atoi(rreply->element[i]->str));
                }
            }
        }
            break;
        default:
            break;
    }
    
    freeReplyObject(rreply);

    
    set<int>::iterator iter;
    foreach(iter, obj_ids)
    {
        SortInfo *info = loadRoleInfoFromDB(*iter);
        if (info == NULL) {
            continue;
        }
        output.push_back(info);
    }

}

//清除数据 output之所以用指针, 方便以后 把这个取数据库数据的函数用到 Sortmgr的 load 函数里面, 这个以后有时间会改
void SortManager::getPaihangPetDbData(vector<SortPetInfo*> &output)
{
    redisReply* rreply = NULL;

    set<int> pet_ids;
    rreply = redisCmd("zrevrange paihang:pet 0 %d withscores", CROSSSERVICE_PAIHANG_LOAD_COUNT);
    
    if (rreply->type == REDIS_REPLY_ARRAY) {
        for (int i = 0; i < rreply->elements; i += 2) {
            pet_ids.insert(Utils::safe_atoi(rreply->element[i]->str));
        }
    }
    
    set<int>::iterator iter;
    foreach(iter, pet_ids)
    {
        SortPetInfo* pet = loadPetInfoFromDB(*iter);
        if (NULL == pet) {
            return;
        }
        output.push_back(pet);
    }
    
    freeReplyObject(rreply);
}


void SortManager::getMyPetRank(int roleid, int &rank)
{
    std::map<int, int>::iterator iter;
    iter = m_role_pet.find(roleid);
    if (iter != m_role_pet.end()) {
        rank = iter->second;
    }
    else rank = -1;
}

void SortManager::onHeartBeat(float dt)
{
    time_t nowtime = Global::tick;

    if (nowtime >= mNextFreshTime) {
        loadSortData();
        setNextFreshTime();
        
        LogMod::addLogRoleSortData(eSortBat, m_sort_bat);
        LogMod::addLogRoleSortData(eSortLvl, m_sort_lvl);
        LogMod::addLogRoleSortData(eSortPvp, m_sort_pvp);
        LogMod::addLogRoleSortData(eSortConsume, m_sort_consume);
        LogMod::addLogRoleSortData(eSortRecharge, m_sort_recharge);

        LogMod::addLogPetBattleForceSortData(m_sort_pet);
    }
}

void SortManager::setNextFreshTime()
{
    int nowtime = time(NULL);

    nowtime = nowtime - nowtime % (SeparateValues::sPaihangFreshPeriod);

    mNextFreshTime = nowtime + SeparateValues::sPaihangFreshPeriod;

    doRedisCmd("hset serverdata paihangFreshTime %d", mNextFreshTime);
}

void SortManager::safe_delRoleInPaihang(int roleid)
{
    create_global(updatePaihangDB, paihang);
    paihang->roleid = roleid;
    paihang->optionType = eRemove;
    sendGlobalMsg(Global::MQ, paihang);
}

void SortManager::safe_delPetInPaihang(int petid)
{
    create_global(updatePetPaihangDB, paihang);
    paihang->optionType = eRemove;
    paihang->petid = petid;
    sendGlobalMsg(Global::MQ, paihang);
}

void geteSortBatData(int roleid, int beginRank, int endRank, GETPAIHANGDATAFUNC cbfunc)
{
    create_global(getPaihangData, msg);
    msg->retCallBackFunc = (void*)cbfunc;
    msg->info.roleid = roleid;
    msg->info.paihangType = eSortBat;
    msg->info.beginRank = beginRank;
    msg->info.endRank = endRank;
    sendGlobalMsg(Global::MQ, msg);
}
void getSortPetData(int roleid, int beginRank, int endRank, GETPAIHANGPETDATAFUNC cbfunc)
{
    create_global(getPaihangData, msg);
    msg->retCallBackFunc = (void*)cbfunc;
    msg->info.roleid = roleid;
    msg->info.paihangType = eSortPet;
    msg->info.beginRank = beginRank;
    msg->info.endRank = endRank;
    sendGlobalMsg(Global::MQ, msg);
}

void getSortData(int roleid, int type, int beginRank, int endRank, GETPAIHANGDATAFUNC cbfunc, int rolejob)
{
    create_global(getPaihangData, msg);
    msg->retCallBackFunc = (void*)cbfunc;
    msg->info.roleid = roleid;
    msg->info.paihangType = type;
    msg->info.beginRank = beginRank;
    msg->info.endRank = endRank;
	msg->info.job = rolejob;
    sendGlobalMsg(Global::MQ, msg);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//向 global 请求数据库排行数据
void reqPaihangDbData(int sessionid, int type)
{
    create_global(CrossServiceGetPaihangDbData, rolePaihang);
    rolePaihang->type       = type;
    rolePaihang->sessionid  = sessionid;
    sendGlobalMsg(Global::MQ, rolePaihang);
}

//发送收集到的角色排行数据
void sendPaihangRoleDbData2CenterServer(int sessionid, int type, vector<SortInfo*> &data)
{
    if (0 == data.size()) {
        return;
    }
    int sendTimes = data.size() / SEND_PAIHANG_DATA_COUNT;
    if (data.size() % SEND_PAIHANG_DATA_COUNT == 0) {
        sendTimes -= 1;
    }
    
    for (int i = 0; i <= sendTimes; i++) {
        
        ack_cs_loadPaihangRoleData ack;
        ack.type  = type;
        ack.total = data.size();
        
        int begin = i * SEND_PAIHANG_DATA_COUNT;
        int end   = (i + 1) * SEND_PAIHANG_DATA_COUNT;
        
        end = data.size() < end ? data.size() : end;
        
        for (int j = begin; j < end; j++) {
            obj_paihangdata obj;
            obj.job         = data[j]->job;
            obj.rolename    = data[j]->rolename;
            obj.roleid      = data[j]->roleid;
            obj.battle      = data[j]->battle;
            obj.lvl         = data[j]->lvl;
            obj.recharge    = data[j]->recharge;
            obj.consume     = data[j]->consume;
            obj.serverid    = data[j]->serverid;
            
            ack.data.push_back(obj);
        }
        sendNetPacket(sessionid, &ack);
    }
}

//发送收集到的幻兽排行数据
//与上面代码一样 以后优化 合成一个
void sendPaihangPetDbData2CenterServer(int sessionid, int type, vector<SortPetInfo*> &data)
{
    if (0 == data.size()) {
        return;
    }
    int sendTimes = data.size() / SEND_PAIHANG_DATA_COUNT;
    if (data.size() % SEND_PAIHANG_DATA_COUNT == 0) {
        sendTimes -= 1;
    }
    
    for (int i = 0; i <= sendTimes; i++) {
        
        ack_cs_loadPaihangPetData ack;
        ack.type  = type;
        ack.total = data.size();
        
        int begin = i * SEND_PAIHANG_DATA_COUNT;
        int end   = (i + 1) * SEND_PAIHANG_DATA_COUNT;
        
        end = data.size() < end ? data.size() : end;
        
        for (int j = begin; j < end; j++) {
            obj_Petpaihangdata obj;
            obj.rolename  = data[j]->mastername;
            obj.petid     = data[j]->petid;
            obj.modid     = data[j]->petmod;
            obj.battle    = data[j]->battle;
            obj.growth    = data[j]->growth;
            obj.roleid    = data[j]->masterid;
            obj.serverid  = data[j]->serverid;
            
            ack.data.push_back(obj);
        }
        sendNetPacket(sessionid, &ack);
    }
    
    for (int i = 0; i < data.size(); i++) {
        SortPetInfo* tmp = data[i];
        delete tmp;
        tmp = NULL;
    }
    data.clear();
}
