//
//  Item.h
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//  背包和装备

#ifndef __ITEM_BAG_INCLUDE_
#define __ITEM_BAG_INCLUDE_

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include "Defines.h"
#include "Utils.h"
#include "ItemHelper.h"
using namespace std;

class Role;

//物品容器
class ItemContainer
{
protected:
    ItemArray items;
    int       capacity;

public:
    ItemContainer(int size) : capacity(size)
    {
        Clear();
    }
    virtual ~ItemContainer(){}

    void Clear()
    {
        items.resize(capacity);
        ItemGroup item;
        for (int i = 0; i < capacity; i++)
        {
            items[i] = item;
        }
    }
    
    void extendTo(int newSize)
    {
        if (newSize <= capacity) {
            return;
        }
        
        int prevCapa = capacity;
        capacity = newSize;
        items.resize(newSize);
        for (int i = prevCapa; i < capacity; i++) {
            ItemGroup item;
            items[i] = item;
        }
    }
    
    int GetCapacity() {return capacity;}
    ItemArray& GetItems() {return items;}
    ItemGroup&  GetItem(int index) {return items[index];}
    void SetItem(int index, const ItemGroup& item) {items[index] = item;}

    virtual void Load(int roleid);
    virtual void Store(int roleid, int index);
    virtual void Store(int roleid, vector<int>& indices);

    virtual void Store(int roleid, const GridArray& grids);
    virtual void Store(int roleid, int index, const ItemGroup& item);
    virtual const char* GetName() = 0;
};

class BackBag : public ItemContainer
{
public:

    BackBag(int bagsizelimit);
    ~BackBag();

    //预添加物品，如果背包有足够的格子放置物品，返回true
    //items：要添加的物品
    //effgrids：添加物品影响到的格子信息
    bool PreAddItems(const ItemArray& items, GridArray& effgrids);

    //预删除物品，如果背包有足够的物品，返回true
    //items：要删除的物品
    //effgrids：删除物品影响到的格子信息
    bool PreDelItems(const ItemArray& items, GridArray& effgrids);

    //更新背包
    //effgrids：要更新的格子信息
    void UpdateBackBag(const GridArray& effgrids);

    //更新背包
    //index：背包的格子索引
    //item：物品信息
    void UpdateBackBag(int index, const ItemGroup& item);

    //更新背包
    //effgrids：要更新的格子信息
    //indexs:发生变化的格子索引
    void UpdateBackBag(const GridArray& effgrids, std::vector<int>& indexs);

    //获取背包中物品数量
    int GetItemNum(int itemid);

    //获取某种类型的物品数量，
    //type：物品类型，0-8 装备，消耗品......
    void GetTypeItems(int type, ItemArray& items);

    //余下空格子数量
    int EmptyGridAmount();
    
    //安全删除道具，自动处理id重复情况
    bool SafePreDelItem(const std::vector<int>& items , GridArray& effgrids);

    //整理背包
    //effgrids：受影响的格子信息
    virtual void Sort(vector<int>& effgrids);
    
    //
    virtual const char* GetName() {return "BackBag";}

public:
    static BackBag* Create(int size);
    static const int sFreeBackbagSize;
    static const int sBackbagSizePerPage;
};

class PlayerEquip : public ItemContainer
{
    // 装备最低品质（装备未共鸣时，值为0）
    READONLY(int, mQuality, Quality);
    // 装备最低星级（装备未共鸣时，值为0）
    READONLY(int, mDeepen, Deepen);
public:
    PlayerEquip(int size);
    ~PlayerEquip();

    const char* GetName() {return "PlayerEquip";}
#ifdef CLIENT
    EquipInfo GetEquipInfo(int index);
    void      setEquipStar(int equipid , int star);   //设置装备的星级
#endif
    void CalcProperty(BattleProp& batprop, BaseProp& baseprop);

	// 计算装备镶嵌达到等级的宝石数量
	int GetStoneNumByLvl(int lvl);
public:
    static PlayerEquip* Create(int size);


    //穿装备
    //roleid：角色id
    //index：装备所在的背包格子号
    //errercode：错误码
    //成功返回true
    static bool PutOnEquip(int roleid, int index, int& errorcode);

    //卸下装备
    //roleid：角色id
    //index：装备所在的装备栏格子号
    //errercode：错误码
    //成功返回true
    static bool TakeOffEquip(int roleid, int index, int& errorcode);

    //直接设置装备栏的装备
    //roleid：角色id
    //index：装备所在的装备栏格子号
    //item：装备信息
    static void UpdatePlayerEquip(int roleid, int index, const ItemGroup& item);
    
    static void UpdatePlayerEquip(Role* role, int index, const ItemGroup& item);
    
    void calculateEquipResonance();// 只用来计算装备共鸣的级数
	
	// 修改装备强化星级等级
	static void AddEquipStarLvl(ItemGroup &item, int lvl);
private:
    // 装备共鸣加成
    void EquipResonance(BattleProp& batprop, BaseProp& baseprop);
    
};

//装材料的地方，类似背包，所以
class MaterialBoxForFashion: public BackBag
{
public:
    
    MaterialBoxForFashion(int size);
    
    static MaterialBoxForFashion* Create(int size);
    
    //整理
    //effgrids：受影响的格子信息
    virtual void Sort(vector<int>& effgrids);
    
    virtual const char* GetName(){return "FashionMaterialBox";}
private:
    
};

class RetinueBag: public BackBag
{
public:
    
    RetinueBag(int size);

    static RetinueBag* Create(int size);
    
    //整理
    //effgrids：受影响的格子信息
    virtual void Sort(vector<int>& effgrids);
    
    virtual const char* GetName(){return "RetinueBag";}
private:
    
};

#endif /* defined(__GameSrv__Item__) */
