//
//  Robot.cpp
//  GameSrv
//
//  Created by prcv on 14-7-2.
//
//

#include "Robot.h"
#include "main.h"
#include "redis_helper.h"
#include "Obj.h"
#include "inifile.h"
#include "DataCfg.h"

const char* RANK_LIST_PVP_KEY = "paihang:jjc";

extern ObjSex RoleType2Sex(int roletype);
extern ObjJob RoleType2Job(int roletype);


RobotCfgDef* RobotCfg::sDummyCfgData = NULL;
int RobotCfg::sFriendId = 0;
vector<pair<pair<int, int>, RobotCfgDef*> > RobotCfg::sCfgDatas;
RobotMgr g_RobotMgr;

RobotCfgDef* RobotCfg::getCfg(int id)
{
    for (int i = 0; i < sCfgDatas.size(); i++) {
        pair<IdInterval, RobotCfgDef*>& element = sCfgDatas[i];
        IdInterval& interval = element.first;
        if (id >= interval.first && id <= interval.second) {
            return element.second;
        }
    }
    
    return sDummyCfgData;
}

bool RobotCfg::load(const char *file)
{
    GameInifile ini(file);
    sFriendId = ini.getValueT("root", "friend_id", 0);
    
    int intervalNum = ini.getValueT("root", "interval_num", 0);
    for (int i = 1; i <= intervalNum; i++) {
        string section = strFormat("interval%d", i);
        
        int startId = ini.getValueT(section, "start_id", 0);
        int endId = ini.getValueT(section, "end_id", -1);
        if (startId > endId) {
            continue;
        }
        
        IdInterval interval;
        interval.first = startId;
        interval.second = endId;
        
        
        RobotCfgDef *cfgData = new RobotCfgDef;
        
        cfgData->startId = startId;
        cfgData->endId = endId;
        
        string name = ini.getValue(section, "name");
        cfgData->name = name;
        
        int roletype = ini.getValueT(section, "role_type", 0);
        cfgData->roletype = roletype;
        
        //base prop
        int inte = ini.getValueT(section, "inte", 1);
        int stre = ini.getValueT(section, "stre", 1);
        int phys = ini.getValueT(section, "phys", 1);
        int capa = ini.getValueT(section, "capa", 1);
        cfgData->baseprop.setInte(inte);
        cfgData->baseprop.setStre(stre);
        cfgData->baseprop.setPhys(phys);
        cfgData->baseprop.setCapa(capa);
        
        //bat prop
        int lvl = ini.getValueT(section, "lvl", 1);
        int atk = ini.getValueT(section, "atk", 1);
        int def = ini.getValueT(section, "def", 1);
        float hit = ini.getValueT(section, "hit", 1.0f);
        float dodge = ini.getValueT(section, "dodge", 1.0f);
        int maxhp = ini.getValueT(section, "maxhp", 1);
        float cri = ini.getValueT(section, "cri", 1.0f);
        cfgData->batprop.setLvl(lvl);
        cfgData->batprop.setAtk(atk);
        cfgData->batprop.setDef(def);
        cfgData->batprop.setHit(hit);
        cfgData->batprop.setDodge(dodge);
        cfgData->batprop.setMaxHp(maxhp);
        cfgData->batprop.setCri(cri);
        
        //skill
        vector<string> skills;
        skills.push_back(ini.getValue(section, "warrior_skills"));
        skills.push_back(ini.getValue(section, "mage_skills"));
        skills.push_back(ini.getValue(section, "assassin_skills"));
        cfgData->skills.resize(skills.size());
        for (int i = 0; i < skills.size(); i++) {
            vector<string> skillIds = StrSpilt(skills[i], ";");
            for (int j = 0; j < skills.size(); j++) {
                int skillId = Utils::safe_atoi(skillIds[j].c_str());
                if (skillId > 0) {
                    cfgData->skills[i].push_back(skillId);
                }
            }
        }
        //equip
        /*
         kEPTHeadGuard, //头部
         kEPTNecklace,//项链
         kEPTBracelet,//手镯
         kEPTRing,//戒指
         kEPTWeapon,//武器
         kEPTBreastGuard,//胸甲
         kEPTHandGuard,//护手
         kEPTLegGuard,//护腿
         kEPTFootGuard,//护脚
         kEPTFashion,//时装
         kEPTCount
         */
        string parts = ini.getValue(section, "part");
        vector<string> equipPartStr = StrSpilt(parts, ";");
        vector<int> equipParts;
        for (int i = 0; i < equipPartStr.size(); i++) {
            int equipPart =  Utils::safe_atoi(equipPartStr[i].c_str());
            equipParts.push_back(equipPart);
        }
        
        vector<string> equips;
        equips.push_back(ini.getValue(section, "warrior_equips"));
        equips.push_back(ini.getValue(section, "mage_equips"));
        equips.push_back(ini.getValue(section, "assassin_equips"));
        cfgData->equips.resize(equips.size());
        for (int i = 0; i < equips.size(); i++) {
            vector<string> equipIds = StrSpilt(equips[i], ";");
            if (equipIds.size() != equipParts.size()) {
                continue;
            }
            cfgData->equips[i].resize(equipIds.size());
            for (int j = 0; j < equipIds.size(); j++) {
                int equipId = Utils::safe_atoi(equipIds[j].c_str());
                int part = equipParts[j];
                int index = part - 1;
                if (index < 0 || index >= cfgData->equips[i].size()) {
                    continue;
                }
                cfgData->equips[i][index] = equipId;
            }
        }
        
        int star = ini.getValueT(section, "star", 0);
        cfgData->star = star;
        
        string stones = ini.getValue(section, "stones");
        vector<string> stoneIds = StrSpilt(stones, ";");
        for (int i = 0; i < stoneIds.size(); i++) {
            int stoneId = Utils::safe_atoi(stoneIds[i].c_str());
            if (stoneId > 0) {
                cfgData->stones.push_back(stoneId);
            }
        }
        
        
        sCfgDatas.push_back(make_pair(interval, cfgData));
    }
    
    return true;
}

Robot* Robot::create(const char* info)
{
    Robot* robot = NULL;
    
    do {
        Json::Value value;
        Json::Reader reader;
        reader.parse(info, value);
        if (!value.isObject()) {
            break;
        }
        
        int id = value["id"].asInt();
        string name = value["name"].asString();
        int roleType = value["role_type"].asInt();
        float propRate = value["prop_rate"].asDouble() + 1.0f;
        
        RobotCfgDef* cfg = RobotCfg::getCfg(id);
        if (cfg == NULL) {
            break;
        }
        
        if (getObjectType(id) != kObjectRobot) {
            break;
        }
        
        int job = RoleType2Job(roleType);
        robot = new Robot;
        robot->mRoleName = name;
        robot->mId = id;
        robot->mRoleType = roleType;
        
        robot->mBaseProp = cfg->baseprop;
        robot->mBaseProp *= propRate;
        robot->mBattleProp = cfg->batprop;
        robot->mBattleProp *= propRate;
        robot->mBattleProp.setLvl(robot->mBattleProp.getLvl() * propRate);
        robot->mSkills = cfg->skills[job];
        robot->mBattle = calcBattleForce(&robot->mBattleProp);
        
        vector<int> equipIds = cfg->equips[job];
        for (int i = 0; i < equipIds.size(); i++) {
            Json::Value equipJson;
            Json::FastWriter writer;
            equipJson["star"] = cfg->star;
            for (int j = 0; j < cfg->stones.size(); j++) {
                string stoneDesc = strFormat("stone%d", j + 1);
                equipJson[stoneDesc] = cfg->stones[j];
            }
            
            ItemGroup item;
            item.item = equipIds[i];
            item.count = 1;
            item.bind = 0;
            item.json = writer.write(equipJson);
            robot->mEquips.push_back(item);
        }
        
        
    } while (0);
    
    return robot;
}

bool RobotMgr::load()
{
    RedisResult result(redisCmd("hgetall robot:info"));
    
    int count = result.getHashElements();
    for (int i = 0; i < count; i += 2) {
        int keyId = result.readHash(i, 0);
        if (keyId == 0 || getData(keyId)) {
            continue;
        }
        
        string robotInfo = result.readHash(i + 1);
        Robot* robot = Robot::create(robotInfo.c_str());
        if (robot == NULL) {
            continue;
        }
        
        mDatas.insert(make_pair(keyId, robot));
        mNameIndices.insert(make_pair(robot->mRoleName, robot));
    }
    
    return true;
}

Robot* RobotMgr::getData(int robotId)
{
    map<int, Robot*>::iterator iter = mDatas.find(robotId);
    if (iter == mDatas.end()) {
        return NULL;
    }
    
    Robot* data = iter->second;
    return data;
}

Robot* RobotMgr::getData(const char *name)
{
    map<string, Robot*>::iterator iter = mNameIndices.find(name);
    if (iter == mNameIndices.end()) {
        return NULL;
    }
    
    Robot* data = iter->second;
    return data;
}

