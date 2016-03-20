
//
//  Item.cpp
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#include "ItemHelper.h"
#include "DataCfg.h"
#include "json/json.h"
#include <cassert>
#include <algorithm>
#include "EnumDef.h"

#define ASSERT assert

#define GET_MIN(x, y) (x) > (y) ? (y) : (x)

#define CHECK_MIN(x, min) do { if (x < min) x = min;}while(0);
#define CHECK_MAX(x, max) do { if (x > max) x = max;}while(0);

void SplitStr(const char* str, char ch, StringArray& strarr)
{
    string substr;
    const char* pos = str;

    while (*pos)
    {
        if (*pos++ == ch)
        {
            strarr.push_back(substr.assign(str, pos - str - 1));
            str = pos;
        }
    }

    strarr.push_back(substr.assign(str, pos - str));
}


void mergeItems(const ItemArray& items, ItemArray& mergedItems)
{
    for (int i = 0; i < items.size(); i++)
    {
        bool merged = false;
        for (int j = 0; j < mergedItems.size(); j++)
        {
            if (items[i].item == mergedItems[j].item)
            {
                mergedItems[j].count += items[i].count;
                merged = true;
                break;
            }
        }

        if (!merged){
            mergedItems.push_back(items[i]);
        }
    }
}

#define PROP_EXP "exp"
#define PROP_GOLD "gold"
#define PROP_BATPOINT "batpoint"
#define PROP_CONSVAL "consval"
#define PROP_RMB "rmb"
#define PROP_PETEXP "petexp"
#define PROP_CONSTRIB "constrib"
#define PROP_EXPLOIT "exploit"
#define PROP_POINTS "petpvppoints"
#define PROP_FAT "fat"
#define PROP_PRESTIGE "prestige"
#define PROP_VIPEXP "vipexp"
#define PROP_ENCHANTDUST "enchantdust"
#define PROP_ENCHANTGOLD "enchantgold"
#define PROP_PVPPOINTS "pvppoints"

void reward2MailFormat(const RewardStruct& reward, const ItemArray& items, vector<string>* contents,
                       vector<string>* attaches,bool isClient)
{
    pair<int, const char*> value2types[] = {
        pair<int, const char*>(reward.reward_exp, PROP_EXP),
        pair<int, const char*>(reward.reward_gold, PROP_GOLD),
        pair<int, const char*>(reward.reward_batpoint, PROP_BATPOINT),
        pair<int, const char*>(reward.reward_consval, PROP_CONSVAL),
        pair<int, const char*>(reward.reward_rmb, PROP_RMB),
        pair<int, const char*>(reward.reward_petexp, PROP_PETEXP),
        pair<int, const char*>(reward.reward_constrib, PROP_CONSTRIB),
        pair<int, const char*>(reward.reward_exploit, PROP_EXPLOIT),
        pair<int, const char*>(reward.petpvp_points, PROP_POINTS),
        pair<int, const char*>(reward.reward_fat, PROP_FAT),
        pair<int, const char*>(reward.reward_prestige, PROP_PRESTIGE),
        pair<int, const char*>(reward.reward_vipexp, PROP_VIPEXP),
        pair<int, const char*>(reward.reward_enchantdust, PROP_ENCHANTDUST),
        pair<int, const char*>(reward.reward_enchantgold, PROP_ENCHANTGOLD),
        pair<int, const char *>(reward.reward_pvppoints, PROP_PVPPOINTS)
    };
    
    for (int i = 0; i < sizeof(value2types) / sizeof(value2types[0]); i++) {
        if (value2types[i].first <= 0) {
            continue;
        }
        
        if (contents) {
            string name = MoneyCfg::getName(value2types[i].second);
            string desc = "";
            if(isClient)
            {
                desc = strFormat("%d%s",value2types[i].first,name.c_str());
            }
            else
            {
                desc = strFormat("%s*%d", name.c_str(), value2types[i].first);
            }
            
            contents->push_back(desc);
        }
        
        if (attaches) {
            string attach = strFormat("%s %d*1", value2types[i].second, value2types[i].first);
            attaches->push_back(attach);
        }
    }
    
    for (int i = 0; i < items.size(); i++) {
        ItemCfgDef* cfg = ItemCfg::getCfg(items[i].item);
        if (cfg == NULL) {
            continue;
        }

        if (contents) {
            string name = cfg->ReadStr("name");
            string content = strFormat("%s*%d", name.c_str(), items[i].count);
            contents->push_back(content);
        }
        
        if (attaches) {
            string attach = strFormat("item %d*%d", items[i].item, items[i].count);
            attaches->push_back(attach);
        }
    }
}


void rewardsCmds2ItemArray(const std::vector<std::string>& rewards,ItemArray& items, RewardStruct& reward)
{
    ItemArray tempItems;
    //发放物品
    for (int i = 0; i < rewards.size(); i++)
    {
        int exp = 0;
        int gold = 0;
        int batpoint = 0;
        int consval = 0;
        int rmb = 0;
        int petexp = 0;
		int constrib = 0;
        int exploit = 0;
        int petpvpPoints = 0;
        int fat = 0;
        int prestige = 0;
        int vipexp = 0;
        int enchantdust = 0;
        int enchantgold = 0;
        int pvppoints = 0;
        
        ItemGroup item;
        if(sscanf(rewards[i].c_str(), "item %d*%d",&item.item,&item.count) == 2)
        {
            //不重复加入
            item.bind = 1;
            tempItems.push_back(item);
        }
        else if (sscanf(rewards[i].c_str(), "exp %d*%*d",&exp)==1||
                 sscanf(rewards[i].c_str(), "gold %d*%*d",&gold)==1 ||
                 sscanf(rewards[i].c_str(), "batpoint %d*%*d",&batpoint)==1 ||
                 sscanf(rewards[i].c_str(), "consval %d*%*d",&consval)==1 ||
                 sscanf(rewards[i].c_str(), "rmb %d*%*d",&rmb)==1 ||
                 sscanf(rewards[i].c_str(), "petexp %d*%*d", &petexp)==1 ||
				 sscanf(rewards[i].c_str(), "constrib %d*%*d", &constrib) == 1 ||
                 sscanf(rewards[i].c_str(), "exploit %d*%*d", &exploit) == 1 ||
                 sscanf(rewards[i].c_str(), "fat %d*%*d", &fat) == 1 ||
                 sscanf(rewards[i].c_str(), "petpvppoints %d*%*d", &petpvpPoints) == 1 ||
                 sscanf(rewards[i].c_str(), "prestige %d*%*d", &prestige) == 1 ||
                 sscanf(rewards[i].c_str(), "vipexp %d*%*d", &vipexp) == 1 ||
                 sscanf(rewards[i].c_str(), "enchantdust %d*%*d", &enchantdust) == 1 ||
                 sscanf(rewards[i].c_str(), "enchantgold %d*%*d", &enchantgold) == 1 ||
                 sscanf(rewards[i].c_str(), "pvppoints %d*%*d", &pvppoints) == 1
                 )
        {
            reward.reward_exp += exp;
            reward.reward_gold += gold;
            reward.reward_batpoint += batpoint;
            reward.reward_consval += consval;
            reward.reward_rmb += rmb;
            reward.reward_petexp += petexp;
			// 贡献值
			reward.reward_constrib += constrib;
            reward.reward_exploit += exploit;
            reward.petpvp_points += petpvpPoints;
            reward.reward_fat += fat;
            reward.reward_prestige += prestige;
            reward.reward_vipexp += vipexp;
            // 附魔尘、附魔钻
            reward.reward_enchantdust += enchantdust;
            reward.reward_enchantgold += enchantgold;
            reward.reward_pvppoints += pvppoints;
        }
    }

    mergeItems(tempItems, items);
}

/**
 *  邮件的附件解析
 */
extern void emailRewardsCmds2ItemArray(const std::vector<std::string>& rewards, ItemArray& items,
                                  int* reward_exp,
                                  int* reward_gold,
                                  int* reward_batpoint,
                                  int* reward_consval,
                                  int* reward_petexp,
                                  int* reward_rmb,
                                  int* reward_fat,
                                  int* reward_constrib,
                                  int* reward_exploit,
                                  int* reward_enchantdust,
                                  int* reward_enchantgold,
                                  int* reward_pvppoints,
                                  int* reward_cspvpoints
                                 )
{
    for (int i = 0; i < rewards.size(); i++) {
        int exp = 0;
        int gold = 0;
        int batpoint = 0;
        int consval = 0;
        int petexp = 0;
        int rmb = 0;
        int fat = 0;

        int constrib = 0;
        int exploit = 0;
        int enchantdust = 0;
        int enchantgold = 0;
        int pvppoints = 0;
        int cspvpoints = 0;

        ItemGroup item;
        if(sscanf(rewards[i].c_str(), "item %d*%d",&item.item, &item.count) == 2)
        {
            //不重复加入
            item.bind = 1;
            //不重复加入
            item.bind = 1;
            int j=0;
            for(; j < items.size(); j++)
            {
                if(items[j].item == item.item)
                {
                    items[j].count += item.count;
                    break;
                }
            }
            if(j == items.size())
            {
                items.push_back(item);
            }
        }
        else if (reward_exp && sscanf(rewards[i].c_str(), "exp %d*%*d",&exp) == 1)
        {
            *reward_exp += exp;
        }
        else if (reward_gold && sscanf(rewards[i].c_str(), "gold %d*%*d",&gold) == 1)
        {
            *reward_gold += gold;
        }
        else if (reward_batpoint && sscanf(rewards[i].c_str(), "batpoint %d*%*d",&batpoint) == 1)
        {
            *reward_batpoint += batpoint;
        }
        else if (reward_consval && sscanf(rewards[i].c_str(), "consval %d*%*d",&consval)==1)
        {
            *reward_consval += consval;
        }
        else if (reward_petexp && sscanf(rewards[i].c_str(), "petexp %d*%*d",&petexp)==1)
        {
            *reward_petexp += petexp;
        }
        else if(reward_rmb && sscanf(rewards[i].c_str(), "rmb %d*%*d",&rmb)==1)
        {
            *reward_rmb += rmb;
        }
        else if(reward_fat && sscanf(rewards[i].c_str(), "fat %d*%*d",&fat)==1)
        {
            *reward_fat += fat;
        }
        else if(reward_constrib && sscanf(rewards[i].c_str(), "constrib %d*%*d",&constrib)==1)
        {
            *reward_constrib += constrib;
        }
        else if(reward_exploit && sscanf(rewards[i].c_str(), "exploit %d*%*d",&exploit)==1)
        {
            *reward_exploit += exploit;
        }
        else if(reward_enchantdust && sscanf(rewards[i].c_str(), "enchantdust %d*%*d",&enchantdust)==1)
        {
            *reward_enchantdust += enchantdust;
        }
        else if(reward_enchantgold && sscanf(rewards[i].c_str(), "enchantgold %d*%*d",&enchantgold)==1)
        {
            *reward_enchantgold += enchantgold;
        }
        else if(reward_pvppoints && sscanf(rewards[i].c_str(), "pvppoints %d*%*d",&pvppoints)==1)
        {
            *reward_pvppoints += pvppoints;
        }
        else if(reward_pvppoints && sscanf(rewards[i].c_str(), "cspvpoints %d*%*d",&pvppoints)==1)
        {
            *reward_cspvpoints += cspvpoints;
        }
    }
}

extern void rewardsCmds2ItemArray(const std::vector<std::string>& rewards, ItemArray& items,
                                  int* reward_exp,
                                  int* reward_gold,
                                  int* reward_batpoint,
                                  int* reward_consval,
                                  int* reward_rmb,
                                  int* reward_fat)
{
    for (int i = 0; i < rewards.size(); i++) {
        int exp = 0;
        int gold = 0;
        int batpoint = 0;
        int consval = 0;
        int rmb = 0;
        int fat = 0;

        ItemGroup item;
        if(sscanf(rewards[i].c_str(), "item %d*%d",&item.item, &item.count) == 2)
        {
            //不重复加入
            item.bind = 1;
            //不重复加入
            item.bind = 1;
            int j=0;
            for(; j < items.size(); j++)
            {
                if(items[j].item == item.item)
                {
                    items[j].count += item.count;
                    break;
                }
            }
            if(j == items.size())
            {
                items.push_back(item);
            }
        }
        else if (reward_exp && sscanf(rewards[i].c_str(), "exp %d*%*d",&exp) == 1)
        {
            *reward_exp += exp;
        }
        else if (reward_gold && sscanf(rewards[i].c_str(), "gold %d*%*d",&gold) == 1)
        {
            *reward_gold += gold;
        }
        else if (reward_batpoint && sscanf(rewards[i].c_str(), "batpoint %d*%*d",&batpoint) == 1)
        {
            *reward_batpoint += batpoint;
        }
        else if (reward_consval && sscanf(rewards[i].c_str(), "consval %d*%*d",&consval)==1)
        {
            *reward_consval += consval;
        }
        else if(reward_rmb && sscanf(rewards[i].c_str(), "rmb %d*%*d",&rmb)==1)
        {
            *reward_rmb += rmb;
        }
        else if(reward_fat && sscanf(rewards[i].c_str(), "fat %d*%*d",&fat)==1)
        {
            *reward_fat += fat;
        }
    }
}






