//
//  PetNumericConvert.cpp
//  GameSrv
//
//  Created by nothing on 14-3-15.
//
//

#include "NumericConvert.h"
#include "Role.h"
#include "redis_helper.h"
#include "DataCfg.h"
#include "Pet.h"
#include "GameLog.h"
#include "process.h"
#include "EnumDef.h"

void doPetNumericConvert(Pet *pet,
                         const int &pet_mod,
                         int &pet_atk,          // 攻击力
                         int &pet_def,          //防御力
                         int &pet_hp,           //血量
                         float &pet_hit,        //命中
                         float &pet_dodge)      //闪避
{
    if (Process::env.getString("lang") != "zh_cn")
    {
        return ;
    }
    
    //0表示未转换，1表示已经转换
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "hget pet:%d ConvertMark", pet->petid);
    RedisResult mark(redisCmd(cmd));
    if (mark.readInt() == 1)
    {
        return ;
    }
    
    PetCfgDef *cfg = PetCfg::getCfg(pet_mod);
    if (cfg == NULL)
    {
        log_warn("没有找到对应petmod的PetCfgDef-->" << pet_mod);
        return ;
    }
    
    Xylog log2(eLogName_PetNumericConvert, 0);
    Xylog log1(eLogName_PetNumericConvert, 0);
    log1 << "旧：" << pet->petid << pet->getGrowth() << pet_atk << pet_def << pet_hp << pet_dodge << pet_hit;
    

    int pet_growth = range_rand(cfg->growth[0], cfg->growth[1]);
    pet_atk = range_rand(cfg->batk[0], cfg->batk[1]);
    pet_def = range_rand(cfg->bdef[0], cfg->bdef[1]);
    pet_hp = range_rand(cfg->bhp[0], cfg->bhp[1]);
    pet_dodge = range_randf(cfg->bdodge[0], cfg->bdodge[1]);
    pet_hit = range_randf(cfg->bhit[0], cfg->bhit[1]);
    
    pet->setGrowth(pet_growth);
    doRedisCmd("hmset pet:%d batk %d bdef %d bhp %d bdodge %f bhit %f ConvertMark 1",
               pet->petid, pet_atk, pet_def, pet_hp, pet_dodge, pet_hit);
    
    
    log2 << "新：" << pet->petid  << pet->getGrowth() << pet_atk << pet_def << pet_hp << pet_dodge << pet_hit;

}


void doBagItemToGold(Role *role, ItemGroup &item, bool *isNeedStore)
{
    if (item.item == 0)
    {
        return ;
    }
    ItemToGoldDef *def = ItemToGoldCfg::getItemToGoldDef(item.item);
    if (!def)
    {
        return ;
    }
    Xylog log(eLogName_BagItemToGold, role->getInstID());
    log << "ItemId" << item.item << item.count << "Gold" << def->compensation_gold * item.count;
    
    role->addGold(def->compensation_gold * item.count, "背包物品转换成金币");
    item = ItemGroup();
    if (isNeedStore)
    {
        *isNeedStore = true;
    }
}


void RoleEquipNumericConvert(Role *role)
{
    if (!role)
    {
        return ;
    }
    //背包指定物品换成金币
    itemToGold(role, role->getBackBag()->GetItems());
    //装备转换
    convertEquip(role, role->getPlayerEquip()->GetItems());
    //背包装备转换
    convertEquip(role, role->getBackBag()->GetItems());
    //保存
    saveRoleEquipAndBackBag(role);
}

void itemToGold(Role *role, ItemArray &items)
{
    ItemArray &bagItems = role->getBackBag()->GetItems();
    for (int i = 0; i < bagItems.size(); ++i)
    {
        doBagItemToGold(role, bagItems[i]);
    }
}

void saveRoleEquipAndBackBag(Role *role)
{
    //保存装备栏
    ItemArray &equipItems = role->getPlayerEquip()->GetItems();
    for (int i = 0; i < equipItems.size(); ++i)
    {
        role->getPlayerEquip()->Store(role->getInstID(), i);
    }
    
    //保存背包
    ItemArray &bagItems = role->getBackBag()->GetItems();
    for (int i = 0; i < bagItems.size(); ++i)
    {
        role->getBackBag()->Store(role->getInstID(), i);
    }
    
    for (int i = 0; i < equipItems.size(); ++i)
    {
        if (equipItems[i].item == 0)
        {
            doRedisCmd("hdel PlayerEquip:%d %d", role->getInstID(), i);
        }
    }
    
    for (int i = 0; i < bagItems.size(); ++i)
    {
        if (bagItems[i].item == 0)
        {
            doRedisCmd("hdel BackBag:%d %d", role->getInstID(), i);
        }
    }
}

void convertEquip(Role *role, ItemArray &items)
{
    if (!role)
    {
        return ;
    }
    for (int i = 0; i < items.size(); ++i)
    {
        int &itemid = items[i].item;
        string &json_ext = items[i].json;
        
        if (itemid == 0)
        {
            continue;
        }
        
        //先判断是否是装备
        ItemCfgDef *def = ItemCfg::getCfg(itemid);
        if (!def)
        {
            continue;
        }
        int type = def->ReadInt("type");
        if (type != kItemEquip)
        {
            continue;
        }
        
        //根据配置表获取旧装备属性
        string name = def->ReadStr("name");
        int job = def->ReadInt("job");      //职业
        int part = def->ReadInt("part");    //装备位置
        int qua = def->ReadInt("qua");      //装备品质
        int isOld = def->ReadInt("is_initial", 0); //判断是否是新装备，0为新装备
        if (isOld == 0)
        {
            continue;
        }
        
        //得到人物等级
        int rolelvl = role->getLvl();
        
        //解析json
        int star = 0;
        Json::Reader reader;
        Json::Value jsonval;
        reader.parse(json_ext, jsonval);
        
        //得到武器星级
        if (!jsonval["star"].isNull())
        {
            star = jsonval["star"].asInt();
        }
        
        Xylog log2(eLogName_EquipNumericConvert, role->getInstID());
        Xylog log1(eLogName_EquipNumericConvert, role->getInstID());
        log1 << "旧" << rolelvl << itemid << name << star << qua << part << json_ext;
        
        
        //根据人物等级，武器品质颜色，星级--->武器新等级，新星级，品质
        int newEquipLvl = EquipChangeCfg::getNewEquipLvl(rolelvl, qua, star);
        int newEquipQua = EquipChangeCfg::getNewEquipQua(rolelvl, qua, star);
        int newEquipStar = EquipChangeCfg::getNewEquipStar(rolelvl, qua, star);
        if (newEquipLvl == 0 || newEquipQua == 0)
        {
            log_error("newEquipLvl 或者 newEquipQua为0！不应该出现这句话！" );
            continue ;
        }
        
        int newItemid = 0;
        if (part == 5)
        {
            newItemid = ItemCfg::getNewItemId(newEquipLvl, part, newEquipQua, job);
        }
        else
        {
            newItemid = ItemCfg::getNewItemId(newEquipLvl, part, newEquipQua, -1);
        }
        //没找到相对应的武器，不应该
        if (newItemid == 0)
        {
            log_error("没找到相对应的武器！不应该出现这句话！！配置表可能有问题。" );
            continue ;
        }
        
        //更换新的itemid
        
        Json::Value value;
        if (star != 0)
        {
            value["star"] = newEquipStar;
        }
        
        //旧装备有的镶嵌孔
        int holemum = def->ReadInt("holenum");
        map<string, int> stones;
        for (int i = 1; i <= holemum; i++)
        {
            char key[32];
            sprintf(key, "stone%d", i);
            int stoneid = jsonval[key].asInt();
            if (stoneid > 0)
            {
                stones[key] = stoneid;
            }
        }
        
        ItemCfgDef *newdef = ItemCfg::getCfg(newItemid);
        string newname = newdef->ReadStr("name");
        int newEquipPart = newdef->ReadInt("part");
        int newHolenum = newdef->ReadInt("holenum");
        
        //新孔个数大于宝石个数，则把宝石不变
        if (newHolenum >= stones.size())
        {
            int i = 1;
            for (map<string, int>::iterator iter = stones.begin(); iter != stones.end(); ++iter)
            {
                char key[32];
                sprintf(key, "stone%d", i++);
                value[key] = iter->second;
            }
            
            itemid = newItemid;
            string jsonstr = Json::FastWriter().write(value);
            json_ext = jsonstr == "null\n" ? "" : jsonstr;

            log2 << "新" << rolelvl << newItemid << newname << newEquipStar << newEquipQua << newEquipPart << json_ext;
        }
        else //宝石数大于孔数目，则少于背包个数的宝石放回背包,要是背包太满就放弃转换
        {
            ItemArray itemarray;
            GridArray effarray;
            
            int i = 1;
            stringstream ss;
            for (map<string, int>::iterator iter = stones.begin(); iter != stones.end(); ++iter)
            {
                if (i <= newHolenum)
                {
                    char key[32];
                    sprintf(key, "stone%d", i++);
                    value[key] = iter->second;
                }
                else
                {
                    itemarray.push_back(ItemGroup(iter->second, 1));
                    ss << iter->second << " ";
                }
            }
            
            if (CE_OK != role->preAddItems(itemarray, effarray))
            {
                log2 << "新" << rolelvl << newItemid << newname << "背包太满，放弃装备转换";
                continue ;
            }
            
            role->updateBackBag(effarray);
            
            string jsonstr = Json::FastWriter().write(value);
            json_ext = jsonstr == "null\n" ? "" : jsonstr;
            itemid = newItemid;
            
            log2 << "新" << rolelvl << newItemid << newname << newEquipStar << newEquipQua << newEquipPart << json_ext
            << "放背包" << ss.str();
        }
    }
}










