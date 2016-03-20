//
//  DataCfgEquip.h
//  GameSrv
//
//  Created by prcv on 14-1-19.
//
//

#ifndef __GameSrv__DataCfgEquip__
#define __GameSrv__DataCfgEquip__

#include <iostream>
#include "Defines.h"
#include <vector>
#include <map>
using namespace std;


/*
 -------------------------------------------------------------------------------
 装备升星
 */

//装备星级属性配置
struct StarCfgDef
{
    int gold;               //升星所需金币
    int reqlvl;             //升星所需人物等级
    int intensifyStones;    //升星所需强化石
#ifdef CLIENT
    ReadWrite_Crypt(float, mEffect, Effect);//升星效果
#else
    READWRITE(float, mEffect, Effect);
#endif
    StarCfgDef()
    {
        setEffect(0);
    }
};

class StarCfg
{
public:
    StarCfg(){}
    virtual ~StarCfg(){}
    
    static void load(std::string fullPath);
    
    static bool getCfg(int starlvl, StarCfgDef& def);
    static int getReqLvl(int starlvl);
    static int getReqGold(int starlvl);
    static int getReqIntensifyStones(int starlvl);
    
    static float getEffect(int starlvl);
    
    STATICREADWRITE(int , mIntensifyStoneId , IntensifyStoneId)   //强化石id    
private:
    static vector<StarCfgDef> cfg;
    static StarCfgDef invalidcfg;
    static int MAX_STAR_LVL;
};

struct StarCostCfgDef
{
    ///------------
    // 强化出售价格相关
    ///------------
    //强化出售返回
    vector<int> sellGold;
    //获取出售价格
    int getSellGold(int starLvl);
    
    ///------------
    // 强化消耗相关
    ///------------
    //最大强化等级
    int maxLvl;
    //强化金币消耗
    vector<int> upgCost;
    vector<int> upgRmbCost;
    int getMaxLvl();
    bool getUpgCost(int starLvl, int& gold, int& rmb);
    
    ///------------
    // 强化经验相关
    ///------------
    //强化等级所需经验
    vector<int> mNeedExp;
    //获取强化所需经验
    int getExp(int starLvl);
    //获取强化所需总经验
    int getTotalExp(int starLvl);
    
    //强化加成效果相关
    vector<float> effects;
    float getEffect(int starLvl);
};

//装备星级属性配置
struct StarCfgDefNew
{
#ifdef CLIENT
    ReadWrite_Crypt(float, mEffect, Effect);//升星效果
#else
    READWRITE(float, mEffect, Effect);
#endif
    StarCfgDefNew()
    {
        setEffect(0);
    }
};

class StarCfgNew
{
public:
    StarCfgNew(){}
    virtual ~StarCfgNew(){}
    
    static void load(std::string fullPath, string costPath);
    
    static StarCostCfgDef* getCostCfg(int lvl, int qua, int part);
   // static float getEffect(int starlvl);
    
    
    static int getMaxLvl(int lvl, int qua, int part);
    static bool getUpgCost(int starlvl, int lvl, int qua, int part, int& gold, int& rmb);
    static int getSellGold(int starlvl, int lvl, int qua, int part);
    static float getEffect(int starlvl, int lvl, int qua, int part);
    
    //获取经验，roll值为[0，1]
    static int getRollExp(float roll);
    
    struct StarCostKey
    {
        int lvl;
        int part;
        int qua;
    };
    
    struct StarCostKeyCmp
    {
        bool operator()(const StarCostKey& key1, const StarCostKey& key2) const;
    };
    //强化获得的经验层次
    static vector<int> sExps;
    //强化层次的概率
    static vector<float> sPros;
    // 提示 
    static vector<string> sTips;
private:
    
    
    static map<StarCostKey, StarCostCfgDef*, StarCostKeyCmp> costCfg;
    
  //  static vector<StarCfgDefNew> cfg;
    static StarCfgDefNew invalidcfg;
   
};

#pragma mark- 装备融合
struct EquipFusionDef
{
    int maxLvl;
    vector<float> effects;
    vector<int> needEquipsCount;
    vector<int> costGold;
    vector<int> costRMB;
    vector<int> returnGold; // 提品或卖物品，有星级的话返还部分金币
    float getEffect(int lvl);
    int getNeedEquipsCount(int lvl);
    int getGold(int lvl);
    int getRMB(int lvl);
    int getReturnGold(int lvl);
};
class EquipFusionCfg
{
    
public:
    EquipFusionCfg(){}
    virtual ~EquipFusionCfg(){}
    
    static void load(std::string fullPath);
    
    static EquipFusionDef* getCostCfg(int lvl, int qua, int part);

    static int getMaxLvl(int lvl, int qua, int part);
    static float getFusionEffect(int fusionlvl, int lvl, int qua, int part);
    static int getNeedEquipCount(int fusionlvl, int lvl, int qua, int part);
    static int getCostGold(int fusionlvl, int lvl, int qua, int part);
    static int getCostRMB(int fusionlvl, int lvl, int qua, int part);
    static int getReturnGold(int fusionlvl, int lvl, int qua, int part);
    
    struct FusionKey
    {
        int lvl;
        int part;
        int qua;
    };
    struct FusionKeyCmp
    {
        bool operator()(const FusionKey& key1, const FusionKey& key2) const;
    };
private:

     static map<FusionKey, EquipFusionDef*, FusionKeyCmp> costCfg;
};



#endif /* defined(__GameSrv__DataCfgEquip__) */
