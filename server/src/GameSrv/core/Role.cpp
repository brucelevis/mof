//
//  Role.cpp
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#include "Role.h"

#include "Game.h"
#include "Scene.h"
#include "Skill.h"
#include "Quest.h"
#include "main.h"
#include "json/json.h"
#include <assert.h>

#include "DBRolePropName.h"
#include "flopcard.h"
#include "INetPacket.h"
#include <time.h>
#include "Pvp.h"
#include "Pet.h"
#include "cmd_def.h"
#include "gate.h"
#include <bitset>
#include "Utils.h"
#include "ActivityMod.h"
#include "GuildRole.h"
#include "MultiPlayerRoom.h"
#include "MysteriousMgr.h"
#include "GuildMgr.h"
#include "Guild.h"
#include "Constellation.h"
#include "PetPvp.h"
#include "ItemChange.h"
#include "datetime.h"
#include "GameLog.h"
#include "TimeLimitKillCopy.h"
#include "gift_activity.h"
#include "offline_action.h"
#include "AntiCheating.h"
#include "RandName.h"
#include "mail_imp.h"
#include "world_cmdmsg.h"
#include "NumericConvert.h"
#include "CapsuletoyMgr.h"
#include "RoleMgr.h"
#include "SceneMgr.h"
#include "Wardrobe.h"
#include "Robot.h"
#include "FashionCollect.h"
#include "daily_schedule/daily_schedule_system.h"
#include "TreasureFight.h"
#include "utility.h"
#include "mail_imp.h"
#include "RoleLoginDaysAward.h"
#include "RoleAwake.h"
#include "BroadcastNotify.h"
#include "Global.h"

#include "TwistEggMgr.h"
#include "WeaponEnchant.h"
#include "PrivateChatHistoryMgr.h"
#include "FriendMgr.h"
#include "ActivationCodeMgr.h"
#include "GuildStore.h"
#include "Honor.h"
#include "GameScript.h"
#include "LuaRole.h"
#include "Retinue.h"
#include "centerClient.h"
#include "cs_pvp/cs_pvp_battle.h"
#include "worship/worship_system.h"

extern float calcWorshipPlus(int common, int special);
// 月卡充值活动
extern void Recharge_MonthRechargeAwardAct( int roleid ,const char* rolename, int rmb,int time );
// 单笔充值大返利（可叠加）
extern void rechargeAwardActivity(int roleid ,const char* rolename, int rmb , bool isOnline);
// 单笔充值一次性返利（不可叠加）
extern void onceRechargeAwardActivity(int roleid ,const char* rolename, int rmb );
// 累计充值活动
extern void Recharge_AccuRechargeAwardAct( int roleid , int rmb );

extern void SendBackBagNotify(Role* role, const GridArray& grids);
extern void SendBackBagNotify(Role* role, int index, const ItemGroup& item);
extern void SendBackBagNotify(Role* role, std::vector<int>& indexs);
extern void SendPlayerEquipNotify(Role* role, const GridArray& grids);
extern void SendPlayerEquipNotify(Role* role, int index, const ItemGroup& equip);
extern void sendRetinueBagNotify(Role* role, const GridArray& grids);

extern void geteSortBatData(int roleid, int beginRank, int endRank, GETPAIHANGDATAFUNC cbfunc);

extern int createRawRole(lua_State* state, Role* role);

const char* Role::sGenIdKey = "gen_roleid";

void sendChangedDungeonState(Role* role);

Role::Role(int _newid, bool isRobot) : Obj(eRole),
                                        mSessionId(0),
                                        mBackBag(NULL),
                                        mPlayerEquip(NULL),
                                        mConstellationMgr(NULL),

                                        mFriendMgr(NULL),
                                        mActivationCodeMgr(NULL),
                                        mTeamCopyMgr(NULL),
										mMidastouchMgr(NULL),
                                        mFriendDungeAct(NULL),
                                        mPrintCopyAct(NULL),
                                        mPetCampAct(NULL),
                                        mPetDungeAct(NULL),
                                        mAssistPet(NULL),
                                        mPetMgr(NULL),
										mHonorMgr(NULL),
										mCustomMailMgr(NULL),
										mGuildStoreMgr(NULL),
										mIllustrationsMgr(NULL),
										mMysteriousMgr(NULL),
                                        mPetPvpMatching(false),
                                        mFightedFrinendVerifyInfo(NULL),
                                        mEnemyVerifyInfo(VerifyInfo()),
                                        mWardrobe(NULL),
                                        mVerifyPropCode(0.0f),
                                        mCurrentPetElitePet(0),
										mRegisterOrderTime(0),
                                        mWorldState(eWorldStateNone),
										mTotemMgr(NULL),
                                        mSyncTeamId(0),
                                        mWeaponEnchantMgr(NULL),
                                        mFashionCollect(NULL),
                                        mIsAllLoad(false),
                                        mTowerDefenseAct(NULL),
                                        mRoleLoginDaysAwardMgr(NULL),
                                        m_CPetAdventureRentedPet(0),

                                        // 星灵失败累积人品
                                        mConstellationAccuRate(0.0f),
                                        mRoleAwake(NULL),
                                        mLuaRole(NULL)

{
    mIsRobot = isRobot;
    setInstID(_newid);
    mPvpAward = mPvpBuyTimes = mPvpTimes = mPaihangUpload = 0;
}

Role::~Role()
{
    if (!mIsRobot) {
        RoleUninit();
    }
    unload();
}

Role* Role::create(int roleId, bool isRobot)
{
    Role* role = new Role(roleId, isRobot);
    if (!role->load()) {
        delete role;
        return NULL;
    }
    if (!isRobot)
    {
        role->RoleInit();
    }
    
    role->mIsAllLoad = true;
    role->CalcPlayerProp();
    
    return role;
}

bool Role::load()
{
    loadRoleSave();
    if (getRolename().empty()) {
        return false;
    }
    /*加载机器人或角色的一些必要信息：
     
     ********** 背包，装备，技能，星座等影响战斗相关的东西 ****
     
     */
	loadNewProperty();
    
    //地下城数据兼容
    int dungeonIndices[3] = {eDungLvl_one, eDungLvl_two, eDungLvl_three};
    int dungeonLevels[3] = {getDungLevel1(), getDungLevel2(), getDungLevel3()};
    int dungeonResetTimes[3] = {getDungResetTimes1(), getDungResetTimes2(), getDungResetTimes3()};
    for (int i = 0; i < 3; i++) {
        if (dungeonLevels[i] != 0) {
            getDungeonState().setDungeonLevel(dungeonIndices[i], dungeonLevels[i]);
        }
        
        if (dungeonResetTimes[i] != 0) {
            getDungeonState().setResetTimes(dungeonIndices[i], dungeonResetTimes[i]);
        }
        
        RoleResetDungCopy();
        RedisArgs args;
        RedisHelper::beginHmset(args, strFormat("role:%d", mInstID).c_str());
        RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungResetTimes1), "0");
        RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungResetTimes2), "0");
        RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungResetTimes3), "0");
        RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungLevel1), "0");
        RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungLevel2), "0");
        RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungLevel3), "0");
        RedisHelper::appendHmset(args, GetRolePropName(eRoleDungTimes), getDungTimes());
        RedisHelper::commitHmset(get_DbContext(), args);
    }
    
    
    //writeDB();
    if (getRmbExtendedBackbagSize() < 0) {
        setRmbExtendedBackbagSize(0);
    }
    mBackBag = BackBag::Create(BackBag::sFreeBackbagSize + getRmbExtendedBackbagSize());
    mBackBag->Load(mInstID);
    
    //装备加载
    mPlayerEquip = PlayerEquip::Create(10);
    mPlayerEquip->Load(mInstID);
    
    // 星座
    mConstellationMgr = ConstellationMgr::create(this);
    mConstellationMgr->LoadDataFromDB();
    
    //加载技能
    getSkillMod()->loadSkillsFromDB(getInstID());
	
	// 图腾
	mTotemMgr = TotemMgr::create(getInstID());
	mTotemMgr->load();
    
    // 加载宠物
    mPetMgr = PetMgr::create(this);
    mPetMgr->load();
    
    mAssistPet = PetAssist::create(this);
    mAssistPet->load();
    
	// 称号
	mHonorMgr = new HonorMgr();
	mHonorMgr->load(this);
    
    mWardrobe = Wardrobe::create(this);
    mWardrobe->load();
    
    mRoleAwake = RoleAwake::Create(this);
    mRoleAwake->init();
    
    // 武器附魔
    mWeaponEnchantMgr = WeaponEnchantMgr::create(this);
    mWeaponEnchantMgr->loadEnchantData();
    
    //时装收集
    mFashionCollect = FashionCollect::Create(this);
    mFashionCollect->loadData();
    
    // 角色奖励
    mRoleLoginDaysAwardMgr = new RoleLoginDaysAwardMgr();
    mRoleLoginDaysAwardMgr->load(this);
    
    //暂时觉得有必要by wangzhigang 2014-11-18
    freshFashionCollectWithFashion();
    
    //加载lua角色数据
    mLuaRole = LuaRole::create(this);
    if (mLuaRole) {
        mLuaRole->loadData();
    }
    
    mRetinueMgr = RetinueMgr::create(this);
    mRetinueMgr->loadAllRetinue();
    
    mCsPvpState = CsPvpState::create(this);
    
    return true;
}

void Role::sendMessageToFriends(INetPacket* packet)
{
    struct Traverse : public Traverser
    {
        vector<int> sessions;
        virtual bool callback(void* data)
        {
            Friend* pFriend = (Friend*)data;
            if (pFriend == NULL) {
                return true;
            }
            
            int64_t friendId = pFriend->mFid;
            if (friendId == RobotCfg::sFriendId) {
                return true;
            }
            
            Role* role = SRoleMgr.GetRole(friendId);
            if (role) {
                sessions.push_back(role->getSessionId());
            }
            
            return true;
        }
    };
    
    Traverse helper;
    mFriendMgr->traverse(&helper);
    multicastPacket(helper.sessions, packet);
}

//不影响战斗属性的信息在这里加载， 否则放在构造函数里加载
void Role::RoleInit()
{   
    mFriendMgr = new FriendMgr();
    mTeamCopyMgr = new TeamCopyMgr();
	mMidastouchMgr = new MidastouchMgr();
    mActivationCodeMgr = new ActivationCodeMgr();
    mMysteriousMgr = new MysteriousMgr();
    mIllustrationsMgr = new IllustrationsMgr();
	mGuildStoreMgr = new GuildStoreMgr();
	mCustomMailMgr = new CustomMailMgr();

    //新添加的答题
    mAnswerActivityMgr = new AnswerActivityMgr();
    mGiftActivityProgressMgr = GiftActivityProgressMgr::create(this);
    mDailyScheduleSystem = DailyScheduleSystem::create(this);
    
    mChatHistoryMgr = PrivateChatHistoryMgr::create();
    //
    mCapsuletoyMgr = CapsuletoyMgr::create();

    if (getPlayerAccount().empty()) {
        string account = SSessionDataMgr.getProp(getSessionId(), "account");
        setPlayerAccount(account.c_str());
        saveProp(GetRolePropName(eRolePropPlayerAccount), account.c_str());
    }

    //setCurrSceneId(mSceneID);
    setPreEnterSceneid(mSceneID);
    mCurrSceneId = mSceneID;
    mLoginMoment = 0;
    
    loadQuestData();

    //friend relatives
    mFriendMgr->loadFriendList(mInstID);

    notify_friendonline onlinenotify;
    onlinenotify.roleid = mInstID;
    sendMessageToFriends(&onlinenotify);

    string faultpackage = GlobalCfg::ReadStr("faultpackage");
    string rolefaultpackage = loadProp("faultpackage");
    int packageid = Utils::safe_atoi(faultpackage.c_str());
    if (faultpackage != rolefaultpackage && packageid != 0)
    {
        saveProp("faultpackage", faultpackage.c_str());

        ItemArray items;
        items.push_back(ItemGroup(packageid, 1));
        GridArray grids;
        
        //wangzhigang 2014-11-19
        this->preAddItems(items, grids);
        //mBackBag->PreAddItems(items, grids);
        //mBackBag->UpdateBackBag(grids);
        //mBackBag->Store(mInstID, grids);
        //updateBackBag(grids, items, true, "faultpackage", false);
        playerAddItemsAndStore(grids, items, "faultpackage", false);
    }

    string rolermbpackage = loadProp("rmbpackage");
    if (rolermbpackage.empty())
    {
        int rmbpackage = GlobalCfg::ReadInt("rmbpackage");
        saveProp("rmbpackage", "1");

        ItemArray items;
        items.push_back(ItemGroup(rmbpackage, 1));
        GridArray grids;
        
        preAddItems(items, grids);
//        mBackBag->PreAddItems(items, grids);
        //mBackBag->UpdateBackBag(grids);
        //mBackBag->Store(mInstID, grids);
//        updateBackBag(grids, items, true, "rmbpackage", false);
        playerAddItemsAndStore(grids, items, "rmbpackage", false);
    }

    //teamcopy
    mTeamCopyMgr->loadTeamCopyData(getInstID());

    //friendDunge
    mFriendDungeAct = new FriendDungeAct(this, ae_friend_dunge);
    mFriendDungeAct->loadFriendDungeData(getInstID());

    //print copy
    mPrintCopyAct = new PrintCopyAct(this, ae_print_copy);
    mPrintCopyAct->loadPrintCopyData(getInstID());

    //petcamp
    mPetCampAct = new PetCampAct(this, ae_pet_copy);
    mPetCampAct->loadPetCampData(getInstID());

    //pet dunge
    mPetDungeAct = new PetDungeAct(this, ae_friend_tower);
    mPetDungeAct->loadPetDungeData(getInstID());

    //
    mTowerDefenseAct = new TowerDefenseAct(this, ae_tower_defense);
    mTowerDefenseAct->loadData();
    
    // 激活码
    mActivationCodeMgr->load(this);
    // 神秘商店
    mMysteriousMgr->load(this);
    // 图鉴
    mIllustrationsMgr->loadIllustrations(this);
    // 公会商店
	mGuildStoreMgr->load(this);
	mCustomMailMgr->load(this);
    //答题活动
	mAnswerActivityMgr->load(this);
	
	mMidastouchMgr->load(this);
    //好友聊天记录
    mChatHistoryMgr->load(this);
    //
    mCapsuletoyMgr->load(this);
        
    //转换人物背包和装备栏上面的装备,暂时不用
    //RoleEquipNumericConvert(this);
    
    //删除背包过期物品
    removeExpiredItemWhenRoleInit(this);

    //
    LoadFinisEliteCopy();

    //计算一下属性
    //onCalPlayerProp();

    Pet *pet = mPetMgr->getActivePet();
    if (pet) {
        UploadPetPaihangData(eUpdate, pet->petid, pet->mBattle);
    }

    viplvlup();
    doRedisCmd("hmset role:%d %s %d %s %d",getInstID(), GetRolePropName(eRolePropVipLvl), getVipLvl(),
               GetRolePropName(eRolePropVipExp), getVipExp());

    //保存武器品质
    saveWeaponQuality();
    //ChekEveryDayFirstLogin();
    
    //初始化决斗数据
    setDuelTarget(0);
    setDuelState(0);
    setDuelTime(0);
    
    setLastHeartbeatTime(Game::tick);
    
    //保存一下公会id
    int guildid = SRoleGuildMgr.getRoleGuild(this->getInstID()).getGuild();
    setGuildid(guildid);
    
}


void Role::unload()
{
    delete mBackBag;
    delete mPlayerEquip;
    delete mConstellationMgr;
    delete mTotemMgr;
    if (mPetMgr) {
        mPetMgr->unload();
    }

    delete mPetMgr;
    delete mAssistPet;
    delete mHonorMgr;
    delete mWardrobe;
    delete mWeaponEnchantMgr;
    delete mFashionCollect;
    delete mRoleLoginDaysAwardMgr;

}

void Role::RoleUninit()
{
//    g_friendDungeActMgr.eraseActData(getInstID());
//    g_PrintCopyActMgr.eraseActData(getInstID());
//    g_PetCampActMgr.eraseActData(getInstID());
//    g_PetDunngeActMgr.eraseActData(getInstID());

    if (!mCurrentActRoom.empty())
    {
        //断线或者崩溃时，发放限时击杀副本奖励
        //if (mCurrentActRoom == TimeLimitKillCopy::getClassName())
        //{
        //    TimeLimitKillCopy* copy = dynamic_cast<TimeLimitKillCopy*>(ActRoomMGR.find(mCurrentActRoom.c_str(), this));
        //    if (copy)
        //    {
        //        copy->sendAwards(this, false);
        //    }
        //}
    }
    ActRoomMGR.RemovePlayer(this);

    //保存部分属性
    saveRoleProp();


    delete mFriendMgr;
    delete mTeamCopyMgr;
	delete mMidastouchMgr;

    delete mFriendDungeAct;
    delete mPrintCopyAct;
    delete mPetCampAct;
    delete mPetDungeAct;

    delete mActivationCodeMgr;
    delete mMysteriousMgr;
    delete mIllustrationsMgr;
    delete mGuildStoreMgr;
	delete mCustomMailMgr;

    //删除答题
    delete mAnswerActivityMgr;
    delete mGiftActivityProgressMgr;
    delete mDailyScheduleSystem;
    
    delete mChatHistoryMgr;
    delete mCapsuletoyMgr;
}

void Role::addExp(int exp, const char* mark){
    check_min(exp, 0);
    if (getLvl() >= RoleCfg::getMaxRoleLvl()){
        return;
    }

    mExp += exp;
    lvlUp();
    doRedisCmd("hmset role:%d %s %d %s %d",getInstID(), GetRolePropName(eRolePropLvl), getLvl(),
               GetRolePropName(eRolePropExp), getExp());

    LogMod::addLogGetexp(getInstID(), getRolename().c_str(), getSessionId(), exp, mark, mExp);
}

void Role::addGold(int gold, const char* mark)
{
    if (!gold) {
        return;
    }

    mGold += gold;
    check_range(mGold, 0, 0x7FFFFFFF);
    doRedisCmd("hmset role:%d %s %d",getInstID(), GetRolePropName(eRolePropGold), mGold);
    notify_syn_gold noti;
    noti.gold = mGold;
    sendNetPacket(getSessionId(),&noti);

    if(gold < 0)
    {
        LogMod::addLogConsumGold(getInstID(), getRolename().c_str(), getSessionId(), gold, mark, getGold());
    }
    else
    {
        LogMod::addLogGetGold(getInstID(), getRolename().c_str(), getSessionId(), gold, mark, getGold());
		// 称号检查
		SHonorMou.procHonor(eHonorKey_Gold, this);
    }
}

// 累计消费活动
extern void Consume_AccuConsumeAwardAct( int roleid , int rmb );

void Role::addRmb(int rmb, const char* mark, bool sync)
{
    if (0 == rmb) {
        return;
    }

    mRmb += rmb;
    check_range(mRmb, 0, 0x7FFFFFFF);
    doRedisCmd("hmset role:%d %s %d",getInstID(), GetRolePropName(eRolePropRmb), mRmb);

	if (sync){
        notify_syn_rmb noti;
        noti.rmb = mRmb;
        sendNetPacket(getSessionId(),&noti);
    }

    if(rmb < 0)
    {
        LogMod::addLogConsumRmb(getInstID(), getRolename().c_str(), getSessionId(), rmb, mark, getRmb(), getLvl(), getVipLvl(), getVipExp(), getBattleForce());

        Consume_AccuConsumeAwardAct( getInstID() , - rmb);
        
        
        do
        {
            ServerGroupCfgDef*  def = g_ConsumeRankListGroupCfg.getServerGroupCfgDefByServerid(Process::env.getInt("server_id"));
            if (def == NULL) {
                break;
            }
            
            int beginTime = def->mBeginTime;
            int endTime   = def->mEndTime;
            
            int currentTime = time(NULL);
            if (beginTime <= currentTime && currentTime <= endTime) {
                insertRmbConsumedRecord(- rmb);

                UploadPaihangData(eUpdate);
                
                notify_syn_accumulateRmbConsumedRecord notify;
                notify.record = getRmbConsumedRecord().getRecord();
                sendNetPacket(getSessionId(), &notify);
            }
            
        }while (false);

    }
    else
    {
        LogMod::addLogGetRmb(getInstID(), getRolename().c_str(), getSessionId(), rmb, mark, getRmb(), getLvl(), getBattleForce());
    }
}


// 累计消耗精力
extern void Consume_AccuConsumeFatAwardAct( int roleid , int fat );

void Role::addFat(int fat)
{
    mFat += fat;

    mFat = mFat < 0 ? 0 : mFat;

    doRedisCmd("hmset role:%d %s %d", getInstID(), GetRolePropName(eRolePropFat), mFat);

    notify_syn_fat notify;
    notify.fat = mFat;
    sendNetPacket(getSessionId(), &notify);
    
    if(fat > 0)
    {
        Xylog log(eLogName_GetFat, getInstID());
        log << fat << mFat;
    }
    else
    {
        Xylog log(eLogName_ConsumeFat, getInstID());
        log << fat << mFat;
        Consume_AccuConsumeFatAwardAct(getInstID(), -fat);
    }
}

void Role::addBattlePoint(int bpoint, const char* mark)
{
    if (bpoint == 0)
    {
        return;
    }

    mBattlePoint += bpoint;
    check_range(mBattlePoint, 0, 0x7FFFFFFF);

    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRolePropBattlePoint), mBattlePoint);

    notify_syn_batpoint notify;
    notify.batpoint = mBattlePoint;
    sendNetPacket(getSessionId(), &notify);

    if(bpoint < 0)
    {
        LogMod::addLogConsumBattlePoint(getInstID(), getRolename().c_str(), getSessionId(), bpoint, mark, getBattlePoint());
    }
    else
    {
        LogMod::addLogGetBattlePoint(getInstID(), getRolename().c_str(), getSessionId(), bpoint, mark, getBattlePoint());
    }
}

void Role::addConstellVal(int consval, const char* mark)
{
    if (consval == 0)
    {
        return;
    }

    int val = getConstellVal() + consval;
    check_range(val, 0, 0x7FFFFFFF);

    setConstellVal(val);

    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRolePropConstellVal), val);

    notify_syn_constellval notify;
    notify.constval = val;
    sendNetPacket(getSessionId(), &notify);

    LogMod::addLogGetConstellVal(getInstID(), getRolename().c_str(), getSessionId(), consval, mark, val);

}

void Role::addEnchantDust(int enchantDust, const char *mark)
{
    if (enchantDust == 0)
    {
        return;
    }
    
    int val = getEnchantDust() + enchantDust;
    check_range(val, 0, 0x7FFFFFFF);
    
    setEnchantDust(val);
    
    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRolePropEnchantDust), val);
    
    notify_syn_enchantDust notify;
    notify.enchantDust = val;
    sendNetPacket(getSessionId(), &notify);
    
    LogMod::addLogGetEnchantDust(getInstID(), getRolename().c_str(), getSessionId(), enchantDust, mark, val);
}

void Role::addEnchantGold(int enchantGold, const char *mark)
{
    if (enchantGold == 0)
    {
        return;
    }
    
    int val = getEnchantGold() + enchantGold;
    check_range(val, 0, 0x7FFFFFFF);
    setEnchantGold(val);
    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRolePropEnchantGold), val);
    
    notify_syn_enchantGold notify;
    notify.enchantGold = val;
    sendNetPacket(getSessionId(), &notify);
    
    LogMod::addLogGetEnchantGold(getInstID(), getRolename().c_str(), getSessionId(), enchantGold, mark, val);
}

void Role::addFatBuyTimes(int times)
{
    if (!times) {
        return;
    }

    mFatBuyTimes += times;

    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRolefatBuyTimes), mFatBuyTimes);
}

void Role::addPvpPoints(int points, const char* mark)
{
    if (points == 0)
    {
        return;
    }
    
    int myPoint = getPvpPoints();
    
    myPoint += points;
    check_range(myPoint, 0, 0x7FFFFFFF);
    
    setPvpPoints(myPoint);
    NewRoleProperty::save();
    
    //log
    LogMod::addLogPvpPointChange(this->getInstID(), points, myPoint, this->getPvpPoints(), mark);
}

void Role::saveProp(const char* propname, const char* value)
{
    doRedisCmd("hset role:%d %s %s", getInstID(), propname, value);
}


void Role::saveProp(const char* propname, int value)
{
    doRedisCmd("hset role:%d %s %d", getInstID(), propname, value);
}


string Role::loadProp(const char* propname)
{
    string value = "";

    char cmd[256];
    sprintf(cmd, "hget role:%d %s", getInstID(), propname);

    redisReply* reply = redisCmd(cmd);
    do
    {
        if (reply == NULL)
        {
            break;
        }

        if (reply->str != NULL)
        {
            value = reply->str;
        }
    }
    while (0);

    freeReplyObject(reply);

    return value;
}

void Role::loadNewProperty()
{
    NewRoleProperty::load(getInstID());
    mLotteryHistory.load("lotteryhistory", getInstID());
    mDiceLotteryHistory.load("dicelotteryhistory",getInstID());
}

void Role::saveNewProperty()
{
	NewRoleProperty::save();
    mLotteryHistory.save();
    mDiceLotteryHistory.save();
}



bool Role::addAwards(const vector<string>& awards, ItemArray& output, const char* comeFrom)
{
    ItemArray items;
    RewardStruct reward;

    rewardsCmds2ItemArray(awards, items, reward);

    GridArray effgrids;

    if(preAddItems(items, effgrids) != CE_OK)
    {
        return false;
    }

    onAddItemAward(comeFrom, reward);

    output.clear();
    output.resize(items.size());
    std::copy(items.begin(), items.end(), output.begin());
    
    playerAddItemsAndStore(effgrids, items, comeFrom, true);
	
    return true;
}

void Role::onAddItemAward(const char* comeFrom, RewardStruct reward)
{
    addRmb(reward.reward_rmb, comeFrom);
    addExp(reward.reward_exp, comeFrom);
    addGold(reward.reward_gold, comeFrom);
    addBattlePoint(reward.reward_batpoint, comeFrom);
    addConstellVal(reward.reward_consval, comeFrom);
    addPetExp(mActivePetId, reward.reward_petexp, comeFrom);
    addVipExp(reward.reward_vipexp, comeFrom);
    addFat(reward.reward_fat);
    addEnchantDust(reward.reward_enchantdust, comeFrom);
    addEnchantGold(reward.reward_enchantgold, comeFrom);
    addPvpPoints(reward.reward_pvppoints, comeFrom);

    SRoleGuild( getInstID() ).addConstrib( reward.reward_constrib , comeFrom);
    SRoleGuild( getInstID() ).addExploit( reward.reward_exploit , comeFrom);

    if (reward.petpvp_points) {
        SPetPvpMgr.safe_gmAddPetPvpPoints(getInstID(), reward.petpvp_points);
    }

    getHonorMgr()->addPrestige(reward.reward_prestige);
}

void Role::addEliteTimes(int times)
{
    if (this->getIncrEliteCopyTimes() > 0 && times < 0)
    {
        this->setIncrEliteCopyTimes(this->getIncrEliteCopyTimes() - 1);
        this->saveNewProperty();
    }
    else
    {
        mEliteTimes += times;
        
        mEliteTimes = mEliteTimes < 0 ? 0 : mEliteTimes;
        
        doRedisCmd("hmset role:%d %s %d", getInstID(), GetRolePropName(eRoleEliteTimes), mEliteTimes);
    }
    notify_syn_elitetimes notify;
    notify.times = mEliteTimes + this->getIncrEliteCopyTimes();
    sendNetPacket(getSessionId(), &notify);
}

void Role::setCurrSceneId(int sceneid)
{
    if (mCurrSceneId == sceneid) {
        return;
    }
    
    SceneCfgDef* scene = SceneCfg::getCfg(sceneid);
    if (scene == NULL) {
        return;
    }
    
    mCurrSceneId = sceneid;
    
    if (scene->sceneType != stTown) {
        LogMod::addLogEnterCopy(getInstID(), sceneid, scene->sceneType);
    }
    
    if (mScene)
    {
        notify_player_state_refresh notify;
        notify.roleid = mInstID;
        notify.rolestate = mCurrSceneId;
        mScene->broadcast(&notify);
    }
    
    notify_friend_state_refresh fnotify;
    fnotify.friendid = mInstID;
    fnotify.sceneid = mCurrSceneId;
    sendMessageToFriends(&fnotify);
    
    //回到城镇或去地下城下一层回满血
    if (scene->sceneType == stTown ||           //城镇
        scene->sceneType == stDungeon ||        //普通地下城
        scene->sceneType == stFriendDunge ||    //好友地下城
        scene->sceneType == stScriptScene ) {   //脚本场景
        
        sendRolePropInfoAndVerifyCode(true);
        
        Pet* pet = mPetMgr->getActivePet();
        
        if (pet) {
            pet->makePropVerifyCode();
            onSendPetPropAndVerifycode(pet, true);
        }
    }
}

int Role::getCurrSceneId()
{
    return mCurrSceneId;
}

void Role::addEliteBuyTimes(int times)
{
    if (!times) {
        return;
    }

    mEliteBuyTimes += times;

    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRoleEliteBuyTimes), mEliteBuyTimes);
}

void Role::addDungTimes(int times)
{
    mDungTimes += times;

    mDungTimes = mDungTimes < 0 ? 0 : mDungTimes;

    doRedisCmd("hmset role:%d %s %d", getInstID(), GetRolePropName(eRoleDungTimes), mDungTimes);

    notify_syn_dungtimes notify;
    notify.times = mDungTimes;
    sendNetPacket(getSessionId(), &notify);
}

void Role::addPvpTimes(int times)
{
    mPvpTimes += times;
    mPvpTimes = mPvpTimes < 0 ? 0 : mPvpTimes;

    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRolePvpTimes), mPvpTimes);

    notify_syn_pvptimes notify;
    notify.times = mPvpTimes;
    sendNetPacket(getSessionId(), &notify);
}

void Role::addPvpBuyTimes(int times)
{
    if (!times) {
        return;
    }

    mPvpBuyTimes += times;

    mTeamCopyResetTimes = mTeamCopyResetTimes < 0 ? 0 : mTeamCopyResetTimes;

    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRolePvpBuyTimes), mPvpBuyTimes);
}

void Role::addSyncPvpBuff()
{
    mRealBattleProp.setMaxHp(getMaxHp() * SynPvpFuntionCfg::mCfgDef.roleHpMultiple);
    sendRolePropInfoAndVerifyCode(true);
    
    Pet* pet = mPetMgr->getActivePet();
    if (pet) {
        pet->mRealBattleProp.setMaxHp(pet->getMaxHp() * SynPvpFuntionCfg::mCfgDef.petHpMultiple);
        sendPetPropAndVerifycode(pet, true);
    }
}

void Role::addTreasureFightBuff()
{
    mRealBattleProp.setMaxHp(getMaxHp() * GuildTreasureFightCfg::mCfg.roleHpRate);
    sendRolePropInfoAndVerifyCode(true);
}

void Role::addPvpBuff()
{
    mRealBattleProp.setMaxHp(getMaxHp() * 10);
    mRealBattleProp.setDef(getDef() + 200);
    sendRolePropInfoAndVerifyCode(true);

    Pet* pet = mPetMgr->getActivePet();
    if (pet) {
        pet->mRealBattleProp.setMaxHp(pet->getMaxHp()*10);
//        pet->calculateBattle();
        pet->makePropVerifyCode();
        sendPetPropAndVerifycode(pet, true);
    }
}


void Role::addCsPvpBuf()
{
    mRealBattleProp.setMaxHp(getMaxHp() * 10);
    mRealBattleProp.setDef(getDef() + 200);
    float worshipRate = 1.0 + calcWorshipPlus(g_WorshipSystem.getCommonBeWorshipTimes(mInstID),
                                        g_WorshipSystem.getSpecialBeWorshipTimes(mInstID));
    mRealBattleProp *= worshipRate;
    sendRolePropInfoAndVerifyCode(true);
    
    Pet* pet = mPetMgr->getActivePet();
    if (pet) {
        pet->mRealBattleProp.setMaxHp(pet->getMaxHp()*10);
        //        pet->calculateBattle();
        pet->makePropVerifyCode();
        sendPetPropAndVerifycode(pet, true);
    }
}

void Role::removeBattleBuff()
{
    mRealBattleProp.setHit(getHit());
    mRealBattleProp.setAtk(getAtk());
    mRealBattleProp.setDodge(getDodge());
    mRealBattleProp.setCri(getCri());
    mRealBattleProp.setMaxHp(getMaxHp());
    mRealBattleProp.setDef(getDef());
    sendRolePropInfoAndVerifyCode(true);

    Pet* pet = mPetMgr->getActivePet();
    if (pet) {
        pet->mRealBattleProp.setMaxHp(pet->getMaxHp());
        pet->mRealBattleProp.setAtk(pet->getAtk());
        pet->mRealBattleProp.setDef(pet->getDef());
        pet->mRealBattleProp.setCri(pet->getCri());
        pet->mRealBattleProp.setHit(pet->getHit());
        pet->mRealBattleProp.setDodge(pet->getDodge());
//        pet->calculateBattle();
        pet->makePropVerifyCode();
        sendPetPropAndVerifycode(pet, true);
    }
}

void Role::clearDuelInfo()
{
    setDuelState(0);
    setDuelEnemyName("");
    setDuelTarget(0);
    setDuelTime(0);
}

void Role::addTeamCopyResetTimes(int times)
{
    if (!times) {
        return;
    }
    mTeamCopyResetTimes += times;
    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRoleTeamCopyResetTimes), mTeamCopyResetTimes);

    notify_syn_teamresettimes notify;
    notify.times = mTeamCopyResetTimes;
    sendNetPacket(getSessionId(), &notify);
}


void Role::lvlUp(){
    RoleCfgDef& cfg = RoleCfg::getCfg(mJob, mLvl);

    if(mExp < cfg.getExp()){
        syn_exp sd;
        sd.exp = getExp();
        sendNetPacket(getSessionId(), &sd);
        return;
    }

    if (getLvl() >= RoleCfg::getMaxRoleLvl()){
        mExp = 0;
        syn_exp sd;
        sd.exp = getExp();
        sendNetPacket(getSessionId(), &sd);
        return;
    }

    // 升级了
    setLvl(getLvl()+1);
    setExp(mExp-cfg.getExp());

    //open pvp
    if (getLvl() == PvpCfg::getOpenLvl()) {
        addToPvp();
    }

    notify_lvl_up sd;
    sd.lvl = getLvl();
    sd.exp = getExp();
    sendNetPacket(getSessionId(), &sd);

    CalcPlayerProp();

    //SendDailyQuest(this);
    create_cmd(dailyquest, dailyquest);
    dailyquest->roleid = mInstID;
    sendMessageToGame(0, CMD_MSG, dailyquest, 0);
    // todo  和等级提升相关的模块

	// 称号检查
	SHonorMou.procHonor(eHonorKey_Lvl, this);
	// 日志记录
	Xylog log(eLogName_RoleUp, getInstID());
	log<<getExp()<<getLvl();

    lvlUp();
}

void Role::checkRefreshFat(time_t nowTime){

    tm refreshTm, nowTm;
    time_t refreshTime = getMassFatFreshTime();
    localtime_r(&refreshTime, &refreshTm);
    localtime_r(&nowTime, &nowTm);
    if (refreshTime > nowTime){
        return;
    }

    int refreshPoint = GlobalCfg::mMassRefreshPoint;
    vector<int>& refreshHours = GlobalCfg::mMassRefreshHours;
    //refreshHours.push_back(4);
    //refreshHours.push_back(12);
    //sort(refreshHours.begin(), refreshHours.end());

    int addPoints = 0;
    //过去了多少天
    int days = (nowTime - refreshTime) / SECONDS_PER_DAY;
    addPoints += days * (refreshPoint * refreshHours.size());
    refreshTime += days * SECONDS_PER_DAY;

    localtime_r(&refreshTime, &refreshTm);
    localtime_r(&nowTime, &nowTm);
    int i = 0;
    for (; i < refreshHours.size(); i++){
        int hour = refreshHours[i];
        if (hour < refreshTm.tm_hour){
            continue;
        }

        if (hour > nowTm.tm_hour){
            break;
        }

        addPoints += refreshPoint;
    }

    RoleFreshFat(addPoints);

    if (i == refreshHours.size()) {
        if (refreshHours.size() > 0) {
            refreshTime = DateTime::getNextTimeHitHour(refreshHours[0]);
        }
    }
    else{
        refreshTime = nowTime + (refreshHours[i] - nowTm.tm_hour) * 3600 - nowTm.tm_min * 60 - nowTm.tm_sec;
    }

    setMassFatFreshTime(refreshTime);
    save();
}

void Role::OnHeartBeat(int nBeat)
{
    checkRefreshFat(Game::tick);

    CheckEveryDayFirstLogin();
    
    checkPvpTimeout();
    
    int now = Game::tick;      //time(NULL);
    
    if ( (now - mFatFreshTime) > GlobalCfg::mRefreshPeriod) {
        int times = ( now - mFatFreshTime ) / (GlobalCfg::mRefreshPeriod);
        RoleFreshFat(GlobalCfg::mRefreshPoint * times, mFatFreshTime + times * GlobalCfg::mRefreshPeriod);
    }

    if (nBeat % 10 == 0)
    {
        checkFashionExpiration();
    }
}

void Role::checkFashionExpiration()
{
    vector<int> fashions;
    bool isActiveExpired = mWardrobe->checkExpiredFashions(&fashions);
    if (isActiveExpired)
    {
    }
    
    if (fashions.size() > 0)
    {
        notify_fashion_expire notify;
        notify.fashions = fashions;
        sendNetPacket(getSessionId(), &notify);
    }
}

void Role::checkPvpTimeout()
{
    SceneCfgDef* currscene = SceneCfg::getCfg(getCurrSceneId());
    
    if (currscene == NULL || currscene->sceneType != stPvp) {
        return;
    }
    
    int timeout = PvpCfg::getTimeout();
    if (!timeout) {
        return;
    }
    
    int now = Game::tick;
    int beginTime = getPvpBeginTime();
    
    if (beginTime == 0) {
        return;
    }
    
    if ( (now - beginTime) > timeout) {
        PvpAward* pvpDef = PvpCfg::getResultAward(false);
        addGold(pvpDef->gold, "pvp_lose");
        addConstellVal(pvpDef->protoss, "pvp_lose");
    
        create_global(pvpResult, result);
        result->roleid = MAKE_OBJ_ID(kObjectRole, getInstID());
        result->bechallengeid = getBechallengeId();
        result->isWin = false;
        result->battleTime = timeout;
        
        sendGlobalMsg(Global::MQ, result);
        
        setPvpBeginTime(0);
    }
}

void Role::DealWithRoleHangUp()
{
//    if (mHangData.copyId && mHangData.hangTimes) {
//        mHangData.costTime--;
//
//        if (mHangData.costTime == 0) {
//            mHangData.hangTimes--;
//
//            //发放奖励、扣除精力等操作
//            BackBag* bag= getBackBag();
//
//            ItemArray items;
//            int reward_exp = 0;
//            int reward_gold = 0;
//
//            int fcindex = 0;
//            vector<int> dropindices;
//            std::vector<std::string> rewards = StrSpilt(SflopcardMgr.RandomCard(mHangData.copyId, fcindex, dropindices));
////            rewardsCmds2ItemArray(rewards,items,reward_exp,reward_gold);
//
//            GridArray effgrids;
//            //检查是否可以发放奖励
//            bool canAddItem = bag->PreAddItems(items,effgrids);
//
//            SceneCfgDef* scfgdef = SceneCfg::getCfg(mHangData.copyId);
//            addExp(reward_exp+scfgdef->copy_exp);
//            addGold(reward_gold+scfgdef->copy_gold);
//
//
//            if (canAddItem) {
//                BackBag::UpdateBackBag(getInstID(), effgrids);
////                notify_flopcard notify;
////                notify.index = fcindex;
////                sendRolePacket(getInstID(),&notify);
//
//            }
//        }
//    }
}
//bool Role::updateQuestProgress(int questid,int item,int val)
//{
//    std::map<int, std::vector<int> >::iterator fit = mQuestProgress.find(questid);
//    Quest *quest = QuestMgr::FindQuest(questid);
//    if (fit != mQuestProgress.end()
//        &&(quest != NULL)
//        &&fit->second.size() >= item) {
//        std::vector<int>& progress = fit->second;
//        //内存进度和配置表进度不一样时修复
//        if (progress.size() > quest->conditions.size()) {
//            progress.erase(progress.begin()+quest->conditions.size(), progress.end());
//        }
//        if (progress.size() < quest->conditions.size()) {
//            progress.resize(quest->conditions.size());
//        }
//
//        std::string tokstr = quest->conditions[item];
//        char* cmd = NULL;
//        int id = 0;
//        int num = 0;
//        sscanf(tokstr.c_str(), "%s %d*%d",cmd,&id,&num);
//        progress[item];
//    }
//    return false;
//}
bool Role::recvQuest(int questid)
{
    std::map<int, std::vector<int> >::iterator fit = mQuestProgress.find(questid);
    Quest *quest = QuestMgr::FindQuest(questid);

    if ((fit == mQuestProgress.end())&&(quest != NULL))
    {
        if (find(mFinishQuestIds.begin(),mFinishQuestIds.end(), questid) != mFinishQuestIds.end())
        {
            return false;
        }

        if (quest->minLvl > getLvl() || getLvl() > quest->maxLvl)
            return false;

        if ((quest->preQuestId == 0) ||
         (std::find(mFinishQuestIds.begin(),mFinishQuestIds.end(),quest->preQuestId)!= mFinishQuestIds.end())
         )
        {
            std::vector<int> projress(quest->conditions.size());
            mQuestProgress[questid] = projress;
            return true;
        }
    }
    return false;
}

bool Role::checkQuest(int questid)
{

    std::map<int, std::vector<int> >::iterator fit = mQuestProgress.find(questid);
    Quest *quest = QuestMgr::FindQuest(questid);
    if ((fit != mQuestProgress.end())&&(quest != NULL))
    {
        return CheckQuestCanFinish(quest, fit->second);
    }
    return false;
}
void Role::finshQuest(int questid)
{
    mQuestProgress.erase(questid);
}
void Role::doingQuest(std::vector<int>& outquest)
{
    for (std::map<int, std::vector<int> >::iterator it = mQuestProgress.begin(); it != mQuestProgress.end(); it++) {
        outquest.push_back(it->first);
    }

}

void Role::onCalPlayerProp()
{
	int oldBattle = getBattleForce();
	
    BaseProp baseprop;
    BattleProp batprop;

	//人物成长属性
    RoleCfgDef rolecfg = RoleCfg::getCfg(mJob, mLvl);

	BaseProp roleBase;
    BattleProp roleBat;

    roleBase.mCapa = rolecfg.getCapa();
    roleBase.mStre = rolecfg.getStre();
    roleBase.mInte = rolecfg.getInte();
    roleBase.mPhys = rolecfg.getPhys();

    roleBat.mAtk = rolecfg.getAtk();
    roleBat.mDef = rolecfg.getDef();
    roleBat.mDodge = rolecfg.getDodge();
    roleBat.mHit = rolecfg.getHit();
    roleBat.mCri = rolecfg.getCri();
    roleBat.mMaxHp = rolecfg.getHp();
    roleBat.mMaxMp = rolecfg.getMp();
	
	baseprop += roleBase;
	batprop += roleBat;

    //人物装备属性
    BaseProp equipbase;
    BattleProp equipbat;
    mPlayerEquip->CalcProperty(equipbat, equipbase);
    batprop += equipbat;

	// 技能
    BaseProp skillBase;
    BattleProp skillBattle;
    getSkillMod()->equipPassiveSkill(skillBase, skillBattle);
    batprop += skillBattle;
    baseprop += skillBase;

	// 星座
    BaseProp constellbase;
    BattleProp constellbat;
    getConstellationMgr()->CalcProperty(constellbat, constellbase);
    batprop += constellbat;
    baseprop += constellbase;

    //公会纹章
    BaseProp guilHeraldryBase;
    BattleProp guilHeraldryBat;
    calHeraldryPropertyAddition(getInstID(), guilHeraldryBase, guilHeraldryBat);
    baseprop += guilHeraldryBase;
    batprop += guilHeraldryBat;

    //公会被动技能加成
    BaseProp guilSkillBase;
    BattleProp guilSkillBat;
    calGuildSkillPropertyAdditionForSelf(getInstID(), guilSkillBase, guilSkillBat);
    baseprop += guilSkillBase;
    batprop += guilSkillBat;

	// 称号加成
	BaseProp honorBase;
    BattleProp honorBat;
	if (mHonorMgr) {
		mHonorMgr->CalcProperty(honorBat, honorBase);
	}
	baseprop += honorBase;
    batprop += honorBat;

	// 助阵
    BaseProp petassistBase;
    BattleProp petassistBat;
    if (mAssistPet){
        mAssistPet->calcProperty(petassistBat, petassistBase);
    }
    batprop += petassistBat;
    baseprop += petassistBase;

    // 衣柜加成
	BaseProp wardrobeBase;
    BattleProp wardrobeBat;
    if (mWardrobe) {
        mWardrobe->accuProperty(wardrobeBase, wardrobeBat);
    }

	baseprop += wardrobeBase;
    batprop += wardrobeBat;

    BaseProp enchantBasepro;
    BattleProp enchantBatpro;
    // 武器附魔加成
    if (mWeaponEnchantMgr) {
        
        mWeaponEnchantMgr->accuProperty(enchantBatpro);
    }
    
    batprop += enchantBatpro;
    
    //时装收集的属性
    BattleProp fashionCollect;
    
    if (getFashionCollect()) {
        
        getFashionCollect()->addPlayerTotalProperty(fashionCollect);
        
    }
    
    batprop += fashionCollect;
    
    BattleProp awakeProp;
    BaseProp awakeBaseProp;
    if (mRoleAwake)
    {
        mRoleAwake->calcAwakeAddProp(awakeProp, awakeBaseProp);
    }
    
    batprop += awakeProp;
    
    //侍魂通灵的属性
    BattleProp spiritspeakProp;
    if (mRetinueMgr) {
        mRetinueMgr->mSpiritSpeakMgr->calcProp(spiritspeakProp);
    }
    
    batprop +=spiritspeakProp;
    
    //lua模块的属性
//    mLuaRole->accuProperty(baseprop, batprop);
    //calcLuaRoleProp(this, 0, baseprop, batprop);
    
    //设置人物最终属性
    setCapa(baseprop.mCapa);
    setStre(baseprop.mStre);
    setInte(baseprop.mInte);
    setPhys(baseprop.mPhys);

    setAtk(batprop.mAtk);
    setDef(batprop.mDef);
    setDodge(batprop.mDodge);
    setHit(batprop.mHit);
    setCri(batprop.mCri);
    setHp(batprop.mMaxHp);
    setMaxHp(batprop.mMaxHp);
    setMaxMp(batprop.mMaxMp);

    int battle = calcBattleForce(this); //mAtk + mDef *2 + mMaxHp /10 + int ((mHit-90) *10) +int(mDodge * 10);
    setBattleForce(battle);
    
    //sendChangedDungeonState(this);

	// 称号检查
	SHonorMou.procHonor(eHonorKey_EquipQua, this);
	SHonorMou.procHonor(eHonorKey_EquipLvl, this);
	SHonorMou.procHonor(eHonorKey_Power, this);

    mRealBattleProp.setAtk(getAtk());
    mRealBattleProp.setDef(getDef());
    mRealBattleProp.setCri(getCri());
    mRealBattleProp.setDodge(getDodge());
    mRealBattleProp.setHit(getHit());
    
    mRealBattleProp.setMaxHp(getMaxHp());
    mRealBattleProp.setHpIncr(getHpIncr());
    mRealBattleProp.setMoveSpeed(getMoveSpeed());
	
	// 记录变化日志
	if (oldBattle != battle) {
		Xylog log(eLogName_BattleChange, getInstID());
		log << battle << oldBattle;
		LogMod::addBattleLog(log, "result", baseprop, batprop);
		LogMod::addBattleLog(log, "base", roleBase, roleBat);
		LogMod::addBattleLog(log, "equip", equipbase, equipbat);
		LogMod::addBattleLog(log, "skill", skillBase, skillBattle);
		LogMod::addBattleLog(log, "constell", constellbase, constellbat);
		LogMod::addBattleLog(log, "heraldry", guilHeraldryBase, guilHeraldryBat);
		LogMod::addBattleLog(log, "guilSkill", guilSkillBase, guilSkillBat);
		LogMod::addBattleLog(log, "honor", honorBase, honorBat);
		LogMod::addBattleLog(log, "petassist", petassistBase, petassistBat);
		LogMod::addBattleLog(log, "wardrobe", wardrobeBase, wardrobeBat);
        LogMod::addBattleLog(log, "weaponEnchant", enchantBasepro, enchantBatpro);
	}
}

void Role::CalcPlayerProp(eSceneType sceneType)
{
    if (mIsAllLoad == false) {
        return;
    }
    
    BattleProp::setLvl(mLvl);
    
    onCalPlayerProp();

    if (mIsRobot == false) {
        save();
    }
    
    SceneCfgDef* scene = SceneCfg::getCfg(getCurrSceneId());

    if (scene == NULL || scene->sceneType != stTown) {
        return;
    }

    sendRolePropInfoAndVerifyCode();
}

void Role::loadRoleSave()
{
    /*char rcmd[256];
    sprintf(rcmd, "hmget role:%d %s %s %s %s %s %s %s %s %s %s %s %s %s", mInstID,
            GetRolePropName(eRolePropExp),
            GetRolePropName(eRolePropGold),
            GetRolePropName(eRolePropRmb),
            GetRolePropName(eRolePropFat),
            GetRolePropName(eRolePropBattlePoint),
            GetRolePropName(eRolePropConstellVal),
            GetRolePropName(eRoleLastFinishCopy),
            GetRolePropName(eRoleDungTimes),
            GetRolePropName(eRoleEliteTimes),
            GetRolePropName(eRolefatFreshTime),
            GetRolePropName(eRolefatBuyTimes),
            GetRolePropName(eRoleEliteBuyTimes),
            GetRolePropName(eRoleDungBuyTimes)
            );*/

    const char* props[] =
    {
        GetRolePropName(eRolePropExp),
        GetRolePropName(eRolePropGold),
        GetRolePropName(eRolePropRmb),
        GetRolePropName(eRolePropFat),
        GetRolePropName(eRolePropBattlePoint),
        GetRolePropName(eRolePropConstellVal),
        GetRolePropName(eRoleLastFinishCopy),
        GetRolePropName(eRoleDungTimes),
        GetRolePropName(eRoleEliteTimes),
        GetRolePropName(eRolefatFreshTime),

        GetRolePropName(eRolefatBuyTimes),
        GetRolePropName(eRoleEliteBuyTimes),
        GetRolePropName(eRolePvpAward),
        GetRolePropName(eRolePvpTimes),
        GetRolePropName(eRolePvpBuyTimes),
        GetRolePropName(eRolePropLvl),
        GetRolePropName(eRolePropRoleType),
        GetRolePropName(eRolePropRoleName),
        GetRolePropName(eRolePropSceneType),
        GetRolePropName(eRoleLoginAward),

        GetRolePropName(eRoleVipAward),
        GetRolePropName(eRoleMoneyTreeTimes),
        GetRolePropName(eRoleTeamCopyResetTimes),
        GetRolePropName(eRolePropCrossDay),
        GetRolePropName(eRolePropCrossWeek),
        GetRolePropName(eRolePropVipLvl),
        GetRolePropName(eRolePropVipExp),
        GetRolePropName(eRolePropDungLevel1),
        GetRolePropName(eRolePropDungLevel2),
        GetRolePropName(eRolePropDungLevel3),

        GetRolePropName(eRolePropDungResetTimes1),
        GetRolePropName(eRolePropDungResetTimes2),
        GetRolePropName(eRolePropDungResetTimes3),
        GetRolePropName(eRolePropVipLvlAward),
        GetRolePropName(eRolePropBanchat),
        GetRolePropName(eRoleIsInPvp),
        GetRolePropName(eRolePropPlayerAccount),
        GetRolePropName(eDailyQuestRefreshTimes),
        GetRolePropName(eDailyQuestCount),
        GetRolePropName(eRoleActivePetId),

//        GetRolePropName(eRoleTowerDefenseTimes),
        GetRolePropName(eRoleMysterCopyTimes),
        GetRolePropName(eRolePetPvpTimes),
		GetRolePropName(eRoleWorldChatTime),
        GetRolePropName(eRoleNextPvpResetTime),
        GetRolePropName(eRolePropWeapQua),
        GetRolePropName(eRolePropAnger),
        GetRolePropName(eLastLogin),
		GetRolePropName(eRoleBattleForce),
        GetRolePropName(eRolePropEnchantDust),
        GetRolePropName(eRolePropEnchantGold),
        GetRolePropName(eRoleLoginDays),
        GetRolePropName(eRoleAwakeLvl),
        NULL,
    };
    string allprops = StrJoin(props, &props[sizeof(props) / sizeof(const char*) - 1], " ");
    string cmd = "hmget role:";
    cmd.append(Utils::itoa(mInstID));
    cmd.append(" ");
    cmd.append(allprops.c_str());

    RedisResult result(redisCmd(cmd.c_str()));
    {
        setExp(result.readHash(0, 0));
        setGold(result.readHash(1, 0));
        setRmb(result.readHash(2, 0));
        setFat(result.readHash(3, 0));
        setBattlePoint(result.readHash(4, 0));
        setConstellVal(result.readHash(5, 0));
        setLastFinishCopy(result.readHash(6, 0));
        setDungTimes(result.readHash(7, 0));
        setEliteTimes(result.readHash(8, 0));
        setFatFreshTime(result.readHash(9, 0));

        setFatBuyTimes(result.readHash(10, 0));
        setEliteBuyTimes(result.readHash(11, 0));
        setPvpAward(result.readHash(12, 0));
        setPvpTimes(result.readHash(13, 0));
        setPvpBuyTimes(result.readHash(14, 0));
        setLvl(result.readHash(15, 0));
        BattleProp::setLvl(getLvl());
        setRoleType(result.readHash(16, 0));
        setRolename(result.readHash(17, ""));
        setSceneID(result.readHash(18, 0));
        setLoginAward(result.readHash(19, 0));

        setVipAward(result.readHash(20, 0));
        setMoneyTreeTimes(result.readHash(21, 0));
        setTeamCopyResetTimes(result.readHash(22, 0));
        setCrossDay(result.readHash(23, 0));
        setCrossWeek(result.readHash(24, 0));
        setVipLvl(result.readHash(25, 0));
        setVipExp(result.readHash(26, 0));
        setDungLevel1(result.readHash(27, 0));
        setDungLevel2(result.readHash(28, 0));
        setDungLevel3(result.readHash(29, 0));

        setDungResetTimes1(result.readHash(30, 0));
        setDungResetTimes2(result.readHash(31, 0));
        setDungResetTimes3(result.readHash(32, 0));
        setVipLvlAward(result.readHash(33, 0));
        setBanChat(result.readHash(34, 0));
        setIsInPvp(result.readHash(35, 0));
        setPlayerAccount(result.readHash(36, ""));
        setDailyQuestRefreshTimes(result.readHash(37, 0));
        setDailyQuestCount(result.readHash(38, 0));
        setActivePetId(result.readHash(39, 0));
        
        setMysticalCopyTimes(result.readHash(40, 0));
        setPetPvpTimes(result.readHash(41, 0));
		setNextWorldChatTime(result.readHash(42, 0));
        setNextPvpResetTime(result.readHash(43, 0));
        setWeaponQua(result.readHash(44, 0));
        setRoleAnger(result.readHash(45, 0));
        setLastLogintm(result.readHash(46, 0));
		setBattleForce(result.readHash(47, 0));
        setEnchantDust(result.readHash(48, 0));
        setEnchantGold(result.readHash(49, 0));
        setLoginDays(result.readHash(50, 0));
        
        setRoleAwakeLvl(result.readHash(51, 0));
    }

    mSex = (ObjSex)((mRoleType - 1) / 3);
    mJob = RoleType2Job(mRoleType);
}

void Role::saveWeaponQuality()
{
    if (mPlayerEquip == NULL) {
        return;
    }
    int weaponquality = 0;

    //多套装备的换装显示
    int equipId = getPlayerEquip()->GetItem(kEPTWeapon).item;
    if(GameFunctionCfg::getGameFunctionCfg().equipType == eMultipleEquipDrop)
    {
        //weaponquality = getPlayerEquip()->GetEquipInfo((int)kEPTWeapon).mode;
        weaponquality = ItemCfg::readInt(equipId, "model");
    }
    else //大陆版单装备的品质
    {
        //weaponquality = getPlayerEquip()->GetEquipInfo((int)kEPTWeapon).qua;
        weaponquality = ItemCfg::readInt(equipId, "qua");
    }
    
    setWeaponQua(weaponquality);
    
    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRolePropWeapQua), weaponquality);
}

void Role::saveRoleProp()
{
    RedisArgs args;

    string cmd = Utils::makeStr("role:%d", mInstID);
    RedisHelper::beginHmset(args, cmd.c_str());

    //保存怒气值
    RedisHelper::appendHmset(args, GetRolePropName(eRolePropAnger), getRoleAnger());

    //保存武器品质或模板
    if (mPlayerEquip != NULL) {

        int weaponquality = 0;

        //多套装备的换装显示
        int equipId = getPlayerEquip()->GetItem(kEPTWeapon).item;
        if(GameFunctionCfg::getGameFunctionCfg().equipType == eMultipleEquipDrop)
        {
            //weaponquality = getPlayerEquip()->GetEquipInfo((int)kEPTWeapon).mode;
            weaponquality = ItemCfg::readInt(equipId, "model");
        }
        else //大陆版单装备的品质
        {
            //weaponquality = getPlayerEquip()->GetEquipInfo((int)kEPTWeapon).qua;
            weaponquality = ItemCfg::readInt(equipId, "qua");
        }

        RedisHelper::appendHmset(args, GetRolePropName(eRolePropWeapQua), weaponquality);
    }

    RedisHelper::commitHmset(get_DbContext(), args);
}

void Role::makePropVerifyCode()
{
    mVerifyPropCode = calVerifyCode(&(this->mRealBattleProp));
}

void Role::sendRolePropInfoAndVerifyCode(bool isBattleBuf)
{
    notify_role_prop_info notify;

    notify.Phys = getPhys();
    notify.Capa = getCapa();
    notify.Inte = getInte();
    notify.Stre = getStre();

    notify.maxHp = mRealBattleProp.getMaxHp();
    notify.atk = mRealBattleProp.getAtk();
    notify.def = mRealBattleProp.getDef();
    notify.cri = mRealBattleProp.getCri();
    notify.dodge = mRealBattleProp.getDodge();
    notify.hit = mRealBattleProp.getHit();

    makePropVerifyCode();
    
    notify.verifyPropCode = mVerifyPropCode;
    notify.isBattleBuf = isBattleBuf;

    sendNetPacket(getSessionId(), &notify);
}

void Role::onSendPetPropAndVerifycode(Pet *pet, bool isBattleBuf)
{
    if (pet == NULL) {
        return;
    }
    notify_pet_prop_info notify;
    
    notify.petid = pet->petid;
    notify.isActive = pet->isactive;
    notify.Phys = pet->mPhys;
    notify.Capa = pet->mCapa;
    notify.Inte = pet->mInte;
    notify.Stre = pet->mStre;
    
    notify.maxHp = pet->mRealBattleProp.getMaxHp();
    notify.atk = pet->mRealBattleProp.getAtk();
    notify.def = pet->mRealBattleProp.getDef();
    notify.cri = pet->mRealBattleProp.getCri();
    notify.dodge = pet->mRealBattleProp.getDodge();
    notify.hit = pet->mRealBattleProp.getHit();
    
    notify.verifyPropCode = pet->verifyPropCode;
    notify.isBattleBuf = isBattleBuf;
    
    sendNetPacket(getSessionId(), &notify);
}

void Role::sendPetPropAndVerifycode(Pet* pet, bool isBattleBuf)
{
//    Pet* activepet = mPetMgr->getActivePet();

    SceneCfgDef* scene = SceneCfg::getCfg(getCurrSceneId());
    if (scene == NULL || scene->sceneType != stTown) {
        return;
    }

    if (pet == NULL) {
        return;
    }

    onSendPetPropAndVerifycode(pet, isBattleBuf);
}

void Role::setFightedFriendInfo(Role *friendInfo, int petBattle, int activePetid)
{
    if (mFightedFrinendVerifyInfo) {
        VerifyInfo* tmp = mFightedFrinendVerifyInfo;
        delete tmp;
    }

    mFightedFrinendVerifyInfo = new VerifyInfo;

    mFightedFrinendVerifyInfo->roleid = friendInfo->getInstID();
    mFightedFrinendVerifyInfo->atk = friendInfo->getAtk();
    mFightedFrinendVerifyInfo->def = friendInfo->getDef();
    mFightedFrinendVerifyInfo->maxHp = friendInfo->getMaxHp();
    mFightedFrinendVerifyInfo->cri = friendInfo->getCri();
    mFightedFrinendVerifyInfo->dodge = friendInfo->getDodge();
    mFightedFrinendVerifyInfo->hit = friendInfo->getHit();
    mFightedFrinendVerifyInfo->petBattle = petBattle;
    mFightedFrinendVerifyInfo->activePetid = activePetid;
}

void Role::setFightedFriendInfo(Robot *friendInfo)
{
    if (mFightedFrinendVerifyInfo) {
        VerifyInfo* tmp = mFightedFrinendVerifyInfo;
        delete tmp;
    }
    
    mFightedFrinendVerifyInfo = new VerifyInfo;
    
    mFightedFrinendVerifyInfo->roleid = friendInfo->mId;
    mFightedFrinendVerifyInfo->atk = friendInfo->mBattleProp.getAtk();
    mFightedFrinendVerifyInfo->def = friendInfo->mBattleProp.getDef();
    mFightedFrinendVerifyInfo->maxHp = friendInfo->mBattleProp.getMaxHp();
    mFightedFrinendVerifyInfo->cri = friendInfo->mBattleProp.getCri();
    mFightedFrinendVerifyInfo->dodge = friendInfo->mBattleProp.getDodge();
    mFightedFrinendVerifyInfo->hit = friendInfo->mBattleProp.getHit();
    mFightedFrinendVerifyInfo->petBattle = 0;
    mFightedFrinendVerifyInfo->activePetid = 0;
}

int64_t Role::getFightedFriendId()
{
    if (mFightedFrinendVerifyInfo) {
        return mFightedFrinendVerifyInfo->roleid;
    }

    return 0;
}

VerifyInfo* Role::getFightedFriendInfo()
{
    return mFightedFrinendVerifyInfo;
}

void Role::clearFriendInfo()
{
    if (mFightedFrinendVerifyInfo) {
        VerifyInfo* del = mFightedFrinendVerifyInfo;
        delete del;
        mFightedFrinendVerifyInfo = NULL;
    }
}

void Role::setEnemyVerifyInfo(BattleProp* enemy, int petBattle, int activePetid)
{
    mEnemyVerifyInfo = VerifyInfo();
    mEnemyVerifyInfo.atk = enemy->getAtk();
    mEnemyVerifyInfo.def = enemy->getDef();
    mEnemyVerifyInfo.maxHp = enemy->getMaxHp();
    mEnemyVerifyInfo.cri = enemy->getCri();
    mEnemyVerifyInfo.dodge = enemy->getDodge();
    mEnemyVerifyInfo.hit = enemy->getHit();
    mEnemyVerifyInfo.petBattle = petBattle;
    mEnemyVerifyInfo.activePetid = activePetid;
}

VerifyInfo& Role::getEnemyVerifyInfo()
{
    return mEnemyVerifyInfo;
}

void Role::insertRmbConsumedRecord(int rmb)
{
    getRmbConsumedRecord().insertRecord(rmb);
    save();
}

void Role::insertRechargeRecord(int rmb)
{
    getRechargeRecord().insertRecord(rmb);
    save();
}
void Role::clearEnemyVerifyInfo()
{
    mEnemyVerifyInfo = VerifyInfo();
}

void UpdateQuestProgress(std::string type, int target, int num, Role* role, bool updatedb)
{
    std::list<Quest*> typequests = QuestMgr::TypeQuests(type.c_str(), target);
    for (std::list<Quest*>::iterator iter = typequests.begin(); iter != typequests.end(); iter++)
    {
        std::map<int, std::vector<int> >::iterator fqpiter = role->mQuestProgress.find((*iter)->questId);
        if (fqpiter == role->mQuestProgress.end())
        {
            continue;
        }

        for (int vi = 0;vi != (*iter)->conditions.size();vi++)
        {
            int condtarget;
            int condnum;
            char condtype[32];
            sscanf((*iter)->conditions[vi].c_str(),"%s %d*%d", condtype, &condtarget, &condnum);
            if (type != condtype)
            {
                continue;
            }

            if (condtarget != target && condtarget != -1)
            {
                continue;
            }

            if (strcmp(type.c_str(), "dialog")==0)
            {
                fqpiter->second[vi] = num;
            }
            else if((strcmp(type.c_str(), "copycombo")== 0) || strcmp(type.c_str(), "copystar") == 0)
            {
                if(num >= condnum)
                {
                    fqpiter->second[vi] = condnum;
                }
            }
            else
            {
                if (condnum <= fqpiter->second[vi])
                {
                    continue;
                }

                fqpiter->second[vi] += num;
                if(fqpiter->second[vi] > condnum)
                {
                    fqpiter->second[vi] = condnum;
                }
            }

            if (updatedb)
            {
                doRedisCmd("hset questProgress:%d %d %d",role->getInstID(),(*iter)->questId,fqpiter->second[vi]);
            }
        }
    }
}

bool checkSkillStudy(vector<obj_studyable_skill>& skills, vector<int> skilltable)
{
    for (int i = 0; i < skills.size(); i++)
    {

        int index = (skills[i].skillID/100) % 100 ;

        for (int j = 0; j < skilltable.size(); j++)
        {
            if (index ==( (skilltable[j]/100) % 100 ))
            {
                skills[i].skillID = skilltable[j];
                skills[i].nextSkillID = skilltable[j] + 1;
                break;
            }
        }
    }
    return true;
}

//目前这个函数已经没用了  2013-12-17
vector<obj_studyable_skill> Role::GetStudyableSKills()
{
    vector<int> skillList;
    skillList.clear();

    //获取已学习的技能列表
    skillList = getSkillMod()->getStudySkillList();
    int lvl = getLvl();
    vector<obj_studyable_skill> skills;
    skills.clear();

    for (int i = 1; i < MAX_SKILL_COUNT + 1; i++) {
        int skillid = getStudySkill(this, i);
        obj_studyable_skill tmp;
        tmp.skillID = skillid;
        tmp.nextSkillID = skillid + 1;
        skills.push_back(tmp);
    }

    checkSkillStudy(skills, skillList);

    for (int i = 0; i < skills.size(); i++) {
        if (skills[i].skillID%100 !=0 ) {
            bool isExist;
            SkillCfgDef* skillData =  SkillCfg::getCfg(skills[i].nextSkillID, isExist);

            if (!isExist) {
                skills[i].nextSkillID = 0;
            }
            else if ( lvl < skillData->learnLvl) {
                skills[i].nextSkillID = -1;
            }
        }
        else
        {
            bool isExist;
            SkillCfgDef* skillData =  SkillCfg::getCfg(skills[i].nextSkillID, isExist);
            if (!isExist) {
                skills[i].nextSkillID = -1;
            }
            else if ( lvl < skillData->learnLvl) {
                skills[i].skillID = skills[i].nextSkillID - 1;
                skills[i].nextSkillID = -1;
            }
        }
    }

    /*for(int i=0; i<skills.size();i++)
    {
        cout<<"sudy:  "<<skills[i].skillID<<endl;
        cout<<"next:  "<<skills[i].nextSkillID<<endl;
    }*/
    return skills;
}

//void Role::SetDungTimes(int times)
//{
//    mDungTimes = times;
//    redisReply* rreply;
//    rreply=redisCmd("hmset role:%d dungtimes %d",getInstID(), times);
//    freeReplyObject(rreply);
//
//}
//
//void Role::SetEliteTimes(int times)
//{
//    mEliteTimes = times;
//    redisReply* rreply;
//    rreply=redisCmd("hmset role:%d elitetimes %d",getInstID(), times);
//    freeReplyObject(rreply);
//}


extern void sendLoginAwardState(Role* role);
int getMaxDungeonSweepLvl(Role* role, int maxLvl);


void getRoleObjDungeonState(Role* role, vector<obj_dungeonstate>& states)
{
    RoleDungeonState::Iterator iter = role->getDungeonState().begin();
    while (iter != role->getDungeonState().end()) {
        //int maxSweepLvl = getMaxDungeonSweepLvl(role, iter->second.mSweepLevel);
        
        obj_dungeonstate state;
        state.index = iter->second.mIndex;
        state.level = iter->second.mDungeonLevel;
        state.sweepinglevel = iter->second.mSweepLevel;
        //state.sweepinglevel = maxSweepLvl;
        state.resettimes = iter->second.mResetTimes;
        states.push_back(state);
        
        iter++;
    }
}

void Role::RoleFresh(int freshtime)
{
    RedisArgs args;
    RedisArgs delargs;

    char buf[64];
    sprintf(buf, "role:%d", mInstID);
    RedisHelper::beginHmset(args, buf);
    RedisHelper::beginDelKeys(delargs);

    setCrossDay(freshtime);
    RedisHelper::appendHmset(args, GetRolePropName(eRolePropCrossDay), freshtime);

    //fat buy reset
    mFatBuyTimes = 0;


    //reset elitecopy
    RoleResetEliteCopy();
    RedisHelper::appendHmset(args, GetRolePropName(eRoleEliteTimes), getEliteTimes());
    RedisHelper::appendHmset(args, GetRolePropName(eRoleEliteBuyTimes), "0");

    //刷新地下城
    RoleResetDungCopy();
    RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungResetTimes1), "0");
    RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungResetTimes2), "0");
    RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungResetTimes3), "0");
    RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungLevel1), "0");
    RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungLevel2), "0");
    RedisHelper::appendHmset(args, GetRolePropName(eRolePropDungLevel3), "0");
    RedisHelper::appendHmset(args, GetRolePropName(eRoleDungTimes), getDungTimes());

    //pvp reset
//    RoleResetPvp();

    //0点恢复
    //int fat = 50;
    //RoleCfgDef rolecfg = RoleCfg::getCfg(getJob(), getLvl());
    ///int maxFat = rolecfg.energe;

    //check_max(fat, maxFat - mFat);
    //check_min(fat, 0);
    //mFat += fat;
    //RedisHelper::appendHmset(args, GetRolePropName(eRolePropFat), Utils::itoa(getFat()).c_str());
    RedisHelper::appendHmset(args, GetRolePropName(eRolefatBuyTimes), "0");

    //重置组队副本
    mTeamCopyMgr->TeamCopyReset(getInstID(), getSessionId());

    sprintf(buf, "fightedfriends:%d", getInstID());
    RedisHelper::appendKeyname(delargs, buf);

    sprintf(buf, "teamcopy:%d", getInstID());
    RedisHelper::appendKeyname(delargs, buf);

    mTeamCopyResetTimes = 0;
    RedisHelper::appendHmset(args, GetRolePropName(eRoleTeamCopyResetTimes), "0");

    //日常任务
    refreshDailyQuest();
    RedisHelper::appendHmset(args, GetRolePropName(eDailyQuestCount), mDailyQuestCount);
    RedisHelper::appendHmset(args, GetRolePropName(eDailyQuestRefreshTimes), mDailyQuestRefreshTimes);

    //vip奖励
    if(getVipLvl())
    {
        setVipAward(1);
        RedisHelper::appendHmset(args, GetRolePropName(eRoleVipAward), "1");
    }

    //摇钱树刷新
    setMoneyTreeTimes(0);
    RedisHelper::appendHmset(args, GetRolePropName(eRoleMoneyTreeTimes), "0");

    //登陆奖励通知
    sendLoginAwardState(this);

    //reset friendDunge
    //2.5后不再这里刷新，改为玩家第一次进入好友地下城的时候检查
//    mFriendDungeAct->actFresh();

    //reset PrintCopy
//    mPrintCopyAct->PrintCopyReset(getInstID(), getSessionId());

//    sprintf(buf, "printcopy:%d", getInstID());
//    RedisHelper::appendKeyname(delargs, buf);
//
//    sprintf(buf, "printcopyreset:%d", getInstID());
//    RedisHelper::appendKeyname(delargs, buf);

    //reset PetCamp
    mPetCampAct->PetCampReset(getInstID(), getSessionId());

    sprintf(buf, "petcamp:%d", getInstID());
    RedisHelper::appendKeyname(delargs, buf);

    sprintf(buf, "petcampreset:%d", getInstID());
    RedisHelper::appendKeyname(delargs, buf);

    //reset Petdunge
    mPetDungeAct->petDungeReset();

    sprintf(buf, "petdungefriends:%d", getInstID());
    RedisHelper::appendKeyname(delargs, buf);

    //reset towerdefense
//    setTowerDefenseTimes(0);
//    RedisHelper::appendHmset(args, GetRolePropName(eRoleTowerDefenseTimes), Utils::itoa(getTowerDefenseTimes()).c_str());

    //抽奖次数更新
    setLotteryFree(0);
    setLotteryRmb(0);
    mLotteryHistory.clear();

    setDiceLotteryPosition(0);
    setDiceLotteryTime(0);
    mDiceLotteryHistory.clear();

    // 公会次数更新
    setGuildTrainTime(0);
    getGuildBlessMate().clear();
    //setGuildBless(0);
    setGuildBlessed(0);
    setGuildDnoateNum(0);
    setGuildItemTrainBossTime(0);
    setGuildRmbTrainBossTime(0);
    setGuildInspire(0);
    
    //重置聚魂冷却次数
    setRetinueSummonResetTimes(0);
	
	// 扭蛋次数更新
	getTwistEgg().refresh();

    // 神秘商人次数更新
    mMysteriousMgr->reFresh();

	// 公会商店次数更新
	mGuildStoreMgr->reFresh();

	// 答题每天更新
    mAnswerActivityMgr->dayRefresh();
    
    setQueenBless("");
    
    //
    mCapsuletoyMgr->dayRefresh();

    // 幻兽精英本更新
    setPetEliteBuyTimes(0);
    setPetEliteTimes(0);
    getPetEliteFightedPet().clear();

    //Mysticalcopy reset
    setMysticalCopyTimes(0);
    RedisHelper::appendHmset(args, GetRolePropName(eRoleMysterCopyTimes), getMysticalCopyTimes());

    //petpvptimes reset
    setPetPvpTimes(0);
    RedisHelper::appendHmset(args, GetRolePropName(eRolePetPvpTimes), getPetPvpTimes());
    
    //realpvp match times reset
//    setRealPvpMatchTimes(0);
    
    //重设同步组队副本收益次数
    setSyncTeamCopyAttendTimes(0);
    {
        string addAction = "daily_fresh";
        addLogActivityCopyEnterTimesChange(eActivityEnterTimesSyncTeamCopy, addAction.c_str());
    }
    
    //幻兽大冒险副本次数每天重置
    refreshPetAdventureTimes(0);
    refreshPetAdventureRobotPetRentedTimes(0);

    /**************************************************/
    //提交到数据库
    RedisHelper::commitHmset(get_DbContext(), args);
    RedisHelper::commitDelKeys(get_DbContext(), delargs);
    
    getDungeonState().refresh();
    
    getRmbConsumedRecord().refresh();
    getRechargeRecord().refresh();
    
    getCsPvpStateData().refresh();
    getWorshipData().refresh();
    
    saveNewProperty();
    
    getDailyScheduleSystem()->checkRefresh();


    ack_freshprop_byday ack;
    ack.fatbuytimes     = mFatBuyTimes;
    ack.elitebuytimes   = mEliteBuyTimes;
    ack.pvpbuytimes     = mPvpBuyTimes;
    getRoleObjDungeonState(this, ack.dungeonstates);

    //幻兽大冒险购买次数
    ack.petadventurebuytimes            = getPetAdventureBuyTimesRecordPerDay();
    ack.petadventurerobotpetbuyingrentedtimesperday = getPetAdventureRobotPetBuyingRentTimesRecordPerDay();
    
    ack.moneytreetimes  = getMoneyTreeTimes();
    ack.freelottery     = getLotteryFree();
    ack.buylottery      = getLotteryRmb();
    sendNetPacket(getSessionId(), &ack);

    //ack_lotterystate lotack;
    //lotack.buylottery = 0;
    //lotack.freelottery = 0;
    //sendNetPacket(getSessionId(), &lotack);

    // 更新登陆天数和角色奖励状态  added by wangck
    getRoleLoginDaysAwardMgr()->refreshLoginDays();
    
    notify_syn_dailycount notify;
    notify.count = mDailyQuestCount;
    sendNetPacket(mSessionId, &notify);

    notify_syn_petelitetimes nty_petelitetime;
    nty_petelitetime.times = getPetEliteCopyLeftTime();
    sendNetPacket(getSessionId(), &nty_petelitetime);
}

void Role::SaveLastCopy(int copyid)
{
    SceneCfgDef* sceneCfg = SceneCfg::getCfg(copyid);
    if (sceneCfg == NULL){
        log_error("cope id " << copyid << "not exist at Role::SaveLastCopy");
        return;
    }
    if (sceneCfg->sceneType == stCopy) {
        if (copyid <= getLastFinishCopy()) {
            return;
        }
        redisReply* rreply;
        mLastFinishCopy = copyid;
        rreply=redisCmd("hmset role:%d %s %d",getInstID(), GetRolePropName(eRoleLastFinishCopy), copyid);
        freeReplyObject(rreply);
        return;
    }

    if (sceneCfg->sceneType == stEliteCopy) {
        if (mFinishEliteCopy.find(copyid) != mFinishEliteCopy.end()) {
            return;
        }
        mFinishEliteCopy.insert(copyid);

        char tmp[128] = {0};
        sprintf(tmp, "sadd finishelitecopy:%d %d", getInstID(), copyid);
        doRedisCmd(tmp);
        return;
    }
}

void Role::backToCity()
{
    //下一步是回城镇
    setPreEnterSceneid(getSceneID());
    //setCurrSceneId(getSceneID());
    setBattleTime(0);
}

void Role::CheckEveryDayFirstLogin()
{
    time_t now = time(NULL);
    struct tm nowtm;
    localtime_r(&now, &nowtm);

    if (getCrossDay() <= now)
    {
        struct tm nextdaytm = nowtm;
        nextdaytm.tm_hour = 0;
        nextdaytm.tm_min = nextdaytm.tm_sec = 0;
        time_t nextdayt = mktime(&nextdaytm) + 3600 * 24 + ( getInstID() % 100);
        RoleFresh(nextdayt);

        sendFriendAwardMail();

        //判断是否跨周
        if (getCrossWeek() <= now)
        {
            int days = (7 - nowtm.tm_wday) % 7;
            time_t nextweekt = nextdayt + days * 3600 * 24;
            weeklyRefresh(nextweekt);
        }
    }
    if (getNextPvpResetTime() == 0) {
        setNextPvpResetTime(SPvpMgr.safe_getNextDistribAwardTime());
        saveProp(GetRolePropName(eRoleNextPvpResetTime), getNextPvpResetTime());
    }
    else if (now >= getNextPvpResetTime()) {
        RoleResetPvp();
        setNextPvpResetTime(SPvpMgr.safe_getNextDistribAwardTime());
        saveProp(GetRolePropName(eRoleNextPvpResetTime), getNextPvpResetTime());
    }
}

void Role::weeklyRefresh(int refreshTime)
{
    RedisArgs args;
    RedisHelper::beginHmset(args, strFormat("role:%d", mInstID).c_str());
    RedisHelper::appendHmset(args, GetRolePropName(eRolePropCrossWeek), Utils::itoa(refreshTime).c_str());
    RedisHelper::appendHmset(args, GetRolePropName(eRoleLoginAward), "0");
    RedisHelper::commitHmset(get_DbContext(), args);

    setCrossWeek(refreshTime);
    setLoginAward(0);
    // 答题每周更新
    mAnswerActivityMgr->weekRefresh();
	
	// 分享奖励每周刷新
	setShareAwardWeekly(0);
	
	mMidastouchMgr->weeklyRefresh();
}

void Role::calcMaxFatLimit()
{
    int maxFat = RoleCfg::getCfg(getJob(), getLvl()).getEnerge();
    maxFat += calHeraldryFatAddition( getInstID());			// 公会纹章加成 2013.9.16
	maxFat += getHonorMgr()->CalHeraldryFatAddition();		// 称号加成 2013.10.18
    
    setMaxFat(maxFat);
}

void Role::RoleFreshFat(int addPoint, int freshTime)
{

    if (freshTime) {
        char rcmd[128];
        sprintf(rcmd, "hmset role:%d %s %d", getInstID(), GetRolePropName(eRolefatFreshTime), freshTime);
        mFatFreshTime = freshTime;
        doRedisCmd(rcmd);
    }

    int maxFat = getMaxFat();

    if (getFat() >= maxFat) {
        return;
    }

    addPoint = mFat + addPoint;
    addPoint = addPoint > maxFat ? (maxFat - mFat) : (addPoint - mFat);

    if (addPoint) {
        addFat(addPoint);
    }
}

void Role::RoleFreshProp(int propType, int propVal)
{
    switch (propType) {
        case eRoleEliteTimes:
            ;
            break;
        case eRoleDungTimes:
            ;
            break;
        default:
            break;
    }
}


void Role::onGetItems(const ItemArray& items)
{
    for (int i = 0; i < items.size(); i++) {
        ItemCfgDef* itemCfg = ItemCfg::getCfg(items[i].item);
        if (itemCfg == NULL) {
            continue;
        }
        int type = itemCfg->ReadInt("type");
        if (type == kItemPropStone) {
            int stoneLvl = itemCfg->ReadInt("lvl");
            if (stoneLvl > 7) {
                broadcastGetStone(getRolename(), items[i].item);
            }
            
            g_GiftActivityMgr.check(eTargetType_ComposeLvlStone, stoneLvl, 0, this);
        } else if (type == KItemFashionCollectMaterial) {
            UpdateQuestProgress("fashioncollectmaterial", -1, items[i].count, this, true);
        }
    }
}

void Role::onUseItems(const ItemArray& items)
{
}

//void Role::updateBackBag(const GridArray &grids, const ItemArray &items, bool isAdd, const char* desc, bool notify)
//{
//    if (isAdd) {
//        LogMod::addLogGetItem(mInstID, mRolename.c_str(), mSessionId, items, desc);
//        onGetItems(items);
//    } else {
//        LogMod::addUseItemLog(this, items, desc);
//    }
//    
//    getBackBag()->UpdateBackBag(grids);
//    getBackBag()->Store(mInstID, grids);
//    
//    if (notify) {
//        SendBackBagNotify(this, grids);
//    }
//}

void Role::RoleResetEliteCopy()
{
    mEliteBuyTimes = 0;

    RoleCfgDef cfg = RoleCfg::getCfg(getJob(), getLvl());
    int maxTimes = cfg.getElitefreeTimes();

    if (mEliteTimes > maxTimes) {
        return;
    }

    setEliteTimes(maxTimes);
    notify_syn_elitetimes notify;
    notify.times = mEliteTimes + getIncrEliteCopyTimes();
    sendNetPacket(getSessionId(), &notify);
}

int Role::getPetEliteCopyLeftTime()
{
    const RoleCfgDef& cfg = RoleCfg::getCfg(getJob(), getLvl());
    int freeTimes = cfg.getPetElitefreeTimes();

    // 剩余次数 ＝ 免费的 + 购买的 - 已打的
    int left =  freeTimes + getPetEliteBuyTimes() - getPetEliteTimes();

    return  ( left < 0 ? 0 : left );
}


void Role::RoleResetDungCopy()
{
    mDungResetTimes1 = mDungResetTimes2 = mDungResetTimes3 = 0;
    mDungLevel1 = mDungLevel2 =mDungLevel3 = 0;
}

void Role::RoleResetPvp()
{
    mPvpBuyTimes = 0;

    mPvpAward = 1;

    int freeTimes = PvpCfg::getfreeTimes();
    if (mPvpTimes <= freeTimes) {
        setPvpTimes(freeTimes);
        
        string describe = Utils::makeStr("daily_fresh");
        addLogActivityCopyEnterTimesChange(eActivityEnterTimesPvp, describe.c_str());
    }

    RedisArgs args;
    char buf[64];
    sprintf(buf, "role:%d", mInstID);
    RedisHelper::beginHmset(args, buf);

    RedisHelper::appendHmset(args, GetRolePropName(eRolePvpBuyTimes), "0");
    RedisHelper::appendHmset(args, GetRolePropName(eRolePvpAward), "1");
    RedisHelper::appendHmset(args, GetRolePropName(eRolePvpTimes), getPvpTimes());

    //提交到数据库
    RedisHelper::commitHmset(get_DbContext(), args);
    notify_syn_pvptimes notify;
    notify.times = mPvpTimes;
    sendNetPacket(getSessionId(), &notify);
}




void Role::onEnterScene()
{
    if (mScene)
    {
        mSceneID = mScene->getID();
        setCurrSceneId(mSceneID);
        
        notify_enter_city notify;
        getRoleInfo(notify.roleinfo);
        mScene->broadcast(&notify, mInstID);
        
    }
}

void Role::onLeaveScene()
{
    if (mScene)
    {
        notify_leave_city leavenotify;
        leavenotify.roleid = mInstID;
        leavenotify.rolename = mRolename;
        mScene->broadcast(&leavenotify, mInstID);
        
        mSceneID = 0;
        setCurrSceneId(mSceneID);
    }
}


void Role::addPetExp(int petid, int exp, const char* comeFrom)
{
    Pet* pet = mPetMgr->getPet(petid);
    if (pet != NULL)
    {
        int preTotalExp = pet->totalexp;
        pet->addExp(exp, mLvl);
        notify_sync_pet_exp notify;
        notify.petid = pet->petid;
        notify.lvl = pet->lvl;
        notify.exp = pet->exp;
        sendNetPacket(getSessionId(), &notify);

        //更新出战宠属性
        sendPetPropAndVerifycode(pet);
        
        if (exp) {
            LogMod::addLogAddPetExp(this, petid, pet->petmod, preTotalExp, exp, pet->totalexp, pet->lvl, comeFrom);
        }
    }
}

extern void SendDailyQuest(Role* role);
//计算任务个数
extern void CountQuest(std::vector<int>& doingquest,int& outmaincount,int& outbranchcount,int& outdailycount);

void Role::loadQuestData()
{
    mQuestProgress.clear();
    mFinishQuestIds.clear();

    {
        redisReply* rreply = redisCmd("hgetall questProgress:%d", mInstID);
        for (int i = 0; i < rreply->elements/2; i++) {
            int questid = atoi((rreply->element[i*2])->str); //id
            std::vector<int> prog;
            StringTokenizer token((rreply->element[i*2+1])->str,";"); //prog
            for (StringTokenizer::Iterator iter = token.begin(); iter != token.end(); ++iter) {
                prog.push_back(atoi(iter->c_str()));
            }
            mQuestProgress.insert(make_pair(questid, prog));
        }
        freeReplyObject(rreply);

        mFinishQuestIds.clear();
        rreply = redisCmd("smembers questFinish:%d", mInstID);
        for (int i = 0; i < rreply->elements; ++i) {
            mFinishQuestIds.push_back(atoi(rreply->element[i]->str));
        }
        freeReplyObject(rreply);
    }

    SendDailyQuest(this);
}

void Role::addDailyQuestCount(int count)
{
    mDailyQuestCount += count;
    saveProp(GetRolePropName(eDailyQuestCount), mDailyQuestCount);
}

void Role::addDailyQuestRefreshTimes(int times)
{
    mDailyQuestRefreshTimes += times;
    saveProp(GetRolePropName(eDailyQuestRefreshTimes), mDailyQuestRefreshTimes);
}

void Role::refreshDailyQuest()
{
    std::vector<int> doingquest;
    doingQuest(doingquest);
    
    int maincount = 0;
    int branchcount = 0;
    int dailycount = 0;
    CountQuest(doingquest, maincount, branchcount, dailycount);
    
    setDailyQuestCount(dailycount);
    setDailyQuestRefreshTimes(0);
    
    if (dailycount == 0)
    {
        create_cmd(dailyquest, dailyquest);
        dailyquest->roleid = mInstID;
        sendCmdMsg(Game::MQ, dailyquest);
    }
}

/*-----------------------------*/

void Role::RolePvpInit()
{
    mPvpAward = mPvpBuyTimes = mPvpTimes = 0;
}

void Role::stopPvp()
{
    create_global(pvpEraseRoleInpvp, erase);
    erase->bechallengeid = getBechallengeId();
    erase->roleid = MAKE_OBJ_ID(kObjectRole, getInstID());
    sendGlobalMsg(Global::MQ, erase);

    setBechallengeId(0);
}

void Role::LoadFinisEliteCopy()
{
    mFinishEliteCopy.clear();
    char buf[256];
    sprintf(buf, "smembers finishelitecopy:%d", getInstID());

    redisReply* reply = redisCmd(buf);
    if (reply == NULL)
    {
        return;
    }

    for (int i = 0; i < reply->elements; i++)
    {
        mFinishEliteCopy.insert(atoi(reply->element[i]->str));
    }

    freeReplyObject(reply);
}

bool Role::IsEliteCopyFinish(int copyid)
{
    return mFinishEliteCopy.find(copyid) != mFinishEliteCopy.end() ? true : false;
}

int Role::CheckMoneyEnough(int cost, int moneyType, const char* usage)
{
    if (cost < 0) {
        return -1;
    }

    switch (moneyType) {
        case eRmbCost:
            if (cost > getRmb()) {
                return CE_SHOP_RMB_NOT_ENOUGH;
            }
            addRmb( -cost, usage);
            return CE_OK;
        case eGoldCost:
            if (cost > getGold()) {
                return CE_SHOP_GOLD_NOT_ENOUGH;
            }
            addGold( -cost, usage );
            return CE_OK;
        default:
            return -1;
    }
}

// // 是否够钱消费，不够钱返回0，够钱返回钱数
int Role::hasEnoughMoney(int buyid,int time)
{
    assert( buyid >=0 && time >=0);

    int costType = 0 ;
    int cost =  BuyCfg::getCost(buyid,time, costType);
    assert(cost>0);

    if(costType == eRmbCost && getRmb() >= cost)
    {
        return cost;
    }

    if(costType == eGoldCost && getGold() >= cost)
    {
        return cost;
    }

    return 0;
}

// // 为某件事消费，必须保证够钱
void Role::consumeMoney(int buyid,int time,const char* usage)
{
    assert(buyid >= 0 && time >=0 );
    xyassertf( hasEnoughMoney(buyid,time) ,"%d , %d , %s, %d , %d ",buyid,time,usage,getRmb(),getGold() );


    int costType = 0 ;
    int money =  BuyCfg::getCost(buyid,time, costType);

    assert( CE_OK == CheckMoneyEnough(money,costType,usage) );
}

void Role::addVipExp(int val, const char* comefrom)
{
    mVipExp += val;
    viplvlup();
    doRedisCmd("hmset role:%d %s %d %s %d",getInstID(), GetRolePropName(eRolePropVipLvl), getVipLvl(),
               GetRolePropName(eRolePropVipExp), getVipExp());

    LogMod::addLogGetVipexp(getInstID(), getRolename().c_str(), getSessionId(), val, comefrom, getVipExp(), getVipLvl());

}

void Role::viplvlup()
{
    if (getVipLvl() >= VipPrivilegeCfg::getMaxLvl()) {
        notify_syn_vipexp notify;
        notify.vipexp = getVipExp();
        sendNetPacket(getSessionId(), &notify);
        return;
    }

    VipPrivilegeCfgDef* vipdata = VipPrivilegeCfg::getCfg(getVipLvl() + 1);
    if (vipdata != NULL) {
        if ( mVipExp < vipdata->needexp) {
            notify_syn_vipexp notify;
            notify.vipexp = getVipExp();
            sendNetPacket(getSessionId(), &notify);
            return;
        }
    }

    mVipLvl++;
    updateVipLvlAward(mVipLvl, true);

    broadcastVipLvl(mRolename, mVipLvl);
    
    //处理角色登陆天数奖励状态的刷新
    getRoleLoginDaysAwardMgr()->refreshRoleAwardState();

    if (mVipLvl == 1) {
        setVipAward(1);
        saveProp(GetRolePropName(eRoleVipAward), "1");
    }

    notify_syn_viplvl notify;
    notify.viplvl = mVipLvl;
    notify.vipaward = mVipAward;
    sendNetPacket(getSessionId(), &notify);

    //处理各种变化
    notify_syn_petadventurerobotpetrenttimesremain PetAdventureRobotPetRentTimesRemainNotify;
    PetAdventureRobotPetRentTimesRemainNotify.times = calPetAdventureRobotPetRentTimesRemain();
    sendNetPacket(getSessionId(), &PetAdventureRobotPetRentTimesRemainNotify);
    
	// 日志
	Xylog log(eLogName_RoleVipUp, getInstID());
	log<<mVipLvl<<getVipExp();

    //继续判断
    viplvlup();
}

int getMaxDungeonSweepLvl(Role* role, int maxLvl)
{
    int battleForce = role->getBattleForce();
    if (maxLvl <= 0) {
        return 0;
    }
    
    int maxSweepLvl = maxLvl;
    
    int maxFloor = maxLvl % 100;
    int i = maxFloor;
    for (; i > 0; i--, maxSweepLvl--) {
        //          获取对应层ID的推荐战斗力
        SceneCfgDef* sceneCfg = SceneCfg::getCfg(maxSweepLvl);
        if (sceneCfg == NULL || sceneCfg->sceneType != stDungeon) {
            continue;
        }
        int dungFightPoint = sceneCfg->fightpoint;
        
        if (battleForce >= dungFightPoint) {
            break;
        }
    }
    
    if (i == 0) {
        maxSweepLvl = 0;
    }
    
    return maxSweepLvl;
}

void sendChangedDungeonState(Role* role)
{
    ack_get_dungeon_state ack;
    getRoleObjDungeonState(role, ack.dungeonstates);
    role->send(&ack);
}

void sendChangedDungeonState(Role* role, RoleDungeonState::Iterator iter)
{
    //int maxSweepLevel = getMaxDungeonSweepLvl(role, iter->second.mSweepLevel);
    
    notify_dungeon_state notify;
    obj_dungeonstate state;
    state.index = iter->second.mIndex;
    state.resettimes = iter->second.mResetTimes;
    state.level = iter->second.mDungeonLevel;
    state.sweepinglevel = iter->second.mSweepLevel;
    //state.sweepinglevel = maxSweepLevel;
    notify.states.push_back(state);
    role->send(&notify);
}

void Role::saveDungLevelRecord(int dungcopyeid, int index)
{
    RoleDungeonState::Iterator iter = getDungeonState().setDungeonLevel(index, dungcopyeid);
    if (dungcopyeid > iter->second.mSweepLevel) {
        iter->second.mSweepLevel = dungcopyeid - 1;
    }
    save();
    
    sendChangedDungeonState(this, iter);
}

void Role::addDungResetTimes(int times, int index)
{
    RoleDungeonState::Iterator iter = getDungeonState().addResetTimes(index, times);
    save();
    
    sendChangedDungeonState(this, iter);
}

int Role::getDungResetTimes(int index)
{
    return getDungeonState().getResetTimes(index);
}

int Role::getDungeLevelRecord(int index)
{
    return getDungeonState().getDungeonLevel(index);
}


int Role::getDungeMaxSweepingLevel(int index)
{
    return getDungeonState().getSweepLevel(index);
}

void Role::setDungMaxSweepingLevel(int level, int index)
{
    RoleDungeonState::Iterator iter = getDungeonState().setSweepLevel(index, level);
    save();
    
    sendChangedDungeonState(this, iter);
}

void Role::updateVipLvlAward(int viplvl, bool canget)
{
    std::bitset<32> award(mVipLvlAward);
    award.set(viplvl, canget);
    mVipLvlAward = award.to_ulong();
    doRedisCmd("hset role:%d %s %d", getInstID(), GetRolePropName(eRolePropVipLvlAward), mVipLvlAward);

    notify_syn_viplvlaward notify;
    notify.viplvlaward = mVipLvlAward;
    sendNetPacket(getSessionId(), &notify);
}

bool Role::checkVipLvlAward(int viplvl)
{
    std::bitset<32> award(mVipLvlAward);
    return award.test(viplvl);
}

void Role::UploadPaihangData(int optionType)
{
    doRedisCmd("hset role:%d bat %d", getInstID(), getBattleForce());

    create_global(updatePaihangDB, paihang);
    paihang->roleid     = getInstID();
    paihang->lvl        = getLvl();
    paihang->battle     = getBattleForce();
    paihang->consume    = getRmbConsumedRecord().getRecord();
    paihang->recharge   = getRechargeRecord().getRecord();

    paihang->optionType = optionType;
    sendGlobalMsg(Global::MQ, paihang);
}

void Role::UploadPetPaihangData(int optionType, int petid, int petbattle)
{
    create_global(updatePetPaihangDB, paihang);
    paihang->optionType = optionType;
    paihang->petid = petid;
    paihang->petBattle = petbattle;
    sendGlobalMsg(Global::MQ, paihang);
}

void DeleteRoleRoleInfoInDB(int roleid, int sessionid)
{
    // 清掉在公会的数据，相当于玩家离开公会
    SGuildMgr.on_req_guild_leave(roleid);
    
    SSortMgr.safe_delRoleInPaihang(roleid);

    SPvpMgr.safe_DeleteRoleInpvp(roleid);

    RedisResult result(redisCmd("hmget role:%d %s %s %s %s %s", roleid, GetRolePropName(eRolePropPetList), GetRolePropName(eRolePropLvl), GetRolePropName(eRolePropVipLvl), GetRolePropName(eRolePropVipExp), GetRolePropName(eRolePropRoleName)));

    string petlist = result.readHash(0, "");
    vector<string> petids = StrSpilt(petlist, ",");
    vector<int> petIdList;
    for (int i = 0; i < petids.size(); i++)
    {
        int petid = atoi(petids[i].c_str());
        if (petid > 0)
        {
            Pet* pet = new Pet(petid);
            if (pet == NULL) {
                continue;
            }
            pet->load(get_DbContext());

            if (pet->isactive)
            {
                
                SSortMgr.safe_delPetInPaihang(petid);
                
                break;
            }
            delete pet;
            petIdList.push_back(petid);
        }
    }
    PetMgr::delPetsFromDB(petIdList);

    //backup for seven days
    vector<string> rolekeys;
    rolekeys.push_back("role");
    rolekeys.push_back("questProgress");
    rolekeys.push_back("questFinish");
    rolekeys.push_back("friends");
    rolekeys.push_back("finishelitecopy");
    rolekeys.push_back("fightedfriends");
    rolekeys.push_back("BackBag");
    rolekeys.push_back("PlayerEquip");

    doRedisCmd("MULTI");
    for (int i = 0; i < rolekeys.size(); i++)
    {
        string newkey = "backup:";
        newkey.append(rolekeys[i]);
        doRedisCmd("rename %s %s", rolekeys[i].c_str(), newkey.c_str());
        doRedisCmd("expire %s %d", newkey.c_str(), 7 * 24 * 3600);
    }
    doRedisCmd("EXEC");

    /*RedisArgs args;
    RedisHelper::beginDelKeys(args);
    RedisHelper::appendKeyname(args, strFormat("role:%d", roleid).c_str());
    RedisHelper::appendKeyname(args, strFormat("questProgress:%d", roleid).c_str());
    RedisHelper::appendKeyname(args, strFormat("questFinish:%d", roleid).c_str());
    RedisHelper::appendKeyname(args, strFormat("friends:%d", roleid).c_str());
    RedisHelper::appendKeyname(args, strFormat("finishelitecopy:%d", roleid).c_str());
    RedisHelper::appendKeyname(args, strFormat("fightedfriends:%d", roleid).c_str());
    RedisHelper::appendKeyname(args, strFormat("BackBag:%d", roleid).c_str());
    RedisHelper::appendKeyname(args, strFormat("PlayerEquip:%d", roleid).c_str());
    RedisHelper::commitDelKeys(get_DbContext(), args);*/

    int lvl = result.readHash(1, 0);
    int viplvl = result.readHash(2, 0);
    int vipexp = result.readHash(3, 0);
    string rolename = result.readHash(4, "");

    LogMod::addLogDelRole(roleid, rolename.c_str(), sessionid, lvl, viplvl, vipexp);




}

void StoreOfflineItem(int roleid, const char* itemdesc)
{
    RedisResult result(redisCmd("hget role:%d offlineitems", roleid));
    string stored = result.readStr();
    stored.append(itemdesc);
    doRedisCmd("hset role:%d offlineitems %s", roleid, stored.c_str());
}


int  GetRoleIdByName(const char* name)
{
    RedisResult result(redisCmd("get rolename:%s", name));
    int roleid = result.readInt();
    return roleid;
}




void Recharge(int roleid, Role* role, int rmb, const char* platform, const char* billno)
{
    string roleName;

    if(role)
    {
        role->addRmb(rmb, platform);
        role->addVipExp(rmb, platform);
        roleName = role->getRolename();
        

        g_GiftActivityMgr.check(eTargetType_Recharge, rmb, 0, role);
    }
    else
	{
        //上面已判断role已经存在
        RedisResult result(redisCmd("hget role:%d %s", roleid, GetRolePropName(eRolePropRmb)));
        int mRmb = result.readInt();
        mRmb += rmb;
        check_range(mRmb, 0, 0x7FFFFFFF);
        doRedisCmd("hmset role:%d %s %d",roleid, GetRolePropName(eRolePropRmb), mRmb);

        result.setData(redisCmd("hget role:%d %s", roleid,GetRolePropName(eRolePropVipExp)));
        int mVipExp = result.readInt();
        mVipExp += rmb;
        check_range(mVipExp, 0, 0x7FFFFFFF);
        doRedisCmd("hmset role:%d %s %d", roleid, GetRolePropName(eRolePropVipExp), mVipExp);

        result.setData(redisCmd("hmget role:%d %s %s %s %s", roleid,
								GetRolePropName(eRolePropPlayerAccount),
								GetRolePropName(eRolePropRoleName),
                                GetRolePropName(eRolePropLvl),
								GetRolePropName(eRoleBattleForce)
								));
		
        string playerAccount = result.readHash(0, "");
        string rolename = result.readHash(1, "");
		string lvl = result.readHash(2, "");
        string atk = result.readHash(3, "");
        string getfrom = platform;
        getfrom.append("_offline");

        if (!playerAccount.empty() && !rolename.empty()) {
            LogMod::addLogGetRmb(roleid,
								 rolename.c_str(),
								 playerAccount.c_str(),
								 platform,
								 rmb,
								 getfrom.c_str(),
								 mRmb,
								 Utils::safe_atoi(lvl.c_str()),
								 Utils::safe_atoi(atk.c_str())
								 );
        }

        roleName = rolename;

        vector<string> params;
        params.push_back(getNameByTargetType(eTargetType_Recharge));
        params.push_back(strFormat("%d", rmb));
        params.push_back("0");
        addRoleOfflineAction(roleid, "GiftActivity", params);
    }
	
	Xylog log(eLogName_Payment, roleid);
	log << rmb << platform << billno;

    rechargeAwardActivity(roleid,roleName.c_str(),rmb , NULL != role);
    onceRechargeAwardActivity(roleid,roleName.c_str(),rmb);
    Recharge_AccuRechargeAwardAct(roleid,rmb);
    
    Recharge_MonthRechargeAwardAct(roleid,roleName.c_str(),rmb,time(NULL));
    
    do
    {
        ServerGroupCfgDef*  def = g_RechargeRankListGroupCfg.getServerGroupCfgDefByServerid(Process::env.getInt("server_id"));
        if (def == NULL) {
            break;
        }
        
        int beginTime = def->mBeginTime;
        int endTime   = def->mEndTime;
        
        int currentTime = time(NULL);
        if (beginTime <= currentTime && currentTime <= endTime) {
            
            if (role != NULL) {
                role->insertRechargeRecord(rmb);
                role->UploadPaihangData(eUpdate);
                
                notify_syn_accumulateRechargeRecord notify;
                notify.record = role->getRechargeRecord().getRecord();
                sendNetPacket(role->getSessionId(), &notify);
            }
            else {
                RedisResult result(redisCmd("hget role:%d %s", roleid, ""));
                string rechargeRecord = result.readStr();
                
                Json::Reader reader;
                Json::Value oldValue;
                map<int, int> mapRecord;

                if (reader.parse(rechargeRecord, oldValue)) {
                    try
                    {
                        for (int i = 0; i < oldValue.size(); i++) {
                            mapRecord.insert(make_pair(oldValue[i]["Time"].asInt(), oldValue[i]["rechargerecord"].asInt()));
                        }
                    }
                    catch(...)
                    {
                    }
                }

                int currentTime = time(NULL);
                mapRecord.insert(make_pair(currentTime, rmb));
                
                Json::Value value;
                map<int, int>::iterator it;
                int sum = 0;
                
                for (it = mapRecord.begin(); it != mapRecord.end(); it++) {
                    Json::Value tmp;
                    tmp["Time"] = it->first;
                    tmp["rechargerecord"] = it->second;
                    
                    sum += it->second;
                    value.append(tmp);
                }
                
                Json::Value tmp;
                tmp["Sum"] = sum;
                value.append(tmp);
                
                doRedisCmd("hset role:%d %s %s", roleid, "rechargerecord",  xyJsonWrite(value).c_str());
                
                notify_syn_accumulateRechargeRecord notify;
                notify.record = sum;
                sendNetPacket(role->getSessionId(), &notify);
                
                doRedisCmd("zadd paihang:recharge %d %d", roleid, sum);
                
                RedisResult sortInfo( redisCmd("hmget role:%d rolename lvl bat roletype rechargerecord rmbconsumedrecord", roleid) );
                string rolename             = sortInfo.readHash(0, "");
                int rolelvl                 = Utils::safe_atoi(result.readHash(1, "").c_str());
                int battle                  = Utils::safe_atoi(result.readHash(2, "").c_str());
                int job                     = Utils::safe_atoi(result.readHash(3, "").c_str());
                string sRechargeRecord      = sortInfo.readHash(4, "");
                string sRmbconsumedrecord   = sortInfo.readHash(5, "");
                
                if ((!rolename.empty()) && rolelvl > 0) {
                    
                    Json::Reader reader;
                    Json::Value value;
                    int recharge = 0;
                    if (reader.parse(sRechargeRecord, value) && value.type() == Json::arrayValue) {
                        for (int i = 0 ; i < value.size(); i++) {
                            if (0 < value[i]["Sum"].asInt()) {
                                recharge = value[i]["Sum"].asInt();
                                break;
                            }
                        }
                    }
                    
                    int consume = 0;
                    if (reader.parse(sRmbconsumedrecord, value) && value.type() == Json::arrayValue) {
                        for (int i = 0 ; i < value.size(); i++) {
                            if (0 < value[i]["Sum"].asInt()) {
                                consume = value[i]["Sum"].asInt();
                                break;
                            }
                        }
                    }
                    
                    SortInfo *info  = new SortInfo;
                    info->roleid    = roleid;
                    info->rolename  = rolename;
                    info->lvl       = rolelvl;
                    info->battle    = battle;
                    info->job       = job;
                    info->objType   = kObjectRole;
                    info->recharge  = recharge;
                    info->consume   = consume;
                
                    vector<SortInfo*> ret(1, info);
                    //暂时只知道 在 global 线程里  这样拿 centerserver 的sessionid 以后设计框架完善了 再改, esortend 用来标示这次发送是 update
                    sendPaihangRoleDbData2CenterServer(CenterClient::instance()->getSession(), eSortEnd, ret);
                    
                    delete info;
                }
            }
        }
        
    }while (false);

	
	SMidastouchMou.rechargeToReward(roleid, rmb);
}

void Role::addToPvp()
{
    create_global(addRoleToPvp, pvp);
    pvp->roleid = MAKE_OBJ_ID(kObjectRole, getInstID());
    sendGlobalMsg(Global::MQ, pvp);
}

int Role::checkClientBatttle(Obj *target, int damageType, int averageHurt)
{
//    float rol1 = rand()%100;
    int dmg = 0;
    float DefPro = target->BattleProp::getDef() / (float)(target->BattleProp::getDef() + target->getLvl()*60 + 500);

    dmg = getAtk()*(1.0f - DefPro); // ( 0.95f + 0.1 *(rol1/100));

    if (damageType == eCri) {
        dmg *= 1.5;
    }

    if (averageHurt < dmg) {
        return 0;
    }

    int errorRate = ((averageHurt - dmg) / dmg) * 100;

    if (errorRate > 30) {
        return -1;
    }

    return 0;
}

void Role::pushLotteryHistory(int item , int amount ,int rmb ,  bool isDice )
{
    IntMapJson* pHistory = &mLotteryHistory;
    if( isDice )
    {
        pHistory = &mDiceLotteryHistory;
    }
    IntMapJson& history = *pHistory;

    while(history.size() > 30)
    {
        int key = history.begin()->first;
        history.erase( key );
    }

    int id = history.nextIntKey();
    history[id]["item"] = item;
    history[id]["amount"] = amount;
    history[id]["rmb"] = rmb;
    history[id]["date"]= (int)time(0);
}

void Role::onEnterGame()
{
    
}

//主线程检测角色是否存在于数据库
bool isRoleExistInDbForMainThread(int roleid)
{
    Role* role = SRoleMgr.GetRole(roleid);
    if(role != NULL)
        return true;

    redisReply* rreply = redisCmd("exists role:%d", roleid);
    int hasrole = (int)rreply->integer;
    freeReplyObject(rreply);
    if (hasrole == 0) {
        return false;
    }else{
        return true;
    }
}


static void sendDissmissGuildMail(const char* guildname , const char* rolename)
{
    MailFormat *f =  MailCfg::getCfg("guild_dismiss");
    if (f)
    {
        string mail_content = f->content;
        if (1 != find_and_replace(mail_content, 1, guildname))
        {
            log_warn("mail content format error. [guild_dissmiss]");
        }
        
        sendMail(0, f->sendername.c_str(), rolename, f->title.c_str(), mail_content.c_str(), "", "");
    }
}

static void takeOffFashionFromEquip(Role* role)
{
    PlayerEquip* equip = role->getPlayerEquip();
    ItemGroup item = equip->GetItem(kEPTFashion);
    if (item.item > 0 && item.count > 0)
    {
        bool ret = true;
        GridArray grids;
        ItemArray items;
        items.push_back(item);
        if (role->preAddItems(items, grids) == CE_OK)
        {
//            backBag->UpdateBackBag(grids);
//            backBag->Store(role->getInstID(), grids);
            role->playerAddItemsAndStore(grids, items, "", false);
        }
        else
        {
            string sender = "system";
            string title = "fashion";
            string content = "";
            string attachment = strFormat("item %d*1", item.item);
            MailFormat *f = MailCfg::getCfg("OldFashionSendBack");
            if (f)
            {
                sender = f->sendername;
                title = f->title;
                content = f->content;
            }
            
            if (!sendMail(0, sender.c_str(), role->getRolename().c_str(), title.c_str(), content.c_str(), attachment.c_str(), ""))
            {
                ret = false;
            }
        }
        
        if (ret)
        {
            ItemGroup noEquip;
            equip->SetItem(kEPTFashion, noEquip);
            equip->Store(role->getInstID(), kEPTFashion);
        }
    }
}

//角色上线
void GlobalRoleOnline(Role* role)
{

    syncClinetTime(role);
	// 背包物品转换
	ItemChangeMgr::itemChange(role->getInstID());
	// 处理称号上线
	role->getHonorMgr()->online();
    
    role->calcMaxFatLimit();

    resetImpeachInfoWhenMasterOnLine(role->getInstID(), "guild_impeach_two");
	
	// 处理离线激活码
	role->getActivationCodeMgr()->offlineAction();
    
    //处理角色登陆天数奖励状态的刷新
    role->getRoleLoginDaysAwardMgr()->refreshRoleAwardState();
   
	// 重新计算图腾加成
	STotemMon.updateAddition(role->getInstID());
	
    takeOffFashionFromEquip(role);
	
    //处理角色登陆天数奖励状态的刷新
    int logindays = role->getLoginDays();
    if ((role->getCrossDay() > time(NULL)) && logindays == 0) {
        role->setLoginDays(1);
        role->saveProp(GetRolePropName(eRoleLoginDays), "1");
    }
    role->getRoleLoginDaysAwardMgr()->refreshRoleAwardState();
    
	// 上线检测定制邮件
	g_CustomMailAppMou.roleOnlineCheck(role->getInstID());
    
	SHonorMou.procAppHonor(eHonorApp_OnlineTips, role);
	SHonorMou.procAppHonor(eHonorApp_WorldTalk, role);
    
    // 扭蛋改版，价格降低，对之前的购买的进行补偿
    if (role->getEggCompensation() == 0) {
        role->getCapsuletoyMgr()->sendEggCompensation();
        role->setEggCompensation(1);
    }

	role->getMidastouchMgr()->online();
}

//角色下线，注意释构顺序
void GlobalRoleOffline(int roleid)
{
    SRoleGuildMgr.offline(roleid);
    g_HeatBeatAnitCheating.onDisconnect(roleid);
}

void Role::getRoleInfo(obj_roleinfo &info)
{
    info.roleid = getInstID();
    info.rolename = getRolename();
    info.roletype = getRoleType();
    info.lvl = getLvl();
    info.viplvl = getVipLvl();
	info.honorId = getUseHonor();
    info.guildPosition = SRoleGuild(getInstID()).getPosition();
    info.guildname = SGuildMgr.getRoleGuild(getInstID()).getGuildName();
    //海外版多装备
    
    int equipId = getPlayerEquip()->GetItem(kEPTWeapon).item;
    if(GameFunctionCfg::getGameFunctionCfg().equipType == eMultipleEquipDrop)
    {
        //weaponquality = getPlayerEquip()->GetEquipInfo((int)kEPTWeapon).mode;
        info.weaponquality = ItemCfg::readInt(equipId, "model");
    }
    else //大陆版单装备的品质
    {
        //weaponquality = getPlayerEquip()->GetEquipInfo((int)kEPTWeapon).qua;
        info.weaponquality = ItemCfg::readInt(equipId, "qua");
    }
    
    info.weaponfashion = getWeaponFashion();
    info.bodyfashion = getBodyFashion();//getPlayerEquip()->GetItem(kEPTFashion).item;
    info.cityBodyFashion = getHomeFashion();
    info.sceneid = getCurrSceneId();
    info.enchantId = getEnchantId();
    info.enchantLvl = getEnchantLvl();
    info.awakeLvl = getRoleAwakeLvl();
}

void Role::addPetPvpTime(int times)
{
    int nowtimes = getPetPvpTimes();

    nowtimes += times;
    check_min(nowtimes, 0);
    check_max(nowtimes, 0x7FFFFFFF);

    setPetPvpTimes(nowtimes);
    saveProp(GetRolePropName(eRolePetPvpTimes), nowtimes);

}

bool Role::checkWorldChatTime()
{
    int now = Game::tick;
    if (now >= getNextWorldChatTime()) {
        setNextWorldChatTime(now + SeparateValues::sWorldChatCd);
        saveProp(GetRolePropName(eRoleWorldChatTime), getNextWorldChatTime());
        return true;
    }
    return false;
}

void Role::getPlayerBattleProp(obj_roleBattleProp &info)
{
    vector<int> equipSkills;

    equipSkills = getSkillMod()->getEquipSkillList();
    for (int i = 0; i < equipSkills.size(); i++) {
        info.skills.push_back(equipSkills[i]);
    }

//    this->CalcPlayerProp();
    info.roleid = getInstID();
    info.maxhp = getMaxHp();
    info.atk = getAtk();
    info.def = getDef();
    info.hit = getHit();
    info.dodge = getDodge();
    info.cri = getCri();
    
    int weaponId = getPlayerEquip()->GetItem(kEPTWeapon).item;
    //int fashionId = getPlayerEquip()->GetItem(kEPTFashion).item;
//    info.weaponquality = ItemCfg::readInt(weaponId, "qua");
    
    info.weaponquality = getWeaponQua();
    info.bodyfashion = getBodyFashion();
    info.weaponfashion = getWeaponFashion();
    info.enchantId = getEnchantId();
    info.enchantLvl = getEnchantLvl();
    info.awakeLvl = getRoleAwakeLvl();
}

bool Role::checkRegisterOrder()
{
	if (time(NULL) - mRegisterOrderTime > 5) {
		mRegisterOrderTime = time(NULL);
		return true;
	}
	return false;
}

void Role::send(INetPacket* packet)
{
	sendNetPacket(getSessionId(), packet);
}

void Role::sendFriendAwardMail()
{
    RedisResult result(redisCmd("hmget role:%d %s %s", getInstID(), GetRolePropName(eRoleFriendAwardCount), GetRolePropName(eRoleFriendAwards)));

    int todayCount = result.readHash(0, 0);
    string todayaAwards = result.readHash(1, "");

    if (todayCount <= 0) {
        return;
    }

    ItemArray items;
    RewardStruct reward;
    vector<string> itemstrs = StrSpilt(todayaAwards, ";");
    rewardsCmds2ItemArray(itemstrs,items,reward);

    MailFormat* formate = NULL;
    string sendername = "";
    string title = "无";
    string content = "";

    formate = MailCfg::getCfg("friend_award");

    if (formate) {
        sendername = formate->sendername;
        title = formate->title;
        
        vector<string> contents;
        reward2MailFormat(reward, items, &contents, NULL);
        string awardStr = StrJoin(contents.begin(), contents.end(), ",");

        //您成功协助好友通关，获得%d份好友奖励，你得到总计%s。vip等级越高，可获得的奖励上限越高。
        content = formate->content;
        string todayCount_str = Utils::itoa(todayCount);

        if (2 != find_and_replace(content, 2, todayCount_str.c_str(), awardStr.c_str()))
        {
            log_warn("mail content format error.[friend_award]");
        }
    }

    bool ret = sendMail(0,
                        sendername.c_str(),
                        getRolename().c_str(),
                        title.c_str(),
                        content.c_str(),
                        todayaAwards.c_str(),
                        "");

    if( false ==  ret ){
        StoreOfflineItem(getInstID(), todayaAwards.c_str());
    }

    doRedisCmd("hmset role:%d %s %d, %s %s", getInstID(), GetRolePropName(eRoleFriendAwardCount), 0, GetRolePropName(eRoleFriendAwards), "");
}

void RoleMgr::saveRoleData(int tick)
{
/* 1.7 再做这个
    const int BASE = 10;    //分10波
    int saveid = tick % BASE;

    typedef std::map<int, Role*> Map;

    for (Map::iterator iter = _Roles.begin(); iter != _Roles.end(); ++iter)
    {
        int current = iter->first % BASE;
        if(current != saveid)
            continue;
        iter->second->saveNewProperty();
    }
*/
}

void Role::getBriefBatforce(obj_batforce_brief& out)
{
    out.atk = getAtk();
    out.maxhp = getMaxHp();
    out.def = getDef() ;
    out.dodge = getDodge();
    out.cri = getCri();
    out.hit = getHit();
}

extern bool canInorgeBattleCheck();

bool checkPlayerCompatPowerForCheat(Role* role, int sceneid, int& battleTime)
{
    if (canInorgeBattleCheck()) {
        return false;
    }
    
    int nowtime = Game::tick;
    int beginBattleTime = role->getBattleTime();

    //算出实际的打斗时间
    int costSec = nowtime - beginBattleTime;
//    printf("实际战斗时间：%d\n", costSec);

    battleTime = costSec;

    role->setBattleTime(0);
    SceneCfgDef* scene = SceneCfg::getCfg(sceneid);
    if (scene == NULL) {
        return false;
    }

    if (role->getBattleForce() >= scene->fightpoint) {
        return false;
    }

    int checkPercent = 100 * (( scene->fightpoint - role->getBattleForce())*1.0f / scene->fightpoint);

    CompatPowerVerifyCfgDef* checkCfg = CompatPowerVerifyCfg::getCfg(checkPercent);
    if (checkCfg == NULL) {
        return false;
    }

    //取出标准的时间
    int referTime = scene->fightTime * checkCfg->timeMulti;

    //如果实际的时间比标准的时间少，则按一定的概率判定是否作弊
    if (costSec < referTime) {
        int rol = rand()%100;
        if (rol < (checkCfg->cheatPercent * 100)) {

            LogMod::addLogCompatPowerError(role->getInstID(), sceneid, battleTime);
            kickAwayRoleWithTips(role, "");
            return true;
        }
    }
    return false;
}

void kickAwayRoleWithTip(Role* role, int errorcode)
{
    ack_client_cheat ack;
    ack.errorcode = errorcode;
    sendNetPacket(role->getSessionId(), &ack);
    SRoleMgr.disconnect(role, eClientCheat);
}

void removeExpiredItemWhenRoleInit(Role *role)
{
    //保存背包
    ItemArray &bagItems = role->getBackBag()->GetItems();
    
    GridArray effeGrids;
    
    //判断物品是否有过期值，若有并且过期了则删除物品。
    for (int i = 0; i < bagItems.size(); ++i)
    {
        ItemCfgDef *def = ItemCfg::getCfg(bagItems[i].item);
        if (!def)
        {
            continue;
        }
        string overtime = def->ReadStr("overdate", "");
        if (!overtime.empty())
        {
            int expiretime = Utils::parseDate(overtime.c_str());
            if (expiretime != -1)
            {
                int now = time(NULL);
                if (now >= expiretime)
                {
                    BagGrid grid;
                    grid.index = i;
                    grid.item = ItemGroup();
                    
                    effeGrids.push_back(grid);
                }
            }
        }
    }
    
    role->updateBackBag(effeGrids);
    
    /*
    for (int i = 0; i < bagItems.size(); ++i)
    {
        if (bagItems[i].item == 0)
        {
            doRedisCmd("hdel BackBag:%d %d", role->getInstID(), i);
        }
    }
    */
}

int Role::renameRolename(string newname)
{
    //判断角色名是否存在
    RedisResult result;
    string oldName = getRolename();
    string oldnameKey = Utils::makeStr("rolename:%s", oldName.c_str());

    result.setData(redisCmd("exists rolename:%s", newname.c_str()));

    if (result.readInt() == 1)
    {
        g_RandNameMgr.update(getSex() == eBoy, newname);

        return CE_ROLE_NAME_EXIST;
    }

    if (!mailChangeRoleName(mInstID, mRolename.c_str(), newname.c_str()))
    {
        return CE_RENAME_SYSTEM_ERROR;
    }
	
	Xylog log(eLogName_ReRoleName,  getInstID());
	log<<getRolename();

    setRolename(newname);

    SRoleMgr.delRoleNameIdx(oldName);
    SRoleMgr.addRoleNameIdx(newname, this);
    g_RandNameMgr.update(getSex() == eBoy, newname);

    doRedisCmd("hmset role:%d %s %s", getInstID(), GetRolePropName(eRolePropRoleName), newname.c_str());

    string newnameKey = Utils::makeStr("rolename:%s",newname.c_str());
    doRedisCmd("rename %s %s", oldnameKey.c_str(), newnameKey.c_str());

    RoleGuildProperty& prop = SRoleGuild(getInstID());
    if ( !prop.isNull()) {
        prop.setRoleName(newname.c_str());
    }
	
	log<<newname;
	
    return CE_OK;
}

bool Role::deattachEquipInbag(int index)
{
    ItemGroup item = getBackBag()->GetItem(index);
    
    ItemCfgDef* cfg = ItemCfg::getCfg(item.item);
    if (cfg == NULL)
    {
        return false;
    }
    
    int type = cfg->ReadInt("type");
    
    if (type == kItemEquip)
    {
        Json::Reader reader;
        Json::FastWriter writer;
        
        Json::Value jsonval;
        reader.parse(item.json.c_str(), jsonval);
        
        //检查装备上的石头
        int holenum = cfg->ReadInt("holenum");
        
        ItemArray stones;
        GridArray addGffgrids;
        
        for(int stoneIndex = 1; stoneIndex <= holenum; stoneIndex++)
        {
            char key[32];
            sprintf(key, "stone%d", stoneIndex);
            int stoneid = jsonval[key].asInt();
            if (stoneid <= 0)
            {
                continue;
            }
            jsonval[key] = 0;
            stones.push_back(ItemGroup(stoneid, 1));
        }
        
        if (preAddItems(stones, addGffgrids) != CE_OK)
        {
            return false;
        }
        
        item.json = writer.write(jsonval);
        
        //删除装备上的石头
        updateBackBagItems(index, item);
        
        //把脱下的石头放到背包
//        updateBackBag(addGffgrids, stones, true, "deattachEquipInbag");
        playerAddItemsAndStore(addGffgrids, stones, "deattachEquipInbag", true);
        
        return true;
    }
    
    return false;
}


bool Role::tryLeaveWorldScene()
{
    if (getWorldState() != eWorldStateInWorld)
    {
        return false;
    }
    
    create_cmd(LeaveWorldScene, leaveWorldScene);
    leaveWorldScene->roleId = getInstID();
    leaveWorldScene->sessionId = getSessionId();
    sendMessageToWorld(0, CMD_MSG, leaveWorldScene, 0);
    setWorldState(eWorldStateLeaving);
    return true;
}


bool Role::tryLeaveWorld()
{
    create_cmd(LeaveWorld, leaveWorld);
    leaveWorld->roleId = getInstID();
    leaveWorld->sessionId = getSessionId();
    sendMessageToWorld(0, CMD_MSG, leaveWorld, 0);
    return true;
}

void Role::addGuildInspired()
{
    float inspireAddBattleForce = ChallengeCfg::getGuildDef().inspiredfighting *
    SGuildMgr.getRoleGuild(getInstID()).getInspired();
    
    if (math::floatEq(inspireAddBattleForce, 0.0f)) {
        inspireAddBattleForce = 0.0f;
    }
    
    int maxhp = getMaxHp() * (1 + inspireAddBattleForce);
    int atk = getAtk() * (1 + inspireAddBattleForce);
    int def = getDef() * (1 + inspireAddBattleForce);
    float hit = getHit() * (1 + inspireAddBattleForce);
    float dodge = getDodge() * (1 + inspireAddBattleForce);
    float cri = getCri() * (1 + inspireAddBattleForce);
    
    mRealBattleProp.setMaxHp(maxhp);
    mRealBattleProp.setDef(def);
    mRealBattleProp.setAtk(atk);
    mRealBattleProp.setHit(hit);
    mRealBattleProp.setDodge(dodge);
    mRealBattleProp.setCri(cri);
    
    sendRolePropInfoAndVerifyCode(true);
}

void Role::removeGuildInspired()
{
//    onCalPlayerProp();
    
    mRealBattleProp.setAtk(getAtk());
    mRealBattleProp.setDef(getDef());
    mRealBattleProp.setCri(getCri());
    mRealBattleProp.setDodge(getDodge());
    mRealBattleProp.setHit(getHit());
    
    mRealBattleProp.setMaxHp(getMaxHp());
    
    sendRolePropInfoAndVerifyCode(true);
}

int Role::getSyncTeamCopyLeftTimesOfAward()
{
    int leftTimes = SyncTeamCopyFunctionCfg::getIncomeTimes() - getSyncTeamCopyAttendTimes();
    
    check_min(leftTimes, 0);
    
    leftTimes += getIncrSyncTeamCopyTimesForIncome();
    
    return leftTimes;
}

//角色登录是检查已有的时装是否能激活时装收集新的阶段
void Role::freshFashionCollectWithFashion()
{
    vector<int> fashionIds = getWardrobe()->getAllFashionId();
    
    for (int i = 0; i < fashionIds.size(); i++) {
        getFashionCollect()->playerGetNewFashionid(fashionIds[i]);
    }
}

 void finishLoadPaihangSortBatDataByAddTopNPerson(int rid,int type,vector<SortInfo> data)
 {
     vector<SortInfo> sortdata ;
     sortdata.insert(sortdata.end(),data.begin(),data.end());
     
     int roleID = rid;
      int roleid = roleID;
      int totalAddNum = 0;
      
      Role *self = SRoleMgr.GetRole(roleID);
      
      bool IsFalkOnLine = false;
      ack_addfriend ack;
      for (vector<SortInfo>::iterator beg = sortdata.begin();
      beg != sortdata.end() && \
      self->getFriendMgr()->getFriendCount() < VipPrivilegeCfg::getCfg(self->getVipLvl())->friendCount
      ;
      ++beg)
      {
      Role* tarrole = SRoleMgr.GetRole((beg)->roleid);
      if (tarrole == NULL)
      {
      ack.errorcode = 1;
      tarrole = SRoleMgr.createRole((beg)->roleid);
      tarrole->RoleInit();
      IsFalkOnLine = true;
      
      }
      
      if (tarrole->getId() == self->getId()) {
          continue;
      }
      Friend fself((beg)->roleid);
      Friend ftar(roleid);
      
      VipPrivilegeCfgDef* vipData = VipPrivilegeCfg::getCfg(self->getVipLvl());
      
      if (vipData == NULL) {
      ack.errorcode = CE_READ_VIPCFG_ERROR;
      continue;
      }
      
      if (self->getFriendMgr()->getFriendCount() >= vipData->friendCount) {
      ack.errorcode = CE_YOUR_FRIENDCOUNT_IS_FULL;
      continue;
      }
      
      VipPrivilegeCfgDef* targetVipData = VipPrivilegeCfg::getCfg(tarrole->getVipLvl());
      if (targetVipData == NULL) {
      ack.errorcode = CE_READ_VIPCFG_ERROR;
      continue;
      }
      
      if (tarrole->getFriendMgr()->getFriendCount() >= targetVipData->friendCount) {
      ack.errorcode = CE_THE_TARGETS_FRIENDCOUNT_IS_FULL;
      continue;
      }
      
      if (!self->getFriendMgr()->isFriend((beg)->roleid))
      {
      self->getFriendMgr()->addNewFriend((beg)->roleid);
      FriendMgr::saveFriend(roleid, fself);
      }
      
      if (!tarrole->getFriendMgr()->isFriend(roleid))
      {
      tarrole->getFriendMgr()->addNewFriend(roleid);
      FriendMgr::saveFriend((beg)->roleid, ftar);
      }
      
      ++totalAddNum;
      
      
      
      notify_agreefriend notify;
      notify.agree = 1;
      notify.role.roleid = tarrole->getId();
      notify.role.rolename = tarrole->getRolename();
      notify.role.lvl = tarrole->getLvl();
      notify.role.roletype = tarrole->getRoleType();
      notify.role.isonline = 1;
      notify.role.intimacy = fself.mIntimacy;
      notify.role.battleForce = tarrole->getBattleForce();
      sendNetPacket(self->getSessionId(), &notify);
      
      if (IsFalkOnLine) {
      IsFalkOnLine = false;
      SRoleMgr.destroyRole(tarrole);
      }
      else{
      notify_agreefriend notify2;
      notify2.agree = 1;
      notify2.role.roleid = roleid;
      notify2.role.rolename = self->getRolename();
      notify2.role.lvl = self->getLvl();
      notify2.role.roletype = self->getRoleType();
      notify2.role.isonline = 1;
      notify2.role.intimacy = ftar.mIntimacy;
      notify2.role.battleForce = self->getBattleForce();
      sendNetPacket(tarrole->getSessionId(), &notify2);
      }
      
      
      }
      
 
  
 }

bool Role::decrPetAdventureTimes(int times)
{
    if (times <= 0) {
        return false;
    }
    
    if (calPetAdventureTimes() - times < 0) {
        return false;
    }

    if( !addPetAdventureConsumableIncrTimes(-times) ) {
        int overIncr = getIncrPetAdventureTimes() - times;
        setIncrPetAdventureTimes(0);

        if (!addPetAdventureBuyTimes(overIncr)) {
            int overBuyTimes = overIncr + getPetAdventureBuyTimes();
            setPetAdventureBuyTimes(0);

            setPetAdventureFreeTimesUsedRecord(getPetAdventureFreeTimesUsedRecord() + abs(overBuyTimes));

            RoleCfgDef cfg = RoleCfg::getCfg(getJob(), getLvl());
            int petAdventureFreeTimes = cfg.getPetAdventurefreeTimes();

            if(getPetAdventureFreeTimesUsedRecord() > petAdventureFreeTimes)
                    setPetAdventureFreeTimesUsedRecord(petAdventureFreeTimes);
        }
    }
    
    saveNewProperty();
    
    notify_syn_petadventuretimes notify;
    notify.times = calPetAdventureTimes();
    sendNetPacket(getSessionId(), &notify);
    
    return true;
    
}

void Role::refreshPetAdventureTimes(int times)
{
    setPetAdventureFreeTimesUsedRecord(0);
    setPetAdventureBuyTimesRecordPerDay(0);
    
    notify_syn_petadventuretimes notify;
    notify.times = calPetAdventureTimes();
    sendNetPacket(getSessionId(), &notify);
    
}

bool Role::addPetAdventureBuyTimes(int times)
{

    if (getPetAdventureBuyTimes() < 0) {
        setPetAdventureBuyTimes(0);
        return false;
    }
    
    if (getPetAdventureBuyTimes() + times < 0) {
        return false;
    }
    
    setPetAdventureBuyTimes(getPetAdventureBuyTimes() + times);

    if (times > 0) {
        setPetAdventureBuyTimesRecordPerDay(getPetAdventureBuyTimesRecordPerDay() + times);
    }
    
    saveNewProperty();
    
    notify_syn_petadventuretimes notify;
    notify.times = calPetAdventureTimes();
    sendNetPacket(getSessionId(), &notify);
        
    return true;
}

bool Role::addPetAdventureConsumableIncrTimes(int times)
{
    if (getIncrPetAdventureTimes() < 0) {
        setIncrPetAdventureTimes(0);
        return false;
    }
    
    if (getIncrPetAdventureTimes() + times < 0) {
        return false;
    }

    setIncrPetAdventureTimes(getIncrPetAdventureTimes() + times);
    
    saveNewProperty();

    return true;
}

int Role::calPetAdventureFreeTimesRemain()
{
    RoleCfgDef cfg = RoleCfg::getCfg(getJob(), getLvl());
    int petAdventureFreeTimes = cfg.getPetAdventurefreeTimes();
    
    int petAdventureFreeTimesRemain = petAdventureFreeTimes - getPetAdventureFreeTimesUsedRecord();
    
    if (petAdventureFreeTimesRemain < 0 || petAdventureFreeTimes < petAdventureFreeTimesRemain) {
        return -1;
    }
    
    return petAdventureFreeTimesRemain;
}

int Role::calPetAdventureTimes()
{
    int petAdventureFreeTimesRemain = calPetAdventureFreeTimesRemain();
    
    int petAdventureTimes = ( petAdventureFreeTimesRemain > 0 ? petAdventureFreeTimesRemain : 0)
                                + getPetAdventureBuyTimes()
                                + getIncrPetAdventureTimes();

    return petAdventureTimes;
}


//================================================================================================================================
//幻兽大冒险机器幻兽租用次数

void Role::refreshPetAdventureRobotPetRentedTimes(int times)
{
    setPetAdventureRobotPetFreeRentTimesUsedRecordPerDay(0);
    setPetAdventureRobotPetBuyingRentTimesRecordPerDay(0);
    
    notify_syn_petadventurerobotpetrenttimesremain notify;
    notify.times = calPetAdventureRobotPetRentTimesRemain();
    sendNetPacket(getSessionId(), &notify);
    
}


bool Role::decrPetAdventureRobotPetBuyingRentTimesRemain(int times)
{
    
    if (times <= 0) {
        return false;
    }
    
    int nPetAdventureRobotPetBuyingRentTimesRemain = calPetAdventureRobotPetBuyingRentTimesRemain();
    
    if (nPetAdventureRobotPetBuyingRentTimesRemain < 0) {
        return false;
    }
    
    if (nPetAdventureRobotPetBuyingRentTimesRemain - times < 0) {
        return false;
    }
    
    setPetAdventureRobotPetBuyingRentTimesRecordPerDay(getPetAdventureRobotPetBuyingRentTimesRecordPerDay() + times);
    saveNewProperty();

    
    notify_syn_petadventurerobotpetrenttimesremain notify;
    notify.times = calPetAdventureRobotPetRentTimesRemain();
    sendNetPacket(getSessionId(), &notify);
    
    return true;
    
}

bool Role::decrPetAdventureRobotPetFreeRentTimesRemain(int times)
{
    if (times <= 0) {
        return false;
    }
    
    if (calPetAdventureRobotPetFreeRentTimesRemain() - times < 0) {
        return false;
    }
    
    setPetAdventureRobotPetFreeRentTimesUsedRecordPerDay(getPetAdventureRobotPetFreeRentTimesUsedRecordPerDay() + times);
    saveNewProperty();

    
    notify_syn_petadventurerobotpetrenttimesremain notify;
    notify.times = calPetAdventureRobotPetRentTimesRemain();
    sendNetPacket(getSessionId(), &notify);
    
    return true;
    
}

int Role::calPetAdventureRobotPetFreeRentTimesRemain()
{
    RoleCfgDef cfg = RoleCfg::getCfg(getJob(), getLvl());
    int petAdventureRobotPetFreeTimesLimit      = cfg.getPetAdventureRobotPetfreeRentedTimes();
    
    int petAdventureRobotPetFreeRentTimesRemain = petAdventureRobotPetFreeTimesLimit - getPetAdventureRobotPetFreeRentTimesUsedRecordPerDay();
    
    if (petAdventureRobotPetFreeRentTimesRemain < 0 || petAdventureRobotPetFreeTimesLimit < petAdventureRobotPetFreeRentTimesRemain) {
        return 0;
    }
    
    return petAdventureRobotPetFreeRentTimesRemain;
}

int Role::calPetAdventureRobotPetBuyingRentTimesRemain()
{

    VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(getVipLvl());
    if (vipcfg == NULL) {
        return -1;
    }
    
    int petAdventureRobotPetRentTimesLimit        = vipcfg->petAdventureRobotPetRentBuyingTimesLimit;

    int petAdventureRobotPetRentBuyingTimesRemain = petAdventureRobotPetRentTimesLimit - getPetAdventureRobotPetBuyingRentTimesRecordPerDay();
    
    if (petAdventureRobotPetRentBuyingTimesRemain < 0 || petAdventureRobotPetRentTimesLimit < petAdventureRobotPetRentBuyingTimesRemain) {
        return 0;
    }
    
    return petAdventureRobotPetRentBuyingTimesRemain;
}

int Role::calPetAdventureRobotPetRentTimesRemain()
{
    int nPetAdventureRobotPetBuyingRentTimesRemain = calPetAdventureRobotPetBuyingRentTimesRemain();
    
    if (nPetAdventureRobotPetBuyingRentTimesRemain < 0) {
        return -1;
    }

    int petAdventureRobotPetRentTimesRemain = calPetAdventureRobotPetFreeRentTimesRemain() + nPetAdventureRobotPetBuyingRentTimesRemain;
    
    return petAdventureRobotPetRentTimesRemain;
}

//幻兽大冒险租用幻兽付款(金钻)
int Role::payPetAdventureRobotPetRentedBill(bool bIsFreeCharged)
{
    int payResult;
    
    stringstream szLogRobotPetRentBuyingTimesRecordPerDay;
    stringstream szLogRobotPetFreeRentTimesRemain;
    stringstream szLogRobotPetBuyingRentTimesRemain;
    stringstream szLogRobotPetRentTimesRemain;
    
    int robotPetBuyingRentTimesRecordPerDay = getPetAdventureRobotPetBuyingRentTimesRecordPerDay();
    
    szLogRobotPetRentBuyingTimesRecordPerDay << robotPetBuyingRentTimesRecordPerDay            << ";";
    szLogRobotPetFreeRentTimesRemain         << calPetAdventureRobotPetFreeRentTimesRemain()   << ";";
    szLogRobotPetBuyingRentTimesRemain       << calPetAdventureRobotPetBuyingRentTimesRemain() << ";";
    szLogRobotPetRentTimesRemain             << calPetAdventureRobotPetRentTimesRemain()       << ";";
    
    //from config
    int cost               = 0;
    int costType           = 0;
    bool bLogPayIsSucceed = false;

    if (bIsFreeCharged) {
        decrPetAdventureRobotPetFreeRentTimesRemain(1);
        payResult = CE_OK;
        bLogPayIsSucceed = true;
    }
    else {
        do
        {
            VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(getVipLvl());
            if (vipcfg == NULL) {
                payResult = CE_READ_VIPCFG_ERROR;
                break;
            }
            
            int petAdventureRobotPetBuyingRentTimesLimit = vipcfg->petAdventureRobotPetRentBuyingTimesLimit;
            if ( robotPetBuyingRentTimesRecordPerDay + 1 > petAdventureRobotPetBuyingRentTimesLimit) {
                payResult = CE_BUY_TIMES_USEUP;
                break;
            }
            
            cost = BuyCfg::getCost(ePetAdventureRobotPetRent, robotPetBuyingRentTimesRecordPerDay + 1, costType);
            
        }while(false);
        
        payResult = CheckMoneyEnough(cost, costType, "购买幻兽大冒险副本次数");
        
        if (payResult == CE_OK) {
            if (decrPetAdventureRobotPetBuyingRentTimesRemain(1) == false) {
                payResult = CE_UNKNOWN;
            }
            else {
                bLogPayIsSucceed = true;
            }
        }
    }

    //购买后
    szLogRobotPetRentBuyingTimesRecordPerDay << getPetAdventureRobotPetBuyingRentTimesRecordPerDay();
    szLogRobotPetFreeRentTimesRemain         << calPetAdventureRobotPetFreeRentTimesRemain()   << ";";
    szLogRobotPetBuyingRentTimesRemain       << calPetAdventureRobotPetBuyingRentTimesRemain() << ";";
    szLogRobotPetRentTimesRemain             << calPetAdventureRobotPetRentTimesRemain();
    
    LogMod::addLogPetAdventureRobotPetRentTimes(this, 1, cost, costType, szLogRobotPetRentBuyingTimesRecordPerDay.str(), szLogRobotPetFreeRentTimesRemain.str(), szLogRobotPetBuyingRentTimesRemain.str(), szLogRobotPetRentTimesRemain.str(), bLogPayIsSucceed);
    
    return payResult;
}

//====================================================================================================================================


int Role::preAddItems(const ItemArray& newitems, GridArray& effectGrid)
{
    ItemArray bagItems;
    ItemArray fashionMaterials;
    ItemArray retinuePieces;
    
    for (int i = 0; i < newitems.size(); i++)
    {
        ItemCfgDef* itemcfg = ItemCfg::getCfg(newitems[i].item);
        if (itemcfg == NULL)
        {
            //背包不添加不存在的物品
            continue;
        }
        
        int itemType = itemcfg->ReadInt("type");
        
        if ( itemType == KItemFashionCollectMaterial) {
            fashionMaterials.push_back(newitems[i]);
        }
        else if (itemType == KItemRetinuePieces || itemType == KitemRetinueSkillPieces)
        {
            retinuePieces.push_back(newitems[i]);
        }
        else{
            bagItems.push_back(newitems[i]);
        }
    }
    
    if (!getFashionCollect()->getMaterialBox()->PreAddItems(fashionMaterials, effectGrid)) {
        return 1;
    }
    
    if (!mBackBag->PreAddItems(bagItems, effectGrid)) {
        return 2;
    }
    
    if (!getRetinueMgr()->mRetinueBag->PreAddItems(retinuePieces, effectGrid)) {
        return 3;
    }
    
    return CE_OK;
}

void Role::playerAddItemsAndStore(const GridArray &grids, const ItemArray &items, const char* desc, bool notify)
{
    
    if(strcmp(desc, "") != 0)
    {
        LogMod::addLogGetItem(this, mRolename.c_str(), mSessionId, items, desc);
    }
    onGetItems(items);
    
    GridArray bagGrids;
    GridArray fashionMaterialGrids;
    GridArray retinuePiecesGrids;
    
    for (int i = 0; i < grids.size(); i++) {
        
        if (grids[i].itemType == KItemFashionCollectMaterial) {
            fashionMaterialGrids.push_back(grids[i]);
        }
        else if (grids[i].itemType == KItemRetinuePieces || grids[i].itemType == KitemRetinueSkillPieces)
        {
            retinuePiecesGrids.push_back(grids[i]);
        }
        else{
            bagGrids.push_back(grids[i]);
        }
    }
    
    mBackBag->UpdateBackBag(bagGrids);
    mBackBag->Store(getInstID(), bagGrids);
//    SendBackBagNotify(this, bagGrids);
    
    
    getFashionCollect()->getMaterialBox()->UpdateBackBag(fashionMaterialGrids);
    getFashionCollect()->getMaterialBox()->Store(getInstID(), fashionMaterialGrids);
    
    getRetinueMgr()->mRetinueBag->UpdateBackBag(retinuePiecesGrids);
    getRetinueMgr()->mRetinueBag->Store(getInstID(), retinuePiecesGrids);
    
    if (notify) {
        SendBackBagNotify(this, bagGrids);
        getFashionCollect()->sendMaterialBoxNotify(fashionMaterialGrids);
    
        sendRetinueBagNotify(this, retinuePiecesGrids);
    }
}

void Role::playerDeleteItemsAndStore(const GridArray &grids, const ItemArray &items, const char *desc, bool notify)
{
    GridArray bagGrids;
    GridArray fashionMaterialGrids;
    GridArray retinuePiecesGrids;
    
    for (int i = 0; i < grids.size(); i++) {
        
        if (grids[i].itemType == KItemFashionCollectMaterial) {
            fashionMaterialGrids.push_back(grids[i]);
        }
        else if (grids[i].itemType == KItemRetinuePieces || grids[i].itemType == KitemRetinueSkillPieces)
        {
            retinuePiecesGrids.push_back(grids[i]);
        }
        else{
            bagGrids.push_back(grids[i]);
        }
    }
    
    mBackBag->UpdateBackBag(bagGrids);
    mBackBag->Store(getInstID(), bagGrids);
//    SendBackBagNotify(this, bagGrids);
    
    
    getFashionCollect()->getMaterialBox()->UpdateBackBag(fashionMaterialGrids);
    getFashionCollect()->getMaterialBox()->Store(getInstID(), fashionMaterialGrids);
    
    getRetinueMgr()->mRetinueBag->UpdateBackBag(retinuePiecesGrids);
    getRetinueMgr()->mRetinueBag->Store(getInstID(), retinuePiecesGrids);
    
    if (notify) {
        
        SendBackBagNotify(this, bagGrids);
        
        sendRetinueBagNotify(this, retinuePiecesGrids);
    }
    
    if(strcmp(desc, "") != 0)
    {
        LogMod::addUseItemLog(this, items, desc);
    }
}

void Role::updateItemsChange(GridArray &effectGrid)
{
    GridArray bagGrids;
    GridArray fashionMaterialGrids;
    GridArray retinuePiecesGrids;
    
    for (int i = 0; i < effectGrid.size(); i++) {
        
        if (effectGrid[i].itemType == KItemFashionCollectMaterial) {
            fashionMaterialGrids.push_back(effectGrid[i]);
        }
        else if (effectGrid[i].itemType == KItemRetinuePieces || effectGrid[i].itemType == KitemRetinueSkillPieces)
        {
            retinuePiecesGrids.push_back(effectGrid[i]);
        }
        else{
            bagGrids.push_back(effectGrid[i]);
        }
    }
    
    mBackBag->UpdateBackBag(bagGrids);
    mBackBag->Store(getInstID(), bagGrids);
    SendBackBagNotify(this, bagGrids);
    
    
    getFashionCollect()->getMaterialBox()->UpdateBackBag(fashionMaterialGrids);
    getFashionCollect()->getMaterialBox()->Store(getInstID(), fashionMaterialGrids);
    
    getRetinueMgr()->mRetinueBag->UpdateBackBag(retinuePiecesGrids);
    getRetinueMgr()->mRetinueBag->Store(getInstID(), retinuePiecesGrids);
    
    sendRetinueBagNotify(this, retinuePiecesGrids);
}

void Role::updateBackBag(GridArray &effectGrid)
{
    mBackBag->UpdateBackBag(effectGrid);
    mBackBag->Store(getInstID(), effectGrid);
    SendBackBagNotify(this, effectGrid);
}

void Role::updateBackBagItems(int index, ItemGroup &item)
{
    mBackBag->UpdateBackBag(index, item);
    mBackBag->Store(getInstID(), index, item);
    SendBackBagNotify(this, index, item);
}

void Role::addLogActivityCopyEnterTimesChange(int activityCopyType, const char* changeAction)
{
    int freeTimes = 0;
    int itemIncrTimes = 0;
    int rmbIncrTimes = 0;
    
    bool canLog = false;
    
    switch (activityCopyType) {
        case eActivityEnterTimesTowerDefense:
        {
            RoleCfgDef rolecfg = RoleCfg::getCfg(getJob(), getLvl());
            freeTimes = rolecfg.getTowertimes() - getTowerDefenseAct()->getEnterTimes();
            itemIncrTimes = getIncrTowerdefenseTimes();
            canLog = true;
            break;
        }
    
        case eActivityEnterTimesSyncTeamCopy:
        {
            freeTimes = SyncTeamCopyFunctionCfg::getIncomeTimes() - getSyncTeamCopyAttendTimes();
            itemIncrTimes = getIncrSyncTeamCopyTimesForIncome();
            canLog = true;
            break;
        }
        
        case eActivityEnterTimesFriendDunge:
        {
            RoleCfgDef rolecfg = RoleCfg::getCfg(getJob(), getLvl());
            
            freeTimes = rolecfg.getFriendDungeTimes() - getFriendDungeAct()->getAttendTimes();
            itemIncrTimes = getIncrFriendDungeonTimes();
            canLog = true;
            break;
        }
            
        case eActivityEnterTimesFriendCopyDoubleAward:
        {
            itemIncrTimes = getDoublePotionNum();
            canLog = true;
            break;
        }
        
        case eActivityEnterTimesMysticalCopyBOSSandGoblin:
        {
            itemIncrTimes = getIncrMysticalCopyBOSSandGoblinTimes();
            canLog = true;
            break;
        }
            
        case eActivityEnterTimesPetAdventure:
        {
            freeTimes = calPetAdventureFreeTimesRemain();
            itemIncrTimes = getIncrPetAdventureTimes();
            rmbIncrTimes = getPetAdventureBuyTimes();
            
            canLog = true;
            break;
        }
            
        case eActivityEnterTimesSyncPvp:
        {
            freeTimes = SynPvpFuntionCfg::getAwardTimes(getRealPvpMatchTimes());
            canLog = true;
            break;
        }
            
        case eActivityEnterTimesPvp:
        {
            freeTimes = getPvpTimes();
            canLog = true;
            break;
        }
        
        case eActivityEnterTimesEliteCopy:
        {
            freeTimes = getEliteTimes();
            canLog = true;
            break;
        }
        default:
            break;
    }

    if (true == canLog) {
        
        LogMod::addLogActivityEnterTimesChange(getInstID(), getRolename().c_str(), activityCopyType, itemIncrTimes, rmbIncrTimes, freeTimes, freeTimes + itemIncrTimes + rmbIncrTimes, changeAction);
    }
}

bool Role::skillCanLearn(int skillId)
{
    RoleTpltCfg* roleTplt = RoleTpltCfg::getCfg(this->getRoleType());
    
    if (roleTplt == NULL) {
        return false;
    }
    
    int skillIndex = getSkillIndex(skillId);
    
    //检查普通技能
    for (int i = 0; i < roleTplt->mSkillInitialvalue.size(); i++) {
        
        int skillIdTplt = roleTplt->mSkillInitialvalue[i];
        if (getSkillIndex(skillIdTplt) == skillIndex) {
            return true;
        }
        
    }
    
    //检查觉醒技能
    vector<AwakeSkillReplace> awakeSkillInfo;
    RoleAwakeCfg::getAlreadySkillReplace(this->getJob(), this->getRoleAwakeLvl(), awakeSkillInfo);
    
    for (int i = 0; i < awakeSkillInfo.size(); i++) {
        if (skillIndex == getSkillIndex(awakeSkillInfo[i].newSkillId)) {
            return true;
        }
    }
    
    return false;
}

void RoleMgr::safe_gmAddTop50Person(int roleID, int num)
{
    gmAddTop50Person(roleID, num);
}

void RoleMgr::gmAddTop50Person(int roleID, int num)
{
    geteSortBatData(roleID, 1, num, finishLoadPaihangSortBatDataByAddTopNPerson);
}