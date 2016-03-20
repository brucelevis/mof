//
//  Log.cpp
//  GameSrv
//
//  Created by pireszhi on 13-6-15.
//
//

#include "GameLog.h"
#include "MQ.h"
#include "Game.h"
#include "Utils.h"
#include "main.h"
#include "log_server.h"
#include "SessionData.h"
#include "GuildRole.h"
#include "Role.h"
#include "Paihang.h"
#include "EnumDef.h"

/***********************************************************************************
 
 日志消息定义：
     msg.SetInt(0);     //固定值
     msg.SetInt(1);     //固定值
     msg.SetInt(GM_LOG);    //发到log server的消息号
     
     msg.SetString("Login");    //这里设定txt的文件名，同时如果存到mysql，这同时也是相对应的表的名字
     msg.SetString(now.c_str()); //必须设定时间，以兼容mysql表的格式
     ...    //后面是日志中的相应列
 **********************************************************************************/
namespace LogMod {
    
    void addLoginLog(int roleid, const char* rolename, int sessionid, int rolesCount)
    {
		int lvl = 0;
		int job = 0;
		int gold = 0;
		int rmb = 0;
		string ip = "";
        Role*  role = SRoleMgr.GetRole(roleid);
        if (role)
        {
            lvl = role->getLvl();
            job = role->getJob();
            gold = role->getGold();
            rmb = role->getRmb();
			ip = SSessionDataMgr.getProp(sessionid, "ip");
        }

		Xylog log(eLogName_Login, roleid);
		log << SRoleMgr.getRoleNum() << lvl << job << gold << rmb <<ip;
    }

    void addLogLogout(int roleid, const char* rolename, int sessionid, int timeonline, int logoutType)
    {
		int lvl = 0;
		int job = 0;
		int gold = 0;
		int rmb = 0;
		Role* role = SRoleMgr.GetRole(roleid);
        if (role)
        {
            lvl = role->getLvl();
            job = role->getJob();
            gold = role->getGold();
            rmb = role->getRmb();
        }
		
		Xylog log(eLogName_Logout, roleid);
		log << timeonline << logoutType << lvl << job << gold << rmb;
    }


    void addLogPrivateChat(int senderid, const char* senderName, int sessionid, int recvid, const char* recvName, const char* recvAccount, const char* content)
    {
        string senderAccount = SSessionDataMgr.getProp(sessionid, "account");
        string playerPlatform = SSessionDataMgr.getProp(sessionid, "platform");
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str());
        
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("PrivateChat");
        msg.SetString(now.c_str());
        msg.SetInt(senderid);
        msg.SetString(senderName);
        msg.SetString(senderAccount.c_str());
        msg.SetString(playerPlatform.c_str());
        
        msg.SetInt(recvid);
        msg.SetString(recvName);
        msg.SetString(recvAccount);
        msg.SetString(content);

        sendLogMsg(GameLog::MQ, msg);
        
    }

    void addLogWorldChat(int senderid, const char* senderName, int sessionid, const char* content)
    {
        string senderAccount = SSessionDataMgr.getProp(sessionid, "account");
        string playerPlatform = SSessionDataMgr.getProp(sessionid, "platform");
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str());
        
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("WorldChat");
        msg.SetString(now.c_str());
        msg.SetInt(senderid);
        msg.SetString(senderName);
        msg.SetString(senderAccount.c_str());
        msg.SetString(playerPlatform.c_str());
        msg.SetString(content);
        
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogGetGold(int roleid, const char* rolename, int sessionid, int getCount , const char* getfrom, int total)
    {
		Xylog log(eLogName_GetGold, roleid);
		log << getCount << total << getfrom;
    }
    
    void addLogConsumGold(int roleid, const char* rolename, int sessionid, int consumCount, const char* usage, int total )
    {
		Xylog log(eLogName_ConsumeGold, roleid);
		log << consumCount << usage << total;
    }
    
    //rmb log
    void addLogGetRmb(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total, int lvl, int atk)
    {
		Xylog log(eLogName_GetRmb, roleid);
		log << getCount << total << getfrom << lvl << atk;
    }
    //重载
    void addLogGetRmb(int roleid, const char* rolename, const char* account, const char* platform, int getCount, const char* getfrom, int total, int lvl, int atk)
    {
		Xylog log(eLogName_GetRmb, roleid);
		log << getCount << total << getfrom << lvl << atk;
    }
    
    void addLogConsumRmb(int roleid, const char* rolename, int sessionid, int consumCount, const char* usage, int total, int lvl, int vipLvl, int vipExp, int atk)
    {
		Xylog log(eLogName_ConsumeRmb, roleid);
		log << consumCount << usage << total << lvl << vipLvl << vipExp << atk;
    }
    
    void addLogGetBattlePoint(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total)
    {
		Xylog log(eLogName_GetBattlePoint, roleid);
		log << getCount << total << getfrom;
    }
    
    void addLogConsumBattlePoint(int roleid, const char* rolename, int sessionid, int consumCount, const char* usage, int total)
    {	
		Xylog log(eLogName_ConsumeBattlePoint, roleid);
		log << consumCount << usage << total;
    }

    void addLogGetexp(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total)
    {
		Xylog log(eLogName_Getexp, roleid);
		log << getCount << total << getfrom;
    }
    
    void addLogGetConstellVal(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total)
    {
		Xylog log(eLogName_GetConstellVal, roleid);
		log << getCount << total << getfrom;
    }
    
    void addLogGetEnchantDust(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total)
    {
        Xylog log(eLogName_GetEnchantDust, roleid);
        log << getCount << total << getfrom;
    }
    
    void addLogGetEnchantGold(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total)
    {
        Xylog log(eLogName_GetEnchantGold, roleid);
        log << getCount << total << getfrom;
    }
    
    void addLogUpgradeEquip(int roleid, const char* rolename, int sessionid, int equipid, int newlvl)
    {
		Xylog log(eLogName_RoleUpGradeEquip, roleid);
		log << equipid << newlvl;
    }
    
    void addLogGetinPvp(int roleid, const char* rolename, int sessionid, int myBattleForce, int challengeid, int enemyBattleForce, int battleTime, int result, int prerank, int newrank)
    {
		Xylog log(eLogName_RoleGetinPvp, roleid);
		log << myBattleForce << challengeid << enemyBattleForce << battleTime << result
		<< prerank << newrank;
    }
    
    void addLogUpgradeSkill(int roleid, int skillid, const char* skillname)
    {
        Xylog log(eLogName_StudySkill, roleid);
		log<<skillname << skillid;
    }
    
    void addLogClientCheat(int roleid, const char* rolename, int sessionid, const char* kickInfo, const char* cheatinfo)
    {
        string playerAccoount = SSessionDataMgr.getProp(sessionid, "account");
        string playerPlatform = SSessionDataMgr.getProp(sessionid, "platform");
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str());
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("ClientCheat");
        msg.SetString(now.c_str());
        msg.SetInt(roleid);
        msg.SetString(rolename);
        msg.SetString(playerAccoount.c_str());
        msg.SetString(playerPlatform.c_str());
        msg.SetString(kickInfo);
        msg.SetString(cheatinfo);
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogAppleRecharge(int roleid,int reqroleid,int rmb ,const char* transaction,const char* apple_receipt)
    {
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str());
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("AppleRecharge");
        msg.SetString(now.c_str());
        msg.SetInt(roleid);
        msg.SetInt(reqroleid);
        msg.SetInt(rmb);
        msg.SetString(transaction);
        msg.SetString(apple_receipt);
        sendLogMsg(GameLog::MQ, msg);
    }
    
    //vipexp log
    void addLogGetVipexp(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total, int viplvl)
    {
		Xylog log(eLogName_GetVipexp, roleid);
		log << getCount << total << getfrom << viplvl;
    }
    
    void addLogPetProChange(int owner,int petid,const char* petname,const char* action,const char* before,const char* change,const char* after,const char* extra)
    {
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str()) ;
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("PetProChange");
        
        msg.SetString(now.c_str());
        msg.SetInt(owner);
        msg.SetInt(petid);
        msg.SetString(petname);
        msg.SetString(action);
        msg.SetString(before);
        msg.SetString(change);
        msg.SetString(after);
        msg.SetString(extra);
        
        sendLogMsg(GameLog::MQ, msg);
    }
    
    //行为操作
    void addLogGetItem(Role* role, const char* rolename, int sessionid, ItemArray items, const char* getfrom)
    {
        for (int i = 0; i < items.size(); i++)
		{
            int remainCount = role->getBackBag()->GetItemNum(items[i].item);
			Xylog log(eLogName_GetItems, role->getInstID());
			log << items[i].item << items[i].count << getfrom << remainCount;
        }
    }
    
    void addLogComposeItem(int roleid, const char* rolename, int sessionid,const char* consumitems, const char* getitems)
    {
		Xylog log(eLogName_ComposeItems, roleid);
		log << consumitems << getitems;
    }
    
    void addLogGetPet(int roleid, const char* rolename, int sessionid, int petTmpId, int petid, const char* getfrom)
    {
		Xylog log(eLogName_GetPet, roleid);
		log << petTmpId << petid << getfrom;
    }
    
    void addLogConstellationUpgrade(int roleid, const char* rolename, int sessionid, int constellid, int prestep, int newstep, int consum, int total)
    {
        string playerAccoount = SSessionDataMgr.getProp(sessionid, "account");
        string playerPlatform = SSessionDataMgr.getProp(sessionid, "platform");
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str()) ;
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("ConstellationUpgrade");
        msg.SetString(now.c_str());
        msg.SetInt(roleid);
        msg.SetString(rolename);
        msg.SetString(playerAccoount.c_str());
        msg.SetString(playerPlatform.c_str());
        
        msg.SetInt(constellid);
        msg.SetInt(prestep);
        msg.SetInt(newstep);
        msg.SetInt(consum);
        msg.SetInt(total);
        
//        SPSMgr.sendMessage(eLogServer, msg);
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogDelRole(int roleid, const char* rolename, int sessionid, int lvl, int viplvl, int vipexp)
    {
        string playerAccoount = SSessionDataMgr.getProp(sessionid, "account");
        string playerPlatform = SSessionDataMgr.getProp(sessionid, "platform");
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str()) ;
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("DelRole");
        msg.SetString(now.c_str());
        msg.SetInt(roleid);
        msg.SetString(rolename);
        msg.SetString(playerAccoount.c_str());
        msg.SetString(playerPlatform.c_str());
        
        msg.SetInt(lvl);
        msg.SetInt(viplvl);
        msg.SetInt(vipexp);
        
//        SPSMgr.sendMessage(eLogServer, msg);
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogCreateRole(int roleid, const char* rolename, int sessionid)
    {
		string playerAccoount = SSessionDataMgr.getProp(sessionid, "account");
        string playerPlatform = SSessionDataMgr.getProp(sessionid, "platform");
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str()) ;
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("CreateRole");
        msg.SetString(now.c_str());
        msg.SetInt(roleid);
        msg.SetString(rolename);
        msg.SetString(playerAccoount.c_str());
        msg.SetString(playerPlatform.c_str());
		msg.SetString(SSessionDataMgr.getProp(sessionid, "ip").c_str());
		
        Role* role = SRoleMgr.GetRole(roleid);
        if (role)
        {
            msg.SetInt(role->getLvl());
            msg.SetInt(role->getJob());
            msg.SetInt(role->getGold());
            msg.SetInt(role->getRmb());
        }
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogPetPvpPointsChange(int roleid, const char* rolename, int sessionid, int changePoints, int total, const char* operation)
    {
        string playerAccoount = SSessionDataMgr.getProp(sessionid, "account");
        string playerPlatform = SSessionDataMgr.getProp(sessionid, "platform");
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str()) ;
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("PetpvpPoints");
        msg.SetString(now.c_str());
        msg.SetInt(roleid);
        msg.SetString(rolename);
        msg.SetString(playerAccoount.c_str());
        msg.SetString(playerPlatform.c_str());
        
        msg.SetInt(changePoints);
        msg.SetInt(total);
        msg.SetString(operation);
        
        //        SPSMgr.sendMessage(eLogServer, msg);
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogPetPvpPointsChange(int roleid, const char* rolename, const char* account, int changePoints, int total, const char* operation)
    {
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str()) ;
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("PetpvpPoints");
        msg.SetString(now.c_str());
        msg.SetInt(roleid);
        msg.SetString(rolename);
        msg.SetString(account);
        msg.SetString("NULL");
        
        msg.SetInt(changePoints);
        msg.SetInt(total);
        msg.SetString(operation);
        
        //        SPSMgr.sendMessage(eLogServer, msg);
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogPetPvpCupsChange(int roleid, const char* roleName, int preTotalCups, int changeCups, int totalCups, const char* pvpType, const char* from)
    {
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str()) ;
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("PetpvpCups");
        msg.SetString(now.c_str());
        msg.SetInt(roleid);
        msg.SetString(roleName);
        msg.SetInt(preTotalCups);
        msg.SetInt(changeCups);
        msg.SetInt(totalCups);
        msg.SetString(pvpType);
        msg.SetString(from);
        
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogPetCampPassRecord(int roleid, int copyid, int passtime, int mysticalBossid)
    {
        Xylog log(eLogName_PetCampPassRecord,roleid);
        log<<copyid<<passtime<<mysticalBossid;
    }
    
    void addLogCompatPowerError(int roleid, int copyid, int passtime)
    {
        Xylog log(eLogName_CompatPowerError,roleid);
        log<<copyid<<passtime;
    }
    
    void addLogEnterCopy(int roleid, int copyid, int copyType)
    {
        Xylog log(eLogName_EnterCopy,roleid);
        log<<copyid << copyType;
    }
    
    void addLogFinishCopy(Role* role, int copyid, bool isPass, string award, int copyType)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_FinishCopy,role->getInstID());
        log<<role->getVipExp()<<role->getVipLvl()<<copyid<<isPass<<award << copyType;
    }
    
    void addLogSendRoleMail(int roleid, const char* title, const char* attach, bool sendSuccess)
    {
        Xylog log(eLogName_SendRoleMail,roleid);
        log<<title<<attach<<sendSuccess;
    }
    
    void addLogFinishDungeException(Role* role, int sceneid, int costSec)
    {
        if (role == NULL) {
            return;
        }
        
        SceneCfgDef* scene = SceneCfg::getCfg(sceneid);
        if (scene == NULL) {
            return;
        }
        int petBattle = 0;
        int petid = 0;
        Pet* pet = role->mPetMgr->getActivePet();
        if (pet) {
            petBattle = pet->mBattle;
            petid = pet->petid;
        }
        int testBattle = role->getBattleForce() + petBattle * 0.35;
        
        if ( testBattle > (scene->fightpoint * 0.7)) {
            return;
        }
        
        Xylog log(eLogNmae_FinishDungeException,role->getInstID());
        log<<role->getBattleForce()<<petid<<petBattle<<costSec<<sceneid;
    }
    
    void addLogTowerDefensePass(Role* role, int waves, int costSec, int getPoints, int friends, int sceneid)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_TowerDefense_Pass_Condition, role->getInstID());
        log<<role->getBattleForce()<<waves<<costSec<<getPoints<<friends<<sceneid;
    }
    
    void addUseItemLog(Role *role, const ItemArray& items, const string &description)
    {
        Xylog ulog(eLogName_UseItem, role->getInstID());
        ulog << description;
        
        ItemArray merged;
        
        mergeItems(items, merged);
        
        for (int i = 0; i < items.size(); ++i)
        {
            int remainCount = role->getBackBag()->GetItemNum(merged[i].item);
            ulog << merged[i].item << merged[i].count<<remainCount;
        }
    }
    
    void addLogSynPvpMedal(int roleId, const char* roleName, int getmedal, int total, string getfrom)
    {
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str()) ;
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("SynpvpMedal");
        
        msg.SetString(now.c_str());
        msg.SetInt(roleId);
        msg.SetString(roleName);
        msg.SetInt(getmedal);
        msg.SetInt(total);
        msg.SetString(getfrom.c_str());
        
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogLimitTimeKill(Role* role, int useTime, int getscore)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_TimeLimitKillCopy, role->getInstID());
        log<<useTime<<getscore;
    }
    
    void addLogTowerDefenseAward(Role* role, int waves, int friendcount, int friendIntimacySum, string award, int sceneid)
    {
        if (role == NULL) {
            return;
        }
        Xylog log(eLogName_TowerDefenseAward, role->getInstID());
        log<<waves<<friendcount<<friendIntimacySum<<award<<sceneid;
    }
    
    void addLogAddFashion(Role* role, int fashionId, int fashionType, int expireTime)
    {
        if (role == NULL)
        {
            return;
        }
        
        Xylog log(eLogName_AddFashion, role->getInstID());
        log << fashionId << fashionType << expireTime;
    }
	
	void addSdkLoginLog(const char *loginMode, const char *url, const char *send, const char *ret, int result)
	{
		Xylog log(eLogName_SdkLoginLog, 0);
		log
		<< loginMode
		<< url
		<< send
		<< ret
		<< result;
	}
    
    void addLogGetFamesHallAward(Role* role, int level, string award)
    {
        Xylog log(eLogName_FamesHallAward, role->getInstID());
        
        log<<level;         //通关的层次
        log<<award;         //奖励
    }
    
    void addLogFinishFriendDunge(Role* role, int index, int level)
    {
        Xylog log(eLogName_FinishFriendDunge, role->getInstID());

        log<<index<<level;
    }
    
    void addLogFinishDunge(Role* role, int index, int level)
    {
        Xylog log(eLogName_FinishDunge, role->getInstID());
        log<<index<<level;
    }
    
    void addLogFinishEliteCopy(Role* role, int sceneid)
    {
        Xylog log(eLogName_FinishEliteCopy, role->getInstID());

        log << sceneid << (sceneid / 1000) % 10;

    }
    
    void addLogFinishPetElite(Role* role, int sceneid)
    {
        Xylog log(eLogName_FinishPetElite, role->getInstID());
        log<<sceneid;
    }
    
    void addLogSkillHurtRecord(Role* role, int skillid, int effectIndex, int damage)
    {
        Xylog log(eLogName_SkillHurtRecord, role->getInstID());
        log<<skillid<<effectIndex<<damage;
    }
    
    void addLogPetEvolutionInfo(Role* role, string leftPetInfo, string rightPetInfo, string newPetInfo)
    {
        Xylog log(eLogName_PetEvolution, role->getInstID());
        log<<leftPetInfo<<rightPetInfo<<newPetInfo;
    }
	
	void addBattleLog(Xylog &log, const char *desc, BaseProp &base, BattleProp &bat)
	{
		log << desc
		<< xystr("%d,%d,%d,%d", base.getStre(), base.getInte(), base.getPhys(), base.getCapa())
		<< xystr("%d,%d,%d,%d,%d,%f,%f,%f,%d,%d",
				 bat.getHp(), bat.getMaxHp(), bat.getMaxMp(), bat.getAtk(),
				 bat.getDef(), bat.getHit(), bat.getDodge(), bat.getCri(),
				 bat.getMoveSpeed(), bat.getHpIncr());
	}
    
    void addLogWeaponEnchantErr(Role* role, int enchantid, int enchantlvl, string details)
    {
        Xylog log(eLogName_WeaponEnchantErr, role->getInstID());
        log<<enchantid<<enchantlvl<<details;
    }
    
    void addLogFinishPetAdventureCopy(Role* role, int copyid, bool isPass, string award, int copyType)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_PetAdventureCopy,role->getInstID());
        log << copyid << isPass << award << copyType;
    }
    
    void addLogWeaponEnchantUpgrade(Role* role, int enchantid, int preLvl, int newLvl, int preExp, int newExp, string details)
    {
        Xylog log(eLogName_WeaponEnchantUpgrade, role->getInstID());
        log << enchantid << preLvl << newLvl << preExp << newExp << details;
    }
    
    void addLogPetAdventureBufOnPet(Role* role, int copyid, int copyType, string szLogbeforeAddPetBuf, string szLogAfterAddPetBuf)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_PetAdventureBufOnPet,role->getInstID());
        
        log << copyid << copyType << szLogbeforeAddPetBuf << szLogAfterAddPetBuf;
        
    }
    
    void addLogPetAdventureCopyBuyTimes(Role* role, int buytimes, int cost, int costtype, string buytimeschange, string buyTimesRecordPerDay, string petAdventureTimes, bool buyissucceeded)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_PetAdventureCopyBuyTimes,role->getInstID());
        
        log << buytimes << cost << costtype << buytimeschange << buyTimesRecordPerDay << petAdventureTimes << buyissucceeded;
        
    }
    
    void addLogPetAdventureRobotPetRentTimes(Role* role, int renttimes, int cost, int costtype, string szLogRobotPetRentBuyingTimesRecordPerDay, string szLogRobotPetFreeRentTimesRemain, string szLogRobotPetBuyingRentTimesRemain, string szLogRobotPetRentTimesRemain, bool rentissucceeded)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_PetAdventureRobotPetRentTimes,role->getInstID());
        
        log << renttimes << cost << costtype << role->getVipLvl()
            << szLogRobotPetRentBuyingTimesRecordPerDay
            << szLogRobotPetFreeRentTimesRemain
            << szLogRobotPetBuyingRentTimesRemain
            << szLogRobotPetRentTimesRemain
            << rentissucceeded;
        
    }
    
    void addLogDungeonSweeping(Role* role, int beginsceneid, int endsceneId, string award, bool sweeped)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_DungeonSweeping,role->getInstID());
        
        log << beginsceneid << endsceneId << award << sweeped;
        
    }
    
    void addLogMysticalCopyTimesAndIncrTimes(Role* role, string IncrTimesChange, string copyTimesChange, bool bossAppear, bool monsterAppear)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_MysticalCopyTimesAndIncrTimes,role->getInstID());
        
        log << IncrTimesChange << copyTimesChange << bossAppear << monsterAppear;
        
    }

    void addLogFashionCollectPutOnMaterial(Role* role, int collectProgressId, int materialid, int grid, int count, string alldMaterial)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_FashionCollectPutOnMaterial,role->getInstID());
        
        log<<collectProgressId<<materialid<<count<<alldMaterial;
    }
    
    void addLogFashionCollectFinishProgress(Role* role, int progressId, int way, int buyFashionId)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_FashionCollectFinish,role->getInstID());
        
        //way = 1 代表是通过购买时装完成, buyFashionId = fashionid,=0代表通过正常收集过程完成,buyFashionId = 0
        log<<progressId<<way<<buyFashionId;
    }
    
    void addLogFashionCollectMaterialRecycle(Role* role, int materialId, int count, int recycleLvl, int recycleExp)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_FashionCollectMaterialRecycle,role->getInstID());
        
        log<<materialId<<count<<recycleLvl<<recycleExp;
    }
    
    
    //同步战斗日志
    void addLogChangeSpeed(int sceneMod, int sceneId, int roleId, const char* rolename,
                           float changePer, int changeVal, float curPer, int curVal)
    {
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str()) ;
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("ChangeSpeed");
        
        msg.SetString(now.c_str());
        msg.SetInt(sceneMod);
        msg.SetInt(sceneId);
        msg.SetInt(roleId);
        msg.SetString(rolename);
        msg.SetFloat(changePer);
        msg.SetInt(changeVal);
        msg.SetFloat(curPer);
        msg.SetInt(curVal);
        
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogActivityEnterTimesChange(int roleid, const char* roleName, int changeType, int itemAddTimes, int rmbAddTimes, int freeTimes, int totalTimes, const char* changeAction)
    {
        string now = Utils::currentDate();
        now.append(" ");
        now.append(Utils::currentTime().c_str()) ;
        CMsgTyped msg;
        msg.SetInt(0);
        msg.SetInt(1);
        msg.SetInt(GM_LOG);
        
        msg.SetString("ActivityEnterTimes");
        
        msg.SetString(now.c_str());
        msg.SetInt(roleid);
        msg.SetString(roleName);
        msg.SetInt(changeType);
        msg.SetInt(itemAddTimes);
        msg.SetInt(rmbAddTimes);
        msg.SetInt(freeTimes);
        msg.SetInt(totalTimes);
        msg.SetString(changeAction);
        
        sendLogMsg(GameLog::MQ, msg);
    }
    
    void addLogDelPetInDB(Role* role, int petId, int petMod, const char* petInfo, const char* describe)
    {
        if (role == NULL) {
            return;
        }
        
        Xylog log(eLogName_DelectPetFromDB, role->getInstID());
        log<<petId<<petMod<<petInfo<<describe;
    }
    
    void addLogGetMailAttach(Role* role, const char* mailId, const char* mailTitle, const char* mailAttach)
    {
        Xylog log(eLogName_GetMailAttach, role->getInstID());
        int activePet = role->getActivePetId();
        log
        << mailId           //邮件id
        << mailTitle        //邮件标题
        << mailAttach       //附件串
        << activePet;       //出战幻兽
    }
    
    void addLogAddPetExp(Role* role, int petId, int petMod, int preTotalExp, int addExp, int totalExp, int level, const char* comeFrom)
    {
        Xylog log(eLogName_AddPetExp, role->getInstID());
        
        log
        << petId        //宠物id
        << petMod       //模板id
        << preTotalExp  //增加前的总经验
        << addExp       //增加的经验
        << totalExp     //当前总经验
        << level        //等级
        << comeFrom;    //来源
    }
    
    void addLogDuokuSdkLogin(string AccessToken, string uid, int curlcode, string ret, int result)
    {
        
        Xylog log(eLogName_DuokuSdkLogin, 0);
        
        log << AccessToken << uid << curlcode << ret << result;
        
    }
    
    void addLogPetSwallowLockOperate(Role* role, int petid, vector<int> swallowLockStatusChange)
    {
        if (role == NULL) {
            return;
        }
        Pet* pet = role->mPetMgr->getPet(petid);
        if (pet) {
            return;
        }
        
        Xylog log(eLogName_SwallowLockOperate, role->getInstID());

        
        log << petid << pet->petmod;
        
        for (vector<int>::iterator it = swallowLockStatusChange.begin(); it != swallowLockStatusChange.end(); it++) {
            switch (*it) {
                case false:
                    log << *it << "没锁";
                    break;
                case true:
                    log << *it << "锁定";
                    break;
                default:
                    log << "吞噬锁的值有误";
                    break;
            }
        }
    }
    
    void addLogRoleSortData(int sortType, vector<SortInfo*>& input)
    {
        if (sortType == eSortBat) {
            Xylog log(eLogName_RoleBatlleForcePaihang, 0);
            
            int vsize = input.size();
            
            stringstream context;
            
            for (int i = 0; i < 20 && i < vsize; i++) {
                SortInfo* info = input[i];
                
                if (info == NULL) {
                    continue;
                }
                
                context
                << info->battleRank << ","
                << info->roleid << ","
                << info->rolename << ","
                << info->battle << ","
                << info->lvl << ";";
            }
            
            log << context.str();
        }
        else if (sortType == eSortLvl)
        {
            Xylog log(eLogName_RoleLvlPaihang, 0);

            int vsize = input.size();
            
            stringstream context;
            
            for (int i = 0; i < 20 && i < vsize; i++) {
                SortInfo* info = input[i];
                
                if (info == NULL) {
                    continue;
                }
                
                context
                << info->lvlRank << ","
                << info->roleid << ","
                << info->rolename << ","
                << info->lvl << ","
                << info->battle << ";";
            }
            
            log << context.str();
        }
        else if(sortType == eSortPvp)
        {
            Xylog log(eLogName_RolePvpPaihang, 0);
            
            int vsize = input.size();
            
            stringstream context;
            
            for (int i = 0; i < 20 && i < vsize; i++) {
                SortInfo* info = input[i];
                
                if (info == NULL) {
                    continue;
                }
                
                context
                << info->pvpRank << ","
                << info->roleid << ","
                << info->rolename << ","
                << info->battle << ","
                << info->lvl << ";";
            }
            
            log << context.str();
        }
        else if (sortType == eSortRecharge) {
            Xylog log(eLogName_RoleRechargePaihang, 0);
            
            int vsize = input.size();
            
            stringstream context;
            
            for (int i = 0; i < 20 && i < vsize; i++) {
                SortInfo* info = input[i];
                
                if (info == NULL) {
                    continue;
                }
                
                context
                << info->rechargeRank   << ","
                << info->roleid         << ","
                << info->rolename       << ","
                << info->recharge       << ","
                << info->lvl            << ";";
            }
            
            log << context.str();
        }
        else if (sortType == eSortConsume) {
            Xylog log(eLogName_RoleConsumePaihang, 0);
            
            int vsize = input.size();
            
            stringstream context;
            
            for (int i = 0; i < 20 && i < vsize; i++) {
                SortInfo* info = input[i];
                
                if (info == NULL) {
                    continue;
                }
                
                context
                << info->consumeRank    << ","
                << info->roleid         << ","
                << info->rolename       << ","
                << info->consume        << ","
                << info->lvl            << ";";
            }
            
            log << context.str();
        }
    }
    
    void addLogPetBattleForceSortData(vector<SortPetInfo*>& input)
    {
        Xylog log(eLogName_PetBatlleForcePaihang, 0);
        
        int vsize = input.size();
        
        stringstream context;
        
        for (int i = 0; i < 20 && i < vsize; i++) {
            SortPetInfo* info = input[i];
            
            if (info == NULL) {
                continue;
            }
            
            context
            << info->rank << ","
            << info->masterid << ","
            << info->mastername << ","
            << info->petid << ","
            << info->battle << ";";
        }
        
        log << context.str();
    }
    
    void addLogEquipBaptize(int roleId, int equipIndex, int equipId, int baptizeQua, int baptizeIndex, const char* beforeProp, const char* afterProp, const char* operation)
    {
        Xylog log(eLogName_EquipBaptize, roleId);
        
        log
        << equipIndex
        << equipId
        << baptizeQua
        << baptizeIndex
        << beforeProp
        << afterProp
        << operation;
    }
    
    void addLogRetinueSummon(int roleid, int summonId, int summonType, const char* getItems)
    {
        Xylog log(eLogName_RetinueSummon, roleid);
        log
        << summonId
        << summonType
        << getItems;
    }
    
    void addLogRetinueAddExp(int roleid, int retinueId, int addExp, int beforExp, int afterExp, const char* beforeInfo, const char* afterInfo, const char* comsume)
    {
        Xylog log(eLogName_RetinueAddExp, roleid);
        log
        << retinueId
        << addExp
        << beforExp
        << afterExp
        << beforeInfo
        << afterInfo
        << comsume;
    }
    
    void addLogRetinueStarup(int roleid, int retinueId, int beforStar, int afterStar, const char* beforeInfo, const char*afterInfo, const char* comsume)
    {
        Xylog log(eLogName_RetinueStarup, roleid);
        
        log
        << retinueId
        << beforStar
        << afterStar
        << beforeInfo
        << afterInfo
        << comsume;
    }
    
    void addLogRetinueSkillStudy(int roleid, int retinueSkillId, int beforLvl, int afterLvl, const char* comsume)
    {
        Xylog log(eLogName_RetinueSkillStudy, roleid);
        
        log
        << retinueSkillId
        << beforLvl
        << afterLvl
        << comsume;
    }
    
    void addLogPvpPointExchange(int roleid, int comsumePoints, int beforePoints, int afterPoints, const char* getItems)
    {
        Xylog log(eLogName_PvpPointExchange, roleid);
        
        log
        << comsumePoints
        << beforePoints
        << afterPoints
        << getItems;
    }
    
    void addLogPvpPointChange(int roleid, int changePoints, int beforePoints, int afterPoints, const char* usageOrComeFrom)
    {
        Xylog log(eLogName_PvpPointChange, roleid);
        
        log
        << changePoints
        << beforePoints
        << afterPoints
        << usageOrComeFrom;
    }
    
    void addLogRetinueCompose(int roleid, int retinueId, int retinueModid, const char* retinueInfo, const char* comsume)
    {
        Xylog log(eLogName_RetinueCompose, roleid);
        
        log
        << retinueId
        << retinueModid
        << retinueInfo
        << comsume;
    }
    
    
    
    void addWorshipLog(int roleId, int targetId, const char* type, int costRmb)
    {
        Xylog log(eLogName_Worship, roleId);
        log << targetId << type << costRmb;
    }
}

static void xylog(const char* file, const char * data)
{
    //MAKE_VA_STR_DEF(data);
    
    string now = Utils::currentDate();
    now.append(" ");
    now.append(Utils::currentTime().c_str()) ;
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(1);
    msg.SetInt(GM_LOG);
    
    if(file)
        msg.SetString(file);
    else
        msg.SetString("xylog");
    msg.SetString(now.c_str());
    
    msg.SetString(data);
    sendLogMsg(GameLog::MQ, msg);
}

void xyerr(const char * data, ...)
{
    MAKE_VA_STR_DEF(data);

    string now = Utils::currentDate();
    now.append(" ");
    now.append(Utils::currentTime().c_str()) ;
    CMsgTyped msg;
    msg.SetInt(0);
    msg.SetInt(1);
    msg.SetInt(GM_LOG);
    msg.SetString("xyerr");
    msg.SetString(now.c_str());
    msg.SetString(strdata.c_str());
    sendLogMsg(GameLog::MQ, msg);
}


Xylog::Xylog(eLogPropName logname, int roleid, eLogAction action)
{
	_file = GetLogPropName(logname);
	_saved = false;
    _roleid = roleid;
	_logname = logname;
	_action = action;
}

Xylog::~Xylog()
{
    if(! _saved)
	{
		save();
		sendActionMsg();
	}
}

void Xylog::initValueChange()
{
	_change_exp = 0;
	_change_gold = 0;
	_change_batpoint = 0;
	_change_consval = 0;
	_change_rmb = 0;
	_change_constrib = 0;
	_change_exploit = 0;
	_change_fat = 0;
	
	Role* role = SRoleMgr.GetRole(_roleid);
    if(role)
    {
		_change_exp = role->getExp();
		_change_gold = role->getGold();
		_change_batpoint = role->getBattlePoint();
		_change_consval = role->getConstellVal();
		_change_rmb = role->getRmb();
		_change_constrib = SRoleGuild(_roleid).getConstrib();
		_change_exploit = SRoleGuild(_roleid).getExploit();
		_change_fat = role->getFat();
    }
}

void Xylog::checkValueChange()
{
	Role* role = SRoleMgr.GetRole(_roleid);
    if(NULL == role) {
		return;
	}
	
	CMsgTyped msg;
	// logserver 消息串, 以下内容为固定, tb_change_log为mysql日志表名
	msg.SetInt(0);
	msg.SetInt(1);
	msg.SetInt(GM_LOG);
	msg.SetString("tb_change_log");
	msg.SetString("");
	msg.SetString("");
	
	// 内容
	msg.SetInt(time(NULL));
	msg.SetInt(Process::env.getInt("server_id"));
	msg.SetString(SSessionDataMgr.getProp(role->getSessionId(), "account").c_str());
	msg.SetString(SSessionDataMgr.getProp(role->getSessionId(), "platform").c_str());
	msg.SetInt(_roleid);
	msg.SetString(role->getRolename().c_str());
	msg.SetInt(_logname);
	msg.SetInt(_action);
	
	sendValueChange(msg, _change_exp, role->getExp(), "exp");
	sendValueChange(msg, _change_gold, role->getGold(), "gold");
	sendValueChange(msg, _change_batpoint, role->getBattlePoint(), "batpoint");
	sendValueChange(msg, _change_consval, role->getConstellVal(), "consval");
	sendValueChange(msg, _change_rmb, role->getRmb(), "rmb");
	sendValueChange(msg, _change_constrib, SRoleGuild(_roleid).getConstrib(), "constrib");
	sendValueChange(msg, _change_exploit, SRoleGuild(_roleid).getExploit(), "exploit");
	sendValueChange(msg, _change_fat, role->getFat(), "fat");
}

void Xylog::sendValueChange(CMsgTyped &msg, int begin, int end, const char* log)
{
	int change = end - begin;
	// 值没变化时, 不写日志
	if (change == 0) {
		return;
	}
	CMsgTyped tmpMsg = msg;
	tmpMsg.SetString(log);
	tmpMsg.SetInt(begin);
	tmpMsg.SetInt(end);
	tmpMsg.SetInt(change);
			
	SPSMgr.sendMessage(eLogServer, tmpMsg);
}

void Xylog::sendActionMsg()
{
	// 没有写日志内容时, 不处理
	if (_logs.empty()) {
		return;
	}
	
	// logserver 消息串, 以下内容为固定, tb_log为mysql日志表名
	CMsgTyped msg;
	msg.SetInt(0);
	msg.SetInt(1);
	msg.SetInt(GM_LOG);
	msg.SetString("tb_log");
	msg.SetString("");	// id, 数据行插入自增,　所以填空
	msg.SetString("");	// time,　数据行插入填充,　所以填空
	
	// logserver 消息串, 组基本header参数
	string name = "";
    string account = "";
	string platform = "";
	int lvl = 0;
	int vipLvl = 0;
	int vipExp = 0;
	int job = 0;
	int battle = 0;
	int guildId = 0;
	
    Role* role = SRoleMgr.GetRole(_roleid);
    if(role)
    {
		name = role->getRolename();
		account = SSessionDataMgr.getProp(role->getSessionId(), "account");
		platform = SSessionDataMgr.getProp(role->getSessionId(), "platform");
		lvl = role->getLvl();
		vipLvl = role->getVipLvl();
		vipExp = role->getVipExp();
		job = role->getJob();
		battle = role->getBattleForce();
		guildId = SRoleGuild(_roleid).getGuild();
    }
	msg.SetInt(time(NULL));
	msg.SetInt(Process::env.getInt("server_id"));
	msg.SetString(account.c_str());
	msg.SetString(platform.c_str());
	msg.SetInt(_roleid);
	msg.SetString(name.c_str());
	msg.SetInt(lvl);
	msg.SetInt(vipLvl);
	msg.SetInt(vipExp);
	msg.SetInt(job);
	msg.SetInt(battle);
	msg.SetInt(guildId);
	msg.SetInt(_logname);
	msg.SetInt(_action);
    
    // 内容体
    for (int i = 0; i < _logs.size(); ++i)
    {
		// logserver 消息串, 组基本内容参数
		msg.SetString(_logs[i].c_str());
    }
	// 发送消息至logserver
	SPSMgr.sendMessage(eLogServer, msg);
}


void Xylog::save()
{
    if(_logs.empty())
        return;
    
    stringstream ss;
    
    //header
	string name = "nil";
    string account = "nil";
	string platform = "nil";
	
    if(_roleid)
    {
        Role* role = SRoleMgr.GetRole(_roleid);
        if(role)
        {
            name = role->getRolename();
            account = SSessionDataMgr.getProp(role->getSessionId(), "account");
            platform = SSessionDataMgr.getProp(role->getSessionId(), "platform");
        }
    }
    
    ss<<_roleid<<"\t";
    ss<<name.c_str()<<"\t";
    ss<<account.c_str()<<"\t";
    ss<<platform.c_str()<<"\t";
    
    // content
    for (int i = 0; i < _logs.size(); ++i)
    {
        ss<<_logs[i]<<"\t";
    }
    
    xylog(_file.c_str(), ss.str().c_str());
    _saved = true;
}

Xylog& Xylog::operator << (const string& val)
{
    if(val.empty())
        _logs.push_back("nil");
    else
        _logs.push_back(val);
    return *this;
}

Xylog& Xylog::operator << (int val)
{
    stringstream ss;
    ss<<val;
    _logs.push_back(ss.str());
    return *this;
}

Xylog& Xylog::operator << (unsigned int val)
{
    stringstream ss;
    ss<<val;
    _logs.push_back(ss.str());
    return *this;
}

Xylog& Xylog::operator << (double val)
{
    stringstream ss;
    ss<<val;
    _logs.push_back(ss.str());
    return *this;
}

Xylog& Xylog::operator << (size_t val)
{
    stringstream ss;
    ss<<val;
    _logs.push_back(ss.str());
    return *this;
}

Xylog& Xylog::operator << (time_t val)
{
    stringstream ss;
    ss<<val;
    _logs.push_back(ss.str());
    return *this;
}



#define begin_log(name) const char* name::sName = #name;
#define log_member(type, name)
#define end_log()

#include "log_define.h"

#undef begin_log
#undef log_member
#undef end_log


#define begin_log(name) void name::specSave(){
#define log_member(type, name)  addField(name);
#define end_log() }

#include "log_define.h"

#undef begin_log
#undef log_member
#undef end_log
