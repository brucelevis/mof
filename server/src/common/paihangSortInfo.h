//
//  paihangSortInfo.h
//  CenterServer
//
//  Created by Cheung Zirpon on 15-6-24.
//
//

#ifndef __CenterServer__paihangSortInfo__
#define __CenterServer__paihangSortInfo__

#include <iostream>
#include "json/value.h"
#include "json/writer.h"
#include "json/reader.h"
#include "Utils.h"
#include "EnumDef.h"
#include "process.h"

//排行榜数据结构

using namespace std;

const int SEND_PAIHANG_DATA_COUNT = 10;
const int CROSSSERVICE_PAIHANG_LOAD_COUNT = 100;

struct PaihangInfo
{
    PaihangInfo(){}
    ~PaihangInfo(){}
    
    virtual int getSortValueByType(int type){return -1;}
    virtual void setSortRankByType(int type, int value){}
    virtual int getSortRankByType(int type){return -1;}


    virtual string str (){return "";}
    virtual bool load(const string &sInfo){return false;}

};

struct SortInfo : public PaihangInfo
{
    SortInfo(): rolename(""),
    roleid(0),
    job(0),
    lvl(0),
    pvp(-1),
    battle(0),
    lvlRank(-1),
    pvpRank(-1),
    battleRank(-1),
    objType(0),
    
    recharge(0),
    consume(0),
    rechargeRank(-1),
    consumeRank(-1),
    serverid(Process::env.getInt("server_id")){}
    
    std::string rolename;
    int roleid;
    int job;
    int lvl;
    int pvp;
    int battle;
    
    int lvlRank;
    int pvpRank;
    int battleRank;

    
    //累计消费充值
    int recharge;
    int consume;
    
    int rechargeRank;
    int consumeRank;

    int serverid;
    int objType;
    
    string str ()
    {
        if (roleid <= 0 || rolename == "" || job < 0 || 6 < job || lvl <= 0) {
            return "";
        }
        
        Json::Value value;
        value["rolename"]           = rolename;
        value["roleid"]             = roleid;
        value["roletype"]           = job;
        value["lvl"]                = lvl;
        value["battle"]             = battle;
        value["rechargerecord"]     = recharge;
        value["rmbconsumedrecord"]  = consume;
        value["serverid"]           = serverid;

        return xyJsonWrite(value);
    }
    
    bool load(const string &sInfo)
    {
        try
        {
            Json::Value value;
            Json::Reader reader;
            
            if (!reader.parse(sInfo, value) || value.type() != Json::objectValue)
            {
                return false;
            }
            
            string tmp_rolename = value["rolename"].asString();
            int tmp_roleid      = value["roleid"].asInt();
            int tmp_job         = value["roletype"].asInt();
            int tmp_lvl         = value["lvl"].asInt();
            
            if (tmp_roleid <= 0 || tmp_rolename.empty() || tmp_job < 0 || 6 < tmp_job || tmp_lvl <= 0) {
                return false;
            }
            
            rolename = tmp_rolename;
            roleid   = tmp_roleid;
            job      = tmp_job;
            lvl      = tmp_lvl;
            battle   = value["battle"].asInt();
            recharge = value["rechargerecord"].asInt();
            consume  = value["rmbconsumedrecord"].asInt();
            serverid = value["serverid"].asInt();
            
        } catch (...)
        {
            return false;
        }
        
        return true;
    }
    
    int getSortValueByType(int type)
    {
        switch (type) {
            case eSortBat:
            {
                return this->battle;
            }
            case eSortConsume:
            {
                return this->consume;
                
            }
            case eSortRecharge:
            {
                return this->recharge;
                
            }
            case eSortServerid:
            {
                return this->serverid;
                
            }
            case eSortIndex:
            {
                return this->roleid;
                
            }
            default:
                break;
        }
        return -1;
    }
    
    void setSortRankByType(int type, int value)
    {
        switch (type) {
            case eSortBat:
            {
                this->battleRank = value;
            }
            case eSortConsume:
            {
                this->consumeRank = value;
                
            }
            case eSortRecharge:
            {
                this->rechargeRank = value;
                
            }
            default:
                break;
        }
    }
    int getSortRankByType(int type)
    {
        switch (type) {
            case eSortBat:
            {
                return this->battleRank;
            }
            case eSortConsume:
            {
                return this->consumeRank;
            }
            case eSortRecharge:
            {
                return this->rechargeRank;
            }
            default:
                break;
        }
        return -1;
    }


};

struct SortPetInfo : public PaihangInfo
{
    int masterid;
    int petmod;
    int petid;
    std::string mastername;
    int battle;
    int growth;
    int rank;
    int serverid;
    
    SortPetInfo(): mastername(""),
    masterid(0),
    petmod(0),
    petid(0),
    battle(0),
    growth(0),
    rank(-1),
    serverid(Process::env.getInt("server_id")){}
    
    string str ()
    {
        if (masterid <= 0 || mastername == "" || petid <= 0 || petmod <= 0) {
            return "";
        }
        
        Json::Value value;
        value["mastername"] = mastername;
        value["masterid"]   = masterid;
        value["petid"]      = petid;
        value["petmod"]     = petmod;
        value["battle"]     = battle;
        value["growth"]     = growth;
        value["serverid"]   = serverid;
        
        return xyJsonWrite(value);
        
    }
    
    bool load(const string &sInfo)
    {
        try
        {
            Json::Value value;
            Json::Reader reader;
            
            if (!reader.parse(sInfo, value) || value.type() != Json::objectValue)
            {
                return false;
            }
            
            string tmp_mastername = value["mastername"].asString();
            int tmp_masterid      = value["masterid"].asInt();
            int tmp_petid         = value["petid"].asInt();
            int tmp_petmod        = value["petmod"].asInt();
            
            if (tmp_masterid <= 0 || tmp_mastername.empty() || tmp_petid <= 0 || tmp_petmod <= 0) {
                return false;
            }
            
            mastername = tmp_mastername;
            masterid   = tmp_masterid;
            petid      = tmp_petid;
            petmod     = tmp_petmod;
            battle     = value["battle"].asInt();
            growth     = value["growth"].asInt();
            serverid   = value["serverid"].asInt();
            
        } catch (...)
        {
            return false;
        }
        
        return true;
    }
    
    int getSortValueByType(int type)
    {
        switch (type) {
            case eSortPet:
            {
                return this->battle;
            }
            case eSortServerid:
            {
                return this->serverid;
                
            }
            case eSortIndex:
            {
                return this->petid;
                
            }
            case eSortRank:
            {
                return this->rank;
                
            }
            default:
                break;
        }
        return -1;
    }
    
    void setSortRankByType(int type, int value)
    {
        switch (type) {
            case eSortPet:
            {
                this->rank = value;
            }
            default:
                break;
        }
    }
    
    int getSortRankByType(int type)
    {
        switch (type) {
            case eSortPet:
            {
                return this->rank;
            }
            default:
                break;
        }
        return -1;
    }
};

#endif /* defined(__CenterServer__paihangSortInfo__) */
