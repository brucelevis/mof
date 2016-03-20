//
//  task_hand.cpp
//  GameSrv
//
//  Created by cxy on 13-1-10.
//
//

#include "flopcard.h"
#include "hander_include.h"
#include "Pet.h"
#include "cmd_def.h"
#include "Game.h"
#include "daily_schedule_system.h"
#include "Quest.h"
#include "Honor.h"

 //获取进行中的任务
 hander_msg(req_getQuests,req)
 {
     hand_Sid2Role_check(sessionid,roleid,role);

     ack_getQuests ack;
     ack.dailycount = role->getDailyQuestCount();
     for (std::map<int, std::vector<int> >::iterator Iter = role->mQuestProgress.begin();
          Iter != role->mQuestProgress.end(); ++Iter) {
         obj_questProgress obj;
         obj.questid = Iter->first; //id
         obj.progress = Iter->second;
         ack.questProgress.push_back(obj);
     }
     ack.finishQuestIds = role->mFinishQuestIds;
     sendNetPacket(sessionid, &ack);
 }}

 //接受任务 npc,主,支线
 hander_msg(req_recvQuest,req)
 {
     hand_Sid2Role_check(sessionid,roleid,role);
     ack_recvQuest ack;
     ack.questid = req.questid;

     //检查是否可以接受此任务
     Quest* quest = QuestMgr::FindQuest(req.questid);
     if (quest == NULL) {
         return;
     }
     
     if (quest->type == qtDailyQuest)
     {
         return;
     }
     
     if (role->recvQuest(req.questid)) {

        const std::vector<int>& Progress = role->mQuestProgress[req.questid];
        std::string strProgress = StrJoin(Progress.begin(),Progress.end(),";");
        doRedisCmd("hset questProgress:%d %d %s",roleid,req.questid,strProgress.c_str());
     }
     else
     {
         ack.err = 1;
     }
     sendNetPacket(sessionid, &ack);
 }}


 //放弃任务
 hander_msg(req_giveupQuest,req)
 {
     hand_Sid2Role_check(sessionid,roleid,role);

     ack_giveupQuest ack;
     ack.errorcode = 1;
     ack.questid = req.questid;
     if(role->mQuestProgress.erase(req.questid) != 0)
     {
         ack.errorcode = 0;
         sendNetPacket(sessionid,&ack);
         doRedisCmd("hdel questProgress:%d %d",roleid,req.questid);
         
         create_cmd(dailyquest, dailyquest);
         dailyquest->roleid = roleid;
         sendCmdMsg(Game::MQ, dailyquest);
     }
     else
     {
         sendNetPacket(sessionid,&ack);
     }
 }}


 //完成任务 领取奖励
 hander_msg(req_finishQuest,req)
 {
     hand_Sid2Role_check(sessionid,roleid,role);

     ack_finishQuest ack;
     ack.questid = req.questid;
     if (role->checkQuest(req.questid))
     {
         Quest* quest = QuestMgr::FindQuest(req.questid);
         std::vector<std::string> rewards = quest->rewards;
         switch (role->getJob()) {
             case eWarrior:  // 剑士
                 copy_all(quest->warrior_rewards,std::back_inserter(rewards));
                 break;
             case eMage:    // 法师
                 copy_all(quest->mage_rewards,std::back_inserter(rewards));
                 break;
             case eAssassin:// 刺客
                 copy_all(quest->assassin_rewards,std::back_inserter(rewards));
                 break;
            default:
                 break;
         }

         ItemArray items;
         RewardStruct reward;

         rewardsCmds2ItemArray(rewards,items,reward);

         GridArray effgrids;
         //检查是否可以发放奖励
         int canAddItem = role->preAddItems(items,effgrids);

         if (canAddItem != CE_OK) {
             ack.errorcode = CE_BAG_FULL;
             sendNetPacket(sessionid,&ack);
             return;
         }

         role->finshQuest(req.questid);
         doRedisCmd("hdel questProgress:%d %d",roleid,req.questid);

         if (quest->type != qtDailyQuest) {
             role->mFinishQuestIds.push_back(req.questid);
             doRedisCmd("sadd questFinish:%d %d",roleid,req.questid);
         }

         char buf[32] = "";
         snprintf(buf, sizeof(buf), "finis_quest:%d", req.questid);

         //BackBag::UpdateBackBag(role->getInstID(), effgrids);
         //LogMod::addLogGetItem(roleid, role->getRolename().c_str(), sessionid, items, buf);
//         role->updateBackBag(effgrids, items, true, buf);
         role->playerAddItemsAndStore(effgrids, items, buf, true);
         role->addExp(reward.reward_exp, buf);
         role->addGold(reward.reward_gold, buf);
         role->addBattlePoint(reward.reward_batpoint,buf);
         role->addConstellVal(reward.reward_consval, buf);
         role->addEnchantDust(reward.reward_enchantdust, buf);
         role->addEnchantGold(reward.reward_enchantgold, buf);

         role->addPetExp(role->getActivePetId(), quest->pet_exp, buf);

         sendNetPacket(sessionid, &ack);
         
         if (quest->type == qtDailyQuest) {
             if (role->getDailyScheduleSystem()) {
                 role->getDailyScheduleSystem()->onEvent(kDailyScheduleDailyQuest, req.questid, 1);
             }
         }

         //SendDailyQuest(role);
         create_cmd(dailyquest, dailyquest);
         dailyquest->roleid = roleid;
         sendMessageToGame(0, CMD_MSG, dailyquest, 0);
     }
     else
     {
         //服务器还没用完成该任务
         ack.errorcode = CE_QUEST_NOT_FINISH;
         sendNetPacket(sessionid, &ack);
     }
 }}

hander_msg(req_dialog,req)
{
    hand_Sid2Role_check(sessionid,roleid,role);

    UpdateQuestProgress("dialog", req.npdid, req.indexid, role,true);
}}

hander_msg(req_enter_general_copy,req)
{
    hand_Sid2Role_check(sessionid,roleid,role);
    
    ack_enter_general_copy ack;
    ack.errorcode = CE_OK;
    
    //防止客户端重复发多条消息过来
    if (0 < role->getPreEnterSceneid()) {
        return;
    }
    
    SceneCfgDef* scenecfg = SceneCfg::getCfg(req.copyid);
    if (scenecfg == NULL) {
        return;
    }
    
    if (scenecfg->sceneType != stCopy) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    if (scenecfg->townid != role->getSceneID() || scenecfg->preCopy > role->getLastFinishCopy())
    {
        kickAwayRoleWithTips(role, "副本号不对");
        return;
    }
    
    SendAckEnterCopyHelper ackEnterCopy(role,req.copyid);
    
    if (!ackEnterCopy.checkCanEnterCopy(role, req.copyid)) {
        ack.errorcode = 1;
        sendNetPacket(sessionid, &ack);
        return;
    }

    UpdateQuestProgress("entercopy", req.copyid, 1, role, true);
        
    role->setPreEnterSceneid(req.copyid);
    role->setBattleTime(Game::tick);
    ack.copyid = req.copyid;
    
    sendNetPacket(sessionid, &ack);
    
}}

hander_msg(req_enter_scene, req)
{
    hand_Sid2Role_check(sessionid,roleid,role);
    
    ack_enter_scene ack;
    
    if (req.sceneid != role->getPreEnterSceneid()) {
        string desc = Utils::makeStr("req_id:%d, server_sceneid:%d", req.sceneid, role->getPreEnterSceneid());
        kickAwayRoleWithTips(role, desc.c_str());
        return;
    }
      
    ack.sceneid = role->getPreEnterSceneid();
    sendNetPacket(sessionid, &ack);
    
    SceneCfgDef* scene = SceneCfg::getCfg(req.sceneid);
    
    if (scene == NULL) {
        return;
    }
    
    if (scene->sceneType != stTown) {
        role->setBattleTime(Game::tick);
    }
    
    if (scene->sceneType == stChallenge) {
        role->addGuildInspired();
    }
    
    if (scene->sceneType == stTown) {
        
        role->removeBattleBuff();
        
        notify_backToCity notify;
        notify.cityid = req.sceneid;
        sendNetPacket(sessionid, &notify);
    }
    
    role->setCurrSceneId(req.sceneid);
    role->setPreEnterSceneid(0);
}}

void SaveLastCopyId(int roleid, int copyId)
{
    redisReply* rreply;
    rreply=redisCmd("hset role:%d lastFinishCopy %d",roleid, copyId);
    freeReplyObject(rreply);
}


hander_msg(req_finishcopy,req)
{
    hand_Sid2Role_check(sessionid,roleid,role);
    ack_finishcopy ack;
    notify_flopcard notify;
    bool cangetAward = false;
    int friendId = 0;
    ack.rescode = 1;

    int copyId =req.copyid;
    int flopid = 0;

    SceneCfgDef* _sceneCfg = SceneCfg::getCfg(copyId);
    if (_sceneCfg == NULL) {
        ack.rescode = 0;
        sendNetPacket(sessionid,&ack);
        return;
    }
    sendNetPacket(sessionid,&ack);

    //神秘副本不在这里处理
    if (_sceneCfg->sceneType == stMysticalCopy || _sceneCfg->sceneType == stPvp ) {
        return;
    }
    
    if (req.copyid != role->getCurrSceneId())
    {
        kickAwayRoleWithTips(role, "修改副本号");
        return;
    }

    role->backToCity();
    role->setBattleTime(0);
    
    //扣精力值
    if (role->getFat() < _sceneCfg->energy_consumption) {
        SRoleMgr.disconnect(role, eClientCheat);
        return;
    }
    
    if (_sceneCfg->sceneType == stDungeon)
    {
        kickAwayRoleWithTips(role, "");
        return;
    }

    UpdateQuestProgress("copycombo", req.copyid, req.maxCombo, role, true);
    UpdateQuestProgress("copystar", req.copyid, req.star, role, true);
    
    if (req.finishstate == 2)
    {
        do
        {
            flopid = copyId;
            
            if (!canInorgeBattleCheck()) {
                
                if (req.deadMonsterNum != SceneCfg::getMonsterCount(_sceneCfg)) {
                    kickAwayRoleWithTips(role, "怪物数目不对");
                    return;
                }
            }
            
            //玩家战斗力的验证
            int battleTime = 0;
            if (checkPlayerCompatPowerForCheat(role, req.copyid, battleTime)) {
                return;
            }
            
            role->addFat( - _sceneCfg->energy_consumption);
            
            //普通副本
            if (_sceneCfg->sceneType == stCopy) {
                
                UpdateQuestProgress("finishcopy", req.copyid, 1, role, true);
                
                role->SaveLastCopy(req.copyid);
                
                cangetAward = true;
                
                break;
            }

            //精英副本
            if (_sceneCfg->sceneType == stEliteCopy)
            {
                if(role->getEliteTimes() + role->getIncrEliteCopyTimes() <= 0)
                {
                    kickAwayRoleWithTips(role, "");
                    return;
                }
                
                flopid = req.copyid * 10 + req.star;

                int nowtime = time(NULL);
                int lasttime = role->getLastEnterDungeonTime();
                if (isSameDay(nowtime, lasttime))
                {
                    role->addEliteTimes(-1);
                    
                    string describe = Utils::makeStr("passcopy:%d", req.copyid);
                    role->addLogActivityCopyEnterTimesChange(eActivityEnterTimesEliteCopy, describe.c_str());
                }
                
                UpdateQuestProgress("finishelitecopy", req.copyid, req.star, role, true);
                
                cangetAward = true;
                role->SaveLastCopy(req.copyid);
                
                
                if (role->getDailyScheduleSystem()) {
                    role->getDailyScheduleSystem()->onEvent(kDailyScheduleEliteCopy, req.copyid, 1);
                }
                
                LogMod::addLogFinishEliteCopy(role, _sceneCfg->sceneId);
                
                break;
            }
            
            //图纸副本
            if (_sceneCfg->sceneType == stPrintCopy)
            {
                //已经完成过了，还发过来果断踢了
                if (role->getPrintCopyAct()->IsCopyFinish(req.copyid)) {
                    kickAwayRoleWithTips(role, "");
                    return;
                }
                
                role->getPrintCopyAct()->addFinishCopyid(roleid, req.copyid);
                LogMod::addLogFinishDungeException(role, req.copyid, battleTime);
                
                cangetAward = true;
                
                break;
            }
            
            //好友副本
            if (_sceneCfg->sceneType == stTeamCopy) {
                friendId = role->getFightedFriendId();
                // 添加亲密度
                role->getFriendMgr()->procAddIntimacy(role, friendId, req.copyid);
                
                if (req.finishstate == 2) {
                    saveFriendAward(_sceneCfg, role);
                }
                role->clearFriendInfo();
                
                cangetAward = true;
                
                if (role->getDailyScheduleSystem()) {
                    role->getDailyScheduleSystem()->onEvent(kDailyScheduleTeamCopy, req.copyid, 1);
                }
                
                break;
            }

        }while(false);
    }
    else if (req.finishstate == 1)
    {
        LogMod::addLogFinishCopy(role, req.copyid, false, "", _sceneCfg->sceneType);
    }

    int fcindex = 0;
    std::vector<int> dropindices;
    RewardStruct notifyReward;
	string awardItems;
    
    if (cangetAward)
    {
        notify.errorcode = sendNormalCopyAward(role, _sceneCfg, flopid, friendId,
											   fcindex, dropindices, notifyReward, awardItems);
		
		// 称号检查
		SHonorMou.procHonor(eHonorKey_Copy, role, xystr("%d",copyId));
    }
    
    notify.award.flopid= flopid;
    notify.award.findex = fcindex;
    notify.award.dropindices = dropindices;
    
    notify.award.gold = notifyReward.reward_gold;
    notify.award.battlepoint = notifyReward.reward_batpoint;
    notify.award.exp = notifyReward.reward_exp;
    notify.award.constell = notifyReward.reward_consval;
    notify.award.petexp = notifyReward.reward_petexp;
	notify.award.awardItems = awardItems;
    notify.award.enchantdust = notifyReward.reward_enchantdust;
    notify.award.enchantgold = notifyReward.reward_enchantgold;
    
    sendNetPacket(sessionid,&notify);
    
}}

handler_msg(req_refresh_daily_quest, req)
{
    hand_Sid2Role_check(sessionid,roleid,role);
    
    ack_refresh_daily_quest ack;
    ack.errorcode = 1;
    
    do{
        int totaltimes = BuyCfg::getLvlTimes(eDailyQuestRefresh, role->getLvl());
        int usedtimes = role->getDailyQuestRefreshTimes();
        if (usedtimes >= totaltimes){
            break;
        }
        
        int cost, type;
        cost = BuyCfg::getCost(eDailyQuestRefresh, usedtimes, type);
        if (cost < 0){
            break;
        }
        
        if (type == eRmbCost && cost <= role->getRmb()){
            role->addRmb(-cost, "刷新日常任务");
        }
        else if (type == eGoldCost && cost <= role->getGold()){
            role->addGold(-cost, "刷新日常任务");
        }
        else{
            break;
        }
        
        role->addDailyQuestCount(-role->getDailyQuestCount());
        role->addDailyQuestRefreshTimes(1);
        ack.errorcode = 0;
        ack.times = role->getDailyQuestRefreshTimes();
        
        create_cmd(dailyquest, dailyquest);
        dailyquest->roleid = roleid;
        sendCmdMsg(Game::MQ, dailyquest);
    }
    while (0);
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_add_anger, req)
{
    hand_Sid2Role_check(sessionid,roleid,role);
    
    int currAnger = role->getRoleAnger();
    
    if (req.addPoint < 0) {
        //if ( ( req.addPoint + currAnger ) < 0) {
        //    kickAwayRoleWithTips(role, "");
        //    return;
        //}
        //role->setRoleAnger(0);
        return;
    }
    
    
    RoleTpltCfg* roleCfg = RoleTpltCfg::getCfg(role->getRoleType());
    
    if (roleCfg == NULL) {
        return;
    }
    
    int maxAnger = roleCfg->getMaxAnger();
    
    int newAnger = (currAnger + req.addPoint) > maxAnger ? maxAnger : (currAnger + req.addPoint);
    
    role->setRoleAnger(newAnger);
}}

hander_msg(req_begin_petAdventure, req)
{
    hand_Sid2Role_check(sessionid,roleid,self);
    
    ack_begin_petAdventure ack;
    ack.errorcode   = CE_OK;
    ack.oncemore    = req.oncemore;
    bool isRented   = req.isrented;
    int  sceneid    = req.sceneid;
    int  carryPetid = req.petid;

    Pet* carryPet = NULL;

    //防止客户端重复发多条消息过来
    if (0 < self->getPreEnterSceneid()) {
        if (req.oncemore == 0) {
            return;
        } else if (self->getCurrSceneId() != sceneid) {
            return;
        } else if (self->getPreEnterSceneid() == sceneid) {
            return;
        }
    }
    
    if (self->calPetAdventureTimes() <= 0) {
        ack.errorcode = CE_PET_ADVENTURE_TIMES_EXHAUSTED;
        sendNetPacket(sessionid, &ack);
        return;
    }

    PetAdventureCfgDef* def = PetAdventureCfg::getPetAdventureCfgDef(sceneid);
    if (def == NULL) {
        ack.errorcode = CE_PET_ADVENTURE_CONFIG_NOT_FOUND;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    SceneCfgDef* sceneCfg = SceneCfg::getCfg(sceneid);
    if (sceneCfg == NULL || sceneCfg->sceneType != stPetAdventure) {
        ack.errorcode = CE_PET_ADVENTURE_SCENE_NOT_FOUND;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    do
    {
        if (self->getLvl() < sceneCfg->minLv || sceneCfg->maxLv < self->getLvl() ) {
            ack.errorcode = CE_PET_ADVENTURE_PLAYERLVL_OUT_RANGE;
            break;
        }
        
        if (self->getBattleForce() < sceneCfg->fightpoint) {
            ack.errorcode = CE_PET_ADVENTURE_FIGHTPOINT_INSUFFICIENT;
            break;
        }
        
        if (def->pet_adventure_type == ePetAdventure_temporary_activity) {
            int nowtime = time(NULL);
            if (nowtime < def->begin_time || def->end_time < nowtime) {
                ack.errorcode = CE_PET_ADVENTURE_TEMPORARY_ACTIVITY_CLOSE;
                break;
            }
        }
        
        if (isRented) {
            if (self->calPetAdventureRobotPetFreeRentTimesRemain() == 0) {
                
                int hasEnoughMoney = self->hasEnoughMoney(ePetAdventureRobotPetRent, self->getPetAdventureRobotPetBuyingRentTimesRecordPerDay() + 1);
                if (hasEnoughMoney == 0) {
                    ack.errorcode = CE_PET_ADVENTURE_NOT_ENOUGH_GOLD_FOR_RENTPET;
                    break;
                }
            }
            
            if (self->calPetAdventureRobotPetRentTimesRemain() <= 0) {
                ack.errorcode = CE_PET_ADVENTURE_RENTPET_TIMES_EXHAUSTED;
                break;
            }
            
            if (initPetAdventureRobotPetProp(sceneid, carryPetid, self->m_CPetAdventureRentedPet) == false) {
                ack.errorcode = CE_PET_ADVENTURE_CREATE_ROBOTPET_ERROR;
                sendNetPacket(sessionid, &ack);
                return;
            }
            
            carryPet = &(self->m_CPetAdventureRentedPet);
        }
        else {
            carryPet = self->mPetMgr->getPet(carryPetid);
        }

    }while (false);
    
    if (ack.errorcode == CE_OK) {
        
        self->m_nPetAdventureCarryPetId = carryPetid;

        if (carryPet != NULL) {
            
            PetCfgDef* petcfg = PetCfg::getCfg(carryPet->petmod);
            xyassertf(petcfg,"petmod = %d", carryPet->petmod);
            
            int carryPetModId = carryPet->petmod;
            
            vector<int>::iterator result = def->recommended_pet.begin();
            for (; result != def->recommended_pet.end(); result ++) {
                if ( *result == carryPetModId) {
                    break;
                }
            }
            
            if (result != def->recommended_pet.end()) {
                
                //属性未加成前日志字符串
                stringstream szLogbeforeAddPetBuf;
                szLogbeforeAddPetBuf << carryPet->getMaxHp() << " "
                                     << carryPet->getAtk  () << " "
                                     << carryPet->getDef  () << " "
                                     << carryPet->getHit  () << " "
                                     << carryPet->getDodge() << ";";

                //幻兽属性加成
                carryPet->mRealBattleProp.setMaxHp(carryPet->getMaxHp() * def->getHpTimes     () + def->getHp     ());
                carryPet->mRealBattleProp.setAtk  (carryPet->getAtk  () * def->getAttackTimes () + def->getAttack ());
                carryPet->mRealBattleProp.setDef  (carryPet->getDef  () * def->getDefenceTimes() + def->getDefence());
                carryPet->mRealBattleProp.setHit  (carryPet->getHit  () * def->getHitTimes    () + def->getHit    ());
                carryPet->mRealBattleProp.setDodge(carryPet->getDodge() * def->getDodgeTimes  () + def->getDodge  ());
                
                //加成属性以后日志字符串
                stringstream szLogAfterAddPetBuf;
                szLogAfterAddPetBuf << carryPet->mRealBattleProp.getMaxHp() << " "
                                    << carryPet->mRealBattleProp.getAtk  () << " "
                                    << carryPet->mRealBattleProp.getDef  () << " "
                                    << carryPet->mRealBattleProp.getHit  () << " "
                                    << carryPet->mRealBattleProp.getDodge() << ";";
                
                IntArrayObj::const_iterator it = carryPet->getSkill().const_begin();
                
                for (; it != carryPet->getSkill().const_end(); ++it)
                {
                    //技能等级未加成前记日志
                    szLogbeforeAddPetBuf << *it << " ";

                    SkillCfgDef* skillcfg = SkillCfg::getCfg(*it);
                    xycheck( skillcfg && skillcfg->needPages > 0 ); // 页为0表示非宠物技能
                    
                    int quaMaxLvl = 0;
                    if( skillcfg->getPetSkillType() == ePetSkillType_Common)
                    {
                        quaMaxLvl = PetCfg::getMaxCommmonSkills(petcfg->getQua());
                        
                    }else{  // special skill
                        quaMaxLvl = PetCfg::getMaxSpecialSkills(petcfg->getQua());
                    }
                    
                    if (quaMaxLvl > skillcfg->getMaxLvl()) {
                        quaMaxLvl = skillcfg->getMaxLvl();
                    }
                    
                    std::pair<int,int> carryPetStudiedSkill = calSkillCfgIdAndLevel( *it );
                    int studiedSkillCfgId = carryPetStudiedSkill.first;
                    int studiedSkillLevel = carryPetStudiedSkill.second;
    
                    studiedSkillLevel     = studiedSkillLevel * def->getSkillTimes() + def->getSkill();
                    
                    if (studiedSkillLevel > quaMaxLvl) {
                        studiedSkillLevel = quaMaxLvl;
                    }
                    
                    int upgradeskill      = studiedSkillCfgId + studiedSkillLevel;
                    
                    //技能等级加成后记日志
                    szLogAfterAddPetBuf   << upgradeskill << " ";
                    carryPet->upgrade_skill.push_back(upgradeskill);

                }
                
                carryPet->makePropVerifyCode();

                if (isRented) {
                    notify_petAdventurerRobotPet_prop_info robotpet_info;
                    carryPet->getBattleProp(robotpet_info.robotpetinfo);
                    robotpet_info.verifyPropCode = carryPet->verifyPropCode;
                    
                    sendNetPacket(sessionid, &robotpet_info);
                }
                else {
                    notify_pet_prop_info notify;
                    
                    notify.petid          = carryPet->petid;
                    notify.isActive       = carryPet->isactive;
                    notify.Phys           = carryPet->mPhys;
                    notify.Capa           = carryPet->mCapa;
                    notify.Inte           = carryPet->mInte;
                    notify.Stre           = carryPet->mStre;
                    
                    notify.maxHp          = carryPet->mRealBattleProp.getMaxHp();
                    notify.atk            = carryPet->mRealBattleProp.getAtk  ();
                    notify.def            = carryPet->mRealBattleProp.getDef  ();
                    notify.cri            = carryPet->mRealBattleProp.getCri  ();
                    notify.dodge          = carryPet->mRealBattleProp.getDodge();
                    notify.hit            = carryPet->mRealBattleProp.getHit  ();
                    notify.skill          = carryPet->upgrade_skill;
                    
                    notify.verifyPropCode = carryPet->verifyPropCode;
                    
                    sendNetPacket(sessionid, &notify);
                }
                                
                LogMod::addLogPetAdventureBufOnPet(self, sceneid, sceneCfg->sceneType, szLogbeforeAddPetBuf.str(), szLogAfterAddPetBuf.str());
                
            }
        }
        
        int nowtime = time(NULL);
        self->setLastEnterPetAdventureTime(nowtime);
        self->saveNewProperty();
        
        ack.sceneid = sceneid;
        self->setPreEnterSceneid(sceneid);
    }
    
    sendNetPacket(sessionid, &ack);
    
}}

hander_msg(req_finish_petAdventure,req)
{
    hand_Sid2Role_check(sessionid,roleid,self);
    
    ack_finish_petAdventure ack;

    bool cangetAward = false;
    int  sceneid     = req.sceneid;
    
    bool bCarryRecommendedPet          = false;
    bool isRented                      = false;
    bool bFreeChargeRobotPetRentedBill = true;

    int  flopid = 0;
    
    if (sceneid != self->getCurrSceneId())
    {
        string tipInfo = strFormat("%s:%d", __FILE__, __LINE__);
        LogMod::addLogClientCheat(self->getInstID(), self->getRolename().c_str(), self->getSessionId(), tipInfo.c_str(), "修改副本号");
        ack.errorcode = CE_PET_ADVENTURE_CONFIG_NOT_FOUND;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    SceneCfgDef* sceneCfg = SceneCfg::getCfg(sceneid);
    if (sceneCfg == NULL || sceneCfg->sceneType != stPetAdventure) {
        ack.errorcode = CE_PET_ADVENTURE_SCENE_NOT_FOUND;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    PetAdventureCfgDef* def = PetAdventureCfg::getPetAdventureCfgDef(sceneid);
    if (def == NULL) {
        ack.errorcode = CE_PET_ADVENTURE_CONFIG_NOT_FOUND;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (self->calPetAdventureTimes() <= 0) {
        ack.errorcode = CE_PET_ADVENTURE_TIMES_EXHAUSTED;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    //扣精力值
    if (self->getFat() < sceneCfg->energy_consumption) {
        SRoleMgr.disconnect(self, eClientCheat);
        return;
    }
        
    if (self->m_nPetAdventureCarryPetId != req.petid) {
        ack.errorcode = CE_PET_ADVENTURE_WRONG_CARRYPETID;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    Pet* carryPet = NULL;
    self->m_nPetAdventureCarryPetId = 0;
    if (req.isrented) {
        if (self->calPetAdventureRobotPetFreeRentTimesRemain() <= 0) {
            
            int hasEnoughMoney = self->hasEnoughMoney(ePetAdventureRobotPetRent, self->getPetAdventureRobotPetBuyingRentTimesRecordPerDay() + 1);
            if (hasEnoughMoney == 0) {
                ack.errorcode = CE_PET_ADVENTURE_NOT_ENOUGH_GOLD_FOR_RENTPET;
                sendNetPacket(sessionid, &ack);
                return;
            }
            
            bFreeChargeRobotPetRentedBill = false;
        }
        
        if (self->calPetAdventureRobotPetRentTimesRemain() <= 0) {
            ack.errorcode = CE_PET_ADVENTURE_RENTPET_TIMES_EXHAUSTED;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        isRented = true;
        bCarryRecommendedPet = true;

    }
    else {
        carryPet = self->mPetMgr->getPet(req.petid);
    }
    
    if (carryPet != NULL) {
        int carryPetModId = carryPet->petmod;
        
        vector<int>::iterator result = def->recommended_pet.begin();
        for (; result != def->recommended_pet.end(); result ++) {
            if ( *result == carryPetModId) {
                bCarryRecommendedPet = true;
                break;
            }
        }
        if (result != def->recommended_pet.end()) {
            
            //属性消除前日志字符串
            stringstream szLogBeforeRemovePetBuf;
            szLogBeforeRemovePetBuf << carryPet->mRealBattleProp.getMaxHp() << " "
                                    << carryPet->mRealBattleProp.getAtk  () << " "
                                    << carryPet->mRealBattleProp.getDef  () << " "
                                    << carryPet->mRealBattleProp.getHit  () << " "
                                    << carryPet->mRealBattleProp.getDodge() << ";";
            
            //消除幻兽buf加成
            carryPet->mRealBattleProp.setMaxHp(carryPet->getMaxHp());
            carryPet->mRealBattleProp.setAtk  (carryPet->getAtk  ());
            carryPet->mRealBattleProp.setDef  (carryPet->getDef  ());
            carryPet->mRealBattleProp.setHit  (carryPet->getHit  ());
            carryPet->mRealBattleProp.setDodge(carryPet->getDodge());
            
            //属性消除后日志字符串
            stringstream szLogAfterRemovePetBuf;
            szLogAfterRemovePetBuf << carryPet->getMaxHp() << " "
                                   << carryPet->getAtk  () << " "
                                   << carryPet->getDef  () << " "
                                   << carryPet->getHit  () << " "
                                   << carryPet->getDodge() << ";";
            
            carryPet->makePropVerifyCode();
            
            notify_pet_prop_info notify;
            
            notify.petid    = carryPet->petid;
            notify.isActive = carryPet->isactive;
            notify.Phys     = carryPet->mPhys;
            notify.Capa     = carryPet->mCapa;
            notify.Inte     = carryPet->mInte;
            notify.Stre     = carryPet->mStre;
            
            notify.maxHp    = carryPet->mRealBattleProp.getMaxHp();
            notify.atk      = carryPet->mRealBattleProp.getAtk();
            notify.def      = carryPet->mRealBattleProp.getDef();
            notify.cri      = carryPet->mRealBattleProp.getCri();
            notify.dodge    = carryPet->mRealBattleProp.getDodge();
            notify.hit      = carryPet->mRealBattleProp.getHit();
            
            for (int i = 0; i < carryPet->upgrade_skill.size(); ++i)
            {
                szLogBeforeRemovePetBuf << carryPet->upgrade_skill[i] << " ";
            }
            
            IntArrayObj::const_iterator it = carryPet->getSkill().const_begin();
            for (; it != carryPet->getSkill().const_end(); ++it)
            {
                notify.skill.push_back(*it);
                szLogAfterRemovePetBuf << *it << " ";
            }
            
            notify.verifyPropCode = carryPet->verifyPropCode;
            
            sendNetPacket(sessionid, &notify);
            
            LogMod::addLogPetAdventureBufOnPet(self, req.sceneid, sceneCfg->sceneType, szLogBeforeRemovePetBuf.str(), szLogAfterRemovePetBuf.str());
            
            //消除
            carryPet->upgrade_skill.clear();
            
        }
    }

    if (req.finishstate == 1)
    {
        if (req.deadMonsterNum != SceneCfg::getMonsterCount(sceneCfg)) {
            
            string tipInfo = strFormat("%s:%d", __FILE__, __LINE__);
            LogMod::addLogClientCheat(roleid, self->getRolename().c_str(), self->getSessionId(), tipInfo.c_str(), "怪物数目不对");
            ack.errorcode = CE_PET_ADVENTURE_WRONG_DEADED_MONSTER_NUM;
            sendNetPacket(sessionid, &ack);
            return;
        }
        
        //玩家战斗力的验证
        int battleTime = 0;
        if (checkPlayerCompatPowerForCheat(self, sceneid, battleTime)) {
            return;
        }
        
        flopid = sceneid * 10 + req.star;

        int nowtime  = time(NULL);
        int lasttime = self->getLastEnterPetAdventureTime();
        if (isSameDay(nowtime, lasttime))
        {
            if (self->decrPetAdventureTimes(1) == false) {
                ack.errorcode = CE_PET_ADVENTURE_TIMES_EXHAUSTED;
                sendNetPacket(sessionid, &ack);
                return;
            }
            
            string action = Utils::makeStr("passcopy_%d", sceneid);
            self->addLogActivityCopyEnterTimesChange(eActivityEnterTimesPetAdventure, action.c_str());
        }
        
        self->addFat( - sceneCfg->energy_consumption);
        
        if (isRented) {
            if (self->payPetAdventureRobotPetRentedBill(bFreeChargeRobotPetRentedBill) != CE_OK) {
                ack.errorcode = CE_PET_ADVENTURE_NOT_ENOUGH_GOLD_FOR_RENTPET;
                sendNetPacket(sessionid, &ack);
                return;
            }
        }

        if (self->getDailyScheduleSystem()) {
            self->getDailyScheduleSystem()->onEvent(kDailySchedulePetAdventure, 0, 1);
        }
        
        UpdateQuestProgress("finishpetadventure", sceneid, 1, self, true);
        
        cangetAward = true;
        
        ack.errorcode = CE_OK;
        sendNetPacket(sessionid, &ack);
                
    }
    else if (req.finishstate == 0)
    {

        LogMod::addLogFinishCopy(self, sceneid, false, "", sceneCfg->sceneType);
        LogMod::addLogFinishPetAdventureCopy(self, sceneid, false, "", sceneCfg->sceneType);

    }
    
    self->backToCity();

    int fcindex = 0;
    std::vector<int> dropindices;
    RewardStruct notifyReward;
    notify_flopcard notify;
    string itemsResult = "";
    if (cangetAward)
    {
        //这个发奖励内含日志
        notify.errorcode = sendPetAdventureAward(self, req.petid, sceneCfg, flopid, fcindex, dropindices, notifyReward, bCarryRecommendedPet, itemsResult);
    }
    
    notify.award.flopid      = flopid;
    notify.award.findex      = fcindex;
    notify.award.dropindices = dropindices;
    
    notify.award.gold        = notifyReward.reward_gold;
    notify.award.battlepoint = notifyReward.reward_batpoint;
    notify.award.exp         = notifyReward.reward_exp;
    notify.award.constell    = notifyReward.reward_consval;
    notify.award.petexp      = notifyReward.reward_petexp;
    notify.award.awardItems  = itemsResult;
    notify.award.enchantdust = notifyReward.reward_enchantdust;
    notify.award.enchantgold = notifyReward.reward_enchantgold;
    
    sendNetPacket(sessionid, &notify);
    
}}

hander_msg(req_petadventure_temporaryactcopy_time, req)
{
    hand_Sid2Role_check(sessionid,roleid,self);
    
    ack_petadventure_temporaryactcopy_time ack;
    
    time_t nowtime = time(NULL);

    ack.nowTime = nowtime;
    
    sendNetPacket(sessionid, &ack);
}}

/*
//翻牌
hander_msg(req_treasurebox,req)
{

}}
*/
