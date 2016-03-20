//
//  DataCfgEquip.cpp
//  GameSrv
//
//  Created by prcv on 14-1-19.
//
//

#include <stdint.h>
#include <limits.h>
#include "DataCfgEquip.h"
#include <limits.h>
#include "inifile.h"
#include "Utils.h"
#include "DataCfg.h"

vector<StarCfgDef> StarCfg::cfg;
StarCfgDef StarCfg::invalidcfg;
int StarCfg::MAX_STAR_LVL = 100;
int StarCfg::mIntensifyStoneId = 0;

void StarCfg::load(std::string fullPath)
{
    GameInifile starini(fullPath);
    
    // char buf[16];
    
    string buf;
    cfg.clear();
    string _common = "common";
    invalidcfg.setEffect(1.0);
    invalidcfg.gold = INT_MAX;
    invalidcfg.reqlvl = MAX_ROLE_LVL + 1;
    
    StarCfg::setIntensifyStoneId(starini.getValueT(_common.c_str(), "intensifyStoneId", 0));
    for (int i = 0; i <= MAX_STAR_LVL; i++)
    {
        StarCfgDef starcfg;
        //sprintf(buf, "star%d", i);
        buf = Utils::makeStr("star%d", i);
        starcfg.setEffect(atof(starini.getValue(buf, "effect", "1.0").c_str()));
        starcfg.gold            = atoi(starini.getValue(buf, "gold", "0").c_str());
        starcfg.intensifyStones = atoi(starini.getValue(buf, "intensifyStones", "0").c_str());
        starcfg.reqlvl          = atoi(starini.getValue(buf, "reqlvl", "0").c_str());
        
        cfg.push_back(starcfg);
    }
}

int StarCfg::getReqLvl(int starlvl)
{
    if (starlvl > MAX_STAR_LVL || starlvl < 0)
    {
        return MAX_ROLE_LVL + 1;
    }
    
    return cfg[starlvl].reqlvl;
}
int StarCfg::getReqGold(int starlvl)
{
    if (starlvl > MAX_STAR_LVL || starlvl < 0)
    {
        return INT_MAX;
    }
    
    return cfg[starlvl].gold;
}
int StarCfg::getReqIntensifyStones(int starlvl)
{
    if (starlvl > MAX_STAR_LVL || starlvl < 0)
    {
        return INT_MAX;
    }
    
    return cfg[starlvl].intensifyStones;
}
float StarCfg::getEffect(int starlvl)
{
    if (starlvl > MAX_STAR_LVL || starlvl < 0)
    {
        return 1.0;
    }
    
    return cfg[starlvl].getEffect();
}


bool StarCfg::getCfg(int starlvl, StarCfgDef& def)
{
    if (starlvl > MAX_STAR_LVL || starlvl <= 0)
    {
        def = invalidcfg;
        return true;
    }
    
    def = cfg[starlvl];
    return true;
}


///--------------------------------------------------------------------------
// 新版强化
///--------------------------------------------------------------------------

int StarCostCfgDef::getSellGold(int starLvl)
{
    if (starLvl <= 0 || starLvl > sellGold.size())
    {
        return 0;
    }
    
    return sellGold[starLvl - 1];
}




int StarCostCfgDef::getMaxLvl()
{
    
    return maxLvl;
}

bool StarCostCfgDef::getUpgCost(int starLvl, int& gold, int& rmb)
{
    gold = INT_MAX;
    rmb = INT_MAX;
    if (starLvl <= 0 || starLvl > upgCost.size())
    {
        return false;
    }
    
    gold = upgCost[starLvl - 1];
    rmb = upgRmbCost[starLvl - 1];
    
    return true;
}


//获取强化所需经验
int StarCostCfgDef::getExp(int starLvl)
{
    if (starLvl <= 0 || starLvl > mNeedExp.size())
    {
        return INT_MAX;
    }
    
    int exp = mNeedExp[starLvl - 1];
    return exp;
}

//获取强化所需总经验
int StarCostCfgDef::getTotalExp(int starLvl)
{
    if (starLvl <= 0 || starLvl > mNeedExp.size())
    {
        return false;
    }
    
    int exp = 0;
    for (int i = 0; i < starLvl; i++)
    {
        exp += mNeedExp[i];
    }
    
    return exp;
}
float StarCostCfgDef::getEffect(int starLvl)
{
    if (starLvl <= 0 || starLvl > effects.size())
    {
        return 1.0;
    }
    return  effects[starLvl - 1];
}


vector<int> StarCfgNew::sExps;
vector<float> StarCfgNew::sPros;
//vector<StarCfgDefNew> StarCfgNew::cfg;
StarCfgDefNew StarCfgNew::invalidcfg;
map<StarCfgNew::StarCostKey, StarCostCfgDef*, StarCfgNew::StarCostKeyCmp> StarCfgNew::costCfg;
vector<string>StarCfgNew::sTips;

bool StarCfgNew::StarCostKeyCmp::operator()(const StarCfgNew::StarCostKey& key1, const StarCfgNew::StarCostKey& key2) const
{
    int value1[] = {key1.lvl, key1.part, key1.qua};
    int value2[] = {key2.lvl, key2.part, key2.qua};
    
    for (int i = 0; i < 3; i++)
    {
        if (value1[i] < value2[i])
        {
            return true;
        }
        else if (value1[i] > value2[i])
        {
            return false;
        }
    }
    
    return false;
}


void StarCfgNew::load(std::string fullPath, string costPath)
{
    GameInifile starCfg(costPath);
    
    
    // char buf[16];
    
    string buf;
   // cfg.clear();
    
    invalidcfg.setEffect(1.0);
    //invalidcfg.gold = INT_MAX;
    //invalidcfg.reqlvl = MAX_ROLE_LVL + 1;
    
   // string effects = starCfg.getValue("common", "effects");
    string exps = starCfg.getValue("common", "exp");
    string pros = starCfg.getValue("common", "pro");
    string tips_id = starCfg.getValue("common", "tips_id");
    
   // vector<string> effectArray = StrSpilt(effects, ";");
    vector<string> expArray = StrSpilt(exps, ";");
    vector<string> proArray = StrSpilt(pros, ";");
    
//    for (int i = 0; i < effectArray.size(); i++)
//    {
//        StarCfgDefNew starcfg;
//        starcfg.setEffect(atof(effectArray[i].c_str()));
//        cfg.push_back(starcfg);
//    }
    
    assert(expArray.size() == proArray.size() && expArray.size() > 0);
    int totalPro = 0;
    vector<int> tempPro;
    for (int i = 0; i < expArray.size(); i++)
    {
        int exp = Utils::safe_atoi(expArray[i].c_str());
        int pro = atof(proArray[i].c_str());
        totalPro += pro;
        tempPro.push_back(pro);
        sExps.push_back(exp);
    }
    float proVal = 0.0f;
    for (int i = 0; i < expArray.size(); i++)
    {
        proVal += tempPro[i] / (float)totalPro;
        sPros.push_back(proVal);
    }
#ifdef CLIENT
    //lihengjin
    GameInifile mlFile;
    if(!(Cfgable::getFullFilePath("strengthen_costMultiLanguage.ini") == "strengthen_costMultiLanguage.ini"))
    {
        mlFile =  GameInifile(Cfgable::getFullFilePath("strengthen_costMultiLanguage.ini"));
    }
    //GameInifile mlFile(Cfgable::getFullFilePath("strengthen_costMultiLanguage.ini"));
    string tips;
    if(starCfg.exists("common", "tips"))
    {
        tips = starCfg.getValue("common", "tips");
    }else
    {
        tips= mlFile.getValue(tips_id, "desc");
    }
    //string tips = mlFile.getValue(tips_id, "desc");
    vector<string> tipArray = StrSpilt(tips, ";");
    assert(expArray.size() == tipArray.size());
    for (int i = 0; i < tipArray.size(); ++i) {
        sTips.push_back(tipArray[i]);
    }
#endif
    int typenum = starCfg.getValueT("common", "typenum", 0);
    for (int i = 0; i < typenum; i++)
    {
        string section = strFormat("type%d", i + 1);
        
        int maxlvl = starCfg.getValueT(section, "maxlvl", 0);
        int qua = starCfg.getValueT(section, "qua", 0);
        int lvl = starCfg.getValueT(section, "lvl", 0);
        string parts = starCfg.getValue(section, "part");
        float rate = starCfg.getValueT(section, "rate", 0.0f);
        string sgolds = starCfg.getValue(section, "sgold");
        string upggold = starCfg.getValue(section, "upgcost");
        string upgrmb = starCfg.getValue(section, "upgrmb");
        string needexps = starCfg.getValue(section, "needexp");
        string effects = starCfg.getValue(section, "effects");
        
        vector<string> goldArray = StrSpilt(upggold, ";");
        vector<string> rmbArray = StrSpilt(upgrmb, ";");
        vector<string> sgoldArray = StrSpilt(sgolds, ";");
        vector<string> expArray = StrSpilt(needexps, ";");
        vector<string> effectsArray = StrSpilt(effects,";");
        
        
        bool cfgError = false;
        string errStr;
        
        do
        {
            if (rate < 1.0)
            {
                cfgError = true;
                errStr = "star cost error at section " + section;
                break;
            }
            
            int goldSize = goldArray.size();
            int rmbSize = rmbArray.size();
            int sellSize = sgoldArray.size();
            if (goldSize < maxlvl || rmbSize < maxlvl || expArray.size() < maxlvl || sellSize < maxlvl)
            {
                cfgError = true;
                errStr = "star cost error: cost size or sgold size or need exp error";
                break;
            }
            
            StarCostCfgDef* costDef = new StarCostCfgDef;
            costDef->maxLvl = maxlvl;
            for (int i = 0; i < goldArray.size(); i++)
            {
                int upgGold = Utils::safe_atoi(goldArray[i].c_str(), INT_MAX);
                int upgRmb = Utils::safe_atoi(rmbArray[i].c_str(), INT_MAX);
                if (upgGold < 0 || upgGold == INT_MAX)
                {
                    cfgError = true;
                    errStr = "star cost error: cost gold config error";
                    break;
                }
                
                if (upgRmb < 0 || upgRmb == INT_MAX)
                {
                    cfgError = true;
                    errStr = "star cost error: cost rmb config error";
                    break;
                }
                
                int newUpgGold = upgGold * rate;
                if (newUpgGold < upgGold)
                {
                    cfgError = true;
                    errStr = "star cost error: cost gold rate config error";
                    break;
                }
                
                costDef->upgCost.push_back(newUpgGold);
                costDef->upgRmbCost.push_back(upgRmb);
            }
            
            for (int i = 0; i < sgoldArray.size(); i++)
            {
                int sgold = Utils::safe_atoi(sgoldArray[i].c_str(), INT_MAX);
                if (sgold < 0 || sgold == INT_MAX)
                {
                    cfgError = true;
                    errStr = "star cost error: sell gold array config error";
                    break;
                }
                
                int newSgold = sgold * rate;
                if (newSgold < 0)
                {
                    cfgError = true;
                    errStr = "star cost error: sell gold array config error";
                    break;
                }
                
                costDef->sellGold.push_back(newSgold);
            }
            
            for (int i = 0; i < expArray.size(); i++)
            {
                int needexp = Utils::safe_atoi(expArray[i].c_str(), INT_MAX);
                if (needexp < 0 || needexp == INT_MAX)
                {
                    cfgError = true;
                    errStr = "star cost error: need exp config error";
                    break;
                }
                
                costDef->mNeedExp.push_back(needexp);
            }
            
            for (int i=0; i< effectsArray.size(); i++)
            {
                float effect = atof(effectsArray[i].c_str());
                if (effect < 0 || (int) effect == INT_MAX)
                {
                    cfgError = true;
                    errStr = "star cost error: effect config error";
                    break;
                }
                
                costDef->effects.push_back(effect);
            }
            if (cfgError)
            {
                delete costDef;
                break;
            }
            
            vector<string> partArray = StrSpilt(parts, ";");
            for (int i = 0; i < partArray.size(); i++)
            {
                int part = Utils::safe_atoi(partArray[i].c_str(), -1);
                
                StarCostKey costKey;
                costKey.qua = qua;
                costKey.part = part;
                costKey.lvl = lvl;
                
                costCfg[costKey] = costDef;
            }
            
        }
        while (0);
        
        if (cfgError)
        {
            log_info(errStr);
        }
    }
}


int StarCfgNew::getRollExp(float roll)
{
    int i;
    for (i = 0; i < sPros.size() - 1; i++)
    {
        if (roll < sPros[i])
        {
            break;
        }
    }
    
    return sExps[i];
}

StarCostCfgDef* StarCfgNew::getCostCfg(int lvl, int qua, int part)
{
    StarCostKey costKey;
    costKey.lvl = lvl;
    costKey.qua = qua;
    costKey.part = part;
    
    map<StarCostKey, StarCostCfgDef*>::iterator iter = costCfg.find(costKey);
    if (iter == costCfg.end())
    {
        return NULL;
    }
    
    return iter->second;
}


int StarCfgNew::getMaxLvl(int lvl, int qua, int part)
{
    
    StarCostKey costKey;
    costKey.lvl = lvl;
    costKey.qua = qua;
    costKey.part = part;
    
    map<StarCostKey, StarCostCfgDef*>::iterator iter = costCfg.find(costKey);
    if (iter == costCfg.end())
    {
        return INT_MAX;
    }
    
    return iter->second->maxLvl;
    
}

bool StarCfgNew::getUpgCost(int starlvl, int lvl, int qua, int part, int& gold, int& rmb)
{
    StarCostKey costKey;
    costKey.lvl = lvl;
    costKey.qua = qua;
    costKey.part = part;
    
    gold = INT_MAX;
    rmb = INT_MAX;
    
    map<StarCostKey, StarCostCfgDef*>::iterator iter = costCfg.find(costKey);
    if (iter == costCfg.end())
    {
        return false;
    }
    
    bool ret = iter->second->getUpgCost(starlvl, gold, rmb);
    return ret;
}

int StarCfgNew::getSellGold(int starlvl, int lvl, int qua, int part)
{
    StarCostKey costKey;
    costKey.lvl = lvl;
    costKey.qua = qua;
    costKey.part = part;
    
    map<StarCostKey, StarCostCfgDef*>::iterator iter = costCfg.find(costKey);
    if (iter == costCfg.end())
    {
        return 0;
    }
    
    return iter->second->getSellGold(starlvl);
}

float StarCfgNew::getEffect(int starlvl, int lvl, int qua, int part)
{
    StarCostKey costKey;
    costKey.lvl = lvl;
    costKey.qua = qua;
    costKey.part = part;
    
    map<StarCostKey, StarCostCfgDef*>::iterator iter = costCfg.find(costKey);
    if (iter == costCfg.end())
    {
        return 0.0;
    }
    
    return iter->second->getEffect(starlvl);

}

//float StarCfgNew::getEffect(int starlvl)
//{
//    if (starlvl > cfg.size() || starlvl <= 0)
//    {
//        return 1.0;
//    }
//    
//    return cfg[starlvl - 1].getEffect();
//}


float EquipFusionDef::getEffect(int lvl)
{
    if(lvl <= 0 || lvl > maxLvl)
        return 1.0;
    return effects[lvl -1];
}
int EquipFusionDef::getNeedEquipsCount(int lvl)
{
    if(lvl <=0 || lvl > maxLvl)
        return INT_MAX;
    return needEquipsCount[lvl -1];
}
int EquipFusionDef::getGold(int lvl)
{
    if(lvl <=0 || lvl > maxLvl || costGold.size() == 0)
        return 0;
    return costGold[lvl -1];
}
int EquipFusionDef::getRMB(int lvl)
{
    if(lvl <=0 || lvl > maxLvl || costRMB.size() == 0)
        return 0;
    return costRMB[lvl -1];
}

int EquipFusionDef::getReturnGold(int lvl)
{
    if(lvl <=0 || lvl > maxLvl || returnGold.size() == 0)
        return 0;
    return returnGold[lvl -1];
}


map<EquipFusionCfg::FusionKey, EquipFusionDef*, EquipFusionCfg::FusionKeyCmp>EquipFusionCfg::costCfg;

bool EquipFusionCfg::FusionKeyCmp::operator()(const EquipFusionCfg::FusionKey& key1, const EquipFusionCfg::FusionKey& key2) const
{
    int value1[] = {key1.lvl, key1.part, key1.qua};
    int value2[] = {key2.lvl, key2.part, key2.qua};
    
    for (int i = 0; i < 3; i++)
    {
        if (value1[i] < value2[i])
        {
            return true;
        }
        else if (value1[i] > value2[i])
        {
            return false;
        }
    }
    
    return false;
}


void EquipFusionCfg::load(std::string fullPath)
{
    GameInifile blendCfg(fullPath);
    
    int typenum = blendCfg.getValueT("common", "typenum", 0);
    for (int i = 0; i < typenum; i++)
    {
        string section = strFormat("type%d", i + 1);
        
        int maxlvl = blendCfg.getValueT(section, "max_fuse_lvl", 0);
        string parts = blendCfg.getValue(section, "part");
        string quas = blendCfg.getValue(section, "qua");
        string lvls = blendCfg.getValue(section, "lvl");
        float rate = blendCfg.getValueT(section, "rate", 0.0f);
        string effect = blendCfg.getValue(section, "effects");
        string blendgold = blendCfg.getValue(section, "fuse_cost_gold");
        string blendrmb = blendCfg.getValue(section, "fuse_cost_rmb");
        string needEquipCount = blendCfg.getValue(section, "need_equip_num");
        string blendrgold = blendCfg.getValue(section, "fuse_sell_gold"); // 返还金币
        
        vector<string> effectArray = StrSpilt(effect, ";");
        vector<string> blendgoldArray = StrSpilt(blendgold, ";");
        vector<string> blendrmbArray = StrSpilt(blendrmb, ";");
        vector<string> needEquipCountArray = StrSpilt(needEquipCount, ";");
        vector<string> blendrgoldArray = StrSpilt(blendrgold, ";");  // 返还金币
        
        bool cfgError = false;
        string errStr;
        
        do
        {
            if (rate < 1.0)
            {
                cfgError = true;
                errStr = "star cost error at section " + section;
                break;
            }
            
            int goldSize = blendgoldArray.size();
            int rmbSize = blendrmbArray.size();
            int countSize = needEquipCountArray.size();
            int rgoldSize = blendrgoldArray.size();
            
            if (goldSize < maxlvl || rmbSize < maxlvl || countSize < maxlvl || rgoldSize < maxlvl)
            {
                cfgError = true;
                errStr = "star cost error: cost size or sgold size or need exp error";
                break;
            }
            
            EquipFusionDef* def = new EquipFusionDef;
            def->maxLvl = maxlvl;
            for(int i=0; i< effectArray.size(); i++)
            {
                float effect = atof(effectArray[i].c_str());
                def->effects.push_back(effect);
            }
            for (int i = 0; i < blendgoldArray.size(); i++)
            {
                int blendgold = Utils::safe_atoi(blendgoldArray[i].c_str(), 0);
                def->costGold.push_back(blendgold * rate);
               
            }
            
            for (int i = 0; i < blendrgoldArray.size(); ++i)
            {
                int rgold = Utils::safe_atoi(blendrgoldArray[i].c_str(), 0);
                def->returnGold.push_back(rgold * rate);
            }
            
            for (int i=0; i < blendrmbArray.size(); i++)
            {
                 int blendrmb = Utils::safe_atoi(blendrmbArray[i].c_str(), 0);
                 def->costRMB.push_back(blendrmb * rate);
            }
            for(int i = 0; i < needEquipCountArray.size(); i++)
            {
                int needEquipCount = Utils::safe_atoi(needEquipCountArray[i].c_str(), 0);
                def->needEquipsCount.push_back(needEquipCount);
            }
            
            vector<string> partArray = StrSpilt(parts, ";");
            vector<string> quaArray = StrSpilt(quas,";");
            vector<string> lvlArray = StrSpilt(lvls,";");
            for (int i = 0; i < lvlArray.size(); i++)
            {
                int lvl = Utils::safe_atoi(lvlArray[i].c_str());
                
                for(int j=0; j< quaArray.size(); j++)
                {
                    int qua = Utils::safe_atoi(quaArray[j].c_str());
                    
                    for(int k = 0; k< partArray.size(); k++)
                    {
                        int part =  Utils::safe_atoi(partArray[k].c_str(), -1);
                        
                        FusionKey key;
                        key.lvl = lvl;
                        key.qua = qua;
                        key.part = part;
                        costCfg[key] = def;
                    }
                }
               
            }
            
        }
        while (0);
        if (cfgError)
        {
            log_info(errStr);
        }
    }
}

int EquipFusionCfg::getMaxLvl(int lvl, int qua, int part)
{
    FusionKey key;
    key.lvl = lvl;
    key.qua = qua;
    key.part = part;
    
    map<FusionKey, EquipFusionDef *>::iterator iter = costCfg.find(key);
    if (iter == costCfg.end())
    {
        return 0;
    }
    
    return iter->second->maxLvl;
}
float EquipFusionCfg::getFusionEffect(int fusionlvl, int lvl, int qua, int part)
{
    FusionKey key;
    key.lvl = lvl;
    key.qua = qua;
    key.part = part;
    
    map<FusionKey, EquipFusionDef *>::iterator iter = costCfg.find(key);
    if (iter == costCfg.end())
    {
        return 0.0;
    }
    
    return iter->second->getEffect(fusionlvl);
}
int EquipFusionCfg::getNeedEquipCount(int fusionlvl, int lvl, int qua, int part)
{
    FusionKey key;
    key.lvl = lvl;
    key.qua = qua;
    key.part = part;
    
    map<FusionKey, EquipFusionDef *>::iterator iter = costCfg.find(key);
    if (iter == costCfg.end())
    {
        return INT_MAX;
    }
    
    return iter->second->getNeedEquipsCount(fusionlvl);
}
int EquipFusionCfg::getCostGold(int fusionlvl, int lvl, int qua, int part)
{
    FusionKey key;
    key.lvl = lvl;
    key.qua = qua;
    key.part = part;
    
    map<FusionKey, EquipFusionDef *>::iterator iter = costCfg.find(key);
    if (iter == costCfg.end())
    {
        return INT_MAX;
    }
    
    return iter->second->getGold(fusionlvl);
}
int EquipFusionCfg::getCostRMB(int fusionlvl, int lvl, int qua, int part)
{
    FusionKey key;
    key.lvl = lvl;
    key.qua = qua;
    key.part = part;
    
    map<FusionKey, EquipFusionDef *>::iterator iter = costCfg.find(key);
    if (iter == costCfg.end())
    {
        return INT_MAX;
    }
    
    return iter->second->getRMB(fusionlvl);
}


int EquipFusionCfg::getReturnGold(int fusionlvl, int lvl, int qua, int part)
{
    FusionKey key;
    key.lvl = lvl;
    key.qua = qua;
    key.part = part;
    
    map<FusionKey, EquipFusionDef *>::iterator iter = costCfg.find(key);
    if (iter == costCfg.end())
    {
        return 0;
    }
    
    return iter->second->getReturnGold(fusionlvl);
}



