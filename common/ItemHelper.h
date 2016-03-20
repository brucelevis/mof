//
//  Item.h
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//  背包和装备

#ifndef __ITEM_HELPER_INCLUDE_
#define __ITEM_HELPER_INCLUDE_

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include "Defines.h"
#include "Utils.h"
using namespace std;

class BattleProp;
class BaseProp;

typedef vector<string> StringArray;
void SplitStr(const char* str, char ch, StringArray& strarr);
//背包和装备的物品定义
struct ItemGroup
{
    static const char splitchar = ';';

    int item;//物品id
    int count;//数量
    int bind;//是否绑定
    std::string json; //装备扩展属性，json值，如：{"star":50, "stone1":317001}

    ItemGroup() : item(0), count(0), bind(1), json("")
    {
    }

    ItemGroup(int itemid, int itemcnt) : item(itemid), count(itemcnt), bind(1), json("")
    {
    }

	ItemGroup(int itemid, int itemcnt, const string &str) : item(itemid), count(itemcnt), bind(1), json(str)
    {
    }

    ItemGroup(const ItemGroup& src) :
        item(src.item),
        count(src.count),
        bind(src.bind),
        json(src.json)
    {
    }

    ItemGroup operator=(const ItemGroup& src)
    {
        if (this != &src)
        {
            item = src.item;
            count = src.count;
            bind = src.bind;
            json = src.json;
        }

        return *this;
    }

    string ToString() const
    {
        string buf = strFormat("%d%c%d%c%d%c%s", item, splitchar,
                    count, splitchar, bind, splitchar, json.c_str());
        return buf;
    }

    bool Parse(const char* str)
    {
		string itemStr = str;
		string splits[4];
		size_t startPos = 0;
		for (int i = 0; i < 3; i++){
			size_t pos = itemStr.find(splitchar, startPos);
			if (pos == string::npos){
				return false;
			}

			splits[i] = itemStr.substr(startPos, pos - startPos);

			startPos = pos + 1;
		}
		splits[3] = itemStr.substr(startPos);

        item = atoi(splits[0].c_str());
        count = atoi(splits[1].c_str());
        bind = atoi(splits[2].c_str());
        json = splits[3].c_str();

        return true;
    }
};

//装备详细信息
struct EquipInfo
{
    int id;
    string name;
    int index;
    ObjJob job;
    int qua;
    int part;
    int mode; //武器换装模板号
    string json;
#ifdef CLIENT
    ReadWrite_Crypt(int, mDef, Def);//防御
     ReadWrite_Crypt(int, mAtk, Atk);//攻击
     ReadWrite_Crypt(int, mMaxhp, Maxhp);//防御
     ReadWrite_Crypt(int, mMaxmp, Maxmp);//防御
     ReadWrite_Crypt(float, mHit, Hit);//命中
     ReadWrite_Crypt(float, mDodge, Dodge);//闪避
     ReadWrite_Crypt(float, mCri, Cri);
    
#else
    READWRITE(int, mDef, Def);//防御
    READWRITE(int, mAtk, Atk);//攻击
    READWRITE(int, mMaxhp, Maxhp);//防御
    READWRITE(int, mMaxmp, Maxmp);//防御
    READWRITE(float, mHit, Hit);//命中
    READWRITE(float, mDodge, Dodge);//闪避
    READWRITE(float, mCri, Cri);
#endif

    int price;

    int star;
    int bind;
    int stone1;
    int stone2;
    int stone3;
    int stone4;
    int stone5;

    int starExp;
    int fusionLvl;

    EquipInfo()
    {
        id = 0;
        name = "";
        price = 0;
        index = qua = part = mode = 0;
        setDef(0);
        setAtk(0);
        setMaxhp(0);
        setMaxmp(0);
        setHit(0.0);
        setDodge(0.0);
        setCri(0.0);
        star = bind = stone1 = stone2 = stone3 = stone4 = stone5 = fusionLvl = 0;
        starExp = 0;
        job = eAllJob;
        json="";
        
    }

    EquipInfo(const EquipInfo& info){
        id = info.id;
        name = info.name;
        price = info.price;
        index = info.index;
        job = info.job;
        qua = info.qua;
        part = info.part;
        
        setDef(info.getDef());
        setAtk(info.getAtk());
        
        setMaxhp(info.getMaxhp());
        setMaxmp(info.getMaxmp());
        setHit(info.getHit());
        
        setDodge(info.getDodge());
        setCri(info.getCri());
        star = info.star;
        bind = info.bind;
        stone1 = info.stone1;
        stone2 = info.stone2;
        stone3 = info.stone3;
        stone4 = info.stone4;
        stone5 = info.stone5;
        fusionLvl = info.fusionLvl;
        
        starExp = info.starExp;
        json = info.json;
    }

    const EquipInfo& operator=(const EquipInfo& info){

        id = info.id;
        name = info.name;
        price = info.price;
        index = info.index;
        job = info.job;
        qua = info.qua;
        part = info.part;
        
        setDef(info.getDef());
        setAtk(info.getAtk());
        
        setMaxhp(info.getMaxhp());
        setMaxmp(info.getMaxmp());
        setHit(info.getHit());
        
        setDodge(info.getDodge());
        setCri(info.getCri());

        star = info.star;
        bind = info.bind;
        stone1 = info.stone1;
        stone2 = info.stone2;
        stone3 = info.stone3;
        stone4 = info.stone4;
        stone5 = info.stone5;
        
        starExp = info.starExp;
        fusionLvl = info.fusionLvl;
        json = info.json;

        return *this;
    }
};

#ifdef CLIENT
void GetEquipInfo(const ItemGroup& item, EquipInfo& info);
#endif

struct BagGrid
{
    BagGrid(): itemType(0)
    {
        
    }
    int itemType;
    int index;
    ItemGroup item;
};

typedef vector<ItemGroup> ItemArray;
typedef vector<BagGrid>   GridArray;
//奖励命令转奖励数据结构

struct RewardStruct
    {
        RewardStruct()
        {
            reward_petexp = reward_gold = reward_exp = reward_consval =  reward_batpoint = reward_rmb =
            reward_constrib = reward_exploit = petpvp_points = reward_fat = reward_prestige = reward_vipexp = reward_enchantdust = reward_enchantgold = reward_pvppoints = 0;
        }

        RewardStruct operator+(const RewardStruct& src)
        {
            if (this != &src)
            {
                reward_exp += src.reward_exp;
                reward_gold += src.reward_gold;
                reward_batpoint += src.reward_batpoint;
                reward_consval += src.reward_consval;
                reward_rmb += src.reward_rmb;
                reward_petexp += src.reward_petexp;
                reward_constrib += src.reward_constrib;            // 公会个人贡献值
                reward_exploit += src.reward_exploit;             // 公会个人功勋
                petpvp_points += src.petpvp_points;
                reward_fat += src.reward_fat;
                reward_prestige += src.reward_prestige;
                reward_vipexp += src.reward_vipexp;
                reward_enchantdust += src.reward_enchantdust;
                reward_enchantgold += src.reward_enchantgold;
                reward_pvppoints += src.reward_pvppoints;
                
            }

            return *this;
        }
        
        RewardStruct& operator+=( const RewardStruct& src)
        {
            reward_exp += src.reward_exp;
            reward_gold += src.reward_gold;
            reward_batpoint += src.reward_batpoint;
            reward_consval += src.reward_consval;
            reward_rmb += src.reward_rmb;
            reward_petexp += src.reward_petexp;
            reward_constrib += src.reward_constrib;            // 公会个人贡献值
            reward_exploit += src.reward_exploit;             // 公会个人功勋
            petpvp_points += src.petpvp_points;
            reward_fat += src.reward_fat;
            reward_prestige += src.reward_prestige;
            reward_vipexp += src.reward_vipexp;
            reward_enchantdust += src.reward_enchantdust;
            reward_enchantgold += src.reward_enchantgold;
            reward_pvppoints += src.reward_pvppoints;
            
            return *this;
        }
        
        RewardStruct operator=(const RewardStruct& src)
        {
            if (this != &src)
            {
                reward_exp = src.reward_exp;
                reward_gold = src.reward_gold;
                reward_batpoint = src.reward_batpoint;
                reward_consval = src.reward_consval;
                reward_rmb = src.reward_rmb;
                reward_petexp = src.reward_petexp;
                reward_constrib = src.reward_constrib;            // 公会个人贡献值
                reward_exploit = src.reward_exploit;             // 公会个人功勋
                petpvp_points = src.petpvp_points;
                reward_fat = src.reward_fat;
                reward_prestige = src.reward_prestige;
                reward_vipexp = src.reward_vipexp;
                reward_enchantdust = src.reward_enchantdust;
                reward_enchantgold = src.reward_enchantgold;
                reward_pvppoints = src.reward_pvppoints;
            }
            
            return *this;
        }
        
        RewardStruct& operator*=(float rate)
        {
            reward_exp = reward_exp * rate;
            reward_gold = reward_gold * rate;
            reward_batpoint = reward_batpoint * rate;
            reward_consval = reward_consval * rate;
            reward_rmb = reward_rmb * rate;
            reward_petexp = reward_petexp * rate;
            reward_constrib = reward_constrib * rate;            // 公会个人贡献值
            reward_exploit = reward_exploit * rate;             // 公会个人功勋
            petpvp_points = petpvp_points * rate;
            reward_fat = reward_fat * rate;
            reward_prestige = reward_prestige * rate;
            reward_vipexp = reward_vipexp * rate;
            reward_enchantdust = reward_enchantdust * rate;
            reward_enchantgold = reward_enchantgold * rate;
            reward_pvppoints = reward_pvppoints * rate;
            
            return *this;
        }
        
        int reward_exp;
        int reward_gold;
        int reward_batpoint;
        int reward_consval;
        int reward_rmb;
        int reward_petexp;
		int reward_constrib;            //公会个人贡献值
        int reward_exploit;             //公会个人功勋
        int petpvp_points;              //幻兽竞技场积分
        int reward_fat;              //精力
        int reward_prestige;        //声望
        int reward_vipexp; //vip exp
        int reward_enchantdust;     // 附魔尘
        int reward_enchantgold;     // 附魔钻
        int reward_pvppoints;       //异步PVP积分
    };

extern void rewardsCmds2ItemArray(const std::vector<std::string>& cmds,ItemArray& items, RewardStruct& rewards);

extern void rewardsCmds2ItemArray(const std::vector<std::string>& cmds,ItemArray& items,
                                  int* reward_exp = NULL,
                                  int* reward_gold = NULL,
                                  int* reward_batpoint = NULL,
                                  int* reward_consval = NULL,
                                  int* reward_rmb = NULL,
                                  int* reward_fat = NULL
                                  );
extern void emailRewardsCmds2ItemArray(const std::vector<std::string>& rewards, ItemArray& items,
                                        int* reward_exp = NULL,
                                        int* reward_gold = NULL,
                                        int* reward_batpoint = NULL,
                                        int* reward_consval = NULL,
                                        int* reward_petexp = NULL,
                                        int* reward_rmb = NULL,
                                        int* reward_fat = NULL,
                                        int* reward_constrib = NULL,
                                        int* reward_exploit= NULL,
                                        int* reward_enchantdust = NULL,
                                        int* reward_enchantgold = NULL,
                                        int* reward_pvppoints = NULL,
                                        int* reward_cspvpoints = NULL
                                       );
    
extern void reward2MailFormat(const RewardStruct& rewards, const ItemArray& items, vector<string>* contents,
                              vector<string>* attaches,bool isClient = false);

//把ID相同的合并到一齐
void mergeItems(const ItemArray& items, ItemArray& mergedItems);


#endif /* defined(__GameSrv__Item__) */
