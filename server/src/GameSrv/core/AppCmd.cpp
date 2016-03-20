//
//  GmCmd.cpp
//  GameSrv
//
//  Created by jin on 14-4-28.
//
//
#include "Role.h"
#include "Game.h"
#include "DBRolePropName.h"
#include "mail_imp.h"
#include "Quest.h"
#include "json/json.h"
#include "Skill.h"
#include "GuildRole.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "PetPvp.h"
#include "GameLog.h"
#include "RealPvpMgr.h"
#include <string>
#include <vector>
#include "recharge_mod.h"
#include "WebStateMgr.h"
#include "Paihang.h"
#include "Honor.h"

using namespace std;

#define APPCMD "appcmd"

//　注意用这宏,　要有roleId字段
#define APPCMD_ROLE(role) \
	int roleId = xyJsonAsInt(data["roleId"]); \
	Role *role = SRoleMgr.GetRole(roleId); \
	if (NULL == role) \
	{	\
	return; \
	}

typedef void (*AppCmdHandlerCb)(const Json::Value &data);
map<string, AppCmdHandlerCb> cmdAppMap;

bool registerAppCmd(const string &cmd, AppCmdHandlerCb handler)
{
    cmdAppMap[cmd] = handler;
	return true;
}

void notifyAppCmdMsg(const string &cmdStr, const Json::Value &data)
{
	APPCMD_ROLE(role);
	notify_app_cmd notify;
	notify.data = cmdStr;
	role->send(&notify);
}

void appCmdResponse(const string &data)
{
	Xylog log(eLogName_AppCmd, 0);
	log << data;
	
	// game.ini的开关关了 并且 web的开关也关了
	if (Process::env.getInt("gm_mode") != 1 && !g_WebStateMou.isAppCmd()) {
        return;
    }
	Json::Value value;
	if (!xyJsonSafeParse(data, value)) {
		return;
	}
	string cmd = value["cmd"].asString();
	map<string, AppCmdHandlerCb>::iterator it = cmdAppMap.find(cmd);
	if (it == cmdAppMap.end()) {
		return;
	}
	it->second(value["data"]);
	log << "1";
	
	notifyAppCmdMsg(data, value["data"]);
}

#define APPCMD_FUNC(cmd) \
	void appCmd##cmd(const Json::Value &data);\
	bool cmd##ret = registerAppCmd(#cmd, appCmd##cmd); \
	void appCmd##cmd

APPCMD_FUNC(recharge)(const Json::Value &data)
{
	APPCMD_ROLE(role);
	int num = xyJsonAsInt(data["num"]);
	createNewOrderCmd(APPCMD, num, role->getInstID(), APPCMD);
}

APPCMD_FUNC(fat)(const Json::Value &data)
{
	APPCMD_ROLE(role);
	int num = xyJsonAsInt(data["num"]);
	role->addFat(num - role->getFat());
}

APPCMD_FUNC(gold)(const Json::Value &data)
{
	APPCMD_ROLE(role);
	int num = xyJsonAsInt(data["num"]);
	role->addGold(num - role->getGold(), APPCMD);
}
APPCMD_FUNC(batpoint)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    role->addBattlePoint(num-role->getBattlePoint(), APPCMD);
}
APPCMD_FUNC(constell)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    
    role->addConstellVal(num - role->getConstellVal(),APPCMD);
}
APPCMD_FUNC(lvl)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_max(num, RoleCfg::getMaxRoleLvl());
    check_min(num, 1);
    
    role->setLvl(num );
    
    role->saveProp(GetRolePropName(eRolePropLvl), Utils::itoa(role->getLvl()).c_str());
    notify_lvl_up notify;
    notify.lvl = role->getLvl();
    notify.exp = role->getExp();
    sendNetPacket(role->getSessionId(), &notify);
}
APPCMD_FUNC(vipexp)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    role->addVipExp(num - role->getVipExp(), APPCMD);
}
APPCMD_FUNC(rmb)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    role->addRmb(num - role->getRmb(), APPCMD);
    //role->addRmb(num , APPCMD);
}
APPCMD_FUNC(constrib)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    RoleGuildProperty& prop = SRoleGuild(role->getInstID());
    
    prop.addConstrib(num - prop.getConstrib() , APPCMD);

}
APPCMD_FUNC(prestige)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    int old = role->getPrestige();
    role->getHonorMgr()->addPrestige(num - old);
    
}

APPCMD_FUNC(exp)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    RoleCfgDef& cfg = RoleCfg::getCfg(role->getJob(), role->getLvl());
    if (xyJsonAsInt(data["overUpLvl"]) == 0) {
        num = cfg.getExp() -1 ;
    }
    role->setExp(0);
    role->addExp( num, APPCMD);
}

APPCMD_FUNC(sycMilitaryCfg)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    g_RealpvpMgr.safe_gmSetMilitary(role->getInstID(),num);
    
}

APPCMD_FUNC(sycHonour)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    g_RealpvpMgr.safe_gmSetHons(role->getInstID(),num);
    
    
}
APPCMD_FUNC(sycMedal)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    g_RealpvpMgr.safe_gmSetMedal(role->getInstID(),num);
    
}

APPCMD_FUNC(sycLostTimes)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    g_RealpvpMgr.safe_gmSetLostTimes(role->getInstID(),num);
    
}

APPCMD_FUNC(sycWinningTimes)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    g_RealpvpMgr.safe_gmSetWinningTimes(role->getInstID(),num);
    
}

APPCMD_FUNC(petpvppoints)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    SPetPvpMgr.safe_gmSetPetPvpPoints(role->getInstID(), num);
    
}

APPCMD_FUNC(petpvpcups)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    SPetPvpMgr.safe_gmSetPetPvpCups(role->getInstID(), num);
    
}

APPCMD_FUNC(petLvl)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    Pet* pet = role->mPetMgr->getActivePet();
    if (pet)
    {
        pet->gm_SetLvl(num);
    }
    
}

APPCMD_FUNC(petStarLvl)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    Pet* pet = role->mPetMgr->getActivePet();
    if (pet)
    {
        pet->setStage(num);
        pet->save();
        notify_sync_pet_stage notifystage;
        notifystage.petid = pet->petid;
        notifystage.stage = pet->getStage();
        sendNetPacket(role->getSessionId(), &notifystage);
        

    }
    
}

//主战幻兽力量
APPCMD_FUNC(petStre)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    Pet* pet = role->mPetMgr->getActivePet();
    if (pet)
    {
        pet->mStre = num;
        pet->save();
    }
    
}
//主战幻兽智力
APPCMD_FUNC(petInte)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    Pet* pet = role->mPetMgr->getActivePet();
    if (pet)
    {
        pet->mInte = num;
        pet->save();
    }
    
}
//主战幻兽体力
APPCMD_FUNC(petPhys)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    Pet* pet = role->mPetMgr->getActivePet();
    if (pet)
    {
        pet->mPhys = num;
        pet->save();
    }
    
}
//主战幻兽耐力
APPCMD_FUNC(petCapa)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    Pet* pet = role->mPetMgr->getActivePet();
    if (pet)
    {
        pet->mCapa=num;
        pet->save();
    }
    
}

APPCMD_FUNC(petStageLvl)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    Pet* pet = role->mPetMgr->getActivePet();
    if (pet)
    {
        pet->setMergeLvl(num);
        pet->save();
        /*
        notify_sync_pet_mergelvl notifystage;
        notifystage.petid = pet->petid;
        notifystage.stage = pet->getStage();
        sendNetPacket(role->getSessionId(), &notifystage);
         */
        notify_sync_pet_mergelvl notify;
        notify.petid = pet->petid;
        notify.merge_lvl = num;
        notify.current_exp = 0;
        notify.extlvladd = pet->getMergeExtlvlAdd();
        notify.maxlvladd = pet->getMergeMaxlvlAdd();
        notify.growthadd = pet->getMergeGrowthAdd();
        sendNetPacket(role->getSessionId(), &notify);
    }
    
}

APPCMD_FUNC(petExp)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    
    Pet* pet = role->mPetMgr->getActivePet();;
    if (pet)
    {
        obj_petinfo petinto;
        pet->getPetInfo(petinto);
        
        role->addPetExp(pet->getId(), num - petinto.exp, "gm_command");
        pet->save();
        
    }
    
    
    
}

APPCMD_FUNC(guidFortune)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    SGuildMgr.safe_gmSetFortune(role->getId(), num);
}

APPCMD_FUNC(guidDcons)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    SGuildMgr.safe_gmSetDcons(role->getId(),num);
    
}
APPCMD_FUNC(guidLvl)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    int num = xyJsonAsInt(data["num"]);
    check_min(num, 0);
    SGuildMgr.safe_gmSetLvl(role->getId(),num);
    
}


APPCMD_FUNC(addTop50PersonOfGuild)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    //SSortMgr.safe_gmAddTop50Person(role->getId());
    SRoleMgr.safe_gmAddTop50Person(role->getId());
    
}

APPCMD_FUNC(gmMail)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    
    std::string serder = data["sender"].asString();
    std::string content = data["content"].asString();
    std::string title = data["title"].asString();
    std::string attach = data["attach"].asString();
    

    sendMail(0, serder.c_str(), role->getRolename().c_str(), title.c_str(),
             content.c_str(), attach.c_str(), Utils::itoa(role->getInstID()).c_str());
     
    
}


APPCMD_FUNC(gmItem)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    
    int count = 1;
    int item = xyJsonAsInt(data["itemID"]);
    count = xyJsonAsInt(data["count"]);
    
    GridArray grids;
    ItemArray items;
    items.push_back(ItemGroup(item, count));
    role->preAddItems(items, grids);
    
    //BackBag::UpdateBackBag(role->getInstID(), grids);
    //LogMod::addLogGetItem(role->getInstID(), role->getRolename().c_str(), role->getSessionId(), items, APPCMD);
//    role->updateBackBag(grids, items, true, APPCMD);
    role->playerAddItemsAndStore(grids, items, APPCMD, true);
}

APPCMD_FUNC(gmCopy)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    
    int copyid = xyJsonAsInt(data["copyID"]);
    role->SaveLastCopy(copyid);

    
}

APPCMD_FUNC(gmQuest)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    
    int lvl = xyJsonAsInt(data["lvl"]);
    //int rSectionID = role->getSessionId();
    for (int i = 1; i <= check_max(lvl, role->getLvl()); i++){
        list<int> quests;
        list<int> finishedquests;
        for (vector<int>::iterator iter = role->mFinishQuestIds.begin();  iter != role->mFinishQuestIds.end(); ++iter){
            finishedquests.push_back(*iter);
        }
        QuestMgr::GetQuests(i, finishedquests, NULL, &quests);
        
        //改成直接保存到数据库
        for (list<int>::iterator iter = quests.begin(); iter != quests.end(); ++iter){
            doRedisCmd("sadd questFinish:%d %d",role->getInstID(), *iter);
////////////////////////////////////////////            
//            ack_recvQuest ack;
//            ack.questid = *iter;
//            ack.err = 0;
//            role->send(&ack);
////////////////////////////////////////////            
////            //接受任务 npc,主,支线
////            begin_msg(req_recvQuest,5,2)
////            def_int(questid)
////            end_msg()
//            
//            req_recvQuest* moli_req_recvQuest = (req_recvQuest*)create_recvpacket(5,2);
//            moli_req_recvQuest->questid = *iter;
//            hander_recvpacketfun(5,2)((INetPacket*)moli_req_recvQuest,rSectionID);
////            ack_recvQuest ack;
////            ack.questid = *iter;
////            ack.err = 0;
////            role->send(&ack);
//            
//            //QuestMgr::FindQuest(*iter);
//            
//            //role->finshQuest(*iter);
//            //INetPacket* create_recvpacket(int type,int id);
//            //NetPacketHander hander_recvpacketfun(int type,int id);
////////////////////////////////////////////      
        }
    }
//    std::map<int, std::vector<int> >::iterator Iter = role->mQuestProgress.begin();
//    for (;Iter != role->mQuestProgress.end(); ++Iter)
//    {
//        req_finishQuest* moli_req_finishQuest = (req_finishQuest*)create_recvpacket(5,8);
//        moli_req_finishQuest->questid = Iter->first;
//        
//        Quest* TMPquest = QuestMgr::FindQuest(Iter->first);
//        std::vector<std::string> conditions=TMPquest->conditions;  //完成条件
//        TMPquest->conditions.clear();
//        hander_recvpacketfun(5,8)((INetPacket*)moli_req_finishQuest,rSectionID);
//        TMPquest->conditions = conditions;
//        
//        Iter = role->mQuestProgress.begin();
//    }
    
//    req_getQuests* moli_req_getQuests = (req_getQuests*)create_recvpacket(5,0);
//    hander_recvpacketfun(5,0)((INetPacket*)moli_req_getQuests,rSectionID);
    
    
}

APPCMD_FUNC(gmFreshguild)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    
    Guild& guild = SGuildMgr.getRoleGuild(role->getInstID());
    if( ! guild.isNull())
        SGuildMgr.freshGuild(guild);
    
    
}

APPCMD_FUNC(gmFreshrole)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    
    role->saveProp(GetRolePropName(eRolePropCrossDay), "0");
    role->RoleResetPvp();
        
        
}

APPCMD_FUNC(gmSkillfull)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    
    //公会技能
    RoleGuildProperty& prop = SRoleGuild( role->getInstID() );  //
//    RoleGuildSkill::Iterator beg = prop.getSkill().begin();     //
//    RoleGuildSkill::Iterator end = prop.getSkill().end();
    
    
    Guild& guild = SGuildMgr.getRoleGuild(role->getInstID());
    int guildLvl = guild.getLevel();
    
    std::map<int, int>  outData;
    GuildCfgMgr::getAllSkillByGuildLvl(guildLvl, outData);

    //prop.getSkill().set(find_iter->first, find_iter->second);
    std::map<int, int>::iterator beg = outData.begin();
    std::map<int, int>::iterator end = outData.end();
    for(;beg!=end;++beg){
        std::map<int, int>::iterator find_iter = outData.find(beg->first);
        if(find_iter != outData.end())
        {
            prop.getSkill().set(find_iter->first, find_iter->second);
        }
    }
    
    // 非公会技能
    vector< int > out_data_SkillCfgDef;
    int Job = role->getJob();
    if(! SkillCfg::getCfgByRoleJob(Job ,out_data_SkillCfgDef))
    {
        xyerr("一键完成技能错误 %s %d",role->getRolename().c_str(),role->getInstID());
        return ;
    }
    vector<int>::const_iterator const_beg = out_data_SkillCfgDef.begin();
    vector<int>::const_iterator const_end = out_data_SkillCfgDef.end();
    int i = 1;
    for (; const_beg!=const_end;++const_beg ) {
        i = *const_beg;
        int skillid = getStudySkill(role, i);
        int tmpid = skillid + 1;
        bool exist = false;
        SkillCfgDef* def = SkillCfg::getCfg(tmpid,exist);
        if (!exist) {
            break;
        }
        int maxlvl = def->getMaxLvl();
        check_max(maxlvl, role->getLvl());
        skillid += maxlvl;
        
        role->getSkillMod()->gm_learn(skillid);
    }
    
    vector<int> skillList;
    skillList = role->getSkillMod()->getStudySkillList();
//    SaveSkillTable(role->getInstID(), skillList);
    role->getSkillMod()->saveStudySkillList();
    
    ack_equiped_skill_list ack;
    ack.errorcode = 0;
    ack.skillList.clear();
    for(int i = 0;i < skillList.size(); i++)
    {
        obj_equiped_skill skill;
        skill.skillID = skillList[i];
        skill.index = i;
        ack.skillList.push_back(skill);
    }
    sendNetPacket(role->getSessionId(), &ack);
    
    role->CalcPlayerProp();
    
    //更新战斗力排行
    role->UploadPaihangData(eUpdate);
    
    
}

APPCMD_FUNC(gmClearbag)(const Json::Value & data)
{
    APPCMD_ROLE(role);
    
    string name = "BackBag";
    doRedisCmd("del %s:%d", name.c_str(), role->getInstID());
    
    
}
