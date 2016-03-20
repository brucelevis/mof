
//
//  Item.cpp
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#include "ItemBag.h"
#include "DataCfg.h"
#include "Role.h"
#include "json/json.h"
#include <cassert>
#include <algorithm>
#include "EnumDef.h"
#include "main.h"
#include "PlayerEquip.h"

#define ASSERT assert

#define GET_MIN(x, y) (x) > (y) ? (y) : (x)

#define CHECK_MIN(x, min) do { if (x < min) x = min;}while(0);
#define CHECK_MAX(x, max) do { if (x > max) x = max;}while(0);

struct ItemCompair
{
    bool operator()(const ItemGroup& a, const ItemGroup& b)
    {
        return a.item > b.item ? true : a.item == b.item ? a.count > b.count ? true : false : false;
    }
};

extern void SendBackBagNotify(Role* role, const GridArray& grids);
extern void SendBackBagNotify(Role* role, int index, const ItemGroup& equip);
extern void SendBackBagNotify(Role* role, std::vector<int>& indexs);
extern void SendPlayerEquipNotify(Role* role, const GridArray& grids);
extern void SendPlayerEquipNotify(Role* role, int index, const ItemGroup& equip);
extern void sendRetinueBagNotify(Role* role, const GridArray& grids);

static bool CheckStackItem(const ItemGroup& item1, const ItemGroup& item2)
{
    return item1.item == item2.item && item1.bind == item2.bind;
}

static bool StackItem(ItemGroup& item1, const ItemGroup& item2)
{
    if (item1.item == item2.item && item1.bind == item2.bind)
    {
        item1.count += item2.count;
        return true;
    }

    return true;
}

const int BackBag::sFreeBackbagSize = 140;
const int BackBag::sBackbagSizePerPage = 20;

BackBag::BackBag(int bagsizelimit) : ItemContainer(bagsizelimit)
{
}

BackBag::~BackBag()
{

}

BackBag* BackBag::Create(int size)
{
    return new BackBag(size);
}


int BackBag::GetItemNum(int itemid)
{
    int num = 0;
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i].item == itemid)
        {
            num += items[i].count;
        }
    }

    return num;
}


void BackBag::GetTypeItems(int type, ItemArray& typeitems)
{
    for (int i = 0; i < items.size(); i++)
    {
        ItemCfgDef* itemcfg = ItemCfg::getCfg(items[i].item);
        if (!itemcfg || itemcfg->ReadInt("type") != type)
        {
            continue;
        }

        bool isstack = false;
        for (int j = 0; j < typeitems.size(); j++)
        {
            if (typeitems[j].item == items[i].item)
            {
                typeitems[j].count += items[i].count;
                isstack = true;
            }
        }

        if (!isstack)
        {
            typeitems.push_back(items[i]);
        }
    }
}


int BackBag::EmptyGridAmount()
{
    int ret = 0;
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].item <= 0)
        {
            ++ret;
        }
    }
    return ret;
}

bool BackBag::PreAddItems(const ItemArray& newitems, GridArray& effgrids)
{
    int emptyindex = 0;

    for (int i = 0; i < newitems.size(); i++)
    {
        ItemCfgDef* itemcfg = ItemCfg::getCfg(newitems[i].item);
        if (itemcfg == NULL)
        {
            //背包不添加不存在的物品
            continue;
        }

        int remaincount = newitems[i].count;
        int itemstack = itemcfg->ReadInt("stack");
        int itemType = itemcfg->ReadInt("type");
        CHECK_MIN(itemstack, 1);


        //监测可以堆叠的物品格子
        for (int j = 0; j < items.size(); j++)
        {
            if (remaincount <= 0)
            {
                break;
            }

            if (items[j].item <= 0)
            {
                continue;
            }

            int gridcount = items[j].count;
            if (newitems[i].item != items[j].item || gridcount >= itemstack)
            {
                continue;
            }

            BagGrid newgrid;
            newgrid.itemType = itemType;
            newgrid.index = j;
            newgrid.item = items[j];
            int effnum = GET_MIN(itemstack - gridcount, remaincount);
            newgrid.item.count += effnum;
            effgrids.push_back(newgrid);
            remaincount -= effnum;
        }

        //监测空格子
        for (; emptyindex < items.size(); emptyindex++)
        {
            if (remaincount <= 0)
            {
                break;
            }

            if (items[emptyindex].item > 0)
            {
                continue;
            }

            int griditem = items[emptyindex].item;
            if (griditem > 0)
            {
                continue;
            }

            BagGrid newgrid;
            newgrid.item = newitems[i];
            newgrid.index = emptyindex;
            int effnum = GET_MIN(itemstack, remaincount);
            newgrid.item.count = effnum;
            newgrid.itemType = itemType;
            effgrids.push_back(newgrid);
            remaincount -= effnum;
        }

        if (remaincount > 0)
        {
            //背包放不下所有物品
            return false;
        }
    }

    return true;
}


bool BackBag::PreDelItems(const ItemArray& newitems, GridArray& effgrids)
{
    for (int i = 0; i < newitems.size(); i++)
    {
        int remaincount = newitems[i].count;
        ItemCfgDef* itemCfg = ItemCfg::getCfg(newitems[i].item);
        if (itemCfg == NULL) {
            continue;
        }
        
        int itemType = itemCfg->ReadInt("type");
        
        for (int j = 0; j < items.size(); j++)
        {
            if (items[j].item == newitems[i].item)
            {
                BagGrid newgrid;
                newgrid.item = newitems[i];
                newgrid.index = j;
                int effnum = GET_MIN(remaincount, items[j].count);
                newgrid.item.count = items[j].count - effnum;
                newgrid.itemType = itemType;
                if (newgrid.item.count == 0)
                {
                    newgrid.item = ItemGroup();
                }
                effgrids.push_back(newgrid);
                remaincount -= effnum;
                if (remaincount == 0)
                {
                    break;
                }
            }
        }

        if (remaincount > 0)
        {
            //背包内物品不够
            return false;
        }
    }

    return true;
}

void BackBag::UpdateBackBag(const GridArray& effgrids)
{
    for (int i = 0; i < effgrids.size(); i++)
    {
        SetItem(effgrids[i].index, effgrids[i].item);
    }
}


void BackBag::UpdateBackBag(int index, const ItemGroup& item)
{
    SetItem(index, item);
}

void BackBag::UpdateBackBag(const GridArray &effgrids, std::vector<int> &indexs)
{
    for (int i = 0; i < effgrids.size(); i++)
    {
        bool newIndex = true;
        SetItem(effgrids[i].index, effgrids[i].item);
        for (int j = 0; j < indexs.size(); j++) {
            if (indexs[j] == effgrids[i].index) {
                newIndex = false;
                break;
            }
        }
        if (newIndex) {
            indexs.push_back(effgrids[i].index);
        }
    }
}

void BackBag::Sort(vector<int>& effgrids)
{
    ItemArray sortitems;
    vector<int> prevgrids;

    for (int i = 0; i < items.size(); i++)
    {
        if (items[i].item == 0)
        {
            continue;
        }

        int stacknum = 1;
        ItemCfgDef* itemcfg = ItemCfg::getCfg(items[i].item);
        if (itemcfg)
        {
            stacknum = itemcfg->ReadInt("stack");
        }
        CHECK_MIN(stacknum, 1);

        int remainnum = items[i].count;
        for (int j = 0; j < sortitems.size(); j++)
        {
            if (items[i].item != sortitems[j].item)
            {
                continue;
            }

            if (remainnum <= 0)
            {
                break;
            }

            if (sortitems[j].count < stacknum)
            {
                int effnum = GET_MIN(remainnum, stacknum - sortitems[j].count);
                sortitems[j].count += effnum;
                remainnum -= effnum;
                break;
            }
        }

        while (remainnum > 0)
        {
            ItemGroup item = items[i];
            item.count = GET_MIN(remainnum, stacknum);
            sortitems.push_back(item);
            remainnum -= stacknum;
        }

        prevgrids.push_back(i);
    }

    if (sortitems.size() > GetCapacity())
    {
        return;
    }

    std::sort(sortitems.begin(), sortitems.end(), ItemCompair());
    std::copy(sortitems.begin(), sortitems.end(), items.begin());

    for (int i = 0; i < sortitems.size(); i++)
    {
        effgrids.push_back(i);
    }

    for (int i = 0; i < prevgrids.size(); i++)
    {
        if (prevgrids[i] >= sortitems.size())
        {
            items[prevgrids[i]] = ItemGroup();
            effgrids.push_back(prevgrids[i]);
        }
    }
}

bool BackBag::SafePreDelItem(const std::vector<int>& items , GridArray& effgrids)
{
    ItemArray materail,merged;

    for (int i = 0; i< items.size(); ++i)
    {
        ItemGroup itemgroup;
        itemgroup.item = items[i];
        itemgroup.count = 1;
        materail.push_back(itemgroup);
    }

    //因为PreDelItems 不能有重复的
    mergeItems(materail, merged);

    return PreDelItems( merged, effgrids);
}

void ItemContainer::Load(int roleid)
{
    char cmd[64];
    sprintf(cmd, "hgetall %s:%d", GetName(), roleid);

    redisReply* rreply = redisCmd(cmd);
    for (int i = 0; i < rreply->elements / 2; i++)
    {
        int index = Utils::safe_atoi(rreply->element[i * 2]->str);
        ItemGroup item;
        item.Parse(rreply->element[i * 2 + 1]->str);
        if (index > items.size())
        {
            continue;
        }

        items[index] = item;
    }
    freeReplyObject(rreply);
}

void ItemContainer::Store(int roleid, const GridArray& grids)
{
    if (grids.size() <= 0) {
        return;
    }
    
    char setname[128];
    sprintf(setname, "%s:%d", GetName(), roleid);

    RedisArgs args;
    RedisHelper::beginHmset(args, setname);
    for (int i = 0; i < grids.size(); i++)
    {
        RedisHelper::appendHmset(args, Utils::itoa(grids[i].index).c_str(), grids[i].item.ToString().c_str());
    }
    RedisHelper::commitHmset(get_DbContext(), args);
}


void ItemContainer::Store(int roleid, int index, const ItemGroup& item)
{
    doRedisCmd("hset %s:%d %d %s", GetName(), roleid, index, item.ToString().c_str());
}

void ItemContainer::Store(int roleid, int index)
{
    if (index >= capacity)
    {
        return;
    }

    Store(roleid, index, items[index]);
}


void ItemContainer::Store(int roleid, vector<int>& indices)
{
    char setname[128];
    sprintf(setname, "%s:%d", GetName(), roleid);

    RedisArgs args;
    RedisHelper::beginHmset(args, setname);
    for (int i = 0; i < indices.size(); i++)
    {
        RedisHelper::appendHmset(args, Utils::itoa(indices[i]).c_str(), items[indices[i]].ToString().c_str());
    }
    RedisHelper::commitHmset(get_DbContext(), args);
}

PlayerEquip::PlayerEquip(int equiplimit) : ItemContainer(equiplimit)
{
}

PlayerEquip::~PlayerEquip()
{

}

PlayerEquip* PlayerEquip::Create(int size)
{
    return new PlayerEquip(size);
}


void AddItemProp(int itemid, BattleProp&  batprop, BaseProp& baseprop)
{
    ItemCfgDef* itemcfg = ItemCfg::getCfg(itemid);
    if (itemcfg == NULL)
    {
        return;
    }

    baseprop.setCapa(baseprop.getCapa() + itemcfg->ReadInt("capa"));
    baseprop.setStre(baseprop.getStre() + itemcfg->ReadInt("stre"));
    baseprop.setInte(baseprop.getInte() + itemcfg->ReadInt("inte"));
    baseprop.setPhys(baseprop.getPhys() + itemcfg->ReadInt("phys"));

    batprop.setAtk(batprop.getAtk() + itemcfg->ReadInt("atk"));
    batprop.setDef(batprop.getDef() + itemcfg->ReadInt("def"));
    batprop.setDodge(batprop.getDodge() + itemcfg->ReadFloat("dodge"));
    batprop.setHit(batprop.getHit() + itemcfg->ReadFloat("hit") );
    batprop.setCri(batprop.getCri() + itemcfg->ReadFloat("cri"));
    batprop.setMaxHp(batprop.getMaxHp() + itemcfg->ReadInt("maxhp"));
    batprop.setMaxMp(batprop.getMaxMp() + itemcfg->ReadInt("maxmp"));

}

int PlayerEquip::GetStoneNumByLvl(int lvl)
{
	int num = 0;
	for (int i = 0; i < items.size(); i++)
    {
		ItemCfgDef* itemcfg = ItemCfg::getCfg(items[i].item);
        if (itemcfg == NULL) {
            continue;
        }

		Json::Reader reader;
        Json::Value jsonval;
        reader.parse(items[i].json, jsonval);

		int holemum = itemcfg->ReadInt("holenum");
		for (int i = 1; i <= holemum; i++) {
			char key[32];
			sprintf(key, "stone%d", i);
			int stoneid = jsonval[key].asInt();
			if (stoneid <= 0) continue;

			ItemCfgDef* cfg = ItemCfg::getCfg(stoneid);
			if (cfg == NULL) {
				continue;
			}
			int stonelvl = cfg->ReadInt("stonelvl");
			// 满足等级
			if (stonelvl >= lvl) {
				num++;
			}
		}
	}
	return num;
}

void PlayerEquip::AddEquipStarLvl(ItemGroup &item, int lvl)
{
	if (lvl == 0) {
		return; 
	}
	Json::Value val;
	if (!xyJsonSafeParse(item.json, val)) {
		return;
	}
	// 修改星级
	int starlvl = val["star"].asInt();
	if (lvl < 0 && starlvl < abs(lvl)) {
		starlvl = 0;
	}
	else {
		starlvl += lvl;
	}
	val["star"] = starlvl;
	val["star_exp"] = 0;
	val["fusionlvl"] = 0;
	item.json = xyJsonWrite(val);
}

void PlayerEquip::EquipResonance(BattleProp& batprop, BaseProp& baseprop)
{
    int quality = -1;
    int deepen = -1;
    int equipCount = 0;

    for (int i = 0; i < items.size(); i++)
    {
        if (items[i].item <= 0)
        {
            continue;
        }
        ItemCfgDef* itemcfg = ItemCfg::getCfg(items[i].item);
        if (itemcfg == NULL)
        {
            continue;
        }
        if (itemcfg->ReadInt("part") > kEPTFootGuard + 1) continue;

        Json::Reader reader;
        Json::Value jsonval;
        reader.parse(items[i].json, jsonval);

        // 计算装备品质最低
        int qua = itemcfg->ReadInt("qua");
        if (quality > qua || quality == -1)
        {
            quality = qua;
        }

        // 计算装备星级最低
        int starlvl = jsonval["star"].asInt();
        if (deepen > starlvl || deepen == -1)
        {
            deepen = starlvl;
        }

        equipCount++;
    }
    // 装备件数是否满足
    if (equipCount > kEPTFootGuard)
    {
        // 获得系统加成值
        EquipResonanceDef def;
        EquipResonanceCfg::getEquipResonanceCount(def, deepen, quality);

        batprop.setAtk(batprop.getAtk() + def.getAtkup());
        batprop.setDef(batprop.getDef() + def.getDefup());
        batprop.setDodge(batprop.getDodge() + def.getDodgeup());
        batprop.setHit(batprop.getHit() + def.getHitup());
        batprop.setCri(batprop.getCri() + def.getCriup());
        batprop.setMaxHp(batprop.getMaxHp() + def.getHpup());
    }
    else
    {
        quality = 0;
        deepen = 0;
    }

    mQuality = quality;
    mDeepen = deepen;
}
void PlayerEquip::calculateEquipResonance()
{
    int quality = -1;
    int deepen = -1;
    int equipCount = 0;
    
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i].item <= 0)
        {
            continue;
        }
        ItemCfgDef* itemcfg = ItemCfg::getCfg(items[i].item);
        if (itemcfg == NULL)
        {
            continue;
        }
        if (itemcfg->ReadInt("part") > kEPTFootGuard + 1) continue;
        
        Json::Reader reader;
        Json::Value jsonval;
        reader.parse(items[i].json, jsonval);
        
        // 计算装备品质最低
        int qua = itemcfg->ReadInt("qua");
        if (quality > qua || quality == -1)
        {
            quality = qua;
        }
        
        // 计算装备星级最低
        int starlvl = jsonval["star"].asInt();
        if (deepen > starlvl || deepen == -1)
        {
            deepen = starlvl;
        }
        
        equipCount++;
    }
    // 装备件数是否满足
    if (equipCount <= kEPTFootGuard)
    {
        quality = 0;
        deepen = 0;
    }
    mQuality = quality;
    mDeepen = deepen;

}

void PlayerEquip::CalcProperty(BattleProp& batprop, BaseProp& baseprop)
{
    for (int i = 0; i < items.size(); i++)
    {
        if (items[i].item <= 0)
        {
            continue;
        }

        ItemCfgDef* itemcfg = ItemCfg::getCfg(items[i].item);
        if (itemcfg == NULL)
        {
            continue;
        }

        Json::Reader reader;
        Json::Value jsonval;
        reader.parse(items[i].json, jsonval);

        //star effect
        {
            int starlvl = 0;
            float effect;
            float fusionEffect = 1.0;
            if (GameFunctionCfg::getGameFunctionCfg().equipType == eMultipleEquipDrop)
            {
                starlvl = jsonval["star"].asInt();
                effect = StarCfgNew::getEffect(starlvl,itemcfg->ReadInt("lvl"), itemcfg->ReadInt("qua"), itemcfg->ReadInt("part"));
                //装备融合加成
                fusionEffect = EquipFusionCfg::getFusionEffect(jsonval["fusionlvl"].asInt(), itemcfg->ReadInt("lvl"), itemcfg->ReadInt("qua"), itemcfg->ReadInt("part"));
            }
            else
            {
                starlvl = jsonval["star"].asInt();
                effect = StarCfg::getEffect(starlvl);
            }
            
            string baptizeData = jsonval[EquipBaptizeCfg::getBaptizeKeyName()].asString();
            EquipBaptize::calcBaptizeProp(baptizeData, itemcfg->ReadInt("qua"), 0, batprop);
            
            batprop.setAtk(batprop.getAtk() + itemcfg->ReadInt("atk") * fusionEffect * effect);
            batprop.setDef(batprop.getDef() + itemcfg->ReadInt("def") * fusionEffect * effect);
            batprop.setDodge(batprop.getDodge() + itemcfg->ReadFloat("dodge") * fusionEffect * effect);
            batprop.setHit(batprop.getHit() + itemcfg->ReadFloat("hit") * fusionEffect * effect);
            batprop.setCri(batprop.getCri() + itemcfg->ReadFloat("cri") * fusionEffect * effect);
            batprop.setMaxHp(batprop.getMaxHp() + itemcfg->ReadInt("hp") * fusionEffect * effect);
            batprop.setMaxMp(batprop.getMaxMp() + itemcfg->ReadInt("mp") * fusionEffect * effect);
        }

        {
            //符文石
            int holemum = itemcfg->ReadInt("holenum");
            for (int i = 1; i <= holemum; i++)
            {
                char key[32];
                sprintf(key, "stone%d", i);
                int stoneid = jsonval[key].asInt();
                if (stoneid > 0)
                {
                    AddItemProp(stoneid, batprop, baseprop);
                }
            }
        }
    }
    // 加成装备共鸣
    EquipResonance(batprop, baseprop);
}

bool PlayerEquip::PutOnEquip(int roleid, int index, int& errorcode)
{
    Role* role = SRoleMgr.GetRole(roleid);

    ItemGroup bagequip = role->getBackBag()->GetItem(index);

    ItemCfgDef* itemcfg = ItemCfg::getCfg(bagequip.item);
    if (itemcfg == NULL)
    {
        errorcode = 1000;
        return false;
    }
    int part = itemcfg->ReadInt("part");
    if (part <= 0 || part > 10)
    {
        errorcode = 1000;
        return false;
    }
    int job = itemcfg->ReadInt("job", eAllJob);
    if (job == -1) {
        job = eAllJob;
    }
    if (job != eAllJob && job != role->getJob()) {
        errorcode = 1000;
        return false;
    }

    int partindex = part - 1;
    ItemGroup prevequip = role->getPlayerEquip()->GetItem(partindex);

    role->getPlayerEquip()->SetItem(partindex, bagequip);
    role->getPlayerEquip()->Store(role->getInstID(), partindex, bagequip);
    SendPlayerEquipNotify(role, partindex, bagequip);
    
    //如果是武器，保存品质
    if (part == kEPTWeapon) {
        role->saveWeaponQuality();
    }
    
//    BackBag::UpdateBackBag(roleid, index, prevequip);
    role->updateBackBagItems(index, prevequip);
    role->CalcPlayerProp(); //
    errorcode = 0;
    return true;
}


bool PlayerEquip::TakeOffEquip(int roleid, int index, int& errorcode)
{
    Role* role = SRoleMgr.GetRole(roleid);

    ItemGroup equip = role->getPlayerEquip()->GetItem(index);
    if (equip.item > 0)
    {
        ItemArray items;
        items.push_back(equip);

        GridArray grids;
        if(role->preAddItems(items, grids) != CE_OK)
        {
            errorcode = CE_BAG_FULL;
            return false;
        }
//        BackBag::UpdateBackBag(roleid, grids);
        role->updateItemsChange(grids);

        role->getPlayerEquip()->SetItem(index, ItemGroup());
        role->getPlayerEquip()->Store(roleid, index);
        SendPlayerEquipNotify(role, index, ItemGroup());
        role->CalcPlayerProp();
    }

    errorcode = 0;
    return true;
}


void PlayerEquip::UpdatePlayerEquip(int roleid, int index, const ItemGroup& equip)
{
    Role* role = SRoleMgr.GetRole(roleid);

    role->getPlayerEquip()->SetItem(index, equip);
    role->getPlayerEquip()->Store(roleid, index, equip);
    SendPlayerEquipNotify(role, index, equip);
}

void PlayerEquip::UpdatePlayerEquip(Role* role, int index, const ItemGroup& equip)
{
    if (role == NULL) {
        return;
    }
    
    role->getPlayerEquip()->SetItem(index, equip);
    role->getPlayerEquip()->Store(role->getInstID(), index, equip);
    SendPlayerEquipNotify(role, index, equip);
}

/*********************
 MaterialBoxForFashion
 ********************/

MaterialBoxForFashion::MaterialBoxForFashion(int size):BackBag(size)
{
    
}

MaterialBoxForFashion* MaterialBoxForFashion::Create(int size)
{
    if (size <= 0) {
        return NULL;
    }
    
    return new MaterialBoxForFashion(size);
}

void MaterialBoxForFashion::Sort(vector<int>& effgrids)
{
    BackBag::Sort(effgrids);
}

/*********************
 RetinueBag
 ********************/

RetinueBag::RetinueBag(int size):BackBag(size)
{
    
}

RetinueBag* RetinueBag::Create(int size)
{
    if (size <= 0) {
        return NULL;
    }
    
    return new RetinueBag(size);
}

void RetinueBag::Sort(vector<int>& effgrids)
{
    BackBag::Sort(effgrids);
}





