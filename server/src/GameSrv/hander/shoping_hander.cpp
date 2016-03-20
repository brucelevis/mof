//
//  shoping_hander.cpp
//  GameSrv
//
//  Created by pireszhi on 13-4-15.
//
//

#include "hander_include.h"
#include "Skill.h"
#include "DefConf.h"
#include "DBRolePropName.h"

hander_msg(req_buy_fat, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_buy_fat ack;
    int costType = 0;
    int cost = 0;
    int addPoint = 0;
    do
    {
        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(role->getVipLvl());
        if (vipcfg == NULL) {
            ack.errorCode = CE_READ_VIPCFG_ERROR;
            break;
        }

        int buyTimes = role->getFatBuyTimes();
        if (buyTimes >= vipcfg->fatBuyTimes) {
            ack.errorCode = CE_BUY_TIMES_USEUP;
            sendNetPacket(sessionid, &ack);
            return;
        }

        cost = BuyCfg::getCost(eFatBuy,buyTimes, costType);
        addPoint = BuyCfg::getAddpoint(eFatBuy);
    }while(false);

    ack.errorCode = role->CheckMoneyEnough(cost, costType, "购买精力");
    if (ack.errorCode == CE_OK) {
        role->addFat(addPoint);
        role->addFatBuyTimes(1);

        ack.buyTime = role->getFatBuyTimes();
        ack.costrmb = cost;
        ack.costType = costType;
		
		Xylog log(eLogName_BuyFat, role->getInstID());
		log << role->getFatBuyTimes();
    }

    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_buy_elitetimes, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    int addtimes = req.buytime;

    ack_buy_elitetimes ack;

    int buyTimes = role->getEliteBuyTimes();

    //from config
    int cost = 0;
    int costType = 0;

    do
    {
        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(role->getVipLvl());
        if (vipcfg == NULL) {
            ack.errorCode = CE_READ_VIPCFG_ERROR;
            break;
        }

        if ( buyTimes >= vipcfg->eliteBuyTimes) {
            ack.errorCode = CE_BUY_TIMES_USEUP;
            break;
        }

        if ( ( buyTimes + addtimes ) > vipcfg->eliteBuyTimes) {
            SRoleMgr.disconnect(role);
            return;
        }

        for (int i = 0; i < addtimes; i++) {
            cost += BuyCfg::getCost(eEliteBuy,buyTimes + i, costType);
        }
    }while(false);

    ack.errorCode = role->CheckMoneyEnough(cost, costType, "购买精英本次数");

    if (ack.errorCode == CE_OK) {
        role->addEliteTimes(addtimes);
        role->addEliteBuyTimes(addtimes);

        ack.addtime = addtimes;
        ack.costrmb = cost;
        ack.buyTime = role->getEliteBuyTimes();
        ack.costType = costType;
    }
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_buy_petelitetimes, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    xycheck( req.buytime > 0 );
    ack_buy_petelitetimes ack;

    VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(role->getVipLvl());
    send_if(NULL == vipcfg, CE_READ_VIPCFG_ERROR);
    send_if(role->getPetEliteBuyTimes() >= vipcfg->petEliteBuytimes , CE_BUY_TIMES_USEUP );
    send_if(role->getPetEliteBuyTimes() + req.buytime > vipcfg->petEliteBuytimes , CE_BUY_TIMES_USEUP );
    
    int cost = 0, costType = 0;
    for (int i = 0; i < req.buytime; i++) {
        cost += BuyCfg::getCost(eOperateBuy,role->getPetEliteBuyTimes() + i, costType);
    }
    ack.errorcode = role->CheckMoneyEnough(cost, costType, "购买幻兽试炼场次数");
    
    if (ack.errorcode == CE_OK) {
        role->setPetEliteBuyTimes( role->getPetEliteBuyTimes() +  req.buytime );
		role->saveNewProperty();
        ack.addtime = req.buytime;
        ack.costrmb = cost;
        ack.buyTime = role->getPetEliteBuyTimes();
        ack.costType = costType;
        
        notify_syn_petelitetimes nty;
        nty.times = role->getPetEliteCopyLeftTime();
        role->send(&nty);
    }
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_buy_pvptimes, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    ack_buy_pvptimes ack;
    do
    {
        ack.errorcode = CE_OK;

        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(self->getVipLvl());
        if (vipcfg == NULL) {
            ack.errorcode = CE_READ_VIPCFG_ERROR;
            break;
        }

        if (self->getPvpBuyTimes() >= vipcfg->pvpbuytimes ) {
            ack.errorcode = CE_BUY_TIMES_USEUP;
            break;
        }

        int costType = 0;
        int cost = BuyCfg::getCost(ePvpTimesBuy,self->getPvpBuyTimes(), costType);

        ack.errorcode = self->CheckMoneyEnough(cost, costType, "购买竞技场次数");

        if (ack.errorcode == CE_OK) {
            self->addPvpTimes(1);
            
            string describe = Utils::makeStr("usermb:%d", cost);
            self->addLogActivityCopyEnterTimesChange(eActivityEnterTimesPvp, describe.c_str());
            
            self->addPvpBuyTimes(1);
            ack.costrmb = cost;
            ack.buytimes = self->getPvpBuyTimes();
            ack.costType = costType;
        }

    }while(false);

    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_teamcopy_reset, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    ack_teamcopy_reset ack;

    do
    {
        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(self->getVipLvl());
        if (vipcfg == NULL) {
            ack.errorcode = CE_READ_VIPCFG_ERROR;
            break;
        }
        int maxTimes = vipcfg->teamCopyBuyTimes;
        if (self->getTeamCopyResetTimes() >= maxTimes) {
            ack.errorcode = CE_BUY_TIMES_USEUP;
            break;
        }

        if (!self->getTeamCopyMgr()->IsCopyFinish(req.copyid)) {
            return;
        }

        int costType = 0;
        int cost = BuyCfg::getCost(eTeamCopyReset, self->getTeamCopyResetTimes(), costType);

        ack.errorcode = self->CheckMoneyEnough(cost, costType, "好友副本重设");
        if (ack.errorcode == CE_OK) {
            self->addTeamCopyResetTimes(1);
            self->mTeamCopyMgr->DelCopyFromFinish(roleid, req.copyid);
            ack.copyid = req.copyid;
        }
    }while(false);

    sendNetPacket(sessionid, &ack);

}}

hander_msg(req_dungcopy_reset, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    int currentTimes = self->getDungResetTimes(req.dungcopytype);
    ack_dungcopy_reset ack;
    do
    {
        ack.errorcode = CE_OK;

        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(self->getVipLvl());
        if (vipcfg == NULL) {
            ack.errorcode = CE_READ_VIPCFG_ERROR;
            break;
        }

        if (currentTimes >= vipcfg->dungeonsTimes ) {
            ack.errorcode = CE_BUY_TIMES_USEUP;
            break;
        }

        int costType = eRmbCost;
//        int cost = BuyCfg::getCost(eDungBuy,currentTimes, costType);
        int cost = DungeonCfg::getResetCost(req.dungcopytype, currentTimes);

        if (cost < 0) {
            ack.errorcode = CE_UNKNOWN;
            break;
        }

        if (self->getDungeLevelRecord(req.dungcopytype) == 0) {
            return;
        }

        ack.errorcode = self->CheckMoneyEnough(cost, costType, "地下城扫荡");

        if (ack.errorcode == CE_OK) {
            
            DungeonCfgDef* dungeCfg =  DungeonCfg::getDungeonCfg(stDungeon, req.dungcopytype);
            
            if (dungeCfg == NULL) {
                ack.errorcode = CE_CAN_NOT_FIND_CFG;
                break;
            }
            
            int beginsceneid = dungeCfg->firstmap;
            
            int endsceneId = self->getDungeLevelRecord(req.dungcopytype);
            
            if (endsceneId < 0) {
                endsceneId = dungeCfg->firstmap + dungeCfg->floor - 1;
            }
            else if (endsceneId > 0){
                endsceneId = endsceneId - 1;
            }
            
            ack.errorcode = dungeWipeOut(req.dungcopytype, beginsceneid, endsceneId, self, ack.awards);

            if (ack.errorcode != CE_OK) {
                switch (costType) {
                    case eGoldCost:
                        self->addGold(cost, "地下城扫荡失败返回");
                        break;
                    case eRmbCost:
                        self->addRmb(cost, "地下城扫荡失败返回");
                        break;
                    default:
                        break;
                }

                break;
            }

            self->addDungResetTimes(1, req.dungcopytype);
            ack.cost = cost;
            ack.costType = costType;
        }

    }while(false);

    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_printcopy_reset, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    int currentTimes = self->getPrintCopyAct()->getCopyResetTimes(req.copyid);

    ack_printcopy_reset ack;
    do
    {
        ack.errorcode = CE_OK;

        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(self->getVipLvl());
        if (vipcfg == NULL) {
            ack.errorcode = CE_READ_VIPCFG_ERROR;
            break;
        }

//        if (currentTimes >= 10) {
//            ack.errorcode = CE_BUY_TIMES_USEUP;
//            break;
//        }

        int costType = eRmbCost;

        SceneCfgDef* scenecfg = SceneCfg::getCfg(req.copyid);
        if (scenecfg == NULL) {
            ack.errorcode = CE_READ_SCENE_CFG_ERROR;
            break;
        }

        int cost = scenecfg->getResetCost(currentTimes);

        if (cost < 0) {
            ack.errorcode = CE_UNKNOWN;
            break;
        }

        if (!self->getPrintCopyAct()->IsCopyFinish(req.copyid)) {
            return;
        }

        ack.errorcode = self->CheckMoneyEnough(cost, costType, "图纸副本重设");

        if (ack.errorcode == CE_OK) {
            self->getPrintCopyAct()->addCopyResetTimes(self->getInstID(), req.copyid, 1);
            self->getPrintCopyAct()->DelCopyFromFinish(self->getInstID(), req.copyid);
            ack.cost = cost;
            ack.costType = costType;
            ack.copyid = req.copyid;
        }

    }while(false);

    sendNetPacket(sessionid, &ack);

}}

handler_msg(req_fresh_pvp_immediately, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    ack_fresh_pvp_immediately ack;
    ack.errorcode = CE_OK;
    do
    {
        int pvptimes = self->getPvpTimes();
        if (pvptimes <= 0) {
            ack.errorcode = CE_PVP_YOU_HAVE_NO_TIMES_TO_PVP;
            break;
        }
        int freetimes = PvpCfg::getfreeTimes();
        int freshcost = PvpCfg::getFreshCost(freetimes - pvptimes - 1);

        RedisResult result(redisCmd("hget role:%d %s", roleid, GetRolePropName(eRolePvpCoolDown)));
        int cooldowntime = result.readInt();
        check_min(cooldowntime, 0);

        int restMinute = ( cooldowntime - time(NULL) + 59 ) / 60;
        check_min(restMinute, 0);

        int cost = restMinute * freshcost;
        ack.errorcode = self->CheckMoneyEnough(cost, eRmbCost, "pvp_cooldown");
        if (ack.errorcode == CE_OK) {
            doRedisCmd("hset role:%d %s %d", roleid, GetRolePropName(eRolePvpCoolDown), time(NULL));
        }
    }while(false);

    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_petcamp_reset, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    int currentTimes = self->getPetCampAct()->getCampResetTimes(req.copyid);

    ack_petcamp_reset ack;
    do
    {
        ack.errorcode = CE_OK;

        SceneCfgDef* scenecfg = SceneCfg::getCfg(req.copyid);
        if (scenecfg == NULL) {
            ack.errorcode = CE_READ_SCENE_CFG_ERROR;
            break;
        }

        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(self->getVipLvl());
        if (vipcfg == NULL) {
            ack.errorcode = CE_READ_VIPCFG_ERROR;
            break;
        }

        int costType = eRmbCost;

        int cost = scenecfg->getResetCost(currentTimes);

        if (cost < 0) {
            ack.errorcode = CE_UNKNOWN;
            break;
        }
        if (!self->getPetCampAct()->IsCampFinish(req.copyid)) {
            return;
        }

        ack.errorcode = self->CheckMoneyEnough(cost, costType, "幻兽大本营重设");

        if (ack.errorcode == CE_OK) {
            self->getPetCampAct()->addCampResetTimes(self->getInstID(), req.copyid, 1);
            self->getPetCampAct()->DelCampFromFinish(self->getInstID(), req.copyid);
            ack.cost = cost;
            ack.costType = costType;
            ack.copyid = req.copyid;
        }

    }while(false);

    sendNetPacket(sessionid, &ack);

}}

//幻兽大冒险副本次数购买次数
hander_msg(req_buy_petadventuretimes, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    int addtimes = req.buytime;
    
    ack_buy_petadventuretimes ack;
    stringstream szLogBuyTimesChange;
    stringstream szLogBuyTimesRecordPerDay;
    stringstream szLogPetAdventureTimes;

    int buyTimes = role->getPetAdventureBuyTimesRecordPerDay();
    szLogBuyTimesChange << role->getPetAdventureBuyTimes() << ";";
    szLogBuyTimesRecordPerDay << buyTimes << ";";
    szLogPetAdventureTimes << role->calPetAdventureTimes() << ";";
    
    //from config
    int cost = 0;
    int costType = 0;
    bool bLogBuyIsSucceed = false;
    
    do
    {
        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(role->getVipLvl());
        if (vipcfg == NULL) {
            ack.errorCode = CE_READ_VIPCFG_ERROR;
            break;
        }
        
        if ( buyTimes >= vipcfg->petAdventureBuyTimes) {
            ack.errorCode = CE_BUY_TIMES_USEUP;
            break;
        }
        
        if ( ( buyTimes + addtimes ) > vipcfg->petAdventureBuyTimes) {
            SRoleMgr.disconnect(role);
            return;
        }
        
        for (int i = 0; i < addtimes; i++) {
            cost += BuyCfg::getCost(ePetAdventureBuy,buyTimes + i, costType);
        }
    }while(false);
    
    ack.errorCode = role->CheckMoneyEnough(cost, costType, "购买幻兽大冒险副本次数");
    
    if (ack.errorCode == CE_OK) {

        role->addPetAdventureBuyTimes(addtimes);
        
        string action = Utils::makeStr("usermb_%d", cost);
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesPetAdventure, action.c_str());
        
        //购买后
        szLogBuyTimesChange << role->getPetAdventureBuyTimes();
        szLogBuyTimesRecordPerDay << role->getPetAdventureBuyTimesRecordPerDay();
        szLogPetAdventureTimes << role->calPetAdventureTimes();
        
        ack.addtime = addtimes;
        ack.costrmb = cost;
        ack.buyTime = role->getPetAdventureBuyTimesRecordPerDay();
        ack.costType = costType;
        
        bLogBuyIsSucceed = true;
        
    }
    
    LogMod::addLogPetAdventureCopyBuyTimes(role, addtimes, cost, costType, szLogBuyTimesChange.str(), szLogBuyTimesRecordPerDay.str(), szLogPetAdventureTimes.str(), bLogBuyIsSucceed);
    
    sendNetPacket(sessionid, &ack);
}}


handler_msg(req_buy_crossservicewartimes, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_buy_crossservicewartimes ack;
    ack.errorCode = 1;
    int addtimes = req.buytime;
    int buyTimes = role->getCsPvpStateData().getRmbBuyTimes();
    int cost = 0;
    int costType = 0;
    do
    {
        if (addtimes <= 0) {
            break;
        }
        
        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(role->getVipLvl());
        if (vipcfg == NULL) {
            ack.errorCode = CE_READ_VIPCFG_ERROR;
            break;
        }
        
        if ( buyTimes >= vipcfg->crossServiceWarBuyTimes) {
            ack.errorCode = CE_BUY_TIMES_USEUP;
            break;
        }
        
        if ( ( buyTimes + addtimes ) > vipcfg->crossServiceWarBuyTimes) {
            SRoleMgr.disconnect(role);
            return;
        }
        
        for (int i = 0; i < addtimes; i++) {
            cost += BuyCfg::getCost(eCrossServiceWarBuy, buyTimes + i, costType);
        }
    }while(false);
    
    ack.errorCode = role->CheckMoneyEnough(cost, costType, "购买跨服战战斗次数");
    if (ack.errorCode == 0) {
        role->getCsPvpStateData().buyTime(addtimes);
        
        ack.buyTime = role->getCsPvpStateData().getRmbBuyTimes();
        RoleCfgDef& cfgdef = RoleCfg::getCfg(role->getJob(), role->getLvl());
        int remainFreeTimes = cfgdef.mCrossServiceWarFreeTimes - role->getCsPvpStateData().getUsedFreeTimes();
        ack.addtime = role->getCsPvpStateData().getRmbTimes() + remainFreeTimes;
        ack.costrmb = cost;
        ack.costType = costType;
    }
    
    sendNetPacket(sessionid, &ack);
}}






