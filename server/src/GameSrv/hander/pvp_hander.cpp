       //
//  pvp_hander.cpp
//  GameSrv
//
//  Created by pireszhi on 13-4-18.
//
//

#include "hander_include.h"
#include "Pvp.h"
#include "Skill.h"
#include "Global.h"
#include "GlobalMsg_def.h"
#include "PetPvp.h"
#include "RealPvpMgr.h"
#include "world_cmdmsg.h"

hander_msg(req_getPvpRank, req)
{
    hand_Sid2Role_check(sessionid,roleid,self);
    ack_getPvpRank ack;
    
    create_global(getPvpData, pvp);
    pvp->roleid = roleid;
    pvp->myAward = self->getPvpAward();
    pvp->sessionid = sessionid;
    pvp->getalldata = req.getalldata;
    pvp->sessionid = self->getSessionId();
    sendGlobalMsg(Global::MQ, pvp);
    
}}

hander_msg(req_begin_pvp, req)
{
    hand_Sid2Role_check(sessionid,roleid,role);
    if (role->getPreEnterSceneid() > 0) {
        return;
    }
    
    BattleProp selfProp;
    if ( role->getPvpTimes() <= 0) {
        ack_begin_pvp ack;
        ack.errorcode = CE_PVP_YOU_HAVE_NO_TIMES_TO_PVP;
        sendNetPacket(role->getSessionId(), &ack);
        return;
    }
    
    //判断可否进入，防止同时进入多个副本
    if (!checkCanEnterCopy(role, PVP_SCENEID)) {
        return;
    }
    
//    role->setPreEnterSceneid(PVP_SCENEID);
    
    int64_t targetId = MAKE_OBJ_ID(req.objType, req.bechallengeId);
    int64_t playerId = MAKE_OBJ_ID(kObjectRole, roleid);
    create_global(beginPvp, pvp);
    pvp->roleid = playerId;
    pvp->bechallengeid = targetId;
    pvp->bechallengeRank = req.bechallengeRank;
    sendGlobalMsg(Global::MQ, pvp);
    
}}

hander_msg(req_PvpResult, req)
{
    hand_Sid2Role_check(sessionid,roleid,role);
    
    if ( PVP_SCENEID != role->getCurrSceneId()) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    if (role->getBattleTime() == 0) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    role->setBattleTime(0);
    role->backToCity();
    
    int battleTime = Game::tick - role->getPvpBeginTime();
    
    //关闭定时
    role->setPvpBeginTime(0);
    
    int64_t playerId = MAKE_OBJ_ID(kObjectRole, roleid);
    
    create_global(pvpResult, result);
    result->roleid = playerId;
    result->bechallengeid = role->getBechallengeId();
    result->isWin = req.iswin;
    result->battleTime = battleTime;
    sendGlobalMsg(Global::MQ, result);
}}

hander_msg(req_getPvpAward, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_getPvpAward ack;
    if ( !role->getPvpAward()) {
        ack.errorcode = CE_PVP_YOU_HAD_GETAWARD_TODAY;
        sendNetPacket(role->getSessionId(), &ack);
        return;
    }
    
    int awardRank = 0;
    
    awardRank = SPvpMgr.safe_GetRoleAwardRank(roleid);
    
    if (awardRank) {
        //get award
        ack.errorcode = CE_OK;
        ack.awardrank = awardRank;
        PvpAward* award = PvpCfg::getRankAward(awardRank);
        
        if (award == NULL) {
            ack.errorcode = CE_READ_CFG_ERROR;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        char buf[32] = "";
        sprintf(buf, "pvp_rank:%d", awardRank);
        
        vector<string> awards = StrSpilt(award->itemAward, ";");
        ItemArray items;
        if (!role->addAwards(awards, items, buf))
        {
            ack.errorcode = CE_BAG_FULL;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        LogMod::addLogGetItem(role, role->getRolename().c_str(), sessionid, items, buf);
        
        ack.awardItem = award->itemAward;
        
        role->addConstellVal(award->protoss, buf);
        role->addGold(award->gold, buf);
        
        {
            role->setPvpAward(0);
            doRedisCmd("hset role:%d pvpaward 0", roleid);
        }
        sendNetPacket(role->getSessionId(), &ack);
    } 
}}

/**************************************** petpvp ***************************************/

hander_msg(req_get_petpvp_data, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    if (role->getLvl() < PetPvpCfg::mCfgData.mOpenLvl) {
        return;
    }
    SPetPvpMgr.safe_getPlayerData(role);
}}

hander_msg(req_edit_petpvp_formation, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SPetPvpMgr.safe_editFormation(role, req.newformation);
}}

hander_msg(req_search_petpvp_enemy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_search_petpvp_enemy ack;
    if (role->getPetPvpTimes() >= PetPvpCfg::getEnterTimes()) {
        ack.errorcode = CE_PETPVP_TIMES_UP;
        sendNetPacket(sessionid, &ack);
    }

    SPetPvpMgr.safe_matchEnemy(role, req.firstSearch);
}}

hander_msg(req_begin_petpvp, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (!checkCurrSceneType(req.sceneid, stPetArena)) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    if (0 < role->getPreEnterSceneid()) {
        return;
    }
    
    //判断可否进入，防止同时进入多个副本
    if (!checkCanEnterCopy(role, req.sceneid)) {
        return;
    }
    
    SPetPvpMgr.safe_beginBattle(role, req.enemyid, req.sceneid);
}}

hander_msg(req_commit_petpvp_battle_report, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (!checkCurrSceneType(role->getCurrSceneId(), stPetArena)) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    int battleTime = Game::tick - role->getBattleTime();
    
    //这么短时间就淫了？不可能！
    if (battleTime < 5) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    role->backToCity();
    
    role->setBattleTime(0);
    SPetPvpMgr.safe_UpdatePlayerData(role, req.iswin);
    
}}

hander_msg(req_end_search_petpvp_enemy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SPetPvpMgr.safe_endPlayerSearchEnemy(role);
}}

hander_msg(req_get_petpvp_formation, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SPetPvpMgr.safe_getPetPvpFormation(role);
}}

hander_msg(req_get_item_by_consum_petpvp_point, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ItemArray items;
    GridArray effgrids;
    items.push_back(ItemGroup(req.item , req.count));

    if(role->preAddItems(items, effgrids) != CE_OK){
        ack_get_item_by_consum_petpvp_point ack;
        ack.errorcode = CE_BAG_FULL;
        sendNetPacket(role->getSessionId(), &ack);
        return;
    }
    
    SPetPvpMgr.safe_buyItemsUsingPoints(role, req.item, req.count);
}}

/********************************* real pvp ***********************************/

hander_msg(req_get_realpvp_data, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    g_RealpvpMgr.safe_getMyRealpvpInfo(role);
    
}}

hander_msg(req_bereadyto_realpvp_matching, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    
    if ( !SActMgr.IsActive(ae_synpvp)) {
        
        ack_bereadyto_realpvp_matching ack;
        ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
        sendNetPacket(sessionid, &ack);
        return;
        
    }
    
    g_RealpvpMgr.safe_readyToRealpvpMatching(role);
    
}}

hander_msg(req_cancel_realpvp_matching, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    g_RealpvpMgr.safe_cancelMatching(roleid);
}}

hander_msg(req_realpvp_medal_exchange, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_realpvp_medal_exchange ack;
    
    SynPvpStoreGoodsDef* itemdef = SynPvpStoreCfg::getGoodsDefByIdx(req.itemIndex);
    
    if (itemdef == NULL) {
        ack.errorcode = CE_REALPVP_CAN_NOT_FIND_THIS_ITEM;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (req.itemCount <= 0) {
        return;
    }
    
    ItemGroup newitem;
    ItemArray items;
    GridArray effgrids;
    
    newitem.item = itemdef->goodsId;
    newitem.count = req.itemCount;
    
    items.push_back(newitem);
    
    if (role->preAddItems(items, effgrids) != CE_OK) {
        ack.errorcode = CE_BAG_FULL;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    g_RealpvpMgr.safe_medalExchange(roleid, req.itemIndex, req.itemCount);
}}

hander_msg(req_realpvp_exchange_itemlist, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    vector<SynPvpStoreGoodsDef*>& itemlist = SynPvpStoreCfg::getCfg();
    
    int listSize = itemlist.size();
    ack_realpvp_exchange_itemlist ack;
    
    for (int i = 0; i < listSize; i++) {
        
        SynPvpStoreGoodsDef* itemDef = itemlist[i];
        if (itemDef) {
            obj_realpvp_item item;
            
            item.index = itemDef->index;
            
            item.itemid = itemDef->goodsId;
            
            item.consum = itemDef->needMedal;
            
            item.needHonor = itemDef->needHonor;
            
            ack.items.push_back(item);
        }
    }
    
    sendNetPacket(sessionid, &ack);
    
}}

/**
 *  对决
 **/
hander_msg(req_duel_invite, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_duel_invite ack;
    ack.errorcode = CE_OK;
    
    int targetId = req.targetid;
    
    Role* target = SRoleMgr.GetRole(targetId);
    
    do
    {
        if (target == NULL) {
            ack.errorcode = CE_DUEL_TARGET_DISONLINE;
            break;
        }
        
        if (!SynPvpFuntionCfg::checkDuelLvl(role->getLvl())) {
            ack.errorcode = CE_YOUR_LVL_TOO_LOW;
            break;
        }
        
        if (!SynPvpFuntionCfg::checkDuelLvl(target->getLvl())) {
            ack.errorcode = CE_DUEL_TARGET_LVL_IS_TOO_LOW;
            break;
        }
        
        SceneCfgDef* targetCurrScene = SceneCfg::getCfg(target->getCurrSceneId());
        
        if ( targetCurrScene == NULL || targetCurrScene->sceneType != stTown) {
            ack.errorcode = CE_DUEL_TARGET_NOT_IN_TOWN;
            break;
        }
        
        if (target->getDuelState()) {
            ack.errorcode = CE_DUEL_TARGET_IS_IN_DUEL;
            break;
        }
        
        int now = Game::tick;
        
        notify_duel_invite_notice notice;
        notice.inviterid = roleid;
        notice.inviterName = role->getRolename();
        notice.inviterLvl = role->getLvl();
        notice.inviterRoletype = role->getRoleType();
        notice.inviteTime = now;
        
        sendNetPacket(target->getSessionId(), &notice);
        
        role->setDuelTarget(targetId);
        role->setDuelTime(now);

    }while(false);
    
    sendNetPacket(sessionid, &ack);
    
}}

hander_msg(req_duel_invite_respond, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_duel_invite_respond ack;
    ack.errorcode = CE_OK;
    ack.agree = req.agree;
    
    Role* inviter = SRoleMgr.GetRole(req.inviterid);
    do
    {
        if (inviter == NULL) {
            ack.errorcode = CE_DUEL_TARGET_DISONLINE;
            break;
        }
        
        if (inviter->getDuelState() == true) {
            ack.errorcode = CE_DUEL_TARGET_IS_IN_DUEL;
            break;
        }
        
        if (inviter->getDuelTarget() != roleid || inviter->getDuelTime() != req.inviteTime) {
            ack.errorcode = CE_DUEL_INVITER_HAD_CANCEL_DUEL;
            break;
        }
        
        //拒绝对方
        if (req.agree == 0) {
            notify_duel_invite_respond respond;
            respond.agree = req.agree;
            respond.targetid = roleid;
            sendNetPacket(inviter->getSessionId(), &respond);
            break;
        }
        
        role->setDuelState(true);
        role->setDuelTarget(inviter->getInstID());
        role->setDuelEnemyName(inviter->getRolename());
        
        inviter->setDuelState(true);
        inviter->setDuelEnemyName(role->getRolename());
        
        notify_duel_be_ready readynotice;
        readynotice.errorcode = CE_OK;
        
        sendNetPacket(sessionid, &readynotice);
        sendNetPacket(inviter->getSessionId(), &readynotice);
        
        create_cmd(CreateWorldScene, createWorldScene);
        createWorldScene->sceneMod = 501227;
        createWorldScene->extendStr = "duel";
        
        //第一个玩家
        inviter->addSyncPvpBuff();
        WorldPlayerInfo firstPlayerInfo = getWorldPlayerInfo(inviter);
        createWorldScene->playerinfos.push_back(firstPlayerInfo);
        
        //第2个玩家
        role->addSyncPvpBuff();
        WorldPlayerInfo secondPlayerInfo = getWorldPlayerInfo(role);
        createWorldScene->playerinfos.push_back(secondPlayerInfo);
        
        sendMessageToWorld(0, CMD_MSG, createWorldScene, 0);
        
    }while(false);
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_duel_invite_cancel, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_duel_invite_cancel ack;
    ack.errorcode = CE_OK;
    
    //已经进入决斗状态不管
    if (role->getDuelState()) {
        return;
    }
    
    Role* target = SRoleMgr.GetRole(role->getDuelTarget());
    
    if (target) {
        notify_duel_invite_cancel notify;
        notify.inviterId = roleid;
        notify.inviterName = role->getRolename();
        sendNetPacket(target->getSessionId(), &notify);
    }
    
    role->clearDuelInfo();
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_pvp_exchange_itemlist, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    vector<PvpStoreGoodsDef*>& itemlist = PvpStoreCfg::getCfg();
    
    int listSize = itemlist.size();
    ack_pvp_exchange_itemlist ack;
    ack.myPvpPoint = role->getPvpPoints();
    
    for (int i = 0; i < listSize; i++) {
        
        PvpStoreGoodsDef* itemDef = itemlist[i];
        if (itemDef) {
            obj_pvp_exchange_item item;
            
            item.index = itemDef->index;
            
            item.itemid = itemDef->goodsId;
            
            item.consum = itemDef->needPoints;
            
            ack.items.push_back(item);
        }
    }
    
    sendNetPacket(sessionid, &ack);
    
}}

hander_msg(req_pvp_point, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_pvp_point ack;
    ack.pvpPoint = role->getPvpPoints();

    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_pvp_point_exchange, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_pvp_point_exchange ack;
    
    PvpStoreGoodsDef* itemdef = PvpStoreCfg::getGoodsDefByIdx(req.itemIndex);
    
    if (itemdef == NULL) {
        ack.errorcode = CE_CAN_NOT_FIND_CFG;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (req.itemCount <= 0) {
        return;
    }
    
    int myPoints = role->getPvpPoints();
    int needPoints = itemdef->needPoints * req.itemCount;
    
    if(myPoints < needPoints)
    {
        ack.errorcode = CE_PVP_EXCHANGE_POINTS_NOT_ENOUGH;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    ItemGroup newitem;
    ItemArray items;
    GridArray effgrids;
    
    newitem.item = itemdef->goodsId;
    newitem.count = req.itemCount;
    
    items.push_back(newitem);
    
    if (role->preAddItems(items, effgrids) != CE_OK) {
        ack.errorcode = CE_BAG_FULL;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    int beforePoints = role->getPvpPoints();
    
    role->addPvpPoints(-needPoints, "pvp_exchange");
    role->playerAddItemsAndStore(effgrids, items, "pvp_exchange", true);
    
    ack.errorcode = CE_OK;
    ack.itemCount = req.itemCount;
    ack.myPvpPoint = role->getPvpPoints();
    sendNetPacket(sessionid, &ack);
    
    if (ack.errorcode == CE_OK)
    {
        string getItems = "";
        for (int i = 0; i < items.size(); i++) {
            getItems.append(strFormat("item %d*%d;", items[i].item, items[i].count));
        }
        
        LogMod::addLogPvpPointExchange(roleid, needPoints, beforePoints, role->getPvpPoints(), getItems.c_str());
    }
}}