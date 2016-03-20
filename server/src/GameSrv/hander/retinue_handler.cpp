//
//  retinue_handler.cpp
//  GameSrv
//
//  Created by pireszhi on 15-6-24.
//
//

#include <stdio.h>
#include "Retinue.h"
#include "hander_include.h"
#include "flopcard.h"

handler_msg(req_get_retinues_info, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_get_retinues_info ack;
    
    role->getRetinueMgr()->clientGetRetinueList(ack.retinues);
    
    sendNetPacket(sessionid, &ack);
    
}}

handler_msg(req_get_retinue_skills, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_get_retinue_skills ack;
    
    role->getRetinueMgr()->mSkillMgr->clientGetSkillsInfo(ack.skills);
    
    sendNetPacket(sessionid, &ack);
    
}}

handler_msg(req_compose_retinue, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_compose_retinue ack;
    
    int targetRetinueId = req.targetId;
    
    RetinueModDef* modCfg = RetinueCfg::getRetinueModCfg(targetRetinueId);
    
    if (modCfg == NULL) {
        ack.errorcode = CE_RETINUE_MOD_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
    }
    
    ItemArray items;
    //检查碎片是是否足够
    for (int i = 0; i < modCfg->mNeedMaterials.size(); i++) {
        
        int piecesId = modCfg->mNeedMaterials[i].itemid;
        int needCount = modCfg->mNeedMaterials[i].count;
        
        ItemGroup item = ItemGroup(piecesId, needCount);
        items.push_back(item);
    }
    
    GridArray effgrids;
    if ( !role->getRetinueMgr()->mRetinueBag->PreDelItems(items, effgrids)) {
        ack.errorcode = CE_RETINUE_MATERIAL_NOT_ENOUGH;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    //产生侍魂
    Retinue* newRetinue = RetinueMgr::Create(targetRetinueId);
    if (newRetinue == NULL) {
        ack.errorcode = CE_RETINUE_COMPOSE_FAILD;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    //扣除物品
    role->playerDeleteItemsAndStore(effgrids, items, "侍魂合成", true);
    
    //给侍魂
    role->getRetinueMgr()->addRetinue(newRetinue);
    role->getRetinueMgr()->save();

    newRetinue->getRetinueInfo(ack.newRetinue);
    ack.errorcode = CE_OK;
    sendNetPacket(sessionid, &ack);
    
    string comsume = "";
    for (int i = 0; i < items.size(); i++) {
        comsume.append(strFormat("item %d*%d;", items[i].item, items[i].count));
    }
    LogMod::addLogRetinueCompose(roleid, newRetinue->getId(), newRetinue->getModId(), newRetinue->retinueInfo2String().c_str(), comsume.c_str());
}}

handler_msg(req_retinue_lvlup, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_retinue_lvlup ack;
    
    Retinue* retinue = role->getRetinueMgr()->getRetinue(req.retinueId);
    
    //是否存在这个侍魂
    if (NULL == retinue) {
        ack.errorcode = CE_RETINUE_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    RetinueModDef* cfg = RetinueCfg::getRetinueModCfg(retinue->getModId());
    
    //不存在侍魂的配置文件
    if (cfg == NULL) {
        ack.errorcode = CE_READ_CFG_ERROR;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (retinue->getLvl() >= cfg->maxLvl) {
        ack.errorcode = CE_RETINUE_IS_MAXLVL;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    int totalAddExp = 0;
    ItemArray items;

    //验证材料是否合法
    if (!cfg->checkRetinueLvlupMaterial(req.materialId)) {
        ack.errorcode = CE_RETINUE_MATERIAL_NOT_RIGHT;
        sendNetPacket(sessionid, &ack);
        return;
    }
    ItemCfgDef* itemCfg = ItemCfg::getCfg(req.materialId);
    
    //是否存在这种材料
    if (NULL == itemCfg) {
        ack.errorcode = CE_READ_CFG_ERROR;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    //材料是否足够
    items.push_back(ItemGroup(req.materialId, 1));

    totalAddExp += itemCfg->ReadInt("retinue_exp");
    
    GridArray effeGrid;
    if (!role->getRetinueMgr()->mRetinueBag->PreDelItems(items, effeGrid)) {
        ack.errorcode = CE_RETINUE_MATERIAL_NOT_ENOUGH;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    string usage = Utils::makeStr("侍魂升级:%d", req.retinueId);
    role->playerDeleteItemsAndStore(effeGrid, items, usage.c_str(), true);
    
    string beforeRetinueInfo = retinue->retinueInfo2String();
    int beforExp = retinue->getExp();
    
    int oldLvl = retinue->getLvl();
    retinue->addExp(totalAddExp);
    
    if (oldLvl != retinue->getLvl()) {
        retinue->calcProp();
        role->CalcPlayerProp();
    }
    
    string afterRetinueInfo = retinue->retinueInfo2String();
    int afterExp = retinue->getExp();
    
    sendRetinueNotify(role, retinue);
    ack.errorcode = CE_OK;
    sendNetPacket(sessionid, &ack);
    
    if (ack.errorcode == CE_OK) {
        
        string comsume;
        for (int i = 0; i < items.size(); i++) {
            comsume.append(strFormat("item %d*%d;", items[i].item, items[i].count));
        }
        
        LogMod::addLogRetinueAddExp(roleid, retinue->RetinueData::getId(), totalAddExp, beforExp, afterExp, beforeRetinueInfo.c_str(), afterRetinueInfo.c_str(), comsume.c_str());
    }
}}

handler_msg(req_retinue_starup, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_retinue_starup ack;
    
    Retinue* retinue = role->getRetinueMgr()->getRetinue(req.retinueId);
    
    //是否存在这个侍魂
    if (NULL == retinue) {
        ack.errorcode = CE_RETINUE_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    RetinueModDef* cfg = RetinueCfg::getRetinueModCfg(retinue->getModId());
    
    //不存在侍魂的配置文件
    if (cfg == NULL) {
        ack.errorcode = CE_RETINUE_MOD_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (retinue->getStar() >= cfg->maxStar)
    {
        ack.errorcode = CE_RETINUE_IS_MAXSTAR;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    int starupMaterialId = cfg->getStarUpMaterial(retinue->getStar() + 1);
    int uniStarupMaterialId = cfg->getUniStarupMaterial(retinue->getStar() + 1);
    
    ItemCfgDef* materialCfg = ItemCfg::getCfg(starupMaterialId);
    ItemCfgDef* uniMaterialCfg = ItemCfg::getCfg(uniStarupMaterialId);
    
    //是否存在这种材料
    if (NULL == materialCfg || NULL == uniMaterialCfg) {
        ack.errorcode = CE_READ_CFG_ERROR;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    int myMaterialCount = role->getRetinueMgr()->mRetinueBag->GetItemNum(starupMaterialId);
    
    int myUniCount = role->getRetinueMgr()->mRetinueBag->GetItemNum(uniStarupMaterialId);
    
    if ((myMaterialCount + myUniCount) < retinue->getNextStarupExp()) {
        
        ack.errorcode = CE_RETINUE_MATERIAL_NOT_ENOUGH;
        sendNetPacket(sessionid, &ack);
        return;
        
    }
    
    ItemArray needItems;
    GridArray effeGrib;
    
    if (myMaterialCount < retinue->getNextStarupExp()) {
        
        needItems.push_back(ItemGroup(starupMaterialId, myMaterialCount));
        
        needItems.push_back(ItemGroup(uniStarupMaterialId, retinue->getNextStarupExp() - myMaterialCount));
    }
    else
    {
        needItems.push_back(ItemGroup(starupMaterialId, retinue->getNextStarupExp()));
    }
    
    if (!role->getRetinueMgr()->mRetinueBag->PreDelItems(needItems, effeGrib)) {
        ack.errorcode = CE_RETINUE_MATERIAL_NOT_ENOUGH;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    string beforeRetinueInfo = retinue->retinueInfo2String();
    int beforStar = retinue->getStar();
    
    if (retinue->starUp()) {

        string usage = Utils::makeStr("侍魂升星:%d", retinue->getId());
        role->playerDeleteItemsAndStore(effeGrib, needItems, usage.c_str(), true);
        
        retinue->calcProp();
        role->CalcPlayerProp();
        sendRetinueNotify(role, retinue);

    }
    
    string afterRetinueInfo = retinue->retinueInfo2String();
    int afterStar = retinue->getStar();
    
    ack.errorcode = CE_OK;
    sendNetPacket(sessionid, &ack);
    
    if (ack.errorcode == CE_OK) {
        
        string comsume;
        for (int i = 0; i < needItems.size(); i++) {
            comsume.append(strFormat("item %d*%d;", needItems[i].item, needItems[i].count));
        }
        
        LogMod::addLogRetinueStarup(roleid, retinue->RetinueData::getId(), beforStar, afterStar, beforeRetinueInfo.c_str(), afterRetinueInfo.c_str(), comsume.c_str());
    }
    
}}

handler_msg(req_get_retinue_equiped_skills, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_get_retinue_equiped_skills ack;
    
    role->getRetinueMgr()->mSkillMgr->clientGetEquipedSkills(ack.equipedSkills);
    
    sendNetPacket(sessionid, &ack);
    
}}

handler_msg(req_study_retinue_skill, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_study_retinue_skill ack;
    
    int skillId = req.skillid;
    int targetLvl = req.skillLvl;
    
    RetinueSkillDef* skillCfg = RetinueCfg::getRetinueSkillCfg(skillId);
    if (skillCfg == NULL) {
        ack.errorcode = CE_RETINUE_SKILL_CFG_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    RetinueSkillInfo* skill = role->getRetinueMgr()->mSkillMgr->getSkillInfo(skillId);
    
    int beginLvl = 0;
    if (skill) {
        beginLvl = skill->lvl;
    }
    
    if (beginLvl >= targetLvl) {
        return;
    }
    
    if (targetLvl > skillCfg->maxLvl) {
        ack.errorcode = CE_RETINUE_SKILL_IS_MAXLVL;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    int lastLvl = beginLvl;
    ItemArray logNeedItems;
    
    for (int i = beginLvl + 1; i <= targetLvl; i++) {
        
        ItemCell material = skillCfg->getUpgMaterial(i);
        ItemCell uniUpgMaterial = skillCfg->getUniUpgMaterial(i);
        
        ItemArray needItems;
        
        int myCount = role->getRetinueMgr()->mRetinueBag->GetItemNum(material.itemid);
        GridArray effeGrib;
        
        //学第一级的时候只能用专用材料
        int uniCount = 0;
        if (beginLvl > 0)
        {
            uniCount = role->getRetinueMgr()->mRetinueBag->GetItemNum(uniUpgMaterial.itemid);
        }
        
        if ((myCount + uniCount) < material.count)
        {
            break;
        }
        
        if (myCount < material.count) {
            
            needItems.push_back(ItemGroup(material.itemid, myCount));
            
            needItems.push_back(ItemGroup(uniUpgMaterial.itemid, material.count - myCount));
        }
        else
        {
            needItems.push_back(ItemGroup(material.itemid, material.count));
        }
        
        if (!role->getRetinueMgr()->mRetinueBag->PreDelItems(needItems, effeGrib)) {
            break;
        }
        
        role->getRetinueMgr()->mRetinueBag->UpdateBackBag(effeGrib);
        
        string desc = Utils::makeStr("侍魂技能升级:%d", skillId);
        role->playerDeleteItemsAndStore(effeGrib, needItems, desc.c_str(), true);
        
        lastLvl = i;
        
        for (int k = 0; k < needItems.size(); k++) {
            logNeedItems.push_back(needItems[k]);
        }
    }
    
    if (lastLvl == beginLvl) {
        ack.errorcode = CE_RETINUE_MATERIAL_NOT_ENOUGH;
    }
    else
    {
        role->getRetinueMgr()->mSkillMgr->upgSkill(skillId, lastLvl - beginLvl);
        ack.errorcode = CE_OK;
    }
    ack.skillid = req.skillid;
    ack.skillLvl = lastLvl;

    sendNetPacket(sessionid, &ack);
    
    if (ack.errorcode == CE_OK) {
        
        string comsume;
        for (int i = 0; i < logNeedItems.size(); i++) {
            comsume.append(strFormat("item %d*%d;", logNeedItems[i].item, logNeedItems[i].count));
        }
        
        LogMod::addLogRetinueSkillStudy(roleid, skillId, beginLvl, lastLvl, comsume.c_str());
    }
}}

handler_msg(req_get_spiritspeak_info, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_get_spiritspeak_info ack;
    
    role->getRetinueMgr()->mSpiritSpeakMgr->clientGetSpiritSpeakInfo(ack.spiritspeaks);
    
    sendNetPacket(sessionid, &ack);
    
}}

handler_msg(req_do_spiritspeak, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_do_spiritspeak ack;
    
    SpiritSpeakDef* cfg = RetinueCfg::getSpiritSpeakCfg(req.spiritspeakId);
    
    if (cfg == NULL) {
        ack.errorcode = CE_SPIRITSPEAK_CFG_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (cfg->gribCount <= req.position) {
        ack.errorcode = CE_SPIRITSPEAK_POSITION_NOT_RIGHT;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (req.retinueId > 0) {
        Retinue* retinue = role->getRetinueMgr()->getRetinue(req.retinueId);
        if (retinue == NULL) {
            ack.errorcode = CE_RETINUE_NOT_EXIST;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        //该侍魂已经被用于通灵
        if (retinue->getSpiritSpeakId()) {
            ack.errorcode = CE_SPIRITSPEAK_RETINUE_BEING_USED;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        RetinueModDef* modCfg = RetinueCfg::getRetinueModCfg(retinue->getModId());
        if (modCfg == NULL) {
            ack.errorcode = CE_RETINUE_MOD_NOT_EXIST;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        //检查位置和阶数
        if (modCfg->steps != cfg->retinueStep || modCfg->position != req.position) {
            ack.errorcode = CE_SPIRITSPEAK_POSITION_NOT_RIGHT;
            sendNetPacket(sessionid, &ack);
            return;
        }
    }
    
    if (!role->getRetinueMgr()->mSpiritSpeakMgr->setSpiritSpeak(req.spiritspeakId, req.position, req.petId, req.retinueId)) {
        
        ack.errorcode = CE_SPIRITSPEAK_PARAM_NOT_RIGHT;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    role->CalcPlayerProp();
    role->getRetinueMgr()->mSpiritSpeakMgr->clientGetSingleSpiritSpeakInfo(req.spiritspeakId, ack.newSpiritspeak);
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_get_retinue_summon_info, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    notify_retinue_summon_info notify;
    
    notify.cooldown = role->getRetinueSummonCooldown() - Game::tick;
    
    check_min(notify.cooldown, 0);
    
    notify.remianRemoveTimes = RetinueCfg::getRetinueSummonResetTimes(0) - role->getRetinueSummonResetTimes();
    check_min(notify.remianRemoveTimes, 0);
    
    sendNetPacket(sessionid, &notify);
}}

handler_msg(req_remove_retinue_summon_cooldown, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_remove_retinue_summon_cooldown ack;
    
    if (role->getRetinueSummonCooldown() == 0) {
        ack.errorcode = CE_SUMMON_NOT_COOLING_DOWN;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    RetinueSummonDef* cfg = RetinueCfg::getRetinueSummonCfg(req.summonId);
    
    if (cfg == NULL) {
        ack.errorcode = CE_SUMMON_CFG_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    //次数验证
    if (role->getRetinueSummonResetTimes() >= cfg->resetTimes) {
        ack.errorcode = CE_SUMMON_RESETTIME_USEUP;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    //验证
    if (role->getRmb() < cfg->resetCost) {
        ack.errorcode = CE_SHOP_RMB_NOT_ENOUGH;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    role->addRmb(-(cfg->resetCost), "消除聚魂冷却");
    role->setRetinueSummonResetTimes(role->getRetinueSummonResetTimes() + 1);
    role->setRetinueSummonCooldown(0);
    role->NewRoleProperty::save();
    
    notify_retinue_summon_info notify;
    notify.cooldown = 0;
    notify.remianRemoveTimes = cfg->resetTimes - role->getRetinueSummonResetTimes();
    
    check_min(notify.remianRemoveTimes, 0);
    sendNetPacket(sessionid, &notify);
    
    ack.errorcode = CE_OK;
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_retinue_summon, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_retinue_summon ack;
    
    RetinueSummonDef* summonCfg = RetinueCfg::getRetinueSummonCfg(req.summonId);
    
    if (summonCfg == NULL) {
        ack.errorcode = CE_SUMMON_CFG_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (summonCfg->roleLvl > role->getLvl()) {
        ack.errorcode = CE_ROLE_LVL_TOO_LOW;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (summonCfg->cooldown > (Game::tick - role->getRetinueSummonCooldown())) {
        ack.errorcode = CE_SUMMON_COOLING_DOWN;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    int flopId = 0;
    string summonDesc = "";
    if (req.summonType == 1) {
        
        if (role->getRmb() < summonCfg->rmbSummonCost) {
            
            ack.errorcode = CE_SHOP_RMB_NOT_ENOUGH;
            sendNetPacket(sessionid, &ack);
            return;
            
        }
        
        flopId = summonCfg->rmbSummon;
        summonDesc = "高级聚魂";
        
    }
    else
    {
        flopId = summonCfg->ordinarySummon;
        summonDesc = "普通聚魂";
    }
    
    int fcindex = 0;
    string dropStr = "";
    vector<int> drops;
    
    string itemStr = SflopcardMgr.RandomCard(flopId, fcindex, drops, dropStr, 0);
    
    ItemArray items;
    RewardStruct reward;
    std::vector<std::string> rewards = StrSpilt(itemStr, ";");
    rewardsCmds2ItemArray(rewards, items, reward);
    
    if (!role->addAwards(rewards, items, summonDesc.c_str())) {
        ack.errorcode = CE_BAG_FULL;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (req.summonType == 1) {
        
        role->addRmb( -(summonCfg->rmbSummonCost));
        
    }
    
    role->setRetinueSummonCooldown(Game::tick + summonCfg->cooldown);
    
    role->NewRoleProperty::save();
    
    ack.errorcode = CE_OK;
    ack.retinuePieces = itemStr;
    sendNetPacket(sessionid, &ack);
    
    notify_retinue_summon_info notify;
    notify.cooldown = summonCfg->cooldown;
    notify.remianRemoveTimes = summonCfg->resetTimes - role->getRetinueSummonResetTimes();
    sendNetPacket(sessionid, &notify);
    
    LogMod::addLogRetinueSummon(roleid, req.summonId, req.summonType, itemStr.c_str());
}}

handler_msg(req_get_retinue_bag, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    const ItemArray& bagitems = role->getRetinueMgr()->mRetinueBag->GetItems();
    ack_get_retinue_bag ack;
    ack.errorcode = 0;

    ack.boxsize = role->getRetinueMgr()->mRetinueBag->GetCapacity();
    
    for (int i = 0; i < ack.boxsize; i++)
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
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_use_retinue, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_use_retinue ack;
    ack.retinueid = req.retinueid;
    int errorcode = 0;
    
    Retinue* activeRetinue = role->getRetinueMgr()->getActiveRetinue();
    
    //把原来的取消
    if (activeRetinue) {
        activeRetinue->setActive(false);
        activeRetinue->RetinueData::save();
        
        role->getRetinueMgr()->setActiveRetinue(NULL);
    }
    
    //如果是出战，设置出战
    if(req.isuse){
        
        Retinue * retinue = role -> getRetinueMgr() -> getRetinue(req.retinueid);
        
        if (retinue == NULL) {
            errorcode = CE_RETINUE_NOT_EXIST;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        retinue -> setActive(req.isuse);
        retinue -> RetinueData::save();
        
        role->getRetinueMgr()->setActiveRetinue(retinue);
        
    }
    
    ack.isuse = req.isuse;
    ack.errorcode = errorcode;
    sendNetPacket(sessionid, &ack);
    
}}

handler_msg(req_equip_retinue_skill, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_equip_retinue_skill ack;
    ack.errrorcode = CE_OK;
    int skilid = (req.retinueSkillId / 100) * 100;
    
    if (req.retinueSkillId)
    {
        if (!role->getRetinueMgr()->mSkillMgr->equipSkill(skilid, req.index)) {
            ack.errrorcode = 1;
        }
    }
    else
    {
        if (!role->getRetinueMgr()->mSkillMgr->unEquipSkill(req.index)) {
            ack.errrorcode = 1;
        }
    }
    
    ack.index = req.index;
    ack.retinueSkillId = req.retinueSkillId;
    
    sendNetPacket(sessionid, &ack);
    
}}