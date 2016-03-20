//
//  Log.h
//  GameSrv
//
//  Created by pireszhi on 13-6-15.
//
//

#ifndef __GameSrv__Log__
#define __GameSrv__Log__

#include <iostream>
#include "thread.h"
#include "timestamp.h"
#include "ItemBag.h"
#include <vector>
#include <string>
#include "MsgBuf.h"
#include "LogPropName.h"
#include "logservice.h"

using namespace std;

struct message_queue;
class Role;
class Xylog;
class SortInfo;
class SortPetInfo;

enum LogActivityEnterTimesType
{
    eActivityEnterTimesNone = 0,
    eActivityEnterTimesTowerDefense = 1,    //守卫之战进入次数
    eActivityEnterTimesFriendDunge = 2,     //好友地下城进入次数
    eActivityEnterTimesFriendCopyDoubleAward = 3,   //好友副本双倍药水次数
    eActivityEnterTimesSyncTeamCopy = 4,    //同步组队副本收益次数
    eActivityEnterTimesMysticalCopyBOSSandGoblin = 5,  //幻兽大本营使用物品增加打副本百分之一百出现隐藏BOSS和哥布林的次数
    eActivityEnterTimesPetAdventure = 6,     //幻兽大冒险进入次数
    eActivityEnterTimesSyncPvp = 7,         //同步竞技场
    eActivityEnterTimesPvp = 8,             //普通竞技场
    eActivityEnterTimesEliteCopy = 9        //精英副本
    
};

namespace LogMod {
    void addLoginLog(int roleid, const char* rolename, int sessionid, int rolesCount);
    void addLogLogout(int roleid, const char* rolename, int sessionid, int timeonline, int logoutType);

    //chat log
    void addLogPrivateChat(int senderid, const char* senderName, int sessionid, int recvid, const char* recvName, const char* recvAccount, const char* content);
    void addLogWorldChat(int senderid, const char* senderName, int sessionid, const char* content);
    
    //gold log
    void addLogGetGold(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total);
    void addLogConsumGold(int roleid, const char* rolename, int sessionid, int consumCount, const char* usage, int total);
    
    //rmb log
    void addLogGetRmb(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total, int lvl, int atk);
    void addLogGetRmb(int roleid, const char* rolename, const char* accountname, const char* platform, int getCount, const char* getfrom, int total, int lvl, int atk);
    void addLogConsumRmb(int roleid, const char* rolename, int sessionid, int consumCount, const char* usage, int total, int lvl, int vipLvl, int vipExp, int atk);
    
    //battlepoint
    void addLogGetBattlePoint(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total);
    void addLogConsumBattlePoint(int roleid, const char* rolename, int sessionid, int consumCount, const char* usage, int total);
    //get exp
    void addLogGetexp(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total);
    
    //get Constellval
    void addLogGetConstellVal(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total);
    
    // get enchantDust
    void addLogGetEnchantDust(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total);
    
    // get enchantGold
    void addLogGetEnchantGold(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total);
    
    //vipexp log
    void addLogGetVipexp(int roleid, const char* rolename, int sessionid, int getCount, const char* getfrom, int total, int viplvl);
    

    //option log
    void addLogUpgradeEquip(int roleid, const char* rolename, int sessionid, int equipid, int newlvl);
    void addLogGetinPvp(int roleid, const char* rolename, int sessionid, int myBattleForce, int challengeid, int enemyBattleForce, int battleTime, int result, int prerank, int newrank);
    
    void addLogUpgradeSkill(int roleid, int skillid, const char* skillname);

    void addLogClientCheat(int roleid, const char* rolename, int sessionid, const char* kickInfo, const char* cheatinfo);

    //苹果充值记录
    void addLogAppleRecharge(int roleid,int reqroleid,int rmb ,const char* transaction,const char* apple_receipt);

    //宠物属性变化记录
    void addLogPetProChange(int owner,int petid,const char* petname,const char* action,const char* before,const char* change,const char* after,const char* extra);
    
    //行为操作
    void addLogGetItem(Role* role, const char* rolename, int sessionid, ItemArray items, const char* getfrom);
    
    void addLogComposeItem(int roleid, const char* rolename, int sessionid,const char* consumitems, const char* getitems);

    void addLogGetPet(int roleid, const char* rolename, int sessionid, int petTmpId, int petid, const char* getfrom);
    void addLogConstellationUpgrade(int roleid, const char* rolename, int sessionid, int constellid, int prestep, int newstep, int consum, int total);
    
    void addLogDelRole(int roleid, const char* rolename, int sessionid, int lvl, int viplvl, int vipexp);
    void addLogCreateRole(int roleid, const char* rolename, int sessionid);
    
    void addLogPetPvpPointsChange(int roleid, const char* rolename, int sessionid, int changePoints, int total, const char* operation);
    
    void addLogPetPvpPointsChange(int roleid, const char* rolename, const char* account, int changePoints, int total, const char* operation);
    
    void addLogPetPvpCupsChange(int roleid, const char* roleName, int preTotalCups, int changeCups, int totalCups, const char* pvpType, const char* from);
    
    void addLogPetCampPassRecord(int roleid, int copyid, int passtime, int mysticalBossid);
    
    void addLogCompatPowerError(int roleid, int copyid, int passtime);
    
    void addLogEnterCopy(int roleid, int copyid, int copyType);
    
    void addLogFinishCopy(Role* role, int copyid, bool isPass, string award, int copyType);
    
    void addLogSendRoleMail(int roleid, const char* title, const char* attach, bool sendSuccess);
    
    //变异精英，俗称图纸副本
    void addLogFinishDungeException(Role* role, int sceneid, int costSec);
   
    void addLogTowerDefensePass(Role* role, int waves, int costSec, int getPoints, int friends, int sceneid);
    
    void addLogAddFashion(Role* role, int fashionId, int fashionType, int expireTime);
    
    //使用物品日志
    void addUseItemLog(Role *role, const ItemArray& items, const string &description);
    
    void addLogSynPvpMedal(int roleId, const char* roleName, int getmedal, int total, string getfrom);
    
    void addLogLimitTimeKill(Role* role, int useTime, int getscore);
    
    void addLogTowerDefenseAward(Role* role, int waves, int friendcount, int friendIntimacySum, string award, int sceneid);
    
    
    //操作日志
    void addLogGetLoginAward(Role* role, int day, const char* awards);
	
	void addSdkLoginLog(const char *loginMode, const char *url, const char *send, const char *ret, int result);
    
    void addLogGetFamesHallAward(Role* role, int level, string award);
    
    void addLogFinishFriendDunge(Role* role, int index, int level);
    
    void addLogFinishDunge(Role* role, int index, int level);
    
    void addLogFinishEliteCopy(Role* role, int sceneid);
    
    void addLogFinishPetElite(Role* role, int sceneid);
    void addLogSkillHurtRecord(Role* role, int skillid, int effectIndex, int damage);
    
    void addLogPetEvolutionInfo(Role* role, string leftPetInfo, string rightPetInfo, string newPetInfo);
	
	void addBattleLog(Xylog &log, const char *desc, BaseProp &base, BattleProp &bat);
    
    void addLogWeaponEnchantErr(Role* role, int enchantid, int enchantlvl, string details);
    void addLogWeaponEnchantUpgrade(Role* role, int enchantid, int preLvl, int newLvl, int preExp, int newExp, string details);
    
    //幻兽大冒险日志
    void addLogFinishPetAdventureCopy(Role* role, int copyid, bool isPass, string award, int copyType);
    //幻兽大冒险幻兽属性加成
    void addLogPetAdventureBufOnPet(Role* role, int copyid, int copyType, string szLogbeforeAddPetBuf, string szLogAfterAddPetBuf);
    //幻兽大冒险购买副本次数日志
    void addLogPetAdventureCopyBuyTimes(Role* role, int buytimes, int cost, int costtype, string buytimeschange, string buyTimesRecordPerDay, string petAdventureTimes, bool buyissucceeded);
    //幻兽大冒险租用幻兽次数日志
    void addLogPetAdventureRobotPetRentTimes(Role* role, int renttimes, int cost, int costtype, string szLogRobotPetRentBuyingTimesRecordPerDay, string szLogRobotPetFreeRentTimesRemain, string szLogRobotPetBuyingRentTimesRemain, string szLogRobotPetRentTimesRemain, bool rentissucceeded);

    //地下城扫荡日志
    void addLogDungeonSweeping(Role* role, int beginsceneid, int endsceneId, string award, bool sweeped);
    //幻兽大本营完成副本消耗品与副本次数改变日志
    void addLogMysticalCopyTimesAndIncrTimes(Role* role, string IncrTimesChange, string copyTimesChange, bool bossAppear, bool monsterAppear);

    
    void addLogFashionCollectPutOnMaterial(Role* role, int collectProgressId, int materialid, int grid, int count, string alldMaterial);
    
    void addLogFashionCollectFinishProgress(Role* role, int progressId, int way, int buyFashionId);
    
    void addLogFashionCollectMaterialRecycle(Role* role, int materialId, int count, int recycleLvl, int recycleExp);

    
    //同步战斗日志
    void addLogChangeSpeed(int sceneMod, int sceneId, int roleId, const char* rolename,
                           float changePer, int changeVal, float curPer, int curVal);
    
    //
    /**
     *	@brief	一些副本或者活动进入次数的日志
     *
     *	@param 	roleid 	角色id
     *	@param 	roleName 	角色名
     *	@param 	changeType 	副本的类型或者活动类型 具体的值参照 枚举:LogActivityEnterTimesType
     *	@param 	itemAddTimes 	使用物品增加的次数
     *	@param 	rmbAddTimes 	使用金钻增加的次数
     *	@param 	freeTimes 	每天的免费次数
     *	@param 	totalTimes 	总次数
     *	@param 	changeAction 	消耗的去向或者增加方式的描述
     */
    void addLogActivityEnterTimesChange(int roleid, const char* roleName, int changeType, int itemAddTimes, int rmbAddTimes, int freeTimes, int totalTimes, const char* changeAction);
    
    void addLogDelPetInDB(Role* role, int petId, int petMod, const char* petInfo, const char* describe);
    
    void addLogGetMailAttach(Role* role, const char* mailId, const char* mailTitle, const char* mailAttach);
    
    void addLogAddPetExp(Role* role, int petId, int petMod, int preTotalExp, int addExp, int totalExp, int level, const char* comeFrom);
    
    void addLogDuokuSdkLogin(string AccessToken, string uid, int curlcode, string ret, int result);
    
    void addLogPetSwallowLockOperate(Role* role, int petid, vector<int> swallowLockStatusChange);

    void addLogRoleSortData(int sortType, vector<SortInfo*>& input);
    
    void addLogPetBattleForceSortData(vector<SortPetInfo*>& input);
    
    void addLogEquipBaptize(int roleId, int equipIndex, int equipId, int baptizeQua, int baptizeIndex, const char* beforeProp, const char* afterProp, const char* operation);
    
    void addLogRetinueSummon(int roleid, int summonId, int summonType, const char* getItems);
    
    void addLogRetinueAddExp(int roleid, int retinueId, int addExp, int beforExp, int afterExp, const char* beforeInfo, const char*afterInfo, const char* comsume);
    
    void addLogRetinueStarup(int roleid, int retinueId, int beforStar, int afterStar, const char* beforeInfo, const char*afterInfo, const char* comsume);
    
    void addLogRetinueSkillStudy(int roleid, int retinueSkillId, int beforLvl, int afterLvl, const char* comsume);
    
    void addLogPvpPointExchange(int roleid, int comsumePoints, int beforePoints, int afterPoints, const char* getItems);
    
    void addLogPvpPointChange(int roleid, int changePoints, int beforePoints, int afterPoints, const char* usageOrComeFrom);
    
    void addLogRetinueCompose(int roleid, int retinueId, int retinueModid, const char* retinueInfo, const char* comsume);
    
    void addWorshipLog(int roleId, int targetId, const char* type, int costRmb);
}


//void xylog(const char* file,const char * data, ...);    //新游调试
void xyerr(const char* data,...);


/*
 Xylog使用示例
 
 Xylog log(eLogName_Non, 10001, eLogAction_Test);
 log<<"参数1";
 {
	... 需监控值变化的代码段
 }
 log<<"参数2";
 log<<"参数3";
 
 */


//突然发现Xylog线程不安全； by wangzhigang 2014、4、29
class Xylog
{
public:
	/**
	 logname	日志名
	 roleid		角色id（如果是系统日志, 填0）
	 action		影响动作
	 */
	Xylog(eLogPropName logname, int roleid, eLogAction action = eLogAction_Non);
    ~Xylog();
	void save();
	
    Xylog& operator << (const string& val);
    Xylog& operator << (int val);
    Xylog& operator << (unsigned int val);
    Xylog& operator << (double val);
    Xylog& operator << (size_t val);
	Xylog& operator << (time_t val);
	
	void cancelSave() {
		_saved = true;
	}
	
private:
	// 初始值变化
	void initValueChange();
	
	// 发送值变化日志
	void checkValueChange();
	void sendValueChange(CMsgTyped &msg, int begin, int end, const char* log);

	// 发送行为日志
	void sendActionMsg();
	
protected:
    int            _roleid;
    bool           _saved;
    string         _file;
    vector<string> _logs;
	
	// 日志名
	eLogPropName _logname;
	// 日志动作
	eLogAction _action;
	
	// 记录值变化
	int _change_exp;
	int _change_gold;
	int _change_batpoint;
	int _change_consval;
	int _change_rmb;
	int _change_constrib;
	int _change_exploit;
	int _change_fat;
};



//正在常检测
#define xycheck( assertion ) \
do{\
    if( !(assertion) ) { \
        return ;\
    } \
}while(0)

// 带返回值
#define send_if(cond ,  err )\
do{\
    if( (cond) )\
    {\
        ack.errorcode = (err) ;\
        return sendNetPacket(role->getSessionId(), & ack);\
    }\
}while(0)


//此宏只能在主线程起来才可用
#define xyassert( assertion ) \
    do{\
        if( !(assertion) ) { \
            xyerr( "%s:%i ASSERTION FAILED:  %s", __FILE__, __LINE__, #assertion );  \
            return ;\
        } \
    }while(0)

//此宏只能在主线程起来才可用
#define xyassertf(assertion , ...)\
do{\
    if( !(assertion) ) { \
        xyerr( "%s:%i ASSERTION FAILED:  %s", __FILE__, __LINE__, #assertion );  \
        xyerr( __VA_ARGS__ ) ;\
        return ;\
    } \
}while(0)

#include "LogBase.h"

#define begin_log(name) class name : public BaseLog { \
static const char* sName;  \
virtual void specSave();  \
const char* getName() {return sName;}
#define log_member(type, name)  public:\
type name;
#define end_log() };

#include "log_define.h"

#undef begin_log
#undef log_member
#undef end_log






#endif /* defined(__GameSrv__Log__) */
