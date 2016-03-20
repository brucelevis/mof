//
//  gmCommand.cpp
//  GameSrv
//
//  Created by prcv on 13-5-20.
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
#include "CapsuletoyMgr.h"
#include "Wardrobe.h"
#include "recharge_mod.h"
#include "daily_schedule_system.h"
#include "Global.h"
#include "EnumDef.h"
#include "Honor.h"
#include "Retinue.h"
using namespace std;

typedef void (*GmHandler)(Role* role, vector<string>& params);
map<string, GmHandler> gmhandlers;

int register_handler(string str, GmHandler handler)
{
    gmhandlers[str] = handler;
    return 0;
}

bool gmCommand(Role* role, const char* cmd)
{
	bool bet = false;
    if (Process::env.getInt("gm_mode") != 1){
        return bet;
    }

    do{
        vector<string> params = StrSpilt(cmd, " ");
        if (params.size() < 1){
            break;
        }

        map<string, GmHandler>::iterator iter = gmhandlers.find(params[0]);
        if (iter == gmhandlers.end()){
            break;
        }

        GmHandler handler = iter->second;
        params.erase(params.begin());
        handler(role, params);
		bet = true;
    }
    while (0);

    return bet;
}

#define GM_FUNC(cmd) \
    void gm##cmd(Role* role, vector<string>& params);\
    int cmd##ret = register_handler(#cmd, gm##cmd); \
    void gm##cmd


GM_FUNC(Fat)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    role->addFat(atoi(params[0].c_str()));
}

GM_FUNC(Lvl)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }

    int lvl = atoi(params[0].c_str());
    check_max(lvl, RoleCfg::getMaxRoleLvl());
    check_min(lvl, 1);
	
	role->setLvl(1);
	role->setExp(0);

	int expSum = 0;
	for (int i = 1; i < lvl; i++)
	{
		RoleCfgDef def = RoleCfg::getCfg(role->getJob(), i);
		expSum += def.getExp();
	}
	role->addExp(expSum, "gm_cmd");
}


GM_FUNC(Exp)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    role->addExp(atoi(params[0].c_str()), "gm_cmd");
}

GM_FUNC(Gold)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }

    role->addGold(atoi(params[0].c_str()), "GM");
}

GM_FUNC(Rmb)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }

    role->addRmb(atoi(params[0].c_str()), "gm命令");
}

GM_FUNC(Batpoint)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }

    role->addBattlePoint(atoi(params[0].c_str()), "gm_cmd");
}


GM_FUNC(Constell)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }

    role->addConstellVal(atoi(params[0].c_str()), "gm_cmd");
}


GM_FUNC(Item)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }

    int count = 1;
    int item = atoi(params[0].c_str());
    if (params.size() > 1)
    {
        count = atoi(params[1].c_str());
    }

    GridArray grids;

    ItemArray items;
    items.push_back(ItemGroup(item, count));

    role->preAddItems(items, grids);

//    role->updateBackBag(grids, items, true, "gm_cmd");
    role->playerAddItemsAndStore(grids, items, "gm_cmd", true);
}

GM_FUNC(Copy)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }

    int copyid = atoi(params[0].c_str());
    role->SaveLastCopy(copyid);
}

GM_FUNC(Vipexp)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }

    role->addVipExp((atoi(params[0].c_str())), "gm命令");
}


GM_FUNC(Mail)(Role* role, vector<string>& params)
{
    if (params.size() < 3)
    {
        return;
    }
    
    string attach;
    if (params.size() > 3)
    {
        attach = StrJoin(params.begin() + 3, params.end(), " ");
    }

    sendMail(0, params[0].c_str(), role->getRolename().c_str(), params[1].c_str(),
        params[2].c_str(), attach.c_str(), Utils::itoa(role->getInstID()).c_str());
}

GM_FUNC(EnchantDust)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    
    role->addEnchantDust(atoi(params[0].c_str()), "gm命令");
}

GM_FUNC(EnchantGold)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    
    role->addEnchantGold(atoi(params[0].c_str()), "gm命令");
}


#include "cmd_def.h"

bool finishQuest(Role* role, int questId)
{
    ack_finishQuest ack;
    ack.errorcode = 1;
    ack.questid = questId;
    
    Quest* quest = QuestMgr::FindQuest(questId);
    if (quest == NULL) {
        role->mQuestProgress.erase(questId);
        role->send(&ack);
        //SendDailyQuest(role);
        create_cmd(dailyquest, dailyquest);
        dailyquest->roleid = role->getInstID();
        sendMessageToGame(0, CMD_MSG, dailyquest, 0);
        return false;
    }
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
        ack.errorcode = -1;
        role->send(&ack);
        return false;
    }
    
    role->finshQuest(questId);
    doRedisCmd("hdel questProgress:%d %d",role->getInstID(),questId);
    
    if (quest->type != qtDailyQuest) {
        role->mFinishQuestIds.push_back(questId);
        doRedisCmd("sadd questFinish:%d %d",role->getInstID(),questId);
    }
    
    char buf[32] = "";
    snprintf(buf, sizeof(buf), "finis_quest:%d", questId);
    
    role->playerAddItemsAndStore(effgrids, items, buf, true);
    role->addExp(reward.reward_exp, buf);
    role->addGold(reward.reward_gold, buf);
    role->addBattlePoint(reward.reward_batpoint,buf);
    role->addConstellVal(reward.reward_consval, buf);
    role->addEnchantDust(reward.reward_enchantdust, buf);
    role->addEnchantGold(reward.reward_enchantgold, buf);
    
    role->addPetExp(role->getActivePetId(), quest->pet_exp, buf);
    ack.errorcode = 0;
    role->send(&ack);
    
    //SendDailyQuest(role);
    create_cmd(dailyquest, dailyquest);
    dailyquest->roleid = role->getInstID();
    sendMessageToGame(0, CMD_MSG, dailyquest, 0);

    return true;
}


GM_FUNC(Quest)(Role* role, vector<string>& params)
{
    vector<int> questIds;
    for (std::map<int, std::vector<int> >::iterator iter = role->mQuestProgress.begin();
         iter != role->mQuestProgress.end(); iter++) {
        questIds.push_back(iter->first);
    }
    
    for (int i = 0, len = questIds.size(); i < len; i++) {
        finishQuest(role, questIds[i]);
    }
    
    if (params.size() == 0) {
        return;
    }
    
    int lvl = atoi(params[0].c_str());
    int maxLvl = RoleCfg::getMaxRoleLvl();
    lvl = check_max(lvl, maxLvl);
    for (int i = 1; i < lvl; i++){
        list<int> quests;
        list<int> finishedquests;
        for (vector<int>::iterator iter = role->mFinishQuestIds.begin();  iter != role->mFinishQuestIds.end(); ++iter){
            finishedquests.push_back(*iter);
        }
        QuestMgr::GetQuests(i, finishedquests, NULL, &quests);
        
        for (list<int>::iterator iter = quests.begin(); iter != quests.end(); ++iter){
            doRedisCmd("sadd questFinish:%d %d",role->getInstID(), *iter);
        }
    }
}

GM_FUNC(UpgEquip)(Role* role, vector<string>& params)
{
    PlayerEquip* playerequip = role->getPlayerEquip();
    ItemArray& equips = playerequip->GetItems();
    
    for (int i = 0; i < equips.size(); i++){
        if (equips[i].item <= 0){
            continue;
        }
        
        Json::Reader reader;
        Json::Value value;
        Json::FastWriter writer;
        if (!reader.parse(equips[i].json, value)){
            break;
        }
        
        value["star"] = role->getLvl();
        equips[i].json = writer.write(value);
        PlayerEquip::UpdatePlayerEquip(role->getInstID(), i, equips[i]);
    }
}

GM_FUNC(Petexp)(Role* role, vector<string>& params)
{
    role->addPetExp(role->mActivePetId, 0x3FFFFFF, "gm_command");
}


GM_FUNC(Petgrowth)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    
    Pet* pet = role->mPetMgr->getActivePet();;
    if (pet)
    {
        pet->setGrowth( atoi(params[0].c_str()) );
        pet->save();
    }
}


GM_FUNC(Petstar)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    
    Pet* pet = role->mPetMgr->getActivePet();
    if (pet)
    {
        pet->starlvl = atoi(params[0].c_str());
        pet->saveStar();
    }
}

GM_FUNC(Skillfull)(Role* role, vector<string>& params)
{
    for (int i = 1; i < MAX_SKILL_COUNT + 1; i++) {
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

GM_FUNC(LastFinishCopy)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    
    int copy = atoi(params[0].c_str());
    
    role->setLastFinishCopy(copy);
}

GM_FUNC(Freshrole)(Role* role, vector<string>& params)
{
    
    role->saveProp(GetRolePropName(eRolePropCrossDay), "0");
    role->RoleResetPvp();
    
    role->getTowerDefenseAct()->actFresh();
    role->getFriendDungeAct()->actFresh();
    role->getPrintCopyAct()->actFresh();
    
    role->setRealPvpFreshMatchTimes(0);
    role->setRealPvpMatchTimes(0);
    role->NewRoleProperty::save();
}

GM_FUNC(FreshWeekly)(Role* role, vector<string>& params)
{
	time_t now = time(NULL);
    struct tm nowtm;
    localtime_r(&now, &nowtm);
	struct tm nextdaytm = nowtm;
	nextdaytm.tm_hour = 0;
	nextdaytm.tm_min = nextdaytm.tm_sec = 0;
	
	int days = (7 - nowtm.tm_wday) % 7;
	time_t nextdayt = mktime(&nextdaytm) + 3600 * 24 + ( role->getInstID() % 100);

	time_t nextweekt = nextdayt + days * 3600 * 24;
    role->weeklyRefresh(nextweekt);
	role->saveNewProperty();
}

GM_FUNC(Constrib)(Role* role, vector<string>& params)
{
	if (params.size() < 1)
    {
        return;
    }
	RoleGuildProperty& prop = SRoleGuild(role->getInstID());

    prop.addConstrib(atoi(params[0].c_str()) , "GM命令");
}

GM_FUNC(Guildfort)(Role* role, vector<string>& params)
{
	if (params.size() < 1)
    {
        return;
    }
    
    Guild& guild = SGuildMgr.getRoleGuild(role->getInstID());
    guild.addFortuneConstruction(atoi(params[0].c_str()), 0,
                                 role->getRolename().c_str(),"GM命令");
}

GM_FUNC(Guildcons)(Role* role, vector<string>& params)
{
	if (params.size() < 1)
    {
        return;
    }

    Guild& guild = SGuildMgr.getRoleGuild(role->getInstID());
    guild.addFortuneConstruction(0, atoi(params[0].c_str()),
                                 role->getRolename().c_str(),"GM命令");
}

GM_FUNC(Petpvppoints)(Role* role, vector<string>& params)
{
    if(params.size() < 1)
    {
        return;
    }
    int roleid = role->getInstID();
    int addpoint = Utils::safe_atoi(params[1].c_str());
    SPetPvpMgr.safe_gmAddPetPvpPoints(roleid, addpoint);
}

GM_FUNC(Petpvpcups)(Role* role, vector<string>& params)
{
    if(params.size() < 1)
    {
        return;
    }
    int roleid = role->getInstID();
    int setcups = Utils::safe_atoi(params[1].c_str());
    SPetPvpMgr.safe_gmSetPetPvpCups(roleid, setcups);
}


GM_FUNC(Prestige)(Role* role, vector<string>& params)
{
	if (params.size() < 1)
    {
        return;
    }
	role->getHonorMgr()->addPrestige(atoi(params[0].c_str()));
}

GM_FUNC(Clearbag)(Role* role, vector<string>& params)
{

    string name = role->getBackBag()->GetName();
    doRedisCmd("del %s:%d", name.c_str(), role->getInstID());
}

GM_FUNC(ClearAnswer)(Role* role, vector<string>& params)
{
    role->setAnswerActivityDaySw(0);
    role->saveNewProperty();
}

GM_FUNC(Dayadd)(Role* role, vector<string>& params)
{
    int day = atoi(params[1].c_str());
    if (day >= 5)
    {
        day = 5;
    }
    role->setAnswerContinueFunNum(day);
    role->saveNewProperty();
}

GM_FUNC(Clearday)(Role* role, vector<string>& params)
{
    role->setAnswerContinueFunNum(0);
    role->setAnswerContinueFunNumReward(0);
    role->saveNewProperty();
}

// 刷新公会
GM_FUNC(Freshguild)(Role* role, vector<string>& params)
{
    Guild& guild = SGuildMgr.getRoleGuild(role->getInstID());
    if( ! guild.isNull())
        SGuildMgr.freshGuild(guild);
}

GM_FUNC(Addrealpvpmedal)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    
    int addCount = Utils::safe_atoi(params[1].c_str());
    
    g_RealpvpMgr.safe_addMedal(role->getInstID(), addCount);
    
}

GM_FUNC(Realpvpwin)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    
    int addCount = Utils::safe_atoi(params[1].c_str() );
    
    g_RealpvpMgr.safe_gmAddWins(role->getInstID(), addCount);
    
}

GM_FUNC(Rtoy)(Role* role, vector<string>& params)
{    
    role->getCapsuletoyMgr()->resetData();
    SRoleMgr.disconnect(role);
}




extern int parseFashionType(const string& typeStr);
GM_FUNC(AddFashion)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    
    int fashionId = Utils::safe_atoi(params[0].c_str());
    ItemCfgDef* def = ItemCfg::getCfg(fashionId);
    if (def == NULL)
    {
        return;
    }
    Wardrobe* wardrobe = role->getWardrobe();
    wardrobe->addFashion(def);
    
    string bodyPart = def->ReadStr("body_part");
    int type = parseFashionType(bodyPart);

    switch (type)
    {
        case eWeapon:
            role->setWeaponFashion(fashionId);
            break;
        case eWholeBody:
            role->setBodyFashion(fashionId);
            break;
        case eCityBody:
            role->setHomeFashion(fashionId);
            break;
        default:
            break;
    }
}

extern void gm_cleanRoleData(int roleid);
GM_FUNC(Clmonth)(Role* role, vector<string>& params)
{
    int roldID = role->getId();
    gm_cleanRoleData(roldID);
}

GM_FUNC(Autotest)(Role* role, vector<string>& params)
{
    ack_auto_testtool ack;
	ack.state = Process::env.getInt("gm_mode");
	role->send(&ack);
}

GM_FUNC(Intimacy)(Role* role, vector<string>& params)
{
	if (params.size() < 2)
    {
        return;
    }
    
    struct Traverse : public Traverser
    {
        int mCurIndex;
        int mDestIndex;
        int mIntimacy;
        Role* mRole;
        bool callback(Friend* pFriend)
        {
            if (mCurIndex == mDestIndex) {
                mRole->getFriendMgr()->procAddIntimacyIncr(mRole, pFriend->mFid, mIntimacy);
                return false;
            }
            mDestIndex++;
            return true;
        }
    };
    
    Traverse helper;
    helper.mRole = role;
    helper.mCurIndex = 1;
    helper.mDestIndex = Utils::safe_atoi(params[0].c_str());
    helper.mIntimacy = Utils::safe_atoi(params[1].c_str());
    role->getFriendMgr()->traverse(&helper);
	
    /*
	map<int, Friend>& fs = role->getFriendMgr()->getFriends();
	map<int, Friend>::iterator it;
	int i = 1;
	for (it = fs.begin(); it != fs.end(); it++, i++)
	{
		if (i == Utils::safe_atoi(params[0].c_str()))
		{
			role->getFriendMgr()->procAddIntimacyIncr(role, it->first, Utils::safe_atoi(params[1].c_str()));
			break;
		}
	}*/
}

GM_FUNC(Constellationfull)(Role* role, vector<string>& params)
{
    string cmd = Utils::makeStr("hmset constellpro:%d", role->getInstID());
    
    if (params.size() < 2)
    {
        return;
    }
    
    int roleid = role->getInstID();
    int conid = Utils::safe_atoi(params[0].c_str());
    int lvl = Utils::safe_atoi(params[1].c_str());
    
    check_max(conid, 410000);
    
    bool setdata = false;
    for(int i = 400001; i <= conid; i++)
    {
        ConstellCfgDef* cfg = ConstellationCfg::getCfg(i);
        if(cfg == NULL)
        {
            continue;
        }
        if( i < conid)
        {
            string data = Utils::makeStr(" %d %d", cfg->consid, cfg->step);
            cmd.append(data.c_str());
        }
        else if(i == conid){
            
            check_max(lvl, cfg->step);
            check_min(lvl, 1);
            
            string data = Utils::makeStr(" %d %d", cfg->consid, lvl);
            cmd.append(data.c_str());
        }
        setdata = true;

    }
    
    if (setdata)
    {
        doRedisCmd("del constellpro:%d", roleid);
        
        SRoleMgr.disconnect(role, eLogout);
        
        doRedisCmd(cmd.c_str());
    }
}
extern void onRoleEnterCity(Role* role, Scene* prevScene);
GM_FUNC(AddRole)(Role* role, vector<string>& params)
{
    for (int i = 0; i < params.size(); i++) {
        string roleDesc = params[i];
        if (roleDesc.find("bat-") == string::npos) {
            continue;
        }
        
        int rank = Utils::safe_atoi(roleDesc.c_str() + 4);
        if (rank == 0) {
            continue;
        }
        
        int roleId = 0;
        if (rank > 0)
        {
            RedisResult result(redisCmd("zrevrange paihang:bat %d %d", rank, rank));
            roleId = result.readHash(0, 0);
        }
        
        if (roleId > 0) {
            Role* newRole = SRoleMgr.createRole(roleId);
            if (newRole) {
                role->getScene()->addRole(newRole);
                
                onRoleEnterCity(newRole, NULL);
            }
        }
    }
}

GM_FUNC(Recharge)(Role* role, vector<string>& params)
{
	if (params.size() < 1)
    {
        return;
    }
	int num = Utils::safe_atoi(params[0].c_str()) ;
	createNewOrderCmd("gm", num, role->getInstID(), "gm");
}

GM_FUNC(Occupy)(Role* role, vector<string>& params)
{
	if (params.size() < 1)
    {
        return;
    }
}

GM_FUNC(DsPoint)(Role* role, vector<string>& params)
{
    if (params.size() < 1)
    {
        return;
    }
    int num = Utils::safe_atoi(params[0].c_str()) ;
    if (role->getDailyScheduleSystem()) {
        role->getDailyScheduleSystem()->setPoint(num);
    }
}

GM_FUNC(DsFresh)(Role* role, vector<string>& params)
{
    if (role->getDailyScheduleSystem()) {
        role->getDailyScheduleSystem()->refresh(time(NULL));
    }
}


GM_FUNC(AddFriend)(Role* role, vector<string>& params)
{
    if (params.size() != 1) {
        return;
    }
    
    int num = Utils::safe_atoi(params[0].c_str());
    if (num <= 0 || num >= 100) {
        return;
    }
    SRoleMgr.safe_gmAddTop50Person(role->getId(), num);
    
}

GM_FUNC(Pvppoints)(Role* role, vector<string>& params)
{
    if (params.size() != 1) {
        return;
    }
    
    int num = Utils::safe_atoi(params[0].c_str());
    
    int points = role->getPvpPoints() + num;
    check_min(points, 0);

    role->setPvpPoints(points);
    role->NewRoleProperty::save();
    
}

GM_FUNC(Petskill)(Role* role, vector<string>& params)
{
    Pet* pet = role->mPetMgr->getActivePet();
    
    if(pet == NULL)
    {
        return;
    }
    
    vector<int> lvls;
    lvls.clear();
    lvls.resize(3, 0);
    
    for(int i = 0; i < params.size() && i < lvls.size(); i++)
    {
        int lvl = Utils::safe_atoi(params[i].c_str());
        check_min(lvl, 0);
        
        lvls[i] = lvl;
    }
    
    PetCfgDef* petCfg = PetCfg::getCfg(pet->petmod);
    
    if(petCfg == NULL)
    {
        return;
    }
    
    int maxLvl = PetCfg::getMaxCommmonSkills(petCfg->getQua());
    
    vector<int> oldSkill = pet->getSkill().toArray();
    
    pet->getSkill().clear();
    
    for(int i = 0; i < oldSkill.size() && i < lvls.size(); i++)
    {
        int newLvl = lvls[i];
        
        if(newLvl == 0)
        {
            newLvl = maxLvl;
        }
        
        check_max(newLvl, maxLvl);
        
        int newSkillId = getSkillIndex(oldSkill[i]) + newLvl;
        SkillCfgDef* skillcfg = SkillCfg::getCfg(newSkillId);
        
        if(skillcfg)
        {
            pet->getSkill().push_back(newSkillId);
        }
    }
    
    pet->PetNewProperty::save();
}

GM_FUNC(Clearretinuebag)(Role* role, vector<string>& params)
{
    string name = role->getRetinueMgr()->mRetinueBag->GetName();
    doRedisCmd("del %s:%d", name.c_str(), role->getInstID());
}

GM_FUNC(Info)(Role* role, vector<string>& params)
{
	notify_app_cmd notify;
	
	Json::Value value;
	value["testmode"] = Process::env.getInt("testmode");
	value["lang"] = Process::env.getString("lang");
	value["serverId"] = Process::env.getInt("server_id");
	
	notify.data = xyJsonWrite(value);
	role->send(&notify);
}

