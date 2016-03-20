//
//  sceneplayer_hand.cpp
//  GameSrv
//
//  Created by cxy on 13-1-10.
//
//

#include "flopcard.h"
#include "hander_include.h"
#include "ItemBag.h"
#include "json/json.h"
#include "EnumDef.h"
#include "Pet.h"
#include "GlobalMsg_def.h"
#include "Global.h"
#include "Paihang.h"
#include "MysteriousMgr.h"
#include "CustomGift.h"
#include "gift_activity.h"
#include "Wardrobe.h"
#include "GameLog.h"
#include "CopyExtraAward.h"
#include "FashionCollect.h"
#include "daily_schedule_system.h"
#include "RoleAwake.h"
#include "RandomEvents.h"
#include "Honor.h"
#include "PlayerEquip.h"
#include "centerClient.h"

void SendBackBagNotify(Role* role, const GridArray& grids)
{
    if (grids.size() <= 0) {
        return;
    }
    
    notify_playerbag notify;
    notify.errorcode = 0;

    for (int i = 0; i < grids.size(); i++)
    {
        obj_addordelItem item;
        item.deloradd = (grids[i].item.item > 0 ? 1 : 0);
        item.index = grids[i].index;
        item.itemid = grids[i].item.item;
        item.bind = grids[i].item.bind;
        item.count = grids[i].item.count;
        item.json = grids[i].item.json;

        notify.items.push_back(item);
    }

    sendNetPacket(role->getSessionId(), &notify);
}

void sendRetinueBagNotify(Role* role, const GridArray& grids)
{
    if (grids.size() <= 0) {
        return;
    }
    
    notify_retinue_bag notify;
    notify.errorcode = 0;
    
    for (int i = 0; i < grids.size(); i++)
    {
        obj_addordelItem item;
        item.deloradd = (grids[i].item.item > 0 ? 1 : 0);
        item.index = grids[i].index;
        item.itemid = grids[i].item.item;
        item.bind = grids[i].item.bind;
        item.count = grids[i].item.count;
        item.json = grids[i].item.json;
        
        notify.items.push_back(item);
    }
    
    sendNetPacket(role->getSessionId(), &notify);
}

void SendBackBagNotify(Role* role, int index, const ItemGroup& item)
{

    notify_playerbag notify;
    notify.errorcode = 0;

    obj_addordelItem itemele;
    itemele.deloradd = (item.item > 0 ? 1 : 0);
    itemele.index = index;
    itemele.itemid = item.item;
    itemele.bind = item.bind;
    itemele.count = item.count;
    itemele.json = item.json;
    notify.items.push_back(itemele);

    sendNetPacket(role->getSessionId(), &notify);
}

void SendBackBagNotify(Role* role, std::vector<int>& indexs)
{
    notify_playerbag notify;
    notify.errorcode = 0;

    for (int i = 0;  i < indexs.size(); i++) {
        ItemGroup item = role->getBackBag()->GetItem(indexs[i]);
        obj_addordelItem itemele;
        itemele.deloradd = (item.item > 0 ? 1 : 0);
        itemele.index = indexs[i];
        itemele.itemid = item.item;
        itemele.bind = item.bind;
        itemele.count = item.count;
        itemele.json = item.json;
        notify.items.push_back(itemele);
    }
    sendNetPacket(role->getSessionId(), &notify);
}

void SendPlayerEquipNotify(Role* role, const GridArray& grids)
{
    notify_PlayerEquip notify;
    notify.errorcode = 0;

    for (int i = 0; i < grids.size(); i++)
    {
        obj_updateEquip item;
        item.deloradd = (grids[i].item.item > 0 ? 1 : 0);
        item.index = grids[i].index;
        item.itemid = grids[i].item.item;
        item.bind = grids[i].item.bind;
        item.json = grids[i].item.json;

        notify.equips.push_back(item);
    }

    sendNetPacket(role->getSessionId(), &notify);
}

void SendPlayerEquipNotify(Role* role, int index, const ItemGroup& equip)
{
    BagGrid grid;
    GridArray grids;

    grid.item = equip;
    grid.index = index;
    grids.push_back(grid);

    SendPlayerEquipNotify(role, grids);
}

void sendBagList(Role* role)
{
    const ItemArray& bagitems = role->getBackBag()->GetItems();
    ack_playerbag ack;
    ack.errorcode = 0;
    ack.maxbagsize = BackBag::sFreeBackbagSize + ActivateBagGridsCfg::getMaxPage() * BackBag::sBackbagSizePerPage;
    ack.bagsize = role->getBackBag()->GetCapacity();
    ack.rmbextendedsize = role->getRmbExtendedBackbagSize();
    for (int i = 0; i < ack.bagsize; i++)
    {
        if (bagitems[i].item <= 0)
        {
            continue;
        }
        
        obj_bagItem item;
        
        
        item.index = i;
        item.itemid = bagitems[i].item;
        item.count = bagitems[i].count;
        item.bind = bagitems[i].bind;
        item.json = bagitems[i].json;
        ack.items.push_back(item);
    }
    
    role->send(&ack);
}

hander_msg(req_playerbag,req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    sendBagList(role);
}}

hander_msg(req_sortbag, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    //wangzhigang 2014-11-19
//    BackBag::Sort(roleid);
    vector<int> effgrids;
    role->getBackBag()->Sort(effgrids);
    role->getBackBag()->Store(roleid, effgrids);

    sendBagList(role);

    ack_sortbag ack;
    ack.errorcode = 0;
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_dropBagEquip,req)
{

}}

hander_msg(req_PlayerEquip,req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    const ItemArray& bagitems = role->getPlayerEquip()->GetItems();

    ack_PlayerEquip ack;
    ack.errorcode = 0;
    
    EquipBaptize::checkPlayerEquipBaptize2TimeOut(role);
    
    for (int i = 0; i < role->getPlayerEquip()->GetCapacity(); i++)
    {
        if (bagitems[i].item <= 0)
        {
            continue;
        }

        obj_playerItem equip;
        
        equip.index = i;
        equip.itemid = bagitems[i].item;
        equip.bind = bagitems[i].bind;
        equip.json = bagitems[i].json;
        ack.equips.push_back(equip);
    }

    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_unloadEquip,req)
{

    hand_Sid2Role_check(sessionid, roleid, role)

    int equipindex = req.itemindex;
    if (equipindex >= role->getPlayerEquip()->GetCapacity() || equipindex < 0)
    {
        return;
    }

    ack_unloadEquip ack;
    PlayerEquip::TakeOffEquip(roleid, equipindex, ack.errorcode);
    if (ack.errorcode == CE_OK) {
        role->UploadPaihangData(eUpdate);
    }
    sendNetPacket(sessionid, &ack);
}}
// 穿装备
hander_msg(req_loadEquip,req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    int bagindex = req.itemindex;
    if (bagindex < 0 || bagindex >= role->getBackBag()->GetCapacity())
    {
        return;
    }
    
    ack_loadEquip ack;
    ack.errorcode = 0;
    PlayerEquip::PutOnEquip(roleid, bagindex, ack.errorcode);
    if (ack.errorcode == CE_OK) {
        role->UploadPaihangData(eUpdate);
    }
    sendNetPacket(sessionid, &ack);
	//　称号检查
	SHonorMou.procHonor(eHonorKey_EquipStone, role);
}}

//升星 强化装备
hander_msg(req_upgEquip, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    ack_upgEquip ack;
    ack.errorcode = 1;
    ack.position = req.position;
    ack.index = req.index;
    
    if (req.index < 0 || req.index >= role->getPlayerEquip()->GetCapacity())
    {
        return;
    }

    ItemGroup equip = role->getPlayerEquip()->GetItem(req.index);
    if (equip.item <= 0 || equip.count <= 0)
    {
        return;
    }

    Json::Reader reader;
    Json::Value jsonval;
    int starlvl;
    do
    {
        ItemArray items;
        GridArray grids;
        float stonepro = 0.0;
        if (req.luckystoneid != 0) {
            ItemCfgDef* luckystoneCfg = ItemCfg::getCfg(req.luckystoneid);
            CheckCondition(luckystoneCfg, break)
            stonepro = luckystoneCfg->ReadFloat("probability");
            items.push_back(ItemGroup(req.luckystoneid, 1));
            if (!role->getBackBag()->PreDelItems(items, grids))
            {
                break;
            }
            
        }
        
        ItemCfgDef* itemCfg = ItemCfg::getCfg(equip.item);
        CheckCondition(itemCfg, break)
        
        reader.parse(equip.json.c_str(), jsonval);
        CheckCondition(jsonval.empty() || jsonval.isObject(), return)

        starlvl = jsonval["star"].asInt();

        //获取强化所需金钱，这是多语言版本？？
        if (GameFunctionCfg::getGameFunctionCfg().equipType == eMultipleEquipDrop)
        {
            int qua = itemCfg->ReadInt("qua");
            int part = itemCfg->ReadInt("part");
            int lvl = itemCfg->ReadInt("lvl");

            StarCostCfgDef* def = StarCfgNew::getCostCfg(lvl, qua, part);
            CheckCondition(def != NULL, return)

            int needgold, needrmb;

            int maxLvl = def->getMaxLvl();
            CheckCondition(starlvl < maxLvl && def->getUpgCost(starlvl + 1, needgold, needrmb), return)
            CheckCondition(needgold > 0 || needrmb > 0, return)
            CheckCondition(role->getGold() >= needgold, return)
            CheckCondition(role->getRmb() >= needrmb, return)
            
            if (req.luckystoneid != 0)
            {
                //wangzhigang 2014-11-19
//                BackBag::UpdateBackBag(roleid, grids); //更新背包
                role->updateItemsChange(grids);
            }
            
            { // use luckystone log
                LogMod::addUseItemLog(role, items, "req_upgEquip");
            }
            
            if (needgold > 0)
            {
                role->addGold(-needgold, "upgrade_equip");
            }
            if (needrmb > 0)
            {
                role->addRmb(-needrmb, "upgrade_equip");
            }

            float roll = randf() + stonepro;
            int rollExp = StarCfgNew::getRollExp(roll);
            int starExp = jsonval["star_exp"].asInt();
            starExp += rollExp;

            int needExp = def->getExp(starlvl + 1);
            if (needExp <= starExp)
            {
                starlvl += 1;
                jsonval["star"] = starlvl;
                jsonval["star_exp"] = 0;
            }
            else
            {
                jsonval["star_exp"] = starExp;
            }

            ack.errorcode = 0;
            ack.starexp = rollExp;
            ack.starlvl = starlvl;
        }
        else
        {
            int reqlevel = StarCfg::getReqLvl(starlvl + req.upgEquipCount);
            if (role->getLvl() < reqlevel)
            {
                ack.errorcode = CE_YOUR_LVL_TOO_LOW;
                break;
            }
            
            if (0 == req.materialid) {
                int needgold = 0;
                
                for (int i = 1; i <= req.upgEquipCount; i++) {
                    needgold += StarCfg::getReqGold(starlvl + 1);
                    starlvl++;
                }
                
                if (role->getGold() < needgold || needgold <= 0)
                {
                    ack.errorcode = CE_SHOP_GOLD_NOT_ENOUGH;
                    break;
                }
                role->addGold(-needgold, "upgrade_equip");
            }
            else {
                
                int needstone = 0;
                for (int i = 1; i <= req.upgEquipCount; i++) {
                    needstone += StarCfg::getReqIntensifyStones(starlvl + 1);
                    starlvl++;
                }
             
                int nIntensifyStoneId = StarCfg::getIntensifyStoneId();
                ItemCfgDef* materialCfg = ItemCfg::getCfg(nIntensifyStoneId);
                if (materialCfg == NULL) {
                    ack.errorcode = CE_UPGRADEEQUIP_INTENSIFYSTONE_ID_ERROR;
                    break;
                }
                
                GridArray effgrids;
                ItemArray stone;
                stone.push_back(ItemGroup(nIntensifyStoneId, needstone));
                
                if (!role->getBackBag()->PreDelItems(stone, effgrids))
                {
                    ack.errorcode = CE_UPGRADEEQUIP_INTENSIFYSTONE_NOT_ENOUGH;
                    break;
                }

                role->updateItemsChange(effgrids);
                LogMod::addUseItemLog(role, stone, "req_upgEquip");
            }

            jsonval["star"] = starlvl;
            
            ack.errorcode = 0;
            ack.starlvl = starlvl;
        }
    }
    while (0);
    ack.upgEquipCount = req.upgEquipCount;
    sendNetPacket(sessionid, &ack);
    
    if (ack.errorcode == 0)
    {
        equip.json = Json::FastWriter().write(jsonval);
        PlayerEquip::UpdatePlayerEquip(role->getInstID(), req.index, equip);

        role->CalcPlayerProp();
        //更新战斗力排行
        role->UploadPaihangData(eUpdate);

        UpdateQuestProgress("upequip", equip.item, req.upgEquipCount, role, true);
        LogMod::addLogUpgradeEquip(roleid, role->getRolename().c_str(), role->getSessionId(), equip.item, starlvl);
    }
    else{
        notify_syn_gold noti;
        noti.gold = role->getGold();
        sendNetPacket(sessionid, &noti);
    }
}}

static int _return_gold_if_equip_have_fusionlvl(ItemCfgDef* cfg, const ItemGroup &item)
{
    Json::Value jsonval;
    string ext = item.json;
    if (!xyJsonSafeParse(ext, jsonval))
    {
        return 0;
    }
    
    try
    {
        // 返还部分融合金币
        if (jsonval["fusionlvl"].isNull() || jsonval["fusionlvl"].asInt() <= 0)
        {
            return 0;
        }
        
        int fusionLvl = jsonval["fusionlvl"].asInt();
        int equiplvl = cfg->ReadInt("lvl");
        int equipQua = cfg->ReadInt("qua");
        int equipPart = cfg->ReadInt("part");
        
        return EquipFusionCfg::getReturnGold(fusionLvl, equiplvl, equipQua, equipPart);
    }
    catch (...)
    {
        return 0;
    }
    return 0;
}

//提品
hander_msg(req_promoteEquip, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (req.index < 0 || req.index > role->getPlayerEquip()->GetCapacity())
    {
        return;
    }

    ack_promoteEquip ack;
    ack.errorcode = -1;
	int returnGold = 0;
    int index = req.index;
    int makeid;

    do
    {
        ItemGroup equip = role->getPlayerEquip()->GetItem(index);
        if (equip.item <= 0)
        {
            break;
        }

        ItemCfgDef* itemcfg = ItemCfg::getCfg(equip.item);
        if (itemcfg == NULL)
        {
            break;
        }

        makeid = itemcfg->ReadInt("upgprint");
        ItemCfgDef* makecfg = ItemCfg::getCfg(makeid);
        if (itemcfg == NULL || makecfg == NULL)
        {
            break;
        }

        int tarid = makecfg->ReadInt("tarid");
		// 需要的材料
		ItemArray materail;
		string matlog;
		for (int i = 1; i <= makecfg->ReadInt("matcount"); i++) {
			int matId = makecfg->ReadInt(xystr("mat%d", i).c_str());
			int matNum = makecfg->ReadInt(xystr("num%d", i).c_str());
			
			matlog.append(xystr("%d %d %d;", i, matId, matNum));
			// 是原装备
			if (matId == equip.item) {
				continue;
			}
			if (matId <= 0 || matNum <= 0) {
				continue;
			}
			ItemGroup mat;
			mat.item = matId;
			mat.count = matNum;
			materail.push_back(mat);
		}
		

        GridArray effgrids;
        
        if (!role->getBackBag()->PreDelItems(materail, effgrids))
        {
            break;
        }
        
        //wangzhigang 2014-11-19
//        BackBag::UpdateBackBag(role->getInstID(), effgrids);
        role->updateItemsChange(effgrids);

        // use item log
        {
            LogMod::addUseItemLog(role, materail, "promote_equip");
        }
        
        equip.item = tarid;
        
        // 提品的时候返还部分装备融合的金币（如果有融合等级的话）。
        returnGold = _return_gold_if_equip_have_fusionlvl(itemcfg, equip);
        role->addGold(returnGold, "return equip fusion gold");
    
        //去掉融合等级
        Json::Reader reader;
        Json::Value jsonval;
        reader.parse(equip.json, jsonval);
        jsonval["fusionlvl"] = 0;
        equip.json = Json::FastWriter().write(jsonval);

		// 提品后降低强化等级
		// 05-12 并清空经验与星阶
		PlayerEquip::AddEquipStarLvl(equip, - makecfg->ReadInt("reduce_star"));

        PlayerEquip::UpdatePlayerEquip(role->getInstID(), req.index, equip);

        ack.errorcode = 0;
        role->CalcPlayerProp();
        //更新战斗力排行
        role->UploadPaihangData(eUpdate);

        ItemCfgDef* tarcfg = ItemCfg::getCfg(tarid);
        if (tarcfg && tarcfg->ReadInt("qua") >= 3)
        {
            broadcastPromoteEquip(role->getRolename(), tarid);
        }
		// 记录提品日志
		Xylog log(eLogName_EquipPromote, roleid);
		log
		<<matlog
		<<equip.item
		<<itemcfg->ReadStr("name")
		<<tarid
		<<tarcfg->ReadStr("name")
		<<equip.json;
    }
    while (0);
    ack.index = index;
    ack.makeid = makeid;
	ack.returnGold = returnGold;

    sendNetPacket(sessionid, &ack);

}}


hander_msg(req_attachEquip, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (req.equipindex < 0 || req.equipindex > role->getPlayerEquip()->GetCapacity())
    {
        return;
    }

    ack_attachEquip ack;
    ack.errorcode = -1;
    ack.equipindex = req.equipindex;
    ack.stoneid = req.stoneid;

    do
    {
        ItemGroup equip = role->getPlayerEquip()->GetItem(req.equipindex);
        if (equip.item <= 0)
        {
            break;
        }

        GridArray effgrids;
        ItemArray stone;
        stone.push_back(ItemGroup(ack.stoneid, 1));
        if (!role->getBackBag()->PreDelItems(stone, effgrids))
        {
            break;
        }


        ItemCfgDef* equipcfg = ItemCfg::getCfg(equip.item);
        ItemCfgDef* itemcfg = ItemCfg::getCfg(ack.stoneid);
        if (equipcfg == NULL || itemcfg == NULL){
            break;
        }

        int type = itemcfg->ReadInt("type");
        if (type != kItemPropStone)
        {
            break;
        }

        int holenum = equipcfg->ReadInt("holenum");
        Json::Reader jsonreader;
        Json::Value jsonval;
        jsonreader.parse(equip.json, jsonval);

        int holeindex = 0;
        char key[32];
        int stonetype = itemcfg->ReadInt("stype");
        for (int i = 1; i <= holenum; i++)
        {
            sprintf(key, "stone%d", i);
            int stoneid = jsonval[key].asInt();
            if (stoneid == 0)
            {
                if (holeindex == 0)
                {
                    holeindex = i;
                }
            }
            else
            {
                ItemCfgDef* stonecfg = ItemCfg::getCfg(stoneid);
                if (stonecfg == NULL || stonecfg->ReadInt("stype") == stonetype)
                {
                    holeindex = 0;
                    break;
                }
            }
        }

        if (holeindex == 0)
        {
            break;
        }

//        BackBag::UpdateBackBag(role->getInstID(), effgrids);
        role->updateItemsChange(effgrids);

        {   //use item log
            LogMod::addUseItemLog(role, stone, "attach_equip");
        }
        
        sprintf(key, "stone%d", holeindex);
        jsonval[key] = req.stoneid;
        equip.json = Json::FastWriter().write(jsonval);
        PlayerEquip::UpdatePlayerEquip(role->getInstID(), req.equipindex, equip);

        ack.errorcode = 0;

        role->CalcPlayerProp();
        //更新战斗力排行
        role->UploadPaihangData(eUpdate);
    }
    while (0);

    sendNetPacket(sessionid, &ack);
	//　称号检查
	SHonorMou.procHonor(eHonorKey_EquipStone, role);
}}

hander_msg(req_deattachEquip, req)
{

    hand_Sid2Role_check(sessionid, roleid, role)
    if (req.equipindex < 0 || req.equipindex > role->getPlayerEquip()->GetCapacity())
    {
        return;
    }


    int equipindex = req.equipindex;
    int stonepos = req.stoneindex;

    ack_deattachEquip ack;
    ack.errorcode = -1;
    ack.equipindex = equipindex;
    ack.stoneindex = stonepos;

    do
    {
        ItemGroup item = role->getPlayerEquip()->GetItem(equipindex);

        Json::Reader reader;
        Json::Value jsonval;
        reader.parse(item.json, jsonval);

        char key[32];
        sprintf(key, "stone%d", stonepos);
        int stoneid = jsonval[key].asInt();
        if (stoneid <= 0)
        {
            break;
        }

        ItemArray items;
        GridArray effgrids;
        items.push_back(ItemGroup(stoneid, 1));
        
        //wangzhigang 2014-11-19
        if (role->preAddItems(items, effgrids) != CE_OK)
        {
            break;
        }

        Json::FastWriter writer;
        jsonval[key] = 0;
        item.json = writer.write(jsonval);

//        BackBag::UpdateBackBag(roleid, effgrids);
        role->updateItemsChange(effgrids);
        
        PlayerEquip::UpdatePlayerEquip(roleid, equipindex, item);

        ack.errorcode = 0;

        role->CalcPlayerProp();
        //更新战斗力排行
        role->UploadPaihangData(eUpdate);
    }
    while (0);

    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_equip_baptize, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (req.equipIndex < 0 || req.equipIndex > role->getPlayerEquip()->GetCapacity())
    {
        return;
    }
    
    ItemGroup& item = role->getPlayerEquip()->GetItem(req.equipIndex);
    
    if (item.item <= 0) {
        return;
    }
    
    ack_equip_baptize ack;
    
    //花费
    int needCount = EquipBaptizeCfg::getBatizeCost();
    check_min(needCount, 0);
    
    int costItem = EquipBaptizeCfg::getBatizeStoneId();
    
    ItemArray items;
    GridArray effeGrib;
    
    if (costItem) {
        
        items.push_back(ItemGroup(costItem, needCount));
        if (!role->getBackBag()->PreDelItems(items, effeGrib)) {
            ack.errorcode = CE_ITEM_NOT_ENOUGH;
            sendNetPacket(sessionid, &ack);
            return;
        }
    }

    //随机出新属性
    pair<string, string> newPro = EquipBaptizeCfg::makeBaptizeProp();
    
    if (newPro.first.empty()) {
        ack.errorcode = CE_EQUIP_BAPTIZE_FAILD;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (costItem) {
        role->playerDeleteItemsAndStore(effeGrib, items, "洗练", true);
    }
    
    
    //写进新属性
    EquipBaptize::equipBaptize(roleid, req.equipIndex, item, req.baptizeQua, req.baptizeIndex, newPro.first, newPro.second);
    
    PlayerEquip::UpdatePlayerEquip(role, req.equipIndex, item);
    
    role->CalcPlayerProp();
    
    ack.errorcode = CE_OK;
    ack.equipIndex = req.equipIndex;
    ack.baptizeQua = req.baptizeQua;
    ack.baptizeIndex = req.baptizeIndex;
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_equip_baptize_recover, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (req.equipIndex < 0 || req.equipIndex > role->getPlayerEquip()->GetCapacity())
    {
        return;
    }
    
    ack_equip_baptize_recover ack;

    ItemGroup& item = role->getPlayerEquip()->GetItem(req.equipIndex);
    
    if (item.item <= 0) {
        return;
    }
    
    ItemCfgDef* equipcfg = ItemCfg::getCfg(item.item);
    if(equipcfg == NULL)
    {
        ack.errorcode = CE_READ_CFG_ERROR;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (req.baptizeQua < 0 || req.baptizeQua > equipcfg->ReadInt("qua")) {
        return;
    }
    
    int rmbNeed = EquipBaptizeCfg::getRecoveryCost();
    if (rmbNeed > role->getRmb()) {
        ack.errorcode = CE_SHOP_RMB_NOT_ENOUGH;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (!EquipBaptize::recoverBaptizeProp(roleid, req.equipIndex, item, req.baptizeQua, req.baptizeIndex))
    {
        ack.errorcode = CE_EQUIP_BAPTIZE_BACKUPPROP_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    PlayerEquip::UpdatePlayerEquip(role, req.equipIndex, item);
    role->addRmb(-rmbNeed, "洗练还原");
    role->CalcPlayerProp();
    
    SendPlayerEquipNotify(role, req.equipIndex, item);
    
    ack.errorcode = CE_OK;
    ack.equipIndex = req.equipIndex;
    ack.baptizeIndex = req.baptizeIndex;
    ack.baptizeQua = req.baptizeQua;
    
    sendNetPacket(sessionid, &ack);
}}


string itemArray2LogStr(const ItemArray& items)
{
    string ret;
    for (int i = 0; i < items.size(); i++) {
        string itemStr = strFormat("%d:%d;", items[i].item, items[i].count);
        ret.append(itemStr);
    }
    return ret;
}

hander_msg(req_composeItem, req)
{

    hand_Sid2Role_check(sessionid, roleid, role)

    ack_composeItem ack;
    ack.errorcode = CE_OK;
    ack.composeid = req.composeid;
    ack.newcount = req.count;

    do {
        int composeId = req.composeid;
        SynthesisCfgDef* cfgDef = SynthesisCfg::getSynthesisCfg(composeId);
        if (cfgDef == NULL) {
            ack.errorcode = CE_READ_CFG_ERROR;
            break;
        }
        
        if (cfgDef->needGold < 0) {
            break;
        }
        if (role->getGold() < cfgDef->needGold) {
            ack.errorcode = CE_SHOP_GOLD_NOT_ENOUGH;
            break;
        }
        
        if (role->getLvl() < cfgDef->needLvl) {
            ack.errorcode = CE_YOUR_LVL_TOO_LOW;
            break;
        }
        
        ItemArray materials;
        for (vector<ItemCell>::iterator iter = cfgDef->materials.begin();
             iter != cfgDef->materials.end(); iter++) {
            materials.push_back(ItemGroup(iter->itemid, iter->count * req.count));
        }
        
        ItemArray targets;
        for (vector<ItemCell>::iterator iter = cfgDef->synthetics.begin();
             iter != cfgDef->synthetics.end(); iter++) {
            targets.push_back(ItemGroup(iter->itemid, iter->count * req.count));
        }
        
        GridArray addGrids;
        GridArray delGrids;
        if (!role->getBackBag()->PreDelItems(materials, delGrids)) {
            ack.errorcode = CE_ITEM_NOT_ENOUGH;
            break;
        }

        if(role->preAddItems(targets, addGrids)){
            ack.errorcode = CE_BAG_FULL;
            break;
        }
        
        role->addGold(-cfgDef->needGold);
//        role->updateBackBag(addGrids, targets, true, "compose_item", true);
//        role->updateBackBag(delGrids, materials, false, "compose_item", true);
        role->playerAddItemsAndStore(addGrids, targets, "compose_item", true);
        role->playerDeleteItemsAndStore(delGrids, materials, "compose_item", true);
        
        string materialStr = itemArray2LogStr(materials);
        string targetStr = itemArray2LogStr(targets);
        LogMod::addLogComposeItem(roleid, role->getRolename().c_str(), role->getSessionId(),
                                  materialStr.c_str(), targetStr.c_str());
        
            /*
        int comptar = itemcfg->ReadInt("comptar");
        int compnum = itemcfg->ReadInt("compnum");
        if (compnum <= 0 || comptar <= 0)
        {
            ack.errorcode = CE_READ_CFG_ERROR;
            break;
        }


        int tarnum = req.count / compnum;
        if (tarnum <= 0)
        {
            ack.errorcode = CE_READ_CFG_ERROR;
            break;
        }
        int prevnum = tarnum * compnum;

        ItemArray previtems;
        GridArray prevgrids;
        previtems.push_back(ItemGroup(composeid, prevnum));

        if (!role->getBackBag()->PreDelItems(previtems, prevgrids))
        {
            ack.errorcode = CE_ITEM_NOT_ENOUGH;
            break;
        }

        ItemArray newitems;
        GridArray newgrids;
        newitems.push_back(ItemGroup(comptar, tarnum));
        if (!role->getBackBag()->PreAddItems(newitems, newgrids))
        {
            ack.errorcode = CE_BAG_FULL;
            break;
        }
        
        {   // use item log
            LogMod::addUseItemLog(role, previtems, "compose_item");
        }

        //log
        {
            string compItems = "";
            string newItems = "";
            char buf[32] = "";
            for (int i = 0; i < previtems.size(); i++) {
                sprintf(buf, "%d:%d", previtems[i].item, previtems[i].count);
                compItems.append(buf);
            }

            for (int i = 0; i < newitems.size(); i++) {
                sprintf(buf, "%d:%d", newitems[i].item, newitems[i].count);
                newItems.append(buf);
            }
            LogMod::addLogComposeItem(roleid, role->getRolename().c_str(), role->getSessionId(), compItems.c_str(), newItems.c_str());

            LogMod::addLogGetItem(roleid, role->getRolename().c_str(), role->getSessionId(), newitems, "compose_item");
        }

        BackBag::UpdateBackBag(roleid, prevgrids);
        BackBag::UpdateBackBag(roleid, newgrids);
        ack.errorcode = 0;
        ack.previtemid = composeid;
        ack.previtemid = req.composeid;
        ack.newitemid = comptar;
        ack.newcount = tarnum;

        ItemCfgDef* tarcfg = ItemCfg::getCfg(comptar);
        if (tarcfg == NULL)
        {
            ack.errorcode = CE_READ_CFG_ERROR;
            break;
        }

        int type = tarcfg->ReadInt("type");
        if (type == kItemPropStone)
        {
            int stoneLvl = tarcfg->ReadInt("lvl");
            if (stoneLvl > 7)
            {
                broadcastGetStone(role->getRolename(), comptar);
            }
            g_GiftActivityMgr.check(eTargetType_ComposeLvlStone, stoneLvl, 0, role);
        }*/

    }
    while (0);

    sendNetPacket(sessionid, &ack);
}}



//static Pet* producePet(Role* role, int petmod, std::vector<Pet*>& newPets)
//{
//    PetCfgDef* cfg = PetCfg::getCfg(petmod);
//    if (cfg == NULL)
//    {
//        return NULL;
//    }
//
//    if (role->mPetMgr->getPetsCount() >= MAX_PETS_COUNT) {
//        return NULL;
//    }
//
//    PetSave info;
//    info.owner = role->getInstID();
//    info.isactive = 0;
//    info.totalexp = 0;
//    info.exp = 0;
//    info.lvl = 1;
//    info.petmod = petmod;
//    info.oldGrowth = range_rand(cfg->growth[0], cfg->growth[1]);
//    info.batk = range_rand(cfg->batk[0], cfg->batk[1]);
//    info.bdef = range_rand(cfg->bdef[0], cfg->bdef[1]);
//    info.bhp = range_rand(cfg->bhp[0], cfg->bhp[1]);
//    info.bdodge = range_randf(cfg->bdodge[0], cfg->bdodge[1]);
//    info.bhit = range_randf(cfg->bhit[0], cfg->bhit[1]);
//
//    info.mCapa = cfg->getCapa();
//    info.mInte = cfg->getInte();
//    info.mPhys = cfg->getPhys();
//    info.mStre = cfg->getStre();
//
//    info.star = 0;
//    info.starlvl = 0;
//
//    if ( (rand()%100) < (cfg->getExtraProp() * 100)) {
//        switch (rand()%4) {
//            case 0:
//                info.mCapa += cfg->getExtraPropPoint();
//                break;
//            case 1:
//                info.mPhys += cfg->getExtraPropPoint();
//                break;
//            case 2:
//                info.mInte += cfg->getExtraPropPoint();
//                break;
//            case 3:
//                info.mStre += cfg->getExtraPropPoint();
//                break;
//            default:
//                break;
//        }
//    }
//
//    for (int i = 0; i < cfg->skills.size(); i++)
//    {
//        float randnum = range_randf(0.0, 1.0);
//        if (randnum > cfg->skillprops[i])
//            continue;
//
//        bool isLock = range_randf(0.0, 1.0) < cfg->skilllockprops[i];
//
//        if(isLock)
//            info.oldlockSkills.push_back(cfg->skills[i]);
//        else
//            info.oldskills.push_back(cfg->skills[i]);
//
//    }
//
//    Pet* pet = PetMgr::create(info);
//    if (pet == NULL)
//    {
//        return NULL;
//    }
//
//    role->mPetMgr->addPet(pet);
//    role->mPetMgr->save();
//    newPets.push_back(pet);
//
//    return pet;
//}

static void notifyNewPets(Role* role, std::vector<Pet*>& newPets)
{
    if (newPets.size() == 0) {
        return;
    }
    
    int broadcastTimes = 0;

    notify_get_pet notify;

    for (int i = 0; i < newPets.size(); i++) {

        PetCfgDef* cfg = PetCfg::getCfg(newPets[i]->petmod);
        if (cfg == NULL)
        {
            continue;
        }

        if (cfg->getQua() > 3 && broadcastTimes < 3)
        {
            broadcastTimes++;
            broadcastGetPet(role->getRolename(), newPets[i]->petmod);
        }

        if (newPets[i] != NULL) {
            obj_petinfo info;
            newPets[i]->getPetInfo(info);
            notify.petsinfo.push_back(info);
        }
    }

    sendNetPacket(role->getSessionId(), &notify);
    
    for (int i = 0; i < newPets.size(); i++) {
        
        if (newPets[i] == NULL) {
            continue;
        }
        
        //添加图鉴
        role->getIllustrationsMgr()->addIllustrations(newPets[i]->petmod);
    }
}

string getRandBoxItems(ItemCfgDef* randbox)
{
    string boxRewards = randbox->ReadStr("fixaward");
    
    if (!boxRewards.empty()) {
        boxRewards.append(";");
    }

    vector<int> props;
    vector<string> rewards;

    int rewardnum = randbox->ReadInt("rewardnum", 5);
    for (int i = 1; i <= rewardnum; i++)
    {
        char rewardbuf[32], propbuf[32];
        sprintf(rewardbuf, "rewards%d", i);
        sprintf(propbuf, "pro%d", i);

        int prop = randbox->ReadInt(propbuf);
        string reward = randbox->ReadStr(rewardbuf);

        props.push_back(prop);
        rewards.push_back(reward);
    }

    int rewardindex = getRand(props.data(), rewardnum);
    boxRewards.append(rewards[rewardindex]);
    
    return boxRewards;
}

static int openPetEgg(ItemCfgDef* petegg)
{
    int petmod = petegg->ReadInt("pet_id");
    if (petmod == 0)
    {
        int randpet = petegg->ReadInt("randpet");
        if (randpet == 0)
        {
            return 0;
        }

        ItemCfgDef* randbox = ItemCfg::getCfg(randpet);
        if (randbox && randbox->ReadInt("type") == kItemRandBox)
        {
            string petstr = getRandBoxItems(randbox);
            sscanf(petstr.c_str(), "item %d", &petmod);
        }
    }

    return petmod;
}

//使用地下城道具
int useResetDungeTimesItem(ItemCfgDef* cfg, Role* self, string &award)
{
    if (self->getBackBag()->EmptyGridAmount() < 10)
    {
        //背包格子小于10格
        return eUseResetDungeTimesItemResult_BagCapacityFull;
    }
    
    int dungcopytype = cfg->ReadInt("dungeon_type", 0);
    //if (dungcopytype <= 0 || dungcopytype > 3)
    //{
        //配置表有问题
    //    return eUseResetDungeTimesItemResult_ConfigError;
    //}
    
    //dungeWipeOut(<#int dungeIndex#>, <#Role *role#>, <#std::vector<obj_dungeawawrd> &outdata#>)

    DungeonCfgDef* dungeCfg =  DungeonCfg::getDungeonCfg(stDungeon, dungcopytype);
    if (!dungeCfg) {
        return eUseResetDungeTimesItemResult_ConfigError;
    }
    
    vector<string> awards;
    RewardStruct reward;
    ItemArray allitems;
    
    //获取记录
    int record = self->getDungeLevelRecord(dungcopytype);
    if (record == 0)
    {
        //没有通关任意地下城
        return eUseResetDungeTimesItemResult_NotGoDunge;
    }
    
    int beginsceneid = dungeCfg->firstmap;
    int endsceneId = self->getDungeLevelRecord(dungcopytype);
    if (endsceneId < 0)
    {
        endsceneId = dungeCfg->firstmap + dungeCfg->floor;
    }
    
    for (int i = beginsceneid; i < endsceneId; i++)
    {
        int fcindex = 0;

        std::vector<int> dropindices;
        string dropStr;
        std::vector<string>   rewards = StrSpilt(SflopcardMgr.RandomCard(i,fcindex, dropindices, dropStr),";");
        
        rewardsCmds2ItemArray(rewards, allitems, reward);
        
        SceneCfgDef* _sceneCfg = SceneCfg::getCfg(i);
        if (_sceneCfg == NULL)
        {
            //配置读取错误，不应该
            return eUseResetDungeTimesItemResult_ConfigError;
        }

        reward.reward_exp += _sceneCfg->copy_exp;
        reward.reward_batpoint += _sceneCfg->copy_battlepoint;
        reward.reward_consval += _sceneCfg->copy_protoss;
        reward.reward_gold += _sceneCfg->copy_gold;
        reward.reward_petexp += _sceneCfg->copy_petExp;
    }
    
    char buffer[100];
    snprintf(buffer, 100, "exp %d*1", reward.reward_exp);
    awards.push_back(buffer);
    snprintf(buffer, 100, "batpoint %d*1", reward.reward_batpoint);
    awards.push_back(buffer);
    snprintf(buffer, 100, "consval %d*1", reward.reward_consval);
    awards.push_back(buffer);
    snprintf(buffer, 100, "gold %d*1", reward.reward_gold);
    awards.push_back(buffer);
    snprintf(buffer, 100, "petexp %d*1", reward.reward_petexp);
    awards.push_back(buffer);
    
    for (int i = 0; i < allitems.size(); ++i)
    {
        
        if (allitems[i].item != 0)
        {
            snprintf(buffer, 100, "item %d*%d", allitems[i].item, allitems[i].count);
            awards.push_back(buffer);
        }
    }
    
    stringstream ss;
    for (int i = 0; i < awards.size(); ++i)
    {
        if (i == 0)
        {
            ss << awards[i];
        }
        else
        {
            ss << ";" << awards[i] << ";";
        }
    }
    award = ss.str();
    
    return eUseResetDungeTimesItemResult_Ok;
}

//使用消耗品，成功返回0，失败返回1
int useConsumeItem(ItemCfgDef* cfg, Role* role, int count, string *getitems /*=NULL*/)
{
    if(cfg == NULL || role == NULL)
        return 1;
    
    
    //判断物品是否过期
    string itemexpiretime = cfg->ReadStr("overdate", "");
    if (!itemexpiretime.empty())
    {
        int deadline = Utils::parseDate(itemexpiretime.c_str());
        if (deadline != -1)
        {
            time_t now = time(NULL);
            if (now >= deadline)
            {
                //物品已经过期了，不可用。
                return eUseResetDungeTimesItemResult_ItemExpired;
            }
        }
    }

    string affect_type1 = cfg->ReadStr("affect_type1");
    int affect_value1 = cfg->ReadInt("affect_value1") * count;

    if(affect_type1.empty())
        return 1;

    string mark("useitem:");
    char buf[32] = "";
    mark += Utils::itoa(cfg->itemid, 10, buf);
    


    if( strcmp(affect_type1.c_str(), "incr_fat") == 0){
        role->addFat(affect_value1);
        return 0;

    }

    if( strcmp(affect_type1.c_str(), "incr_gold") == 0){
        role->addGold(affect_value1,mark.c_str());
        return 0;
    }

    if( strcmp(affect_type1.c_str(), "incr_exp") == 0){
        role->addExp(affect_value1, mark.c_str());
        return 0;
    }

    if( strcmp(affect_type1.c_str(), "incr_batpoint") == 0){
        role->addBattlePoint(affect_value1, mark.c_str());
        return 0;
    }

    if( strcmp(affect_type1.c_str(), "incr_constell") == 0){
        role->addConstellVal(affect_value1, mark.c_str());
        return 0;
    }

    if( strcmp(affect_type1.c_str(), "incr_rmb") == 0){
        role->addRmb(affect_value1,mark.c_str());
        return 0;
    }
    
    if (strcmp(affect_type1.c_str(), "incr_doublePotionNum_times") == 0) {
        role->setDoublePotionNum(role->getDoublePotionNum() + count);
        
        string addAction = Utils::makeStr("useitem_%d", cfg->itemid);
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesFriendCopyDoubleAward, addAction.c_str());
        return 0;
    }

    //精英副本次数
    if( strcmp(affect_type1.c_str(), "incr_elitecopy_times") == 0){
        role->setIncrEliteCopyTimes(role->getIncrEliteCopyTimes() + affect_value1);
        notify_syn_elitetimes notify;
        notify.times = role->getEliteTimes() + role->getIncrEliteCopyTimes();
        role->send(&notify);
        
        string describe = Utils::makeStr("useitem_%d", cfg->itemid);
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesEliteCopy, describe.c_str());
        return 0;
    }
    //地下城扫荡次数
    if( strcmp(affect_type1.c_str(), "incr_dungeonhangup_times") == 0){
        //直接扫荡奖励
        string award;
        //传值进去，诶。。。
        int ret = useResetDungeTimesItem(cfg, role, award);
        if (getitems)
        {
            *getitems = award;
        }
        return ret;
    }
    
    //幻兽大冒险副本次数
    if( strcmp(affect_type1.c_str(), "incr_petadventure_times") == 0){
        role->setIncrPetAdventureTimes(role->getIncrPetAdventureTimes() + affect_value1);
        
        notify_syn_petadventuretimes notify;
        notify.times = role->calPetAdventureTimes();
        role->send(&notify);
        
        string action = Utils::makeStr("useitem_%d", cfg->itemid);
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesPetAdventure, action.c_str());
        
        return 0;
    }
    
    //幻兽大本营使用物品增加打副本百分之一百出现隐藏BOSS和哥布林的次数
    if( strcmp(affect_type1.c_str(), "incr_mysticalcopy_certainly_appear_bossandgoblin_times") == 0){
        role->setIncrMysticalCopyBOSSandGoblinTimes(role->getIncrMysticalCopyBOSSandGoblinTimes() + affect_value1);
        
        string action = Utils::makeStr("useitem_%d", cfg->itemid);
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesMysticalCopyBOSSandGoblin, action.c_str());
        return 0;
    }
    
    //转盘抽奖次数
    if( strcmp(affect_type1.c_str(), "incr_lottery_times") == 0){
        role->setIncrLotteryTimes(role->getIncrLotteryTimes() + affect_value1);
        return 0;
    }
    
    //保护神像
    if( strcmp(affect_type1.c_str(), "incr_towerdefense_times") == 0){
        role->setIncrTowerdefenseTimes(role->getIncrTowerdefenseTimes() + affect_value1);
        
        string addAction = Utils::makeStr("useitem_%d", cfg->itemid);
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesTowerDefense, addAction.c_str());
        return 0;
    }
    //好友地下城
    if( strcmp(affect_type1.c_str(), "incr_frienddungeon_times") == 0){
        role->setIncrFriendDungeonTimes(role->getIncrFriendDungeonTimes() + affect_value1);
        
        string addAction = Utils::makeStr("useitem_%d", cfg->itemid);
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesFriendDunge, addAction.c_str());
        return 0;
    }
    
    //同步组队副本收益次数
    if( strcmp(affect_type1.c_str(), "incr_syncteamcopyincome_times") == 0){
        role->setIncrSyncTeamCopyTimesForIncome(role->getIncrSyncTeamCopyTimesForIncome() + affect_value1);
        
        string addAction = Utils::makeStr("useitem_%d", cfg->itemid);
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesSyncTeamCopy, addAction.c_str());
        return 0;
    }

    // 附魔尘
    if (strcmp(affect_type1.c_str(), "incr_enchantdust") == 0) {
        role->addEnchantDust(affect_value1, mark.c_str());
        return 0;
    }
    
    // 附魔钻
    if (strcmp(affect_type1.c_str(), "incr_enchantgold") == 0) {
        role->addEnchantGold(affect_value1, mark.c_str());
        return 0;
    }
    
    return 1;
}

obj_fashion_info getFashionObjInfo(WardrobeFashion* fashion)
{
    obj_fashion_info  info;
    if (fashion->mExpireTime == -1)
    {
        info.expiration = fashion->mExpireTime;
    }
    else
    {
        info.expiration = fashion->mExpireTime - Game::tick;
    }
    info.isexpired = fashion->mExpired;
    info.fashionid = fashion->mId;

    return info;
}

bool useFashion(ItemCfgDef* cfg, Role* role)
{
    int itemType = cfg->ReadInt("type");
    if (itemType != KItemFashion)
    {
        return false;
    }
    
    int lvl = cfg->ReadInt("lvl");
    int job = cfg->ReadInt("job", (int)eAllJob);
    int sex = cfg->ReadInt("sex", (int)eAllSex);
    if (lvl > role->getLvl() ||
        (job != eAllJob && job != role->getJob()) ||
        (sex != eAllSex && sex != role->getSex()))
    {
        return false;
    }
    
    Wardrobe* wardrobe = role->getWardrobe();
    wardrobe->addFashion(cfg);
    
    //wardrobe->checkExpiredFashions(NULL);
    WardrobeFashion* fashion = wardrobe->getFashion(cfg->ReadInt("template"));
    if (fashion)
    {
        notify_refresh_fashion notify;
        notify.fashion = getFashionObjInfo(fashion);
        sendNetPacket(role->getSessionId(), &notify);
        LogMod::addLogAddFashion(role, cfg->itemid, fashion ? -1 : fashion->mType, fashion->mExpireTime);
        
        role->getFashionCollect()->playerGetNewFashionid(fashion->mId);
        return true;
    }
    
    return false;
}

static string __itemtype_to_string(const int &itemtype)
{
    switch (itemtype)
    {
        case 1:
            return "kItemEquip"; break;
        case 2:
            return "kItemConsumable"; break;
        case 3:
            return "kItemMaterial"; break;
        case 4:
            return "kItemPackage"; break;
        case 5:
            return "kItemPropStone"; break;
        case 6:
            return "kItemMakePrint"; break;
        case 7:
            return "kItemRandBox"; break;
        case 8:
            return "kItemPetEgg"; break;
        case 9:
            return "KItemPetDebris"; break;
        case 10:
            return "KItemProps"; break;
        case 11:
            return "KItemActivityBox"; break;
        case 12:
            return "kItemSkillBook"; break;
        case 13:
            return "kItemGrowthPill"; break;
        case 14:
            return "KItemTicket"; break;
        case 16:
            return "KItemRenameCard"; break;
        case 17:
            return "KItemShopProps"; break;
        case 99:
            return "kItemCustomGift"; break;
        default:
            return "unknown";
    }
}

hander_msg(req_useitem, req)
{

    hand_Sid2Role_check(sessionid, roleid, role)
    if (req.index < 0 || req.index > role->getBackBag()->GetCapacity())
    {
        return;
    }

    ack_useitem ack;
    ack.errorcode = CE_READ_CFG_ERROR;
    ack.index = req.index;

    do
    {
        ItemGroup item = role->getBackBag()->GetItem(req.index);
        ack.itemid = item.item;

        if (item.item <= 0 || item.count <= 0)
        {
            break;
        }

        ItemCfgDef* cfg = ItemCfg::getCfg(item.item);
        if (cfg == NULL)
        {
            break;
        }

        string getitems = "";

        int itemtype = cfg->ReadInt("type");
        int uselvl = cfg->ReadInt("lvl");
        if (uselvl > role->getLvl()) {
            break;
        }

        string comefrom = "";
        switch (itemtype) {
            case kItemPackage:
            {
                getitems = cfg->ReadStr("oitems");
                comefrom = Utils::makeStr("itemPackage:%d", cfg->itemid);
                ack.errorcode = CE_OK;
                break;
            }
            case kItemRandBox:
            {
                int emptyGridNum = role->getBackBag()->EmptyGridAmount();
                if (emptyGridNum <= 0)
                {
                    ack.errorcode = CE_BAG_FULL;
                    break;
                }

                getitems = getRandBoxItems(cfg);
                comefrom = Utils::makeStr("randBoxItem:%d", cfg->itemid);
                
                ack.errorcode = CE_OK;
                break;
            }
            case kItemPetEgg:
            {
                int petmod = openPetEgg(cfg);
                std::vector<Pet*> newpets;
                newpets.clear();

                if (role->mPetMgr->getPetsCount() >= MAX_PETS_COUNT) {
                    ack.errorcode = CE_CAGE_IS_FULL;
                    break;
                }
                
                Pet* newpet = producePet(role, petmod);

                if (newpet == NULL) {
                    ack.errorcode = CE_READ_CFG_ERROR;
                    break;
                }
                newpets.push_back(newpet);
                notifyNewPets(role, newpets);
                
                // 添加图鉴
//                role->getIllustrationsMgr()->addIllustrations(petmod);

                ack.errorcode = CE_OK;
                comefrom = Utils::makeStr("petegg:%d", item.item);
                LogMod::addLogGetPet(roleid, role->getRolename().c_str(), role->getSessionId(), petmod, newpet->petid, comefrom.c_str());
                
                ack.errorcode = CE_OK;
                break;
            }
            case kItemConsumable:
            {
                //判断是否使用地下城物品
                int ret = useConsumeItem(cfg,role, 1, &getitems);
                if(eUseResetDungeTimesItemResult_ConfigError == ret)
                {
                    ack.errorcode = CE_READ_CFG_ERROR;
                }
                else if (eUseResetDungeTimesItemResult_BagCapacityFull == ret)
                {
                    ack.errorcode = CE_BAG_CAPACITY_LESS_THAN_N_CELL;
                }
                //使用扫荡地下城物品，并且还没有打地下城
                else if (eUseResetDungeTimesItemResult_NotGoDunge == ret)
                {
                    ack.errorcode = CE_USE_DUNGEITEM_ERROR;
                }
                //item expired
                else if (eUseResetDungeTimesItemResult_ItemExpired == ret)
                {
                    ack.errorcode = CE_ITEM_EXPIRED;
                }
                else
                {
                    ack.errorcode = CE_OK;
                    role->saveNewProperty();
                }
                break;
            }
            case KItemActivityBox:
            {
                getitems = SRandomEventsSys.getRandBox(cfg, role);
                comefrom = Utils::makeStr("活动宝箱:%d:%s", cfg->itemid, getitems.c_str());
                ack.errorcode = CE_OK;
                break;
            }
			case kItemCustomGift:
			{
				CustomGift cg(item.json);
				getitems = cg.getRandItem();
				comefrom = Utils::makeStr("自定义礼包:%d:%s", cfg->itemid, getitems.c_str());
                ack.errorcode = CE_OK;
				break;
			}
            case kItemPartedByJobSex:
            {
                ObjJob mJob = role->getJob();
                ObjSex mSex = role->getSex();
                string parteditem = strFormat("jobpackage%d%d", mJob, mSex);
                string oitems = cfg->ReadStr("oitems");
                string jobpackage = cfg->ReadStr(parteditem.c_str());
                if (oitems == "") {
                    getitems = jobpackage;
                } else if (jobpackage == "") {
                    getitems = oitems;
                } else {
                    getitems = oitems + ";" + jobpackage;
                }
                
                comefrom = Utils::makeStr("按职业性别开礼包:%d", cfg->itemid);
                ack.errorcode = CE_OK;
                break;
            }
            case KItemFashion:
            {
                if (useFashion(cfg, role))
                {
                    ack.errorcode = CE_OK;
                }
                else
                {
                    ack.errorcode = CE_USE_FASHION_ERROR;
                }
                break;
            }
            default:
                break;
        }
        
        if (ack.errorcode == CE_OK)
        {
            switch (itemtype) {
                case KItemFashion:
                {
                    Wardrobe* wardrobe = role->getWardrobe();
                    
                    notify_wardrobe_exp notify;
                    notify.lvl = wardrobe->getLvl();
                    notify.exp = wardrobe->getExp();
                    sendNetPacket(sessionid, &notify);
                    break;
                }
                default:
                    break;
            }
        }

        if (ack.errorcode == CE_OK && !getitems.empty())
        {
            vector<string> awards = StrSpilt(getitems.c_str(), ";");

            ItemArray items;
            if (role->addAwards(awards, items, comefrom.c_str()))
            {
                ack.errorcode = 0;

                //for (int i = 0; i < items.size(); i++){
                //    obj_item item;
                //    item.itemid = items[i].item;
                //    item.count = items[i].count;
                //    ack.items.push_back(item);
                //}
            }
            else{
                ack.errorcode = CE_BAG_FULL;
            }
        }

        if (ack.errorcode == CE_OK)
        {
            //使用物品日志
            Xylog log(eLogName_UseItem, role->getInstID());
            log << __itemtype_to_string(itemtype) << item.item << 1;
            
			// 称号检查
			SHonorMou.procHonor(eHonorKey_UseItem, role, xystr("%d", item.item));
            
            ack.items = getitems;
            item.count--;
            if (item.count == 0)
            {
                item = ItemGroup();
            }
//            BackBag::UpdateBackBag(roleid, req.index, item);
            role->updateBackBagItems(req.index, item);
        }

    }while (0);

    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_sellitem, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (req.index < 0 || req.index > role->getBackBag()->GetCapacity())
    {
        return;
    }

    ack_sellitem ack;
    int index = req.index;
    int count = req.num;

    ItemGroup item = role->getBackBag()->GetItem(index);
    if (item.count < count || count <= 0){
        ack.errorcode = 1;
        ack.itemid = 0;
        ack.sgold = 0;
        sendNetPacket(sessionid, &ack);

        //踢掉
        SRoleMgr.disconnect(role, eClientCheat);
        return;
    }

    ItemCfgDef* cfg = ItemCfg::getCfg(item.item);
    if (cfg == NULL)
    {
        return;
    }

    int gold = cfg->ReadInt("sgold");
    if (gold <= 0) {
        return;
    }
    
    char buf[32] = "";
    sprintf(buf, "sell_item:%d", cfg->itemid);

    int returnGold = 0;
    if (GameFunctionCfg::getGameFunctionCfg().equipType == eMultipleEquipDrop)
    {
        int type = cfg->ReadInt("type");
        if (type == kItemEquip)
        {
            returnGold = _return_gold_if_equip_have_fusionlvl(cfg, item);
            
            Json::Reader reader;
            Json::FastWriter writer;
            
            Json::Value jsonval;
            reader.parse(item.json.c_str(), jsonval);
            
            //检查装备上的石头
            int holenum = cfg->ReadInt("holenum");
            
            ItemArray stones;
            GridArray addGffgrids;
            
            for(int stoneIndex = 1; stoneIndex <= holenum; stoneIndex++)
            {
                char key[32];
                sprintf(key, "stone%d", stoneIndex);
                int stoneid = jsonval[key].asInt();
                if (stoneid <= 0)
                {
                    continue;
                }
                jsonval[key] = 0;
                stones.push_back(ItemGroup(stoneid, 1));
            }
            
            if(role->preAddItems(stones, addGffgrids))
            {
                ack.errorcode = CE_BAG_FULL;
                sendNetPacket(sessionid, &ack);
                return;
            }

            //计算价格
            int starlvl = jsonval["star"].asInt();
            if (starlvl > 0)
            {
                int qua = cfg->ReadInt("qua");
                int part = cfg->ReadInt("part");
                int lvl = cfg->ReadInt("lvl");

                StarCostCfgDef* def = StarCfgNew::getCostCfg(lvl, qua, part);
                if (def == NULL)
                {
                    return;
                }

                int starSellGold = def->getSellGold(starlvl);
                gold += starSellGold;
            }
            
            item.json = writer.write(jsonval);
            
            //删除装备上的石头
//            BackBag::UpdateBackBag(roleid, index, item);
            role->updateBackBagItems(index, item);
            
            //把脱下的石头放到背包
//            BackBag::UpdateBackBag(roleid, addGffgrids);
            role->updateItemsChange(addGffgrids);
        }
    }
    
    { //use item log
        Xylog ulog(eLogName_UseItem, role->getInstID());
        ulog << "sell_item" << item.item << count;
    }
    
    int sumGold = count * gold + returnGold;

    item.count -= count;
    ack.itemid = item.item;
    ack.sgold = sumGold;
    if (item.count == 0){
        item.item = 0;
    }
//    BackBag::UpdateBackBag(roleid, index, item);
    role->updateBackBagItems(index, item);
    
    role->addGold(sumGold, buf);

    ack.errorcode = 0;
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_hang_up, req)
{

    hand_Sid2Role_check(sessionid, roleid, role)
    ack_hang_up ack;
    int copyId =req.copyId;
    SceneCfgDef* _sceneCfg = SceneCfg::getCfg(copyId);

    if (_sceneCfg == NULL) {
        ack.errorCode = CE_READ_SCENE_CFG_ERROR;
        sendNetPacket(role->getSessionId(), &ack);
        return;
    }

    if (_sceneCfg->sceneType != stCopy && _sceneCfg->sceneType != stEliteCopy) {
        SRoleMgr.disconnect(role);
        return;
    }

    if (_sceneCfg->minLv > role->getLvl())
    {
        SRoleMgr.disconnect(role);
        return;
    }

    //去掉战斗力限制
    //if ( _sceneCfg->sceneType == stCopy && _sceneCfg->fightpoint > role->getBattleForce())
    //{
    //    ack.errorCode = CE_YOUR_BATTLEFORCE_IS_TOO_LOW;
    //    sendNetPacket(role->getSessionId(), &ack);
    //    return;
    //}

    if (_sceneCfg->sceneType == stCopy) {
        if (req.copyId > role->getLastFinishCopy()) {
            ack.errorCode = CE_COPY_NOT_PASS;
            sendNetPacket(role->getSessionId(), &ack);
            return;
        }
    }

    //精英副本
    if (_sceneCfg->sceneType == stEliteCopy) {
        if ( !role->IsEliteCopyFinish(copyId)) {
            ack.errorCode = CE_COPY_NOT_PASS;
            sendNetPacket(role->getSessionId(), &ack);
            return;
        }

        VipPrivilegeCfgDef* vipData = VipPrivilegeCfg::getCfg(role->getVipLvl());
        if (!vipData->hangupElite && role->getLvl() < _sceneCfg->wipeOutLvl) {
            ack.errorCode = CE_YOUR_VIPLVL_IS_LOWER;
            sendNetPacket(role->getSessionId(), &ack);
            return;
        }
    }

    int fcindex = 0;
    int flopid = req.copyId;
    int flopIndexs = 1;
    int hangtimes = 0;

    if (_sceneCfg->sceneType == stEliteCopy) {
        flopid = req.copyId*10 + 3;
    }

    for (int i = 0; i < req.hangTimes; i++) {

        ItemArray items;
        RewardStruct reward;

        vector<int> dropindices;
        string dropStr = "";
		string awardBase = SflopcardMgr.RandomCard(flopid,fcindex, dropindices, dropStr);
		string awardResult = awardBase;

		SCopyExtraAward.extraItemsAward(role->getInstID(), _sceneCfg->sceneId, awardBase, awardResult);
		
        std::vector<std::string> rewards = StrSpilt(awardResult, ";");
        rewardsCmds2ItemArray(rewards,items,reward);

        GridArray effgrids;
        //检查是否可以发放奖励
        int canAddItem = role->preAddItems(items,effgrids);

        if (role->getFat() < _sceneCfg->energy_consumption) {
            ack.errorCode = CE_FAT_NOT_ENOUGH;
            break;
        }

        if (_sceneCfg->sceneType == stEliteCopy) {
            if (role->getEliteTimes() + role->getIncrEliteCopyTimes() <= 0) {
                ack.errorCode = CE_ELITETIMES_USEUP;
                break;
            }
        }
        if (canAddItem == CE_OK) {

            //扣精力值
            role->addFat( - _sceneCfg->energy_consumption);
            
            char buf[32] = "";
            sprintf(buf, "hang_up:%d", req.copyId);
            //BackBag::UpdateBackBag(role->getInstID(), effgrids);
            
            reward.reward_exp += _sceneCfg->copy_exp;
            reward.reward_gold += _sceneCfg->copy_gold;
            reward.reward_batpoint += _sceneCfg->copy_battlepoint;
            reward.reward_consval += _sceneCfg->copy_protoss;
            reward.reward_petexp += _sceneCfg->pet_exp;
			
			// 副本活动奖励
			SCopyExtraAward.extraBaseAward(role->getInstID(), _sceneCfg, reward);
            
            role->addExp(reward.reward_exp, buf);
            role->addGold(reward.reward_gold, buf);
            role->addBattlePoint(reward.reward_batpoint, buf);
            role->addConstellVal(reward.reward_consval, buf);
            role->addPetExp(role->getActivePetId(), reward.reward_petexp, buf);
            role->addEnchantDust(reward.reward_enchantdust, buf);
            role->addEnchantGold(reward.reward_enchantgold, buf);
            
//            role->updateBackBag(effgrids, items, true, buf);
            role->playerAddItemsAndStore(effgrids, items, buf, true);

            obj_copyaward copyaward;
            copyaward.flopid = flopid;
            copyaward.findex = fcindex;
            copyaward.dropindices = dropindices;
            
            copyaward.exp = reward.reward_exp;
            copyaward.gold = reward.reward_gold;
            copyaward.battlepoint = reward.reward_batpoint;
            copyaward.constell = reward.reward_consval;
            copyaward.petexp = reward.reward_petexp;
			copyaward.awardItems = awardResult;
            copyaward.enchantdust = reward.reward_enchantdust;
            copyaward.enchantgold = reward.reward_enchantgold;
            
            ack.copyawards.push_back(copyaward);
            
            //LogMod::addLogGetItem(roleid, role->getRolename().c_str(), role->getSessionId(), items, buf);
        
            UpdateQuestProgress("finishcopy", copyId, req.hangTimes, role, true);
            if (_sceneCfg->sceneType == stEliteCopy){
                UpdateQuestProgress("finishelitecopy", copyId, 1, role, true);
                if (role->getDailyScheduleSystem()) {
                    role->getDailyScheduleSystem()->onEvent(kDailyScheduleEliteCopy, 0, 1);
                }
            }

            if (_sceneCfg->sceneType == stEliteCopy)
            {
                role->addEliteTimes(-1);
                string describe = Utils::makeStr("hang_up:%d", _sceneCfg->sceneId);
                role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesEliteCopy, describe.c_str());
            }

            hangtimes++;

            flopIndexs = flopIndexs * 10 + fcindex;

            ack.errorCode = 0;
        }
        else
        {
            ack.errorCode = CE_BAG_FULL;
            break;
        }
    }

    ack.copyid = req.copyId;
    ack.hangTimes = hangtimes;
    sendNetPacket(role->getSessionId(), &ack);

}}

handler_msg(req_getPaihangData, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    int paihangType = req.paihangType;
    int index = req.job;
    int beginRank = req.beginRank;
    int endRank = req.endRank;
    
    if (req.isCrossService) {
        req_cs_sendRankList req;
        req.clientSession   = sessionid;
        req.beginRank       = beginRank;
        req.endRank         = endRank;
        req.type            = paihangType;
        req.roleid          = roleid;
        req.petid           = role->getActivePetId();
        
        sendNetPacket(CenterClient::instance()->getSession(), &req);
        
    } else {
    struct PaihangMsgData *msg = new struct PaihangMsgData;

    msg->roleid = MAKE_OBJ_ID(kObjectRole, roleid);
    msg->paihangType    = paihangType;
    msg->job            = index;
    msg->beginRank      = beginRank;
    msg->endRank        = endRank;
    msg->sessionid      = sessionid;

    create_global(ClientGetPaihangData, paihang);
    paihang->data = (void*)msg;
    paihang->sessionid = role->getSessionId();
    sendGlobalMsg(Global::MQ, paihang);
    }
}}

hander_msg(req_checkCrossServiceRankListOpen, req)
{
    req_getCrossServiceRankListStatus reqGetStatus;
    reqGetStatus.types = req.types;
    reqGetStatus.clientSession = sessionid;
    
    sendNetPacket(CenterClient::instance()->getSession(), &reqGetStatus);
}}

hander_msg(req_crossServiceRankListTopThreeFashion, req)
{
    
    set<int>  onReqServerIdList;
    
    for (int i = 0; i < req.roleFashionInfos.size(); i++) {
        onReqServerIdList.insert(req.roleFashionInfos[i].serverid);
    }
    
    for (set<int>::iterator it = onReqServerIdList.begin(); it != onReqServerIdList.end(); it ++) {
        req_get_csRoleFashion_through_cs toCenterReq;
        toCenterReq.clientSession = sessionid;
        toCenterReq.doReqServerid = Process::env.getInt("server_id");
        toCenterReq.onReqServerid = *it;
        toCenterReq.rankListType  = req.rankListType;

        for (int j = 0; j < req.roleFashionInfos.size(); j ++) {
            if (req.roleFashionInfos[j].serverid == *it) {
                toCenterReq.roleFashionInfos.push_back(req.roleFashionInfos[j]);
            }
        }
        sendNetPacket(CenterClient::instance()->getSession(), &toCenterReq);
    }

}}


hander_msg(req_use_allitems, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    if (req.index < 0 || req.index > role->getBackBag()->GetCapacity())
    {
        return;
    }

    ack_use_allitems ack;
    ack.errorcode = 1;
    ack.index = req.index;
    int newPetmod = 0;
    ItemGroup item = role->getBackBag()->GetItem(req.index);
    ack.itemid = item.item;
    ack.index = req.index;
    do
    {
        if (item.item <= 0 || item.count <= 0)
        {
            break;
        }

        ItemCfgDef* cfg = ItemCfg::getCfg(item.item);
        if (cfg == NULL)
        {
            break;
        }

        string getitems = "";

        int itemtype = cfg->ReadInt("type");
        int uselvl = cfg->ReadInt("lvl");
        if (uselvl > role->getLvl()) {
            break;
        }

        std::vector<Pet*> newpets;
        newpets.clear();

        ItemArray getNewItems;
        RewardStruct itemReward;
        std::vector<int> bagChangeIndexs;
        bagChangeIndexs.clear();

        string newItemComeFrom = "";
        
        while(item.count)
        {
            
            switch (itemtype) {
                case kItemPackage:
                {
                    getitems = cfg->ReadStr("oitems");
//                    sprintf(buf, "itemPackage:%d", cfg->itemid);
                    newItemComeFrom = Utils::makeStr("itemPackage:%d", cfg->itemid);
                    break;
                }
                case kItemRandBox:
                {
                    int emptyGridNum = role->getBackBag()->EmptyGridAmount();
                    if (emptyGridNum < cfg->ReadInt("rewardnum"))
                    {
                        ack.errorcode = CE_BAG_FULL;
                        break;
                    }

                    getitems = getRandBoxItems(cfg);
//                    sprintf(buf, "randBoxItem:%d", cfg->itemid);
                    newItemComeFrom = Utils::makeStr("randBoxItem:%d", cfg->itemid);
                    break;
                }
                case kItemPetEgg:
                {
                    newPetmod = openPetEgg(cfg);
                    
                    if (role->mPetMgr->getPetsCount() >= MAX_PETS_COUNT) {
                        ack.errorcode = CE_CAGE_IS_FULL;
                        break;
                    }
                    
                    Pet* newPet = producePet(role, newPetmod);
                    if (newPet == NULL) {
                        ack.errorcode = CE_READ_CFG_ERROR;
                        break;
                    }
                    newpets.push_back(newPet);
                    ack.errorcode = CE_OK;
                    char buf[32] = "";
                    sprintf(buf, "petegg:%d", item.item);
                    LogMod::addLogGetPet(roleid, role->getRolename().c_str(), role->getSessionId(), newPetmod, newPet->petid, buf);
                    break;
                }
                case kItemConsumable:
                {
                    //判断是否使用地下城物品
                    int ret = useConsumeItem(cfg,role, 1, &getitems);
                    if (ret == 0) {
                        ack.errorcode = 0;
                        role->saveNewProperty();
                    }
                    else if(eUseResetDungeTimesItemResult_ConfigError == ret)
                    {
                        ack.errorcode = CE_READ_CFG_ERROR;
                    }
                    else if (eUseResetDungeTimesItemResult_BagCapacityFull == ret)
                    {
                        ack.errorcode = CE_BAG_CAPACITY_LESS_THAN_N_CELL;
                    }
                    //使用扫荡地下城物品，并且还没有打地下城
                    else if (eUseResetDungeTimesItemResult_NotGoDunge == ret)
                    {
                        ack.errorcode = CE_USE_DUNGEITEM_ERROR;
                    }
                    //item expired
                    else if (eUseResetDungeTimesItemResult_ItemExpired == ret)
                    {
                        ack.errorcode = CE_ITEM_EXPIRED;
                    }
                    else
                    {
                        ack.errorcode = CE_OK;
                    }
                    break;

                }
                case KItemActivityBox:
                {
                    getitems = SRandomEventsSys.getRandBox(cfg, role);
//                    sprintf(buf, "randBoxItem:%d", cfg->itemid);
                    newItemComeFrom = Utils::makeStr("ActivityBoxItem_All:%d", cfg->itemid);
                    break;
                }
                case KItemFashion:
                {
                    if (useFashion(cfg, role))
                    {
                        ack.errorcode = CE_OK;
                    }
                    else
                    {
                        ack.errorcode = CE_USE_FASHION_ERROR;
                    }
                    
                    break;
                }
                case kItemPartedByJobSex:
                {
                    ObjJob mJob = role->getJob();
                    ObjSex mSex = role->getSex();
                    string parteditem = strFormat("jobpackage%d%d", mJob, mSex);
                    string oitems = cfg->ReadStr("oitems");
                    string jobpackage = cfg->ReadStr(parteditem.c_str());
                    if (oitems == "") {
                        getitems = jobpackage;
                    } else if (jobpackage == "") {
                        getitems = oitems;
                    } else {
                        getitems = oitems + ";" + cfg->ReadStr(parteditem.c_str());
                    }
                    
                    newItemComeFrom = Utils::makeStr("按职业性别开礼包:%d", cfg->itemid);
                    break;
                }
                
                default:
                    break;
            }
            
            //不懂为什么加ack.errorcode == CE_OK的判断，以前是没有的,暂时去掉否则其他物品用不了
            //if (ack.errorcode == CE_OK && !getitems.empty())
            
            if (!getitems.empty())
            {
                vector<string> awards = StrSpilt(getitems.c_str(), ";");

                ItemArray tmpItems = getNewItems;
                
                RewardStruct reward;
                rewardsCmds2ItemArray(awards, tmpItems, reward);

                GridArray effgrids;
                if(role->preAddItems(tmpItems, effgrids) != CE_OK)
                {
                    ack.errorcode = CE_BAG_FULL;
                    break;
                }
                else{
                    
                    itemReward = itemReward + reward;

//                    //先更新内存中得背包信息
//                    role->playerAddItemsAndStore(effgrids, items, buf, true);
//
//                    LogMod::addLogGetItem(roleid, role->getRolename().c_str(), role->getSessionId(), items, buf);

//                    for (int i = 0; i < items.size(); i++){
//                        getNewItems.push_back(items[i]);
//                    }
                    
                    getNewItems = tmpItems;
                    
                    ack.errorcode = CE_OK;

                }
            }

            if (ack.errorcode == CE_OK) {
                item.count--;
                
                //使用物品日志
                Xylog log(eLogName_UseItem, role->getInstID());
                log << __itemtype_to_string(itemtype) << item.item << 1;
				
				// 称号检查
				SHonorMou.procHonor(eHonorKey_UseItem, role, xystr("%d", item.item));
            }
            else{
                break;
            }
        }   //while
        
        if (ack.errorcode == CE_OK)
        {
            switch (itemtype) {
                case KItemFashion:
                {
                    Wardrobe* wardrobe = role->getWardrobe();
                    
                    notify_wardrobe_exp notify;
                    notify.lvl = wardrobe->getLvl();
                    notify.exp = wardrobe->getExp();
                    sendNetPacket(sessionid, &notify);
                    break;
                }
                default:
                    break;
            }
        }

        GridArray effeGrid;
        role->preAddItems(getNewItems, effeGrid);
        role->playerAddItemsAndStore(effeGrid, getNewItems, newItemComeFrom.c_str(), true);
        
        //删除背包物品
        if (item.count == 0)
        {
            item = ItemGroup();
        }
//        BackBag::UpdateBackBag(roleid, req.index, item);
        role->updateBackBagItems(req.index, item);
        
        //推送宠物列表
        notifyNewPets(role, newpets);

        //给玩家发放金钱，经验，宠物经验等
        string comefrom = Utils::makeStr("物品使用:%d", item.item);
        role->onAddItemAward(comefrom.c_str(), itemReward);

        if (getNewItems.size()) {
            ItemArray tempItems;

            mergeItems(tempItems, getNewItems);

            for (int i = 0; i < getNewItems.size(); i++) {
                obj_item getitem;
                getitem.itemid = getNewItems[i].item;
                getitem.count = getNewItems[i].count;
                ack.items.push_back(getitem);
            }
        }
        ack.count = item.count;

    }while (0);

    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_fusionEquip, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_fusionEquip ack;
    ack.errorcode = CE_OK;
    do
    {
        ItemGroup item = role->getPlayerEquip()->GetItem(req.equipindex);
        
        ItemCfgDef* cfg = ItemCfg::getCfg(item.item);
        if (cfg == NULL)
        {
            ack.errorcode = CE_READ_CFG_ERROR;
            break;
        }
        
        Json::Reader reader;
        Json::FastWriter writer;
        
        Json::Value jsonval;
        reader.parse(item.json.c_str(), jsonval);
        
        int fusionLvl = jsonval["fusionlvl"].asInt();
        int equiplvl = cfg->ReadInt("lvl");
        int equipQua = cfg->ReadInt("qua");
        int equipPart = cfg->ReadInt("part");
        
        int maxFusionLvl = EquipFusionCfg::getMaxLvl(equiplvl, equipQua, equipPart);
        
        if (fusionLvl >= maxFusionLvl) {
            ack.errorcode = CE_EQUIP_FUSION_IS_MAX;
            break;
        }
        
        int needItemCount = EquipFusionCfg::getNeedEquipCount(fusionLvl + 1, equiplvl, equipQua, equipPart);
        int neeGold = EquipFusionCfg::getCostGold(fusionLvl + 1, equiplvl, equipQua, equipPart);
        
        ItemArray previtems;
        GridArray prevgrids;
        previtems.push_back(ItemGroup(item.item, needItemCount));
        if (!role->getBackBag()->PreDelItems(previtems, prevgrids))
        {
            ack.errorcode = CE_ITEM_NOT_ENOUGH;
            break;
        }
        
        if (role->CheckMoneyEnough(neeGold, eGoldCost, "equip_fusion") != CE_OK) {
            ack.errorcode = CE_SHOP_GOLD_NOT_ENOUGH;
            break;
        }
        
        //摘掉宝石
        for (int i = 0; i < prevgrids.size(); i++) {
            if (!role->deattachEquipInbag(prevgrids[i].index)) {
                ack.errorcode = CE_BAG_FULL;
                break;
            }
        }
        
        if (ack.errorcode) {
            break;
        }
        
        
        //{ //use item log
        //    LogMod::addUseItemLog(role, previtems, "fusion_equip");
        //}
        
        //BackBag::UpdateBackBag(roleid, prevgrids);
//        role->updateBackBag(prevgrids, previtems, false, "fusion_equip");
        role->playerDeleteItemsAndStore(prevgrids, previtems, "fusion_equip", true);
        
        jsonval["fusionlvl"] = fusionLvl + 1;
        item.json = writer.write(jsonval);
        
        PlayerEquip::UpdatePlayerEquip(roleid, req.equipindex, item);
        
        role->CalcPlayerProp();
        
    }while(false);
    
    sendNetPacket(sessionid, &ack);
}}


handler_msg(req_activate_baggrids, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_activate_baggrids ack;
    ack.errorcode = CE_UNKNOWN;
    
    int activePage = req.activatepage;
    do {
        int curRmbExtendedPage = role->getRmbExtendedBackbagSize() / BackBag::sBackbagSizePerPage;
        int finalRmbExtendedPage = activePage + curRmbExtendedPage;
        if (activePage <= 0 || finalRmbExtendedPage > ActivateBagGridsCfg::getMaxPage()) {
            ack.errorcode = CE_REQ_PARAM_ERROR;
            break;
        }
        
        int totalCost = 0;
        int i = 1;
        for (; i <= activePage; i++) {
            int pageCost =  ActivateBagGridsCfg::getCfg(i + curRmbExtendedPage);
            if (pageCost <= 0) {
                ack.errorcode = CE_SYSTEM_ERROR;
                break;
            }
            totalCost += pageCost;
        }
        if (i <= activePage) {
            break;
        }
        
        if (role->getRmb() < totalCost) {
            ack.errorcode = CE_SHOP_RMB_NOT_ENOUGH;
            break;
        }
        role->addRmb(-totalCost, "active bag");
        
        int finalRmbExtendSize = finalRmbExtendedPage * BackBag::sBackbagSizePerPage;
        int bagcapacity = finalRmbExtendSize + BackBag::sFreeBackbagSize;
        
        BackBag* backbag = role->getBackBag();
        backbag->extendTo(bagcapacity);
        role->setRmbExtendedBackbagSize(finalRmbExtendSize);
        role->saveNewProperty();
        
        ack.errorcode = CE_OK;
        ack.bagsize = backbag->GetCapacity();
        ack.rmbextendedsize = finalRmbExtendSize;
        
    } while (0);
    
    role->send(&ack);
}}

hander_msg(req_role_awake_lvlup, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_role_awake_lvlup ack;
    ack.errorcode = role->getRoleAwake()->awakeLvlUp();
    ack.awakelvl = role->getRoleAwakeLvl();
    
    if(ack.errorcode == CE_OK)
    {
        role->getRoleAwake()->allSkillReplace();
    }
    
    sendNetPacket(sessionid, &ack);
}}

