//
//  wardrobe_handler.cpp
//  GameSrv
//
//  Created by xinyou on 14-5-23.
//
//
#include "hander_include.h"
#include "Wardrobe.h"
#include <errno.h>

#include "FashionCollect.h"
#include "mail_imp.h"

extern obj_fashion_info getFashionObjInfo(WardrobeFashion* fashion);

//衣柜
handler_msg(req_wardrobe_info, req)
{
	hand_Sid2Role_check(sessionid, roleid, role)
    
    Wardrobe* wardrobe = role->getWardrobe();
    ack_wardrobe_info ack;
    ack.exp = wardrobe->getExp();
    ack.lvl = wardrobe->getLvl();
    ack.activebody = role->getBodyFashion();
    ack.activeweapon = role->getWeaponFashion();
    ack.activehome = role->getHomeFashion();
    
    //wardrobe->checkExpiredFashions(NULL);
    
    WardrobeFashionList* fashionLists[] = {&wardrobe->getBodyFashion(), &wardrobe->getWeaponFashion(),&wardrobe->getHomeFashion()};
    for (int i = 0; i < sizeof(fashionLists) / sizeof(WardrobeFashionList*); i++)
    {
        WardrobeFashionList& fashionList = *fashionLists[i];
        for (WardrobeFashionList::Iterator iter = fashionList.begin(); iter != fashionList.end(); iter++)
        {
            WardrobeFashion* fashion = *iter;
            obj_fashion_info info = getFashionObjInfo(fashion);
            ack.fashions.push_back(info);
        }
    }
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_puton_fashion, req)
{
	hand_Sid2Role_check(sessionid, roleid, role)
    
    Wardrobe* wardrobe = role->getWardrobe();
    
    ack_puton_fashion ack;
    
    WardrobeFashion* fashion = wardrobe->getFashion(req.fashionid);
    if (fashion)
    {
        if (fashion->mExpired)
        {
            ack.errorcode = CE_FASHION_EXPIRED;
        }
        else
        {
            ack.errorcode = CE_OK;
            switch (fashion->mType)
            {
                case eWeapon:
                    role->setWeaponFashion(req.fashionid);
                    break;
                case eWholeBody:
                    role->setBodyFashion(req.fashionid);
                    break;
                case eCityBody:
                    role->setHomeFashion(req.fashionid);
                    break;
                default:
                    ack.errorcode = CE_UNKNOWN;
                    break;
            }
        }
    }
    else
    {
        ack.errorcode = CE_PUTON_FASHION_ERROR;
    }
    
    if (ack.errorcode == CE_OK)
    {
        role->save();
    }
    
    ack.fashionid = req.fashionid;
    ack.type = req.type;
    
    sendNetPacket(sessionid, &ack);
}}


handler_msg(req_takeoff_fashion, req)
{
	hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_takeoff_fashion ack;
    
    ack.errorcode = CE_UNKNOWN;
    switch (req.type)
    {
        case eWeapon:
            if (role->getWeaponFashion() != 0)
            {
                ack.errorcode = CE_OK;
                role->setWeaponFashion(0);
            }
            break;
        case eWholeBody:
            if (role->getBodyFashion())
            {
                ack.errorcode = CE_OK;
                role->setBodyFashion(0);
            }
            break;
        case eCityBody:
            if (role->getHomeFashion()) {
                ack.errorcode = CE_OK;
                role->setHomeFashion(0);
            }
            break;
        default:
            ack.errorcode = CE_FASHION_TYPE_NOT_EXIST;
            break;
    }
    
    if (ack.errorcode == CE_OK)
    {
        role->save();
    }

    ack.type = req.type;
    sendNetPacket(sessionid, &ack);

}}

/*************************
      时   装   收   集
 ************************/


handler_msg(req_fashionCollect_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_fashionCollect_list ack;
    
    ack.errorcode = CE_OK;
    
    role->getFashionCollect()->finishProgressFailLastTime();
    
    role->getFashionCollect()->clientGetCollectionList(ack.listInfo);
    
    role->getFashionCollect()->clientGetLatestCollectionId(ack.latestCollect);
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_collectFashion, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_collectFashion ack;
    ack.objectId = req.objectId;
    
    do
    {
        FashionCollectCfgDef* cfg = FashionCollectCfg::getCfg(req.objectId);
        
        if (cfg == NULL) {
            ack.errocode = 1;
            break;
        }
        
        //预判奖励是否够空间存放
        GridArray effeGrids;
        ItemArray awardItems;
        
        awardItems = FashionCollect::getFashionCollectProduct(req.objectId, role->getRoleType());
        
        bool canAddItem = false;
        if(role->preAddItems(awardItems, effeGrids) == CE_OK)
        {
            canAddItem = true;
        }
        
        ack.errocode = role->getFashionCollect()->finishProgress(req.objectId);
        
        //发奖励
        if (ack.errocode == CE_OK) {
            
            if (canAddItem) {
                
                role->playerAddItemsAndStore(effeGrids, awardItems, "fashioncollect", true);
                
            }
            else
            {
                RewardStruct rewards;
                vector<string> attachs;
                reward2MailFormat(rewards, awardItems, NULL, &attachs);
                string attach = StrJoin(attachs.begin(), attachs.end(), ";");
                
                sendMailWithName(0, "", role->getRolename().c_str(), "bag_full", attach.c_str());
            }
            
            role->CalcPlayerProp();
            
            FashionCollectCfgDef* nextProgressCfg = FashionCollectCfg::getCfg(req.objectId + 1);
            
            //检查是否是最高阶
            if (nextProgressCfg == NULL) {
                ack.isMaxStage = true;
            }
            else{
                ack.isMaxStage = false;
            }
            
            for (int i = 0; i < awardItems.size(); i++) {
                obj_item obj;
                obj.itemid = awardItems[i].item;
                obj.count = awardItems[i].count;
                ack.items.push_back(obj);
            }
        }

    }while(false);
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_fashioncollect_get_materialbox, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    const ItemArray& boxitems = role->getFashionCollect()->getMaterialBoxInfo();
    
    ack_fashioncollect_get_materialbox ack;
    ack.errorcode = CE_OK;
    ack.boxsize = role->getFashionCollect()->getMaterialSize();
    
    
    for (int i = 0; i < ack.boxsize; i++)
    {
        if (boxitems[i].item <= 0)
        {
            continue;
        }
        
        obj_bagItem item;
        
        item.index = i;
        item.itemid = boxitems[i].item;
        item.count = boxitems[i].count;
        item.bind = boxitems[i].bind;
        item.json = boxitems[i].json;
        ack.items.push_back(item);
    }
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_puton_fashioncollect_material, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_puton_fashioncollect_material ack;
    
    int errorcode = role->getFashionCollect()->putMaterialIntoGrid(req.materialId, req.materialCount, req.objectId, req.position);
    
    if (errorcode == CE_OK) {
        
        role->CalcPlayerProp();
    }
    
    ack.errorcode = errorcode;
    sendNetPacket(sessionid, &ack);
    
    notify_fashioncollect_material_change change;
    role->getFashionCollect()->clientGetCollectionInfo(req.objectId, change.collectinfo);
    sendNetPacket(sessionid, &change);
}}

handler_msg(req_fashionCollect_prop_added, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_fashionCollect_prop_added ack;
    
    role->getFashionCollect()->sendPropAddNotify();
}}

handler_msg(req_fashionCollect_recycle_material, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_fashionCollect_recycle_material ack;
    
    ack.errorcode = role->getFashionCollect()->materialReclaim(req.grids);
    
    sendNetPacket(sessionid, &ack);
    
    //发送属性等更新
    role->getFashionCollect()->sendPropAddNotify();
    
}}

handler_msg(req_fashionCollect_recycle_single_material, req)
{
    ack_fashionCollect_recycle_single_material ack;
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack.errorcode = role->getFashionCollect()->materialReclaim(req.gridIndex, req.count);
    
    sendNetPacket(sessionid, &ack);
    
    role->getFashionCollect()->sendPropAddNotify();
}}
