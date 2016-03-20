//
//  PlayerEquip.cpp
//  GameSrv
//
//  Created by pireszhi on 15-6-12.
//
//

#include <stdio.h>
#include "PlayerEquip.h"
#include "json/json.h"
#include "Utils.h"
#include "DataCfg.h"
#include "Game.h"
#include "Role.h"
#include "GameLog.h"

extern string getJsonValue(string& str, string key);

///////////////////////////////////////////////
//char* EquipBaptize::sBaptizeKeyName = "Baptize";
//char* EquipBaptize::sBackupBaptizeKeyName = "BackupBaptize";

bool EquipBaptize::equipBaptize(int roleid, int equipIndex, ItemGroup& equip, int qua, int index, string& propName, string& value)
{
    //先保存原来的属性
    pair<string, string> propBak = EquipBaptizeCfg::getBaptizeProp(equip.json, qua, index);
    
    Json::Reader equipPropReader;
    Json::Value equipJsonVal;
    equipPropReader.parse(equip.json, equipJsonVal);
    
    string baptizeData = equipJsonVal[EquipBaptizeCfg::getBaptizeKeyName()].asString();
    string backupBaptizeData = equipJsonVal[EquipBaptizeCfg::getBackupBaptizeKeyName()].asString();
    
    //更新属性
    equipJsonVal[EquipBaptizeCfg::getBaptizeKeyName()] = setBaptizeProp(baptizeData, qua, index, propName.c_str(), value.c_str());
    
    //更新备份属性
    equipJsonVal[EquipBaptizeCfg::getBackupBaptizeKeyName()] = setBaptizeProp(backupBaptizeData, qua, index, propBak.first.c_str(), propBak.second.c_str());
    
    //更新装备数据
    Json::FastWriter writer;
    equip.json = writer.write(equipJsonVal);
    
    string beforeProp = Utils::makeStr("%s-%s", propBak.first.c_str(), propBak.second.c_str());
    string newProp = Utils::makeStr("%s-%s", newProp.c_str(), value.c_str());
    
    LogMod::addLogEquipBaptize(roleid, equipIndex, equip.item, qua, index,  beforeProp.c_str(), newProp.c_str(), "洗练");
    
    return true;
}

bool EquipBaptize::recoverBaptizeProp(int roleid, int equipIndex, ItemGroup& equip, int qua, int index)
{
    //获取原来的属性
    pair<string, string> propBak = EquipBaptizeCfg::getBackupBaptizeProp(equip.json, qua, index);
    
    if(propBak.first.empty() || propBak.second.empty())
    {
        return false;
    }
    
    //取出现在的属性
    pair<string, string> baptizeProp = EquipBaptizeCfg::getBaptizeProp(equip.json, qua, index);
    
    Json::Reader equipPropReader;
    Json::Value equipJsonVal;
    equipPropReader.parse(equip.json, equipJsonVal);
    
    string baptizeData = equipJsonVal[EquipBaptizeCfg::getBaptizeKeyName()].asString();
    string backupBaptizeData = equipJsonVal[EquipBaptizeCfg::getBackupBaptizeKeyName()].asString();
    
    //更新属性
    equipJsonVal[EquipBaptizeCfg::getBaptizeKeyName()] = setBaptizeProp(baptizeData, qua, index, propBak.first.c_str(), propBak.second.c_str());
    
    //更新备份属性
    equipJsonVal[EquipBaptizeCfg::getBackupBaptizeKeyName()] = setBaptizeProp(backupBaptizeData, qua, index, "", "");
    
    //更新装备数据
    Json::FastWriter writer;
    equip.json = writer.write(equipJsonVal);
    
    string beforeProp = Utils::makeStr("%s-%s", baptizeProp.first.c_str(), baptizeProp.second.c_str());
    string newProp = Utils::makeStr("%s-%s", propBak.first.c_str(), propBak.second.c_str());
    
    LogMod::addLogEquipBaptize(roleid, equipIndex, equip.item, qua, index,  beforeProp.c_str(), newProp.c_str(), "复原");
    
    return true;
}

string EquipBaptize::setBaptizeProp(string& baptizeData, int qua, int index, const char* propName, const char* value)
{
    //洗练属性
    Json::Reader baptizeJsonreader;
    Json::Value baptizeJsonval;
    
    baptizeJsonreader.parse(baptizeData, baptizeJsonval);
    
    string quaStr = Utils::makeStr("%d", qua);
    string quaData = baptizeJsonval[quaStr.c_str()].asString();
    
    Json::FastWriter writer;
    baptizeJsonval[quaStr] = setQuaBatize(quaData, index, propName, value);
    baptizeData = writer.write(baptizeJsonval);
    
    return baptizeData;
}

string EquipBaptize::setQuaBatize(string &data, int index, const char* propName, const char* value)
{
    //品质的数据
    Json::Reader propJsonreader;
    Json::Value propJsonval;
    propJsonreader.parse(data, propJsonval);
    
    if (!propJsonval.isArray()) {
        return data;
    }
    
    while (propJsonval.size() <= index) {
        propJsonval.append(Json::Value());
    }
    
    Json::Value newValue;
    newValue["name"] = propName;
    newValue["val"] = value;
    
    propJsonval[index] = newValue;
    
    Json::FastWriter writer;
    string newPropJson = writer.write(propJsonval);
    
    return newPropJson;
}

bool EquipBaptize::calcBaptizeProp(ItemGroup &equip, int job, BattleProp &batProp)
{
    string baptizeData = getJsonValue(equip.json, EquipBaptizeCfg::getBaptizeKeyName());
    
    //洗练属性
    Json::Reader baptizeJsonreader;
    Json::Value baptizeJsonval;
    
    baptizeJsonreader.parse(baptizeData, baptizeJsonval);
    int qua = ItemCfg::readInt(equip.item, "qua");
    
    for (int i = 0; i <= qua; i++) {
        
        string quaStr = Utils::makeStr("%s", i);
        string quaBaptizeStr = baptizeJsonval[quaStr].asString();
        
        if (quaBaptizeStr.empty()) {
            continue;
        }
        
        calcBaptizeQuaProp(quaBaptizeStr, job, batProp);
    }
    
    return true;
}

bool EquipBaptize::calcBaptizeProp(string& baptizeData, int equipQua, int job, BattleProp& batProp)
{
    if (equipQua < 0) {
        return false;
    }
    
    Json::Reader baptizeJsonreader;
    Json::Value baptizeJsonval;
    
    baptizeJsonreader.parse(baptizeData, baptizeJsonval);
    
    for (int i = 0; i <= equipQua; i++) {
        
        string quaStr = Utils::makeStr("%d", i);
        string quaBaptizeStr = baptizeJsonval[quaStr].asString();
        
        if (quaBaptizeStr.empty()) {
            continue;
        }
        
        calcBaptizeQuaProp(quaBaptizeStr, job, batProp);
    }
    return true;
}

bool EquipBaptize::calcBaptizeQuaProp(string& data, int job, BattleProp& batProp)
{
    //品质的数据
    Json::Reader propJsonreader;
    Json::Value propJsonval;
    propJsonreader.parse(data, propJsonval);
    
    if (!propJsonval.isArray()) {
        return false;
    }
    
    int propCount = propJsonval.size();
    for (int i = 0; i < propCount; i++) {
        
        if (propJsonval[i].isNull()) {
            continue;
        }
        
        string propName = propJsonval[i]["name"].asString();
        string val = propJsonval[i]["val"].asString();
        EquipBaptizeCfg::calcBaptizeProp(propName.c_str(), val.c_str(), batProp);
    }
    
    return true;
}

void EquipBaptize::deleteBackupBaptizeProp(ItemGroup& equip)
{
    Json::Reader equipPropReader;
    Json::Value equipJsonVal;
    equipPropReader.parse(equip.json, equipJsonVal);
    
    equipJsonVal[EquipBaptizeCfg::getBackupBaptizeKeyName()] = "";
    
    Json::FastWriter writer;
    
    equip.json = writer.write(equipJsonVal);
}

void EquipBaptize::checkPlayerEquipBaptize2TimeOut(Role* role)
{
    bool removeBaptize2 = false;
    
    if (role->getEquipBackupBaptizeOverTime() <= 0) {
        
        int lastOverTime = Utils::mktimeFromToday(2);
        role->setEquipBackupBaptizeOverTime(lastOverTime);
        role->NewRoleProperty::save();
    }
    else{
        
        int thisOverTime = role->getEquipBackupBaptizeOverTime() + (3600 * 24);
        
        if (Game::tick > thisOverTime) {
            
            removeBaptize2 = true;
            
            int thisTime = Utils::mktimeFromToday(2);
            role->setEquipBackupBaptizeOverTime(thisTime);
            role->NewRoleProperty::save();
        }
    }
    
    if (removeBaptize2) {
        
        ItemArray& bagitems = role->getPlayerEquip()->GetItems();
        
        for (int i = 0; i < role->getPlayerEquip()->GetCapacity(); i++)
        {
            if (bagitems[i].item <= 0)
            {
                continue;
            }
            
            deleteBackupBaptizeProp(bagitems[i]);
        }
    }
}
