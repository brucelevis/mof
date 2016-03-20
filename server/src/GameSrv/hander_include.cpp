//
//  hander_include.cpp
//  GameSrv
//
//  Created by cxy on 13-3-13.
//
//

#include "hander_include.h"
#include "flopcard.h"
#include "mail_imp.h"
#include "MysticalCopy.h"
#include "LogBase.h"
#include "CopyExtraAward.h"
#include "Quest.h"
#include "Role.h"

void rewardsGridArray2notify_addbagItem(GridArray& effgrids,notify_addbagItem& notify)
{
    for (int i =0; i != effgrids.size(); i++) {
        obj_bagItem obj;

        obj.index = effgrids[i].index;
        obj.itemid = effgrids[i].item.item;
        obj.count = effgrids[i].item.count;
        obj.json = effgrids[i].item.json;
        notify.itemlist.push_back(obj);
    }
}

//计算任务个数
void CountQuest(std::vector<int>& quests,int& outmaincount,int& outbranchcount,int& outdailycount)
{
    for (int i=0; i!= quests.size(); ++i) {
        Quest* qust = QuestMgr::FindQuest(quests[i]);
        if (qust) {
            switch (qust->type) {
                case 1:
                    outmaincount++;
                    break;
                case 2:
                    outbranchcount++;
                    break;
                case 3:
                    outdailycount++;
                    break;
                default:
                    break;
            }
        }
    }
}
//发放日常任务
void SendDailyQuest(Role* role)
{
    std::vector<int> doingquest;
    std::vector<int> outDailyQuestIds;
    role->doingQuest(doingquest);

    int maincount = 0;
    int branchcount = 0;
    int dailycount = 0;

    CountQuest(doingquest, maincount, branchcount, dailycount);

    //没有日常任务才发放
    if (0 == dailycount) {
        if (role->getDailyQuestCount() < 10) {
            QuestMgr::GetAcceptableQuests(role->getLvl(), role->mFinishQuestIds, doingquest,NULL,&outDailyQuestIds);
            if (!outDailyQuestIds.empty()) {
                notify_addQuest notify;
                std::random_shuffle(outDailyQuestIds.begin(), outDailyQuestIds.end());
                int questid = outDailyQuestIds[rand()%outDailyQuestIds.size()];
                notify.questids.push_back(questid);
                if(role->recvQuest(questid))
                {
                    const std::vector<int>& Progress = role->mQuestProgress[questid];
                    std::string strProgress = StrJoin(Progress.begin(),Progress.end(),";");
                    doRedisCmd("hset questProgress:%d %d %s",role->getInstID(),questid,strProgress.c_str());
                    role->addDailyQuestCount(1);
                    sendNetPacket(role->getSessionId(),&notify);
                }
            }
        }
    }
}

int sendNormalCopyAward(Role* role, SceneCfgDef* scenecfg, int flopid, int friendId, int& fcindex, vector<int>& drops, RewardStruct& notifyReward, string &awardItems)
{
    ItemArray items;
    RewardStruct reward;
    
    // 获取亲密度加成比例
    float friend_ratio = role->getFriendMgr()->getIntimacyCopyRatio(friendId);
    string dropStr = "";
    string awardBase = SflopcardMgr.RandomCard(flopid, fcindex, drops, dropStr, friend_ratio);
    string awardResult = awardBase;
	
	// 双倍药水
	SCopyExtraAward.doublePotion(role, scenecfg, awardBase, awardResult);
	
	// 副本活动奖励
	SCopyExtraAward.extraItemsAward(role->getInstID(), scenecfg->sceneId, awardBase, awardResult);
	
    std::vector<std::string> rewards = StrSpilt(awardResult, ";");
    rewardsCmds2ItemArray(rewards, items, reward);
    
	string buf = xystr("finishcopy:%d", scenecfg->sceneId);
    GridArray effgrids;
    //检查是否可以发放奖励
    int canAddItem = role->preAddItems(items, effgrids);
    
    int ret = CE_OK;   

    if (canAddItem == CE_OK) {
        
        role->playerAddItemsAndStore(effgrids, items, buf.c_str(), true);
    }
    else
    {
        sendMailWithName(0, "", role->getRolename().c_str(), "bag_full", awardResult.c_str());
        sendNotifyBagFullAndSendMail(role);
    }
    
    if (scenecfg != NULL)
    {
        reward.reward_exp += scenecfg->copy_exp;
        reward.reward_gold += scenecfg->copy_gold;
        reward.reward_batpoint += scenecfg->copy_battlepoint;
        reward.reward_consval += scenecfg->copy_protoss;
        reward.reward_petexp += scenecfg->copy_petExp;
        
        // 亲密度加成 
        role->getFriendMgr()->intimacyReward(friendId, scenecfg, reward);
        
		// 好友副本，如果双倍药水交次数还存在，则所得奖励翻倍
		SCopyExtraAward.doublePotionBase(role, scenecfg, reward);
        
		// 副本活动奖励
		SCopyExtraAward.extraBaseAward(role->getInstID(), scenecfg, reward);
		
        role->addExp(reward.reward_exp, buf.c_str());
        role->addGold(reward.reward_gold, buf.c_str());
        role->addBattlePoint(reward.reward_batpoint, buf.c_str());
        role->addConstellVal(reward.reward_consval, buf.c_str());
        role->addPetExp(role->getActivePetId(), reward.reward_petexp, buf.c_str());
        role->addEnchantDust(reward.reward_enchantdust, buf.c_str());
        role->addEnchantGold(reward.reward_enchantgold, buf.c_str());
        
        notifyReward = reward;
    }
	awardItems = awardResult;
    LogMod::addLogFinishCopy(role, scenecfg->sceneId, true, awardResult, scenecfg->sceneType);
    return ret;
}

bool checkMonsterExist(int monsterid, Role* role, SceneCfgDef* currScene)
{
    if (currScene == NULL) {
        return true;
    }
    if (!SceneCfg::isMonsterExist(currScene, monsterid)) {
        
        if ( currScene->sceneType == stMysticalCopy)
        {
            //检查是否是隐藏小怪
            MysticalCopy* copyroom = (MysticalCopy*)role->getCurrentActRoom();
            if (copyroom) {
                
                if( copyroom->checkMonsterExist(monsterid) || copyroom->checkBossExist(monsterid))
                {
                    return true;
                }
            }
        }
        return false;
    }
    return true;
}

bool checkCurrSceneType(int sceneid,const eSceneType &type)
{
    SceneCfgDef* scenecfg = SceneCfg::getCfg(sceneid);
    
    if (scenecfg == NULL) {
        return false;
    }
    
    return scenecfg->sceneType == type;
}

bool checkCurrSceneType(SceneCfgDef* scene,const eSceneType &type)
{
    if (scene == NULL) {
        return false;
    }
    
    return scene->sceneType == type;
}

bool checkCanEnterCopy(Role* role, int copyid)
{
    SceneCfgDef* newscene = SceneCfg::getCfg(copyid);
    if (newscene == NULL) {
        return false;
    }
    
    SceneCfgDef* currScene = SceneCfg::getCfg(role->getCurrSceneId());
    
    //验证等级
    if (role->getLvl() < newscene->minLv || role->getVipLvl() < newscene->min_vip) {
        return false;
    }
    //验证精力值
    if (role->getFat() < newscene->energy_consumption) {
        return false;
    }
    
    //如果当前是城镇，允许进入任何场景
    int currSceneType = currScene->sceneType;
    if (currSceneType == stTown) {
        return true;
    }   //如果当前是地下城好友地下城，可以进入地下城好友地下城或城镇，但不可以近其他场景
    else if(currSceneType == stDungeon || currSceneType == stFriendDunge ||
            currSceneType == stEliteCopy || currSceneType == stPetEliteCopy ||
            currSceneType == stMysticalCopy)
    {
        if (newscene->sceneType != stTown && currScene->sceneType != newscene->sceneType) {
            return false;
        }
    }
    else{    //除此之外的，只能进城镇
        if (newscene->sceneType != stTown) {
            return false;
        }
    }
    
    return true;
}

bool canInorgeBattleCheck()
{
    bool testMode = Process::env.getInt("testmode") == 1;
    bool oneKeyPass = Process::env.getInt("onekey_pass") == 1;
    if (testMode && oneKeyPass) {
        
        return true;
        
    }
    
    return false;
}
void sendNotifyBagFullAndSendMail(Role* role)
{
    if (role == NULL) {
        return;
    }
    
    notify_bagfull_and_send_mail notify;
    sendNetPacket(role->getSessionId(), &notify);
}

int sendPetAdventureAward(Role* self, int carryPet, SceneCfgDef* sceneCfg, int flopid, int& fcindex, vector<int>& drops, RewardStruct& notifyReward, bool carryRecommendedPet, string &itemsResult)
{
    
    RewardStruct reward;
    ItemArray allitems;
    allitems.clear();
    ItemArray extraItems;
    extraItems.clear();
    
    int sceneid = flopid / 10;
    PetAdventureCfgDef* def = PetAdventureCfg::getPetAdventureCfgDef(sceneid);
    if (def == NULL) {
        return CE_PET_ADVENTURE_CONFIG_NOT_FOUND;
    }
    
    char buf[32] = "";
    sprintf(buf, "finishcopy:%d", sceneCfg->sceneId);
    
    std::vector<int> dropindices;
    string dropStr = "";
    
    string awardStr = SflopcardMgr.RandomCard(flopid,fcindex, dropindices, dropStr);
    
    if (true == carryRecommendedPet) {
        awardStr = awardStr + ";" + def->extra_fall_items;
    }
    
    std::vector<std::string> rewards = StrSpilt(awardStr, ";");
    
    rewardsCmds2ItemArray(rewards, allitems, reward);
    
    float rewardrate = (true == carryRecommendedPet ? def->reward_rate:1.0);
    
    reward.reward_exp      += (sceneCfg->copy_exp         * rewardrate);
    reward.reward_batpoint += (sceneCfg->copy_battlepoint * rewardrate);
    reward.reward_consval  += (sceneCfg->copy_protoss     * rewardrate);
    reward.reward_gold     += (sceneCfg->copy_gold        * rewardrate);
    reward.reward_petexp   += (sceneCfg->copy_petExp      * rewardrate);
    
    if (true == carryRecommendedPet) {
        reward.reward_exp      += def->extra_exp;
        reward.reward_batpoint += def->extra_battlepoint;
        reward.reward_consval  += def->extra_consval;
        reward.reward_gold     += def->extra_gold;
        reward.reward_petexp   += def->extra_petexp;
    }
    
    //      发奖励
    self->addExp(reward.reward_exp, buf);
    self->addGold(reward.reward_gold, buf);
    self->addBattlePoint(reward.reward_batpoint, buf);
    self->addConstellVal(reward.reward_consval, buf);
    self->addPetExp(carryPet, reward.reward_petexp, buf);

    self->addEnchantDust(reward.reward_enchantdust, buf);
    self->addEnchantGold(reward.reward_enchantgold, buf);
    
    notifyReward = reward;
    
    GridArray effgrids;
    //检查是否可以发放奖励
    int canAddItem = self->preAddItems(allitems,effgrids);
    if (canAddItem == CE_OK) {
//        self->updateBackBag(effgrids, allitems, true, buf);
        self->playerAddItemsAndStore(effgrids, allitems, buf, true);
        
        itemsResult = awardStr;
    }
    else
    {
        sendMailWithName(0, "", self->getRolename().c_str(), "bag_full", awardStr.c_str());
        sendNotifyBagFullAndSendMail(self);
    }

    LogMod::addLogFinishPetAdventureCopy(self, sceneCfg->sceneId, true, awardStr, sceneCfg->sceneType);
    LogMod::addLogFinishCopy(self, sceneCfg->sceneId, true, awardStr, sceneCfg->sceneType);

    return CE_OK;
}

