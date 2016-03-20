//
//  award_hand.cpp
//  GameSrv
//
//  Created by cxy on 13-1-10.
//
//
#include "hander_include.h"
#include "Global.h"
#include "GlobalMsg_def.h"
#include "Activity.h"
#include "MultiPlayerRoom.h"
#include "flopcard.h"
#include "ActivityMod.h"
#include "TowerDefense.h"
#include "MysticalCopy.h"
#include "FamesHall.h"
#include "TimeLimitKillCopy.h"
#include "PetCasinoActivity.h"
#include "GuildTrainActivity.h"
#include "GuildMgr.h"
#include "TreasureFight.h"
#include "Robot.h"
#include "daily_schedule/daily_schedule_system.h"
#include "Honor.h"
#include "Retinue.h"

extern void getRoleObjDungeonState(Role* role, vector<obj_dungeonstate>& states);

hander_msg(req_get_act_stats,req)
{
    create_global(reqActivityStats, act);
    act->sessionid = sessionid;
    sendGlobalMsg(Global::MQ, act);
}}
// 称号检查
void checkCopyHonor(Role *role, int copyId)
{
	SHonorMou.procHonor(eHonorKey_Copy, role, xystr("%d", copyId));
}

/**********************************  normal dunge  *****************************/
hander_msg(req_begin_normal_dunge, req)
{
    
    hand_Sid2Role_check(sessionid,roleid,self);
    
    if (0 < self->getPreEnterSceneid()) {
        return;
    }
    
    if (!checkCurrSceneType(req.sceneid, stDungeon)) {
        kickAwayRoleWithTips(self, "");
        return;
    }
    
    ack_begin_normal_dunge ack;
    ack.sceneid = req.sceneid;
    
    int index = (req.sceneid / 1000) % 100;
    int level = req.sceneid % 100;
    int record = self->getDungeLevelRecord(index);
    
    if (!checkCanEnterCopy(self, req.sceneid)) {
        return;
    }
    
    //check record
    if (record == 0) {
        if (level != 1) {

            kickAwayRoleWithTips(self, "地下城层数不对");
            return;
        }
    }
    else if (req.sceneid != record || record < 0) {

        kickAwayRoleWithTips(self, "副本号不对");
        
        return;
    }
    
    self->setPreEnterSceneid(req.sceneid);
    sendNetPacket(sessionid, &ack);
}}


handler_msg(req_finish_dungecopy, req)
{
    hand_Sid2Role_check(sessionid,roleid,self);

    ack_finish_dungecopy ack;
    ack.errorcode = CE_OK;
    ack.result = req.result;
    
    SceneCfgDef* _sceneCfg = SceneCfg::getCfg(self->getCurrSceneId());
    if (_sceneCfg == NULL || _sceneCfg->sceneType != stDungeon) {
        kickAwayRoleWithTips(self, "");
        return;
    }
    
    
    
    int index = (_sceneCfg->sceneId / 1000) % 100;
    int level = (_sceneCfg->sceneId % 100);
    int record = self->getDungeLevelRecord(index);
    int maxSweeplvl = self->getDungeMaxSweepingLevel(index);
    
    
    DungeonCfgDef* dungeCfg =  DungeonCfg::getDungeonCfg(stDungeon, index);
    if (dungeCfg == NULL) {
        ack.errorcode = CE_CAN_NOT_FIND_CFG;
        self->send(&ack);
        return;
    }

    if ((req.sceneid - self->getCurrSceneId()) > 1 ||
         record < 0) {
        kickAwayRoleWithTips(self, "");
        return;
    }
    
    bool isFinishCurFloor = false;
    do
    {
        if (req.result == eDungGetout)
        {
            //check record
            for (int i = 0; i < self->mDungeAward.size(); i++) {
                obj_dungeawawrd awards;
                awards.secneid = self->mDungeAward[i].sceneid;
                awards.flopindex = self->mDungeAward[i].flopIndex;
                awards.drops = self->mDungeAward[i].drops;
                
                awards.gold = self->mDungeAward[i].gold;
                awards.exp = self->mDungeAward[i].exp;
                awards.battlepoint = self->mDungeAward[i].battlepoint;
                awards.constell = self->mDungeAward[i].constell;
                awards.petexp = self->mDungeAward[i].petexp;
				awards.itemsAward = self->mDungeAward[i].awardItems;
                
                ack.awards.push_back(awards);
            }
            ack.result = eDungGetout;
            ack.sceneid = _sceneCfg->sceneId;
            
            sendNetPacket(sessionid, &ack);
            self->mDungeAward.clear();
    
            //下一步是回城镇
            self->backToCity();
            
            break;
        }
        
        if ( req.sceneid != self->getCurrSceneId()) {
            kickAwayRoleWithTips(self, "");
            return;
        }
        
        if (req.result == eDungeNext) {
            if (req.sceneid != self->getCurrSceneId()) {
                kickAwayRoleWithTips(self, "");
                return;
            }
            
            if (record && req.sceneid != record) {
                kickAwayRoleWithTips(self, "");
                return;
            }
            
            if (req.deadMonsterNum != SceneCfg::getMonsterCount(_sceneCfg)) {

                kickAwayRoleWithTips(self, "怪物数目不对");
                return;
            }
            int battleTime = 0;
            if (checkPlayerCompatPowerForCheat(self, _sceneCfg->sceneId, battleTime)) {
                return;
            }
            
            // 称号检查
			checkCopyHonor(self, _sceneCfg->sceneId);
            
//            LogMod::addLogFinishDungeException(self, self->getCurrSceneId(), battleTime);
            LogMod::addLogFinishDunge(self, index, level);
            
            handlerDungeResultAward(_sceneCfg->sceneId, req.result, self);
            
            int index = (req.sceneid / 1000) % 100;
            self->saveDungLevelRecord(_sceneCfg->sceneId + 1, index);
            //判断挑战层数是否超过历史最大扫荡层数
            if (record > maxSweeplvl) {
                self->setDungMaxSweepingLevel(record, index);
            }
            
            
            self->addFat(-_sceneCfg->energy_consumption);
//            self->onCalPlayerProp();
            self->sendRolePropInfoAndVerifyCode(true);
            self->onSendPetPropAndVerifycode(self->mPetMgr->getActivePet(), true);
            
            self->setPreEnterSceneid(_sceneCfg->sceneId + 1);
//            self->setCurrSceneId(_sceneCfg->sceneId + 1);
            
            ack.sceneid = _sceneCfg->sceneId;
            
            sendNetPacket(self->getSessionId(), &ack);
            
            isFinishCurFloor = true;
            break;
        }
        
        if (req.result == eDungeFail)
        {
            for (int i = 0; i < self->mDungeAward.size(); i++) {
                obj_dungeawawrd awards;
                awards.secneid = self->mDungeAward[i].sceneid;
                awards.flopindex = self->mDungeAward[i].flopIndex;
                awards.drops = self->mDungeAward[i].drops;
                
                awards.gold = self->mDungeAward[i].gold;
                awards.exp = self->mDungeAward[i].exp;
                awards.battlepoint = self->mDungeAward[i].battlepoint;
                awards.constell = self->mDungeAward[i].constell;
                awards.petexp = self->mDungeAward[i].petexp;
				awards.itemsAward = self->mDungeAward[i].awardItems;

                ack.awards.push_back(awards);
            }
            ack.result = eDungeFail;
            ack.sceneid = _sceneCfg->sceneId;
            self->backToCity();
            
            sendNetPacket(sessionid, &ack);
            self->mDungeAward.clear();
            break;
        }
        
        if (req.result == eDungeFinish)
        {
            if (req.sceneid != self->getCurrSceneId()) {
                kickAwayRoleWithTips(self, "");
                return;
            }
            
            if (_sceneCfg->hasPortal) {
                kickAwayRoleWithTips(self, "");
                return;
            }
            
            if (req.sceneid != record) {
                kickAwayRoleWithTips(self, "");
                return;
            }
            
            if (req.deadMonsterNum != SceneCfg::getMonsterCount(_sceneCfg)) {
                
                if (!canInorgeBattleCheck()) {

                    kickAwayRoleWithTips(self, "怪物数目不对");
                    return;
                }
            }
            
            int battleTime = 0;
            if (checkPlayerCompatPowerForCheat(self, _sceneCfg->sceneId, battleTime)) {
                return;
            }
            
//            LogMod::addLogFinishDungeException(self, self->getCurrSceneId(), battleTime);
            LogMod::addLogFinishDunge(self, index, level);
            
            handlerDungeResultAward(req.sceneid, req.result, self);
            
            for (int i = 0; i < self->mDungeAward.size(); i++) {
                obj_dungeawawrd awards;
                awards.secneid = self->mDungeAward[i].sceneid;
                awards.flopindex = self->mDungeAward[i].flopIndex;
                awards.drops = self->mDungeAward[i].drops;
                
                awards.gold = self->mDungeAward[i].gold;
                awards.exp = self->mDungeAward[i].exp;
                awards.battlepoint = self->mDungeAward[i].battlepoint;
                awards.constell = self->mDungeAward[i].constell;
                awards.petexp = self->mDungeAward[i].petexp;
				awards.itemsAward = self->mDungeAward[i].awardItems;
                
                ack.awards.push_back(awards);
            }
            ack.result = eDungeFinish;
            ack.sceneid = _sceneCfg->sceneId;
            
            int index = (req.sceneid / 1000) % 100;
            self->saveDungLevelRecord(-1, index);
            sendNetPacket(sessionid, &ack);
            self->mDungeAward.clear();
            
//          设置最大可扫荡层数为配置中最大值  关闭本日扫荡开关
            int dungMaxLevel = dungeCfg->firstmap + dungeCfg->floor - 1;
            self->setDungMaxSweepingLevel(dungMaxLevel, index);
            
            //下一步是回城镇
            self->backToCity();
            
            broadcastDungeon(self->getRolename(), _sceneCfg->sceneId, _sceneCfg->sceneId % 100);
			
			// 称号检查
            checkCopyHonor(self, _sceneCfg->sceneId);
            isFinishCurFloor = true;
            break;
        }
    }while(false);
    
    if (isFinishCurFloor && self->getDailyScheduleSystem()) {
        self->getDailyScheduleSystem()->onEvent(kDailyScheduleDungeon, req.sceneid, 1);
    }
}}

hander_msg(req_get_dungeon_state, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_get_dungeon_state ack;
    getRoleObjDungeonState(role, ack.dungeonstates);
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_dungcopy_sweeping, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    
    ack_dungcopy_sweeping ack;
    do
    {
        ack.errorcode = CE_OK;
        
        ack.index = req.dungcopytype;
        
        int maxSweepingLvl          = self->getDungeMaxSweepingLevel(req.dungcopytype);
        const int dungeLevelRecord  = self->getDungeLevelRecord(req.dungcopytype);
        
        //最高通关层数为0 或者 今天手动通关层数大于等于 历史最高通关层数 则返回
        if ( (maxSweepingLvl == 0) || ( dungeLevelRecord > maxSweepingLvl) ) {
            ack.errorcode = CE_DUNGEON_SWEEPING_MAXSWEEPLVL_PASSED;
            break;
        }
        
        //int beginsceneid = self->getDungeLevelRecord(req.dungcopytype);
        if (dungeLevelRecord == -1) {
            string desc = strFormat("已经通关还继续扫荡地下城 %d", req.dungcopytype);

            kickAwayRoleWithTips(self, desc.c_str());
            break;
        }
        
        //玩家战斗力与每一层地下城的推荐战斗力的比较
        int roleBattleForce = self->getBattleForce();
        
        int maxlevel = maxSweepingLvl % 100;
        int i = maxSweepingLvl;
        int zerolevel = maxSweepingLvl - maxlevel;
        
        for (; i > zerolevel; i--) {
            //获取对应层ID的推荐战斗力
            SceneCfgDef* sceneCfg = SceneCfg::getCfg(i);
            if (sceneCfg == NULL || sceneCfg->sceneType != stDungeon) {
                continue;
            }
            int dungFightPoint = sceneCfg->fightpoint;
            
            if (roleBattleForce >= dungFightPoint) {
                maxSweepingLvl = i;
                break;
            }
        }
        
        //战斗力小于每一层的推荐战斗力 则返回
        if (i <= zerolevel) {
            ack.errorcode = CE_DUNGEON_SWEEPING_PLAYER_FIGHTPOINT_INSUFFICIENT;
            break;
        }
        
        //根据战斗力算出的最大可扫荡层数 小于等于 今天手动通关层数 则返回
        if ( i < dungeLevelRecord) {
            ack.errorcode = CE_DUNGEON_SWEEPING_MAXSWEEPLVL_PASSED;
            break;
        }
        
        if (ack.errorcode == CE_OK) {
            
            DungeonCfgDef* dungeCfg = DungeonCfg::getDungeonCfg(stDungeon, req.dungcopytype);
            if (dungeCfg == NULL) {
                ack.errorcode = CE_CAN_NOT_FIND_CFG;
                break;
            }
            
            int beginsceneid = dungeLevelRecord;
            if (beginsceneid == 0) {
                beginsceneid = dungeCfg->firstmap;
            }
            int endsceneId = maxSweepingLvl;
            
            ack.errorcode = dungeWipeOut(req.dungcopytype,  beginsceneid, endsceneId, self, ack.awards);
            
            if (ack.errorcode != CE_OK) {
                LogMod::addLogDungeonSweeping(self, beginsceneid, endsceneId, "", false);
                break;
            }
            ack.sweepinglvl = maxSweepingLvl;
            
            if (self->getDailyScheduleSystem()) {
                int floorNum = endsceneId - beginsceneid;
                self->getDailyScheduleSystem()->onEvent(kDailyScheduleDungeon, 0, floorNum);
            }
            
            //扫荡后手动打层数从扫荡后的下一层开始
            self->saveDungLevelRecord( (((maxSweepingLvl % 100) < dungeCfg->floor)? maxSweepingLvl+1 : -1), req.dungcopytype);
            
        }
        
    }while(false);
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_get_printcopy_data, req)
{
    hand_Sid2Role_check(sessionid, roleid, self)
    ack_get_printcopy_data ack;
    ack.finishedCopys.clear();

    if (!SActMgr.IsActive(self->getPrintCopyAct()->getActid())) {
        ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    self->getPrintCopyAct()->checkFresh();
    
    const set<int>& copylist = self->getPrintCopyAct()->getFinishCopy();
    for (set<int>::iterator it = copylist.begin(); it != copylist.end(); it++) {
        ack.finishedCopys.push_back(*it);
    }

    const std::map<int, int>& resetlist = self->getPrintCopyAct()->getResetTimesList();
    for (std::map<int, int>::const_iterator iter = resetlist.begin(); iter != resetlist.end(); iter++) {
        obj_copy_resettimes info;
        info.copyid = iter->first;
        info.times = iter->second;
        ack.resettimes.push_back(info);
    }
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_begin_printcopy, req)
{
    hand_Sid2Role_check(sessionid, roleid, self)
    ack_begin_printcopy ack;
    ack.errorcode = CE_OK;
    do
    {

        if (!SActMgr.IsActive(self->getPrintCopyAct()->getActid())) {
            ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
            break;
        }
        
        SceneCfgDef* scene = SceneCfg::getCfg(req.copyid);
        
        if (scene == NULL) {
            return;
        }
        
        if (scene->sceneType != stPrintCopy) {
            kickAwayRoleWithTips(self, "");
            return;
        }
        
        if (0 < self->getPreEnterSceneid()) {
            return;
        }
        
        if (!checkCanEnterCopy(self, req.copyid)) {
            return;
        }

        if(self->getPrintCopyAct()->IsCopyFinish(req.copyid))
        {
            ack.errorcode = CE_TEAMCOPY_YOU_HAD_FINISH_THISCOPY;
            break;
        }

        ack.copyid = req.copyid;
        self->setPreEnterSceneid(req.copyid);
        
        if (self->getDailyScheduleSystem()) {
            self->getDailyScheduleSystem()->onEvent(kDailySchedulePrintCopy, 0, 1);
        }

    }while(false);

    sendNetPacket(sessionid, &ack);
}}

/*********************************  pet camp  **************************************/
handler_msg(req_get_petcamp_data, req)
{
    return;
    
    hand_Sid2Role_check(sessionid, roleid, self)
    ack_get_petcamp_data ack;
    ack.finishedCopys.clear();
    
    const set<int>& copylist = self->getPetCampAct()->getFinishCamp();
    for (set<int>::iterator it = copylist.begin(); it != copylist.end(); it++) {
        ack.finishedCopys.push_back(*it);
    }
    
    const std::map<int, int>& resetlist = self->getPetCampAct()->getResetTimesList();
    for (std::map<int, int>::const_iterator iter = resetlist.begin(); iter != resetlist.end(); iter++) {
        obj_copy_resettimes info;
        info.copyid = iter->first;
        info.times = iter->second;
        ack.resettimes.push_back(info);
    }
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_begin_petcamp, req)
{
    return;
    hand_Sid2Role_check(sessionid, roleid, self)
    ack_begin_petcamp ack;
    ack.errorcode = CE_OK;
    do
    {
        
        if (!SActMgr.IsActive(self->getPetCampAct()->getActid())) {
            ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
            break;
        }
        
        if (!checkCurrSceneType(req.copyid, stPetCamp)) {
            kickAwayRoleWithTip(self);
            return;
        }
        
        if(self->getPetCampAct()->IsCampFinish(req.copyid))
        {
            ack.errorcode = CE_TEAMCOPY_YOU_HAD_FINISH_THISCOPY;
            break;
        }
        
        ack.copyid = req.copyid;
        
        UpdateQuestProgress("enterpetcamp", req.copyid, 1, self, true);
        
    }while(false);
    
    sendNetPacket(sessionid, &ack);
}}

/***************************************   friend dunge  ***********************************/

handler_msg(req_getfrienddunge_data, req)
{
    hand_Sid2Role_check(sessionid, roleid, self)
    ack_getfrienddunge_data ack;
    
    if (!SActMgr.IsActive(ae_friend_dunge)) {
        ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    self->getFriendDungeAct()->checkFresh();
    ack.frightedfriends.clear();
    
    const set<int>& friends = self->mFriendDungeAct->getFightedFriendList();
    for (set<int>::iterator it = friends.begin(); it != friends.end(); it++) {
        ack.frightedfriends.push_back(*it);
    }
    
    RoleCfgDef rolecfg = RoleCfg::getCfg(self->getJob(), self->getLvl());
    ack.leftTimes = rolecfg.getFriendDungeTimes() - self->getFriendDungeAct()->getAttendTimes() + self->getIncrFriendDungeonTimes();
    
    self->getFriendDungeAct()->getFriendDungeRecord(ack.record);
    ack.friend_lasttime = 0;  //self->getFriendDungeFriend();
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_begin_friend_dunge, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    ack_begin_friend_dunge ack;
    ack.errorcode = CE_OK;
    
    int friendid = req.friendid;
    
    do
    {
        if (0 < self->getPreEnterSceneid()) {
            return;
        }
        
        if (!checkCurrSceneType(req.sceneid, stFriendDunge)) {
            kickAwayRoleWithTips(self, "");
            return;
        }
        
        if (!checkCanEnterCopy(self, req.sceneid)) {
            return;
        }
        
        int index = (req.sceneid / 1000) % 100;
        int record = self->getFriendDungeAct()->getFriendDungeRecord(index);
        
        //如果不是从第一城开始打，踢掉
        if (record == 0) {
            if ( (req.sceneid%10) > 1) {
                kickAwayRoleWithTips(self, "");
                return;
            }
        }else
        {            
            if(req.sceneid != record || record < 0)
            {
                kickAwayRoleWithTips(self, "");
                return;
            }
            
            //已打过，用服务器保存的id
            friendid = self->getFriendDungeAct()->getFriendDungeFriendRecord(index);
        }
        
        int actid = self->mFriendDungeAct->getactid();
        if ( !SActMgr.IsActive(actid)) {
            ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
            break;
        }
        
        SceneCfgDef* scenecfg = SceneCfg::getCfg(req.sceneid);
        
        if (scenecfg && scenecfg->minLv > self->getLvl()) {
            kickAwayRoleWithTips(self, "");
            return;
        }
        
        //判断是否是好友或公会成员
        bool isGuildMember = false;
        Guild& guild = SGuildMgr.getRoleGuild(roleid);
        if( ! guild.isNull() && guild.getMembers().exist(friendid) )
        {
            isGuildMember = true;
        }
        
        if ( ! isGuildMember && !self->mFriendMgr->isFriend(friendid)) {
            //把好友id设成0，兼容继续挑战的问题
            friendid = 0;
        }
        
        //如果是重新打，就验证friendid by wangzhigang 2014/5/12
        if (record == 0) {
            
            RoleCfgDef rolecfg = RoleCfg::getCfg(self->getJob(), self->getLvl());
        
            if (self->getIncrFriendDungeonTimes() <= 0 &&  self->getFriendDungeAct()->getAttendTimes() >= rolecfg.getFriendDungeTimes()) {
                return;
            }
            
            //不是好友或公会成员
            if (friendid == 0) {
                ack.errorcode = CE_GET_FRIEND_DATA_ERROR;
                break;
            }
            
            if (self->mFriendDungeAct->IsFriendFighted(friendid)) {
                ack.errorcode = CE_TEAMCOPY_YOU_HAD_EMPLOYED_THIS_FRIEND;
                break;
            }
            
            int energyConsum = DungeonCfg::getEnergyConsumption(index);
            
            if (energyConsum < 0) {
                ack.errorcode = CE_READ_SCENE_CFG_ERROR;
                break;
            }
            
            if (energyConsum > self->getFat())
            {
                ack.errorcode = CE_FAT_NOT_ENOUGH;
                break;
            }
            
            //先扣精力
            self->addFat(-energyConsum);
            
            // 添加亲密度
            self->getFriendMgr()->procAddIntimacy(self, friendid, req.sceneid);
            
            self->getFriendDungeAct()->saveFriendDungeFriendRecord(index, friendid);
            
            self->getFriendDungeAct()->saveFriendDungeRecord(index, req.sceneid);
            
            self->saveNewProperty();
            
            //好友已使用，保存下来
            self->mFriendDungeAct->addFightedFriend(roleid, friendid);
            
            //扣次数
            if (self->getIncrFriendDungeonTimes() > 0)
            {
                self->setIncrFriendDungeonTimes(self->getIncrFriendDungeonTimes() - 1);
                self->saveNewProperty();
            }
            else
            {
                self->mFriendDungeAct->addAttendTimes(roleid, 1);
            }
            
            string addAction = Utils::makeStr("entercopy_%d", req.sceneid);
            self->addLogActivityCopyEnterTimesChange(eActivityEnterTimesFriendDunge, addAction.c_str());
        }
        
        
        
        int objectType = getObjectType(friendid);
        if (objectType == kObjectRobot) {
            Robot* robot = g_RobotMgr.getData(friendid);
            if (robot == NULL) {
                ack.errorcode = CE_GET_FRIEND_DATA_ERROR;
                break;
            }
            
            obj_roleBattleProp info;
            
            info.atk = robot->mBattleProp.getAtk();
            info.def = robot->mBattleProp.getDef();
            info.cri = robot->mBattleProp.getCri();
            info.dodge = robot->mBattleProp.getDodge();
            info.hit = robot->mBattleProp.getHit();
            info.maxhp = robot->mBattleProp.getMaxHp();
            
            info.roleid = friendid;
            info.skills = robot->mSkills;
            info.weaponfashion = 0;
            info.awakeLvl = 0;
            
            ack.friendinfo = info;
            ack.errorcode = CE_OK;
            
            self->setFightedFriendInfo(robot);
        } else {
        
            Role* employee = Role::create(friendid, true);

            if (employee) {
                
                employee->onCalPlayerProp();
                ack.copyid = req.sceneid;
                employee->getPlayerBattleProp(ack.friendinfo);
                ack.errorcode = CE_OK;
                
                //Pet pet(employee->getActivePetId());
                
                Pet* pet = employee->mPetMgr->getActivePet();
                int activeid = 0;
                int petBattle = 0;
                obj_petBattleProp info;
                if (pet && pet->petid > 0)
                {
                    activeid = pet->petid;
                    petBattle = pet->mBattle;
                    pet->getBattleProp(info);
                }
                else
                {
                    info.petid = 0;
                }
                
                ack.petinfo = info;
                self->setFightedFriendInfo(employee, petBattle, activeid);
                
                Retinue *retinue = employee -> mRetinueMgr -> getActiveRetinue();
                obj_retinueBattleProp retinueBPInfo;
                if(retinue != NULL && retinue -> getId() > 0){
                    retinue -> getBattleProp(retinueBPInfo);
                    retinueBPInfo.skills = employee -> getRetinueMgr() -> mSkillMgr -> getEquipedSkills();
                }
                ack.retinueinfo = retinueBPInfo;
                
                delete employee;
                
            } else if (record == 0) {
                ack.errorcode = CE_GET_FRIEND_DATA_ERROR;
            }
        }
        
    }while (false);
    
    if (ack.errorcode == CE_OK)
    {
        //将要进入的场景id
        self->setPreEnterSceneid(req.sceneid);
        ack.copyid = req.sceneid;
        
        if (self->getDailyScheduleSystem()) {
            self->getDailyScheduleSystem()->onEvent(kDailyScheduleFriendDungeon, 0, 1);
        }
    }
    
    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_finish_friend_dunge, req)
{
    hand_Sid2Role_check(sessionid,roleid,self);
    int currSceneid = self->getCurrSceneId();
    ack_finish_friend_dunge ack;
    ack.errorcode = CE_OK;
    ack.result = req.result;
    ack.sceneid = req.sceneid;
    
    if ( (req.sceneid - self->getCurrSceneId()) > 1) {
        kickAwayRoleWithTips(self, "");
        return;
    }
    
    SceneCfgDef* _sceneCfg = SceneCfg::getCfg(currSceneid);
    if (_sceneCfg == NULL || _sceneCfg->sceneType != stFriendDunge) {
        kickAwayRoleWithTips(self, "");
        return;
    }
    
    //如果好友信息清除了，还有这个消息，证明作弊了
    // 2.1存在没带好友的情况，所以此验证去掉，by wangzhigang 2014、6、5
//    if (self->getFightedFriendInfo() == NULL) {
//        kickAwayRoleWithTips(self, "");
//        return;
//    }
    
    do
    {
        if (req.result == eDungGetout)
        {
            //check record
            for (int i = 0; i < self->mDungeAward.size(); i++) {
                obj_dungeawawrd awards;
                awards.secneid = self->mDungeAward[i].sceneid;
                awards.flopindex = self->mDungeAward[i].flopIndex;
                awards.drops = self->mDungeAward[i].drops;
                
                awards.gold = self->mDungeAward[i].gold;
                awards.exp = self->mDungeAward[i].exp;
                awards.battlepoint = self->mDungeAward[i].battlepoint;
                awards.constell = self->mDungeAward[i].constell;
                awards.petexp = self->mDungeAward[i].petexp;
				awards.itemsAward = self->mDungeAward[i].awardItems;
                
                ack.awards.push_back(awards);
            }
            ack.result = eDungGetout;
            sendNetPacket(sessionid, &ack);
            
            SceneCfgDef* passscene = SceneCfg::getCfg(self->getCurrSceneId()-1);
            if (passscene) {
//                sendFriendAwardMail(passscene, self);
                saveFriendAward(passscene, self);
            }
            
            self->mDungeAward.clear();
            self->clearFriendInfo();
            
            self->backToCity();
            
            break;
        }

        if ( req.sceneid != self->getCurrSceneId()) {
        
            kickAwayRoleWithTips(self, "");
            return;
        }
        
        int checkIndex = (req.sceneid / 1000) % 100;
        int level = req.sceneid % 100;
        
        int record = self->getFriendDungeAct()->getFriendDungeRecord(checkIndex);
        
        if (req.result == eDungeNext) {
            
            int battleTime = 0;
            
            if ( record && req.sceneid != record) {
                kickAwayRoleWithTips(self, "");
                return;
            }
            
            if (checkPlayerCompatPowerForCheat(self, currSceneid, battleTime)) {
                return;
            }
            
//            LogMod::addLogFinishDungeException(self, self->getCurrSceneId(), battleTime);
            
            LogMod::addLogFinishFriendDunge(self, checkIndex, level);
            
            if ( canInorgeBattleCheck() == false && req.deadMonsterNum != SceneCfg::getMonsterCount(_sceneCfg)) {
                kickAwayRoleWithTips(self, "");
                return;
            }
            
            handlerDungeResultAward(currSceneid, req.result, self);
            
            self->addFat(-_sceneCfg->energy_consumption);
            
//            self->onCalPlayerProp();不知道为什么要算属性，先去掉by wangzhigang
            
            self->sendRolePropInfoAndVerifyCode(true);
            self->onSendPetPropAndVerifycode(self->mPetMgr->getActivePet(), true);
            self->getFriendDungeAct()->saveFriendDungeRecord(checkIndex, req.sceneid + 1);
            self->setPreEnterSceneid(currSceneid + 1);
            
            sendNetPacket(self->getSessionId(), &ack);
            break;
        }

        if (req.result == eDungeFail)
        {            
            for (int i = 0; i < self->mDungeAward.size(); i++) {
                obj_dungeawawrd awards;
                awards.secneid = self->mDungeAward[i].sceneid;
                awards.flopindex = self->mDungeAward[i].flopIndex;
                awards.drops = self->mDungeAward[i].drops;
                
                awards.gold = self->mDungeAward[i].gold;
                awards.exp = self->mDungeAward[i].exp;
                awards.battlepoint = self->mDungeAward[i].battlepoint;
                awards.constell = self->mDungeAward[i].constell;
                awards.petexp = self->mDungeAward[i].petexp;
				awards.itemsAward = self->mDungeAward[i].awardItems;
                
                ack.awards.push_back(awards);
            }
            ack.result = eDungeFail;

            self->setCurrSceneId(self->getSceneID());
            sendNetPacket(sessionid, &ack);
            
            SceneCfgDef* passscene = SceneCfg::getCfg(self->getCurrSceneId()-1);
            if (passscene) {
//                sendFriendAwardMail(passscene, self);
                saveFriendAward(passscene, self);
            }
            
            self->mDungeAward.clear();
            self->clearFriendInfo();
            
            self->backToCity();
            break;
        }

        if (req.result == eDungeFinish)
        {
            
            if (canInorgeBattleCheck() == false && req.deadMonsterNum != SceneCfg::getMonsterCount(_sceneCfg)) {
                kickAwayRoleWithTips(self, "");
                return;
            }

            if (_sceneCfg->hasPortal) {
                kickAwayRoleWithTips(self, "");
                return;
            }
            
            if (req.sceneid != record) {
                kickAwayRoleWithTips(self, "");
                return;
            }
            
            int battleTime = 0;
            if (checkPlayerCompatPowerForCheat(self, currSceneid, battleTime)) {
                return;
            }
            
//            LogMod::addLogFinishDungeException(self, self->getCurrSceneId(), battleTime);
            
            LogMod::addLogFinishFriendDunge(self, checkIndex, level);
            
            handlerDungeResultAward(currSceneid, req.result, self);
            
            self->addFat(-_sceneCfg->energy_consumption);
            
            for (int i = 0; i < self->mDungeAward.size(); i++) {
                obj_dungeawawrd awards;
                
                awards.secneid = self->mDungeAward[i].sceneid;
                awards.flopindex = self->mDungeAward[i].flopIndex;
                awards.drops = self->mDungeAward[i].drops;
                
                awards.gold = self->mDungeAward[i].gold;
                awards.exp = self->mDungeAward[i].exp;
                awards.battlepoint = self->mDungeAward[i].battlepoint;
                awards.constell = self->mDungeAward[i].constell;
                awards.petexp = self->mDungeAward[i].petexp;
				awards.itemsAward = self->mDungeAward[i].awardItems;
                
                ack.awards.push_back(awards);
            }
            ack.result = eDungeFinish;
            sendNetPacket(sessionid, &ack);
            saveFriendAward(_sceneCfg, self);
            
            self->mDungeAward.clear();
            self->clearFriendInfo();
			self->getFriendDungeAct()->saveFriendDungeRecord(checkIndex, -1);
            self->backToCity();
            
			// 称号检查
			checkCopyHonor(self, currSceneid);
            break;
        }
    }while(false);
 
}}

hander_msg(req_friendDunge_reset, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_friendDunge_reset ack;
    ack.errorcode = CE_OK;
    
    if (req.index < 0) {
        return;
    }
    
    role->getFriendDungeAct()->saveFriendDungeRecord(req.index, 0);
//    role->setFriendDungeFriend(0);
    role->getFriendDungeAct()->saveFriendDungeFriendRecord(req.index, 0);
    
    role->saveNewProperty();
    role->getFriendDungeAct()->getFriendDungeRecord(ack.record);
    
    sendNetPacket(sessionid, &ack);
}}

/******************************************************************************/

hander_msg(req_act_player_move, req)
{
  hand_Sid2Role_check(sessionid, roleid, role)

  ActRoomMGR.SyncMove(role, req);

}}



#pragma mark -
#pragma mark 幻兽大比拼

hander_msg(req_activity_pet_casino_status, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    on_req_activity_pet_casino_status(role,req);
}}

hander_msg(req_activity_pet_casino_wager, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    on_req_activity_pet_casino_wager(role,req);
}}

hander_msg(req_activity_pet_casino_thief, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    on_req_activity_pet_casino_thief(role,req);
}}

hander_msg(req_activity_pet_casino_current_history,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    on_req_activity_pet_casino_current_history(role,req);
}}

#pragma mark -
#pragma mark 公会试炼

hander_msg(req_get_guild_train_data,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    on_req_get_guild_train_data(role,req);
}}

hander_msg(req_enter_guild_train_room,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (!checkCurrSceneType(req.sceneid, stChallenge)) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    on_req_enter_guild_train_room(role,req);
}}

hander_msg(req_leave_guild_train_room,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (!checkCurrSceneType(role->getCurrSceneId(), stChallenge)) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    on_req_leave_guild_train_room(role,req);

}}

hander_msg(req_guild_train_room_next_wave,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    if (!checkCurrSceneType(role->getCurrSceneId(), stChallenge)) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    on_req_guild_train_room_next_wave(role,req);
}}



#pragma mark -
#pragma mark 守卫神像
/**************************  begin towerdefense  *************************/

hander_msg(req_get_towerdefense_data, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    
    ack_get_towerdefense_data ack;
    
    if (!SActMgr.IsActive(self->getTowerDefenseAct()->getActid())) {
        ack.times = 0;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    self->getTowerDefenseAct()->checkFresh();
    
    RoleCfgDef rolecfg = RoleCfg::getCfg(self->getJob(), self->getLvl());
    
    //总次数-已经用过的次数+使用道具增加的次数
    ack.times = rolecfg.getTowertimes() -  self->getTowerDefenseAct()->getEnterTimes() + self->getIncrTowerdefenseTimes();
    
    check_min(ack.times, 0);
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_enter_tower_room, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    
    ack_enter_tower_room ack;
    ack.sceneid = req.sceneid;
    
    //防止客户端重复发多条消息过来
    if (0 < self->getPreEnterSceneid()) {
        if (req.oncemore == 0) {
            return;
        } else if (self->getCurrSceneId() != req.sceneid) {
            return;
        } else if (self->getPreEnterSceneid() == req.sceneid) {
            return;
        }
    }
    
    if (!checkCurrSceneType(req.sceneid, stDefendStatue)) {
        kickAwayRoleWithTips(self, "");
        return;
    }
    
    TowerDefense* towerRoom = TowerDefense::Create();
    do
    {
        if (towerRoom == NULL) {
            ack.errorcode = CE_CREATE_ACTIVITION_ROOM_FAIL;
            break;
        }
        
        if ( !towerRoom->IsOpen() ) {
            ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
            break;
        }
        
        RoleCfgDef rolecfg = RoleCfg::getCfg(self->getJob(), self->getLvl());
        //
        if (self->getIncrTowerdefenseTimes() <= 0 && self->getTowerDefenseAct()->getEnterTimes() >= rolecfg.getTowertimes()) {
            ack.errorcode = CE_YOUR_ACTIVITION_TIMES_USEUP;
            break;
        }
        
        SceneCfgDef* sceneCfg = SceneCfg::getCfg(req.sceneid);
        if (sceneCfg == NULL) {
            ack.errorcode = CE_READ_SCENE_CFG_ERROR;
            break;
        }
        
        if (sceneCfg->sceneType != stDefendStatue) {
            kickAwayRoleWithTips(self, "");
            return;
        }
        
        if (sceneCfg->minLv > self->getLvl()) {
            ack.errorcode = CE_YOUR_LVL_TOO_LOW;
            break;
        }
        towerRoom->Enter(self);
        self->setPreEnterSceneid(req.sceneid);
        // 刷新进入地下城时间
        int nowtime = time(NULL);
        self->setLastEnterDungeonTime(nowtime);
        self->saveNewProperty();
        
        if (self->getDailyScheduleSystem()) {
            self->getDailyScheduleSystem()->onEvent(kDailyScheduleDefendStatue, req.sceneid, 1);
        }
        
    }while(false);
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_leave_tower_room, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    
    if (!checkCurrSceneType(self->getCurrSceneId(), stDefendStatue)) {
        kickAwayRoleWithTips(self, "");
        return;
    }
    
    ack_leave_tower_room ack;
    ack.errorcode = CE_OK;
    do{
        TowerDefense* towerRoom = (TowerDefense*)ActRoomMGR.find("TowerDefense", self);
        if (towerRoom == NULL) {
            ack.errorcode = CE_CAN_NOT_FIND_ACTIVITION_ROOM;
            break;
        }
        
        //手动退出或死亡才扣次数
        if (self->getIncrTowerdefenseTimes() > 0)
        {
            self->setIncrTowerdefenseTimes(self->getIncrTowerdefenseTimes() - 1);
            self->saveNewProperty();
            
            string addAction = Utils::makeStr("enter_%d", self->getCurrSceneId());
            
            self->addLogActivityCopyEnterTimesChange(eActivityEnterTimesTowerDefense, addAction.c_str());
        }
        else
        {
            // 如果是昨天进地下城，今天出来，不扣次数
            int nowtime = time(NULL);
            int lasttime = self->getLastEnterDungeonTime();
            if (isSameDay(nowtime, lasttime))
            {
                self->getTowerDefenseAct()->addEnterTimes(1);
                
                string addAction = Utils::makeStr("enter_%d", self->getCurrSceneId());
                
                self->addLogActivityCopyEnterTimesChange(eActivityEnterTimesTowerDefense, addAction.c_str());
            }
        }
        
        towerRoom->sendAward(self);
        
        towerRoom->Remove(self);
        
        self->backToCity();
        
    }while(false);
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_tower_room_next_wave, req)
{
    hand_Sid2Role_check(sessionid, roleid, self);
    
    int sceneIndex = req.towerlvl; 
    int usetime = req.usetime;
    int monstercount = req.monstercount;
    int waves = req.wave;
    
    TowerDefense* defenseroom = (TowerDefense*)ActRoomMGR.find("TowerDefense", self);
    if (defenseroom == NULL) {
        return;
    }
    
    //add by wangzhigang 2014.5.5
    if (!checkCurrSceneType(self->getCurrSceneId(), stDefendStatue)) {
        kickAwayRoleWithTips(self, "");
        return;
    }
    
    RoleTowerDefenseData roleData = defenseroom->getRoleData(roleid);
    
    //check something
    if ( usetime < 1 || ( waves - roleData.m_waves ) > 1 ) {
        kickAwayRoleWithTips(self, "");
        return;
    }
    
    //算积分
    DefendStatueCfgDef* defenseCfg = DefendStatueCfg::getDefendStatueCfg(sceneIndex, waves);
    if (defenseCfg == NULL) {
        return;
    }
    
    int standarKillTime = defenseCfg->m_standarTime;
    check_min(standarKillTime, 1);
    
    int timediff = standarKillTime - usetime;
    
    int getPoints = ( (timediff/(float)standarKillTime) * 0.01 * defenseCfg->m_coefficient + 1) * defenseCfg->m_standarPoints;
    check_range(getPoints, defenseCfg->m_minPoints, defenseCfg->m_maxPoints);
    
    
    RewardStruct reward;
    
    reward.reward_exp = getPoints * defenseCfg->mAward.exp_coef;
    reward.reward_gold = getPoints * defenseCfg->mAward.gold_coef;
    reward.reward_petexp = getPoints * defenseCfg->mAward.petexp_coef;
    reward.reward_batpoint = getPoints * defenseCfg->mAward.batpoint_coef;
    
    defenseroom->AddPlayerResult(roleid, getPoints, reward);
    
    //加个日志
    LogMod::addLogTowerDefensePass(self, req.wave, usetime, getPoints, roleData.m_friendsInAct.size(), self->getCurrSceneId());
    
}}
/**************************  end  towerdefense  ************************/






/*************************************  神秘副本 ( 俗称幻兽大本营 ) **************************************/
hander_msg(req_get_mystical_copylist, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_get_mystical_copylist ack;
    ack.errorcode = CE_OK;
    do
    {
        /*
        if (!SActMgr.IsActive(ae_mystical_copy)) {
            ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
            break;
        }
        */
        
        MysticalCopyGroupData indexsGroups;
        g_MysticalCopyMgr.getCopyListIndex(ack.indexs);
        g_MysticalCopyMgr.getCycleCopyListIndex(ack.cycleIndexs);
        ack.times = role->getMysticalCopyTimes();
        ack.consumabletimes = role->getIncrMysticalCopyBOSSandGoblinTimes();
        ack.surplusSec = g_MysticalCopyMgr.getSurplusSecForFreshList();
        
    }while(false);
    sendNetPacket(role->getSessionId(), &ack);
}}

hander_msg(req_enter_mysticalcopy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_enter_mysticalcopy ack;
    ack.errorcode = CE_OK;
    ack.oncemore = req.oncemore;
    do
    {
        /*
        if (!SActMgr.IsActive(ae_mystical_copy)) {
            ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
            break;
        }
         */
        
        
        //防止客户端重复发多条消息过来
        if (0 < role->getPreEnterSceneid()) {
            if (req.oncemore == 0) {
                return;
            } else if (role->getCurrSceneId() != req.copyid) {
                return;
            } else if (role->getPreEnterSceneid() == req.copyid) {
                return;
            }
        }
        
        //判断可否进入，防止同时进入多个副本
        if (!checkCanEnterCopy(role, req.copyid)) {
            return;
        }
        
        SceneCfgDef* scenecfg = SceneCfg::getCfg(req.copyid);
        if (scenecfg && scenecfg->energy_consumption > role->getFat()) {
            ack.errorcode = CE_FAT_NOT_ENOUGH;
            break;
        }
        
        if (!checkCurrSceneType(req.copyid, stMysticalCopy)) {
            kickAwayRoleWithTip(role);
            return;
        }
        
        //静态版（海外版）加上推荐战斗力验证
        if ( !g_MysticalCopyMgr.isRunning() && role->getBattleForce() < scenecfg->fightpoint) {
            ack.errorcode = CE_YOUR_BATTLEFORCE_IS_TOO_LOW;
            break;
        }
        
        ack.errorcode = g_MysticalCopyMgr.checkCopyAndIndex(req.listIndex, req.copyid);
        
        if ( ack.errorcode ) {
            break;
        }
        
        MysticalCopy* copyroom = (MysticalCopy*)ActRoomMGR.find("MysticalCopy", role);
        
        if (copyroom) {
            ack.errorcode = CE_YOU_HAVE_IN_THIS_SCENE;
            break;
        }
        
        copyroom = MysticalCopy::Create();
        
        if (copyroom) {
            copyroom->setCopyId(req.copyid);
            
            if (role->getIncrMysticalCopyBOSSandGoblinTimes() > 0 )
            {
                //百分百出现隐藏 BOSS 与 哥布林
                copyroom->CreateMonsters(req.copyid);
            }
            else
            {
                int times = role->getMysticalCopyTimes();
                copyroom->CreateMonsters(req.copyid, times);
            }
        
            copyroom->Add(role, 0, 0, false);
            role->setPreEnterSceneid(req.copyid);
            role->setBattleTime(Game::tick);
            
            const std::vector<MysticalMonsterDef>& monsters = copyroom->getMonsters();
            
            for (int i = 0; i < monsters.size(); i++) {
                obj_mysticalmonster obj;
                obj.monsterid = monsters[i].m_Monsterid;
                obj.existtime = monsters[i].m_Existtime;
                obj.position_x = monsters[i].x;
                obj.position_y = monsters[i].y;
                ack.monsters.push_back(obj);
            }
            
            ack.copyid = req.copyid;
            
            UpdateQuestProgress("enterpetcamp", req.copyid, 1, role, true);
            break;
        }
        
    }while(false);
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_leave_mysticalcopy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_leave_mysticalcopy ack;
    ack.errorcode = CE_OK;
    SceneCfgDef* scenecfg = SceneCfg::getCfg(role->getCurrSceneId());
    
    role->backToCity();
    
    if (scenecfg == NULL) {
        ack.errorcode = CE_READ_SCENE_CFG_ERROR;
        sendNetPacket(sessionid, &ack);
    }
    
    if (!checkCurrSceneType(scenecfg, stMysticalCopy)) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    do
    {
        MysticalCopy* copyroom = (MysticalCopy*)ActRoomMGR.find("MysticalCopy", role);
        if (copyroom == NULL) {
            printf("can not find mysticalRoom\n");
            return;
        }
        
        //验证bossid是否存在
        if (!copyroom->checkBossExist(req.bossid)) {
            copyroom->Remove(role);
            kickAwayRoleWithTips(role, "");
            return;
        }
        
        copyroom->setBattleKillBoss(roleid, req.bossid);
        copyroom->Remove(role);
        role->setBattleTime(0);
        
        role->getDailyScheduleSystem()->onEvent(kDailyScheduleMysteriousCopy, req.copyid, 1);
		
		if (req.bossid != 0) {
			// 称号检查
			SHonorMou.procHonor(eHonorKey_Kill, role, xystr("%d", req.bossid), "1");
		}
    }while(false);

//    sendNetPacket(sessionid, &ack);

}}

handler_msg(req_pass_mysticalcopy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    int copyid = role->getCurrSceneId();
    
    if (!checkCurrSceneType(copyid, stMysticalCopy)) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    MysticalCopy* copyroom = (MysticalCopy*)ActRoomMGR.find("MysticalCopy", role);
    if (copyroom) {
        
        if (role->getBattleTime() == 0) {
            kickAwayRoleWithTips(role, "");
            return;
        }
        
        int monsize = req.monsters.size();
        
        //检查怪物是否存在
        for (int i = 0; i < monsize; i++) {
            if(canInorgeBattleCheck() == false && !copyroom->checkMonsterExist(req.monsters[i]))
            {
                copyroom->Remove(role);
                kickAwayRoleWithTips(role, "");
                return;
            }
        }
        
        int battleTime = 0;
        if (req.ispass) {
            if (checkPlayerCompatPowerForCheat(role, copyid, battleTime)) {
                return;
            }
			// 称号检查
			checkCopyHonor(role, copyid);
            
            ack_pass_mysticalcopy ack;
            
            const MysticalMonsterDef& boss = copyroom->getBoss();
            
            ack.boss.monsterid = boss.m_Monsterid;
            ack.boss.existtime = boss.m_Existtime;
            ack.boss.position_x = boss.x;
            ack.boss.position_y = boss.y;
            ack.bosstalkingid = boss.talkingid;
            
            sendNetPacket(sessionid, &ack);
        }
        
        role->setBattleTime(0);
        
        MysticalResult result;
        result.isPass = req.ispass;
        result.m_killMonsters = req.monsters;
        result.battleTime = battleTime;
        if(!copyroom->setBattleResult(roleid, result))
            printf("save mysticalcopy battleResult error\n");
		
		// 称号检查
		SHonorMou.procHonor(eHonorKey_Kill, role, req.monsters);
    }
}}


handler_msg(req_enter_tlk_copy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_enter_tlk_copy ack;
    ack.errorcode = 1;
    ack.copyId = req.copyId;
    do
    {
        // 判断活动是否开启
		// 04-24 修改 by jin
        if (GameFunctionCfg::getGameFunctionCfg().limitTimeKilled == eEffectLimitTimeKilled
			&& !SActMgr.IsActive(req.activityId))
        {
            ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
            break;
        }
        
        //当前已经在副本
        if (role->getSceneID() != role->getCurrSceneId())
        {
            return;
        }
        
        //防止客户端重复发多条消息过来
        if (0 < role->getPreEnterSceneid()) {
            return;
        }
        
        //获取活动地图配置
        vector<LimitActivityDef *> datas = LimitActivityCfg::getAllLimitActivityDatas(req.activityId);
        LimitActivityDef *def = NULL;
        for (int i = 0; i < datas.size(); i++)
        {
            if (datas[i]->mapid == req.copyId)
            {
                def = datas[i];
            }
        }
        if (def == NULL)
        {
            ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
            break;
        }
        
        //是否达到进入登记
        if (def->minLvl > role->getLvl())
        {
            ack.errorcode = CE_YOUR_LVL_TOO_LOW;
            break;
        }
        
        //是否有足够物品和精力
        CopyCost cost;
        GridArray effgrids;
        rewardsCmds2ItemArray(def->costItmem, cost.mItems, cost.mOther);
        if (cost.mItems.size() > 0)
        {
            if (!role->getBackBag()->PreDelItems(cost.mItems, effgrids))
            {
                ack.errorcode = CE_ITEM_NOT_ENOUGH;
                break;
            }
        }
        if (role->getFat() < cost.mOther.reward_fat)
        {
            ack.errorcode = CE_FAT_NOT_ENOUGH;
            break;
        }
        
        //
        TimeLimitKillCopy* copy = TimeLimitKillCopy::create(req.copyId, cost);
        if (copy != NULL)
        {
            ack.errorcode = 0;
            ack.gametimes = copy->getLastTime();
            copy->Add(role, 0, 0, false);
            role->setCopy(copy);
            ActRoomMGR.Reg(copy);
            // 设置当前副本
            role->setPreEnterSceneid(req.copyId);
            role->setBattleTime(Game::tick);
            
        }
    }
    while (0);

    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_kill_tlk_monster, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (!checkCurrSceneType(role->getCurrSceneId(), stLimitTimeKilled)) {
//        kickAwayRoleWithTips(role, "");
        return;
    }
    
    TimeLimitKillCopy* copy = dynamic_cast<TimeLimitKillCopy*>(ActRoomMGR.find("TimeLimitKillCopy", role));
    if (copy == NULL)
    {
        log_error("can't find time limit kill copy");
        return;
    }
    ack_kill_tlk_monster ack;
    ack.errorcode = 0;
    for (int i = 0; i < req.monsters.size(); i++)
    {
        if (!copy->killMonster(req.monsters[i].monsterid))
        {
            log_error("no such monster");
        }
    }
    ack.score = copy->getScore();
    ack.killscount = copy->getTotalKillCount();
    
    sendNetPacket(sessionid, &ack); 
}}


hander_msg(req_leave_tlk_copy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (!checkCurrSceneType(role->getCurrSceneId(), stLimitTimeKilled)) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    TimeLimitKillCopy* copy = dynamic_cast<TimeLimitKillCopy*>(ActRoomMGR.find("TimeLimitKillCopy", role));
    if (copy == NULL)
    {
        log_error("can't find time limit kill copy");
        return;
    }
    
    int usetime = Game::tick - role->getBattleTime();
    LogMod::addLogLimitTimeKill(role, usetime, copy->getScore());
    
    // 改回主城ID
    ack_leave_tlk_copy ack;
    ack.errorcode = 0;
    ack.type = req.type;
    if (req.type == 1) {
        ack.awards = copy->handleGetAward(role);
    } else {
        copy->handleLeave(role);
    }
    sendNetPacket(sessionid, &ack);
    
    role->backToCity();
    
}}



/*********************************** 名人堂 ***********************************/
hander_msg(req_fameshall_fameslist, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    g_fameHallMgr.safe_getFamesHallFameList(role);
}}

hander_msg(req_fameshall_player_data, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    g_fameHallMgr.safe_getFamesHallPlayerData(role, req.famesVersion);
}}

handler_msg(req_fameshall_beginBattle, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (0 < role->getPreEnterSceneid()) {
        return;
    }
    
    SceneCfgDef* scene = SceneCfg::getCfg(req.sceneid);
    
    if (scene == NULL) {
        return;
    }
    
    if (scene->sceneType != stFamousDunge) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    //判断可否进入，防止同时进入多个副本
    if (!checkCanEnterCopy(role, req.sceneid)) {
        return;
    }
    
    role->setPreEnterSceneid(req.sceneid);
    
    g_fameHallMgr.safe_beginFamesHallBattle(role, req.layer, req.sceneid);
}}

hander_msg(req_finish_famesHall, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SceneCfgDef* sceneCfg = SceneCfg::getCfg(role->getCurrSceneId());
    if (sceneCfg == NULL) {
        return;
    }
    
    if(sceneCfg->sceneType != stFamousDunge)
    {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    FamesHallPointFactor factor;
    factor.roleBattleForce = role->getBattleForce();
    g_fameHallMgr.safe_battleHandle(role, req.result, req.layer, role->getCurrSceneId(), factor);
    role->setBattleTime(0);
    
    //回城镇
    role->backToCity();
     
}}

hander_msg(req_enter_elitecopy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_enter_elitecopy ack;
    ack.errorcode = CE_OK;
    
    if (role->getEliteTimes() + role->getIncrEliteCopyTimes() <= 0) {
        return;
    }
    
    //防止客户端重复发多条消息过来
    if (0 < role->getPreEnterSceneid()) {
        if (req.oncemore == 0) {
            return;
        } else if (role->getCurrSceneId() != req.copyid) {
            return;
//            以前进入副本 ID 是当前请求 ID则返回
        } else if (role->getPreEnterSceneid() == req.copyid) {
            return;
        }
    }
    
    if (!checkCurrSceneType(req.copyid, stEliteCopy))
    {
        kickAwayRoleWithTip(role);
        return;
    }
    
    //判断可否进入，防止同时进入多个副本
    if (!checkCanEnterCopy(role, req.copyid)) {
        ack.errorcode = 1;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    //SendAckEnterCopyHelper ackEnterCopy(role,req.copyid);
    
    //if (!ackEnterCopy.checkCanEnterCopy(role, req.copyid)) {
    //    ack.errorcode = 1;
    //    sendNetPacket(sessionid, &ack);
    //    return;
    //}
    
    // 刷新进入地下城时间
    int nowtime = time(NULL);
    role->setLastEnterDungeonTime(nowtime);
    role->saveNewProperty();
    
    UpdateQuestProgress("entercopy", req.copyid, 1, role, true);
    
    ack.copyid = req.copyid;
    role->setPreEnterSceneid(req.copyid);
    
    sendNetPacket(sessionid, &ack);

}}

/******************************* 公会宝藏战 ********************************/

hander_msg(req_get_treasure_copy_data, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_get_treasure_copy_data ack;
    
    ack.errorcode = CE_OK;
    ack.beginIndex = req.beginIndex;
    ack.endIndex = req.endIndex;
    ack.isGetall = false;
    
    vector<TreasureCopyItem*>& copylist = g_GuildTreasureFightMgr.getTreasureCopyList();
    int vsize = copylist.size();
    
    if (req.beginIndex >= req.endIndex || req.beginIndex >= vsize) {
        ack.errorcode = CE_TREASURECOPY_COPY_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    
    int guildId = SRoleGuild(role->getInstID()).getGuild();
    
    for (int i = req.beginIndex; i < vsize && i <= req.endIndex; i++) {
        obj_treasure_scene sceneinfo;
        
        copylist[i]->getTreasurecopyInfo(sceneinfo, guildId);
        
        ack.scenelist.push_back(sceneinfo);
    }
    
    if (req.endIndex >= (vsize - 1)) {
        ack.isGetall = true;
    }
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_enter_guild_treasurefight_activity, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_enter_guild_treasurefight_activity ack;
    
    if (!SActMgr.IsActive(ae_guild_treasure_fight)) {
        ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    g_GuildTreasureFightMgr.enterActivity(role);
    
    GuildTreasureFightResult* playerdata = g_GuildTreasureFightMgr.getPlayerFightResult(roleid);
    
    if (playerdata) {
        ack.myPoint = playerdata->points;
    }
    else{
        ack.myPoint = 0;
    }
    
    int guildPoints = 0;
    int guildRank = 0;
    
    g_GuildTreasureFightMgr.clientGetGuildResult(role->getGuildid(), guildPoints, guildRank);
    
    ack.errorcode = CE_OK;

    ack.guildPoint = guildPoints;
    ack.guildRank = guildRank;
    ack.actDuration = g_GuildTreasureFightMgr.getActivityDuration();
    ack.recoverCountdown = g_GuildTreasureFightMgr.getPlayerRecoverSec(roleid);

    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_leave_guild_treasure_activity, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_leave_guild_treasure_activity ack;
    ack.errorcode = CE_OK;
    
    g_GuildTreasureFightMgr.leaveActivity(role);
    
}}

hander_msg(req_enter_guild_treasurecopy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_enter_guild_treasurecopy ack;
    ack.errorcode = CE_OK;
    
    //检查活动是否在开放时间内
    if (!SActMgr.IsActive(ae_guild_treasure_fight)) {
        ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if ( (TreasureCopyState)g_GuildTreasureFightMgr.getPlayerState(role).state == eTreasureIncopy) {
        ack.errorcode = CE_TREASURECOPY_YOU_ARE_INCOPY_NOW;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (g_GuildTreasureFightMgr.getPlayerRecoverSec(roleid)) {
        ack.errorcode = CE_TREASURECOPY_YOU_ARE_NOT_RECOVER;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    ack.errorcode = g_GuildTreasureFightMgr.enterTreasureCopy(role, req.sceneid);
    
    ack.sceneid = req.sceneid;
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_quick_enter_guild_treasurecopy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_quick_enter_guild_treasurecopy ack;
    ack.errorcode = CE_TREASURECOPY_COPY_NOT_EXIST;
    
    //检查活动是否在开放时间内
    if (!SActMgr.IsActive(ae_guild_treasure_fight)) {
        ack.errorcode = CE_ACTIVE_IS_NOT_OPEN;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (role->getGuildid() == 0) {
        ack.errorcode = CE_GUILD_NOT_EXIST;
        sendNetPacket(sessionid, &ack);
    }
    
    if ( (TreasureCopyState)g_GuildTreasureFightMgr.getPlayerState(role).state == eTreasureIncopy) {
        ack.errorcode = CE_TREASURECOPY_YOU_ARE_INCOPY_NOW;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    if (g_GuildTreasureFightMgr.getPlayerRecoverSec(roleid)) {
        ack.errorcode = CE_TREASURECOPY_YOU_ARE_NOT_RECOVER;
        sendNetPacket(sessionid, &ack);
        return;
    }
    
    TreasureCopyItem* copy = g_GuildTreasureFightMgr.quickEnterTreasureCopy(role);
    
    if (copy) {
        ack.errorcode = CE_OK;
        ack.sceneid = copy->sceneid;
        ack.sceneName = copy->sceneName;
    }
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_treasurecopy_get_fighting_points, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_treasurecopy_get_fighting_points ack;
    
    g_GuildTreasureFightMgr.clientGetGuildResult(role->getGuildid(), ack.guildPoints, ack.guildRank);
    
    GuildTreasureFightResult* playerResult = g_GuildTreasureFightMgr.getPlayerFightResult(roleid);
    
    if (playerResult) {
        ack.myPoints = playerResult->points;
    }
    else{
        ack.myPoints = 0;
    }
    
    sendNetPacket(sessionid, &ack);
    
}}

extern bool  sortByPoints(GuildTreasureFightResult *first, GuildTreasureFightResult *second);

hander_msg(req_treasurecopy_get_fighting_rank, req)
{
//    hand_Sid2Role_check(sessionid, roleid, role);
//    ack_treasurecopy_get_fighting_rank ack;
//    
//    TreasureCopyItem* copy = g_GuildTreasureFightMgr.getTreasurecopy(role->getGuildTreasureSceneid());
//    
//    vector<GuildTreasureFightResult*> results;
//    results.clear();
//    
//    set<int>::iterator iter;
//    
//    for (; iter != copy->players.end(); iter++) {
//        
//        int roleid = *iter;
//        
//        GuildTreasureFightResult* tmp = g_GuildTreasureFightMgr.getPlayerFightResult(roleid);
//        
//        results.push_back(tmp);
//    }
//    
//    ::sort(results.begin(), results.end(), sortByPoints);
//    
//    for (int i = 0; i < results.size(); i++) {
//        obj_treasurecopy_rankdata obj;
//        obj.roleid = results[i]->ownerid;
//        obj.points = results[i]->points;
//        
//        ack.datas.push_back(obj);
//    }
//    
//    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_treasurecopy_get_manors, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_treasurecopy_get_manors ack;
    
    g_GuildTreasureFightMgr.clientGetManors(roleid, ack);
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_treasurecopy_get_guildmanor_award, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_treasurecopy_get_guildmanor_award ack;
    
    ack.index = req.index;
    
    string award = "";
    
    ack.errorcode = g_GuildTreasureFightMgr.getMyGuildManorAward(req.index, role, award);
    
    ack.award = award;
    
    sendNetPacket(sessionid, &ack);
}}
