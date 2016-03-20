  //
//  GlobalMsg_handler.cpp
//  GameSrv
//
//  Created by pireszhi on 13-7-1.
//
//
#include "Global.h"
#include "GlobalMsg_handler.h"
#include "GlobalMsg_def.h"
#include "Utils.h"
#include "Paihang.h"
#include "msg.h"
#include "main.h"
#include "EnumDef.h"
#include "Pvp.h"
#include "Game.h"
#include "Skill.h"
#include "BroadcastNotify.h"
#include "Pvp.h"
#include "Activity.h"
#include "DBRolePropName.h"
#include "PetPvp.h"
#include "FamesHall.h"
#include "GameLog.h"
#include "RealPvpMgr.h"
#include "world_cmdmsg.h"

#include "GuildRole.h"
#include "Guild.h"
#include "GuildMgr.h"

#include "Robot.h"
#include "daily_schedule_system.h"
#include "Honor.h"
#include "flopcard.h"
#include "Retinue.h"


handler_global(PaihangTest)
{
//    SSortMgr.loadSortData();

//    for (int i = 0; i < this->test.size(); i++) {
//        printf("test Global Arr %s\n", this->test[i].c_str());
//    }
//    printf("test GlobalMsg%d\n",num);
}

handler_global(updatePaihangDB)
{
    SSortMgr.updateDB(this->roleid, this->lvl, this->battle, this->consume, this->recharge, this->optionType);
}

handler_global(updatePetPaihangDB)
{
    SSortMgr.updatePetDB(this->petid, this->petBattle, this->optionType);
}

handler_global(ClientGetPaihangData)
{
    struct PaihangMsgData *msg = (struct PaihangMsgData*)this->data;

    if (msg->paihangType == eSortPet) {
        vector<SortPetInfo*> petData;

        SSortMgr.getPetSortData(msg->beginRank, msg->endRank, petData);

        ack_getPetPaihangData ack;
        ack.beginRank = msg->beginRank;
        ack.endRank = msg->endRank;
        ack.paihangType = msg->paihangType;
        for (int i = 0; i < petData.size(); i++) {
            obj_Petpaihangdata obj;
            obj.battle = petData[i]->battle;
            obj.growth = petData[i]->growth;
            obj.modid = petData[i]->petmod;
            obj.rolename = petData[i]->mastername;
            obj.petid = petData[i]->petid;
            ack.data.push_back(obj);
        }
        SSortMgr.getMyPetRank(msg->roleid, ack.myPetRank);
        sendNetPacket(msg->sessionid, &ack);
    }
    else
    {
        vector<SortInfo*> sortdata;
        SSortMgr.getSortData(msg->paihangType, msg->job, msg->beginRank, msg->endRank, sortdata);
        ack_getPaihangData ack;
        ack.paihangType = msg->paihangType;
        ack.job = msg->job;
        ack.beginRank = msg->beginRank;
        ack.endRank = msg->beginRank + sortdata.size() - 1;

        for (int i = 0; i < sortdata.size(); i++) {
            obj_paihangdata obj;
            obj.job = sortdata[i]->job;
            obj.rolename = sortdata[i]->rolename;
            obj.roleid = sortdata[i]->roleid;
            obj.battle = sortdata[i]->battle;
            obj.lvl =sortdata[i]->lvl;
            obj.objtype = sortdata[i]->objType;
            obj.rank = i+1;
            obj.objtype = sortdata[i]->objType;
            ack.data.push_back(obj);
        }

        SSortMgr.getMyRank(msg->roleid, msg->paihangType, ack.mypvp, ack.myRank);
        sendNetPacket(msg->sessionid, &ack);
    }
    delete msg;
}

handler_global(getPaihangData)
{

    create_global(getPaihangDataRet, ret);
    ret->retCallBackFunc = this->retCallBackFunc;
    ret->type = info.paihangType;
    ret->roleid = info.roleid;
    if (info.paihangType == eSortPet) {
        vector<SortPetInfo*> petData;

        SSortMgr.getPetSortData(info.beginRank, info.endRank, petData);

        for (int i = 0; i < petData.size(); i++) {
            ret->petData.push_back(*petData[i]);
        }
        sendGlobalMsg(Game::MQ, ret);
    }
    else
    {
        vector<SortInfo*> sortdata;
        SSortMgr.getSortData(info.paihangType, info.job, info.beginRank, info.endRank, sortdata);

        for (int i = 0; i < sortdata.size(); i++) {
            ret->sortData.push_back(*sortdata[i]);
        }

        sendGlobalMsg(Game::MQ, ret);
    }
}

handler_global(CrossServiceGetPaihangDbData)
{
    int type = this->type;
    if (eSortPet != type) {
        vector<SortInfo*> roledata;
        SSortMgr.getPaihangRoleDbDataByType(type, roledata);
        
        sendPaihangRoleDbData2CenterServer(this->sessionid, type, roledata);
    }
    else {
        vector<SortPetInfo*> petdata;
        SSortMgr.getPaihangPetDbData(petdata);
        
        sendPaihangPetDbData2CenterServer(this->sessionid, type, petdata);
    }
}

int getPvpRoleInfo(int64_t roleid, obj_pvp_role& info)
{
    int targetId = 0;
    int targetType = 0;
    
    getPvpObjTypeAndObjId(roleid, targetId, targetType);
    
    obj_pvp_role roles;
    
    //如果是机器人
    if (targetType == kObjectRobot) {
        
        Robot* robot = g_RobotMgr.getData(targetId);
        if (robot == NULL) {
            return CE_PVP_GET_RANK_ROLES_ERROR;
        }
        
        info.rolename = robot->mRoleName;
        info.roletype = robot->mRoleType;
        info.lvl = robot->mBattleProp.mLvl;
        info.roleid = targetId;
        info.objType = kObjectRobot;
        return CE_OK;
    }
    
    //如果是真实玩家
    RedisResult result(redisCmd("hmget role:%d  rolename lvl roletype", targetId));

    string rolename = result.readHash(0, "");
    int rolelvl = Utils::safe_atoi( result.readHash(1, "").c_str() );
    int roleType = Utils::safe_atoi( result.readHash(2, "").c_str() );

    if(rolename.empty() || rolelvl <= 0)
    {      
        SPvpMgr.DeleteRoleInRank(roleid);
        return CE_PVP_GET_RANK_ROLES_ERROR;
    }
    info.roleid = roleid;
    info.rolename = rolename;
    info.lvl = rolelvl;
    info.roletype = roleType;
    info.objType = kObjectRole;
    return CE_OK;
}

handler_global(getPvpData)
{
    ack_getPvpRank ack;

    int myRank = SPvpMgr.getRankingByRoleId(roleid);
    ack.errorcode = CE_OK;
    
    //超过5000名的按5001名，version2.2， 2014.7.5，by wzg
    check_max(myRank, PvpCfg::getRobotCount()+1);
    
    ack.myrank = myRank;
    ack.getalldata = getalldata;
    do
    {
        if (myRank <= PVP_RANK_LIST_NUM) {
            for (int i = 1; i <= PVP_RANK_LIST_NUM + 1 && i <= SPvpMgr.getRankingSize(); i++) {
                obj_pvp_role roles;
                roles.rank = i;
                int64_t roleid = SPvpMgr.getRoleIdByRank(i);

                if (i == myRank) {
                    continue;
                }

                if (roleid == -1) {
                    ack.errorcode = CE_PVP_GET_RANK_ROLES_ERROR;
                    break;
                }

                ack.errorcode = getPvpRoleInfo(roleid, roles);

                if (ack.errorcode) {
                    break;
                }

                ack.roles.push_back(roles);
            }
        }
        else
        {
            PvpRankDataDef rule;
            PvpCfg::getRankRule(myRank, rule);

            int period = rule.space / PVP_RANK_LIST_NUM;
            if (!period) {
                log_error("pvp.ini rankrule error\n");
                period = 1;
            }

            for (int i = PVP_RANK_LIST_NUM; i > 0; i--) {

                int rol = rand()%period;
                int rank = myRank - period*i + rol;

                int64_t roleid = SPvpMgr.getRoleIdByRank(rank);
                
                if (roleid == -1) {
                    ack.errorcode = CE_PVP_GET_RANK_ROLES_ERROR;
                    break;
                }
                
                obj_pvp_role roles;
                
                ack.errorcode = getPvpRoleInfo(roleid, roles);
    
                if (ack.errorcode) {
                    break;
                }
                roles.rank = rank;
                ack.roles.push_back(roles);
            }
        }

    }while (false);

    if (ack.errorcode == CE_OK && getalldata) {

        vector<struct PvpLogData *> outputData;
        SPvpMgr.getPvpLog(roleid, outputData);
        for (int i = 0; i < outputData.size(); i++) {
            obj_pvp_log log;
            int64_t challengeid = outputData[i]->mChallengeRoleId;
            if ( !challengeid) {
                break;
            }
            
            int enemyid = 0;
            int enemyType = 0;
            
            getPvpObjTypeAndObjId(challengeid, enemyid, enemyType);
            
            string rolename = "";
            
            if (enemyType == kObjectRole) {
                
                RedisResult result(redisCmd("hget role:%d rolename", challengeid));
                
                rolename = result.readStr();
            }
            else{
                Robot* robot = g_RobotMgr.getData(enemyid);
                if (robot) {
                    rolename = robot->mRoleName;
                }
            }
            
            if (rolename.empty()) {
                continue;
            }
            log.rolename = rolename;
            log.iswin = outputData[i]->mIsWin;
            log.isbechallenge = outputData[i]->mIsBeChallege;
            log.changetype = outputData[i]->mChangeType;
            log.rank = outputData[i]->mRank;
            ack.logs.push_back(log);
        }

        RedisResult result(redisCmd("hmget role:%d %s %s", roleid, GetRolePropName(eRolePvpCoolDown), GetRolePropName(eRolePvpTimes)));
        
        int cooldown = result.readHash(0, 0);
        int pvptimes = result.readHash(1, 0);

        int awardRank = SPvpMgr.safe_GetRoleAwardRank(roleid); 
        
        int restTimes = cooldown - time(NULL);
        check_min(restTimes, 0);

        int pvpfreeTimes = PvpCfg::getfreeTimes();
        int pvpFreshTimes = pvpfreeTimes - pvptimes;
        check_min(pvpFreshTimes, 0);
        int freshcost = PvpCfg::getFreshCost(pvpFreshTimes - 1);

        ack.restTime = restTimes;
        ack.costperminute = freshcost;
        ack.getaward = this->myAward;
        ack.awardrank = awardRank;
        
        PvpAward* award = PvpCfg::getRankAward(myRank);
        ItemArray items;
        RewardStruct reward;
        
        std::vector<std::string> rewards = StrSpilt(award->itemAward, ";");
        rewardsCmds2ItemArray(rewards, items, reward);
        ack.pointsGetPeriod = reward.reward_pvppoints;
        
    }

    sendNetPacket(this->sessionid, &ack);
}

handler_global(deletRoleInPvp)
{
    int roleid = this->roleid;
    SPvpMgr.DeleteRoleInRank(roleid);
}

handler_global(addRoleToPvp)
{
    SPvpMgr.addRoleToRank(this->roleid);
}

handler_global(onAddRoleToPvp)
{
    Role* role = SRoleMgr.GetRole(this->roleid);
    if (role == NULL) {
        return;
    }
    
    if (role->getIsInPvp() != 2) {
        int pvptime = PvpCfg::getfreeTimes();
        role->RolePvpInit();
        role->addPvpTimes(pvptime);
        
        string describe = Utils::makeStr("rolepvp_init");
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesPvp, describe.c_str());
        role->setIsInPvp(2);
    }
}

handler_global(beginPvp)
{
    create_global(beginPvpret, ret);
    ret->errorcode = CE_OK;
    ret->roleid = this->roleid;
    if (!SPvpMgr.IsRealRanking(this->bechallengeRank, this->bechallengeid)) {
        ret->errorcode = CE_PVP_ROLE_IS_NOT_REAL_RANK;
        sendGlobalMsg(Game::MQ, ret);
        return;
    }

    if (SPvpMgr.isRoleChallenged(this->bechallengeid)) {
        ret->errorcode = CE_PVP_SLECTED_ROLE_IS_IN_PVP;
        sendGlobalMsg(Game::MQ, ret);
        return;
    }

    if (SPvpMgr.IsInCoolingDown(this->roleid)) {
        ret->errorcode = CE_PVP_YOU_ARE_IN_COOLING_DOWN;
        sendGlobalMsg(Game::MQ, ret);
        return;
    }
    
    if (SPvpMgr.isRoleChallenging(this->roleid))
    {
        ret->errorcode = CE_PVP_YOU_ARE_IN_COOLING_DOWN;
        sendGlobalMsg(Game::MQ, ret);
        return;
    }
    
//    SPvpMgr.setRoleChallenged(this->bechallengeid, this->roleid);
    SPvpMgr.setRolePvpState(this->roleid, this->bechallengeid);

    SPvpMgr.pushRoleInPvp(roleid);
    SPvpMgr.pushRoleInPvp(this->bechallengeid);
    ret->bechallengeid = this->bechallengeid;
    sendGlobalMsg(Game::MQ, ret);
    return;
}

handler_global(beginPvpret)
{
    ack_begin_pvp ack;
    
    Role* role = SRoleMgr.GetRole((int)this->roleid);
    if (role == NULL) {
        //玩家掉线了，清除Global线程的信息
        create_global(pvpEraseRoleInpvp, erase);
        erase->bechallengeid = this->bechallengeid;
        erase->roleid = this->roleid;
        sendGlobalMsg(Global::MQ, erase);
        return;
    }
    ack.sceneid = PVP_SCENEID;
    
    if (this->errorcode) {
        ack.errorcode = this->errorcode;
        role->setPreEnterSceneid(0);
        sendNetPacket(role->getSessionId(), &ack);
        return;
    }
    
    int bechallengeid = 0;
    int targetType = 0;
    
    //取到真实id和类型
    getPvpObjTypeAndObjId(this->bechallengeid, bechallengeid, targetType);
    
    ack.targetType = targetType;
    
    do
    {
        //如果是角色，直接取数据
        if (targetType == kObjectRole) {
            
            RedisResult result(redisCmd("hget role:%d %s", this->bechallengeid, GetRolePropName(eRolePropRoleName)));
            string enemyName = result.readStr();
            
            if( !enemyName.empty() )
            {
                Role* beChrole = Role::create(this->bechallengeid, true);
                
                ack.enemyinfo.skills.clear();
                
                if (beChrole == NULL) {
                    ack.errorcode = CE_GET_FRIEND_DATA_ERROR;
                    break;
                }
                
                beChrole->onCalPlayerProp();
                beChrole->setMaxHp(beChrole->getMaxHp()*10);
                beChrole->setDef(beChrole->getDef() + 200);
                
                ack.errorcode = 0;
                beChrole->getPlayerBattleProp(ack.enemyinfo);
                
                Pet* pet = beChrole->mPetMgr->getActivePet();
                obj_petBattleProp info;
                int petBattle = 0;
                int activepetId = 0;
                if (pet != NULL && pet->petid > 0)
                {
                    activepetId = pet->petid;
                    pet->getBattleProp(info);
                    petBattle = pet->mBattle;
                }
                else
                {
                    info.petid = 0;
                }
                
                info.maxHp *= 10;
                
                ack.enemypetinfo = info;
                
                Retinue *retinue = beChrole -> mRetinueMgr -> getActiveRetinue();
                obj_retinueBattleProp retinueBPInfo;
                if(retinue != NULL && retinue -> getId() > 0){
                    retinue -> getBattleProp(retinueBPInfo);
                    retinueBPInfo.skills = beChrole -> getRetinueMgr() -> mSkillMgr -> getEquipedSkills();
                }
                ack.enemyretinueinfo = retinueBPInfo;
                
                role->setEnemyVerifyInfo(beChrole, petBattle, activepetId);
                role->setBechallengeBattleForce(beChrole->getBattleForce());
                
                delete beChrole;
                ack.errorcode = CE_OK;
            }
            else{
                ack.errorcode = CE_PVP_GET_ROLE_DATA_ERROR;
                break;
            }
        }
        else if (targetType == kObjectRobot)   //如果是机器人的数据
        {
            //获取这些信息
            Robot* robot = g_RobotMgr.getData(bechallengeid);
            if (robot == NULL) {
                ack.errorcode = CE_PVP_GET_ROLE_DATA_ERROR;
                break;
            }
            
            obj_roleBattleProp info;

            info.atk = robot->mBattleProp.getAtk();
            info.def = robot->mBattleProp.getDef() + 200;
            info.cri = robot->mBattleProp.getCri();
            info.dodge = robot->mBattleProp.getDodge();
            info.hit = robot->mBattleProp.getHit();
            info.maxhp = robot->mBattleProp.getMaxHp() * 10;
            
            info.roleid = bechallengeid;
            info.skills = robot->mSkills;
            info.weaponfashion = 0;
            info.awakeLvl = 0;

            ack.enemyinfo = info;
            ack.errorcode = CE_OK;
            
            role->setEnemyVerifyInfo( &(robot->mBattleProp), 0, 0);
        }
        
    }while(false);
    
    if (this->errorcode == CE_OK) {
        
        role->addPvpBuff();
        
        ack.selfatk = role->mRealBattleProp.getAtk();
        ack.selfMaxhp = role->mRealBattleProp.getMaxHp();
        ack.selfdef = role->mRealBattleProp.getDef();
        ack.selfdodge = role->mRealBattleProp.getDodge();
        ack.selfcri = role->mRealBattleProp.getCri();
        ack.selfhit = role->mRealBattleProp.getHit();
        
        //role->sendPetPropAndVerifycode(role->mPetMgr->getActivePet(), true);
        //role->sendRolePropInfoAndVerifyCode(true);
        
        role->addPvpTimes(-1);
        role->setBechallengeId(this->bechallengeid);
        
        string describe = Utils::makeStr("enterpvp");
        role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesPvp, describe.c_str());
        
        role->setPreEnterSceneid(PVP_SCENEID);
        
        //开始pvp时间
        role->setPvpBeginTime(Game::tick);
        role->setBattleTime(Game::tick);
        
        UpdateQuestProgress("arenabattle", 0, 1, role, true);
    }
    else
    {
        //
        create_global(pvpEraseRoleInpvp, erase);
        erase->bechallengeid = this->bechallengeid;
        erase->roleid = this->roleid;
        sendGlobalMsg(Global::MQ, erase);
    }
    
    sendNetPacket(role->getSessionId(), &ack);
}

handler_global(pvpResult)
{
    if (!SPvpMgr.isRoleChallenging(this->roleid))
    {
        return;
    }
    
    create_global(pvpResultret, ret);
    ret->cooldown = 0;
    ret->broacast = 0;
    int myRank = SPvpMgr.getRankingByRoleId(roleid);
    int otherRank = SPvpMgr.getRankingByRoleId(this->bechallengeid);

    ret->roleid = this->roleid;
    ret->myrank = myRank;

    ret->bechallengeid = this->bechallengeid;
    ret->iswin = this->isWin;
    
    if (this->isWin) {
        //change ranking
        if (myRank > otherRank) {
            SPvpMgr.changRolesRank(roleid, this->bechallengeid);
            SPvpMgr.updatePvpLog(roleid, this->bechallengeid, eRankUp, this->isWin, false);
            SPvpMgr.updatePvpLog(this->bechallengeid, roleid, eRankDown, !this->isWin, true);
            ret->myrank = otherRank;
            if (otherRank == 1) {
                ret->broacast = 1;
            }
        }
        else
        {
            SPvpMgr.updatePvpLog(roleid, this->bechallengeid, eNonChange, this->isWin, false);
            SPvpMgr.updatePvpLog(this->bechallengeid, roleid, eNonChange, !this->isWin, true);
        }

    }
    else{

        SPvpMgr.updatePvpLog(roleid, this->bechallengeid, eNonChange, this->isWin, false);
        SPvpMgr.updatePvpLog(this->bechallengeid, roleid, eNonChange, !this->isWin, true);

    }
    
//    SPvpMgr.unsetRoleChallenged(this->bechallengeid);
//    SPvpMgr.unsetRoleChallenging(this->roleid);

    SPvpMgr.resetRolePvpState(this->roleid, this->bechallengeid);
    
    SPvpMgr.eraseRoleInPvp(this->bechallengeid);
    
    ret->cooldown = SPvpMgr.eraseRoleInPvp(this->roleid, true);
    ret->prerank = myRank;
    ret->battleTime = this->battleTime;
    sendGlobalMsg(Game::MQ, ret);
}

extern int sendNormalCopyAward(Role* role,
                               SceneCfgDef* scenecfg,
                               int flopid,
                               int friendId,
                               int& fcindex,
                               vector<int>& drops,
                               RewardStruct& notifyReward,
                               string &itemsResult);

handler_global(pvpResultret)
{
    Role *role = SRoleMgr.GetRole(this->roleid);
    if (role == NULL) {
        return;
    }
    
    ack_PvpResult ack;
    
    if (this->iswin) {
        //get award
        PvpAward* pvpDef = PvpCfg::getResultAward(true);
        role->addGold(pvpDef->gold, "pvp_win");
        role->addConstellVal(pvpDef->protoss, "pvp_win");
        
        vector<int> drops;
        int fcindex = 0;
        RewardStruct reward;
        
        SceneCfgDef* sceneCfg = SceneCfg::getCfg(PVP_SCENEID);
        
        if (sceneCfg) {
            
            vector<int> drops;
            string dropStr = "";
            string awardBase = SflopcardMgr.RandomCard(PVP_SCENEID, fcindex, drops, dropStr, 0);
            
            ItemArray items;
            RewardStruct reward;
            vector<string> awards = StrSpilt(awardBase, ";");

            if (role->addAwards(awards, items, "pvp_win"))
            {
                ack.award = awardBase;
            }
        }
    }
    else{
        PvpAward* pvpDef = PvpCfg::getResultAward(false);
        role->addGold(pvpDef->gold, "pvp_lose");
        role->addConstellVal(pvpDef->protoss, "pvp_lose");
    }
    
    if(this->broacast)
    {
        broadcastPvpRank(role->getRolename(), this->myrank);
		// 称号检查
		SHonorMou.procHonor(eHonorKey_PvpRankOne, role);
    }
    ack.cooltime = this->cooldown;
    ack.isWin = this->iswin;
    
    sendNetPacket(role->getSessionId(), &ack);
    
    role->setBechallengeId(0);
    
    //默认角色切换回城镇
    role->backToCity();
    
    role->clearEnemyVerifyInfo();
    
	if (this->iswin) {
		// 称号检查
        SHonorMou.procHonor(eHonorKey_PvpKill, role, "", "1");
        if (role->getDailyScheduleSystem()) {
            role->getDailyScheduleSystem()->onEvent(kDailySchedulePvp, 0, 1);
        }
	}
	
    LogMod::addLogGetinPvp(role->getInstID(), role->getRolename().c_str(), role->getSessionId(), role->getBattleForce(), this->bechallengeid, role->getBechallengeBattleForce(), this->battleTime, this->iswin, this->prerank, this->myrank);

    role->setBechallengeBattleForce(0);
}


handler_global(pvpEraseRoleInpvp)
{
    SPvpMgr.eraseRoleInPvp(this->roleid, true);
    SPvpMgr.eraseRoleInPvp(this->bechallengeid);
    
    SPvpMgr.resetRolePvpState(this->roleid, this->bechallengeid);
}

handler_global(regActivityTimer)
{
    SActMgr.CreateTimer(actid, type, relative_secs, timerfun, params);
}

handler_global(reqActivityStats)
{
    SActMgr.SendActiveStats(sessionid);
}

handler_global(getPaihangDataRet)
{
    if (type == eSortPet) {

        GETPAIHANGPETDATAFUNC callbk = (GETPAIHANGPETDATAFUNC) retCallBackFunc;
        if (callbk)
        {
            callbk(roleid, type, petData);
        }
    }
    else
    {
        GETPAIHANGDATAFUNC callbk = (GETPAIHANGDATAFUNC) retCallBackFunc;
        if (callbk)
        {
            callbk(roleid, type, sortData);
        }
    }
}

handler_global(regActivityOnTimer)
{
    ACTTIMERFUN callbk = (ACTTIMERFUN) timerfun;
    if (callbk)
    {
        callbk(actid, number, params);
    }
}

handler_global(onActivityInited)
{
    allActiveInit();

    SActMgr.SetRunning(true);
}
/******************************* petpvp *******************************/
handler_global(getPetPvpData)
{
    create_global(onGetPetPvpData, msg);
    msg->errorcode = CE_OK;
    msg->playerinfo = this->playerinfo;
    do
    {
//        if(!SPetPvpMgr.onPetPvpCheckIn(playerinfo.roleid, playerinfo.lvl))
//        {
//            msg->errorcode = CE_CAN_NOT_FIND_PLAYER_DATA;
//            break;
//        }
        
//        SPetPvpMgr.onPlayerAdvance(playerinfo.roleid, playerinfo.lvl);
        
        
        
        PetPvpData* playerData;
        playerData = SPetPvpMgr.onGetPlayerData(playerinfo.roleid);
        
        if (playerData == NULL) {
            msg->errorcode = CE_CAN_NOT_FIND_PLAYER_DATA;
            break;
        } 
        
        if (playerData->mLvl != msg->playerinfo.lvl) {
            SPetPvpMgr.updatePlayerData(msg->playerinfo.roleid, msg->playerinfo.lvl);
        }
    
        PetPvpAwardData award;
        SPetPvpMgr.onGetWeeklyAward(playerinfo.roleid, playerData, award);
        
        msg->pvptype = playerData->mPvpType;
        msg->randking = playerData->mRank;
        msg->cups = playerData->mCups;
        msg->points = playerData->mPoints;
        playerData->getFormation(msg->petlist);
        
//        msg->historyRank = playerData->mHistoryRank;
        playerData->getHistoryBestRank(msg->historyRank);
        
        msg->weeklyType = award.mPvpType;
        msg->weeklyRank = award.mRank;
        msg->awardpoint = award.mAwardPoint;
        msg->wintimes = playerData->mWinTimes;
        msg->losetimes = playerData->mLostTimes;
        
        if (award.mAwardPoint) {
            create_global(onGetWeeklyAwardPoints, msg);
            msg->roleid = playerData->mRoleid;
            msg->surplusPoints = playerData->mPoints;
            msg->getpoints = award.mAwardPoint;
            sendGlobalMsg(Game::MQ, msg);
        }
        
        
        
    }while(false);
    
    sendGlobalMsg(Game::MQ, msg);
}

handler_global(onGetPetPvpData)
{
    Role* role = SRoleMgr.GetRole(this->playerinfo.roleid);
    
    if (role == NULL) {
        return;
    }
    
    int goldenHistory = getBestHistoryRank(this->pvptype, this->historyRank);
    if (goldenHistory < 0) {
        printf("error\n");
    }
    
	if (goldenHistory == 1) {
		// 称号检查
		SHonorMou.procHonor(eHonorKey_PetPvpRankOne, role->getInstID(), xystr("%d", pvptype), "");
	}
    
    ack_get_petpvp_data ack;
    
    ack.errorcode = this->errorcode;
    ack.randking = this->randking;   //排名
    ack.cups = this->cups;      //所谓的竞技等级
    ack.points = this->points;     //可用积分
    ack.pvptype = this->pvptype ;   //黄金，白银， 青铜段
    ack.pvptimes = role->getPetPvpTimes();   //今天一进行的pvp次数
    ack.wintimes = this->wintimes;  //胜利场次
    ack.losetimes = this->losetimes;  //不胜利场次
    ack.weeklyType = this->weeklyType; //上周结算时的竞技场段位
    ack.weeklyRank = this->weeklyRank; //上周结算排名,为0表示已经发放过积分不提示，非零表示刚领取了积分，客户端提示一下
    ack.awardpoint = this->awardpoint; //上周结算积分
    ack.historyRank = this->historyRank;    //历史战绩
    
    //过滤一下没有的宠物
    bool newList = false;
    for (int i = 0; i < this->petlist.size(); i++) {
    
        int petid = this->petlist[i];
        
        //如果id = 0，不过滤
        if (petid == 0) {
            ack.petlist.push_back(petid);
            continue;
        }
        
        Pet* pet = role->mPetMgr->getPet(petid);
        
        if (pet) {
            ack.petlist.push_back(petid);
        }
        else
        {
            newList = true;
            ack.petlist.push_back(0);
        }
    }
    
    if (newList) {
        create_global(editPetPvpFormation, msg);
        role->getRoleInfo(msg->playerinfo);
        msg->newformation = ack.petlist;
        msg->sessionid = role->getSessionId();
        msg->noAck = true;
        sendGlobalMsg(Global::MQ, msg);
    }
    
    sendNetPacket(role->getSessionId(), &ack);
}

handler_global(onGetWeeklyAwardPoints)
{
    Role* role = SRoleMgr.GetRole(this->roleid);
    if (role == NULL) {
        return;
    }
    
    string getForm = Utils::makeStr("排名奖励");
    
    LogMod::addLogPetPvpPointsChange(this->roleid, role->getRolename().c_str(), role->getSessionId(), this->getpoints, this->surplusPoints, getForm.c_str());
}

handler_global(editPetPvpFormation)
{
    int errorcode = CE_OK;
    if (!SPetPvpMgr.onEditFormation(playerinfo.roleid, playerinfo.lvl, newformation)) {
        errorcode = 1;
    }
    
    if (!this->noAck) {
        ack_edit_petpvp_formation ack;
        ack.errorcode = errorcode;
        ack.petlist = newformation;
        sendNetPacket(this->sessionid, &ack);
    }
}

handler_global(getPetPvpEnemy)
{
    create_global(onGetPetPvpEnemy, msg);
	msg->roleid = playerinfo.roleid;
    msg->errorcode = CE_OK;
    
    do
    {
        if (SPetPvpMgr.isPlayerInPetPvp(this->playerinfo.roleid)) {
            return;
        }
        
        PetPvpData* mydata = SPetPvpMgr.onGetPlayerData(playerinfo.roleid);

        if (mydata == NULL) {
            string error = Utils::makeStr("petpvp not find playerdata role:%d", playerinfo.roleid);
            log_error(error);
            return;
        }
        
        PetPvpSectionCfg* section = PetPvpCfg::getPetPvpSectionCfg((PetPvpType)(mydata->mPvpType));
        if (section == NULL) {
            msg->errorcode = CE_READ_CFG_ERROR;
            break;
        }
        
        if (mydata->mEnemy != NULL) {
            return;
        }
        if (gold < section->mSearchEnemyCost) {
            msg->errorcode = CE_SHOP_GOLD_NOT_ENOUGH;
            break;
        }
        
        if (SPetPvpMgr.isPlayerFormationEmpty(playerinfo.roleid)) {
            msg->errorcode = CE_YOUR_FORMATION_IS_EMPTY;
            break;
        }
        
        PetPvpData* enemy = SPetPvpMgr.matchEnemy(playerinfo.roleid);
        if (enemy == NULL) {
            msg->errorcode = CE_FIND_NO_ENEMY;
            break;
        }
        msg->needGold = section->mSearchEnemyCost;
        msg->enemyInfo.roleid = enemy->mRoleid;
        msg->enemyInfo.lvl = enemy->mLvl;
        msg->enemyInfo.rolename = enemy->mRolename;
        msg->enemyInfo.roletype = enemy->mRoletype;
        msg->enemyPvptype = enemy->mPvpType;
        msg->enemyRnak = enemy->mGlobalRank;
        
        for (int i = 0; i < enemy->mPetsForPvp.mPets.size(); i++) {
            msg->enemyPetformation.push_back(enemy->mPetsForPvp.mPets[i]);
        }
    }while(false);
    sendGlobalMsg(Game::MQ, msg);
}

handler_global(onGetPetPvpEnemy)
{
    Role* role = SRoleMgr.GetRole(roleid);
    if (role == NULL) {
        return;
    }
    
    ack_search_petpvp_enemy ack;
    ack.errorcode = CE_OK;
    
    do
    {
        if (errorcode) {
            ack.errorcode = errorcode;
            break;
        }
        
        ack.roleid = this->enemyInfo.roleid;
        ack.rolelvl = this->enemyInfo.lvl;
        ack.rolename = this->enemyInfo.rolename;
        ack.roletype = this->enemyInfo.roletype;
        ack.pvptype = this->enemyPvptype;
        ack.rank = this->enemyRnak;
        for (int i = 0; i < enemyPetformation.size(); i++) {
            Pet pet(enemyPetformation[i]);
            
            pet.load(get_DbContext());
            if (pet.petid == 0) {
                continue;
            }
            
            obj_petBattleProp info;
            pet.getBattleProp(info);
            info.hp *= PetPvpCfg::mCfgData.mPetMaxHpCoef;
            info.maxHp *= PetPvpCfg::mCfgData.mPetMaxHpCoef;
            ack.petsinfo.push_back(info);
        }
    }while(false);

    sendNetPacket(role->getSessionId(), &ack);
    
    if (errorcode == CE_OK) {
        role->addGold(-needGold);
        role->addPetPvpTime(1);
        role->mPetPvpMatching = true;
    }
}

handler_global(beginPetPvpBattle)
{
    create_global(onBeginPetPvpBattle, msg);
    msg->errorcode = CE_OK;
    msg->roleid = roleid;
    msg->errorcode = CE_FIND_NO_ENEMY;
    
    if (SPetPvpMgr.onBeginBattle(roleid, enemyid)) {
        
        msg->errorcode = CE_OK;
        msg->sceneid = this->sceneid;
        
        SPetPvpMgr.onGetPvpPetList(this->roleid, msg->petlist);
        
        sendGlobalMsg(Game::MQ, msg);
    }
    
}

handler_global(onBeginPetPvpBattle)
{
    Role* role = SRoleMgr.GetRole(roleid);
    
    if (role == NULL) {
        return;
    }
    role->setBattleTime(Game::tick);
    role->mPetPvpMatching = false;
    
    ack_begin_petpvp ack;
    ack.errorcode = errorcode;
    
    if( ack.errorcode == CE_OK)
    {
        role->mPetpvpFormation.clear();
        role->mPetpvpFormation = this->petlist;
        
        //出战宠的血量*10通知前端
        for (int i = 0; i < this->petlist.size(); i++) {
            Pet* pet = role->mPetMgr->getPet(this->petlist[i]);
            if (pet) {
                pet->mRealBattleProp.setMaxHp(pet->getMaxHp() * PetPvpCfg::mCfgData.mPetMaxHpCoef);
                pet->makePropVerifyCode();
                role->sendPetPropAndVerifycode(pet, true);
            }
        }
        role->setPreEnterSceneid(this->sceneid);
    }
    
    ack.sceneid = this->sceneid;
    
    sendNetPacket(role->getSessionId(), &ack);
}

handler_global(petPvpBattleHandle)
{
    if ( !SPetPvpMgr.isPlayerInPetPvp(this->playerinfo.roleid) ) {
        return;
    }
    
    int givePoints = 0;
    if (pvptimes <= PetPvpCfg::mCfgData.mAwardTimes) {
    
        givePoints = iswin == 0 ? PetPvpCfg::mCfgData.mLostGivePoints : PetPvpCfg::mCfgData.mWinGivePoints;
        
    }
    
    int enemyGetPoints = 0;
    int enemySurplusPoints = 0;
    int enemyId;
    int getCups = 0;
    
    PetPvpData* playerData = SPetPvpMgr.onUpdatePlayerData(playerinfo.roleid, iswin, givePoints, getCups, enemyId, enemyGetPoints, enemySurplusPoints);
    if (playerData != NULL) {
        create_global(onPetPvpBattleHandle, msg);
        msg->roleid = playerData->mRoleid;
        msg->rank = playerData->mRank;
        msg->globalRank = playerData->mGlobalRank;
        msg->winTimes = playerData->mWinTimes;
        msg->pvptype = playerData->mPvpType;
        msg->iswin = this->iswin;
        playerData->getHistoryBestRank( msg->bestHistoryRanks );
        msg->getCups = getCups;
        msg->surplusPoints = playerData->mPoints;
        msg->givePoints = givePoints;
        msg->enemyid = enemyId;
        msg->enemyGetPoints = enemyGetPoints;
        msg->enemySurplusPoints = enemySurplusPoints;
        msg->errorcode = CE_OK;
        
        sendGlobalMsg(Game::MQ, msg);
    }
    
    SPetPvpMgr.eraseRole(playerinfo.roleid);
}

handler_global(onPetPvpBattleHandle)
{
    //do something
    
    //example
    
    int goldenHistory = getBestHistoryRank(pvptype, this->bestHistoryRanks);
    if (goldenHistory < 0) {
        printf("error\n");
    }
    
    if (iswin) {
		// 称号检查
		SHonorMou.procHonor(eHonorKey_PetPvpKill, roleid, "", "1");
	}
	
	if (goldenHistory == 1) {
		// 称号检查
		SHonorMou.procHonor(eHonorKey_PetPvpRankOne, roleid, xystr("%d", pvptype), "");
	}
    
    Role* role = SRoleMgr.GetRole(this->roleid);
    
    //先更新服务器这边的信息
    if (role == NULL) {
        RedisResult result(redisCmd("hmget role:%d %s %s", this->roleid, GetRolePropName(eRolePropRoleName), GetRolePropName(eRolePropPlayerAccount) ));
        string name = result.readHash(0, "");
        string account = result.readHash(1, "NULL");
        
        if (name.empty()) {
            return;
        }
        
        LogMod::addLogPetPvpPointsChange(this->roleid, name.c_str(), account.c_str(), this->givePoints, this->surplusPoints, "幻兽竞技场战斗中退出");
    }
    else
    {
        //通知客户端更新
        ack_commit_petpvp_battle_report ack;
        ack.getPoints = givePoints;
        ack.iswin = iswin;
        ack.errorcode = CE_OK;
        ack.getCups = getCups;
        sendNetPacket(role->getSessionId(), &ack);
        
        //出战宠的属性恢复正常
        for (int i = 0; i < role->mPetpvpFormation.size(); i++) {
            Pet* pet = role->mPetMgr->getPet(role->mPetpvpFormation[i]);
            if (pet) {
                pet->mRealBattleProp.setMaxHp(pet->getMaxHp());
                pet->makePropVerifyCode();
                role->onSendPetPropAndVerifycode(pet, true);
            }
        }

        string getFrom = Utils::makeStr("战斗结算");
        LogMod::addLogPetPvpPointsChange(roleid, role->getRolename().c_str(), role->getSessionId(), this->givePoints, this->surplusPoints, getFrom.c_str());
        
        role->setCurrSceneId(role->getSceneID());
    }
    
    //如果获胜，敌人就没获得积分，返回
    if (this->iswin) {
        if (role && role->getDailyScheduleSystem()) {
            role->getDailyScheduleSystem()->onEvent(kDailySchedulePetArena, 0, 1);
        }
        
        return;
    }
    
    Role* enemy = SRoleMgr.GetRole(this->enemyid);
    if (enemy == NULL) {
        RedisResult result(redisCmd("hmget role:%d %s %s", this->enemyid, GetRolePropName(eRolePropRoleName), GetRolePropName(eRolePropPlayerAccount) ));
        string name = result.readHash(0, "");
        string account = result.readHash(1, "NULL");
        
        if (name.empty()) {
            return;
        }
        
        LogMod::addLogPetPvpPointsChange(this->enemyid, name.c_str(), account.c_str(), this->enemyGetPoints, this->enemySurplusPoints, "幻兽竞技场被挑战胜利");
    }
    else
    {
        LogMod::addLogPetPvpPointsChange(this->enemyid, enemy->getRolename().c_str(), enemy->getSessionId(), this->enemyGetPoints, this->enemySurplusPoints, "幻兽竞技场被挑战胜利");
    }
}

handler_global(delPlayerInPetPvp)
{
    SPetPvpMgr.delayDelPalyer(roleid);
}

//handler_global(checkPetPvpLvlUpGrade)
//{
//    SPetPvpMgr.onPlayerAdvance(playerinfo.roleid, playerinfo.lvl);
//}

handler_global(endSearchPetPvpEnemy)
{
    SPetPvpMgr.onPlayerEndSearchEnemy(playerinfo.roleid);
}

handler_global(removePetFromPetPvpFormation)
{
    SPetPvpMgr.onRemovePet(roleid, petlist);
}

handler_global(getPetPvpPetList)
{
    ack_get_petpvp_formation ack;
    SPetPvpMgr.onGetPvpPetList(roleid, ack.petlist);
    sendNetPacket(this->sessionid, &ack);
}

handler_global(buyItemsUsingPetpvpPoints)
{
    create_global(onBuyItemsUsingPetpvpPoints, msg);
    msg->roleid = this->roleid;
    msg->sessionid = this->sessionid;
    int surplus = 0;
    int ret = SPetPvpMgr.onBuyItemsUsingPoints(roleid, itemid, count, surplus);

    if (ret < 0) {
        msg->errorcode = CE_CAN_NOT_FIND_CFG;
    }
    else if(ret == 0){
        msg->errorcode = CE_YOUR_POINTS_NOT_ENOUGH;
    }
    else{
        msg->errorcode = CE_OK;
        msg->usingPoints = ret;
        msg->surplusPoints = surplus;
        msg->itemid = itemid;
        msg->count = count;
    }
    
    sendGlobalMsg(Game::MQ, msg);
}

handler_global(onBuyItemsUsingPetpvpPoints)
{
    ack_get_item_by_consum_petpvp_point ack;
    ack.errorcode = this->errorcode;
    if (ack.errorcode == CE_OK) {
        Role* role = SRoleMgr.GetRole(this->roleid);
        if (role == NULL) {
            return;
        }
        
        ItemArray items;
        GridArray effgrids;
        items.push_back(ItemGroup(this->itemid, this->count));
        
        if (role->preAddItems(items, effgrids) == CE_OK) {
//            role->updateBackBag(effgrids, items, true, "幻兽竞技场积分兑换");
            role->playerAddItemsAndStore(effgrids, items, "幻兽竞技场积分兑换", true);
        }
        ack.consumPoints = this->usingPoints;
        
        string usage = Utils::makeStr("积分兑换:%d*%d", this->itemid, this->count);
        LogMod::addLogPetPvpPointsChange(this->roleid, role->getRolename().c_str(), role->getSessionId(), -this->usingPoints, this->surplusPoints, usage.c_str());
        
    }
    sendNetPacket(this->sessionid, &ack);
}                                                                                                       

handler_global(gmAddPetPvpPoints)
{
    SPetPvpMgr.addPetPvpPoints(roleid, addpoints);
}
handler_global(gmSetPetPvpPoints)
{
    SPetPvpMgr.setPetPvpPoints(roleid, setpoints);
}

handler_global(onGmAddPetPvpPoints)
{
    Role* role = SRoleMgr.GetRole(this->roleid);
    if (role == NULL) {
        return;
    }
    
    string usage = Utils::makeStr("gm命令或邮件附件");
    LogMod::addLogPetPvpPointsChange(this->roleid, role->getRolename().c_str(), role->getSessionId(), this->getPoints, this->surplusPoints, usage.c_str());
}

handler_global(gmSetPetPvpCups)
{
    SPetPvpMgr.setPetPvpCups(this->roleid, this->setCups);
}

/********************************** 名人堂 ***********************************/
handler_global(getFamesHallList)
{
    g_fameHallMgr.onClientGetFamesHallFameList(playerinfo.roleid, this->sessionid);
}

handler_global(getFamesHallPlayerData)
{
    g_fameHallMgr.onGetPlayerData(playerinfo.roleid, this->sessionid, this->versionid);
    g_fameHallMgr.testMyPoints(playerinfo, this->sessionid);
}

handler_global(beginFamesHallBattle)
{
    create_global(onBeginFamesHallBattle, msg);
    
    msg->errorcode = g_fameHallMgr.onBeginBattle(playerinfo.roleid, this->checkLayer);
    
    msg->sceneid = this->sceneid;
    
    msg->roleid = this->playerinfo.roleid;
    
    sendGlobalMsg(Game::MQ, msg);
}

handler_global(onBeginFamesHallBattle)
{
    ack_fameshall_beginbattle ack;
    Role* role = SRoleMgr.GetRole(this->roleid);
    ack.errorcode = this->errorcode;
    
    if (role == NULL) {
        return;
    }
    if (this->errorcode == CE_OK) {
        ack.sceneid = this->sceneid;
    }
    else
    {
        role->setPreEnterSceneid(role->getSceneID());
    }
    sendNetPacket(role->getSessionId(), &ack);
}

handler_global(famesHallBattleHandle)
{
    create_global(onFamesHallBattleHandle, msg);
    
    msg->roleid = this->playerinfo.roleid;
    
    msg->result = this->result;
    
    msg->sceneid = this->sceneid;
    
    msg->layer = this->layer;
    
    ObjJob job = RoleType2Job(playerinfo.roletype);
    
    msg->errorcode = g_fameHallMgr.onBattleHandle(this->playerinfo.roleid, job, this->result, this->layer, this->factor);
    
    sendGlobalMsg(Game::MQ, msg);
}

handler_global(onFamesHallBattleHandle)
{
    Role* role = SRoleMgr.GetRole(this->roleid);
    if (role == NULL) {
        return;
    }
    
    if (errorcode != CE_OK) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    ack_finish_famesHall ack;
    
    if (this->result == eDungeFinish) {
        FamousDungeCfgDef* cfg = FamousCfg::getFamousDungeCfg(this->layer);
        if (cfg != NULL) {
            string awardStr = cfg->award;
            vector<string> awards = StrSpilt(awardStr, ";");
            
            ItemArray items;
            
            string comefrom = Utils::makeStr("名人堂奖励:%d", this->layer);
            
            role->addAwards(awards, items, comefrom.c_str());
            
            role->mFamesHallAward.push_back(this->layer);
            
            role->setBattleTime(0);
            
            LogMod::addLogGetFamesHallAward(role, this->layer, awardStr);
            
            if (role->getDailyScheduleSystem()) {
                role->getDailyScheduleSystem()->onEvent(kDailyScheduleFamousHall, 0, this->layer);
                
            }
            
            UpdateQuestProgress("finishfamoushall", layer, 1, role, true);
        }
    }
    
    ack.result = this->result;
    ack.errorcode = this->errorcode;
    
    for (int i = 0; i < role->mFamesHallAward.size(); i++) {
        ack.awards.push_back(role->mFamesHallAward[i]);
    }
    
    role->mFamesHallAward.clear();
    sendNetPacket(role->getSessionId(), &ack);
}

handler_global(GetFamesHallData)
{
    vector<FamesRankData> data;
    create_global(onGetFamesHallData, msg);
    msg->datas = g_fameHallMgr.gameServerGetAllJobFamesList();
    msg->cb = this->cb;
    
    sendGlobalMsg(Global::MQ, msg);
}

handler_global(onGetFamesHallData)
{
    if (this->cb) {
        GETFAMESHALLDATAFUNC func = (GETFAMESHALLDATAFUNC)this->cb;
        func(this->datas);
    }
}


/********************************** RealPvp ***********************************/

//获取自己的竞技场相关信息
handler_global(GetRealpvpInfo)
{
    create_global(onGetRealpvpInfo, msg);
    
    msg->roleid = this->roleid;
    
    g_RealpvpMgr.getMyRealpvpInfo(this->roleid, this->matchingIndex, this->roleinfo, msg->info);
    
    msg->CurrnetMatchingIndex = g_RealpvpMgr.getIndex();
    
    msg->beginTimes = g_RealpvpMgr.getBeginTimes();
    
    sendGlobalMsg(Game::MQ, msg);
}

handler_global(onGetRealpvpInfo)
{
    Role* role = SRoleMgr.GetRole(this->roleid);
    
    if (role == NULL) {
        return;
    }
    
    //场次变更，重新累计匹配次数(2.6 修改为每天第一场才重置)
    if (!isSameDay(role->getRealPvpFreshMatchTimes(), this->beginTimes)) {
        
        if (role->getRealPvpFreshMatchTimes() < this->beginTimes) {
            
            role->setRealPvpMatchIndex(this->CurrnetMatchingIndex);
            role->setRealPvpFreshMatchTimes(this->beginTimes);
            role->setRealPvpMatchTimes(0);
            role->NewRoleProperty::save();
            
            string describe = Utils::makeStr("daily_fresh");
            role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesSyncPvp, describe.c_str());
            
        }
    }
    
    ack_get_realpvp_data ack;
    
    ack.errorcode = CE_OK;
    
    ack.awardTimes = SynPvpFuntionCfg::getAwardTimes(role->getRealPvpMatchTimes());
    
    ack.roleid = this->roleid;
    
    ack.pvpinfo = this->info;
    
    sendNetPacket(role->getSessionId(), &ack);
}

//玩家通知服务器进入匹配队列
handler_global(ReadyToRealpvpMatching)
{
    create_global(onReadyToRealpvpMatching, msg);
    msg->roleid = this->roleid;
    msg->errorcode = CE_OK;
    
    if ( !g_RealpvpMgr.throwToMatchingWorker(this->roleid, this->roleinfo, this->todayTimes) )
    {
        msg->errorcode = CE_REALPVP_ENTER_MATCHING_ERROR;
    }
    sendGlobalMsg(Game::MQ, msg);
    
}

handler_global(onReadyToRealpvpMatching)
{
    Role* role = SRoleMgr.GetRole(this->roleid);
    
    if (role == NULL) {
        return;
    }
    
    ack_bereadyto_realpvp_matching ack;
    ack.errorcode = this->errorcode;
    ack.matchCountDown = SynPvpFuntionCfg::getMatchingSec();
    sendNetPacket(role->getSessionId(), &ack);
}

//玩家通知服务器取消匹配
handler_global(CancelMatching)
{
    if (g_RealpvpMgr.cancelMatching(this->roleid)) {
        
        create_global(onCancelMatching, msg);
        
        msg->roleid = this->roleid;
        
        sendGlobalMsg(Game::MQ, msg);
    }
}

handler_global(onCancelMatching)
{
    Role* role = SRoleMgr.GetRole(this->roleid);
    
    if (role == NULL) {
        return;
    }
    
    ack_cancel_realpvp_matching ack;
    
    ack.errorcode = CE_OK;
    
    sendNetPacket(role->getSessionId(), &ack);
}

//把匹配的结果通知Game线程，通知玩家及同步场景线程
handler_global(onNotifyRealpvpMatchingResult)
{
    Role* firstRole = SRoleMgr.GetRole(this->firstRoleid);
    Role* secondeRole = SRoleMgr.GetRole(this->secondeRoleid);
    
    //这里如果网速渣的话，可能会产生一个问题，如果其中一个退了，退的那个人次数没扣，没退的那个人被判断赢了，by wangzhigang
    
    if (firstRole) {
        notify_realpvp_matching_info firstNotify;
        firstNotify.enemyid = secondeRoleid;
        firstNotify.enemyinfo = this->secondRoleInfo;
        firstNotify.fightCountDown = SynPvpFuntionCfg::getfightingCountDown();
        sendNetPacket(firstRole->getSessionId(), &firstNotify);
        
        firstRole->setRealPvpMatchTimes(firstRole->getRealPvpMatchTimes() + 1);
        firstRole->NewRoleProperty::save();
        
        string describe = Utils::makeStr("syncpvp_match");
        firstRole->addLogActivityCopyEnterTimesChange(eActivityEnterTimesSyncPvp, describe.c_str());
    }

    if (secondeRole) {
        notify_realpvp_matching_info secondNotify;
        secondNotify.enemyid = firstRoleid;
        secondNotify.enemyinfo = this->firstRoleInfo;
        secondNotify.fightCountDown = SynPvpFuntionCfg::getfightingCountDown();
        sendNetPacket(secondeRole->getSessionId(), &secondNotify);
        
        secondeRole->setRealPvpMatchTimes(secondeRole->getRealPvpMatchTimes() + 1);
        secondeRole->NewRoleProperty::save();
        
        string describe = Utils::makeStr("syncpvp_match");
        secondeRole->addLogActivityCopyEnterTimesChange(eActivityEnterTimesSyncPvp, describe.c_str());
    }
    
    if(firstRole == NULL){
        create_global(HandleRealpvpFightingResult, msg);
        msg->winnerid = secondeRoleid;
        msg->loserid = firstRoleid;
        sendGlobalMsg(Global::MQ, msg);
    }
    
    if (secondeRole == NULL)
    {
        create_global(HandleRealpvpFightingResult, msg);
        msg->winnerid = firstRoleid;
        msg->loserid = secondeRoleid;
        sendGlobalMsg(Global::MQ, msg);
    }
}

extern WorldPlayerInfo getWorldPlayerInfo(Role* role);
extern WorldRoleInfo getWorldRoleInfo(Role* role);
extern WorldPetInfo getWorldPetInfo(Pet* pet);

handler_global(onSendBeginMsgToSynScene)
{
    Role* firstRole = SRoleMgr.GetRole(this->firstRole);
    Role* secondRole = SRoleMgr.GetRole(this->secondRole);
    
    if (firstRole == NULL && secondRole == NULL) {
        
        create_global(ResetRealpvpFightingState, msg);
        
        msg->firstRole = this->firstRole;
        msg->secondRole = this->secondRole;
        
        sendGlobalMsg(Global::MQ, msg);
        
        return;
    }
    
    if (firstRole == NULL) {
        create_global(RealpvpFightingEnemyEscape, msg);
        msg->roleid = this->secondRole;
        msg->enemyid = this->firstRole;
        sendGlobalMsg(Global::MQ, msg);
        return;
    }
    else if(secondRole == NULL){
        create_global(RealpvpFightingEnemyEscape, msg);
        msg->roleid = this->firstRole;
        msg->enemyid = this->secondRole;
        sendGlobalMsg(Global::MQ, msg);
        return;
    }
    
    create_cmd(CreateWorldScene, createWorldScene);
    createWorldScene->sceneMod = this->sceneMod;
    createWorldScene->extendStr = "real_pvp";
    
    //第一个玩家
    firstRole->addSyncPvpBuff();
    WorldPlayerInfo firstPlayerInfo = getWorldPlayerInfo(firstRole);
    createWorldScene->playerinfos.push_back(firstPlayerInfo);
    
    //第2个玩家
    secondRole->addSyncPvpBuff();
    WorldPlayerInfo secondPlayerInfo = getWorldPlayerInfo(secondRole);
    createWorldScene->playerinfos.push_back(secondPlayerInfo);

    
    sendMessageToWorld(0, CMD_MSG, createWorldScene, 0);
}

//通知玩家匹配失败
handler_global(onNotifyRealpvpMatchingFail)
{
    notify_realpvp_matching_fail notify;
    Role* role = SRoleMgr.GetRole(this->roleid);
    
    if (role) {
        notify.awardMedal = this->award.medal;
        notify.awardHonor = this->award.honor;
        sendNetPacket(role->getSessionId(), &notify);
    }
}

//处理
handler_global(HandleRealpvpFightingResult)
{
    create_global(onHandleRealpvpFightingResult, winmsg);
    winmsg->roleid = this->winnerid;
    winmsg->result = 1;
    
    create_global(onHandleRealpvpFightingResult, losermsg);
    losermsg->roleid = this->loserid;
    losermsg->result = 0;
    
    g_RealpvpMgr.handleFightingResult(winnerid, loserid, winmsg->getAward, losermsg->getAward);
    
    sendGlobalMsg(Game::MQ, winmsg);
    sendGlobalMsg(Game::MQ, losermsg);
}

handler_global(onHandleRealpvpFightingResult)
{
    Role* role = SRoleMgr.GetRole(this->roleid);
    
    if (role == NULL) {
        return;
    }
    
    notify_realpvp_fighting_result notify;
    notify.result = this->result;
    notify.awardMedal = 0;
    notify.awardHonor = 0;
    
    if (this->result) {
        notify.awardMedal = this->getAward.medal;
        notify.awardHonor = this->getAward.honor;
    }
    else{
        notify.awardMedal = this->getAward.medal;
        notify.awardHonor = this->getAward.honor;
    }
    
    notify.errorcode = CE_OK;
    sendNetPacket(role->getSessionId(), &notify);
}

handler_global(RealpvpMedalExchange)
{
    if (this->itemCount <= 0) {
        return;
    }
    
    int errorcode = g_RealpvpMgr.medalExchange(this->roleid, this->itemIndex, this->itemCount);
    
    create_global(onRealpvpMedalExchange, msg);
    
    msg->roleid = this->roleid;
    msg->errorcode = errorcode;
    msg->itemIndex = this->itemIndex;
    msg->itemCount = this->itemCount;
    
    sendGlobalMsg(Game::MQ, msg);
}

handler_global(onRealpvpMedalExchange)
{
    Role* role = SRoleMgr.GetRole(this->roleid);
    
    ack_realpvp_medal_exchange ack;
    ack.errorcode = this->errorcode;
    ack.itemCount = this->itemCount;
    
    SynPvpStoreGoodsDef* itemdef = SynPvpStoreCfg::getGoodsDefByIdx(itemIndex);
    
    if (itemdef == NULL) {
        ack.errorcode = CE_REALPVP_CAN_NOT_FIND_THIS_ITEM;
        
        if (role) {
            sendNetPacket(role->getSessionId(), &ack);
            return;
        }
    }
    
    ItemGroup newitem;
    ItemArray items;
    GridArray effgrids;
    
    newitem.item = itemdef->goodsId;
    newitem.count = this->itemCount;
    
    items.push_back(newitem);
    
    if (role == NULL) {
        
        string itemdesc = Utils::makeStr("");
        
        vector<string> attachs;
        
        RewardStruct reward;
        reward2MailFormat(reward, items, NULL, &attachs);
        string attach = StrJoin(attachs.begin(), attachs.end());
        
        StoreOfflineItem(this->roleid, attach.c_str());
        return;
    }
    
    do
    {
        if (ack.errorcode != CE_OK) {
            break;
        }
        
        GridArray effgrids;

        if(role->preAddItems(items, effgrids) != CE_OK){
            ack.errorcode = CE_BAG_FULL;
            break;
        }
//        role->updateBackBag(effgrids, items, true, "realpvp_exchange");
        role->playerAddItemsAndStore(effgrids, items, "realpvp_exchange", true);

    }while(false);
    
    sendNetPacket(role->getSessionId(), &ack);
}

handler_global(GmAddRealpvpMedal)
{
    g_RealpvpMgr.addMedal(this->roleid, this->addmedal);
}

handler_global(GmSetRealpvpMilitary)
{
    g_RealpvpMgr.gmSetMilitary(this->roleid, this->index);
}

handler_global(GmSetRealpvpMeds)
{
    g_RealpvpMgr.gmSetMedal(this->roleid, this->setMeds);
}

handler_global(GmSetRealpvpWinTimes)
{
    g_RealpvpMgr.gmSetWinningTimes(this->roleid, this->setWinTimes);
}

handler_global(GmSetRealpvpLostTimes)
{
    g_RealpvpMgr.gmSetLostTimes(this->roleid, this->setLostTimes);
}

handler_global(GmSetRealpvpHons)
{
    g_RealpvpMgr.gmSetHonour(this->roleid, this->setHons);
}

handler_global(GmAddRealpvpWins)
{
    g_RealpvpMgr.gmAddWins(this->roleid, this->addWins);
}

handler_global(onRealpvpMedalChangeLog)
{
    Role* role = SRoleMgr.GetRole(this->roleid);
    
    if (role == NULL) {
        return;
    }
    
    if (strcmp(this->type.c_str(), "medal") == 0) {
        
        LogMod::addLogSynPvpMedal(role->getInstID(), role->getRolename().c_str(), this->changecount, this->totalcount, this->changeFrom);
    }
}

handler_global(ResetRealpvpFightingState)
{
    g_RealpvpMgr.resetPlayerState(this->firstRole);
    g_RealpvpMgr.resetPlayerState(this->secondRole);
}

handler_global(RealpvpFightingEnemyEscape)
{
    MatchModel* model = g_RealpvpMgr.findModel(roleid);
    if (model == NULL) {
        return;
    }
    
    addRealPvpMatchFailAward(model);
    
    g_RealpvpMgr.resetPlayerState(this->enemyid);
}

handler_global(GmSetGuildLvl)
{
    
    SGuildMgr.gmSetLvl(this->roleid ,this->num);
    
}
handler_global(GmSetGuildFortune)
{
    
    SGuildMgr.gmSetForturn(this->roleid ,this->num);
}
handler_global(GmSetGuildDcons)
{
    
    SGuildMgr.gmSetDcons(this->roleid ,this->num);
}

handler_global(onSendPvpAward)
{
    int rankSize = this->ranks.size();
    int roleSize = this->roles.size();
    
    if (rankSize != roleSize) {
        //记录日志
        
        return;
    }
    
    vector<int> playerRoles;
    vector<int> pvpPoints;
    playerRoles.clear();
    pvpPoints.clear();
    
    {
        MyPipeRedis pipeRedis(get_DbContext());
        
        for (int i = 0; i < roleSize; i++) {
            
            int roleid;
            int objType;
            getPvpObjTypeAndObjId(this->roles[i], roleid, objType);
            
            if (objType == kObjectRobot) {
                continue;
            }

            if( REDIS_OK != pipeRedis.myRedisAppendCommand("hget role:%d PvpPoints", roleid))
            {
                continue;
            }
            
            playerRoles.push_back(roleid);
        }
    
        int effeRoleSize = playerRoles.size();
        for (int i = 0; i < effeRoleSize; i++) {
            redisReply* reply = pipeRedis.myRedisGetReply();
            
            if (reply == NULL) {
                playerRoles[i] = 0;
                pvpPoints.push_back(0);
                continue;
            }
            
            RedisResult result(reply);
            
            int point = result.readInt();
            
            pvpPoints.push_back(point);
        }
    }
    
    MyPipeRedis resultPipeRedis(get_DbContext());
    
    for (int i = 0; i < playerRoles.size(); i++) {

        int getPoints = calcPvpPointsAward(ranks[i]);
        
        int roleid = playerRoles[i];
        if (roleid <= 0) {
            continue;
        }
        
        Role* role = SRoleMgr.GetRole(roleid);
        if (role) {
            role->setPvpPoints(role->getPvpPoints() + getPoints);
        }
        
        resultPipeRedis.myRedisAppendCommand("hset role:%d PvpPoints %d", roleid, pvpPoints[i] + getPoints);
    }
}

