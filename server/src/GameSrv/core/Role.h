//
//  Role.h
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#ifndef __GameSrv__Role__
#define __GameSrv__Role__

#include "DataCfg.h"
#include "Obj.h"
#include "Singleton.h"
#include "INetPacket.h"
#include <vector>
//#include "msg.h"
//#include "FriendMgr.h"

#include "Scene.h"
#include "Pet.h"
#include "ActivityMod.h"
//#include "ActivationCodeMgr.h"

//#include "RandomEvents.h"
#include "process.h"
//#include "GuildStore.h"
//#include "Honor.h"
#include "RedisHashJson.h"
#include "AnswerMgr.h"
#include "MultiPlayerRoom.h"
#include "CustomMailApp.h"

#include "Totem.h"
#include "MidastouchMgr.h"
#include "RoleMgr.h"

using namespace std;



class Pet;
class Robot;
class PetMgr;
class PetAssist;
class BackBag;
class PlayerEquip;
class ConstellationMgr;
class BaseActRoom;
class GiftActivityProgressMgr;
class PrivateChatHistoryMgr;
class CapsuletoyMgr;
class Wardrobe;
class WeaponEnchantMgr;
class FashionCollect;
class DailyScheduleSystem;
class RoleLoginDaysAwardMgr;
class RoleAwake;
class MysteriousMgr;
class IllustrationsMgr;
class FriendMgr;
class ActivationCodeMgr;
class GuildStoreMgr;
class HonorMgr;
class LuaRole;
class RetinueMgr;
class CsPvpState;

struct hangData
{
    hangData()
    {
        copyId = hangTimes = costTime = 0;
    }
    int copyId;
    int hangTimes;
    int costTime;
};

struct VerifyInfo
{
    VerifyInfo():roleid(0),
                 atk(0),
                 def(0),
                 maxHp(0),
                 cri(0.0f),
                 dodge(0.0f),
                 hit(0.0f),
                 activePetid(0),
                 petBattle(0)
    {}
    
    VerifyInfo(const VerifyInfo& src)
    {
        roleid = src.roleid;
        atk = src.atk;
        def = src.def;
        maxHp = src.maxHp;
        cri = src.cri;
        dodge = src.dodge;
        hit = src.hit;
        activePetid = src.activePetid;
        petBattle = src.petBattle;
    }
    
    VerifyInfo& operator=(const VerifyInfo src)
    {
        this->roleid = src.roleid;
        this->atk = src.atk;
        this->def = src.def;
        this->maxHp = src.maxHp;
        this->cri = src.cri;
        this->dodge = src.dodge;
        this->hit = src.hit;
        this->activePetid = src.activePetid;
        this->petBattle = src.petBattle;
        
        return *this;
    }
    
    int roleid;
    int atk;
    int def;
    int maxHp;
    float cri;
    float dodge;
    float hit;
    int activePetid;
    int petBattle;
    
    
};


enum RoleWorldState
{
    eWorldStateNone = 0,
    eWorldStateEntering,
    eWorldStateInWorld,
    eWorldStateLeaving,
};

class Role : public Obj, public ObjSave, public BaseProp , public NewRoleProperty {

    /********************  声明一些保存到数据库的变量  **********************/
    
    READWRITE(int, mSessionId, SessionId)
    READWRITE(int, mRawRef, RawRef)
    READWRITE(int, mLuaRef, LuaRef)
    READWRITE(BackBag*, mBackBag, BackBag)
    READWRITE(PlayerEquip*, mPlayerEquip, PlayerEquip)
    READWRITE(ConstellationMgr*, mConstellationMgr, ConstellationMgr)
    READWRITE(hangData, mHangData, HangData)
    READWRITE(FriendMgr*, mFriendMgr, FriendMgr)
    READWRITE(ActivationCodeMgr*, mActivationCodeMgr, ActivationCodeMgr)
    READWRITE(MysteriousMgr*, mMysteriousMgr, MysteriousMgr);
    READWRITE(IllustrationsMgr*, mIllustrationsMgr, IllustrationsMgr);
	READWRITE(GuildStoreMgr*, mGuildStoreMgr, GuildStoreMgr);
	READWRITE(HonorMgr*, mHonorMgr, HonorMgr);
	READWRITE(CustomMailMgr*, mCustomMailMgr, CustomMailMgr);
    READWRITE(TeamCopyMgr*, mTeamCopyMgr, TeamCopyMgr)
	READWRITE(MidastouchMgr*, mMidastouchMgr, MidastouchMgr)
    READWRITE(std::string, mRolename, Rolename)
    //新添加的答题活动
    READWRITE(AnswerActivityMgr *, mAnswerActivityMgr, AnswerActivityMgr)
    READWRITE(GiftActivityProgressMgr*, mGiftActivityProgressMgr, GiftActivityProgressMgr)
    
    READWRITE(string, mCurrentActRoom, CurrentActRoomName)
    READWRITE(string, mPlayerAccount, PlayerAccount)
    READWRITE(BaseActRoom*, mCurrentActivityRoom, CurrentActRoom)

    //几个限时活动
    READWRITE(FriendDungeAct*, mFriendDungeAct, FriendDungeAct)
    READWRITE(PrintCopyAct*, mPrintCopyAct, PrintCopyAct)
    READWRITE(PetCampAct*, mPetCampAct, PetCampAct)
    READWRITE(PetDungeAct*, mPetDungeAct, PetDungeAct)
    READWRITE(TowerDefenseAct*, mTowerDefenseAct, TowerDefenseAct)
    
    READWRITE(BaseActRoom*, mCopy, Copy)
    //好友聊天记录
    READWRITE(PrivateChatHistoryMgr *, mChatHistoryMgr, ChatHistoryMgr)
    //
    READWRITE(CapsuletoyMgr *, mCapsuletoyMgr, CapsuletoyMgr)
	READWRITE(TotemMgr *, mTotemMgr, TotemMgr)
    // 武器附魔
    READWRITE(WeaponEnchantMgr *, mWeaponEnchantMgr, WeaponEnchantMgr)
    
    READWRITE(DailyScheduleSystem*, mDailyScheduleSystem, DailyScheduleSystem)
	// 角色奖励
    READWRITE(RoleLoginDaysAwardMgr*, mRoleLoginDaysAwardMgr, RoleLoginDaysAwardMgr);
    
    READWRITE(LuaRole*, mLuaRole, LuaRole)
    
    std::set<int> mFinishEliteCopy;
    IntMapJson  mLotteryHistory;        //今日抽奖历史
    IntMapJson  mDiceLotteryHistory;    //色子抽奖历史
    
    //questid [progress]
    std::map<int, std::vector<int> > mQuestProgress;
    std::vector<int> mFinishQuestIds;
    
    //pet
    PetMgr* mPetMgr;
    PetAssist* mAssistPet;
    
    //wardrobe
    READWRITE(Wardrobe*, mWardrobe, Wardrobe)
    
    //好友地下城
//    std::map<int, int> mFriendDungeRecord;
//    std::map<int, int> mFriendDungeFriendRecord;
    
    //所在队伍的队长id
    READWRITE(int, mSyncTeamId, SyncTeamId);
    
    //公会宝藏战的场景id
//    READWRITE(int, mGuildTreasureSceneid, GuildTreasureSceneid)
    
    READWRITE(FashionCollect*, mFashionCollect, FashionCollect)
    
    READWRITE(RoleAwake*, mRoleAwake, RoleAwake);
    
    READWRITE(RetinueMgr*, mRetinueMgr, RetinueMgr)
    
    READWRITE(CsPvpState*, mCsPvpState, CsPvpState)
    
public:
    static const char* sGenIdKey;
    /************  声明一些临时的不保存到数据库的变量  ********/
    
    // 星灵失败累积人品
    READWRITE(float, mConstellationAccuRate, ConstellationAccuRate);
    
    
    //pvp敌人的id，及战斗力
    READWRITE(int64_t, mBechallengeId, BechallengeId)
    READWRITE(int, mBechallengeBattleForce, BechallengeBattleForce)
    
    READWRITE(int, mPaihangUpload, PaihangUpload)
    
    READWRITE(int, mBattleTime, BattleTime)
    
    READWRITE(int, mWorldState, WorldState)
    
    std::vector<struct DungeAward> mDungeAward;
    std::vector<int> mFamesHallAward;
    
    // 用于玩家回归条件判断
    int mLastLogintm;   // 上次登录时间
    //用于日志相关
    int mLoginMoment;   //最近登录的时间点
    
    //验证相关
//    int mfightedFriend;
    
    //是否是机器人，用于加载好友或敌人属性后调用不同的destroy
    bool mIsRobot;
    
	// 申请订单号记录时间
	int mRegisterOrderTime;
    
    int mCurrSceneId;
    
    bool mPetPvpMatching;
    
    int mBigDodgeTimes;  //用于闪避验证，如果连续两次就认为是作弊
    
    float mVerifyPropCode;    //属性校验码
    
    // 当前参与幻兽试炼场的宠
    READWRITE(int, mCurrentPetElitePet, CurrentPetElitePet)
    vector<int> mCurrentPetElitePets;
    
    vector<int> mPetpvpFormation;
    
    //幻兽大冒险玩家携带宠物
    int  m_nPetAdventureCarryPetId;
    Pet  m_CPetAdventureRentedPet;
    
    //邀请决斗的一些信息
    READWRITE(int, mDuelTarget, DuelTarget)
    READWRITE(int, mDuelState, DuelState)
    READWRITE(int, mDuelTime, DuelTime)
    READWRITE(string, mDuelEnemyName, DuelEnemyName);
    
    //竞技场限时
    READWRITE(int, mPvpBeginTime, PvpBeginTime)

    READWRITE(time_t, mLastHeartbeatTime, LastHeartbeatTime)
    
    READWRITE(int, mPreEnterSceneid, PreEnterSceneid)
    
    READWRITE(int, mGuildid, Guildid)
public:

public:
    //Role(){}
    virtual ~ Role();
    Role();
    Role(int _oldid, bool isRobot);
    
    static Role* create(int roleId, bool isRobot);
    
    //对象的销毁
    void unload();
    void RoleUninit();
    
    void onDestroy();
    void onDestroyRobot();
    
    //登录，初始化用到的
    bool load();
    void RoleInit();
    void CalcPlayerProp(eSceneType sceneType = stTown);
    void onCalPlayerProp();
    void loadRoleSave();
    void saveWeaponQuality();
    void saveRoleProp();
    void makePropVerifyCode();
    
    //发送人物属性及验证码
    void sendRolePropInfoAndVerifyCode(bool isBattleBuf = false);
    
    //发送宠物属性及验证码
    void sendPetPropAndVerifycode(Pet* pet, bool isBattleBuf = false);
    void onSendPetPropAndVerifycode(Pet* pet, bool isBattleBuf = false);
    
    //加载保存属性字段的方法
    string loadProp(const char* propname);
    void saveProp(const char* propname, const char* value);
    void saveProp(const char* propname, int value);

    //消耗品，消耗次数的加减方法
    void addExp(int exp, const char* mark = "");
    void addGold(int gold, const char* mark = "");
    void addBattlePoint(int bpoint, const char* mark = "");
    void addConstellVal(int consval, const char* mark = "");
    void addRmb(int rmb, const char* mark = "", bool sync = true);
    void addEnchantDust(int enchantDust, const char *mark = "");
    void addEnchantGold(int enchantGold, const char *mark = "");
    void addPvpPoints(int points, const char* mark = "");
    int CheckMoneyEnough(int cost, int moneyType, const char* usage = "");
    
    void calcMaxFatLimit();
    
    // 是否够钱消费，不够钱返回0，够钱返回钱数
    int hasEnoughMoney(int buyid,int time);
    // 为某件事消费，必须保证够钱
    void consumeMoney(int buyid,int time,const char* usage);
    
    //精力相关
    void addFat(int fat);
    void addFatBuyTimes(int times);
    
    //地下城相关
    void addDungTimes(int times);
    void addDungBuyTimes(int times);
    void saveDungLevelRecord(int dungcopyeid, int index);
    void resetDungLevelRecord(int index);
    void addDungResetTimes(int times, int index);
    void RoleResetDungCopy();
    int getDungResetTimes(int index);
    int getDungeLevelRecord(int index);
    int getDungeMaxSweepingLevel(int index);
    void setDungMaxSweepingLevel(int level, int index);
    
    int getFriendDungeFriendRecord(int index);
    void friendDungeFriendRecordInit(string str);
    void saveFriendDungeFriendRecord(int index, int friendId);
    void resetFriendDungeFriendRecord();
    
    void friendDungeRecordInit(string str);
    
    //精英副本相关
    void RoleResetEliteCopy();
    void LoadFinisEliteCopy();
    void addEliteTimes(int times);
    void addEliteBuyTimes(int times);
    bool IsEliteCopyFinish(int copyid);
    
    // 幻兽精英本剩余次数
    int getPetEliteCopyLeftTime();
    
    
    //pvp相关
    void stopPvp();
    void addToPvp();
    void RolePvpInit();
    void RoleResetPvp();
    void addPvpTimes(int times);
    void addPvpBuyTimes(int times);
    void addPvpBuff();
    void addSyncPvpBuff();
    void addTreasureFightBuff();
    void addCsPvpBuf();
    void removeBattleBuff();
    
    //同步pvp
    void clearDuelInfo();                               //清除对决的一些信息
    
    //好友副本
    void addTeamCopyResetTimes(int times);
    
    //VIP相关
    void addVipExp(int val, const char* comefrom = "");
    void updateVipLvlAward(int viplvl, bool canget);
    bool checkVipLvlAward(int viplvl);
    
    //日常任务
    void refreshDailyQuest();
    void addDailyQuestCount(int count);
    void addDailyQuestRefreshTimes(int times);
    
    //幻兽竞技场
    void addPetPvpTime(int times);
    
    //幻兽大冒险副本次数
    bool decrPetAdventureTimes(int times);
    void refreshPetAdventureTimes(int times);

    bool addPetAdventureBuyTimes(int times);
    bool addPetAdventureConsumableIncrTimes(int times);
    int  calPetAdventureFreeTimesRemain();
    int  calPetAdventureTimes();
    
    //幻兽大冒险机器幻兽租用次数
    void refreshPetAdventureRobotPetRentedTimes(int times);
    int  calPetAdventureRobotPetRentTimesRemain();
    
    bool decrPetAdventureRobotPetFreeRentTimesRemain(int times);
    bool decrPetAdventureRobotPetBuyingRentTimesRemain(int times);
    
    int  calPetAdventureRobotPetFreeRentTimesRemain();
    int  calPetAdventureRobotPetBuyingRentTimesRemain();
    
    int  payPetAdventureRobotPetRentedBill(bool bIsFreeCharged);

    //获取奖励，解析物品相关
    bool addAwards(const vector<string>& items, ItemArray& output, const char* comeFrom);
    void onAddItemAward(const char* comeFrom, RewardStruct reward);
    
    //任务相关
    bool recvQuest(int questid);
    bool checkQuest(int questid);
    void finshQuest(int questid);
    void doingQuest(std::vector<int>& outquest);
    void loadQuestData();
    
    //排行榜相关
    void UploadPaihangData(int optionType);
    void UploadPetPaihangData(int optionType, int petid, int petbattle);
    
    //每日，每周的刷新
    void RoleFresh(int freshtime);
    void weeklyRefresh(int refreshTime);
    void CheckEveryDayFirstLogin();
    void RoleFreshFat( int addPoint, int freshTime = 0);
    void RoleFreshProp( int propType, int propVal);
    
    //背包
//    void updateBackBag(const GridArray& grids, const ItemArray& items, bool isAdd, const char* desc, bool notify = true);

    
    //事件
    void onGetItems(const ItemArray& items);
    void onUseItems(const ItemArray& items);
    
    
    //
    void checkFashionExpiration();
    
    //
    virtual void onEnterScene();
    virtual void onLeaveScene();
    
    //行为相关
    bool enterCity(Scene* scene);
    void setCurrSceneId(int sceneid);
    int getCurrSceneId();
    void SaveLastCopy(int copyid);
    void backToCity();
    
    void onEnterGame();
    void onLeaveGame();
    
    //技能相关，目前这个函数已经没用了 2013-12-17
    vector<obj_studyable_skill> GetStudyableSKills();

    //宠物
    void addPetExp(int petid, int exp, const char* comeFrom);


    //防作弊的一些东西
    int checkClientBatttle(Obj* target, int damageType, int );
    bool checkWorldChatTime();
    
    void pushLotteryHistory(int item , int amount ,int rmb , bool isDice = false);
    
    //可能外部用到的一些借口，如获得一些基本属性
    void getRoleInfo(obj_roleinfo& info);
	void getPlayerBattleProp(obj_roleBattleProp& info);
    
    void loadNewProperty();
	void saveNewProperty();
	
	// 检查是否允许申请订单
	bool checkRegisterOrder();
	
	// 发送消息
	void send(INetPacket* packet);
    
    //定时检测的一些东西
    void checkRefreshFat(time_t nowtime);
    void DealWithRoleHangUp();
    virtual void OnHeartBeat(int nBeat);
    void sendFriendAwardMail();
    void checkPvpTimeout();
    
    // 取出简要战斗信息
    void getBriefBatforce(obj_batforce_brief& out);
    
    //保存出战好友信息
    void setFightedFriendInfo(Role* friendInfo, int petBattle, int activePetid);
    void setFightedFriendInfo(Robot *friendInfo);
    //获取出战好友id
    int64_t getFightedFriendId();
    
    //获取好友的验证信息
    VerifyInfo* getFightedFriendInfo();
    
    //清除出战好友信息
    void clearFriendInfo();
    
    //保存敌人验证信息
    void setEnemyVerifyInfo(BattleProp* enemy, int petBattle, int activePetid);
    
    //获取敌人的验证信息
    VerifyInfo& getEnemyVerifyInfo();
    
    //清除敌人的验证信息
    void clearEnemyVerifyInfo();
    
    //更改名字
    int renameRolename(string newname);

    bool deattachEquipInbag(int index);
    
    void sendMessageToFriends(INetPacket* packet);
    
    
    bool tryLeaveWorld();
    bool tryLeaveWorldScene();
    
    //加减公会鼓舞
    void addGuildInspired();
    void removeGuildInspired();
    
    int getSyncTeamCopyLeftTimesOfAward();
    
    //时装收集相关的
    void freshFashionCollectWithFashion();
    
    int preAddItems(const ItemArray& newitems, GridArray& effectGrid);
    
        /**
     *	@brief	添加物品并保存到数据库 与preAddItems(const ItemArray& newitems, GridArray& effectGrid)配合
     *
     *	@param 	grids 	受影响的个子
     *	@param 	items 	要添加的物品
     *	@param 	desc 	来源的描述 用于日志
     *	@param 	notify 	是否通知前端
     */
    void playerAddItemsAndStore(const GridArray &grids, const ItemArray &items, const char* desc, bool notify)
    ;
    
    /**
     *	@brief	删除物品并保存到数据库 目前可以与getBackBag()->PreDelItems(const ItemArray& newitems, GridArray& effectGrid), 还有时装收集里面的相关配合使用
     *
     *	@param 	grids 	受影响的个子
     *	@param 	items 	要删除的物品
     *	@param 	desc 	在哪里消耗的描述 用于日志
     *	@param 	notify 	是否通知前端
     */
    void playerDeleteItemsAndStore(const GridArray &grids, const ItemArray &items, const char* desc, bool notify);
    
//    void preUpdateItems(const GridArray &grids, vector<int>& effeIndex);
//    
//    void storeItems();
    
    void updateBackBagItems(int index, ItemGroup& item);
    
    void updateItemsChange(GridArray& effectGrid);
    
    void updateBackBag(GridArray &effectGrid);
    
    //日志
    void addLogActivityCopyEnterTimesChange(int activityCopyType, const char* changeAction);

    bool skillCanLearn(int skillId);
    
    void insertRmbConsumedRecord(int rmb);
    void insertRechargeRecord(int rmb);
private:
    
    //验证好友信息
    VerifyInfo* mFightedFrinendVerifyInfo;
    
    //验证敌人信息
    VerifyInfo mEnemyVerifyInfo;
    
    
    //
    bool mIsAllLoad;

    void lvlUp();
    void viplvlup();
};

void UpdateQuestProgress(std::string upatecmd,Role* role,bool updatedb);
void UpdateQuestProgress(std::string type, int target, int num, Role* role, bool updatedb);
void DeleteRoleRoleInfoInDB(int roleid, int sessionid);
void Recharge(int roleid, Role* role, int rmb, const char* platform, const char* billno);
void StoreOfflineItem(int roleid, const char* itemdesc);
int  GetRoleIdByName(const char* name);



bool isRoleExistInDbForMainThread(int roleid);
void GlobalRoleOnline(Role* role);
void GlobalRoleOffline(int roleid);
bool checkPlayerCompatPowerForCheat(Role* role, int sceneid, int& battleTime);
//删除玩家背包过期物品
void removeExpiredItemWhenRoleInit(Role *role);

ObjJob RoleType2Job(int roleType);
ObjSex RoleType2Sex(int roleType);
void kickAwayRoleWithTip(Role* role, int errorcode = 0);

#define kickAwayRoleWithTips(role, cheatDescrip) \
{\
    string tipInfo = strFormat("%s:%d", __FILE__, __LINE__);\
    LogMod::addLogClientCheat(role->getInstID(), role->getRolename().c_str(), role->getSessionId(), tipInfo.c_str(), cheatDescrip);\
    kickAwayRoleWithTip(role);\
}

#endif /* defined(__GameSrv__Role__) */
