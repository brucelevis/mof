//
//  cs_handler.cpp
//  GameSrv
//
//  Created by 丁志坚 on 15/6/23.
//
//

#include <stdio.h>
#include "hander_include.h"
#include "centerClient.h"
#include "cs_pvp/cs_pvp.h"
#include "cs_pvp/cs_pvp_battle.h"
#include "worship/worship_system.h"

extern float calcWorshipPlus(int commonTimes, int specialTimes);

handler_msg(req_worship_info, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    if (CsPvpSystem::isClose(Game::tick)) {
        return;
    }
    
    int beCommonWorship = g_WorshipSystem.getCommonBeWorshipTimes(roleid);
    int beSpecialWorship = g_WorshipSystem.getSpecialBeWorshipTimes(roleid);
    float addProperty = calcWorshipPlus(beCommonWorship, beSpecialWorship);
    
    ack_worship_info ack;
    ack.addproperty = addProperty;
    ack.battlepoint = role->getBattleForce();
    ack.beworshiptimes = g_WorshipSystem.getBeWorshipTimes(roleid);
    ack.commonworshiptimes = role->getWorshipData().getUsedCommonTimes();
    ack.specialworshiptimes = role->getWorshipData().getUsedSpecialTimes();
    sendNetPacket(sessionid, &ack);
}}


handler_msg(req_worship, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (CsPvpSystem::isClose(Game::tick)) {
        ack_worship ack;
        ack.errorcode = CE_PVP_ACTIVITY_TIMEOUT;
        sendNetPacket(sessionid, &ack);
        return;
    }

    ack_worship ack;
    ack.errorcode = -1;
    ack.targetId = ack.targetId;
    ack.type = req.type;
    
    int needRmb = 0;
    do {
        if (!CsPvpSystem::canWorship()) {
            ack.errorcode = CE_PVP_ACTIVITY_TIMEOUT;
            break;
        }
        
        int targetId = req.targetId;
        bool isSpecial = false;
        if (targetId != roleid || req.type == "special") {
            isSpecial = true;
        }
        
        int maxBeWorship = RoleCfg::getCrossServiceWarIsWorShipTimes();
        if (g_WorshipSystem.getBeWorshipTimes(targetId) >= maxBeWorship) {
            ack.errorcode = 2;
            break;
        }
        
        RoleCfgDef& roleCfgDef = RoleCfg::getCfg(role->getJob(), role->getLvl());
        if (isSpecial) {
            int specialTimes = role->getWorshipData().getUsedSpecialTimes();
            int costType = 0;
            needRmb = BuyCfg::getCost(eCrossServiceWarWorShipBuy, specialTimes + 1, costType);
            ack.errorcode = role->CheckMoneyEnough(needRmb, costType, "worship");
            if (ack.errorcode != CE_OK) {
                break;
            }
            role->getWorshipData().addUsedSpecialTimes();
            
            int specialBeWorship = g_WorshipSystem.getSpecialBeWorshipTimes(targetId);
            g_WorshipSystem.setSpecialBeWorshipTimes(targetId, specialBeWorship + 1);
        } else {
            int commonWorship = role->getWorshipData().getUsedCommonTimes();
            int worshipTimes = roleCfgDef.mCrossServiceWarWorShipFreeTimes;
            if (commonWorship >= worshipTimes) {
                ack.errorcode = 3;
                break;
            }
            role->getWorshipData().addUsedCommonTimes();
            
            int commonBeWorship = g_WorshipSystem.getCommonBeWorshipTimes(targetId);
            g_WorshipSystem.setCommonBeWorshipTimes(targetId, commonBeWorship + 1);
        }

        vector<string> awards = CrossServiceWarWorshipCfg::sAwards;
        if (awards.size() > 0) {
            ItemArray items;
            role->addAwards(awards, items, "膜拜奖励");
        }
        
        role->save();
        g_WorshipSystem.addWorshipMember(roleid);
        g_WorshipSystem.save();
        ack.errorcode = 0;
        
        LogMod::addWorshipLog(roleid, targetId, req.type.c_str(), needRmb);
        
    } while (0);

    sendNetPacket(sessionid, &ack);
}}


handler_msg(req_cspvp_challengedata, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (!CsPvpSystem::canBattle()) {
        ack_cspvp_challengedata ack;
        ack.errorcode = CE_PVP_ACTIVITY_TIMEOUT;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    req_cs_get_cspvp_challenge_data routerReq;
    routerReq.role_id = roleid;
    routerReq.server_id = Process::env.getInt("server_id");
    routerReq.setSession(sessionid);
    sendNetPacket(CenterClient::instance()->getSession(), &routerReq);
}}

handler_msg(req_cspvp_roleinfo, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (!CsPvpSystem::canBattle()) {
        ack_cspvp_roleinfo ack;
        ack.errorcode = CE_PVP_ACTIVITY_TIMEOUT;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    //更新角色信息
    int minLvl = 50;
    if (role->getLvl() >= minLvl) {
        req_cs_cspvp_add_role req;
        req.role_id = role->getInstID();
        req.server_id = Process::env.getInt("server_id");
        req.role_name = role->getRolename();
        req.role_type = role->getRoleType();
        req.lvl = role->getLvl();
        req.bat = role->getBattleForce();
        CenterClient::instance()->sendNetPacket(0, &req);
    }
    
    //请求角色信息
    int colddown = role->getCsPvpStateData().getBattleColddown() - Game::tick;
    if (colddown < 0) {
        colddown = 0;
    }
    int costPerMin = PvpCfg::getFreshCost(0);

    req_cs_get_cspvp_roleinfo routerReq;
    routerReq.role_id = roleid;
    routerReq.server_id = Process::env.getInt("server_id");
    routerReq.colddown = colddown;
    routerReq.costperminute = costPerMin;
    routerReq.setSession(sessionid);
    sendNetPacket(CenterClient::instance()->getSession(), &routerReq);
}}

handler_msg(req_cspvp_log, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (!CsPvpSystem::canBattle()) {
        ack_cspvp_log ack;
        ack.errorcode = CE_PVP_ACTIVITY_TIMEOUT;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    req_cs_get_cspvp_log routerReq;
    routerReq.role_id = roleid;
    routerReq.server_id = Process::env.getInt("server_id");
    routerReq.num = req.num;
    routerReq.setSession(sessionid);
    sendNetPacket(CenterClient::instance()->getSession(), &routerReq);
}}

handler_msg(req_begin_cspvp, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (!CsPvpSystem::canBattle()) {
        ack_begin_cspvp ack;
        ack.errorcode = CE_PVP_ACTIVITY_TIMEOUT;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (role->getCsPvpState()->getChallengeTime() <= 0) {
        return;
    }

    if (role->getCsPvpStateData().getBattleColddown() > Game::tick) {
        return;
    }
    
    if (role->getPreEnterSceneid() > 0) {
        return;
    }
    
    if (CenterClient::instance()->isRegistered()) {
        ack_begin_cspvp ack;
        ack.errorcode = CE_SYSTEM_ERROR;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    role->setPreEnterSceneid(CSPVP_SCENEID);
    role->getCsPvpState()->pendSelectTarget(req.serverid, req.bechallengeId);
    
    req_cs_begin_cspvp routerReq;
    routerReq.setSession(req.getSession());
    routerReq.src_server_id = Process::env.getInt("server_id");
    routerReq.src_role_id = roleid;
    routerReq.target_role_id = req.bechallengeId;
    routerReq.target_server_id = req.serverid;
    routerReq.target_rank = req.bechallengeRank;
    sendNetPacket(CenterClient::instance()->getSession(), &routerReq);
        
}}

handler_msg(req_cspvp_result, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (!CsPvpSystem::canBattle()) {
        ack_cspvp_result ack;
        ack.errorcode = CE_PVP_ACTIVITY_TIMEOUT;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    role->getCsPvpState()->submitBattleResult();
    
    req_cs_cspvp_result routerReq;
    routerReq.src_role_id = roleid;
    routerReq.src_server_id = Process::env.getInt("server_id");
    routerReq.target_role_id = req.bechallengeId;
    routerReq.target_server_id = Process::env.getInt("server_id");
    routerReq.iswin = req.iswin;
    CenterClient::instance()->sendNetPacket(sessionid, &routerReq);
}}

handler_msg(req_fresh_cspvp_immediately, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    int colddown = role->getCsPvpStateData().getBattleColddown();
    ack_fresh_cspvp_immediately ack;
    ack.errorcode = 1;
    do {
        int costPerMin = PvpCfg::getFreshCost(0);
        int payTime = colddown - Game::tick;
        if (payTime < 0) {
            ack.errorcode = 0;
            break;
        }

        int needRmb = ((payTime + 59) / 60) * costPerMin;
        ack.errorcode = role->CheckMoneyEnough(needRmb, eRmbCost, "立即刷新跨服竞技场");
        if (ack.errorcode != 0) {
            break;
        }

        role->getCsPvpStateData().setBattleColddown(0);
        role->save();
        ack.errorcode = 0;
    } while (0);

    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_get_cspvp_rolerank, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (CsPvpSystem::isClose(Game::tick)) {
        return;
    }
    
    req_cs_cspvp_get_rolerank routerReq;
    routerReq.setSession(req.getSession());
    routerReq.start_rank = req.start_rank;
    routerReq.end_rank = req.end_rank;
    CenterClient::instance()->sendNetPacket(sessionid, &routerReq);
}}

handler_msg(req_get_cspvp_serverrank, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (CsPvpSystem::isClose(Game::tick)) {
        return;
    }

    req_cs_cspvp_get_serverrank routerReq;
    routerReq.setSession(req.getSession());
    routerReq.start_rank = req.start_rank;
    routerReq.end_rank = req.end_rank;
    CenterClient::instance()->sendNetPacket(sessionid, &routerReq);
}}


