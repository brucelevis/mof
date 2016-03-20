//
//  DataBase.cpp
//  GameSrv
//
//  Created by 麦_Mike on 12-12-31.
//
//

#include <algorithm>
#include <set>
#include "DataCfg.h"
//#include "Role.h"
#include "inifile.h"
#include "Utils.h"
#include "Quest.h"
#include "NpcDialog.h"
#include "flopcard.h"
//#include "LivingObject.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <climits>
#include "tinyxml.h"
#include <sstream>
#include "EnumDef.h"
#include <algorithm>
#include "json/json.h"

#if defined(CLIENT)
#include "cocos2d.h"
#include "GameMgr.h"
#include "MultiLanguageAdapter.h"
#endif

#if defined(CLIENT)
    typedef IniFile GameIniFile;
    #if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    #include "../common2/AndroidPlatformCfg.h"
    #endif
#else
    #include "GameIniFile.h"
#endif

string Cfgable::pathPrefix;

//获取配置文件的完整路径 --- linshusen
string Cfgable::getFullFilePath(string fileName)
{
#if defined(CLIENT)
    //客户端用cocos2d的查询机制，用于自动更新资源。
    return CCFileUtils::sharedFileUtils()->fullPathForFilename(fileName.c_str());
#else
    //服务器用自己定义的路径
    return Cfgable::pathPrefix + fileName;
#endif
}

#if defined(CLIENT)
void Cfgable::loadAll(ThreadDelegateProtocol* delegate, string msgStr, int startPercent, int endPercent)
{
    int updatePercentTimes = 9;
    int updatePercent = (endPercent - startPercent)/updatePercentTimes;
    
    //有新的版本时 再次加载MultiLanguage.plist
    if(GameMgr::getHasNewVersion())
    {
        //在进入游戏的时候  已经读过“MultiLanguageCode.plist” 会有缓存  要是不清除缓存的话 下次还是会在该路径下读取
        CCFileUtils::sharedFileUtils()->purgeCachedEntries();
        MultiLanguageAdapter::Instance()->loadDictionaryDatas();
    }
    
    /**Android加载fanti_config.ini配置文件,分区域,台湾版本才需要加载*/
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    if(zh_tw == MultiLanguageAdapter::Instance()->getLanguageType())
    {
        AndroidFantiCfg::load(Cfgable::getFullFilePath("fanti_config.ini").c_str());
    }
#endif
    
    string fpath = Cfgable::getFullFilePath("flopcard.ini");
    SflopcardMgr.LoadFile(fpath.c_str());
    
    //这个表在GameMgr启动时已经加载，这是2次加载。为了自动更新---linshusen
    Md5VerifyCfg::load(Cfgable::getFullFilePath("skeletonVerify.ini"));
    
    GameFunctionCfg::load(Cfgable::getFullFilePath("gameFunction.ini"));
    SceneCfg::load(Cfgable::getFullFilePath("scene.ini"));
    RoleCfg::load(Cfgable::getFullFilePath("role.ini"));
    SkillCfg::load(Cfgable::getFullFilePath("skill.ini")
                   ,Cfgable::getFullFilePath("skillEffects.ini")
                   );
    DungeonCfg::load(Cfgable::getFullFilePath("dungeon.ini"));
    ThreadMessage* msg = new ThreadMessage();
    msg->mMessageType = tmt_OnLoadProcess;
    msg->mIntData = startPercent + 1 * updatePercent;
    msg->mStrData = msgStr;
    GameMgr::getThreadHandler()->sendThreadMessage(delegate, msg);

    ItemCfg::load(Cfgable::getFullFilePath("item.ini"));
    if (GameFunctionCfg::getGameFunctionCfg().equipType == eMultipleEquipDrop)
    {
        StarCfgNew::load(Cfgable::getFullFilePath("Strengthen.ini"), Cfgable::getFullFilePath("strengthen_cost.ini"));
        EquipFusionCfg::load(Cfgable::getFullFilePath("fuse_cost.ini"));
    }
    else
    {
        StarCfg::load(Cfgable::getFullFilePath("Strengthen.ini"));
    }
    ConstellationCfg::load(Cfgable::getFullFilePath("Constellation.ini"));

    msg = new ThreadMessage();
    msg->mMessageType = tmt_OnLoadProcess;
    msg->mIntData = startPercent + 2 * updatePercent;
    msg->mStrData = msgStr;
    GameMgr::getThreadHandler()->sendThreadMessage(delegate, msg);

    BuyCfg::load(Cfgable::getFullFilePath("buy.ini"));
    PvpCfg::load(Cfgable::getFullFilePath("pvp.ini"));
    PetCfg::load(Cfgable::getFullFilePath("pet.ini"),
                 Cfgable::getFullFilePath("petGrowth.ini"),
                 Cfgable::getFullFilePath("petStar.ini")
                 );
    CrossServiceWarCfg::load(Cfgable::getFullFilePath("CrossServiceWar.ini"));
    msg = new ThreadMessage();
    msg->mMessageType = tmt_OnLoadProcess;
    msg->mIntData = startPercent + 3 * updatePercent;
    msg->mStrData = msgStr;
    GameMgr::getThreadHandler()->sendThreadMessage(delegate, msg);

    string fullpath = Cfgable::getFullFilePath("quests.ini");
    QuestMgr::LoadFile(fullpath.c_str());

    msg = new ThreadMessage();
    msg->mMessageType = tmt_OnLoadProcess;
    msg->mIntData = startPercent + 4 * updatePercent;
    msg->mStrData = msgStr;
    GameMgr::getThreadHandler()->sendThreadMessage(delegate, msg);

    LoginAwardCfg::load(Cfgable::getFullFilePath("loginaward.ini"));
    KoreaLevelAwardCfg::load(Cfgable::getFullFilePath("loginaward.ini"));
    VipPrivilegeCfg::load(Cfgable::getFullFilePath("vip.ini"));
    MoneyTreeCfg::load(Cfgable::getFullFilePath("moneytree.ini"));
    ChargeCfg::load(Cfgable::getFullFilePath("Charge.ini"));
    sysTipCfg::load(Cfgable::getFullFilePath("systemannouncement.ini"));

    //LotteryCfg::load(Cfgable::getFullFilePath("lottery.ini"));
    DiceLotteryCfgMgr::load(Cfgable::getFullFilePath("dicelottery.ini"),
                            Cfgable::getFullFilePath("diceitem.ini"));
    ActivityPetCosinoCfgMgr::load(Cfgable::getFullFilePath("ActivityPetCasino.ini"));
    IconCfg::load(Cfgable::getFullFilePath("UI_Icon.ini"));
    msg = new ThreadMessage();
    msg->mMessageType = tmt_OnLoadProcess;
    msg->mIntData = startPercent + 5 * updatePercent;
    msg->mStrData = msgStr;
    GameMgr::getThreadHandler()->sendThreadMessage(delegate, msg);

    //只有客户端需要加载的配置文件
    NewguidanceCfg::load(Cfgable::getFullFilePath("newguidance.ini"));
    UiIconSortCfg::load(Cfgable::getFullFilePath("newMainUI.ini"));
    DefendStatueCfg::load(Cfgable::getFullFilePath("defendstatue.ini"));
    ResCfg::load(Cfgable::getFullFilePath("resources.ini"));
    AudioCfg::load(Cfgable::getFullFilePath("audios.ini"));
    WordStockCfg::load(Cfgable::getFullFilePath("Wordstock.ini"));
    ActivityCfg::load(Cfgable::getFullFilePath("activity.ini"));//限时日常活动表
    OrdinaryActivityCfg::load(Cfgable::getFullFilePath("daily_activity.ini"));//普通装备宠物活动表
    HelperCfg::load(Cfgable::getFullFilePath("Helper.ini").c_str());
    PetPvpCfg::load(Cfgable::getFullFilePath("petpvp.ini").c_str());
    FamousCfg::load(Cfgable::getFullFilePath("famousdunge.ini").c_str());
    FamousDataCfg::load(Cfgable::getFullFilePath("famousdata.ini"));
    moneyResCfg::load(Cfgable::getFullFilePath("money.ini").c_str());
    KoreaPlatformJump::load(Cfgable::getFullFilePath("KoreaPlatformJump.ini").c_str());
    MilitaryCfg::load(Cfgable::getFullFilePath("synarena_military.ini").c_str());
    //--by lihengjin
    if(!(Cfgable::getFullFilePath("synarena_exchangeMultiLanguage.ini") == "synarena_exchangeMultiLanguage.ini"))
    {
      SynPvpStoreCfgML::load(Cfgable::getFullFilePath("synarena_exchangeMultiLanguage.ini"));
    }
    SynPvpStoreCfg::load(Cfgable::getFullFilePath("synarena_exchange.ini"));
    GashaponMachinesCfg::load(Cfgable::getFullFilePath("GashaponMachines.ini"), Cfgable::getFullFilePath("GashaponMachinesAward.ini"));
    MonthCardCfgMgr::load(Cfgable::getFullFilePath("MonthCardActivity.ini"));
    SyncTeamCopyFunctionCfg::load( Cfgable::getFullFilePath( "SyncTeamCopyFunction.ini" ) );
    
    PetAdventureCfg::load(Cfgable::getFullFilePath("petAdventureCopy.ini"));
    DailyScheduleCfg::load(Cfgable::getFullFilePath("DailySchedule.ini").c_str(),
                           Cfgable::getFullFilePath("DailyScheduleAward.ini").c_str());
    
    LoadingTipsDefCfg::load(Cfgable::getFullFilePath("loadingTips.ini"));
    
    PetAdventureRobotPetsForRentCfg::load(Cfgable::getFullFilePath("robot_petAdventure.ini"));
    CrossServiceWarWorshipCfg::load(Cfgable::getFullFilePath("CrossServiceWorship.ini"));
    CrossServiceWarRankingserviceCfg::load(Cfgable::getFullFilePath("rankingservice.ini"));
    CrossServiceWarRankingsimpleCfg::load(Cfgable::getFullFilePath("rankingsimple.ini"));
    CsPvpGroupCfg::load(Cfgable::getFullFilePath("csPvpGroupCfg.ini").c_str());
    
#pragma mark - 公会活动配置加载
    
    msg = new ThreadMessage();
    msg->mMessageType = tmt_OnLoadProcess;
    msg->mIntData = startPercent + 6 * updatePercent;
    msg->mStrData = msgStr;
    GameMgr::getThreadHandler()->sendThreadMessage(delegate, msg);

    StateMutexCfg::load(Cfgable::getFullFilePath("state_mutex.ini"));
    RoleTpltCfg::load(Cfgable::getFullFilePath("role_tplt.ini"));
    SeparateResIDs::load(Cfgable::getFullFilePath("separate_res_ids.ini"));
    SeparateValues::load(Cfgable::getFullFilePath("separate_values.ini"));
    LocalNotificationCfg::load(Cfgable::getFullFilePath("localNotification.ini"));

    msg = new ThreadMessage();
    msg->mMessageType = tmt_OnLoadProcess;
    msg->mIntData = startPercent + 7 * updatePercent;
    msg->mStrData = msgStr;
    GameMgr::getThreadHandler()->sendThreadMessage(delegate, msg);

    MonsterCfg::load(Cfgable::getFullFilePath("monster_property.ini"),Cfgable::getFullFilePath("monster_tplt.ini"));
    NpcCfg::load(Cfgable::getFullFilePath("npc.ini"));
    NewbieCfg::load(Cfgable::getFullFilePath("newbie.ini"));

    msg = new ThreadMessage();
    msg->mMessageType = tmt_OnLoadProcess;
    msg->mIntData = startPercent + 8 * updatePercent;
    msg->mStrData = msgStr;
    GameMgr::getThreadHandler()->sendThreadMessage(delegate, msg);

    QualityColorCfg::load(Cfgable::getFullFilePath("color.ini"));
    string fullpath1 = Cfgable::getFullFilePath("npcdialog.ini");
    NpcDialogMgr::LoadFile(fullpath1.c_str());
    PortalCfg::load(Cfgable::getFullFilePath("portal.ini"));
    FriendRewardCfg::load(Cfgable::getFullFilePath("friend_reward.ini"));
    EquipResonanceCfg::load(Cfgable::getFullFilePath("equip_resonance.ini"));
    MysteriousCfgMgr::load();
    IllustrationsCfg::load(Cfgable::getFullFilePath("PetCollect.ini"));
    MysticalCopyListCfg::load(Cfgable::getFullFilePath("mysticalcopylist.ini").c_str());
    AnswerActivityCfg::load(Cfgable::getFullFilePath("AnswerActivity.ini"), Cfgable::getFullFilePath("Answerbase.ini"));
    GuildCfgMgr::load();
	HonorCfgMgr::load();
	SysTotemCfgMgr::load();
    ChargeActivityCfgMgr::load();
    ShareAwardCfg::load(Cfgable::getFullFilePath("shareAward.ini"));
    PetAssistCfg::load(Cfgable::getFullFilePath("petAssist.ini").c_str());
    PetPvpRedemptionCfg::load(Cfgable::getFullFilePath("petpvpredemption.ini"));
    
    //限时活动
    LimitActivityCfg::load(Cfgable::getFullFilePath("ticket_activity.ini"));
    TimeLimitKillCopyCfg::load(Cfgable::getFullFilePath("time_limit_activity.ini").c_str());
    ChallengeCfg::load(Cfgable::getFullFilePath("guildTrain.ini").c_str());
    //防作弊配置
    AntiClientCheatCfg::load(Cfgable::getFullFilePath("anti_cheat.ini"));
    
    autoBattleCfg::load(Cfgable::getFullFilePath("autofight.ini"));
    QueenBlessCfg::load(Cfgable::getFullFilePath("QueenBless.ini"));
    PetMergeCfg::load(Cfgable::getFullFilePath("petMerge.ini"));
    
    NetWorkStateCfg::load(Cfgable::getFullFilePath("network_status.ini"));
    FunnyActCfg::load(Cfgable::getFullFilePath("funny_activity.ini"));
    WardrobeCfg::load(Cfgable::getFullFilePath("wardrobe.ini"));
    TotemMultiLanguage::load();
    ActivityInviteCfgMgr::load( Cfgable::getFullFilePath("ActivityInviteCfg.ini"));
    EquipmodeCorrespondedColor::load(Cfgable::getFullFilePath("equipmodeCorrespondedColor.ini"));
    LinkStateCheckCfg::load(Cfgable::getFullFilePath("link_check.ini").c_str());
    PetEvolutionCfgMgr::load(Cfgable::getFullFilePath("PetEvolutionNode.ini"), Cfgable::getFullFilePath("PetEvolutionMaterial.ini"));
    WeaponEnchantCfg::load(Cfgable::getFullFilePath("weaponEnchants.ini"));
    
    FashionCollectCfg::load(Cfgable::getFullFilePath("fashionCollect.ini"));
    FashionMaterialRecycleCfg::load(Cfgable::getFullFilePath("fashionCollect_reclaim.ini"));
    MysticalCopyListCfg::load(Cfgable::getFullFilePath("mysticalcopylist.ini").c_str());
    MysticalCopyCfg::load(Cfgable::getFullFilePath("mysticalcopy.ini").c_str());
    SynthesisCfg::load(Cfgable::getFullFilePath("synthesis.ini"));
    MoneyCfg::load(Cfgable::getFullFilePath("money.ini"));
    ActivateBagGridsCfg::load(Cfgable::getFullFilePath("activatebaggrids.ini"));
    TwistEggCfgMgr::load();
    MidastouchCfgMgr::load();
    RoleAwardCfgMgr::load(Cfgable::getFullFilePath("roleaward.ini").c_str());
    RoleAwakeCfg::load(Cfgable::getFullFilePath("roleawake.ini").c_str());
    EquipBaptizeCfg::load(Cfgable::getFullFilePath("equipBaptizeSlot.ini").c_str(),
                          Cfgable::getFullFilePath("equipBaptizeAttri.ini").c_str());
    msg = new ThreadMessage();
    msg->mMessageType = tmt_OnLoadProcess;
    msg->mIntData = startPercent + 9 * updatePercent;
    msg->mStrData = msgStr;
    GameMgr::getThreadHandler()->sendThreadMessage(delegate, msg);
    
    PvpStoreCfg::load(Cfgable::getFullFilePath("pvp_exchange.ini"));
    
    RetinueCfg::load( Cfgable::getFullFilePath("retinuemod.ini"),
                      Cfgable::getFullFilePath("retinue_skill.ini"),
                      Cfgable::getFullFilePath("retinue_summon.ini"),
                      Cfgable::getFullFilePath("spirit_speak.ini"));
    

}
#else
void Cfgable::loadAll()
{
    //服务器需要加载的配置文件
    
    StateMutexCfg::load("state_mutex.ini");
    
    SflopcardMgr.LoadFile("flopcard.ini");
    
    GameFunctionCfg::load("gameFunction.ini");
    SceneCfg::load("scene.ini");
    RoleCfg::load("role.ini");
    ResCfg::load("resources.ini");
    RoleTpltCfg::load("role_tplt.ini");
    SkillCfg::load("skill.ini"
                   ,"skillEffects.ini"
                   );
    ItemCfg::load("item.ini");
    
    if (GameFunctionCfg::getGameFunctionCfg().equipType == eMultipleEquipDrop)
    {
        StarCfgNew::load("Strengthen.ini", "strengthen_cost.ini");
        
        EquipFusionCfg::load("fuse_cost.ini");
    }
    else
    {
        StarCfg::load("Strengthen.ini");
    }
    
    ConstellationCfg::load("Constellation.ini");
    BuyCfg::load("buy.ini");
    PvpCfg::load("pvp.ini");
    CrossServiceWarCfg::load("CrossServiceWar.ini");
    FriendRewardCfg::load("friend_reward.ini");
    EquipResonanceCfg::load("equip_resonance.ini");
	MysteriousCfgMgr::load();
    IllustrationsCfg::load("PetCollect.ini", "PetCollectConfig.ini");
    PromotionActivityCfg::load("promotion_activity.ini");
	GuildCfgMgr::load();
	HonorCfgMgr::load();
	ItemChangeCfgMgr::load();
	SysTotemCfgMgr::load();
	GuildChampaGoodsCfg::load();
	CopyExtraAwardCfg::load();
	TwistEggCfgMgr::load();
	MidastouchCfgMgr::load();
    
    PetCfg::load("pet.ini",
                 "petGrowth.ini",
                 "petStar.ini"
                 );

    MonsterCfg::load("monster_property.ini","monster_tplt.ini");

    QuestMgr::LoadFile("quests.ini");

    GlobalCfg::load("global.ini");
    LoginAwardCfg::load("loginaward.ini");
    KoreaLevelAwardCfg::load("loginaward.ini");
    VipPrivilegeCfg::load("vip.ini");
    MoneyTreeCfg::load("moneytree.ini");
    ChargeCfg::load("Charge.ini");
    LotteryCfgMgr::load("lottery.ini");
    DiceLotteryCfgMgr::load("dicelottery.ini",
                            "diceitem.ini");
    DungeonCfg::load("dungeon.ini");

    PetAdventureCfg::load("petAdventureCopy.ini");
    PetAdventureRobotPetsForRentCfg::load("robot_petAdventure.ini");
    
    CrossServiceWarWorshipCfg::load("CrossServiceWorship.ini");
    CrossServiceWarRankingserviceCfg::load("rankingservice.ini");
    CrossServiceWarRankingsimpleCfg::load("rankingsimple.ini");

    ActivityPetCosinoCfgMgr::load("ActivityPetCasino.ini");
    ActivityInviteCfgMgr::load("ActivityInviteCfg.ini");
    
    WorldBossCfg::load("worldboss.ini");
    //multilanguage cfg
    //MoneyMLCfg::load(Cfgable::getFullFilePath("moneyMultiLanguage.ini").c_str());
    //MailMLCfg::load(Cfgable::getFullFilePath("mailMultiLanguage.ini").c_str());
    //RandboxGiftMLCfg::load(Cfgable::getFullFilePath("randboxMultiLanguage.ini").c_str());
    //SystemBroadcastMLCfg::load(Cfgable::getFullFilePath("SystemBroadCastMultiLanguage.ini").c_str());
    
    MailCfg::load("mail.ini");
    StrMapCfg::load("str_msg.ini");
    
    DefendStatueCfg::load("defendstatue.ini");
    MysticalCopyListCfg::load("mysticalcopylist.ini");
    MysticalCopyCfg::load("mysticalcopy.ini");
    AnswerActivityCfg::load("AnswerActivity.ini", "Answerbase.ini");
    ShareAwardCfg::load("shareAward.ini");
    
    FriendAwardCoefCfg::load("friendawardcoef.ini");
    PetAssistCfg::load("petAssist.ini");
    PetPvpCfg::load("petpvp.ini");
    SeparateValues::load("separate_values.ini");
    PetPvpRedemptionCfg::load("petpvpredemption.ini");
    
    LimitActivityCfg::load("ticket_activity.ini");
    TimeLimitKillCopyCfg::load("time_limit_activity.ini");
    ChallengeCfg::load("guildTrain.ini");

    FamousCfg::load("famousdunge.ini");
    FamousDataCfg::load("famousdata.ini");

    //SystemBroadCast
    SystemBroadcastCfg::load("SystemBroadCast.ini");
    
    //防作弊配置
    AntiClientCheatCfg::load("anti_cheat.ini");
    
    CompatPowerVerifyCfg::load("compatpower_check.ini");
    
    //game guild log text
    GameTextCfg::load("GameText.ini");
    
    GashaponMachinesCfg::load("GashaponMachines.ini", "GashaponMachinesAward.ini");
    
    //url
    UrlCfg::load("dcConfig.ini");
    
    //EquipChangeCfg::load(Cfgable::getFullFilePath("NumericConvert.ini"));
    //ItemToGoldCfg::load(Cfgable::getFullFilePath("item_sell.ini"));
    
    QueenBlessCfg::load("QueenBless.ini");
    PetMergeCfg::load("petMerge.ini");

    SynPvpStoreCfg::load("synarena_exchange.ini");
    
    SynPvpFuntionCfg::load("synpvpfunction.ini");
    
    TreasureListCfg::load("treasurecopylist.ini");
    
    GuildTreasureFightCfg::load("guildtreasurefight.ini");
    
    WardrobeCfg::load("wardrobe.ini");
    
    MilitaryCfg::load("synarena_military.ini");
    
    SyncTeamCopyCfg::load("SyncTeamCopy.ini");
    
    SyncTeamCopyFunctionCfg::load("SyncTeamCopyFunction.ini");
    
    ReservedNameMgr::load("reserved_names.ini");
    
    LinkStateCheckCfg::load("link_check.ini");
    
    PetEvolutionCfgMgr::load("PetEvolutionNode.ini", "PetEvolutionMaterial.ini");
    
    MoneyCfg::load("money.ini");
    
    PetEvolutionRestitutionCfg::load("PetEvolutionRestitution.ini");
    
    WeaponEnchantCfg::load("weaponEnchants.ini");
    
    FashionCollectCfg::load("fashionCollect.ini");
    FashionMaterialRecycleCfg::load("fashionCollect_reclaim.ini");

    SynthesisCfg::load("synthesis.ini");
    
    DailyScheduleCfg::load("DailySchedule.ini",
                           "DailyScheduleAward.ini");
     ActivateBagGridsCfg::load("activatebaggrids.ini");
    
    RoleAwardCfgMgr::load("roleaward.ini");
    
    ColorCfg::load("colormap.ini");
    
    RoleAwakeCfg::load("roleawake.ini");
    
    PvpStoreCfg::load("pvp_exchange.ini");
    EquipBaptizeCfg::load("equipBaptizeSlot.ini","equipBaptizeAttri.ini");
    
    RetinueCfg::load("retinuemod.ini", "retinue_skill.ini", "retinue_summon.ini", "spirit_speak.ini");

    CsPvpGroupCfg::load(Cfgable::getFullFilePath("csPvpGroupCfg.ini").c_str());
}
#endif


/*
 ----------------------------------------------------------
 */
bool GameCfg::isSkipLoginUI = false;//使用本地测试服
std::string GameCfg::testServerHost;   // 测试服服务器ip
int GameCfg::testServerPort=0;           // 测试服连接端口
std::string GameCfg::testAccount;   // 用户名
std::string GameCfg::testPassword; // 密码
int GameCfg::showFPS;       // ＦＰＳ
int GameCfg::testScene;     // 测试场景
int GameCfg::showDebug;   // 调试输出
ClientMode GameCfg::clientMode = eOnline;    // test mike
int GameCfg::silentmode = 0;
bool GameCfg::vipmod = 0;
bool GameCfg::isOpenAutoUpdater = false;
bool GameCfg::isOpenNetworkTimeout = true;
bool GameCfg::isOpenAtkBox = false;
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) //android平台才启用游戏内更新apk
bool GameCfg::isUpdateApkInGame = true;
#else
bool GameCfg::isUpdateApkInGame = false;
#endif
bool GameCfg::isEnableMemoryCrypt = true;

std::string GameCfg::clientPlatform = "";
std::string GameCfg::clientChannel = "";
std::string GameCfg::clientChildrenChannel = "";
std::string GameCfg::clientVersion = "";
std::string GameCfg::checkServerURL = "";
std::string GameCfg::bakCheckServerURL = "";
std::string GameCfg::serverMgrURL = "";
std::string GameCfg::curAutoUpdateVer = "";

std::string GameCfg::xinyouServerURL = "";
std::string GameCfg::xinyouKey = "";

std::string GameCfg::xinyouCollectURL = "";
int GameCfg::xinyouGameID = 1;

std::string GameCfg::enterGameCollectURL = "";

void GameCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    GameInifile commonInifile(Cfgable::getFullFilePath("gameCommon.ini"));
    
    isSkipLoginUI = commonInifile.getValueT("root","isSkipLoginUI",false);
    testServerHost = commonInifile.getValue("root","testServerHost","127.0.0.1");
    testServerPort = commonInifile.getValueT("root","testServerPort",8080);
    testAccount = commonInifile.getValue("root","testAccount","zwx");
    testPassword = commonInifile.getValue("root","testPassword","123");
    //showFPS = commonInifile.getValueT("root","showFPS",0);
    testScene = commonInifile.getValueT("root","testScene",0);
    showDebug = commonInifile.getValueT("root","showDebug",0);
    clientMode = (ClientMode)commonInifile.getValueT("root","clientMode",0);
    silentmode = commonInifile.getValueT("root", "silentmode", 0);
    vipmod = commonInifile.getValueT("root", "vipmod", 0);
    //isOpenAutoUpdater = commonInifile.getValueT("root", "isOpenAutoUpdater", false);
    isOpenNetworkTimeout = commonInifile.getValueT("root", "isOpenNetworkTimeout", true);
#ifdef DEBUG
    showFPS = commonInifile.getValueT("root","showFPS",0);
    isOpenAutoUpdater = commonInifile.getValueT("root", "isOpenAutoUpdater", false);
    isEnableMemoryCrypt = commonInifile.getValueT("root", "isEnableMemoryCrypt", true);
    isOpenAtkBox = commonInifile.getValueT("root", "isOpenAtkBox", false);
#else
    showFPS = 0;
    isOpenAutoUpdater = true;
    isEnableMemoryCrypt = true;
    isOpenAtkBox = false;
    
#endif
    //android平台才启用游戏内更新apk
#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    isUpdateApkInGame = commonInifile.getValueT("root", "isUpdateApkInGame", true);
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    isUpdateApkInGame = commonInifile.getValueT("root", "isUpdateApkInGame", false);
#endif
    //isEnableMemoryCrypt = commonInifile.getValueT("root", "isEnableMemoryCrypt", true);
//
    clientVersion = commonInifile.getValue("root","clientVersion","");
    clientChannel = inifile.getValue("root","clientChannel","");
    
#if defined(CLIENT)
    #if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    if(AndroidPlatformCfg::mChannelId > 0)
    {
        clientChannel = AndroidPlatformCfg::mClientChannel;
    }
    #endif
#endif

    checkServerURL = commonInifile.getValue("root","checkServerURL","");
    bakCheckServerURL = commonInifile.getValue("root","bakCheckServerURL","");
    clientPlatform = inifile.getValue("root", "clientPlatform","");
    serverMgrURL = commonInifile.getValue("root","serverMgrURL","");
    curAutoUpdateVer = commonInifile.getValue("root", "curAutoUpdateVer","");
//
    xinyouServerURL = commonInifile.getValue("root", "xinyouServerURL","");
    xinyouKey = commonInifile.getValue("root","xinyouKey","").c_str();
//    
    xinyouCollectURL = commonInifile.getValue("root", "xinyouCollectURL","");
    xinyouGameID = commonInifile.getValueT("root", "xinyouGameID", 0);
//
    enterGameCollectURL = commonInifile.getValue("root", "enterGameCollectURL","");
}

/*
 -----------------需要提交加载的配置项-----------------------------------------
 */
bool ClientPreloadCfg::isStrictCheckNetwork = false;
int ClientPreloadCfg::loadingSize = 1000000;
void ClientPreloadCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    ClientPreloadCfg::isStrictCheckNetwork = inifile.getValueT("root", "is_strict_check_net", false);
    ClientPreloadCfg::loadingSize = inifile.getValueT("root", "loadingSize", 1000000);
    
}
/*
 -----------------各个配置文件md5的配置项-----------------------------------------
 */
std::map<std::string, std::string> Md5VerifyCfg::allFileAndMd5s;
string Md5VerifyCfg::getCfgFileMd5(std::string fileName)
{
    if(allFileAndMd5s.find(fileName) != allFileAndMd5s.end())
    {
        return allFileAndMd5s[fileName];
    }
    else
        return "";
}

void Md5VerifyCfg::load(std::string fullPath)
{
#ifdef CLIENT
    CCLOG("allFileAndMd5s size:%d",allFileAndMd5s.size());
#endif
    GameInifile inifile(fullPath);
    
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        string fileName = inifile.getValue(*iter,"file","");
        string fileMd5 = inifile.getValue(*iter,"md5","");
        allFileAndMd5s[fileName]= fileMd5;
    }
}

/*
 -----------------所有美术资源表-----------------------------------------
 */

std::map<int, ResCfg*> ResCfg::sAllRes;


ResCfg* ResCfg::getCfg(int resId)
{
    std::map<int, ResCfg*>::iterator iter = sAllRes.find(resId);
    if (iter == sAllRes.end())
    {
        return NULL;
    }
    
    return iter->second;
}

void ResCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);

    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));

    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        ResCfg* res = new ResCfg();
        res->mID             = inifile.getValueT(*iter,"resID",0);
        res->mName           = inifile.getValue(*iter,"name","");
        res->mType           = (ResourceType)inifile.getValueT(*iter,"type",0);
        res->mIsNeedPreload  = inifile.getValueT(*iter, "needPreload", false);
        res->mIsNeedAsynload = inifile.getValueT(*iter, "needAsynload", false);
        res->mOrient         = inifile.getValueT(*iter, "orient", 1);

        res->mPath           = inifile.getValue(*iter,"path","");
        res->mPlist          = inifile.getValue(*iter,"plist","");

        string aniName      = inifile.getValue(*iter,"animationName","");
        StringTokenizer st(aniName,",");
        std::copy(st.begin(), st.end(), std::back_inserter(res->mAnimationNames));

        res->mAnchorOffsetX  = inifile.getValueT(*iter, "anchorOffsetX", 0.0f);
        res->mAnchorOffsetY  = inifile.getValueT(*iter, "anchorOffsetY", 0.0f);
        res->mScaleType      = (ResScaleType) inifile.getValueT(*iter, "scaleType", 0);
        res->mScaleX        = inifile.getValueT(*iter, "scaleX", 1.0f);
        res->mScaleY        = inifile.getValueT(*iter, "scaleY", 1.0f);
        res->mBoundingBoxX  = inifile.getValueT(*iter, "bbWidth", 0);
        res->mBoundingBoxY  = inifile.getValueT(*iter, "bbHeight", 0);
        res->mAttackLightID  = inifile.getValueT(*iter, "attackLightID", -1);
        res->mRealHeight  = inifile.getValueT(*iter, "realHeight", 0);
        sAllRes[res->mID] = res;
        
#ifndef CLIENT
        if (res->mPath == "Monster_nvwuboss")
        {
            int i = 0;
        }
        ResAnimationCfg::addResAnimation(res->mPath.c_str());
#endif
    }
}

std::string ResCfg::getAnimationName()
{
    if(mAnimationNames.size() == 1)
        return mAnimationNames[0];
    else if(mAnimationNames.size() > 1)
    {
        int index = rand()%mAnimationNames.size();
        return mAnimationNames[index];
    }
    else
        return "";
}

/*
 -----------------客户端预加载 美术资源表-----------------------------------------
 */

int  PreloadResCfg::VxinYouLogoResoures = 0;
int  PreloadResCfg::LoadingEffectResId = 0;

void PreloadResCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    
    try
    {
		int num = inifile.getValueT("common", "num", 0);
        
        PreloadResCfg::VxinYouLogoResoures = inifile.getValueT("common", "VxinYouLogoResoures", 0);
        PreloadResCfg::LoadingEffectResId   = inifile.getValueT("common", "loadingEffect", 0);
        
        assert(num > 0);
        
		for (int i=1; i <= num; i++) {
			char tmp[64] = {0};
			sprintf(tmp, "%d", i);
            
            ResCfg* res = new ResCfg();
            res->mID             = inifile.getValueT(tmp,"resID",0);
            res->mName           = inifile.getValue(tmp,"name","");
            res->mType           = (ResourceType)inifile.getValueT(tmp,"type",0);
            res->mIsNeedPreload  = inifile.getValueT(tmp, "needPreload", false);
            res->mIsNeedAsynload = inifile.getValueT(tmp, "needAsynload", false);
            res->mOrient         = inifile.getValueT(tmp, "orient", 1);
            
            res->mPath           = inifile.getValue(tmp,"path","");
            res->mPlist          = inifile.getValue(tmp,"plist","");
            
            string aniName      = inifile.getValue(tmp,"animationName","");
            StringTokenizer st(aniName,",");
            std::copy(st.begin(), st.end(), std::back_inserter(res->mAnimationNames));
            
            res->mAnchorOffsetX  = inifile.getValueT(tmp, "anchorOffsetX", 0.0f);
            res->mAnchorOffsetY  = inifile.getValueT(tmp, "anchorOffsetY", 0.0f);
            res->mScaleType      = (ResScaleType) inifile.getValueT(tmp, "scaleType", 0);
            res->mScaleX        = inifile.getValueT(tmp, "scaleX", 1.0f);
            res->mScaleY        = inifile.getValueT(tmp, "scaleY", 1.0f);
            res->mBoundingBoxX  = inifile.getValueT(tmp, "bbWidth", 0);
            res->mBoundingBoxY  = inifile.getValueT(tmp, "bbHeight", 0);
            res->mAttackLightID  = inifile.getValueT(tmp, "attackLightID", -1);
            res->mRealHeight  = inifile.getValueT(tmp, "realHeight", 0);
            ResCfg::sAllRes[res->mID] = res;

		}

    }
    catch(...)
    {
        log_error("PreloadRes ERROR !!!");
    }
}

std::string PreloadResCfg::getAnimationName()
{
    if(mAnimationNames.size() == 1)
        return mAnimationNames[0];
    else if(mAnimationNames.size() > 1)
    {
        int index = rand()%mAnimationNames.size();
        return mAnimationNames[index];
    }
    else
        return "";
}

/*
 -----------------声音资源表-----------------------------------------
 */
std::map<int, AudioCfg*> AudioCfg::sAllAudios;

void AudioCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);

    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));

    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        AudioCfg* audio = new AudioCfg();
        audio->mID              = inifile.getValueT(*iter,"id",0);
        audio->mName            = inifile.getValue(*iter,"name","");
        audio->mDes             = inifile.getValue(*iter,"descript","");
        audio->mFileName        = inifile.getValue(*iter,"file_name","");
        audio->mType            = (AudioType)inifile.getValueT(*iter,"type",0);
        audio->mIsLoop          = inifile.getValueT(*iter,"is_loop",false);
        audio->mVolume          = inifile.getValueT(*iter,"volume",1.0f);
        sAllAudios[audio->mID] = audio;
    }
}

/*
 -----------------状态冲突表-----------------------------------------
 */
ObjState StateMutexCfg::stringToObjState( std::string str )
{
    return
    (str == "idle") ? eIDLE :
    (str == "walk") ? eWALK :
    (str == "attack") ? eATTACK :
    (str == "hitted") ? eHITTED :
    (str == "cast") ? eCAST :
    (str == "skill") ? eSKILL :
    (str == "dead") ? eDEAD :
    eNone;
}

std::map<ObjState, std::map<ObjState, bool> > StateMutexCfg::sObjStateMutexTable;

void StateMutexCfg::setStateMutexTable(ObjState fromState, ObjState toState, bool canChange)
{
    sObjStateMutexTable[fromState][toState] = canChange;
}

void StateMutexCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);

    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));

    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        StateMutexCfg* tmp = new StateMutexCfg();
        tmp->state = inifile.getValue(*iter,"state");
        ObjState sta = StateMutexCfg::stringToObjState(tmp->state);

        tmp->none = inifile.getValueT(*iter,"none", true);
        StateMutexCfg::setStateMutexTable(sta, eNone, tmp->none);

        tmp->idle = inifile.getValueT(*iter,"idle", true);
        StateMutexCfg::setStateMutexTable(sta, eIDLE, tmp->idle);

        tmp->walk = inifile.getValueT(*iter,"walk", true);
        StateMutexCfg::setStateMutexTable(sta, eWALK, tmp->walk);

        tmp->attack = inifile.getValueT(*iter,"attack", true);
        StateMutexCfg::setStateMutexTable(sta, eATTACK, tmp->attack);

        tmp->hitted = inifile.getValueT(*iter,"hitted", true);
        StateMutexCfg::setStateMutexTable(sta, eHITTED, tmp->hitted);

        tmp->cast = inifile.getValueT(*iter,"cast", true);
        StateMutexCfg::setStateMutexTable(sta, eCAST, tmp->cast);

        tmp->skill = inifile.getValueT(*iter,"skill", true);
        StateMutexCfg::setStateMutexTable(sta, eSKILL, tmp->skill);

        tmp->dead = inifile.getValueT(*iter,"dead", true);
        StateMutexCfg::setStateMutexTable(sta, eDEAD, tmp->dead);
        delete tmp;

    }
}

/*
 -----------------角色模板表-----------------------------------------
 */

ObjSex RoleTpltCfg::stringToObjSex(std::string str)
{
    return
    (str == "boy") ? eBoy :
    (str == "girl") ? eGirl :

    eBoy;
}

ObjJob RoleTpltCfg::stringToObjJob(std::string str)
{
    return
    (str == "warrior") ? eWarrior :
    (str == "mage") ? eMage :
    (str == "assassin") ? eAssassin :
    eWarrior;
}
string RoleTpltCfg::objJobToString(ObjJob job)
{
    return
    (job == eWarrior) ? "warrior" :
    (job == eMage) ? "mage" :
    (job == eAssassin) ? "assassin" :
    "";
}
std::map<int, RoleTpltCfg*> RoleTpltCfg::sAllRoleTplts;

void RoleTpltCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    //by lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("role_tpltMultiLanguage.ini") == "role_tpltMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("role_tpltMultiLanguage.ini"));
    }
    //GameInifile MLIniFile(Cfgable::getFullFilePath("role_tpltMultiLanguage.ini"));
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));

    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        RoleTpltCfg* roleT = new RoleTpltCfg();
        roleT->mID             = inifile.getValueT(*iter,"id",0);
        roleT->mName           = inifile.getValue(*iter,"name","");
        roleT->mResID          = inifile.getValueT(*iter,"res_id",0);
        roleT->mJob     = stringToObjJob(inifile.getValue(*iter,"job",""));
        roleT->mSex     = stringToObjSex(inifile.getValue(*iter,"sex",""));
        roleT->setMaxAnger(inifile.getValueT(*iter,"max_anger",0));
        roleT->mHeadImage  = inifile.getValue(*iter, "headimage");
        roleT->mBagheadImage = inifile.getValue(*iter, "halfimage");
        string job_name_id = inifile.getValue(*iter, "job_name_id");
        //by lihengjin
        string str_job_name;
        if(inifile.exists(*iter, "job_name"))
        {
            str_job_name = inifile.getValue(*iter, "job_name");
        }else
        {
            str_job_name = MLIniFile.getValue(job_name_id, "desc");
        }

        roleT->mJobName = str_job_name;//inifile.getValue(*iter, "job_name");
        roleT->mAttackMoveDistance = inifile.getValueT(*iter,"attack_move",0);
        roleT->mAttackMoveTime = inifile.getValueT(*iter,"attack_move_time",0.0f);
        roleT->mLastAttackHitBackDistance = inifile.getValueT(*iter,"last_attack_hit_back",0);
        roleT->mLastAttackHitBackTime = inifile.getValueT(*iter,"last_attack_hit_back_time",0.0f);

        //
        string animationTimes = inifile.getValue(*iter, "attack_animation_times");
        StringTokenizer aniT(animationTimes, ",");
        for (int i = 0; i < aniT.count(); i++)
        {
            roleT->mAnimationTimes.push_back(atof(aniT[i].c_str()));
        }
        
        //攻击时间点
        string attackTimes      = inifile.getValue(*iter,"attack_delay_times","");
        StringTokenizer stAT(attackTimes,",");
        for (int i=0; i<stAT.count(); i++)
        {
            roleT->mAttackTimes.push_back(atof(stAT[i].c_str()));
        }

        roleT->mLastAttackAddRate   = inifile.getValueT(*iter,"last_attack_add_rate",0.0f);


        string attackAudios      = inifile.getValue(*iter,"attack_audios","");
        StringTokenizer stAA(attackAudios,",");
        for (int i=0; i<stAA.count(); i++)
        {
            roleT->mAttackAudios.push_back(atoi(stAA[i].c_str()));
        }

        string hittedAudios      = inifile.getValue(*iter,"hitted_audios","");
        StringTokenizer stHA(hittedAudios,",");
        for (int i=0; i<stHA.count(); i++)
        {
            roleT->mHittedAudios.push_back(atoi(stHA[i].c_str()));
        }

        string attackScreamAudios      = inifile.getValue(*iter,"attack_scream_audios","");
        StringTokenizer stASA(attackScreamAudios,",");
        for (int i=0; i<stASA.count(); i++)
        {
            roleT->mAttackScreams.push_back(atoi(stASA[i].c_str()));
        }

        string hittedScreamAudios      = inifile.getValue(*iter,"hitted_scream_audios","");
        StringTokenizer stHSA(hittedScreamAudios,",");
        for (int i=0; i<stHSA.count(); i++)
        {
            roleT->mHittedScreams.push_back(atoi(stHSA[i].c_str()));
        }
        roleT->mWalkAudio = inifile.getValueT(*iter,"walk_audio",0);
        roleT->mDeadAudio = inifile.getValueT(*iter,"dead_audio",0);
        roleT->mKOSreamAudio = inifile.getValueT(*iter,"ko_scream_audio",0);
        
        roleT->mSkillInitialvalue.clear();
        string skillStr = inifile.getValue(*iter, "skill_list");
        StringTokenizer token(skillStr, ";");
        for(int i=0;i< token.count();i++)
        {
            roleT->mSkillInitialvalue.push_back(atoi(token[i].c_str()));
        }
        sAllRoleTplts[roleT->mID] = roleT;
    }
}

RoleTpltCfg* RoleTpltCfg::getCfg(int roleType)
{
    std::map<int, RoleTpltCfg*>::iterator iter = sAllRoleTplts.find(roleType);
    
    if (iter != sAllRoleTplts.end()) {
        return iter->second;
    }
    
    return NULL;
}
/*
 -----------------零碎的资源-----------------------------------------
 */
int SeparateResIDs::sBattleNumber = 0;
int SeparateResIDs::sBruiseEffect = 0;
int SeparateResIDs::sBattleKO = 0;
int SeparateResIDs::sMonsterDead = 0;
int SeparateResIDs::sRoleDead = 0;
int SeparateResIDs::sRoleLevelUp = 0;
int SeparateResIDs::sGetQuest = 0;
int SeparateResIDs::sSubmitQuest = 0;
int SeparateResIDs::sOpenBox = 0;
int SeparateResIDs::sOpenGift = 0;
int SeparateResIDs::sOpenStrengthen = 0;
int SeparateResIDs::sOpenMention = 0;
int SeparateResIDs::sOpenMosaic = 0;
int SeparateResIDs::sUseItem = 0;
int SeparateResIDs::sActivity = 0;
int SeparateResIDs::sMainUIcircle = 0;
int SeparateResIDs::sGodHit = 0;
int SeparateResIDs::sFunnyActEffectID = 0;
int SeparateResIDs::sEnableEvolveID = 0;
int SeparateResIDs::sAutoLearnSkillEffectID = 0;
int SeparateResIDs::sEnchantLvlUp = 0;

int SeparateResIDs::sVoiceReqLoading = 0;

//语音请求时的自己的播放效果
int SeparateResIDs::sVoicePlayMyselfAction=0;

//语音请求时的其他人的播放效果
int SeparateResIDs::sVoicePlayOtherAction=0;

vector<int> SeparateResIDs::sQualityEffect;

int SeparateResIDs::sFirstChargePetAnimation=0;
string SeparateResIDs::sFirstcharge_skillname="";
int SeparateResIDs::sStarUp = 0;
int SeparateResIDs::sSynthetic = 0;
int SeparateResIDs::sSettlement = 0;
int SeparateResIDs::sQualityBleat = 0;
int SeparateResIDs::sPetCollectTip = 0;
int SeparateResIDs::sPetCollectAward = 0;
int SeparateResIDs::sEscapeEffect = 0;
int SeparateResIDs::sWritingNetWork = 0;
int SeparateResIDs::sHelperGoing = 0;
int SeparateResIDs::sTitleGet = 0;
int SeparateResIDs::sBattleEnd = 0;
vector<int> SeparateResIDs::sWaveCounts;
string SeparateResIDs::sFirstChargeAnims="";
string SeparateResIDs::sFirstChargePetSkills="";
int SeparateResIDs::sResetGrowvalue = 0;
int SeparateResIDs::sStarAndSkillUpAnimation=0;
int SeparateResIDs::sNewFunction = 0;
int SeparateResIDs::sHelperNotice = 0;
int SeparateResIDs::sDiceRollAnim = 0;
int SeparateResIDs::sDiceAwardAnim = 0;
int SeparateResIDs::sEquip_starlvlup = 0;
int SeparateResIDs::sQueenBlessingBefore = 0;
int SeparateResIDs::sQueenBlessingBack = 0;
int SeparateResIDs::sPet_stageup = 0;
int SeparateResIDs::sPet_stageup_everytime = 0;
int SeparateResIDs::sPet_eating_everytime = 0;
int SeparateResIDs::sCapsuleToy = 0;
int SeparateResIDs::sQuestAcceptable = 0;
int SeparateResIDs::sQuestFinished = 0;
int SeparateResIDs::sQuestDoing = 0;
int SeparateResIDs::sDailySchedule = 0;
int SeparateResIDs::sFunnytoy_activityBtn = 0;

void SeparateResIDs::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        SeparateResIDs::sBattleNumber = inifile.getValueT(*iter,"battle_number",0);
        SeparateResIDs::sBruiseEffect = inifile.getValueT(*iter,"bruise_effect",0);
        SeparateResIDs::sBattleKO = inifile.getValueT(*iter,"battle_ko",0);
        SeparateResIDs::sMonsterDead = inifile.getValueT(*iter,"monster_dead",0);
        SeparateResIDs::sRoleDead = inifile.getValueT(*iter,"role_dead",0);
        SeparateResIDs::sRoleLevelUp = inifile.getValueT(*iter,"role_level_up",0);
        SeparateResIDs::sGetQuest = inifile.getValueT(*iter, "get_quest", 0);
        SeparateResIDs::sSubmitQuest = inifile.getValueT(*iter, "submit_quest", 0);
        SeparateResIDs::sOpenBox = inifile.getValueT(*iter, "open_box", 0);
        SeparateResIDs::sOpenGift = inifile.getValueT(*iter, "open_gift", 0);
        SeparateResIDs::sOpenStrengthen = inifile.getValueT(*iter, "open_strengthen", 0);
        SeparateResIDs::sOpenMention = inifile.getValueT(*iter, "open_mention", 0);
        SeparateResIDs::sOpenMosaic = inifile.getValueT(*iter, "open_mosaic", 0);
        SeparateResIDs::sUseItem = inifile.getValueT(*iter, "use_item", 0);
        SeparateResIDs::sActivity = inifile.getValueT(*iter, "activity_open", 0);
        SeparateResIDs::sFirstChargePetAnimation=inifile.getValueT(*iter, "firstcharge_animation", 0);
        SeparateResIDs::sFirstcharge_skillname=inifile.getValue(*iter, "firstcharge_skillname");
        SeparateResIDs::sMainUIcircle=inifile.getValueT(*iter, "MainUI_circle",0);
        SeparateResIDs::sQuestAcceptable = inifile.getValueT(*iter, "QuestAcceptable",0);
        SeparateResIDs::sQuestFinished = inifile.getValueT(*iter, "QuestFinished",0);
        SeparateResIDs::sQuestDoing = inifile.getValueT(*iter, "QuestDoing",0);
        SeparateResIDs::sGodHit = inifile.getValueT(*iter, "GodHit", 0);
        SeparateResIDs::sTitleGet = inifile.getValueT(*iter, "new_title", 0);
        SeparateResIDs::sBattleEnd = inifile.getValueT(*iter, "Battle_end_item", 0);
        SeparateResIDs::sFirstChargeAnims = inifile.getValue(*iter, "vip_award_pet");
        SeparateResIDs::sFirstChargePetSkills = inifile.getValue(*iter, "vip_award_skill");
        SeparateResIDs::sDailySchedule = inifile.getValueT(*iter, "dailySchedule_boxlight",0);
        for(int i=0;i<5;i++)
        {
            string name = Utils::makeStr("qua_animation_%d",i+ 1);
            int resoureID = inifile.getValueT(*iter, name.c_str(), i+1);
            SeparateResIDs::sQualityEffect.push_back(resoureID);
        }
        
        SeparateResIDs::sStarUp = inifile.getValueT(*iter, "star_light",0);
        SeparateResIDs::sSynthetic = inifile.getValueT(*iter, "makeup_item", 0);
        SeparateResIDs::sSettlement = inifile.getValueT(*iter, "EndStar",0);
        SeparateResIDs::sQualityBleat = inifile.getValueT(*iter, "QualityBleat", 0);
        SeparateResIDs::sPetCollectTip = inifile.getValueT(*iter, "PetCollect_tip", 0);
        SeparateResIDs::sPetCollectAward = inifile.getValueT(*iter, "PetCollect_award", 0);
        SeparateResIDs::sEscapeEffect = inifile.getValueT(*iter, "escape_effect", 0);
        SeparateResIDs::sWritingNetWork = inifile.getValueT(*iter, "waiting_circle", 0);
        SeparateResIDs::sHelperGoing = inifile.getValueT(*iter, "Helper_Going", 0);
        SeparateResIDs::sResetGrowvalue = inifile.getValueT(*iter, "reset_growvalue", 0);
        //升星和技能升级 特效
        SeparateResIDs::sStarAndSkillUpAnimation=inifile.getValueT(*iter, "skill_levelup", 0);
        SeparateResIDs::sNewFunction = inifile.getValueT(*iter, "new_function", 0);
        SeparateResIDs::sHelperNotice = inifile.getValueT(*iter, "helper_notice", 0);
        SeparateResIDs::sDiceRollAnim = inifile.getValueT(*iter, "Die_dice", 0);
        SeparateResIDs::sDiceAwardAnim = inifile.getValueT(*iter,"Dice_Award", 0);
        SeparateResIDs::sEquip_starlvlup = inifile.getValueT(*iter, "equip_starlvlup", 0);
        SeparateResIDs::sQueenBlessingBefore = inifile.getValueT(*iter, "Queen_Blessing_before", 0);
        SeparateResIDs::sQueenBlessingBack  = inifile.getValueT(*iter, "Queen_Blessing_back", 0);
        SeparateResIDs::sPet_stageup = inifile.getValueT(*iter, "pet_stageup", 0);
        SeparateResIDs::sPet_stageup_everytime = inifile.getValueT(*iter, "pet_stageup_everytime", 0);
        SeparateResIDs::sPet_eating_everytime = inifile.getValueT(*iter, "pet_eating_everytime", 0);
        SeparateResIDs::sCapsuleToy = inifile.getValueT(*iter, "funnytoy",0);
        SeparateResIDs::sFunnyActEffectID = inifile.getValueT(*iter, "funnyActEffectID", 0);
        SeparateResIDs::sEnableEvolveID =inifile.getValueT(*iter, "enable_Evolve", 0);
        SeparateResIDs::sAutoLearnSkillEffectID = inifile.getValueT(*iter, "autoLearnSkillEffectID", 0);
        SeparateResIDs::sEnchantLvlUp = inifile.getValueT(*iter, "Enchant_LvlUp", 0);
        for (int i = 1; i <=10; i++)
        {
            SeparateResIDs::sWaveCounts.push_back(inifile.getValueT(*iter, Utils::makeStr("counts_%d",i).c_str(), 0));
        }
        SeparateResIDs::sFunnytoy_activityBtn = inifile.getValueT(*iter, "funnytoy_activityBtn",0);

        //读取语音播放效果的id
        SeparateResIDs::sVoiceReqLoading = inifile.getValueT(*iter, "waiting_circle",0);
        
        //语音请求时的自己的播放效果
        SeparateResIDs::sVoicePlayMyselfAction = inifile.getValueT(*iter, "chatting_myVoice",0);
        
        //语音请求时的其他人的播放效果
        SeparateResIDs::sVoicePlayOtherAction = inifile.getValueT(*iter, "chatting_othersVoice",0);
    }
}
/*
 -----------------零碎的数值-----------------------------------------
 */
int SeparateValues::sRoleMaxCombo = 4;
float SeparateValues::sCombeTime = 1;
float SeparateValues::sDefaultAttackMoveDistance = 20;
float SeparateValues::sDeadMoveDistance = 200;
float SeparateValues::sDeadMoveTime = 0.3;
float SeparateValues::sDefaultAttackMoveTime = 0.25;
float SeparateValues::sKOSlowMotionTime = 3;
float SeparateValues::sKOSlowMotionScale = 0.15;
int SeparateValues::sWorldbossWaitTime = 10;
string SeparateValues::sOfficalBBSUrl = "http://bbs.vxinyou.com";
int SeparateValues::sWorldChatCd = 0;
int SeparateValues::sIncrHpFrequency = 0;
int SeparateValues::sPaihangFreshPeriod = 600;
std::string SeparateValues::platformNoticeURL="";
std::string SeparateValues::jumpWebsiteURL = "";
vector<int> SeparateValues::lvlUpCollect;
int SeparateValues::isShowAd = 0;
std::string SeparateValues::chargeURL = "";
float SeparateValues::sJoystickResetTime = 0;

float SeparateValues::sJoystickLeftBorder = 0;
float SeparateValues::sJoystickBelowtBorder =0;
float SeparateValues::sTipsIntervalTime = 0;
int SeparateValues::sPetEliteLimitLv[2];
int SeparateValues::sEachFrameCreateObj = 3;
float SeparateValues::sLoadingShortestTime = 1;
bool  SeparateValues::sIsShowTouristsLoginImage = false;
vector<int> SeparateValues::mNoLoagingScenes;
vector<eSceneType> SeparateValues::mOpenChatInCopy;
vector<string> SeparateValues::sInvalidCurrencyCode;
int SeparateValues::mStoreSkillActionCount = 10;


void SeparateValues::load(std::string fullPath)
{
    GameInifile inifile(fullPath);

    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));

    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        SeparateValues::sRoleMaxCombo = inifile.getValueT(*iter,"role_max_combo",4);
        SeparateValues::sCombeTime = inifile.getValueT(*iter,"combo_time",1.0);
        SeparateValues::sDefaultAttackMoveDistance = inifile.getValueT(*iter,"default_attack_move_distance",20.0);
        SeparateValues::sDefaultAttackMoveTime = inifile.getValueT(*iter,"default_attack_move_time",0.25);
        SeparateValues::sDeadMoveDistance = inifile.getValueT(*iter,"dead_move_distance",200.0);
        SeparateValues::sDeadMoveTime = inifile.getValueT(*iter,"dead_move_time",0.3);

        SeparateValues::sKOSlowMotionTime = inifile.getValueT(*iter,"ko_slow_motion_time",3.0);
        SeparateValues::sKOSlowMotionScale = inifile.getValueT(*iter,"ko_slow_motion_scale",0.15);
        SeparateValues::sWorldbossWaitTime = inifile.getValueT(*iter, "worldboss_waitdeadtime", 15);
        SeparateValues::sOfficalBBSUrl = inifile.getValue(*iter, "bbs_address");
        SeparateValues::sWorldChatCd = inifile.getValueT(*iter, "worldchat_cd", 0);
        SeparateValues::sIncrHpFrequency  = inifile.getValueT(*iter, "guildskill_pethp_time", 10);
        int paihangFreshperiod = inifile.getValueT(*iter, "paihang_fresh_period", 3600);
        platformNoticeURL=inifile.getValue(*iter, "platformNoticeURL","").c_str();//平台公告URL
        //排行榜刷新周期，大于10分钟
        check_min(paihangFreshperiod, 600);
        SeparateValues::sPaihangFreshPeriod = paihangFreshperiod;
        jumpWebsiteURL = inifile.getValue(*iter, "jumpWebsiteURL");
        //升级数据收集
        SeparateValues::lvlUpCollect.clear();
        string _sLv = inifile.getValue(*iter, "lvlup_info_collect","");
        StringTokenizer st(_sLv,";");
        for (int i=0; i<st.count(); i++) {
            SeparateValues::lvlUpCollect.push_back(atoi(st[i].c_str()));
        }
        SeparateValues::isShowAd = inifile.getValueT(*iter, "is_show_ad", 0);
        SeparateValues::chargeURL = inifile.getValue(*iter, "chargeURL");
        SeparateValues::sJoystickResetTime = inifile.getValueT(*iter, "joystick_reset_time", 0.0);
        
        SeparateValues::sJoystickLeftBorder = inifile.getValueT(*iter, "joystick_left_border", 0.0);
        SeparateValues::sJoystickBelowtBorder = inifile.getValueT(*iter, "joystick_below_border", 0.0);
        SeparateValues::sTipsIntervalTime = inifile.getValueT(*iter, "tips_interval_time", 0.8);
        string sPetEliteLv = inifile.getValue(*iter, "PetTrain_slot_lvl");
        StringTokenizer sPetElite(sPetEliteLv, ";");
        for (int i = 0; i < sPetElite.count(); i++)
        {
            SeparateValues::sPetEliteLimitLv[i] = Utils::safe_atoi(sPetElite[i].c_str());
        }
        SeparateValues::sEachFrameCreateObj = inifile.getValueT(*iter, "eachframe_create_obj", 3);
        SeparateValues::sLoadingShortestTime = inifile.getValueT(*iter, "loading_shortest_time", 1.0);
        
        SeparateValues::sIsShowTouristsLoginImage = (bool)inifile.getValueT(*iter, "isshow_touristlogin_image", 1);
        
        string noLoadingStr = inifile.getValue(*iter, "noLoading");
        StringTokenizer token(noLoadingStr, ";");
        for(int i=0; i< token.count(); i++)
        {
            mNoLoagingScenes.push_back(atoi(token[i].c_str()));
        }
        string openChatInCopyStr = inifile.getValue(*iter, "openChatInCopy");
        StringTokenizer copyToken(openChatInCopyStr, ";");
        for(int i=0; i< copyToken.count(); i++)
        {
            mOpenChatInCopy.push_back((eSceneType)atoi(copyToken[i].c_str()));
        }
        //无效的货币列表
        string invalidCurrencyCode = inifile.getValue(*iter, "invalidCurrencyCode");
        StringTokenizer invalidCurrencyCodeList(invalidCurrencyCode, ";");
        for(int i=0; i< invalidCurrencyCodeList.count(); i++)
        {
            sInvalidCurrencyCode.push_back(invalidCurrencyCodeList[i]);
        }
        SeparateValues::mStoreSkillActionCount = inifile.getValueT(*iter, "store_SkillAction_count",10);
        
    }
}

/*
 ----------------------------------------------------------
 int exp;

 int stre;  //力量
 int inte;  //智力
 int phys;  //体力
 int capa;  //耐力

 int hp;
 int mp;
 int atk;
 int def;
 float hit;
 float dodge;
 float cri;

 int energe;
 float hpper;
 int hpnum;
 int mpnum;
 */


RoleCfgDef::RoleCfgDef()
{

    setWalkSpeed(0);
    setAtkSpeed(0);
    setExp(0);
    setStre(0);
    setInte(0);
    setPhys(0);
    setCapa(0);
    
    setHp(0);
    setMp(0);
    setAtk(0);
    setDef(0);
    setHit(0.0);
    setDodge(0.0);
    setCri(0.0);
    setCrip(0.0);
    
    setMaxfriendly(0);
    setEnerge(0);
    setHpper(0);
    setHpnum(0);
    setMpnum(0);
    setDungfreeTimes(0);
    
    setElitefreeTimes(0);
    setTowertimes(0);
    setFriendDungeTimes(0);
    setPetElitefreeTimes(0);
    setPetAdventurefreeTimes(0);
    setPetAdventureRobotPetfreeRentedTimes(0);
    setCrossServiceWarWorShipFreeTimes(0);
    setCrossServiceWarFreeTimes(0);

}
RoleCfgDef& RoleCfg::getCfg(int job,int lvl){
    assert(job<MAX_ROLE_JOB && lvl <= maxRoleLvl && lvl > 0);
    return cfg[job][lvl - 1];
}
RoleCfgDef RoleCfg::cfg[MAX_ROLE_JOB][MAX_ROLE_LVL + 1];
int RoleCfg::maxRoleLvl = MAX_ROLE_LVL;
int RoleCfg::crossServiceWarIsWorShipTimes = 0;

int RoleCfg::getMaxRoleLvl()
{
    return maxRoleLvl;
}
int RoleCfg::getCrossServiceWarIsWorShipTimes()
{
    return crossServiceWarIsWorShipTimes;
}

void RoleCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    string tmp;
    
    maxRoleLvl = inifile.getValueT("common", "max_role_lvl", MAX_ROLE_LVL);
    crossServiceWarIsWorShipTimes = inifile.getValueT("common", "crossservicewarisworshiptimes", 0);
    
    assert(maxRoleLvl <= MAX_ROLE_LVL);
    assert(maxRoleLvl >= PREV_MAX_ROLE_LVL);

    const char* jobeffdescs[MAX_ROLE_JOB] = {"warrior_lvl_affect", "master_lvl_affect", "ninja_lvl_affect"};
    for (int job = 0; job < MAX_ROLE_JOB; job++)
    {

        tmp = inifile.getValue(jobeffdescs[job],"exp","");
        StringTokenizer st(tmp,",");
        assert(st.count() == getMaxRoleLvl());
        for (int i=0; i<st.count(); i++) {
            cfg[job][i].setExp(atoi(st[i].c_str()));
        }

        tmp = inifile.getValue(jobeffdescs[job],"hp","");
        StringTokenizer st2(tmp,",");
        assert(st2.count() == getMaxRoleLvl() + 1);
        for (int i=0; i<st2.count(); i++) {
            cfg[job][i].setHp(atoi(st2[i].c_str()));
        }

        tmp = inifile.getValue(jobeffdescs[job],"atk","");
        StringTokenizer st3(tmp,",");
        assert(st3.count() == getMaxRoleLvl() + 1);
        for (int i=0; i<st3.count(); i++) {
            cfg[job][i].setAtk(atoi(st3[i].c_str()));
        }

        tmp = inifile.getValue(jobeffdescs[job],"def","");
        StringTokenizer st4(tmp,",");
        assert(st4.count() == getMaxRoleLvl() + 1);
        for (int i=0; i<st4.count(); i++) {
            cfg[job][i].setDef(atoi(st4[i].c_str()));
        }

        tmp = inifile.getValue(jobeffdescs[job], "stre", "");
        StringTokenizer st5(tmp, ",");
        assert(st5.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st5.count(); i++)
        {
            cfg[job][i].setStre(atoi(st5[i].c_str()));
        }

        tmp = inifile.getValue(jobeffdescs[job], "inte", "");
        StringTokenizer st6(tmp, ",");
        assert(st6.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st6.count(); i++)
        {
            cfg[job][i].setInte(atoi(st6[i].c_str()));
        }

        tmp = inifile.getValue(jobeffdescs[job], "phys", "");
        StringTokenizer st7(tmp, ",");
        assert(st7.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st7.count(); i++)
        {
            cfg[job][i].setPhys(atoi(st7[i].c_str()));
        }

        tmp = inifile.getValue(jobeffdescs[job], "capa", "");
        StringTokenizer st8(tmp, ",");
        assert(st8.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st8.count(); i++)
        {
            cfg[job][i].setCapa(atoi(st8[i].c_str()));
        }

        tmp = inifile.getValue(jobeffdescs[job], "dungfreetimes", "");
        StringTokenizer st9(tmp, ",");
        assert(st9.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st9.count(); i++)
        {
            cfg[job][i].setDungfreeTimes(atoi(st9[i].c_str()));
        }

        tmp = inifile.getValue(jobeffdescs[job], "elitefreetimes", "");
        StringTokenizer st10(tmp, ",");
        assert(st10.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st10.count(); i++)
        {
            cfg[job][i].setElitefreeTimes(atoi(st10[i].c_str()));
        }
        
        tmp = inifile.getValue(jobeffdescs[job], "towertimes", "");
        StringTokenizer st11(tmp, ",");
        
        assert(st11.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st11.count(); i++)
        {
            cfg[job][i].setTowertimes(atoi(st11[i].c_str()));
    
        }
        
        tmp = inifile.getValue(jobeffdescs[job], "friend_dunge_times", "");
        StringTokenizer st12(tmp, ",");
        assert(st12.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st12.count(); i++)
        {
            cfg[job][i].setFriendDungeTimes(atoi(st12[i].c_str()));
        }
        
        tmp = inifile.getValue(jobeffdescs[job], "petelitefreetimes", "");
        StringTokenizer st13(tmp, ",");
        
        assert(st13.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st13.count(); i++)
        {
            cfg[job][i].setPetElitefreeTimes(atoi(st13[i].c_str()));
        }
        
        tmp = inifile.getValue(jobeffdescs[job], "petadventurefreetimes", "");
        StringTokenizer st14(tmp, ",");
        assert(st14.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st14.count(); i++)
        {
            cfg[job][i].setPetAdventurefreeTimes(atoi(st14[i].c_str()));
        }
        
        tmp = inifile.getValue(jobeffdescs[job], "petadventure_robotpet_rent_freetimes", "");
        StringTokenizer st15(tmp, ",");
//        assert(st15.count() == getMaxRoleLvl() + 1);
        for (int i = 0; i < st15.count(); i++)
        {
            cfg[job][i].setPetAdventureRobotPetfreeRentedTimes(atoi(st15[i].c_str()));
        }
        
        tmp = inifile.getValue(jobeffdescs[job], "crossservicewarworshipfreetimes", "");
        StringTokenizer st16(tmp, ",");
        for(int i = 0; i< st16.count(); i++)
        {
            cfg[job][i].setCrossServiceWarWorShipFreeTimes(atoi(st16[i].c_str()));
        }
        
        tmp = inifile.getValue(jobeffdescs[job], "crossservicewarfreetimes", "");
         StringTokenizer st17(tmp, ",");
        for(int i = 0; i< st17.count(); i++)
        {
            cfg[job][i].setCrossServiceWarFreeTimes(atoi(st17[i].c_str()));
        }
    }


    const char* jobefffix[MAX_ROLE_JOB] = {"warrior_affect", "master_affect", "ninja_affect"};
    for (int job = 0; job < MAX_ROLE_JOB; job++)
    {


        string tmp = inifile.getValue(jobefffix[job],"mp","");
        int mp = atoi(tmp.c_str());
        for (int i = 0; i <= getMaxRoleLvl(); i++)
        {
            cfg[job][i].setMp(mp);
        }
        tmp = inifile.getValue(jobefffix[job],"energy","");
        int energe = atoi(tmp.c_str());
        for (int i = 0; i <= getMaxRoleLvl(); i++)
        {
            cfg[job][i].setEnerge(energe);
        }
        tmp = inifile.getValue(jobefffix[job],"hpnum","");
        int hpnum = atoi(tmp.c_str());
        for (int i = 0; i <= getMaxRoleLvl(); i++)
        {
            cfg[job][i].setHpnum(hpnum);
        }
        tmp = inifile.getValue(jobefffix[job],"mpnum","");
        int mpnum = atoi(tmp.c_str());
        for (int i = 0; i <= getMaxRoleLvl(); i++)
        {
            cfg[job][i].setMpnum(mpnum);
        }

        tmp = inifile.getValue(jobefffix[job],"hpper","");
        float hpper = atof(tmp.c_str());
        for (int i = 0; i <= getMaxRoleLvl(); i++)
        {
            cfg[job][i].setHpper(hpper);
        }

        tmp = inifile.getValue(jobefffix[job],"hit","");
        float hit = atof(tmp.c_str());
        for (int i = 0; i <= getMaxRoleLvl(); i++)
        {
            cfg[job][i].setHit(hit);
        }

        tmp = inifile.getValue(jobefffix[job],"dodge","");
        float dodge = atof(tmp.c_str());
        for (int i = 0; i <= getMaxRoleLvl(); i++)
        {
            cfg[job][i].setDodge(dodge);
        }


        tmp = inifile.getValue(jobefffix[job],"cri","");
        float cri = atof(tmp.c_str());
        for (int i = 0; i <= getMaxRoleLvl(); i++)
        {
            cfg[job][i].setCri(cri);
        }

        tmp = inifile.getValue(jobefffix[job],"crip","");
        float crip = atof(tmp.c_str());
        for (int i = 0; i <= getMaxRoleLvl(); i++)
        {
            cfg[job][i].setCrip(crip);
        }

        tmp = inifile.getValue(jobefffix[job],"maxfriendly","");
        int maxfriendly = atof(tmp.c_str());
        for (int i = 0; i <= getMaxRoleLvl(); i++)
        {
            cfg[job][i].setMaxfriendly(maxfriendly);
        }
    }

}



map<string, ResAnimationCfgDef> ResAnimationCfg::cfg;
const int ResAnimationCfg::fps = 40;



float ResAnimationCfg::getAnimationDuration(const char* resName, const char* aniName)
{
    map<string, ResAnimationCfgDef>::iterator iter = cfg.find(resName);
    if (iter == cfg.end())
    {
        return 0.0f;
    }
    
    for (int i = 0; i < iter->second.mAnimations.size(); i++)
    {
        if (iter->second.mAnimations[i].mName == aniName)
        {
            return iter->second.mAnimations[i].mDuration;
        }
    }
    
    return 0.0f;
}

void ResAnimationCfg::addResAnimation(const char* resName)
{
#if defined(CLIENT)
#else
    map<string, ResAnimationCfgDef>::iterator iter = cfg.find(resName);
    if (iter != cfg.end())
    {
        return;
    }
    
    string xmlname = "skeleton/";
    xmlname.append(resName);
    xmlname.append(".xml");
    
    Xml::TiXmlDocument animationXml;
    animationXml.LoadFile(Cfgable::getFullFilePath(xmlname).c_str());
    Xml::TiXmlElement* rootElenmet = animationXml.RootElement();
    if (rootElenmet == NULL) {
        //printf("load res animation: %s fail\n", resName);
        return;
    }
    
    static const char *ANIMATIONS = "animations";
    static const char *ANIMATION = "animation";
    static const char *MOVEMENT = "mov";
    static const char *A_NAME = "name";
    static const char *A_DURATION_TWEEN = "drTW";
    
    Xml::TiXmlDocument* _document = &animationXml;
    Xml::TiXmlElement	*_root = _document->RootElement();
    Xml::TiXmlElement *_animationsXML = _root->FirstChildElement(ANIMATIONS);
    Xml::TiXmlElement *_animationXML = _animationsXML->FirstChildElement(ANIMATION);
    
    ResAnimationCfgDef cfgDef;
    while (_animationXML)
    {
        Xml::TiXmlElement* _movement = _animationXML->FirstChildElement(MOVEMENT);
        
        while( _movement )
        {
            const char* _aniName;
            _aniName = _movement->Attribute(A_NAME);
            
            int _durationTween = 0;
            
            if( _movement->QueryIntAttribute(A_DURATION_TWEEN, &(_durationTween)) == Xml::TIXML_SUCCESS)
            {
                //_durationTween;
                AnimationCfgDef animationCfg;
                animationCfg.mName = _aniName;
                animationCfg.mDuration = (float)_durationTween / fps;
                cfgDef.mAnimations.push_back(animationCfg);
            }
            
            _movement = _movement->NextSiblingElement(MOVEMENT);
            
        }
        _animationXML = _animationXML->NextSiblingElement(ANIMATION);
        
    }
    
    ResAnimationCfg::cfg[resName] = cfgDef;

#endif

}

/*
 -------------------------------------------------------
 */

//场景可能有多个背景乐，随机播放一个
int SceneCfgDef::getMusicID()
{
    if(musicIDs.size() == 1)
        return musicIDs[0];
    else if(musicIDs.size() > 1)
    {
        int index = rand() % musicIDs.size();
        return musicIDs[index];
    }
    else
        return 0;
}

void loadSceneTmx(const char* filename, std::vector<int>& monsters)
{

}

map<int, SceneCfgDef*> SceneCfg::cfg;
void SceneCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("sceneMultiLanguage.ini") == "sceneMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("sceneMultiLanguage.ini"));
    }
    //GameInifile MLIniFile(Cfgable::getFullFilePath("sceneMultiLanguage.ini"));
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
        SceneCfgDef* def = new SceneCfgDef();
#if defined(CLIENT)
        def->setSceneID(inifile.getValueT(*iter,"sceneId",0));
        def->setSceneType(inifile.getValueT(*iter,"sceneType",0));
        def->setMinLevel(inifile.getValueT(*iter,"minLv",0));
        def->setMaxLevel(inifile.getValueT(*iter, "maxLv", 0));
#else
        def->sceneId = inifile.getValueT(*iter,"sceneId",0);
        def->sceneType = inifile.getValueT(*iter,"sceneType",0);
        def->minLv = inifile.getValueT(*iter,"minLv",0);
        def->maxLv = inifile.getValueT(*iter,"maxLv",0);
#endif
        string name_id = inifile.getValue(*iter,"name_id");
        //by lihengjin
        string str_name;
        if(inifile.exists(*iter, "name"))
        {
            str_name = inifile.getValue(*iter,"name");
        }else
        {
            str_name = MLIniFile.getValue(name_id, "desc");
        }

        def->name = str_name;//inifile.getValue(*iter,"name");
        
        def->preCopy = inifile.getValueT(*iter, "pre_copy", 0);
        def->sceneIndex = inifile.getValueT(*iter,"sceneIndex",0);
        def->bornX = inifile.getValueT(*iter,"bornX",0);
        def->bornY = inifile.getValueT(*iter,"bornY",0);
        def->fightpoint = inifile.getValueT(*iter, "fightpoint", 0);

        int _hapPortal = inifile.getValueT(*iter,"hasPortal",0);
        def->hasPortal = _hapPortal==1?true:false;

        def->townInfo = inifile.getValue(*iter,"townInfo");
        def->is_boss = inifile.getValueT(*iter,"is_boss",0);
        def->copy_gold = inifile.getValueT(*iter,"copy_gold",0);
        def->copy_exp = inifile.getValueT(*iter,"copy_exp",0);
        def->copy_battlepoint = inifile.getValueT(*iter,"copy_battlepoint",0);
        def->copy_protoss = inifile.getValueT(*iter,"copy_protoss",0);
        def->energy_consumption = inifile.getValueT(*iter,"energy_consumption",0);
        def->copy_petExp = inifile.getValueT(*iter, "pet_exp", 0);
        def->hang_time = inifile.getValueT(*iter,"hang_time",0);
        def->is_attack = inifile.getValueT(*iter,"is_attack",0);
        def->min_vip = inifile.getValueT(*iter,"min_vip",0);
        def->monsterHead = inifile.getValue(*iter, "copy_icon");
        def->fall_items = inifile.getValue(*iter, "fall_item");
        def->pet_exp = inifile.getValueT(*iter, "pet_exp", 0);
        def->friendaward = inifile.getValue(*iter, "friend_award", "");
        def->wipeOutLvl = inifile.getValueT(*iter, "wipeout_lvl", 0);
        def->fightTime = inifile.getValueT(*iter, "fight_time", 0);
        
#ifdef CLIENT
        do {
            if (!def->fall_items.empty()) {
                break;
            }
            
            int starNum = 5;
            vector<flopcard*> cards;
            if (def->getSceneType() == stEliteCopy || def->getSceneType() == stPetAdventure) {
                for (int i = 0; i < starNum; i++) {
                    int flopId = def->getSceneID() * 10 + i + 1;
                    cards.push_back(SflopcardMgr.Find(flopId));
                }
            } else {
                int flopId = def->getSceneID();
                cards.push_back(SflopcardMgr.Find(flopId));
            }
            
            set<string> flopcardItemIds;
            for (int i = 0; i < cards.size(); i++) {
                if (cards[i] == NULL) {
                    continue;
                }
                flopcardItemIds.insert(cards[i]->allItemIds.begin(), cards[i]->allItemIds.end());
            }
            
            string fall_items;
            for (set<string>::iterator iter = flopcardItemIds.begin(); iter != flopcardItemIds.end();
                 iter++) {
                fall_items.append(*iter);
                fall_items.append(";");
            }
            def->fall_items = fall_items;
            
        } while (0);
#endif
        
        string str = inifile.getValue(*iter, "pet_adventure_type", "");
        def->petAdventureType = getPetAdventureType(str);

        if (!def->townInfo.empty()) {
            StringTokenizer reToken(def->townInfo,";*");
            def->townid = Utils::safe_atoi(reToken[0].c_str());
        }
        string musics = inifile.getValue(*iter,"music","");
        StringTokenizer stMusic(musics,",");
        for (int i=0; i<stMusic.count(); i++)
        {
            def->musicIDs.push_back(atoi(stMusic[i].c_str()));
        }
        string passtime = inifile.getValue(*iter, "copy_passtime","");
        StringTokenizer stPasstime(passtime,",");
        for(int i=0;i<stPasstime.count();i++)
        {
            def->copy_passtime.push_back(atoi(stPasstime[i].c_str()));
        }

        string maxhits = inifile.getValue(*iter, "copy_maxhits","");
        StringTokenizer stMaxhits(maxhits,",");
        for(int i=0;i<stMaxhits.count();i++)
        {
            def->copy_maxhits.push_back(atoi(stMaxhits[i].c_str()));
        }
        string hitby = inifile.getValue(*iter, "copy_hitby","");
        StringTokenizer stHitby(hitby,",");
        for(int i=0;i<stHitby.count();i++)
        {
            def->copy_hitby.push_back(atoi(stHitby[i].c_str()));
        }

        //副本重设消耗金钱
        string resetCost = inifile.getValue(*iter, "resetcost", "");
        StringTokenizer stResetCost(resetCost, ";");
        for (int i = 0; i < stResetCost.count(); i++) {
            def->resetCost.push_back(Utils::safe_atoi(stResetCost[i].c_str()));
        }

#if defined(CLIENT)
#else
        
        do
        {
            vector<Point> bornPoints;
            def->mMonsters.clear();

            string xmlname = "tmx/" + inifile.getValue(*iter,"sceneId","");
            xmlname += ".tmx";
            Xml::TiXmlDocument sceneXml;
            sceneXml.LoadFile(Cfgable::getFullFilePath(xmlname).c_str());

            Xml::TiXmlElement* rootElenmet = sceneXml.RootElement();

            if (rootElenmet == NULL) {
                break;
            }
            
            int row, tileHeight;
            rootElenmet->Attribute("height", &row);
            rootElenmet->Attribute("tileheight", &tileHeight);
            int mapHeight = row * tileHeight;

            Xml::TiXmlElement* objectgroup = rootElenmet->FirstChildElement();

            while (objectgroup) {
                Xml::TiXmlAttribute* groupattribute = objectgroup->FirstAttribute();
                
                //加载所有的area
                if (strcmp(groupattribute->Value(), "area") == 0) {
                    Xml::TiXmlElement* object = objectgroup->FirstChildElement();
                    
                    while (object) {
                        SceneAreaDef areaDef;
                        
                        int prevX, prevY;
                        object->Attribute("x", &prevX);
                        object->Attribute("y", &prevY);
                        object->Attribute("width", &areaDef.mRect.width);
                        object->Attribute("height", &areaDef.mRect.height);
                        
                        areaDef.mRect.x = prevX;
                        areaDef.mRect.y = mapHeight - areaDef.mRect.height -  prevY;
                        
                        def->mAreas.push_back(areaDef);
                        
                        object = object->NextSiblingElement();
                    }
                }
                
                //加载所有的怪物
                if (strcmp(groupattribute->Value(), "monster") == 0) {
                    Xml::TiXmlElement* object = objectgroup->FirstChildElement();

                    while (object) {
                        Xml::TiXmlAttribute* objattribute = object->FirstAttribute();
                        int monsterTmpid = Utils::safe_atoi(objattribute->Value());
                        def->mMonsters.push_back(monsterTmpid);
                        
                        SceneNPCDef monster;
                        object->Attribute("x", &monster.x);
                        object->Attribute("y", &monster.y);
                        monster.y = mapHeight - monster.y;
                        monster.npcId = monsterTmpid;
                        for (int i = 0; i < def->mAreas.size(); i++) {
                            Point pt = {monster.x, monster.y};
                            if (isPointInRect(pt, def->mAreas[i].mRect)) {
                                def->mAreas[i].mMonsters.push_back(monster);
                                break;
                            }
                        }
                        
                        object = object->NextSiblingElement();
                    }
                }
                
                
                if (strcmp(groupattribute->Value(), "rolebornpoint") == 0)
                {
                    Xml::TiXmlElement* object = objectgroup->FirstChildElement();
                    //第一段出生点
                    Point bornPoint = {def->bornX, def->bornY};
                    def->mLandPoses.push_back(bornPoint);
                    //其他段为落地点
                    while (object) {
                        object->Attribute("x", &bornPoint.x);
                        object->Attribute("y", &bornPoint.y);
                        bornPoint.y = mapHeight -  bornPoint.y;
                        def->mLandPoses.push_back(bornPoint);
                        
                        object = object->NextSiblingElement();
                    }
                }
                
                if (strcmp(groupattribute->Value(), "AreaBornPoint") == 0) {
                    Xml::TiXmlElement* object = objectgroup->FirstChildElement();
                    //第一段出生点
                    Point bornPoint = {def->bornX, def->bornY};
                    bornPoints.push_back(bornPoint);
                    //其他段为落地点
                    while (object) {
                        object->Attribute("x", &bornPoint.x);
                        object->Attribute("y", &bornPoint.y);
                        bornPoint.y = mapHeight -  bornPoint.y;
                        bornPoints.push_back(bornPoint);
                        
                        object = object->NextSiblingElement();
                    }
                }
                
                objectgroup = objectgroup->NextSiblingElement();
            }
            
            for (int i = 0; i < bornPoints.size(); i++) {
                Point pt = bornPoints[i];
                for (int j = 0; j < def->mAreas.size(); j++) {
                    if (isPointInRect(pt, def->mAreas[j].mRect)) {
                        def->mAreas[j].mLandPoints.push_back(pt);
                        break;
                    }
                }
            }
            
        } while(0);
#endif
        def->incr_friendly = inifile.getValueT(*iter, "incr_friendly", 0);
		def->incr_prestige = inifile.getValueT(*iter, "incr_prestige", 0);

#if defined(CLIENT)
        cfg[def->getSceneID()] = def;
#else
        cfg[def->sceneId] = def;
#endif
    }
}
SceneCfgDef* SceneCfg::getCfg(int _id)
{
    //assert(cfg.find(_id) != cfg.end());
    if (cfg.find(_id) == cfg.end()) {
        return NULL;
    }
    return cfg[_id];
}

#if defined(CLIENT)
#else
int SceneCfg::getMonsterCount(SceneCfgDef* scene)
{
    if (scene == NULL) {
        return 0;
    }
    return scene->mMonsters.size();
}
bool SceneCfg::isMonsterExist(SceneCfgDef* scene, int monsterid)
{
    if (scene == NULL) {
        return false;
    }

    for (int i = 0 ; i < scene->mMonsters.size(); i++) {
        if (scene->mMonsters[i] == monsterid) {
            return true;
        }
    }

    return false;
}
#endif

int SceneCfg::findSceneForIndex(int _index)
{
    for (map<int, SceneCfgDef*>::iterator i = cfg.begin(); i != cfg.end(); i++) {
        if ((i->second)->sceneIndex == _index) {
            return i->first;
        }
    }
    return 0;
}

void SceneCfg::findSceneForType(int _sceneType, std::vector<int> &_sceneIDs)
{
    for (map<int, SceneCfgDef*>::iterator i = cfg.begin(); i != cfg.end(); i++)
    {
#if defined(CLIENT)
        if ((i->second)->getSceneType() == _sceneType) {
            _sceneIDs.push_back(i->first);
        }
#else
        if ((i->second)->sceneType == _sceneType) {
            _sceneIDs.push_back(i->first);
        }
#endif
    }
}

int SceneCfg::checkCopyOpen(int _copyID, int _lastCopyID)
{
    SceneCfgDef *_sceneCfg = SceneCfg::getCfg(_copyID);
    if (_sceneCfg != NULL && _sceneCfg->preCopy > _lastCopyID)
    {
        return checkCopyOpen(_sceneCfg->preCopy, _lastCopyID);
    }
    else
    {
        return _copyID;
    }
}

std::string SceneCfg::SceneToString(int _sceneID)
{
    char _sceneStr[255] = {0};
    SceneCfgDef *_sceneCfg = SceneCfg::getCfg(_sceneID);
    string _sceneType;
    int sceneType;
#if defined(CLIENT)
    sceneType = _sceneCfg->getSceneType();
#else
    sceneType = _sceneCfg->sceneType;
#endif
    switch (sceneType) {
        case stTown:
            _sceneType = "主城";
            break;
        case stCopy:
            _sceneType = "普通副本";
            break;
        case stPrintCopy:
            _sceneType = "图纸副本";
            break;
        case stEliteCopy:
            _sceneType = "精英副本";
            break;
        case stDungeon:
            _sceneType = "地下城";
            break;
        case stTeamCopy:
            _sceneType = "雇佣副本";
            break;
        case stPvp:
            _sceneType = "竞技场";
            break;
        case stCrossServiceWar:
            _sceneType = "跨服战";
            break;
        case stNewbie:
            _sceneType = "新手地图";
            break;
        case stFriendDunge:
            _sceneType = "好友地下城";
            break;
        case stPetEliteCopy:
            _sceneType = "幻兽试炼场";
            break;
        default:
            _sceneType = "其他";
            break;
    }
    sprintf(_sceneStr, "%d_%s_%s",_sceneID,_sceneType.c_str(),(_sceneCfg->name).c_str());
    return _sceneStr;
}
/*
 --------------------------------------------------------
 */
PortalCfgDef* PortalCfg::getCfg(int _id){
    assert( cfg.find(_id) != cfg.end() );
    return cfg[_id];
}
map<int, PortalCfgDef*> PortalCfg::cfg;
void PortalCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
        PortalCfgDef* def = new PortalCfgDef();
        def->portalID = inifile.getValueT(*iter,"portalID",0);
        def->res_id = inifile.getValueT(*iter,"res_id",0);
        def->isToCopy = inifile.getValueT(*iter,"isToCopy",0);
        def->sceneID = inifile.getValueT(*iter,"sceneID",0);
        def->targetX = inifile.getValueT(*iter,"targetX",0);
        def->targetY = inifile.getValueT(*iter,"targetY",0);
        def->isShow = inifile.getValueT(*iter, "isShow", 0);
        std::string _targetIDs = inifile.getValue(*iter,"targetID");
        StringTokenizer reToken(_targetIDs,";");
        for (int i = 0; i < reToken.count(); i++) {
            if (atoi(reToken[i].c_str()) == 0)
                continue;
            def->targetID.push_back(atoi(reToken[i].c_str()));
        }
        cfg[def->portalID] = def;
    }
}

void PortalCfg::findPortalsForSceneID(int _sceneID, vector<int> *_outPortals)
{
    for (map<int, PortalCfgDef*>::iterator i = cfg.begin(); i != cfg.end(); i++)
    {
        if ((i->second)->sceneID == _sceneID)
        {
            _outPortals->push_back(i->first);
        }
    }
}

int PortalCfg::findPortalForSceneID(int _sceneID, int _targetID)
{
    for (map<int, PortalCfgDef*>::iterator i = cfg.begin(); i != cfg.end(); i++)
    {
        if ((i->second)->sceneID != _sceneID)
            continue;
        vector<int> _targetIDs = (i->second)->targetID;
        for (vector<int>::iterator iter = _targetIDs.begin(); iter != _targetIDs.end(); iter++)
        {
            if (_targetID == *iter)
            {
                return i->first;
            }
        }
    }
    return 0;
}
/*
 --------------------------------------------------------
 */
map<int, NewbieCfgDef*> NewbieCfg::cfg;
void NewbieCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
        NewbieCfgDef* tmp = new NewbieCfgDef;
        tmp->id = inifile.getValueT(*iter,"id",0);
        tmp->newbieSceneID = inifile.getValueT(*iter,"newbieSceneID",0);
        tmp->level = inifile.getValueT(*iter,"level",0);
        tmp->dialogID = inifile.getValueT(*iter,"dialogID",0);
        tmp->hp = inifile.getValueT(*iter,"hp",0);
        tmp->atk = inifile.getValueT(*iter,"atk",0);
        tmp->def = inifile.getValueT(*iter,"def",0);
        tmp->hit = inifile.getValueT(*iter, "hit", 0.0);  //命中
        tmp->dodge = inifile.getValueT(*iter, "dodge",0.0 );//闪避
        tmp->cri = inifile.getValueT(*iter, "cri", 0.0);//暴击
        tmp->hpper = inifile.getValueT(*iter, "hpper", 0.0);//回血
        tmp->hpper_cd = inifile.getValueT(*iter, "hpper_cd", 0.0);//回血间隔
        string skillsStr = inifile.getValue(*iter,"skills");
        StringTokenizer token(skillsStr,";");
        for (int i = 0; i < token.count(); i++)
        {
            tmp->skills.push_back(atoi(token[i].c_str()));
        }
        //        tmp->skill1 = inifile.getValueT(*iter,"skill1",0);
        //        tmp->skill2 = inifile.getValueT(*iter,"skill2",0);
        //        tmp->skill3 = inifile.getValueT(*iter,"skill3",0);
        //        tmp->skill4 = inifile.getValueT(*iter,"skill4",0);
        cfg[tmp->id] = tmp;
    }
}

NewbieCfgDef* NewbieCfg::getNewbiePetCfg()
{
    for (map<int,NewbieCfgDef*>::iterator it = cfg.begin(); it != cfg.end(); it++)
    {
        if (it->first > 100)
        {
            return it->second;
        }
    }
    return NULL;
}

/*
 --------------------------------------------------------
 */
map<int, NpcCfgDef*> NpcCfg::cfg;
void NpcCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("npcMultiLanguage.ini") == "npcMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("npcMultiLanguage.ini"));
    }
    //GameInifile MLIniFile(Cfgable::getFullFilePath("npcMultiLanguage.ini"));
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
        NpcCfgDef* def = new NpcCfgDef();
        def->id = inifile.getValueT(*iter,"id",0);
        def->res_id = inifile.getValueT(*iter,"res_id",0);
        string name_id = inifile.getValue(*iter,"name_id");
        //by lihengjin
        string str_name;
        if(inifile.exists(*iter, "name"))
        {
            str_name = inifile.getValue(*iter,"name");
        }else
        {
            str_name = MLIniFile.getValue(name_id, "desc");
        }

        def->name = str_name;//inifile.getValue(*iter,"name");
        def->scene = inifile.getValueT(*iter,"scene",0);
        string conversation_id = inifile.getValue(*iter, "conversation_id");
        //by lihengjin
        string str_conversation;
        if(inifile.exists(*iter, "conversation"))
        {
            str_conversation = inifile.getValue(*iter, "conversation");
        }else
        {
            str_conversation = MLIniFile.getValue(conversation_id, "desc");
        }

        def->conversation = str_conversation;//inifile.getValue(*iter, "conversation");
        def->npc_half = inifile.getValue(*iter, "NPC_Half");
        def->npcType = inifile.getValueT(*iter, "NPC_type", 0);
        def->npcShowLvl = inifile.getValueT(*iter, "NPC_Show_Lvl", 0);
        cfg[def->id]=def;
    }
}
/*
 ----------------------------------------------------------
 */

MonsterCfgDef::MonsterCfgDef()
{
    setLvl(0);
    setWalkSpeed(0);
    setHp(0);
    setHit(0.0);
    setAtk(0);
    
    setDef(0);
    setDodge(0);
    
    setPerAnger(0);  //角色杀死这个怪物之后获取的怒气值
    setAtkProbality(0.0);
    setAtkPeriod(0.0);
    
    setAtkSpeed(0);
    setAtkRangeX(0.0);
    setAtkRangeY(0.0);
    setAtkRectOrigX(0.0);
    setAtkRectOrigY(0.0);
    
    
    setBeAtkRangeX(0.0);
    setBeAtkRangeY(0.0);
    
    setSkillRangeX(0.0);
    setSkillRangeY(0.0);
    
    setFollowProbality(0);
    
    setPatrolLength(0.0);
    setVision(0.0);
    setChangingRoundX(0.0);
    setChangingRoundY(0.0);
    setActionperiod(0.0);
    setHangAround(0);
    setDeath_duration(0.0);
    setGrowthType(0);
}
MonsterType MonsterCfg::stringToMonsterType( std::string str )
{
    return
    (str == "long_range") ? eLongRange :
    (str == "short_range") ? eShorRange :
    (str == "boss") ? eBoss :
    eTypeError;
}

MonsterCfgDef* MonsterCfg::getCfg(int _id)
{

    if( cfg.find(_id) == cfg.end() )
        return  NULL;
    return cfg[_id];
}
vector<string> MonsterCfg::getAllMonsterHeadIcon()
{
    vector<string > allHeadIcon;
    allHeadIcon.clear();
    map<int,MonsterCfgDef*>::iterator iter;
    for(iter = cfg.begin();iter != cfg.end();iter ++)
    {
        string headIcon = iter->second->monsterHead_img;
        allHeadIcon.push_back(headIcon);
    }
    return allHeadIcon;
}
map<int,MonsterCfgDef*> MonsterCfg::cfg;
void MonsterCfg::load(std::string propFilePath, std::string tpltFilePath)
{
    GameInifile inifile(propFilePath);
    GameInifile monsterTpltFile(tpltFilePath);
    //lihengjin
    GameInifile MLmonsterTpltFile;
    if(!(Cfgable::getFullFilePath("monster_tpltMultiLanguage.ini") == "monster_tpltMultiLanguage.ini"))
    {
        MLmonsterTpltFile = GameInifile(Cfgable::getFullFilePath("monster_tpltMultiLanguage.ini"));
    }
    //GameInifile MLmonsterTpltFile(Cfgable::getFullFilePath("monster_tpltMultiLanguage.ini"));

    cfg.clear();
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        MonsterCfgDef* def = new MonsterCfgDef();

        ////////////////////////基本属性////////////////////////////////////////////////////
        def->setLvl(atoi(inifile.getValue(*iter, "lvl","").c_str()));
        
        
        
        def->setWalkSpeed(inifile.getValueT(*iter,"walk_speed",0));
        def->setHp(inifile.getValueT(*iter,"hp",0));
        def->setHit(inifile.getValueT(*iter, "hit", 0.0f));
        def->setDef(inifile.getValueT(*iter,"def",0));
        def->setAtk(inifile.getValueT(*iter,"atk",0));
        def->setDodge(inifile.getValueT(*iter,"dodge",0));
        def->setPerAnger(inifile.getValueT(*iter,"per_anger",0));
        def->isArmor = inifile.getValueT(*iter, "is_armor", false);
        def->monsterType = (MonsterType)(inifile.getValueT(*iter,"monster_type", 0));
        
        //怪物模板ID
        std::string monsterTpltID = inifile.getValue(*iter, "tplt_id", "");
        
        def->setGrowthType(inifile.getValueT(*iter,"type", 0));
        
        
        if (! monsterTpltFile.exists(monsterTpltID))
        {
            log_error("monster_tplt.ini 没有："<<monsterTpltID);
//            assert(0);
            continue;
        }
        //////////////////////其余的模板属性/////////////////////////////////////////////////
        def->res_id = monsterTpltFile.getValueT(monsterTpltID,"res_id",0);
        string nameId = monsterTpltFile.getValue(monsterTpltID, "name_id", "");
        //by lihengjin
        string nameStr;
        if(monsterTpltFile.exists(monsterTpltID, "name"))
        {
            nameStr = monsterTpltFile.getValue(monsterTpltID, "name", "");
        }else
        {
            nameStr = MLmonsterTpltFile.getValue(nameId, "desc");
        }
        def->name = nameStr;//monsterTpltFile.getValue(monsterTpltID, "name", "");
        //攻击时间点
        string attackTimes      = monsterTpltFile.getValue(monsterTpltID,"attack_delay_times","");
        StringTokenizer stAT(attackTimes,",");
        for (int i=0; i<stAT.count(); i++)
        {
            def->attackDelayTimes.push_back(atof(stAT[i].c_str()));
        }

        def->setVision(monsterTpltFile.getValueT(monsterTpltID, "vision",0.0f));
        def->monster_img = monsterTpltFile.getValue(monsterTpltID, "monster_img","");
        def->monsterHead_img = monsterTpltFile.getValue(monsterTpltID, "monster_head","");
        def->monster_degree = monsterTpltFile.getValueT(monsterTpltID, "degree",0);

        //巡逻长度及乱逛的参数
        def->setPatrolLength(monsterTpltFile.getValueT(monsterTpltID, "patrol_length",0.0f));
        def->setFollowProbality(monsterTpltFile.getValueT(monsterTpltID, "follow_probality",0.0f));
        def->setChangingRoundX(monsterTpltFile.getValueT(monsterTpltID, "changing_round_x",0.0f));
        def->setChangingRoundY(monsterTpltFile.getValueT(monsterTpltID, "changing_round_y",0.0f));
        def->setHangAround(monsterTpltFile.getValueT(monsterTpltID, "HangAround", 0));

        //攻击属性
        def->setActionperiod(monsterTpltFile.getValueT(monsterTpltID, "actionperiod", 0.0f));
        def->setAtkSpeed(monsterTpltFile.getValueT(monsterTpltID,"atk_speed",0));
        def->setAtkPeriod(monsterTpltFile.getValueT(monsterTpltID, "atk_period",0.0f));
        def->setAtkProbality(monsterTpltFile.getValueT(monsterTpltID, "atk_probality",0.0f));

        string oringPoint = monsterTpltFile.getValue(monsterTpltID, "atk_rect_origin_point");
        StringTokenizer point(oringPoint, ",");

        if (point.count() == 2) {
            def->setAtkRectOrigX(atof(point[0].c_str()));
            def->setAtkRectOrigY(atof(point[1].c_str()));
        }else
        {
            def->setAtkRectOrigX(0.0f);
            def->setAtkRectOrigY(0.0f);
        }
        def->setAtkRangeX(monsterTpltFile.getValueT(monsterTpltID, "atk_range_x",0.0f));
        def->setAtkRangeY(monsterTpltFile.getValueT(monsterTpltID, "atk_range_y",0.0f));

        def->setBeAtkRangeX(monsterTpltFile.getValueT(monsterTpltID, "be_atk_range_x",0.0f));
        def->setBeAtkRangeY(monsterTpltFile.getValueT(monsterTpltID, "be_atk_range_y",0.0f));
        
        //死亡停留时间
        def->setDeath_duration(monsterTpltFile.getValueT(monsterTpltID, "Death_duration",0.0f));

        //技能相关
        string monsterSkills = monsterTpltFile.getValue(monsterTpltID, "skill","");

        StringTokenizer st(monsterSkills,";");

        for (int i=0; i<st.count(); i++)
        {
            StringTokenizer tmp(st[i],":");
            if (tmp.count() < 2)
            {
                break;
            }
            MonsterCfg::cfg[i]=def;
            MonsterSkills* skill = new MonsterSkills;
            skill->setSkillid(atoi(tmp[0].c_str()));
            //            skill->rectWidth = Utils::safe_atoi(tmp[1].c_str());
            //            skill->rectHigh = Utils::safe_atoi(tmp[2].c_str());
            skill->setRol(atoi(tmp[1].c_str()));
            def->skills.push_back(skill);
        }//获得技能列表
        def->castSkillRol = monsterTpltFile.getValueT(monsterTpltID,"cast_skill_rol",0);
        def->setSkillRangeX(monsterTpltFile.getValueT(monsterTpltID, "skill_range_x",0.0f));
        def->setSkillRangeY(monsterTpltFile.getValueT(monsterTpltID, "skill_range_y",0.0f));

        //声音
        string attackAudios      = monsterTpltFile.getValue(monsterTpltID,"attack_audios","");
        StringTokenizer stAA(attackAudios,",");
        for (int i=0; i<stAA.count(); i++)
        {
            def->mAttackAudios.push_back(atoi(stAA[i].c_str()));
        }

        string hittedAudios      = monsterTpltFile.getValue(monsterTpltID,"hitted_audios","");
        StringTokenizer stHA(hittedAudios,",");
        for (int i=0; i<stHA.count(); i++)
        {
            def->mHittedAudios.push_back(atoi(stHA[i].c_str()));
        }

        string attackScreamAudios      = monsterTpltFile.getValue(monsterTpltID,"attack_scream_audios","");
        StringTokenizer stASA(attackScreamAudios,",");
        for (int i=0; i<stASA.count(); i++)
        {
            def->mAttackScreams.push_back(atoi(stASA[i].c_str()));
        }

        string hittedScreamAudios      = monsterTpltFile.getValue(monsterTpltID,"hitted_scream_audios","");
        StringTokenizer stHSA(hittedScreamAudios,",");
        for (int i=0; i<stHSA.count(); i++)
        {
            def->mHittedScreams.push_back(atoi(stHSA[i].c_str()));
        }

        def->mDeadAudio = monsterTpltFile.getValueT(monsterTpltID,"dead_audio",0);
        def->mKOScreamAudio = monsterTpltFile.getValueT(monsterTpltID,"ko_scream_audio",0);
        
        int idx = atoi((*iter).c_str());
        MonsterCfg::cfg[idx]=def;
    }
}
/*
 ----------------------------------------------------------
 */
//test printf skillCfg
void SkillCfgPrint()
{

}

SkillOwner StringToSkillOwner( std::string str)
{
    return

    (str == "warrior") ? eOwnerWarrior :
    (str == "assassin") ? eOwnerAssassin :
    (str == "mage") ? eOwnerMage
    :eOwnerMonster
    ;
}

SkillAffect StringToSkillAffect( std::string str )
{   
    return
    //攻击类
    (str == "skill_rush") ? eSkillRush :
    (str == "skill_static_effect") ? eSkillStaticEffect :
    (str == "skill_fly_once") ? eSkillFlyOnce :
    (str == "skill_fly_continue") ? eSkillFlyContinue :
    (str == "skill_buff_effect") ? eSkillBuffEffect :
    (str == "skill_new_body") ? eSkillNewBody :
    (str == "skill_movable_new_body_attack") ? eSkillMovableNewBodyAttack :
    (str == "lock_target_effect") ? eSkillLockTargetEffect :

    //纯表现类
    (str == "shake_screen") ? eShakeScreen :

    //伤害类
    (str == "hurt_hitted") ? eHurtHitted :
    (str == "hurt_knock_back") ? eHurtKnockBack :
    (str == "hurt_knock_down") ? eHurtKnockDown :
    (str == "hurt_dizzy") ? eHurtDizzy :
    (str == "hurt_fire_explosion") ? eHurtFireExplosion :
    (str == "hurt_poisoned") ? eHurtPoisoned :
    
    
    (str == "silence_effect") ? eSilenceEffect:  //沉默
    (str == "control_effect") ? eControlEffect:  //控制
    (str == "constraint_effect") ? eConstraintEffect:  //束缚
    
    //数值类
    (str == "buff_incr_hp") ? eBuffIncrHP :
    (str == "buff_suck_blood") ? eBuffSuckBlood :
    (str == "buff_change_attack") ? eBuffChangeAttack :
    (str == "buff_change_dogge") ? eBuffChangeDogge :
    (str == "buff_change_cir") ? eBuffChangeCri :
    (str == "buff_change_def") ? eBuffChangeDef :
    (str == "buff_change_hit") ? eBuffChangeHit :
    (str == "buff_change_speed") ? eBuffChangeSpeed :


    //被动技能
    (str == "incr_atk_forever") ? eIncrAtkForever :
    (str == "incr_def_forever") ? eIncrDefForever :
    (str == "incr_maxhp_forever") ? eIncrMaxHPForever :
    (str == "incr_hit_forever") ? eIncrHitForever :
    (str == "incr_dodge_forever") ? eIncrDodgeForever :
    (str == "incr_cri_forever") ? eIncrCriForever :
    (str == "incr_speed_forever") ? eIncrSpeedForever :
    (str == "incr_hpincr_forever") ? eIncrHpIncrForever :
    

    (str == "incr_stre_forever") ? eIncrStreForever :
    (str == "incr_inte_forever") ? eIncrInteForever :
    (str == "incr_phys_forever") ? eIncrPhysForever :
    (str == "incr_capa_forever") ? eIncrCapaForever :
    
    eNon;
}
SkillType stringToSkillType(std::string str)
{
    return
    (str == "passive") ? ePassive :
    (str == "initiative") ? eInitiative :
    (str == "anger") ? eAnger :
    (str == "trigger")? eTriggerSkill:
    eUnknowSkill;
}


SkillAffectTargetType StringToSkillTargetType( std::string str )
{
    return
    (str == "for_self") ? eForSelf :
    (str == "for_enemies") ? eForEnemies :
    (str == "for_self_group") ? eForSelfGroup :
    (str == "for_pet") ? eForPet :
    eForSelf;

}

map<int,SkillCfgDef*> SkillCfg::cfg;
GameInifile* SkillCfg::sSkillFile = NULL;
GameInifile* SkillCfg::sSkillEffectFile = NULL;
GameInifile* SkillCfg::mMLIniFile = NULL;
//一个技能允许的最多效果数
const int SkillCfg::sMaxSkillEffectNum = 5;

//从一串技能等级字符串中，取得指定等级的字符串。如果有多个等级的技能，策划可以只配一个值，用来表示所有等级数值一样。
#define set_skill_level_fields_string(strValue, strList, index) \
if ((index) < strList.count())                                  \
    strValue = strList[(index)];                                \
else if (strList.count()>0)                                     \
    strValue = strList[strList.count()-1];

//从一串技能等级字符串中，取得指定等级的数字。如果有多个等级的技能，策划可以只配一个值，用来表示所有等级数值一样。
#define set_skill_level_fields_number(strValue, strList, index) \
if ((index) < strList.count())                                  \
    strValue = strList[(index)];                                \
else if (strList.count()>0)                                     \
    strValue = strList[strList.count()-1];                      \
else                                                            \
    strValue = "0";


//从文件解析不随等级改变的技能字段---linshusen
void SkillCfg::parseUnchangeFields(SkillCfgDef* def, string idInFile_str, GameInifile* skillFile, GameInifile* skillEffectFile)
{
    //不随等级改变的变量
    def->setSkillID(skillFile->getValueT(idInFile_str, "id", 0));
    def->setMaxLvl(skillFile->getValueT(idInFile_str, "max_lvl", 0));
    string name_id = skillFile->getValue(idInFile_str, "name_id", "");
    // by lihengjin
    string str_name;
    if(!skillFile->exists(idInFile_str, "name_id"))
    {
        str_name = skillFile->getValue(idInFile_str, "name", "");
    }else
    {
        str_name = mMLIniFile->getValue(name_id, "desc");
    }

    def->skillname = str_name; //skillFile->getValue(idInFile_str, "name", "");
    def->skillIcon = skillFile->getValue(idInFile_str, "skill_icon", "");
    def->owner = StringToSkillOwner(skillFile->getValue(idInFile_str, "owner", ""));
    def->isBothSide = skillFile->getValueT(idInFile_str, "is_both_side", false);
    def->setAttackRectWidth(skillFile->getValueT(idInFile_str, "attack_rect_width", 0.0f));
    def->setAttackRectHeight(skillFile->getValueT(idInFile_str, "attack_rect_height", 0.0f));
    def->setAttackRectX(skillFile->getValueT(idInFile_str, "attack_rect_x", 0.0f));
    def->setAttackRectY(skillFile->getValueT(idInFile_str, "attack_rect_y", 0.0f));
    def->setCastRangeX(skillFile->getValueT(idInFile_str, "cast_range_x", 0));
    def->setCastRangeY(skillFile->getValueT(idInFile_str, "cast_range_y", 0));
    def->setSKillType(stringToSkillType(skillFile->getValue(idInFile_str, "skilltype", "")));
    def->setPetSkillType((skillFile->getValueT(idInFile_str, "is_commonskill", 0)));
    def->setIsControl((bool)(skillFile->getValueT(idInFile_str, "is_control", 0)));
    
    //读取不随等级改变的技能效果变量
    for (int i = 1; i <= sMaxSkillEffectNum; i++)
    {
        string affectFieldName = "affect_type";
        string indexStr = Utils::itoa(i);
        affectFieldName.append(indexStr);//效果编号
        
        string affectType = skillFile->getValue(idInFile_str,affectFieldName.c_str(),"");
        if (affectType.empty())//没找到效果号，就不会再有了，停止寻找
            break;
        
        SkillEffectVal* effect = new SkillEffectVal;
        //技能效果
        effect->affect = affectType;
        
        affectFieldName = "cast_time";
        affectFieldName.append(indexStr);
        effect->setCastTime(skillFile->getValueT(idInFile_str,affectFieldName.c_str(),0.0f));
        
        affectFieldName = "delay_time";
        affectFieldName.append(indexStr);
        effect->setDelayTime(skillFile->getValueT(idInFile_str,affectFieldName.c_str(),0.0f));
        
        affectFieldName = "target_type";
        affectFieldName.append(indexStr);
        effect->mTargetType = StringToSkillTargetType(skillFile->getValue(idInFile_str,affectFieldName.c_str(),""));
        
        if(skillEffectFile->exists(affectType))
        {
            effect->mEffectName = skillEffectFile->getValue(affectType, "name", "");
            
            effect->affectType =  StringToSkillAffect(skillEffectFile->getValue(affectType, "skill_effects_type", ""));
            
            effect->mIsFullScreen = skillEffectFile->getValueT(affectType, "is_full_screen", false);
            effect->mCharacterAnimation = skillEffectFile->getValue(affectType, "character_animation", "");
            effect->mCharacterAnimationLoop = skillEffectFile->getValueT(affectType, "character_animation_loop", false);
            effect->setEffectResId(skillEffectFile->getValueT(affectType, "effect_res_id", 0));
            effect->mIsBehindCharacter = skillEffectFile->getValueT(affectType, "is_behind_character", false);
            effect->mResPosType = (ResPosType)(skillEffectFile->getValueT(affectType, "effect_position", 1));
            effect->setAppearOffset(skillEffectFile->getValueT(affectType, "appear_offset", 0.0f));
            effect->setSpeed(skillEffectFile->getValueT(affectType, "speed", 0.0f));
            effect->setTargetNum(skillEffectFile->getValueT(affectType, "target_num", -1));
            effect->setMoveLength(skillEffectFile->getValueT(affectType, "move_length", 0.0f));
            effect->setAttackCount(skillEffectFile->getValueT(affectType, "attack_count", 0));
            effect->setAttackIntervalTime(skillEffectFile->getValueT(affectType, "attack_interval_time", 0.0f));
            effect->setAudioID(skillEffectFile->getValueT(affectType, "audio_id", 0));
            effect->setHitBackDistance(skillEffectFile->getValueT(affectType, "hit_back", 0));
            effect->setHitBackTime(skillEffectFile->getValueT(affectType, "hit_back_time", 0.0f));
        }
        else
        {
            effect->mEffectName = "";
            effect->mCharacterAnimation = "";
            effect->mCharacterAnimationLoop = 0;
        }
        def->effectList.push_back(effect);
    }

}
//从文件解析随等级改变的字段。必须确保SkillCfgDef* def先调用parseUnchangeFields()---linshusen
void SkillCfg::parseChangedFields(SkillCfgDef* def, string idInFile_str, int level, GameInifile* skillFile, GameInifile* skillEffectFile)
{
    def->setSkillID(def->getSkillID() + level);//设置真实的ID
    
    //技能描述参数
    StringTokenizer canshu1_s(skillFile->getValue(idInFile_str, "canshu1", ""), ";");
    StringTokenizer canshu2_s(skillFile->getValue(idInFile_str, "canshu2", ""), ";");
    StringTokenizer canshu3_s(skillFile->getValue(idInFile_str, "canshu3", ""), ";");
    StringTokenizer canshu4_s(skillFile->getValue(idInFile_str, "canshu4", ""), ";");
    StringTokenizer canshu5_s(skillFile->getValue(idInFile_str, "canshu5", ""), ";");
    string canshu1;
    set_skill_level_fields_number(canshu1, canshu1_s, level-1);
    string canshu2;
    set_skill_level_fields_number(canshu2, canshu2_s, level-1);
    string canshu3;
    set_skill_level_fields_number(canshu3, canshu3_s, level-1);
    string canshu4;
    set_skill_level_fields_number(canshu4, canshu4_s, level-1);
    string canshu5;
    set_skill_level_fields_number(canshu5, canshu5_s, level-1);
    
    string desc_id = skillFile->getValue(idInFile_str, "desc_id", "");
    //by lihengjin
    string str_desc;
    if(skillFile->exists(idInFile_str, "desc"))
    {
        str_desc = skillFile->getValue(idInFile_str, "desc", "");
    }else
    {
        str_desc = mMLIniFile->getValue(desc_id, "desc");
    }

    string descTile = str_desc; //skillFile->getValue(idInFile_str, "desc", "");
    //char descVal[256] = {0};
   // sprintf(descVal, descTile.c_str(), canshu1.c_str(), canshu2.c_str(), canshu3.c_str(),canshu4.c_str(),canshu5.c_str());
    
    string descVal = Utils::makeStr(descTile.c_str(), canshu1.c_str(), canshu2.c_str(), canshu3.c_str(),canshu4.c_str(),canshu5.c_str());
    def->desc = descVal;
    
    //一些随着等级改变的变量
    string strValue;
    
    StringTokenizer prev_id_s(skillFile->getValue(idInFile_str, "prev_id", ""), ";");
    set_skill_level_fields_number(strValue, prev_id_s, level-1);
    def->setPrevID(atoi(strValue.c_str()));
    
    StringTokenizer learn_lvl_s(skillFile->getValue(idInFile_str, "learn_lvl", ""), ";");
    set_skill_level_fields_number(strValue, learn_lvl_s, level-1);
    def->setLearnLvl(atoi(strValue.c_str()));
    
    StringTokenizer need_gold_s(skillFile->getValue(idInFile_str, "need_gold", ""), ";");
    set_skill_level_fields_number(strValue, need_gold_s, level-1);
    def->setNeedGold(atoi(strValue.c_str()));
    
    StringTokenizer need_battle_point_s(skillFile->getValue(idInFile_str, "need_battle_point", ""), ";");
    set_skill_level_fields_number(strValue, need_battle_point_s, level-1);
    def->setBattlePoint(atoi(strValue.c_str()));
    
    StringTokenizer needPages_s(skillFile->getValue(idInFile_str, "need_pages", ""), ";");
    set_skill_level_fields_number(strValue, needPages_s, level-1);
    def->setNeedPages(atoi(strValue.c_str()));
    
    StringTokenizer skill_cd_s(skillFile->getValue(idInFile_str, "skill_cd", ""), ";");
    set_skill_level_fields_number(strValue, skill_cd_s, level-1);
    def->setSkillCD(atof(strValue.c_str()));
    
    StringTokenizer com_cd_s(skillFile->getValue(idInFile_str, "com_cd", ""), ";");
    set_skill_level_fields_number(strValue, com_cd_s, level-1);
    def->setComCD(atof(strValue.c_str()));
    
    StringTokenizer upFailAddProb_s(skillFile->getValue(idInFile_str, "upfail_add_prob", ""), ";");
    set_skill_level_fields_number(strValue, upFailAddProb_s, level-1);
    def->setUpFailAddProb(atof(strValue.c_str()));
    
    StringTokenizer topTotalProb_s(skillFile->getValue(idInFile_str, "top_total_prob", ""), ";");
    set_skill_level_fields_number(strValue, topTotalProb_s, level-1);
    def->setTopTotalProb(atof(strValue.c_str()));
    
    for (int i=0; i<def->effectList.size(); ++i)
    {
        SkillEffectVal* effect = def->effectList[i];
        string indexStr = Utils::itoa(i+1);
        string affectFieldName = "last_time";
        affectFieldName.append(indexStr);
        StringTokenizer last_time_s(skillFile->getValue(idInFile_str, affectFieldName, ""), ";");
        set_skill_level_fields_number(strValue, last_time_s, level-1);
        effect->setLastTime(atof(strValue.c_str()));
        
        affectFieldName = "affect_value";
        affectFieldName.append(indexStr);
        StringTokenizer affect_value_s(skillFile->getValue(idInFile_str, affectFieldName, ""), ";");
        set_skill_level_fields_number(strValue, affect_value_s, level-1);
        effect->setAffectValue(atoi(strValue.c_str()));
        
        affectFieldName = "affect_float";
        affectFieldName.append(indexStr);
        StringTokenizer affect_float_s(skillFile->getValue(idInFile_str, affectFieldName, ""), ";");
        set_skill_level_fields_number(strValue, affect_float_s, level-1);
        effect->setAffectFloat(atof(strValue.c_str()));
    }
}
SkillCfgDef* SkillCfg::loadOneSkillFromFile(int skillID)
{
    if(sSkillFile == NULL || sSkillEffectFile == NULL)
        return NULL;
    
    int idInFile = (skillID/100)*100;
    string idInFile_str = Utils::itoa(idInFile);
    
    if(! sSkillFile->exists(idInFile_str))//配置表是否存在这个技能
        return NULL;
    //检测是否大于最大等级
    int level = skillID - idInFile;
    int maxLevel = sSkillFile->getValueT(idInFile_str, "max_lvl", 0);
    //如果传进来的是技能的模板 返回NULL
    if(skillID % 100  == 0)
        return NULL;
    if(level > maxLevel)
    {
        printf("Error!!!skill level bigger than max level,skill id:%d\n",skillID);
        return NULL;
    }
    
    SkillCfgDef* def = new SkillCfgDef();
    parseUnchangeFields(def, idInFile_str, sSkillFile, sSkillEffectFile);
    parseChangedFields(def, idInFile_str, level, sSkillFile, sSkillEffectFile);
    return def;
}

void SkillCfg::clientLoad(std::string skillPath, std::string skillEffectsPath)
{
    sSkillFile = new GameInifile(skillPath);
    sSkillEffectFile = new GameInifile(skillEffectsPath);
    
    loadOneSkillFromFile(400104);
}

void SkillCfg::load(std::string skillPath, std::string skillEffectsPath)
{
    //lihengjin
    if(!(Cfgable::getFullFilePath("skillMultiLanguage.ini") == "skillMultiLanguage.ini"))
    {
        mMLIniFile = new GameInifile(Cfgable::getFullFilePath("skillMultiLanguage.ini"));
    }
    //mMLIniFile = new GameInifile(Cfgable::getFullFilePath("skillMultiLanguage.ini"));
#if defined(CLIENT)
    clientLoad(skillPath,skillEffectsPath);
    return;
#endif
    GameInifile inifile(skillPath);
    GameInifile iniEffectFile(skillEffectsPath);

    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
    {
        if(inifile.exists(*iter))
        {
            int MaxLvl = atoi(inifile.getValue(*iter, "max_lvl", "").c_str());
            for (int level=1; level<=MaxLvl; ++level)
            {
                SkillCfgDef* def = new SkillCfgDef();
                parseUnchangeFields(def, *iter, &inifile, &iniEffectFile);
                parseChangedFields(def, *iter, level, &inifile, &iniEffectFile);
                SkillCfg::cfg[def->getSkillID()]= def;
            }
        }
        
        //sprintf(idstr,"%d", i);
//        if(inifile.exists(*iter))
//        {
//            int MaxLvl = atoi(inifile.getValue(*iter, "max_lvl", "").c_str());
//            
//            string name = inifile.getValue(*iter, "name", "");
//            string skillIcon = inifile.getValue(*iter, "skill_icon");
//            string descTile = inifile.getValue(*iter, "desc", "");
//            string owner = inifile.getValue(*iter, "owner", "");
//            string skill_cd = inifile.getValue(*iter, "skill_cd", "");
//            string com_cd = inifile.getValue(*iter, "com_cd", "");
//            string is_both_side = inifile.getValue(*iter, "is_both_side", "");
//
//            string descStr1 = inifile.getValue(*iter, "canshu1", "");
//            string descStr2 = inifile.getValue(*iter, "canshu2", "");
//            string descStr3 = inifile.getValue(*iter, "canshu3", "");
//            string descStr4 = inifile.getValue(*iter, "canshu4", "");
//            string descStr5 = inifile.getValue(*iter, "canshu5", "");
//
//            string prev_id = inifile.getValue(*iter, "prev_id", "");
//            string learn_lvl = inifile.getValue(*iter, "learn_lvl", "");
//            string need_gold = inifile.getValue(*iter, "need_gold", "");
//            string need_battle_point = inifile.getValue(*iter, "need_battle_point", "");
//
//            string attack_rect_width = inifile.getValue(*iter, "attack_rect_width", "");
//            string attack_rect_height = inifile.getValue(*iter, "attack_rect_height", "");
//            string attack_rect_x = inifile.getValue(*iter, "attack_rect_x", "");
//            string attack_rect_y = inifile.getValue(*iter, "attack_rect_y", "");
//
//            string castRangeX = inifile.getValue(*iter, "cast_range_x","");
//            string castRangeY = inifile.getValue(*iter, "cast_range_y", "");
//            string skillType = inifile.getValue(*iter, "skilltype", "");
//            
//            int pet_skill_type = inifile.getValueT(*iter, "is_commonskill", 0);
//           // vector<int> need_pages = inifile.getTableT(*iter, "need_pages", 0);
//            
//            string need_pages  = inifile.getValue(*iter, "need_pages", "");
//
//            StringTokenizer desc1(descStr1, ";");
//            StringTokenizer desc2(descStr2, ";");
//            StringTokenizer desc3(descStr3, ";");
//            StringTokenizer desc4(descStr4, ";");
//            StringTokenizer desc5(descStr5, ";");
//
//            StringTokenizer prevId(prev_id,";");
//            StringTokenizer learnLvl(learn_lvl,";");
//            StringTokenizer needGold(need_gold,";");
//            StringTokenizer needBattlePnt(need_battle_point,";");
//            StringTokenizer needPage(need_pages,";");
//
//            StringTokenizer skillCd(skill_cd, ";");
//            StringTokenizer comCd(com_cd, ";");
//
//            for (int i = 0; i < MaxLvl; i++)
//            {
//                int skillId = atoi((*iter).c_str()) + i + 1;
//                SkillCfgDef* def = new SkillCfgDef();
//
//                char descVal[128] = {0};
//                string canshu1 = "";
//                string canshu2 = "";
//                string canshu3 = "";
//                string canshu4 = "";
//                string canshu5 = "";
//                
//                if (i<desc1.count())
//                    canshu1 = desc1[i];
//                else if (desc1.count()>0)
//                    canshu1 = desc1[desc1.count()-1];
//
//                if (i<desc2.count())
//                    canshu2 = desc2[i];
//                else if (desc2.count()>0)
//                    canshu2 = desc2[desc2.count()-1];
//
//                if (i<desc3.count())
//                    canshu3 = desc3[i];
//                else if (desc3.count()>0)
//                    canshu3 = desc3[desc3.count()-1];
//
//                if (i<desc4.count())
//                    canshu4 = desc4[i];
//                else if (desc4.count()>0)
//                    canshu4 = desc4[desc4.count()-1];
//
//                if (i<desc5.count())
//                    canshu5 = desc5[i];
//                else if (desc5.count()>0)
//                    canshu5 = desc5[desc5.count()-1];
//
//                sprintf(descVal, descTile.c_str(), canshu1.c_str(), canshu2.c_str(), canshu3.c_str(),canshu4.c_str(),canshu5.c_str());
//                def->setMaxLvl(MaxLvl);
//                def->skillname = name;
//                def->skillIcon = skillIcon;
//                def->desc = descVal;
//                def->owner = StringToSkillOwner(owner);
//                def->isBothSide = atoi(is_both_side.c_str());
//
//                def->setAttackRectWidth( atof(attack_rect_width.c_str()));
//                def->setAttackRectHeight( atof(attack_rect_height.c_str()));
//                def->setAttackRectX(atof(attack_rect_x.c_str()));
//                def->setAttackRectY(atof(attack_rect_y.c_str()));
//
//                def->setCastRangeX(Utils::safe_atoi(castRangeX.c_str()));
//                def->setCastRangeY(Utils::safe_atoi(castRangeY.c_str()));
//                def->setSKillType(stringToSkillType(skillType));
//                
//                def->setPetSkillType( pet_skill_type );
//                
//                if( i < need_pages.size() )
//                {
//                    def->setNeedPages( need_pages[i] );
//                }
//                
//                
//                string prevStr = "";
//                if (i < prevId.count())
//                    prevStr = prevId[i];
//                else if (prevId.count()>0)
//                    prevStr = prevId[prevId.count()-1];
//                else
//                    prevStr = "0";
//                def->setPrevID(atoi(prevStr.c_str()));
//
//                string learnlvl = "";
//                if (i < learnLvl.count())
//                    learnlvl = learnLvl[i];
//                else if (learnLvl.count()>0)
//                    learnlvl = learnLvl[learnLvl.count()-1];
//                else
//                    learnlvl = "0";
//                def->setLearnLvl(atoi(learnlvl.c_str()));
//
//                string needGdStr = "";
//                if (i < needGold.count())
//                    needGdStr = needGold[i];
//                else if (needGold.count()>0)
//                    needGdStr = needGold[needGold.count()-1];
//                else
//                    needGdStr = "0";
//                def->setNeedGold(atoi(needGdStr.c_str()));
//
//                string needBtlP = "";
//                if (i < needBattlePnt.count())
//                    needBtlP = needBattlePnt[i];
//                else if (needBattlePnt.count()>0)
//                    needBtlP = needBattlePnt[needBattlePnt.count()-1];
//                else
//                    needBtlP = "0";
//                def->setBattlePoint(atoi(needBtlP.c_str()));
//                
//                string pages = "";
//                if(i < needPage.count())
//                    pages = needPage[i];
//                else if (needPage.count()>0)
//                    pages = needPage[needPage.count()-1];
//                else
//                    pages = "0";
//                def->setNeedPages(atoi(pages.c_str()));
//                
//                string cdStr = "";
//                if (i < skillCd.count())
//                    cdStr = skillCd[i];
//                else if (skillCd.count()>0)
//                    cdStr = skillCd[skillCd.count()-1];
//                else
//                    cdStr = "0";
//                def->setSkillCD(atof(cdStr.c_str()));
//
//                string comCdStr = "";
//                if (i < comCd.count())
//                    comCdStr = comCd[i];
//                else if (comCd.count()>0)
//                    comCdStr = comCd[comCd.count()-1];
//                else
//                    comCdStr = "0";
//                def->setComCD(atof(comCdStr.c_str()));
//
//                for (int ii = 0; ii < sMaxSkillEffectNum; ii++)
//                {
//                    char temp[128] = {0};
//                    string affectType = "";
//                    sprintf(temp, "affect_type%d", ii+1);
//                    affectType = temp;
//                    std::string affectName = inifile.getValue(*iter,affectType.c_str(),"");
//                    if (affectName == "") {
//                        break;
//                    }
//                    SkillEffectVal* value = new SkillEffectVal;
//                    //技能效果
//                    value->affect = affectName;
//
//                    //效果值
//                    sprintf(temp, "affect_value%d",ii+1);
//                    string affectValue = inifile.getValue(*iter,temp, "");
//                    StringTokenizer affectVal(affectValue, ";");
//                    if (i < affectVal.count())
//                        value->setAffectValue(atoi(affectVal[i].c_str()));
//                    else if (affectVal.count()>0)
//                        value->setAffectValue(atoi(affectVal[affectVal.count()-1].c_str()));
//                    else
//                        value->setAffectValue(0);
//
//                    sprintf(temp, "affect_float%d",ii+1);
//                    string affectFloat = inifile.getValue(*iter,temp, "");
//                    StringTokenizer affectFlt(affectFloat, ";");
//                    if (i < affectFlt.count())
//                        value->setAffectFloat(atof(affectFlt[i].c_str()));
//                    else if (affectFlt.count()>0)
//                        value->setAffectFloat(atof(affectFlt[affectFlt.count()-1].c_str()));
//                    else
//                        value->setAffectFloat(0);
//
//                    sprintf(temp, "cast_time%d",ii+1);
//                    string castTime = inifile.getValue(*iter,temp, "");
//                    value->setCastTime(atof(castTime.c_str()));
//                    //                    StringTokenizer castTm(castTime, ";");
//                    //                    if (i < castTm.count()) {
//                    //                        value->castTime = atof(castTm[i].c_str());
//                    //                    }
//
//                    sprintf(temp, "delay_time%d",ii+1);
//                    string delayTime = inifile.getValue(*iter,temp, "");
//                    value->setDelayTime(atof(delayTime.c_str()));
//
//                    sprintf(temp, "last_time%d",ii+1);
//                    string lastTime = inifile.getValue(*iter,temp, "");
//                    StringTokenizer lastTm(lastTime, ";");
//                    if (i < lastTm.count())
//                        value->setLastTime(atof(lastTm[i].c_str()));
//                    else if (lastTm.count()>0)
//                        value->setLastTime(atof(lastTm[lastTm.count()-1].c_str()));
//                    else
//                        value->setLastTime(0);
//
////                    sprintf(temp, "is_self%d", ii+1);
////                    string isSelf = inifile.getValue(*iter,temp, "");
////                    value->isForSelf = atoi(isSelf.c_str());
//                    
//                    sprintf(temp, "target_type%d", ii+1);
//                    string targetStr = inifile.getValue(*iter,temp, "");
//                    value->mTargetType = StringToSkillTargetType(targetStr);
//
//                    if(iniEffectFile.exists(affectName))
//                    {
//                        value->mEffectName = iniEffectFile.getValue(affectName, "name", "");
//
//                        value->affectType =  StringToSkillAffect(iniEffectFile.getValue(affectName, "skill_effects_type", ""));
//
//                        value->mIsFullScreen = iniEffectFile.getValueT(affectName, "is_full_screen", false);
//                        value->mCharacterAnimation = iniEffectFile.getValue(affectName, "character_animation", "");
//                        value->mCharacterAnimationLoop = iniEffectFile.getValueT(affectName, "character_animation_loop", false);
//                        value->setEffectResId(iniEffectFile.getValueT(affectName, "effect_res_id", 0));
//                        value->mIsBehindCharacter = iniEffectFile.getValueT(affectName, "is_behind_character", false);
//                        value->mResPosType = (ResPosType)(iniEffectFile.getValueT(affectName, "effect_position", 1));
//                        value->setAppearOffset(iniEffectFile.getValueT(affectName, "appear_offset", 0.0f));
//                        value->setSpeed(iniEffectFile.getValueT(affectName, "speed", 0.0f));
//                        value->setTargetNum(iniEffectFile.getValueT(affectName, "target_num", -1));
//                        value->setMoveLength(iniEffectFile.getValueT(affectName, "move_length", 0.0f));
//                        value->setAttackCount(iniEffectFile.getValueT(affectName, "attack_count", 0));
//                        value->setAttackIntervalTime(iniEffectFile.getValueT(affectName, "attack_interval_time", 0.0f));
//                        value->setAudioID(iniEffectFile.getValueT(affectName, "audio_id", 0));
//                        value->setHitBackDistance(iniEffectFile.getValueT(affectName, "hit_back", 0));
//                        value->setHitBackTime(iniEffectFile.getValueT(affectName, "hit_back_time", 0.0f));
//                    }
//                    else
//                    {
//                        value->mEffectName = "";
//                        value->mCharacterAnimation = "";
//                        value->mCharacterAnimationLoop = 0;
//
//                    }
//                    def->effectList.push_back(value);
//                }
//                SkillCfg::cfg[skillId]= def;
//            }
//        }
    }
}

vector<int>  SkillCfg::roleTotal(SkillOwner owner){
    //SkillOwner owner = obj->getObjType();
    //QCheck(owner != eOwnerMonster);
    vector<int> ret;
    map<int, SkillCfgDef*>::iterator it=cfg.begin();
    for (; it!= cfg.end(); it++) {
        if (it->second==NULL)
            continue;
        if((it->second->owner | owner) == owner)
            //if (it->second.mOwner != eOwnerMonster)
            ret.push_back(it->first);
    }
    return ret;
}

SkillCfgDef* SkillCfg::getCfg(int _id)
{
    if( cfg.find(_id) == cfg.end() )
    {
#if defined(CLIENT)
        //客户端的技能配置表，用到时才去解析---linshusen
        SkillCfgDef* def = loadOneSkillFromFile(_id);
        if(def != NULL)
            cfg[_id] = def;
        
        return def;
#else
        return NULL;
#endif
    }
    else
    {
        return cfg[_id];
    }
}



bool SkillCfg::getCfgByRoleJob(int Job ,vector< int >& outData)
{
//    int job = role->getJob();
//    //int lvl = role->getLvl();
//    int skillid = 400000 + job*10000 + 100*index + 0;
//    return skillid;
    int skillid = 400000 + Job*10000;
    map<int,SkillCfgDef*>::iterator beg_cfg = cfg.begin();
    map<int,SkillCfgDef*>::iterator end_cfg = cfg.end();
    
    std::set<int> set_data;
    for (; beg_cfg!=end_cfg; ++beg_cfg) {
        if (beg_cfg->first /10000 == skillid / 10000 )
        {
            set_data.insert((beg_cfg->second->getSkillID()-400000)/100%100);
        }
    }
    unique_copy(set_data.begin(),set_data.end(),back_inserter(outData));
    return true;
    
}

SkillCfgDef* SkillCfg::getCfg(int _id,bool& exist )
{
    if( cfg.find(_id) == cfg.end() )
    {
#if defined(CLIENT)
        //客户端的技能配置表，用到时才去解析---linshusen
        SkillCfgDef* def = loadOneSkillFromFile(_id);
        if(def != NULL)
            cfg[_id] = def;
        
        exist = def != NULL;
        return def;
#else
        exist=false;
        return NULL;
#endif
        
    }
    else
    {
        exist = true;
        return cfg[_id];
    }
}

//bool SkillCfg::GetEffectFuncByTargetType(int skillId, bool isForSelf, vector<SkillEffectVal*> &effects)
//{
//    effects.clear();
//    bool exist = false;
//    SkillCfgDef* cfg = SkillCfg::getCfg(skillId, exist);
//    if (!exist) {
//        return false;
//    }
//    effects.clear();
//    for (int i = 0; i < cfg->effectList.size(); i++)
//    {
//        SkillEffectVal* effectVal = cfg->effectList[i];
//        if (effectVal->isForSelf == isForSelf && effectVal->affectType != eNon)
//        {
//            effects.push_back(effectVal);
//        }
//    }
//    return true;
//}

vector<SkillEffectVal*> SkillCfg::GetEffectFuncByTargetType(int skillId, int targetType)
{
    vector<SkillEffectVal*> result;
    SkillCfgDef* cfg = SkillCfg::getCfg(skillId);
    if(cfg != NULL)
    {
        for (int i = 0; i < cfg->effectList.size(); i++)
        {
            SkillEffectVal* effectVal = cfg->effectList[i];
            if (effectVal->mTargetType == (effectVal->mTargetType & targetType) && effectVal->affectType != eNon)
            {
                result.push_back(effectVal);
            }
        }
    }
    
    return result;
}


//技能对基础属性加成
#define SKILL_INCR_BASE( name )\
{\
    out.baseProp.set##name( out.baseProp.get##name() + effects[i]->getAffectValue() );\
    out.basePropFactor.set##name##Factor(out.basePropFactor.get##name##Factor() + effects[i]->getAffectFloat());\
    break;\
}

//技能对战斗属性加成
#define SKILL_INCR_BATTLE( name )\
{\
    out.battleProp.set##name( out.battleProp.get##name() + effects[i]->getAffectValue() );\
    out.battlePropFactor.set##name##Factor(out.battlePropFactor.get##name##Factor() + effects[i]->getAffectFloat());\
    break;\
}

//命中、闪避、暴击是float型的，getAffectValue()返回一个整型，需要除以100变成浮点型再进行加法加成
#define SKILL_INCR_BATTLE_FLOAT( name ) \
{\
    out.battleProp.set##name( out.battleProp.get##name() + effects[i]->getAffectValue()/100.0f);\
    out.battlePropFactor.set##name##Factor(out.battlePropFactor.get##name##Factor() + effects[i]->getAffectFloat());\
    break;\
}


void SkillCfg::calcBattlePropFromSkill(int skillid, SkillAffectTargetType targettype,SkillIncrBattle& out)
{
    vector<SkillEffectVal*> effects = GetEffectFuncByTargetType(skillid, targettype);
    
    if ( effects.empty()) {
        return ;
    }
    
    for (int i = 0; i<effects.size(); i ++)
    {
        switch (effects[i]->affectType)
        {
            case eIncrAtkForever: SKILL_INCR_BATTLE( Atk)
            case eIncrDefForever: SKILL_INCR_BATTLE( Def )
            case eIncrMaxHPForever: SKILL_INCR_BATTLE( MaxHp )
            case eIncrSpeedForever: SKILL_INCR_BATTLE( MoveSpeed )
            case eIncrHpIncrForever: SKILL_INCR_BATTLE( HpIncr )
                
            case eIncrHitForever: SKILL_INCR_BATTLE_FLOAT( Hit )
            case eIncrDodgeForever: SKILL_INCR_BATTLE_FLOAT( Dodge )
            case eIncrCriForever: SKILL_INCR_BATTLE_FLOAT( Cri )
                
            case eIncrStreForever: SKILL_INCR_BASE( Stre )
            case eIncrInteForever: SKILL_INCR_BASE( Inte )
            case eIncrPhysForever: SKILL_INCR_BASE( Phys )
            case eIncrCapaForever: SKILL_INCR_BASE( Capa )

            default:
                break;
        }
    }
    return;
}

string ItemCfgDef::getFullName()
{
    //char _str[255] = {0};
    int _id = ReadInt("id");
    string _name = ReadStr("name");
   // sprintf(_str, "%d_%s",_id,_name.c_str());
    string _str = Utils::makeStr( "%d_%s",_id,_name.c_str());
    return _str;
}

int ItemCfgDef::ReadInt(const char* propname, int def, bool *suc)
{
    string strpropname = propname;
    map<string, string>::iterator iter = props.find(strpropname);
    if (iter == props.end())
    {
        if (suc)
        {
            *suc = false;
        }
        return def;
    }
    if (suc)
    {
        *suc = true;
    }

    return Utils::safe_atoi(iter->second.c_str());
}


float ItemCfgDef::ReadFloat(const char* propname, float def, bool *suc)
{
    map<string, string>::iterator iter = props.find(propname);
    if (iter == props.end())
    {
        if (suc)
        {
            *suc = false;
        }
        return def;
    }
    if (suc)
    {
        *suc = true;
    }
    return atof(iter->second.c_str());

}


string ItemCfgDef::ReadStr(const char* propname, string def, bool *suc)
{
    string propStr = propname;
    if(propStr == "name" || propStr == "desc" || propStr == "job_name" || propStr == "tarname" || propStr == "reminder"){
        
        //lihengjin
        //现在原始表中查找是否有propname
        map<string, string>::iterator iter = props.find(propname);
        if (iter != props.end())
        {
            //原始中有这个字段，直接用
            if (suc)
            {
                *suc = true;
            }
            string name = iter->second;
            return name;
        }
        else//原始表中没有，从多语言表中找
        {
            string realPropStr = propStr + "_id";
            map<string, string>::iterator iter = props.find(realPropStr);
            if (iter == props.end())
            {
                if (suc)
                {
                    *suc = false;
                }
                return def;
            }
            if (suc)
            {
                *suc = true;
            }
            string nameID = iter->second;
            
            MultiLgItemCfgDef * multiLgDef = ItemCfg::getMulitLgCfg(getIniName());
            string nameStr = multiLgDef->getMultiLgDesc(atoi(nameID.c_str()));
            return nameStr;
        }
    }
    else
    {
        map<string, string>::iterator iter = props.find(propname);
        if (iter == props.end())
        {
            if (suc)
            {
                *suc = false;
            }
            return def;
        }
        if (suc)
        {
            *suc = true;
        }
        return iter->second;
    }
        
}

string MultiLgItemCfgDef::getMultiLgDesc(int multiLgId)
{
    map<int,string >::iterator iter = desc.find(multiLgId);
    if(iter == desc.end())
        return "";
    return iter->second;
}

map<int, ItemCfgDef*> ItemCfg::cfg;
vector<int> ItemCfg::shopPropsCfg;
map<string ,MultiLgItemCfgDef *> ItemCfg::multiLgCfg;
//map<int ,string > ItemCfg::iniNames;
void ItemCfg::load(std::string fullPath)
{
    //try
    {
        GameInifile inifile(fullPath);

        vector<string> itemtypes;
        inifile.getSections(inserter(itemtypes, itemtypes.begin()));

        for (int i = 0; i < itemtypes.size(); i++)
        {
            int minid = 0, maxid = 0;
            string idinterval = inifile.getValue(itemtypes[i], "idinterval");
            sscanf(idinterval.c_str(), "%d-%d", &minid, &maxid);
            
            //int type = inifile.getValueT(itemtypes[i],"type", 0);
           // iniNames.insert(make_pair(type,itemtypes[i]));

            try
            {
                GameInifile itemini(Cfgable::getFullFilePath(itemtypes[i] + ".ini"));
                for (int j = minid; j < maxid; j++)
                {
//                    char iddesc[32];
//                    sprintf(iddesc, "%d", j);
                    string iddesc = Utils::makeStr( "%d", j);
                    if (!itemini.exists(iddesc)) {
                        continue;
                    }

                    ItemCfgDef* itemcfg = new ItemCfgDef;
                    itemcfg->itemid = j;
                    itemcfg->qua = itemini.getValueT(iddesc, "qua", 0);
                    itemcfg->stack = itemini.getValueT(iddesc, "stack", 0);
                    itemcfg->setIniName(itemtypes[i]);
                    vector<string> keys;
                    itemini.getKeys(iddesc, inserter(keys, keys.begin()));
                    for (int k = 0; k < keys.size(); k++)
                    {
                        string value = itemini.getValue(iddesc, keys[k]);
                        itemcfg->props.insert(make_pair(keys[k], value));
                    }

                    cfg.insert(make_pair(j, itemcfg));
                    if (itemtypes[i] == "ticket") {  //存储货币道具id号
                        if (itemcfg->ReadInt("is_shopping")==1) {
                            shopPropsCfg.push_back(j);
                        }
                    }
                }
                //尝试读多语言的文本--lihengjin
                string fullPath = Cfgable::getFullFilePath(itemtypes[i] + "MultiLanguage.ini");
                if(fullPath == (itemtypes[i] + "MultiLanguage.ini"))//没找到多国语言表
                {
                    continue;
                }
                else
                {
                    //lihengjin
                    GameInifile multiLgini;
                    std::string strMul =  itemtypes[i] + "MultiLanguage.ini";
                    if(!(Cfgable::getFullFilePath(strMul) == strMul))
                    {
                        multiLgini = GameInifile(Cfgable::getFullFilePath(strMul));
                    }
                    //GameInifile multiLgini(Cfgable::getFullFilePath(itemtypes[i] + "MultiLanguage.ini"));
                    
                    std::list<string> sections;
                    multiLgini.getSections(std::back_inserter(sections));
                    MultiLgItemCfgDef * multiLgDef = new MultiLgItemCfgDef;
                    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
                    {
                        int multiLgId = atoi(iter->c_str());
                        string desc = multiLgini.getValue(*iter, "desc", "");
                        multiLgDef->desc.insert(make_pair(multiLgId, desc));
                    }
                    multiLgCfg.insert(make_pair(itemtypes[i],multiLgDef));
                }

            }
            catch(...){}

        }
    }
    //catch(...)
    {

    }
}

vector<int> ItemCfg::getShopPropsVec(){
    return shopPropsCfg;
}

int ItemCfg::readInt(int itemId, const char* propname, int def)
{
    ItemCfgDef* itemcfg = getCfg(itemId);
    if (itemcfg)
    {
        return itemcfg->ReadInt(propname, def);
    }
    
    return def;
}


ItemCfgDef* ItemCfg::getCfg(int _id)
{
    map<int, ItemCfgDef*>::iterator iter = cfg.find(_id);
    if (cfg.end() != iter)
    {
        return (iter->second);
    }
    return NULL;
}

bool ItemCfg::IsEquip(int _id)
{
    ItemCfgDef* itemcfg = getCfg(_id);
    if (itemcfg == NULL)
    {
        return false;
    }
    int type = itemcfg->ReadInt("type");
    return type == kItemEquip;
}
vector<int> ItemCfg::getItemsByType(int type)
{
    vector<int> itemVec;
    map<int, ItemCfgDef*>::iterator iter = cfg.begin();
    for(; iter != cfg.end();iter++){
        ItemCfgDef * def = iter->second;
        if(def->ReadInt("type") == type)
        {
            itemVec.push_back(iter->first);
        }
    
    }
    return itemVec;
}


int ItemCfg::getSkillBooksByQua(int qua, bool isOrdinary,int skillBookType)
{
    map<int, ItemCfgDef*>::iterator iter = cfg.begin();
    for(; iter != cfg.end();iter++)
    {
        ItemCfgDef * def = iter->second;
        if(def->ReadInt("type") != kItemSkillBook)
            continue;
        if((bool)def->ReadInt("is_commonskill") != isOrdinary)
            continue;
        if(def->ReadInt("skill_book_type",0) == skillBookType)
        {
            if(isOrdinary == true)
            {
                return def->itemid;
            }
            else
            {
                if(def->ReadInt("qua") == qua)
                    return def->itemid;
            }

          
        }
    }
    return -1;
}
map<int, ItemCfgDef*> & ItemCfg::getAllItemDatas()
{
    return cfg;
}
int ItemCfg::getPotionID()
{
    map<int, ItemCfgDef*>::iterator iter = cfg.begin();
    for(; iter != cfg.end();iter++)
    {
        ItemCfgDef * def = iter->second;
        if(def->ReadInt("type") != kItemGrowthPill)
            continue;
        return def->itemid;
        
    }
    return -1;
}
MultiLgItemCfgDef * ItemCfg::getMulitLgCfg(string iniName)
{
    map<string ,MultiLgItemCfgDef *>::iterator iter = multiLgCfg.find(iniName);
    if(iter != multiLgCfg.end())
    {
        return iter->second;
    }
    return NULL;
}


int ItemCfg::getNewItemId(const int &lvl, const int &part, const int &qua, const int &job)
{
    for (map<int, ItemCfgDef*>::iterator iter = cfg.begin(); iter != cfg.end(); ++iter)
    {
        ItemCfgDef *def = iter->second;
        if (def->ReadInt("lvl") == lvl &&
            def->ReadInt("job") == job &&
            def->ReadInt("part") == part &&
            def->ReadInt("qua") == qua &&
            def->ReadInt("is_initial", 0) == 0
            )
        {
            return def->itemid;
        }
    }
    return 0;
}

//string ItemCfg::getItemType(int type)
//{
//    map<int, string>::iterator iter = iniNames.find(type);
//    if(iter == iniNames.end())
//        return "";
//    return iter->second;
//        
//}
string getIniName(int itemid);  //得到物品所在ini的名称
/*
 -----------------------------------------------------------------------------------
 */



ConstellStepDef::ConstellStepDef()
{
    setCons(0);
    setHp(0);
    setAccHp(0);
    setDef(0);
    setAccDef(0);
    setAtk(0);
    setAccAtk(0);

    setHit(0.0);
    setAccHit(0.0);
    setDodge(0.0);
    setAccDodge(0.0);
    setCri(0.0);
    setAccCri(0.0);
    setConPro(0.0);
    setConProAdd(0.0);
    
    // added by jianghan for 星灵命中概率改进
    setConProMaxFail(0.0);
    // end add
}




ConstellationName StringToConstellationName( std::string str )
{
    return

    (str == "aries") ? eAries :
    (str == "taurus") ? eTaurus :
    (str == "gemini") ? eGemini :
    (str == "cancer") ? eCancer :
    (str == "leo") ? eLeo :
    (str == "virgo") ? eVirgo :
    (str == "libra") ? eLibra :
    (str == "scorpio") ? eScorpio :
    (str == "sagittarius") ? eSagittarius :
    (str == "capricorn") ? eCapricorn :
    (str == "aquarius") ? eAquarius :
    (str == "pisces") ? ePisces :
    eConstellationNon;
}


ConstellCfgDef::ConstellCfgDef()
{
    setPageHp(0);
    setPageDef(0);
    setPageAtk(0);
    
    setPageHit(0.0);
    setPageDodge(0.0);
    setPageCri(0.0);
}

std::map<int, ConstellCfgDef*> ConstellationCfg::cfg;
int ConstellationCfg::indexMax = 0;
int ConstellationCfg::star_stepMax = 0;
void ConstellationCfg::load(std::string fullPath)
{
    try {
		int max = 0;
        int _star_stepMax = 0;
        GameInifile inifile(fullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            ConstellCfgDef *def = new ConstellCfgDef();
			def->consid = atoi(iter->c_str());
            def->name = inifile.getValue(*iter, "name");
            def->index = inifile.getValueT(*iter, "index", 0);
			int stepnum= inifile.getValueT(*iter, "step", 0);
			def->starStep = inifile.getValueT(*iter, "star_step", 0);
            def->step = stepnum;
			def->steps.resize(stepnum);

            string atkval = inifile.getValue(*iter, "atk", string(""));
            string defval = inifile.getValue(*iter, "def", string(""));
            string hpval = inifile.getValueT(*iter, "hp", string(""));
            string crival = inifile.getValue(*iter, "crit", string(""));
            string hitval = inifile.getValue(*iter, "hit", string(""));
            string dodgeval = inifile.getValue(*iter, "dodge", string(""));
            string consval = inifile.getValue(*iter, "constellval", string(""));
			string conpro = inifile.getValue(*iter, "con_pro", string(""));
            string conproAdd = inifile.getValue(*iter, "con_pro_add", string(""));
            // added by jianghan for 星灵命中概率改进
            string conproMaxFail = inifile.getValue(*iter, "con_pro_maxfail", string(""));
			// end add
            
            StringTokenizer atktoken(atkval, ",");
            StringTokenizer deftoken(defval, ",");
            StringTokenizer hptoken(hpval, ",");
            StringTokenizer critoken(crival, ",");
            StringTokenizer hittoken(hitval, ",");
            StringTokenizer dodgetoken(dodgeval, ",");
            StringTokenizer constoken(consval, ",");
			StringTokenizer conprotoken(conpro, ",");
            StringTokenizer conproAddtoken(conproAdd, ",");
            // added by jianghan for 星灵命中概率改进
            StringTokenizer conproMaxFailtoken(conproMaxFail, ",");
            // end add
            
            for (int i = 0; i < stepnum; i++)
            {
                ConstellStepDef stepdef;
                stepdef.setAtk(atoi(atktoken[i].c_str()));
                stepdef.setAccAtk(atoi(atktoken[stepnum + i].c_str()));
                stepdef.setDef(atoi(deftoken[i].c_str()));
                stepdef.setAccDef(atoi(deftoken[stepnum + i].c_str()));
                stepdef.setHp(atoi(hptoken[i].c_str()));
                stepdef.setAccHp(atoi(hptoken[stepnum + i].c_str()));
                stepdef.setCri(atof(critoken[i].c_str()));
                stepdef.setAccCri(atof(critoken[stepnum + i].c_str()));
                stepdef.setDodge(atof(dodgetoken[i].c_str()));
                stepdef.setAccDodge(atof(dodgetoken[stepnum + i].c_str()));
                stepdef.setHit(atof(hittoken[i].c_str()));
                stepdef.setAccHit(atof(hittoken[stepnum + i].c_str()));
                stepdef.setCons(atoi(constoken[i].c_str()));
				stepdef.setConPro(atof(conprotoken[i].c_str()));
                stepdef.setConProAdd(atof(conproAddtoken[i].c_str()));
                // added by jianghan for 星灵命中概率改进
                if( conproMaxFailtoken.count() > i )
                    stepdef.setConProMaxFail(atof(conproMaxFailtoken[i].c_str()));
                else
                    stepdef.setConProMaxFail(0.0f);
                // end add
                
                def->steps[i] = stepdef;
            }
            
            def->setPageHp(inifile.getValueT(*iter, "hp_all", 0));
            def->setPageDef(inifile.getValueT(*iter, "def_all", 0));
            def->setPageAtk(inifile.getValueT(*iter, "atk_all", 0));
            def->setPageHit(inifile.getValueT(*iter, "hit_all", 0.0f));
            def->setPageDodge(inifile.getValueT(*iter, "dodge_all", 0.0));
            def->setPageCri(inifile.getValueT(*iter, "crit_all", 0.0));
			
			if ( max < def->index ) max = def->index;
            if ( _star_stepMax < def->starStep) _star_stepMax = def->starStep;
            cfg.insert(make_pair(def->consid, def));
        }
		
		indexMax = max;
        star_stepMax = _star_stepMax;
		
    } catch (...) {
        throw 1;
    }
}


ConstellCfgDef* ConstellationCfg::getCfg(int id_)
{
    map<int, ConstellCfgDef*>::iterator iter = cfg.find(id_);
    if (iter == cfg.end())
    {
        return NULL;
    }

    return iter->second;
}
ConstellCfgDef* ConstellationCfg::getCfg(string name)
{
    map<int, ConstellCfgDef*>::iterator iter = cfg.begin();
    while (iter != cfg.end())
    {
        if (iter->second->name == name)
        {
            return iter->second;
        }
    }

    return NULL;
}
ConstellCfgDef* ConstellationCfg::getCfgByIndex(int index,int starStep)
{
    map<int, ConstellCfgDef*>::iterator iter = cfg.begin();
    while (iter != cfg.end())
    {
        if (iter->second->index == index && iter->second->starStep == starStep)
        {
            return iter->second;
        }
        iter++;
    }

    return NULL;
}


map<string, string> GlobalCfg::props;
vector<int> GlobalCfg::mMassRefreshHours;
int GlobalCfg::mMassRefreshPoint;
int GlobalCfg::mRefreshPeriod;
int GlobalCfg::mRefreshPoint;
int GlobalCfg::sCityPetNum;
int GlobalCfg::sCityRetinueNum;
int GlobalCfg::sBackbagRmbExtendSize;

int GlobalCfg::ReadInt(const char* propname, int def, bool *suc)
{
    string strpropname = propname;
    map<string, string>::iterator iter = props.find(strpropname);
    if (iter == props.end())
    {
        if (suc)
        {
            *suc = false;
        }
        return def;
    }
    if (suc)
    {
        *suc = true;
    }

    return Utils::safe_atoi(iter->second.c_str());
}


float GlobalCfg::ReadFloat(const char* propname, float def, bool *suc)
{
    map<string, string>::iterator iter = props.find(propname);
    if (iter == props.end())
    {
        if (suc)
        {
            *suc = false;
        }
        return def;
    }
    if (suc)
    {
        *suc = true;
    }
    return atof(iter->second.c_str());

}


string GlobalCfg::ReadStr(const char* propname, string def, bool *suc)
{
    map<string, string>::iterator iter = props.find(propname);
    if (iter == props.end())
    {
        if (suc)
        {
            *suc = false;
        }
        return def;
    }
    if (suc)
    {
        *suc = true;
    }
    return iter->second;
}


void GlobalCfg::load(std::string fullPath)
{
    try
    {
        GameInifile globalini(fullPath);

        {
            props.clear();
            const char *section = "common";
            vector<string> keys;
            globalini.getKeys(section, inserter(keys, keys.begin()));

            for (int k = 0; k < keys.size(); k++)
            {
                string value = globalini.getValue(section, keys[k]);
                props.insert(make_pair(keys[k], value));
            }
            
            sCityPetNum = globalini.getValueT(section, "city_pet_num", 0);
            sCityRetinueNum = globalini.getValueT(section, "city_retinue_num", 0);
            sBackbagRmbExtendSize = globalini.getValueT(section, "backbag_rmb_extend_size", 0);
        }
        
        
        {
            const char* fatSection = "fat";
            vector<string> keys;
            globalini.getKeys(fatSection, inserter(keys, keys.begin()));
            
            string massRefreshHours = globalini.getValue(fatSection, "mass_hours", "4;12");
            vector<string> hours = StrSpilt(massRefreshHours, ";");
            for (int i = 0; i < hours.size(); i++)
            {
                int hour = Utils::safe_atoi(hours[i].c_str(), -1);
                if (hour < 0 || hour >= 24)
                {
                    log_error("fat refresh hour error");
                    continue;
                }
                mMassRefreshHours.push_back(hour);
            }
            sort(mMassRefreshHours.begin(), mMassRefreshHours.end());
            mMassRefreshPoint = globalini.getValueT(fatSection, "mass_point", 50);
            mRefreshPeriod = globalini.getValueT(fatSection, "refresh_period", 30) * 60;
            mRefreshPoint = globalini.getValueT(fatSection, "refresh_point", 5);
        }
    }
    catch(...)
    {
        log_error("load global ini failed");
    }
}

vector<VipPrivilegeCfgDef*> VipPrivilegeCfg::cfg;
int VipPrivilegeCfg::mMaxLvl = 0;

void VipPrivilegeCfg::load(std::string fullPath)
{
    try
    {
        GameInifile vipini(fullPath);
        
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("vipMultiLanguage.ini") == "vipMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("vipMultiLanguage.ini"));
        }
        //GameInifile MLIniFile(Cfgable::getFullFilePath("vipMultiLanguage.ini"));

        int defmaxlvl = 0;
        int maxviplvl = vipini.getValueT("global", "maxlvl", defmaxlvl);

        mMaxLvl = maxviplvl;
        cfg.resize(maxviplvl + 1);

        //数值相关
        string needexp = vipini.getValue("val", "needexp");

        string monetreetimes = vipini.getValue("val", "moneytreetimes");
        string elitebuytimes= vipini.getValue("val", "elitebuytimes");
        
        string petadventurebuytimes= vipini.getValue("val", "petadventurebuytimes");
        string szPetAdventureRobotPetRentTimes= vipini.getValue("val", "petadventurerobotpetrenttimes");
        string crossServiceWarWorshipBuyTimes = vipini.getValue("val", "crossservicewarworshipbuytimes");
        string crossServiceWarBuyTimes = vipini.getValue("val", "crossservicewarbuytimes", "");

        string fatbuytimes = vipini.getValue("val", "fatbuytimes");
        string eliteHangeTimes = vipini.getValue("val", "elitehangeuptimes");
        string teamCopyBugTimes = vipini.getValue("val", "teamcopybuytimes");
        
        string operatepetBuyTimes = vipini.getValue("val", "operatepetbuytimes");

        string friendMaxCount = vipini.getValue("val", "friendmaxcount");
        string giftpackage = vipini.getValue("val", "giftpackage");
        string dungresettimes = vipini.getValue("val", "dungresettimes");
        string pvpbuystimes = vipini.getValue("val", "pvpbuytimes");

        string viplvlaward = vipini.getValue("val", "viplvlaward");
        string propstonelvl = vipini.getValue("val", "stone");
        string wakeupzhanhun = vipini.getValue("val", "wakeupzhanhun");
        string expExtra = vipini.getValue("val", "expextra");

        string freelotterytime = vipini.getValue("val", "freelotterytime");
        string buylotterytime = vipini.getValue("val", "buylotterytime");

        //这是各种特权的文字描述
        //        string moneytreedescrip = vipini.getValue("description", "moneytreetimes");
        //        string elitebuytimesdescrip= vipini.getValue("description", "elitebuytimes");
        //        string fatbuytimesdescrip = vipini.getValue("description", "fatbuytimes");
        //        string eliteHangeTimesdescrip = vipini.getValue("description", "elitehangeuptimes");
        //        string teamCopyBugTimesdescrip = vipini.getValue("description", "teamcopybuytimes");
        //
        //        string friendMaxCountdescrip = vipini.getValue("description", "friendmaxcount");
        //        string giftpackagedescrip = vipini.getValue("description", "giftpackage");
        //        string dungresettimesdescrip = vipini.getValue("description", "dungresettimes");
        //
        //        string propstonelvldescrip = vipini.getValue("description", "stone");
        //        string wakeupzhanhundescrip = vipini.getValue("description", "wakeupzhanhun");
        //        string expExtradescrip = vipini.getValue("description", "expextra");

        StringTokenizer exp(needexp, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);

        StringTokenizer moneytree(monetreetimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        StringTokenizer elite(elitebuytimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        StringTokenizer petAdventureCopyBuyTimes(petadventurebuytimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        StringTokenizer petAdventureRobotPetRentTimes(szPetAdventureRobotPetRentTimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);

        StringTokenizer crossServiceWarWorshipBuytimes(crossServiceWarWorshipBuyTimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        
        StringTokenizer crossServiceWarBuytimes(crossServiceWarBuyTimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        
        StringTokenizer operatepet(operatepetBuyTimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
   
        StringTokenizer fat(fatbuytimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        StringTokenizer teamcopy(teamCopyBugTimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        StringTokenizer elitehange(eliteHangeTimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);

        StringTokenizer friendCount(friendMaxCount, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        StringTokenizer gift(giftpackage, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        StringTokenizer dungreset(dungresettimes, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        StringTokenizer pvpbuy(pvpbuystimes, ";", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
        StringTokenizer viplvl(viplvlaward, ";", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);

        StringTokenizer freelotime(freelotterytime, ";", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
        StringTokenizer buylotime(buylotterytime, ";", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);

        //        StringTokenizer stonelvl(propstonelvl, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        //        StringTokenizer zhanhun(wakeupzhanhun, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        //        StringTokenizer expetra(expExtra, ";", StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
        int defauldcount = maxviplvl + 1;
        bool countError = exp.count() < defauldcount || elite.count() < defauldcount || fat.count() < defauldcount || teamcopy.count() < defauldcount || elitehange.count() < defauldcount || friendCount.count() < defauldcount || gift.count() < defauldcount || dungreset.count() < defauldcount || pvpbuy.count() < defauldcount || petAdventureCopyBuyTimes.count() < defauldcount || petAdventureRobotPetRentTimes.count() < defauldcount || crossServiceWarWorshipBuytimes.count() < defauldcount || crossServiceWarBuytimes.count() < defauldcount;
        //        || zhanhun.count() < maxviplvl + 1 ||
        //                        expetra.count() < maxviplvl + 1 || stonelvl.count() < maxviplvl + 1;

        if (countError) {
            log_error("vip.ini [val] is not in correct format!\n");
            assert(countError == false);
        }

        for (int i = 0; i <= maxviplvl; i++)
        {

            VipPrivilegeCfgDef* def = new VipPrivilegeCfgDef;
            def->mPrivilegeDesciption.clear();

            def->needexp = Utils::safe_atoi(exp[i].c_str());

            def->moneyTreeTimes = Utils::safe_atoi(moneytree[i].c_str());
            def->fatBuyTimes = Utils::safe_atoi(fat[i].c_str());
            def->eliteBuyTimes = Utils::safe_atoi(elite[i].c_str());
            def->petAdventureBuyTimes = Utils::safe_atoi(petAdventureCopyBuyTimes[i].c_str());
            def->petAdventureRobotPetRentBuyingTimesLimit = Utils::safe_atoi(petAdventureRobotPetRentTimes[i].c_str());
            def->crossServiceWarWorshipBuyTimes = Utils::safe_atoi(crossServiceWarWorshipBuytimes[i].c_str());
            def->crossServiceWarBuyTimes = Utils::safe_atoi(crossServiceWarBuytimes[i].c_str());
            
            def->petEliteBuytimes = Utils::safe_atoi(operatepet[i].c_str());
            def->teamCopyBuyTimes = Utils::safe_atoi(teamcopy[i].c_str());
            def->hangupElite = Utils::safe_atoi(elitehange[i].c_str());

            def->friendCount = Utils::safe_atoi(friendCount[i].c_str());
            def->giftPackage = Utils::safe_atoi(gift[i].c_str());
            def->dungeonsTimes = Utils::safe_atoi(dungreset[i].c_str());
            def->pvpbuytimes = Utils::safe_atoi(pvpbuy[i].c_str());
            def->viplvlAward = Utils::safe_atoi(viplvl[i].c_str());

            def->freelotterytime = Utils::safe_atoi(freelotime[i].c_str());
            def->buylotterytime = Utils::safe_atoi(buylotime[i].c_str());

            def->upgZhanHun = 0; //Utils::safe_atoi(zhanhun[i].c_str());
            def->expExtra = 0; //Utils::safe_atoi(expetra[i].c_str());
            def->propStoneLvl = 0.0f; //Utils::safe_atoi(stonelvl[i].c_str());

            //加载特权描述，这段代码不太美观
//            char tmp[10] = "";
//            sprintf(tmp, "vip%d", i);
            string tmp = Utils::makeStr("vip%d", i);
            int count = Utils::safe_atoi(vipini.getValue(tmp, "descripcount").c_str());
            for (int j = 1; j <= count; j++) {
//                char desc[10]="";
//                sprintf(desc, "descrip%d", j);
                string desc = Utils::makeStr("descrip%d_id", j);
                string descId = vipini.getValue(tmp, desc);
                //by lihengjin
                string desc_no_mul = Utils::makeStr("descrip%d", j);
                std::string str_desc;
                if(vipini.exists(tmp, desc_no_mul))
                {
                    str_desc = vipini.getValue(tmp, desc_no_mul);
                }else
                {
                    str_desc = MLIniFile.getValue(descId, "desc");
                }

                string str = str_desc;//vipini.getValue(tmp, desc);
                if (!str.empty()) {
                    def->mPrivilegeDesciption.push_back(str);
                }
            }

            cfg[i] = def;
            //如果该等级的值不为0，则代表拥有该值对应的特权，放在mPrivilegeDesciption

            //            if( (def->moneyTreeTimes = Utils::safe_atoi(moneytree[i].c_str()) ))
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, moneytreedescrip.c_str(), moneytree[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }
            //
            //            if( (def->fatBuyTimes = Utils::safe_atoi(fat[i].c_str())) )
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, fatbuytimesdescrip.c_str(), fat[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }
            //
            //            if( (def->eliteBuyTimes = Utils::safe_atoi(elite[i].c_str())) )
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, elitebuytimesdescrip.c_str(), elite[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }
            //
            //            if( (def->teamCopyBuyTimes = Utils::safe_atoi(teamcopy[i].c_str())) )
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, teamCopyBugTimesdescrip.c_str(), teamcopy[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }
            //
            //            if( (def->hangupElite = Utils::safe_atoi(elitehange[i].c_str())) )
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, elitebuytimesdescrip.c_str(), elitehange[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }
            //
            //            if( (def->friendCount = Utils::safe_atoi(friendCount[i].c_str())) )
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, friendMaxCountdescrip.c_str(), friendCount[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }
            //
            //            if( (def->giftPackage = Utils::safe_atoi(gift[i].c_str())) )
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, giftpackagedescrip.c_str(), gift[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }
            //
            //            if( (def->dungeonsTimes = Utils::safe_atoi(dungreset[i].c_str())) )
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, dungresettimesdescrip.c_str(), dungreset[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }
            //
            //            if( (def->upgZhanHun = Utils::safe_atoi(zhanhun[i].c_str())) )
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, wakeupzhanhundescrip.c_str(), zhanhun[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }
            //
            //            if( (def->expExtra = Utils::safe_atoi(expetra[i].c_str())) )
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, expExtradescrip.c_str(), expetra[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }
            //
            //            if( (def->propStoneLvl = Utils::safe_atoi(stonelvl[i].c_str())) )
            //            {
            //                char descrip[] = "";
            //                sprintf(descrip, propstonelvldescrip.c_str(), stonelvl[i].c_str());
            //                def->mPrivilegeDesciption.push_back(descrip);
            //            }

            //            cfg[i] = def;
        }
    }
    catch(...)
    {
        log_error("load vip ini failed");
    }
}

VipPrivilegeCfgDef* VipPrivilegeCfg::getCfg(int viplvl)
{
    if (viplvl >= cfg.size())
    {
        return NULL;
    }

    return cfg[viplvl];
}

int VipPrivilegeCfg::getMaxLvl()
{
    return mMaxLvl;
}

vector<int> LoginAwardCfg::activity_awards;
vector<int> LoginAwardCfg::normal_awards;
int LoginAwardCfg::after_openserver_days;

void LoginAwardCfg::load(std::string fullPath)
{
    try
    {
        GameInifile awardini(fullPath);

        string awardsstr;
        awardsstr = awardini.getValue("global", "activity_awards");

        activity_awards.clear();
        StringTokenizer token(awardsstr, ";");
        for (int i = 0; i < token.count(); i++)
        {
            activity_awards.push_back(atoi(token[i].c_str()));
        }
        
        string normalAwardsStr;
        normalAwardsStr = awardini.getValue("global", "normal_awards");
        normal_awards.clear();
        StringTokenizer item(normalAwardsStr, ";");
        for(int i = 0; i < item.count(); i++){
            normal_awards.push_back(atoi(item[i].c_str()));
        }
        
        after_openserver_days = awardini.getValueT("global", "after_openserver_days", 0);
        
    }
    catch(...)
    {
        log_error("load login award ini failed");
    }

}


int LoginAwardCfg::getAfterOpenServerDays()
{
    return after_openserver_days;
}

vector<int> & LoginAwardCfg::getActivityAwards()
{
    return activity_awards;
}

vector<int> & LoginAwardCfg::getNormalAwards()
{
    return normal_awards;
}

int LoginAwardCfg::getNormalAwardId(int days)
{
    if (days <= 0 || days > normal_awards.size())
    {
        return 0;
    }
    
    return normal_awards[days - 1];
}

int LoginAwardCfg::getAwardId(int days)
{
    if (days <= 0 || days > activity_awards.size())
    {
        return 0;
    }

    return activity_awards[days - 1];
}
 

int KoreaLevelAwardCfg::mLevel = 10;     // 领取等级
std::string KoreaLevelAwardCfg::mAward;  // 奖励内容
std::string KoreaLevelAwardCfg::mMsgUrl; // 留言URL

void KoreaLevelAwardCfg::load(std::string fullpath)
{
    try{
        GameInifile awardini(fullpath);
     
        mAward = awardini.getValue("korea_msg_award", "awards");
        mLevel = awardini.getValueT("korea_msg_award", "lvl",10);
        mMsgUrl = awardini.getValue("korea_msg_award", "url_google");
    }catch(...)
    {
        log_error("load KoreaLevelAwardCfg ini failed");
    }
}


vector<int> MoneyTreeCfg::golds;
vector<int> MoneyTreeCfg::freetimes;
vector<int> MoneyTreeCfg::consrmbs;
vector<float> MoneyTreeCfg::timesAddition;
void MoneyTreeCfg::load(std::string fullPath)
{
    try
    {
        GameInifile awardini(fullPath);

        string goldstr = awardini.getValue("global", "gold");
        string freetimesstr = awardini.getValue("global", "freetimes");
        string consrmbstr = awardini.getValue("global", "consrmb");
        string _timesAddition = awardini.getValue("global", "times_addition", "");
        
        assert(!_timesAddition.empty());
        
        golds.clear();

        StringTokenizer goldtoken(goldstr, ";");
        for (int i = 0; i < goldtoken.count(); i++)
        {
            golds.push_back(atoi(goldtoken[i].c_str()));
        }

        StringTokenizer timestoken(freetimesstr, ";");
        for (int i = 0; i < timestoken.count(); i++)
        {
            freetimes.push_back(atoi(timestoken[i].c_str()));
        }

        StringTokenizer rmbtoken(consrmbstr, ";");
        for (int i = 0; i < rmbtoken.count(); i++)
        {
            consrmbs.push_back(atoi(rmbtoken[i].c_str()));
        }
        
        StringTokenizer addition(_timesAddition, ";");
        for (int i = 0; i < addition.count(); i++) {
            if (!addition[i].empty()) {
                timesAddition.push_back(atof(addition[i].c_str()));
            }
        }
    }
    catch(...)
    {
        log_error("load money tree ini failed");
    }

}
int MoneyTreeCfg::getGold(int lvl, int openTimes)
{
    int basegold = 0;
    float addition = 0.0f;
    if (lvl <= 0 || openTimes <= 0) {
        return 0;
    }
    
    if (lvl > golds.size())
    {
        if (golds.size()) {
            basegold = golds[golds.size() - 1];
        }
    }
    else
    {
        basegold = golds[lvl - 1];
    }
    if (openTimes > timesAddition.size()) {
        if (timesAddition.size()) {
            addition = timesAddition[timesAddition.size() - 1];
        }
    }else
    {
        addition = timesAddition[openTimes - 1];
    }
    return basegold * addition;
    

}
int MoneyTreeCfg::getFreeTimes(int lvl)
{
    if (lvl <= 0 || lvl > freetimes.size())
    {
        return 0;
    }

    return freetimes[lvl - 1];
}
int MoneyTreeCfg::getConsRmb(int times)
{
    if (times <= 0)
    {
        return 0;
    }

    if (times > consrmbs.size())
    {
        return *consrmbs.rbegin();
    }

    return consrmbs[times - 1];
}

std::map<int, Newguidance*> NewguidanceCfg::cfg;
void NewguidanceCfg::load(std::string fullPath)
{

    try {

        GameInifile inifile(fullPath);
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("newguidanceMultiLanguage.ini") == "newguidanceMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("newguidanceMultiLanguage.ini"));
        }
        //GameInifile MLIniFile(Cfgable::getFullFilePath("newguidanceMultiLanguage.ini"));
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            Newguidance *def =  new Newguidance;
            def->level = inifile.getValueT(*iter, "level", 0);
            def->type = inifile.getValue(*iter, "type","");
            def->image = inifile.getValue(*iter,"image","");
            string content_id = inifile.getValue(*iter,"content_id");
            //by lihengjin
            std::string str_content;
            if(inifile.exists(*iter, "content"))
            {
                str_content = inifile.getValue(*iter, "content");
            }else
            {
                str_content = MLIniFile.getValue(content_id, "desc");
            }
            def->content = str_content;//inifile.getValue(*iter,"content","");
            def->taskID = inifile.getValueT(*iter,"taskID",0);
            def->parentType = inifile.getValue(*iter, "parentType" , "");
            def->guidenLevel = inifile.getValueT(*iter, "guidenLevel",0);
            def->targetTag = inifile.getValueT(*iter, "activityID",0);
            def->isOpen = false;     //初始值设为false
            def->prompt = inifile.getValueT(*iter, "prompt", 0);
            def->direction = inifile.getValueT(*iter, "direction",0);
            string sub_content_id = inifile.getValue(*iter, "sub_content_id", "");
            //by lihengjin
            std::string str_sub;
            if(inifile.exists(*iter, "sub_content"))
            {
                str_sub = inifile.getValue(*iter, "sub_content", "");
            }else
            {
                str_sub = MLIniFile.getValue(sub_content_id, "desc");
            }

            def->sInfo = str_sub;//inifile.getValue(*iter, "sub_content", "");
            cfg[atoi(iter->c_str())] = def;
        }
    } catch (...) {
        throw 1;
    }

}

vector<Newguidance *> NewguidanceCfg::getNewguidanceData(int level)
{
    vector<Newguidance *> data;
    for(map<int, Newguidance *>::iterator  iter = cfg.begin();iter != cfg.end();iter++)
    {
        if(iter->second->type != "person" && iter->second->level == level && iter->second->taskID == 0)
        {
            data.push_back(iter->second);
        }
    }
   
    return data;
}

vector<Newguidance *> NewguidanceCfg::getAllNewguildanceData(int level)
{
    vector<Newguidance *> data;
    for(map<int, Newguidance *>::iterator  iter = cfg.begin();iter != cfg.end();iter++)
    {
        if(iter->second->type != "person" && iter->second->level == level )
        {
            data.push_back(iter->second);
        }
    }
    
    return data;
}
vector<Newguidance *> NewguidanceCfg::getNewguidanceData(int level,int  guidanceLevel,string parentType)
{
    vector<Newguidance *> data;
    for(map<int, Newguidance *>::iterator  iter = cfg.begin();iter != cfg.end();iter++)
    {
        if(iter->second->type != "person" && iter->second->level == level && iter->second->taskID == 0 && iter->second->guidenLevel==guidanceLevel && iter->second->isOpen==true && iter->second->parentType==parentType)
        {
            data.push_back(iter->second);
        }
    }
    return data;
}
Newguidance * NewguidanceCfg::getNewguidanceDataByTag(int level,int  guidanceLevel,string parentType,int tag)
{
    map<int, Newguidance *> CFG = cfg;
    for(map<int, Newguidance *>::iterator  iter = cfg.begin();iter != cfg.end();iter++)
    {
        if(iter->second->type != "person" && iter->second->level ==level && iter->second->taskID == 0 && iter->second->guidenLevel==guidanceLevel && iter->second->isOpen==true && iter->second->parentType ==parentType&& iter->second->targetTag==tag)
        {
             return iter->second;
        }
    }
    
    return NULL;

}
vector<Newguidance *>NewguidanceCfg::getNewDateFromTask()
{
    vector<Newguidance *> data;
    for(map<int, Newguidance *>::iterator  iter = cfg.begin();iter != cfg.end();iter++)
    {
        if(iter->second->type != "person" && iter->second->taskID != 0)
        {
            data.push_back(iter->second);
        }
    }
    return data;
}
Newguidance * NewguidanceCfg::getCfg(int step)
{
    map<int, Newguidance *>::iterator iter =cfg.find(step);
    if(iter != cfg.end())
    {
        return iter->second;
    }
    return NULL;
}
vector<Newguidance *> NewguidanceCfg::getSystemPrompt(int level)
{
    vector<Newguidance *> data;
    for(map<int, Newguidance *>::iterator  iter = cfg.begin();iter != cfg.end();iter++)
    {
        if(iter->second->type != "person" && iter->second->level == level &&  iter->second->taskID == 0 && iter->second->prompt == 1)
        {
            data.push_back(iter->second);
        }
    }
    return data;


}
vector<Newguidance *>  NewguidanceCfg::getArrowsPrompt(int level)
{
    vector<Newguidance *> data;
    for(map<int, Newguidance *>::iterator  iter = cfg.begin();iter != cfg.end();iter++)
    {
        if(iter->second->type != "person" && iter->second->level == level && iter->second->taskID == 0 &&iter->second->prompt == 2 && iter->second->guidenLevel == 1)
        {
            data.push_back(iter->second);
        }else if(iter->second->type != "person" && iter->second->level == level && iter->second->taskID == 0 &&iter->second->prompt == 2 && iter->second->guidenLevel != 1)
        {
            Newguidance *newguidance = NewguidanceCfg::getParentGuidance(iter->second);
            if(newguidance!=NULL)
            {
                bool isExist = false;
                for(int i=0; i< data.size();i++)
                {
                    if(data[i]==newguidance)
                    {
                        isExist = true;
                    }
                }
                if(!isExist)
                {
                    data.push_back(newguidance);
                }
            }
            iter->second->isOpen = true;
        }
    }
    
    return data;

}

Newguidance * NewguidanceCfg::getParentGuidance(Newguidance *newguidance)  //通过递归求出级别为1的指引
{
    Newguidance *parentGuidance = NewguidanceCfg::getCfgByName(newguidance->parentType);
    if(parentGuidance!=NULL)
    {
        if(parentGuidance->guidenLevel!=1)
        {
            parentGuidance = NewguidanceCfg::getParentGuidance(parentGuidance);
        }
        return parentGuidance;
    }
    return NULL;
}

int NewguidanceCfg::getLevelByName(std::string name)
{
    for(map<int, Newguidance *>::iterator  iter = cfg.begin();iter != cfg.end();iter++)
    {
        if(iter->second->type == name)
        {
            return iter->second->level;
        }
    }
    return 0;
}
Newguidance * NewguidanceCfg::getCfgByName(string name)
{
    map<int, Newguidance *>::iterator iter =cfg.begin();
    for(;iter != cfg.end(); iter++)
    {
        if(iter->second->type == name)
        {
            return iter->second;
        }
    }
    return NULL;

}
Newguidance * NewguidanceCfg::getCfgByTypeAndImg(string type, string img)
{
    map<int, Newguidance *>::iterator iter =cfg.begin();
    for(;iter != cfg.end(); iter++)
    {
        if(iter->second->type == type &&  iter->second->image == img )
        {
            return iter->second;
        }
    }
    return NULL;
    
}
vector<Newguidance *>  NewguidanceCfg::getCfgByParentType(string parentType)
{
    vector<Newguidance *> data;
    for(map<int, Newguidance *>::iterator  iter = cfg.begin();iter != cfg.end();iter++)
    {
        if(iter->second->parentType == parentType)
        {
            data.push_back(iter->second);
        }
     }
    return data;
}
vector<string> WordStockCfg::wordData;
void WordStockCfg::load(std::string fullPath)
{
    try {

        GameInifile inifile(fullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            wordData.push_back(inifile.getValue(*iter, "desc"));
        }
    } catch (...) {
        throw 1;
    }

}
vector<string> WordStockCfg::getWordStockData()
{
    vector<string> Data;
    for(int i=0;i< wordData.size();i++)
    {
        StringTokenizer token(wordData[i], "|");
        for(int j=0;j< token.count();j++)
        {
            Data.push_back(token[j]);
        }
    }
    return Data;
}
/*
 --------------------------------------------
 */


/*
 --------------------------------------------
 */

BuyType stringToBuyType(std::string str)
{
    return
    (str == "fatbuy") ? eFatBuy :
    (str == "elitebuy") ? eEliteBuy :
    (str == "pvptimesbuy") ? ePvpTimesBuy :
    (str == "dungbuy") ? eDungBuy :
    (str == "teamcopyreset") ? eTeamCopyReset:
    (str == "lotterybuy") ?eLotteryBuy :
    (str == "dailyquestbuy") ? eDailyQuestRefresh :
    (str == "dicelotterybuy") ? eDiceLotteryBuy :
    (str == "operatebuy") ? eOperateBuy:
    (str == "petadventurebuy") ? ePetAdventureBuy:
    (str == "petadventurerobotpetrent") ? ePetAdventureRobotPetRent:
    (str == "crossservicewarworshipbuy") ? eCrossServiceWarWorShipBuy:
    (str == "crossservicewarbuy") ? eCrossServiceWarBuy:

    eBuyNon;
}

CostType stringToCostType(std::string str)
{
    return
    (str == "rmb") ? eRmbCost :
    (str == "gold") ? eGoldCost :
    eCostNon;
}
std::map<int, BuyCfgDef*> BuyCfg::cfg;

void BuyCfg::load(std::string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);

        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            //sprintf(idstr,"%d", i);
            if(inifile.exists(*iter))
            {
                string costTypeStr = inifile.getValue(*iter, "costtype");
                string rmbCostStr = inifile.getValue(*iter, "rmbcost");
                string goldCostStr = inifile.getValue(*iter, "goldcost");
                string doublelotstr = inifile.getValue(*iter, "doublelottery");
                string timesstr = inifile.getValue(*iter, "lvltimes");

                BuyCfgDef* def = new BuyCfgDef;
                def->addpoint = atoi(inifile.getValue(*iter, "addpoint").c_str());
                def->costType = stringToCostType(costTypeStr);
                def->cost.clear();

                StringTokenizer rmbtoken(rmbCostStr, ";");
                for (int i = 0; i < rmbtoken.count(); i++)
                {
                    def->cost.push_back(atoi(rmbtoken[i].c_str()));
                    def->costType = eRmbCost;
                }

                StringTokenizer goldtoken(goldCostStr, ";");
                for (int i = 0; i < goldtoken.count(); i++) {
                    def->cost.push_back(Utils::safe_atoi(goldtoken[i].c_str()));
                    def->costType = eGoldCost;
                }

                StringTokenizer doublelottoken(doublelotstr, ";");
                for (int i = 0; i < doublelottoken.count(); i++) {
                    def->doublelottery.push_back(atof(doublelottoken[i].c_str()));
                }

                StringTokenizer timestoken(timesstr, ";");
                for (int i = 0; i < timestoken.count(); ++i){
                    def->times.push_back(atoi(timesstr.c_str()));
                }

                BuyType type = stringToBuyType(*iter);
                cfg[type] = def;
            }
        }

    }
    catch(...)
    {
        log_error("load buy ini failed");
    }
}

int BuyCfg::getAddpoint(int type)
{
    if (cfg[type]) {
        return cfg[type]->addpoint;
    }
    return 0;
}

int BuyCfg::getLvlTimes(int type, int lvl)
{
    BuyCfgDef* def = cfg[type];
    if (lvl > 0 && def)
    {
        int size = def->times.size();
        if (lvl > size){
            return def->times[size - 1];
        }
        else{
            return def->times[lvl];
        }
    }

    return 0;
}

//int BuyCfg::getRmbCost(int type, int times)
//{
//    if (cfg[type]) {
//        if ( times < 0 || times >= cfg[type]->rmbCost.size() ) {
//            return -1;
//        }
//        return cfg[type]->rmbCost[times];
//    }
//
//    return -1;
//}

int BuyCfg::getCost(int buytype, int times, int& costType)
{
    if (cfg[buytype]) {
        costType = cfg[buytype]->costType;

        int size = cfg[buytype]->cost.size();
        if( size != 0 && size <= times){//如果次数超过数组长度则取最后一个
            return cfg[buytype]->cost[size - 1];
        }

        return cfg[buytype]->cost[times];
    }

    return 0x7FFFFFFF;
}

int BuyCfg::getCost(int buytype, int times, string& moneyname)
{
    moneyname = "";
    if (cfg[buytype]) {
        int costType = cfg[buytype]->costType;
        moneyname = costTypeToMoneyName(costType);
        int size = cfg[buytype]->cost.size();
        if( size != 0 && size <= times){
            return cfg[buytype]->cost[size - 1];
        }
        return cfg[buytype]->cost[times];
    }

    //return 0x7FFFFFFF;
    return 0;
}

int BuyCfg::getSize(int type)
{
    if (cfg[type]) {
        return cfg[type]->cost.size();
    }

    return -1;
}

float BuyCfg::getDouLottery(int times)
{
    if( cfg[eLotteryBuy] ){
        BuyCfgDef* lotcfg = cfg[eLotteryBuy];

        if(times < 0){
            log_error(" BuyCfg::getDouLottery(int times) 出错");
            return 0.0f;
        }

        int size = lotcfg->doublelottery.size();
        if( times >= size){
            return lotcfg->doublelottery[size-1];
        }else{
            return lotcfg->doublelottery[times];
        }
    }

    log_error(" BuyCfg::getDouLottery(int times) 出错");
    return 0.0f;
}

string BuyCfg::costTypeToMoneyName(int costType)
{
#ifdef CLIENT
    switch (costType)
    {
        case eRmbCost:
            return ML_TAG(10004);
        case eGoldCost:
            return  ML_TAG(40722);
        default:
            return "";
    }
#else
    switch (costType) {
        case eRmbCost:
            return  MoneyMLCfg::getValue("10003", "desc");
        case eGoldCost:
            return  MoneyMLCfg::getValue("10001", "desc");
        default:
           return "";
    }
    
#endif

}

vector<int> PvpCfg::mRanges;
std::map<int, PvpAwardDef*> PvpCfg::mRankAward;
vector<PvpAward*> PvpCfg::mPvpResultAward;
int PvpCfg::mfreetimes = 0;
int PvpCfg::mOpenlvl = 0;
int PvpCfg::mTimeout = 0;
vector<PvpRankDataDef*> PvpCfg::mRankDataRule;
vector<int> PvpCfg::mCooldownTime;
vector<int> PvpCfg::mFreshCost;
int PvpCfg::mRobotCount = 0;
int PvpCfg::mAwardPointPeriod = 0;

void PvpCfg::load(std::string fullPath)
{
    mRankDataRule.clear();
    try
    {
        GameInifile inifile(fullPath);

        string ranges = inifile.getValue("rankranges", "ranges", "").c_str();
        StringTokenizer range(ranges, ";");
        mRanges.clear();
        mPvpResultAward.clear();
        mRankAward.clear();

        assert(range.count() > 0);

        for (int i = 0; i < range.count(); i++) {
            int val = Utils::safe_atoi(range[i].c_str());
            mRanges.push_back(val);
        }

        for (int i = 1; i < mRanges.size(); i++) {
            
            string tmp = Utils::makeStr("awardpart%d",i);

            string protossStr = inifile.getValue(tmp, "protoss", "");
            string goldPartStr = inifile.getValue(tmp, "gold", "");
            string itemAwardStr = inifile.getValue(tmp, "awarditem", "");

            StringTokenizer protoss(protossStr, ";");
            StringTokenizer gold(goldPartStr, ";");
            StringTokenizer awardItems(itemAwardStr, ",");
            
            PvpAwardDef* def = new PvpAwardDef;
            for (int j = 0; j < protoss.count() && j < gold.count() && j < awardItems.count(); j++) {
                
                PvpAward* awards = new PvpAward;
                awards->protoss = Utils::safe_atoi(protoss[j].c_str());
                awards->gold = Utils::safe_atoi(gold[j].c_str());
                awards->itemAward = awardItems[j];
                
                def->mAwards.push_back(awards);
            }

            def->mIsShuzu = Utils::safe_atoi(inifile.getValue(tmp, "is_shuzu", "").c_str());
            def->mMinRank = Utils::safe_atoi(inifile.getValue(tmp, "xiaxian", "").c_str());
            def->mMaxRank = Utils::safe_atoi(inifile.getValue(tmp, "shangxian", "").c_str());

            mRankAward.insert(make_pair(i, def));
        }

        string protossStr = inifile.getValue("immediatereward", "protoss", "");
        string goldPartStr = inifile.getValue("immediatereward", "gold", "");

        StringTokenizer protoss(protossStr, ";");
        StringTokenizer gold(goldPartStr, ";");

        assert(protoss.count() == 2 && gold.count() == 2);

        for (int i = 0; i < protoss.count() && i < gold.count(); i++) {
            PvpAward* immedAward = new PvpAward;
            immedAward->protoss = Utils::safe_atoi(protoss[i].c_str());
            immedAward->gold = Utils::safe_atoi(gold[i].c_str());
            mPvpResultAward.push_back(immedAward);
        }

        string freetimes = inifile.getValue("free", "freetimes");
        mfreetimes = Utils::safe_atoi(freetimes.c_str());

        string openlvl = inifile.getValue("free", "openlvl");
        mOpenlvl = Utils::safe_atoi(openlvl.c_str());
        
        mTimeout = inifile.getValueT("free", "timeout", 0);
        mRobotCount = inifile.getValueT("free", "robot_count", 40);
        mAwardPointPeriod = inifile.getValueT("free", "award_point_period", 0);

        int parts = inifile.getValueT("rankparts", "parts", 0);
        for (int i = 0;  i < parts; i++) {
//            char tmp[128] = {0};
//            sprintf(tmp, "rank%d",i + 1);
            
            string tmp = Utils::makeStr("rank%d",i + 1);
            int min = inifile.getValueT(tmp, "min", 0);
            int max = inifile.getValueT(tmp, "max", 0);
            int space = inifile.getValueT(tmp, "space", 0);

            if (min <= 0 || max <= 0 || space <= 0) {
                printf("Pvpcfg formate error [rank rule]\n");
                continue;
            }

            PvpRankDataDef* data = new PvpRankDataDef;
            data->mMin = min;
            data->mMax = max;
            data->space = space;

            mRankDataRule.push_back(data);
        }

        mFreshCost.clear();
        mCooldownTime.clear();
        string cooldowntimeStr = inifile.getValue("cooldown", "time", "");
        string freshcostStr = inifile.getValue("cooldown", "cost", "");

        StringTokenizer times(cooldowntimeStr, ";");
        StringTokenizer costs(freshcostStr, ";");

        for (int i = 0; i < times.count(); i++) {
            mCooldownTime.push_back(Utils::safe_atoi(times[i].c_str()));
        }

        for (int i = 0; i < costs.count(); i++) {
            mFreshCost.push_back(Utils::safe_atoi(costs[i].c_str()));
        }

    }
    catch(...)
    {

    }
}

int PvpCfg::getTimeout()
{
    return mTimeout;
}

int PvpCfg::getRobotCount()
{
    return mRobotCount;
}

PvpAward* PvpCfg::getRankAward(int rank)
{
    int index = 0;
    for (index = 0; index < mRanges.size(); index++) {
        if (rank <= mRanges[index]) {
            break;
        }
    }

    if (index >= mRanges.size()) {
        index = mRanges.size() - 1;
    }
    
    index = index < 1 ? 1 : index;
    PvpAwardDef* def = mRankAward[index];
    
    if (def->mIsShuzu) {
        int tmp = rank - def->mMinRank;
        if (tmp >= def->mAwards.size()) {
            tmp = def->mAwards.size() - 1;
        }
        return def->mAwards[tmp];
    }
    else
    {
        return def->mAwards[0];
    }

}

PvpAward* PvpCfg::getResultAward(bool win)
{
    if (win) {
        return mPvpResultAward[1];
    }
    return mPvpResultAward[0];
}

int PvpCfg::getfreeTimes()
{
    return mfreetimes;
}

int PvpCfg::getOpenLvl()
{
    return mOpenlvl;
}

void PvpCfg::getRankRule(int myrank, PvpRankDataDef &output)
{
    for (int i = 0; i < mRankDataRule.size(); i++) {
        if (myrank > mRankDataRule[i]->mMax) {
            continue;
        }
        else if(myrank >= mRankDataRule[i]->mMin)
        {
            output.mMin = mRankDataRule[i]->mMin;
            output.mMax = mRankDataRule[i]->mMax;
            output.space = mRankDataRule[i]->space;
        }
    }
}
//**********************************  friend_reward  ***********************************//
vector<int> FriendRewardCfg::mRanges;
std::map<int, FriendRewardDef*> FriendRewardCfg::mRankAward;

void FriendRewardCfg::load(std::string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);

        string ranges = inifile.getValue("rankranges", "ranges", "").c_str();
        StringTokenizer range(ranges, ";");
        mRanges.clear();
        mRankAward.clear();

        assert(range.count() > 0);

        for (int i = 0; i < range.count(); i++)
        {
            int val = Utils::safe_atoi(range[i].c_str());
            mRanges.push_back(val);
        }

        for (int i = 1; i < mRanges.size(); i++)
        {
//            char tmp[128] = {0};
//            sprintf(tmp, "awardpart%d",i);
            string tmp = Utils::makeStr("awardpart%d",i);

            FriendRewardDef* def = new FriendRewardDef;

            def->mReward = inifile.getValueT(tmp, "reward", 0.0f);
            def->mMin = inifile.getValueT(tmp, "xiaxian", 0);
            def->mMax = inifile.getValueT(tmp, "shangxian", 0);
            def->mRewardBattlePoint = inifile.getValueT(tmp, "reward_battlepoint", 0.0f);
            def->mRewardExp = inifile.getValueT(tmp, "reward_exp", 0.0f);
            def->mRewardGold = inifile.getValueT(tmp, "reward_gold", 0.0f);
            def->mRewardPetExp = inifile.getValueT(tmp, "reward_petexp", 0.0f);
            
            mRankAward.insert(make_pair(i, def));
        }
    }
    catch(...)
    {

    }
}

FriendRewardDef* FriendRewardCfg::getRankAward(int intimacy)
{
    int index = 1;
    for (int i = 1; i < mRanges.size(); i++)
    {
        if (intimacy <= mRanges[i])
        {
            index = i;
            break;
        }
    }
    std::map<int, FriendRewardDef*>::iterator it = mRankAward.find(index);
    if(it == mRankAward.end())
    {
        return NULL;
    }
    return it->second;
}
#pragma mark - 神秘商店
void MysteriousCfg::readMerchant(GameInifile &inifile)
{
	// 调基类方法
	RandGoodsCfgBase::readMerchant(inifile);
	
	mUpdateGold = inifile.getValueT("gold", "gold", 0);
	mGoldFreeNum = inifile.getValueT("goldfree", "free", 0);
	
	mTimeGoodsDesc = inifile.getValue("time_limit_buy_desc", "desc");
    

    
    string isUseViplvlRefreshLimit = inifile.getValue("use_viplvl_refresh_limit", "use_viplvl_refresh_limit", "false");
    //assert(isUseViplvlRefreshLimit.empty() == true);
    
    if (isUseViplvlRefreshLimit == "true")
    {
        mIsUseViplvlRefreshLimit = true;
    }
    else
    {
        mIsUseViplvlRefreshLimit = false;
    }
    
    if (mIsUseViplvlRefreshLimit == true)
    {
        vector<string> keys;
        inifile.getKeys("viplvl_refresh_limit", inserter(keys, keys.begin()));
        
        assert(keys.size() > 0);
        
        for (vector<string>::iterator it = keys.begin(); it != keys.end(); it++)
        {
            StringTokenizer token(*it, "-");
            assert(token.count() == 2);
            
            int key = Utils::safe_atoi(token[1].c_str());
            int value = inifile.getValueT("viplvl_refresh_limit", *it, 0);
            mViplvlRangeMap.insert(make_pair(key, value));
        }
    }
	
	mTimeStart = Utils::parseDate(inifile.getValue("time_limit_buy", "start_time").c_str());
	mTimeEnd = Utils::parseDate(inifile.getValue("time_limit_buy", "end_time").c_str());
	mTimeIsDelay = inifile.getValueT("time_limit_buy", "is_delay", 0);
	
	mExchangeStart = Utils::parseDate(inifile.getValue("shop_exchange", "start_time").c_str());
	mExchangeEnd = Utils::parseDate(inifile.getValue("shop_exchange", "end_time").c_str());
	mExchangeIsDelay = inifile.getValueT("shop_exchange", "is_delay", 0);
}

void MysteriousCfg::readGoods(GameInifile &inifile)
{
	try
    {
        //lihengjin
        GameInifile MLInifile;
        if(!(Cfgable::getFullFilePath("activitygoodsMultiLanguage.ini") == "activitygoodsMultiLanguage.ini"))
        {
            MLInifile = GameInifile(Cfgable::getFullFilePath("activitygoodsMultiLanguage.ini"));
        }
        //GameInifile MLInifile(Cfgable::getFullFilePath("activitygoodsMultiLanguage.ini"));
        mMysteriousMap.clear();
		
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            MysteriousGoodsDef def;
			def.index = inifile.getValueT(*it, "index", 0);
			def.goodsId = inifile.getValueT(*it, "id", 0);
			
			StringTokenizer lvlToken(inifile.getValue(*it, "lvl", ""), "-");
			assert(lvlToken.count() == 2);
			
			def.lvlMin = Utils::safe_atoi(lvlToken[0].c_str());
			def.lvlMax = Utils::safe_atoi(lvlToken[1].c_str());
            
            //vip level
            StringTokenizer viplvlToken(inifile.getValue(*it, "viplvl", "0-10"), "-");
			assert(viplvlToken.count() == 2);
			
			def.viplvlMin = Utils::safe_atoi(viplvlToken[0].c_str());
			def.viplvlMax = Utils::safe_atoi(viplvlToken[1].c_str());
			
			def.pro = inifile.getValueT(*it, "pro", 0);
			def.type = inifile.getValueT(*it, "type", 0);
			
            StringTokenizer bgoldToken(inifile.getValue(*it, "consume", ""), ";");
            assert(bgoldToken.count() == 2);
			
            def.consumeMin = Utils::safe_atoi(bgoldToken[0].c_str(), 0);
            def.consumeMax = Utils::safe_atoi(bgoldToken[1].c_str(), 0);
			
            std::string DescID = inifile.getValue(*it, "desc_id");            
            //by lihengjin
            std::string DescStr;
            if(inifile.exists(*it, "desc"))
            {
                DescStr = inifile.getValue(*it, "desc");
            }
            else
            {
                DescStr = MLInifile.getValue(DescID, "desc");
            }
			def.desc = DescStr;//inifile.getValue(*it, "desc");
			def.sale = inifile.getValue(*it, "sale");
			
			mMysteriousMap.insert(make_pair(def.index, def));
        }
    }
    catch(...)
    {
        log_error("MysteriousManCfg goods ERROR !!!");
    }
}

void MysteriousCfg::readTimeActivaityGoods(GameInifile &inifile)
{
	try
    {
		mMysteriousTimeGoodsMap.clear();
        //lihengjin
        GameInifile MLIniFile;
        if (!(Cfgable::getFullFilePath("time_limit_buyMultiLanguage.ini") == "time_limit_buyMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("time_limit_buyMultiLanguage.ini"));
        }
		//GameInifile MLIniFile(Cfgable::getFullFilePath("time_limit_buyMultiLanguage.ini"));
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            MysteriousTimeGoodsDef def;
			def.index = inifile.getValueT(*it, "index", 0);
			def.goodsId = inifile.getValueT(*it, "id", 0);
			def.type = inifile.getValueT(*it, "type", 0);
			def.consume = inifile.getValueT(*it, "consume", 0);
            string descId = inifile.getValue(*it, "desc_id");
            //by lihengjin
            std::string str_desc;
            if(inifile.exists(*it, "desc"))
            {
                str_desc = inifile.getValue(*it, "desc");
            }else
            {
                str_desc = MLIniFile.getValue(descId, "desc");
            }

			def.desc = str_desc;//inifile.getValue(*it, "desc");
			def.sale = inifile.getValue(*it, "sale");
			
			def.limit_num = inifile.getValueT(*it, "limit_num", 0);
			def.rank = inifile.getValueT(*it, "rank", 0);
			
			def.start = Utils::parseDate(inifile.getValue(*it, "start_time").c_str());
			def.end = Utils::parseDate(inifile.getValue(*it, "end_time").c_str());
			def.after_openserver_days = inifile.getValueT(*it, "after_openserver_days", 0);
			mMysteriousTimeGoodsMap.insert(make_pair(def.index, def));
        }
    }
    catch(...)
    {
        log_error("MysteriousManCfg readTimeActivaityGoods ERROR !!!");
    }
}

void MysteriousCfg::readExchangeGoods(GameInifile &inifile)
{
	try
    {
		mMysteriousExchangeGoodsMap.clear();
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("shop_exchangeMultiLanguage.ini") == "shop_exchangeMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("shop_exchangeMultiLanguage.ini"));
        }
		//GameInifile MLIniFile(Cfgable::getFullFilePath("shop_exchangeMultiLanguage.ini"));
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            MysteriousExchangeDef def;
			def.index = inifile.getValueT(*it, "index", 0);
			def.goodsId = inifile.getValueT(*it, "id", 0);
			string lvl = inifile.getValue(*it, "lvl");
			
			sscanf(lvl.c_str(), "%d-%d", &def.lvlMin, &def.lvlMax);
			assert(def.lvlMin > 0 && def.lvlMax > 0 && def.lvlMin <= def.lvlMax);
			
			def.exchangeId = inifile.getValueT(*it, "exchange_id", 0);
			def.exchangeNum = inifile.getValueT(*it, "num", 0);
			assert(def.exchangeNum > 0 && def.exchangeId > 0);
			
            string descId = inifile.getValue(*it, "desc_id");
            //by lihengjin
            std::string str_desc;
            if(inifile.exists(*it, "desc"))
            {
                str_desc = inifile.getValue(*it, "desc");
            }
            else
            {
                str_desc = MLIniFile.getValue(descId, "desc");
            }

			def.desc = str_desc;//inifile.getValue(*it, "desc");
			def.sale = inifile.getValue(*it, "sale");
			
			def.limit_num = inifile.getValueT(*it, "limit_num", 0);
			def.rank = inifile.getValueT(*it, "rank", 0);
			
			def.start = Utils::parseDate(inifile.getValue(*it, "start_time").c_str());
			def.end = Utils::parseDate(inifile.getValue(*it, "end_time").c_str());
			def.after_openserver_days = inifile.getValueT(*it, "after_openserver_days", 0);
			mMysteriousExchangeGoodsMap.insert(make_pair(def.index, def));
        }
    }
    catch(...)
    {
        
        log_error("MysteriousManCfg readExchangeGoods ERROR !!!");
    }
}

void MysteriousCfg::read()
{
	try
    {
		{
			GameInifile inifile("activitymerchant.ini");
			readMerchant(inifile);
		}
		{
			GameInifile inifile("activitygoods.ini");
			readGoods(inifile);
		}
		{
			GameInifile inifile("time_limit_buy.ini");
			readTimeActivaityGoods(inifile);
		}
		{
			GameInifile inifile("shop_exchange.ini");
			readExchangeGoods(inifile);
		}
	}
    catch(...)
    {
        log_error("MysteriousManCfg ERROR !!!");
    }
}


void MysteriousCfg::randGoods(int level, int viplevel, vector<MysteriousGoodsDef*> &vec)
{
	vector<int> bat;
	vector<MysteriousGoodsDef*> batTemp;
	MysteriousGoodsTd::iterator it;
    for (it = mMysteriousMap.begin(); it != mMysteriousMap.end(); it++) {
        if (viplevel < it->second.viplvlMin || viplevel > it->second.viplvlMax)
        {
            continue;
        }
        if (level < it->second.lvlMin || level > it->second.lvlMax) {
            continue;
        }
        bat.push_back(it->second.pro);
		batTemp.push_back(&it->second);
    }
	
    if (bat.empty()) {
        return;
    }
	
    // 取等级数量
    int randNum = 0;
    for (std::map<int, int>::iterator it = mLvlRangeMap.begin(); it != mLvlRangeMap.end(); it++) {
        if (level <= it->first) {
            randNum = it->second;
            break;
        }
    }
    if (randNum == 0) {
        randNum = mLvlRangeMap.rbegin()->second;
    }
	
    int batSize = bat.size();
    check_max(randNum, batSize);
	
    // 随机物品
    vector<int> index(randNum, 0);
    getRands(bat.data(), batSize, randNum, index.data(), false);
    for (vector<int>::iterator it = index.begin() ; it != index.end(); it++) {
        vec.push_back(batTemp[*it]);
    }
}

bool MysteriousCfg::checkIsIndex(int index, int goodsId)
{
	MysteriousGoodsTd::iterator iter = mMysteriousMap.find(index);
    if(iter == mMysteriousMap.end()){
        return false;
    }
	if (iter->second.goodsId != goodsId) {
		return false;
	}
	return true;
}

MysteriousGoodsDef * MysteriousCfg::getMysteriousGoodsDef(int index)
{
    MysteriousGoodsTd::iterator iter = mMysteriousMap.find(index);
    if(iter == mMysteriousMap.end()){
        return NULL;
    }
    return &iter->second;
}

MysteriousTimeGoodsDef * MysteriousCfg::getMysteriousTimeGoodsDef(int index)
{
	MysteriousTimeGoodsTd::iterator it = mMysteriousTimeGoodsMap.find(index);
	if (it == mMysteriousTimeGoodsMap.end()) {
		return NULL;
	}
	return &it->second;
}

map<int, MysteriousTimeGoodsDef> MysteriousCfg::getMysteriousTimeGoodsMap()
{
    return mMysteriousTimeGoodsMap;
}

void MysteriousCfg::getMysteriousTimeGoodsMap(vector<MysteriousTimeGoodsDef*> &vec)
{
	int now = time(NULL);
	MysteriousTimeGoodsTd::iterator it;
	for (it = mMysteriousTimeGoodsMap.begin(); it != mMysteriousTimeGoodsMap.end(); it++) {
		if (now > it->second.start && now < it->second.end) {
			vec.push_back(&it->second);
		}
	}
}

std::string MysteriousCfg::getTimeGoodsDesc()
{
    return mTimeGoodsDesc;
}

bool MysteriousCfg::isUseViplvlRefreshLimit() const
{
    return mIsUseViplvlRefreshLimit;
}

const MysteriousExchangeDef * MysteriousCfg::getExchangeGoods(int index)
{
	MysteriousExchangeGoodsTd::iterator it = mMysteriousExchangeGoodsMap.find(index);
	if (it == mMysteriousExchangeGoodsMap.end()) {
		return NULL;
	}
	return &it->second;
}

vector<MysteriousExchangeDef *> MysteriousCfg::getPropsExchangeByLvl(int lvl,int time)
{
    mMysteriousExchangePropsVec.clear();
    MysteriousExchangeGoodsTd::iterator it;
    if (mExchangeStart <= time && mExchangeEnd >= time) { //在活动时间内，所有活动都显示
        for (it = mMysteriousExchangeGoodsMap.begin(); it != mMysteriousExchangeGoodsMap.end(); it++) {
            if (it->second.lvlMax< lvl) { // 不能购买商品等级区间上限小于主角等级的商品
                continue;
            }
            mMysteriousExchangePropsVec.push_back(&it->second);
        }
    }else{
        for (it = mMysteriousExchangeGoodsMap.begin(); it != mMysteriousExchangeGoodsMap.end(); it++) {
            if (it->second.lvlMax< lvl) {
                continue;
            }
            if(it->second.limit_num == 0){
              mMysteriousExchangePropsVec.push_back(&it->second);
            }
        }

    
    }
    
    return mMysteriousExchangePropsVec;
}


void MysteriousCfg::getExchangeGoods(vector<MysteriousExchangeDef*> &vec)
{
	int now = time(NULL);
	MysteriousExchangeGoodsTd::iterator it;
	for (it = mMysteriousExchangeGoodsMap.begin(); it != mMysteriousExchangeGoodsMap.end(); it++) {
		if (now > it->second.start && now < it->second.end) {
			vec.push_back(&it->second);
		}
	}
}

int MysteriousCfg::getPropsExchangeDataSize()
{
    return mMysteriousExchangePropsVec.size();
}

vector<MysteriousTimeGoodsDef *>  MysteriousCfg::getMyMysteriousTimeGoodsInLimitTime(int time){
    vector<MysteriousTimeGoodsDef *> mysteriousTimeGoodsDefVec;
    MysteriousTimeGoodsTd::iterator it;
    if (mTimeStart <=time && time<=mTimeEnd ) {  //在活动时间内，所有活动都显示
        for (it = mMysteriousTimeGoodsMap.begin(); it!= mMysteriousTimeGoodsMap.end(); it++) {
             mysteriousTimeGoodsDefVec.push_back(&it->second);
        }
    }else {
        for (it = mMysteriousTimeGoodsMap.begin(); it!= mMysteriousTimeGoodsMap.end(); it++) {
            if(it->second.limit_num == 0){      //在活动时间外，只显示非限时活动
              mysteriousTimeGoodsDefVec.push_back(&it->second);
            }
        }
    }
    return mysteriousTimeGoodsDefVec;
}


bool MysteriousCfg::checkSpecialTm()
{
	time_t tm = time(NULL);
	if (mTimeStart < tm && tm < mTimeEnd) {
		return true;
	}
	return false;
}

bool MysteriousCfg::checkExchangeTm()
{
	time_t tm = time(NULL);
	if (mExchangeStart < tm && tm < mExchangeEnd) {
		return true;
	}
	return false;
}

bool MysteriousCfg::specialIsDelay(int tm)
{
	if (tm != mTimeEnd && mTimeIsDelay == 0) {
		return true;
	}
	return false;
}

bool MysteriousCfg::exchangeIsDelay(int tm)
{
	if (tm != mExchangeEnd && mExchangeIsDelay == 0) {
		return true;
	}
	return false;
}

MysteriousCfg MysteriousCfgMgr::sCfg;
void MysteriousCfgMgr::load()
{
	sCfg.read();
}


#pragma mark - 每日答题

std::map<int , Question> AnswerActivityCfg::_all_questions_map;
std::vector<int> AnswerActivityCfg::_easy_questions_id;
std::vector<int> AnswerActivityCfg::_hard_questions_id;
int AnswerActivityCfg::_max_questions_count;       //抽得题目个数
std::vector<int> AnswerActivityCfg::_cost_rmb;   //换题所需金钻
int AnswerActivityCfg::_max_change_count;   //最大换题次数
int AnswerActivityCfg::_easy_question_count;//简单题目个数
int AnswerActivityCfg::_hard_question_count;//困难题目个数
int AnswerActivityCfg::_answer_time; //答题时间
vector<ScoreItem> AnswerActivityCfg::_score_rewards; //不同分数段奖励
map<int, TotalawardItem> AnswerActivityCfg::_totalaward_map; //连续奖励

void AnswerActivityCfg::load(const std::string &questionsFilePath, const std::string &answerActivityFilePath)
{
    loadQuestionsFile(questionsFilePath);
    loadAnswerActivityFile(answerActivityFilePath);
}

void AnswerActivityCfg::loadQuestionsFile(const std::string &filepath)
{
    try
    {
        GameInifile inifile(filepath);
        _all_questions_map.clear();
        _easy_questions_id.clear();
        _hard_questions_id.clear();
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        for( std::list<string>::iterator it = sections.begin(); it!=sections.end(); it++)
        {
            Question question;
            question.id = inifile.getValueT(*it, "id", 0);
            question.answer = inifile.getValueT(*it, "answer", 0);
            question.difficulty = inifile.getValueT(*it, "difficulty", 1);
            question.score = inifile.getValueT(*it, "score", 0);
            question.desc = inifile.getValue(*it, "desc", "");
            question.selects.push_back(inifile.getValue(*it, "select1", ""));
            question.selects.push_back(inifile.getValue(*it, "select2", ""));
            question.selects.push_back(inifile.getValue(*it, "select3", ""));
            question.selects.push_back(inifile.getValue(*it, "select4", ""));
            
            //difficulty  难度 1表示简单 2表示困难
            _all_questions_map.insert(std::make_pair(question.id, question));
            if (question.difficulty == 1)
            {
                _easy_questions_id.push_back(question.id);
            }
            else if (question.difficulty == 2)
            {
                _hard_questions_id.push_back(question.id);
            }
        }
    }
    catch (...)
    {
    }
}



void AnswerActivityCfg::loadAnswerActivityFile(const std::string &filepath)
{
    try
    {
        GameInifile inifile(filepath);

        // 基本配置
        _cost_rmb = inifile.getTableT("root", "NeedRmb", 0);
        assert(_cost_rmb.size() > 0);
        vector<int> questions_count_temp = inifile.getTableT("root", "difficulty", 0);
        assert(questions_count_temp.size() == 2);
        
        _easy_question_count = questions_count_temp[0];
        _hard_question_count = questions_count_temp[1];
        _max_questions_count = _easy_question_count + _hard_question_count;
        
        _max_change_count = inifile.getValueT("root", "ChangeNum", 0);
        _answer_time = inifile.getValueT("root", "countdown", 0);
        
        // 不同分数段奖励值
        vector<string> keys;
        inifile.getKeys("award", inserter(keys, keys.begin()));
        assert(keys.size() > 0);
        
        for (vector<string>::iterator it = keys.begin(); it != keys.end(); it++)
        {
            StringTokenizer token(*it, "-");
            assert(token.count() == 2);
            int keys1 = Utils::safe_atoi(token[0].c_str());
            int keys2 = Utils::safe_atoi(token[1].c_str());
            string rewards = inifile.getValue("award", *it, "");
            _score_rewards.push_back(ScoreItem(keys1, keys2, rewards));
        }
        
        //连续答题奖励
        inifile.getKeys("totalaward", inserter(keys, keys.begin()));
        assert(keys.size() > 0);
        char strtmp[50];
        for (int i = 0; i < keys.size()/2; i++)
        {
            TotalawardItem tmp;
            snprintf(strtmp, 50, "award%d", i+1);
            tmp.awarditem = inifile.getValue("totalaward", strtmp);
            snprintf(strtmp, 50, "totalday%d", i+1);
            tmp.totaldays = atoi(inifile.getValue("totalaward", strtmp).c_str());
            snprintf(strtmp, 50, "desc%d", i+1);
            tmp.desc = inifile.getValue("totalaward", strtmp);
            _totalaward_map.insert(make_pair(i + 1, tmp));
        }
        
    }
    catch(...)
    {
    }
}

string AnswerActivityCfg::getScoreRewards(const int &score)
{
    vector<ScoreItem>::iterator iter = _score_rewards.begin();
    for(; iter != _score_rewards.end(); iter++)
    {
        if (score >= iter->minScore && score <= iter->maxScore)
        {
            return iter->rewards;
        }
    }
    //分数没有达到要求则返回空
    return string("");
}

Question AnswerActivityCfg::getQuestion(const int &id)
{
    std::map<int, Question>::iterator it = _all_questions_map.find(id);
    if (it == _all_questions_map.end())
    {
        Question tmp;
        return tmp;
    }
    return it->second;
}
//------------------------------------------------------------------
vector<string> AnswerActivityCfg::getQuestion4Selection(const int &id){
    Question tmp= getQuestion(id);
    return tmp.selects;
}

vector<string>  AnswerActivityCfg::randomQuestion4Selection(const int &id){
    
    vector<string> tmpVec;
    Question tmp= getQuestion(id);
    
    random_shuffle(tmp.selects.begin(), tmp.selects.end());
    std::copy(tmp.selects.begin(), tmp.selects.end(), back_inserter(tmpVec));
    return tmpVec;
}

int AnswerActivityCfg::findOriginalSelection(const int &id, string selection){
    Question tmp= getQuestion(id);
    vector<string>::iterator iter = find(tmp.selects.begin(), tmp.selects.end(), selection);
    if (iter == tmp.selects.end()) {
        return 0;
    }
    return std::distance(tmp.selects.begin(),iter)+1;//返回 选项 是第几个位置 (配置表从1开始)
}
//------------------------------------------------------------------

int AnswerActivityCfg::getAnswer(const int &id)
{
    std::map<int, Question>::iterator it = _all_questions_map.find(id);
    if (it == _all_questions_map.end())
    {
        return 0;
    }
    return it->second.answer;
}

vector<int> AnswerActivityCfg::getRandomQuestions()
{
    vector<int> id;
    vector<int> _easy_id = getEasyQuestionsId();
    vector<int> _hard_id = getHardQuestionsId();
    
    random_shuffle(_easy_id.begin(), _easy_id.end());
    random_shuffle(_hard_id.begin(), _hard_id.end());

    int easy_num = std::min((int)_easy_id.size(), _easy_question_count);
    int hard_num = std::min((int)_hard_id.size(), _hard_question_count);
    
    std::copy(_easy_id.begin(), _easy_id.begin() + easy_num, back_inserter(id));
    std::copy(_hard_id.begin(), _hard_id.begin() + hard_num, back_inserter(id));
    return id;
}
int AnswerActivityCfg::changeEasyQuestion(const vector<int> &easyId)
{
    vector<int> allEasyId = getEasyQuestionsId();
    random_shuffle(allEasyId.begin(), allEasyId.end());
    vector<int>::const_iterator findTemp;
    for (vector<int>::iterator it = allEasyId.begin(); it!= allEasyId.end(); it++)
    {
        findTemp = std::find(easyId.begin(), easyId.end(), *it);
        if (findTemp != easyId.end())
        {
            continue;
        }
        return *it;
    }
    return 0;
}

int AnswerActivityCfg::chargeHardQuestion(const vector<int> &hardId)
{
    vector<int> allHardId = getHardQuestionsId();
    random_shuffle(allHardId.begin(), allHardId.end());
    vector<int>::const_iterator findTemp;
    for (vector<int>::iterator it = allHardId.begin(); it!= allHardId.end(); it++)
    {
        findTemp = std::find(hardId.begin(), hardId.end(), *it);
        if (findTemp != hardId.end())
        {
            continue;
        }
        return *it;
    }
    return 0;
}

int AnswerActivityCfg::chargeQuestion(const int &id, const vector<int> &vec)
{
    Question temp = getQuestion(id);
    if (temp.difficulty == 1)
    {
        return changeEasyQuestion(vec);
    }
    else if (temp.difficulty == 2)
    {
        return chargeHardQuestion(vec);
    }
    return 0;
}

bool AnswerActivityCfg::isRightAnswer(const int &questionId, const int &roleAnswer)
{
    map<int, Question>::iterator iter = _all_questions_map.find(questionId);
    if (iter == _all_questions_map.end())
    {
        return false;
    }
    if (roleAnswer == iter->second.answer)
    {
        return true;
    }
    return false;
}

int AnswerActivityCfg::getQuestionScore(const int &id)
{
    std::map<int, Question>::iterator it = _all_questions_map.find(id);
    if (it == _all_questions_map.end())
    {
        return 0;
    }
    return it->second.score;
}

int AnswerActivityCfg::getChangeQuestionCost(int num)
{
    if (_cost_rmb.empty()) {
        return 0;
    }
    if (num < 0 || num >= _cost_rmb.size()) {
        return *(_cost_rmb.rbegin());
    }
    return _cost_rmb[num];
}

TotalawardItem AnswerActivityCfg::getTotalawardItem(const int &key)
{
    map<int, TotalawardItem>::iterator iter = _totalaward_map.find(key);
    if (iter == _totalaward_map.end())
    {
        TotalawardItem tmp;
        return tmp;
    }
    return iter->second;
}

TemplateCfg::SecKV TemplateCfg::_map;
void TemplateCfg::load(const string &filepath)
{
    GameInifile ini(filepath);
    vector<string> secs;
    vector<string> keys;
    ini.getSections(back_inserter(secs));
    for (vector<string>::iterator iter = secs.begin(); iter != secs.end(); ++iter)
    {
        keys.clear();
        ini.getKeys(*iter, back_inserter(keys));
        for (vector<string>::iterator it = keys.begin(); it != keys.end(); ++it)
        {
            string tmp = ini.getValue(*iter, *it);
            _map[*iter][*it] = tmp;
        }
    }
}

string TemplateCfg::getValue(const string &section, const string &key)
{
    SecKVIterator iter = _map.find(section);
    if (iter == _map.end())
    {
        return string("");
    }
    KVIterator it = iter->second.find(key);
    if (it == iter->second.end())
    {
        return string("");
    }
    return it->second;
}

MassFatLimitCfg::Map MassFatLimitCfg::mLimitMap;
MassFatLimitCfg::Map MassFatLimitCfg::mExtLimitMap;
void MassFatLimitCfg::load(const string &filepath)
{
    GameInifile ini(filepath);
    //1-1=50
    vector<string> keys;
    ini.getKeys("VipFatRecoveryLimit", back_inserter(keys));
    assert(keys.size() != 0);
    
    for (vector<string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        vector<string> tmp = StrSpilt(*iter, ",");
        assert(tmp.size() != 2);
        int val = ini.getValueT("VipFatRecoveryLimit", *iter, 0);
        if (val <= 0) val = 0;
        mLimitMap.insert(make_pair(Utils::safe_atoi(tmp[1].c_str()), val));
    }
    
    keys.clear();
    
    ini.getKeys("ExtVipFatRecoveryLimit", back_inserter(keys));
    assert(keys.size() != 0);
    for (vector<string>::iterator iter = keys.begin(); iter != keys.end(); ++iter)
    {
        vector<string> tmp = StrSpilt(*iter, ",");
        assert(tmp.size() != 2);
        int val = ini.getValueT("ExtVipFatRecoveryLimit", *iter, 0);
        if (val <= 0) val = 0;
        mExtLimitMap.insert(make_pair(Utils::safe_atoi(tmp[1].c_str()), val));
    }
}

int MassFatLimitCfg::getFatLimitByViplvl(const int &viplvl, const int &def)
{
    for (MapIterator iter = mLimitMap.begin(); iter != mLimitMap.end(); ++iter)
    {
        if (viplvl <= iter->second)
        {
            return iter->second;
        }
    }
    log_info("没有找到vip等级对应的精力恢复数！");
    return def;
}

int MassFatLimitCfg::getExtFatLimitByViplvl(const int &viplvl, const int &def)
{
    for (MapIterator iter = mExtLimitMap.begin(); iter != mExtLimitMap.end(); ++iter)
    {
        if (viplvl <= iter->second)
        {
            return iter->second;
        }
    }
    log_info("没有找到vip等级对应的额外精力恢复数！");
    return def;
}


//装备数值转换
struct trans
{
    int operator()(const string &str)
    {
        return Utils::safe_atoi(str.c_str());
    }
};
void strvec2Numvec(vector<string> &strvec, vector<int> &numvec)
{
    numvec.resize(strvec.size());
    transform(strvec.begin(), strvec.end(), numvec.begin(), trans());
}

typedef vector<string> Tokens;
EquipChangeCfg::TopMap EquipChangeCfg::mData;
void EquipChangeCfg::load(const string &filepath)
{
    mData.clear();
    
    GameInifile ini(filepath);
    vector<string> sections;
    ini.getSections(back_inserter(sections));
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
    {
        //人物等级区间
        Tokens lvltokens = StrSpilt(ini.getValue(*iter, "initial_lvl"), "-");
        assert(lvltokens.size() == 2);

        //武器等级和品质颜色
        int init_lvl = Utils::safe_atoi(lvltokens[1].c_str());
        int initial_qua = ini.getValueT(*iter, "initial_qua", 0);
        assert(initial_qua != 0);
        
        
        Tokens qua = StrSpilt(ini.getValue(*iter, "qua"), ";");
        Tokens star = StrSpilt(ini.getValue(*iter, "star"), ";");
        
        EquipChangeMappingTable table;
        
        table.level = ini.getValueT(*iter, "lvl", 0);
        assert(table.level != 0);
        
        strvec2Numvec(qua, table.quality);
        strvec2Numvec(star, table.star);
        
        assert(table.quality.size() == table.star.size());
        assert(table.quality.size() == (init_lvl + 1));
        
        //已经有相同等级了
        TopMapIterator topIter = mData.find(init_lvl);
        if (topIter != mData.end()) //找到了相同等级则插入
        {
            topIter->second.insert(make_pair(initial_qua, table));
        }
        else
        {
            Map mymap;
            mymap.insert(make_pair(initial_qua, table));
            mData.insert(make_pair(init_lvl, mymap));
        }
    }
}

int EquipChangeCfg::getNewEquipQua(const int &roleLvl, const int &oldQua, const int &equipStar)
{
    for (TopMapIterator iter = mData.begin(); iter != mData.end(); ++iter)
    {
        if (roleLvl <= iter->first)
        {
            MapIterator quaIter = iter->second.find(oldQua);
            if (quaIter == iter->second.end())
            {
                break;
            }
            return quaIter->second.quality[equipStar];
        }
    }
    log_warn("没有找到对应武器的新品质！");
    return 0;
}

int EquipChangeCfg::getNewEquipStar(const int &roleLvl, const int &oldQua, const int &equipStar)
{
    for (TopMapIterator iter = mData.begin(); iter != mData.end(); ++iter)
    {
        if (roleLvl <= iter->first)
        {
            MapIterator quaIter = iter->second.find(oldQua);
            if (quaIter == iter->second.end())
            {
                break;
            }
            return quaIter->second.star[equipStar];
        }
    }
    log_warn("没有找到对应星级武器的新星级！");
    return 0;
}

int EquipChangeCfg::getNewEquipLvl(const int &roleLvl, const int &oldQua, const int &equipStar)
{
    for (TopMapIterator iter = mData.begin(); iter != mData.end(); ++iter)
    {
        if (roleLvl <= iter->first)
        {
            MapIterator quaIter = iter->second.find(oldQua);
            if (quaIter == iter->second.end())
            {
                break;
            }
            return quaIter->second.level;
        }
    }
    log_warn("没有找到对应星级武器的新等级！");
    return 0;
}

map<int, ItemToGoldDef> ItemToGoldCfg::mData;
void ItemToGoldCfg::load(const string &filepath)
{
    GameInifile ini(filepath);
    vector<string> sections;
    ini.getSections(back_inserter(sections));
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
    {
        ItemToGoldDef def;
        def.index = ini.getValueT(*iter, "index", 0);
        def.proto_id = ini.getValueT(*iter, "proto_id", 0);
        def.compensation_gold = ini.getValueT(*iter, "compensation_gold", 0);
        mData.insert(make_pair(def.proto_id, def));
    }
}

ItemToGoldDef *ItemToGoldCfg::getItemToGoldDef(const int &itemid)
{
    map<int, ItemToGoldDef>::iterator iter = mData.find(itemid);
    if (iter == mData.end())
    {
        return NULL;
    }
    return &(iter->second);
}

#pragma mark - 
#pragma mark 女王祝福
vector<QueenBlessTimeItem> QueenBlessCfg::mTime;
void QueenBlessCfg::load(const string &filepath)
{
    mTime.clear();
    
    GameInifile ini(filepath);
    
    vector<string> sections;
    ini.getSections(back_inserter(sections));
    
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
    {
        string starttime = ini.getValue(*iter, "start_time");
        string endtime = ini.getValue(*iter, "end_time");
        
        QueenBlessTimeItem item;

        
        item.index = ini.getValueT(*iter, "id", 0);
        assert(item.index > 0);
        
        string awardFats = ini.getValue(*iter, "award_fats");
        StringTokenizer awardFatToken(awardFats, ";");
        assert(awardFatToken.count() > 0);
        
        for (int i = 0; i < awardFatToken.count(); i++)
        {
            int val = Utils::safe_atoi(awardFatToken[i].c_str());
            item.award_fatVec.push_back(val);
        }

        sscanf(starttime.c_str(), "%d:%d", &item.starttime.tm_hour, &item.starttime.tm_min);
        sscanf(endtime.c_str(), "%d:%d", &item.endtime.tm_hour, &item.endtime.tm_min);
        
        mTime.push_back(item);
    }
}

QueenBlessTimeItem *QueenBlessCfg::isDuringTime(const time_t &nowtime)
{
    struct tm tmtime;
    localtime_r(&nowtime, &tmtime);
    for (TimeItemIterator iter = mTime.begin(); iter != mTime.end(); ++iter)
    {
        if (tmtime.tm_hour >= iter->starttime.tm_hour && tmtime.tm_hour <= iter->endtime.tm_hour)
        {
            if (tmtime.tm_hour == iter->starttime.tm_hour && tmtime.tm_min < iter->starttime.tm_min)
            {
                return NULL;
            }
            if (tmtime.tm_hour == iter->endtime.tm_hour && tmtime.tm_min > iter->endtime.tm_min)
            {
                return NULL;
            }
            return &(*iter);
        }
    }
    return NULL;
}

int QueenBlessCfg::getQueenBlessFat(const time_t &nowtime, int vipLvl)
{
    QueenBlessTimeItem   *queenBlessTimeItem = QueenBlessCfg::isDuringTime (nowtime);
    if(queenBlessTimeItem != NULL)
    {
        if (vipLvl >=0 && vipLvl < queenBlessTimeItem->award_fatVec.size())
        {
            return queenBlessTimeItem->award_fatVec[vipLvl];
        }
    }
    return 0;
}

int QueenBlessCfg::getRemainTime()
{
    time_t nowtime = time(NULL);
    struct tm tmtime;
    localtime_r(&nowtime, &tmtime);
    for (TimeItemIterator iter = mTime.begin(); iter != mTime.end(); ++iter)
    {
        if (iter->starttime.tm_hour - tmtime.tm_hour > 0 ||
            (iter->starttime.tm_hour - tmtime.tm_hour == 0 && iter->starttime.tm_min - tmtime.tm_min > 0))
        {
            int remain = 0;
            remain = (iter->starttime.tm_hour - tmtime.tm_hour - 1) * 3600 + (60 - tmtime.tm_min + iter->starttime.tm_min - 1) * 60 + (60 - tmtime.tm_sec);
            return remain;
        }
    }
    int remain = (24 - tmtime.tm_hour - 1)*3600 + (60 - tmtime.tm_min) * 60 + mTime[0].starttime.tm_hour * 3600 + (mTime[0].starttime.tm_min-1) * 60 +(60 - tmtime.tm_sec);
    return remain;
}

//
QueenBlessTimeItem *QueenBlessCfg::getTimeItemByIndex(const int &index)
{
    for (TimeItemIterator iter = mTime.begin(); iter != mTime.end(); ++iter)
    {
        if (index == iter->index)
        {
            return &(*iter);
        }
    }
    return NULL;
}

bool QueenBlessCfg::isActivityTime()
{
    time_t now = time(NULL);
    if (NULL == QueenBlessCfg::isDuringTime(now))
    {
        return false;
    }
    return true;
}


vector<QueenBlessTimeItem> QueenBlessCfg::getQueenBlessTimes()
{
    return  mTime;
}



int PetMergeCfg::mQuanum;
int PetMergeCfg::mMaxStage;
PetMergeCfg::Map PetMergeCfg::mData;
void PetMergeCfg::load(const string &filepath)
{
    GameInifile ini(filepath);
    mQuanum = ini.getValueT("common", "quanum", 0);
    assert(mQuanum != 0);
    mMaxStage = ini.getValueT("common", "maxstage", 0);
    assert(mMaxStage != 0);
    
    for (int i = 1; i <= mQuanum; ++i)
    {
        char section[32];
        snprintf(section, 32, "qua%d", i);
        
        PetMergeItem item;
        memset(&item, 0, sizeof(item));
        
        item.cost_gold = ini.getValueT(section, "cost_gold", 0);
        
        Tokens merge_uplvl_exp_arr = StrSpilt(ini.getValue(section, "merge_uplvl_exp"), ";");
        strVecToNumVec(merge_uplvl_exp_arr, item.merge_uplvl_exp);
        
        Tokens incr_stre_arr = StrSpilt(ini.getValue(section, "incr_stre"), ";");
        strVecToFloatVec(incr_stre_arr, item.incr_stre);
        
        Tokens incr_inte_arr = StrSpilt(ini.getValue(section, "incr_inte"), ";");
        strVecToFloatVec(incr_inte_arr, item.incr_inte);
        
        Tokens incr_phys_arr = StrSpilt(ini.getValue(section, "incr_phys"), ";");
        strVecToFloatVec(incr_phys_arr, item.incr_phys);
        
        Tokens incr_capa_arr = StrSpilt(ini.getValue(section, "incr_capa"), ";");
        strVecToFloatVec(incr_capa_arr, item.incr_capa);

        //断言
        assert(item.merge_uplvl_exp.size() == mMaxStage);
        assert(item.incr_capa.size() == mMaxStage);
        assert(item.incr_inte.size() == mMaxStage);
        assert(item.incr_phys.size() == mMaxStage);
        assert(item.incr_stre.size() == mMaxStage);
        
        mData[i] = item;
    }
    
}

PetMergeItem *PetMergeCfg::getPetMergeItem(const int &pet_qua)
{
    MapIterator iter = mData.find(pet_qua);
    if (iter == mData.end())
    {
        return NULL;
    }
    return &(iter->second);
}

PetMergeLvlItem PetMergeCfg::getPetMergeLvlItem(const int &pet_qua, const int &merge_lvl)
{
    PetMergeLvlItem item;
    memset(&item, 0, sizeof(item));
    if (merge_lvl <= 0 || merge_lvl > mMaxStage)
    {
        return item;
    }
    
    PetMergeItem *mergeItem = PetMergeCfg::getPetMergeItem(pet_qua);
    if (!mergeItem)
    {
        return item;
    }
    
    item.incr_stre = mergeItem->incr_stre[merge_lvl-1];
    item.incr_phys = mergeItem->incr_phys[merge_lvl-1];
    item.incr_inte = mergeItem->incr_inte[merge_lvl-1];
    item.incr_capa = mergeItem->incr_capa[merge_lvl-1];
    
    return item;
}

int PetMergeCfg::getMaxStage()
{
    return mMaxStage;
}

int PetMergeCfg::getUplvlExp(const int &pet_qua, const int &lvl)
{
    if (lvl <= 0 || lvl > mMaxStage)
    {
        return 0;
    }
    PetMergeItem *mergeItem = PetMergeCfg::getPetMergeItem(pet_qua);
    if (!mergeItem)
    {
        return 0;
    }
    return mergeItem->merge_uplvl_exp[lvl - 1];
}

int PetMergeCfg::getUplvlTotalExp(const int &pet_qua, const int &lvl)
{
    if (lvl <= 0 || lvl > mMaxStage)
    {
        return 0;
    }
    PetMergeItem *mergeItem = PetMergeCfg::getPetMergeItem(pet_qua);
    if (!mergeItem)
    {
        return 0;
    }
    
    int i = 1;
    int totalexp = 0;
    for (vector<int>::iterator iter = mergeItem->merge_uplvl_exp.begin();
         iter != mergeItem->merge_uplvl_exp.end(); ++iter)
    {
        if (i > lvl)
        {
            break;
        }
        totalexp += *iter;
        ++i;
    }
    return totalexp;
}

void PetMergeCfg::getUplvlByExp(const int &pet_qua, int &petlvl, const int &exp, int &remain_exp)
{
    PetMergeItem *mergeItem = PetMergeCfg::getPetMergeItem(pet_qua);
    if (!mergeItem)
    {
        return ;
    }
    
    int needexp = 0;
    for (int i = petlvl + 1; i <= mMaxStage; ++i)
    {
        int tmpexp = PetMergeCfg::getUplvlExp(pet_qua, i);
        needexp += tmpexp;
        if (exp - needexp < 0)
        {
            petlvl = i - 1;
            remain_exp = exp - needexp + tmpexp;
            return ;
        }
    }
    remain_exp = 0;
    
    petlvl = mMaxStage;
}



#pragma mark - 
#pragma mark 扭蛋机
bool GashaponMachinesCfg::mIsOpen = false;
int GashaponMachinesCfg::mQuaCount;
GashaponMachinesCfg::Map GashaponMachinesCfg::mQuaItems;
GashaponMachinesCfg::ItemMap GashaponMachinesCfg::mAwardItems;
vector<int> GashaponMachinesCfg::mTimeNumProp;


void GashaponMachinesCfg::load(const string &cfg_filepath, const string award_filepath)
{
    
    // read cfg_filepath
    {
        GameInifile ini(cfg_filepath);
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("GashaponMachinesMultiLanguage.ini") == "GashaponMachinesMultiLanguage.ini"))
        {
            MLIniFile = GameInifile("GashaponMachinesMultiLanguage.ini");
        }
        //GameInifile MLIniFile(Cfgable::getFullFilePath("GashaponMachinesMultiLanguage.ini"));
        mIsOpen = (bool)ini.getValueT("is_open", "is_open", 0);
        if (!mIsOpen)
        {
            return ;
        }
		
		// 不同次数的权值
		mTimeNumProp.clear();
		strToNumVec(ini.getValue("prop", "prop").c_str(), ";", mTimeNumProp);
		assert(mTimeNumProp.size() > 0);
        
        
        mQuaCount = ini.getValueT("is_open", "qua_count", 0);
        assert(mQuaCount != 0);
        
        for (int i = 1; i <= mQuaCount; ++i)
        {
            string section = Utils::makeStr("qua%d", i);
            GashaponMachinesQuaItem item;
            item.refresh_time = ini.getValueT(section, "refresh_time", 0);
            item.everyday_free_times = ini.getValueT(section, "everyday_free_times", 0);
            item.sell_price = ini.getValueT(section, "sell_price", 0);
            item.buy_times_limit = ini.getValueT(section, "buy_limit", 0);

            vector<string> strvec = StrSpilt(ini.getValue(section, "viplvl_buy_limit", ""), ";");
            assert(strvec.empty() == false);
            strVecToNumVec(strvec, item.viplvl_buy_times_limit);
            
            vector<string> sostrvec = StrSpilt(ini.getValue(section, "special_offers"), ";");
            assert(sostrvec.size() == 2);
            vector<int> sonumvec;
            strVecToNumVec(sostrvec, sonumvec);
            item.special_offers_buy_count = sonumvec[0];
            item.special_offers_buy_present = sonumvec[1];
            
            item.icon = ini.getValue(section, "icon");
            string desecIdTmp = ini.getValue(section, "desc_id");
            //by lihengjin
            std::string str_desec;
            if(ini.exists(section, "desc"))
            {
                str_desec = ini.getValue(section, "desc");
            }else
            {
                str_desec = MLIniFile.getValue(desecIdTmp, "desc");
            }

            item.desc_id =  str_desec;
            
            item.is_struct_enable = true;
            item.qua_type = i;
            
            item.is_have_special_offers = ini.getValueT(section, "is_have_special_offers", 0);
            item.can_everyday_free_get = ini.getValueT(section, "can_everyday_free_get", 0);
            item.only_sell = ini.getValueT(section, "only_sell", 0);
            
            //insert item
            mQuaItems.insert(make_pair(i, item));
        }
    }
    
    // read award_filepath
    {
		int propSize = mTimeNumProp.size();
        GameInifile ini(award_filepath);		
        vector<string> sections;
        ini.getSections(back_inserter(sections));
        for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
        {
            GashaponMachinesAwardItem item;
            item.item_id = ini.getValueT(*iter, "item_id", 0);
            item.item_count = ini.getValueT(*iter, "item_count", 0);
			for (int i = 1; i <= propSize; i++) {
				string temp = xystr("prop%d", i);
				item.item_prop.push_back(ini.getValueT(*iter, temp, 0));
			}
            
            int quatype = ini.getValueT(*iter, "qua", 0);
            assert(quatype != 0);
            
            string lvl = ini.getValue(*iter, "lvl");
            int maxlvl = 0;
            sscanf(lvl.c_str(), "%*d-%d", &maxlvl);
            
            ItemMapIterator itemsiter = mAwardItems.find(quatype);
            if (itemsiter == mAwardItems.end()) //not found qua
            {
                map<int, vector<GashaponMachinesAwardItem> > m;
                vector<GashaponMachinesAwardItem> vec;
                vec.push_back(item);
                m.insert(make_pair(maxlvl, vec));
                mAwardItems.insert(make_pair(quatype, m));
            }
            else
            {
                map<int, vector<GashaponMachinesAwardItem> >::iterator it = itemsiter->second.find(maxlvl);
                if (it == itemsiter->second.end()) //not found lvl
                {
                    vector<GashaponMachinesAwardItem> vec;
                    vec.push_back(item);
                    itemsiter->second.insert(make_pair(maxlvl, vec));
                }
                else
                {
                    it->second.push_back(item);
                }
            }
        }
    }
}

bool GashaponMachinesCfg::isOpen()
{
    return mIsOpen;
}

int GashaponMachinesCfg::getQuaCount()
{
    return mQuaCount;
}

GashaponMachinesCfg::Map &GashaponMachinesCfg::getQuaItems()
{
    return mQuaItems;
}

GashaponMachinesCfg::ItemMap &GashaponMachinesCfg::getQuaAwardItems()
{
    return mAwardItems;
}

GashaponMachinesQuaItem GashaponMachinesCfg::getGashaponMachinesQuaItem(const int &qua_type)
{
    MapIterator iter = mQuaItems.find(qua_type);
    if (iter == mQuaItems.end())
    {
        return GashaponMachinesQuaItem();
    }
    return iter->second;
}

int GashaponMachinesCfg::getViplvlBuyTimesLimit(const int &qua_type, const int &viplvl)
{
    if (viplvl < 0 || viplvl > 10)
    {
        return 0;
    }
    MapIterator iter = mQuaItems.find(qua_type);
    if (iter == mQuaItems.end())
    {
        return 0;
    }
    return iter->second.viplvl_buy_times_limit[viplvl];
}

vector<GashaponMachinesAwardItem> GashaponMachinesCfg::getGashaponMachinesAwardItemByLvl(const int &qua_type, const int &lvl)
{
    vector<GashaponMachinesAwardItem> vec;
    ItemMapIterator iter = mAwardItems.find(qua_type);
    if (iter == mAwardItems.end())
    {
        return vector<GashaponMachinesAwardItem>();
    }
    for (map<int, vector<GashaponMachinesAwardItem> >::iterator it = iter->second.begin(); it != iter->second.end(); ++it)
    {
        if (lvl <= it->first)
        {
            return it->second;
        }
    }
    return vector<GashaponMachinesAwardItem>();
}

int GashaponMachinesCfg::getTimeNumIndex(int timeNum)
{
	int size = mTimeNumProp.size();
	
	for (int i = size - 1; i > 0; i--) {
		if (timeNum > mTimeNumProp[i]) {
			return i;
		}
	}
	return 0;
}

#pragma mark - 系统跑马灯公告

SystemBroadcastCfg::Map SystemBroadcastCfg::_broadcast_info;
void SystemBroadcastCfg::load(const string &filepath)
{
    //将配置的所有key value读到map中。
    GameInifile inifile(filepath);
    vector<string> sections;
    vector<string> keys;
    inifile.getSections(back_inserter(sections));
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
    {
        keys.clear();
        inifile.getKeys(*iter, back_inserter(keys));
        for (vector<string>::iterator it = keys.begin(); it != keys.end(); ++it)
        {
            string content = inifile.getValue(*iter, *it);
            //string tmp = inifile.getValue(*iter, *it);
            //string content = SystemBroadcastMLCfg::getValue(tmp, "desc");
            _broadcast_info[*iter][*it] = content;
        }
       
    }

    
}

string SystemBroadcastCfg::getValue(const string &section, const string &key)
{
    MapIterator iter = _broadcast_info.find(section);
    if (iter == _broadcast_info.end())
    {
        return string("");
    }
    map<string, string>::iterator it = iter->second.find(key);
    if (it == iter->second.end())
    {
        return string("");
    }
    return it->second;
}

#pragma mark - Game Guild log Text
string GameTextCfg::getFormatString(const char *id, ...)
{
    string fmtstr = getValue("GameText", id);
    if (fmtstr.empty())
    {
        return "";
    }
    va_list va;
    va_start(va, id);
    char buff[500] = "";
    bzero(buff, sizeof(buff));
    vsnprintf(buff, 500, fmtstr.c_str(), va);
    va_end(va);
    return buff;
}

string GameTextCfg::getString(const string &id)
{
    return getValue("GameText", id);
}








//--------------------------------------------------
#pragma mark - 图鉴
// 图鉴
std::map<int, IllustrationsDef*> IllustrationsCfg::mIllustrationsMap;
int IllustrationsCfg::mVer = 0;
void IllustrationsCfg::load(std::string fullPath, std::string configPath)
{
    try
    {
        mVer = 0;
        if (!configPath.empty()) {
            GameInifile configIniFile(configPath);
            mVer = configIniFile.getValueT("config", "config_id", 0);
        }
        
        
        GameInifile inifile(fullPath);
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("PetCollectMultiLanguage.ini") == "PetCollectMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("PetCollectMultiLanguage.ini"));
        }
        //GameInifile MLIniFile(Cfgable::getFullFilePath("PetCollectMultiLanguage.ini"));
        
        mIllustrationsMap.clear();
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            IllustrationsDef *def = new IllustrationsDef();
            
            def->templetId =inifile.getValueT(*it, "id", 0);
            string nameId = inifile.getValue(*it, "name_id");
            //by lihengjin
            std::string str_name;
            if(inifile.exists(*it, "name"))
            {
                str_name = inifile.getValue(*it, "name");
            }else
            {
                str_name = MLIniFile.getValue(nameId, "desc");
            }
            def->name = str_name;
            def->type = (IllustrationsType)inifile.getValueT(*it,"type",0);
            for(int i=0;i<3;i++)
            {
                string award = Utils::makeStr("rewards_%d",i+ 1);
                string singleReward = inifile.getValue(*it, award.c_str());
                def->singleRewardStr.push_back(singleReward);
                
                
                string awardInfor = Utils::makeStr("rewardsinfo_%d_id",i+1);
                string singleRewardId = inifile.getValue(*it, awardInfor);
                //by lihengjin
                string awardInfor_no_mul = Utils::makeStr("rewardsinfo_%d",i+1);
                string singleAwardInfor;
                if(inifile.exists(*it, awardInfor_no_mul))
                {
                    singleAwardInfor = inifile.getValue(*it, awardInfor_no_mul);
                }else
                {
                    singleAwardInfor = MLIniFile.getValue(singleRewardId, "desc");
                }

               // string singleAwardInfor = MLIniFile.getValue(singleRewardId, "desc");//inifile.getValue(*it,awardInfor.c_str());
                def->singleRewardInfor.push_back(singleAwardInfor);
                
                string getway = Utils::makeStr("dropinfo_%d_id",i+1);
                string singleGetWayId = inifile.getValue(*it, getway);
                //lihengjin
                string getway_no_mul = Utils::makeStr("dropinfo_%d", i+1);
                string singleGetWay;
                if(inifile.exists(*it, getway_no_mul))
                {
                    singleGetWay = inifile.getValue(*it, getway_no_mul);
                }else
                {
                    singleGetWay = MLIniFile.getValue(singleGetWayId, "desc");
                }
              //  string singleGetWay = MLIniFile->getValue(singleGetWayId, "desc");//inifile.getValue(*it, getway.c_str());
                def->singleGetWay.push_back(singleGetWay);
            }
            def->allRewardStr = inifile.getValue(*it, "rewards_4");
            string allRewardInfoID = inifile.getValue(*it, "rewardsinfo_4_id");
            //by lihengjin
            std::string str_allReward;
            if(inifile.exists(*it, "rewardsinfo_4"))
            {
                str_allReward = inifile.getValue(*it, "rewardsinfo_4");
            }else
            {
                str_allReward = MLIniFile.getValue(allRewardInfoID, "desc");
            }

            def->allRewardInfor = str_allReward;//inifile.getValue(*it, "rewardsinfo_4");
            
            string perDescId = inifile.getValue(*it, "petinfo_id");
            //by lihengjin
            std::string str_perDesc;
            if(inifile.exists(*it, "petinfo"))
            {
                str_perDesc = inifile.getValue(*it, "petinfo");
            }else
            {
                str_perDesc = MLIniFile.getValue(perDescId, "desc");
            }
            def->petDescStr = str_perDesc;//inifile.getValue(*it, "petinfo");
           // def->attacktype = inifile.getValueT(*it, "attacktype", 0);
            
            mIllustrationsMap.insert(make_pair(def->templetId, def));
        }

        
    }
    catch(...)
    {
        
    }
}

bool IllustrationsCfg::isBagUpdateVer(int ver)
{
    if (ver == mVer) {
        return false;
    }
    return true;
}

string IllustrationsCfg::getIllQuaReward(int id, IllustrationsQuality qua)
{
    IllustrationsDef *def = getIllustrationsDef(id);
    if (NULL == def) {
        return "";
    }
    if (qua > eIllQua_Purple) {
        return def->allRewardStr;
    }
    return def->singleRewardStr[qua];
}

std::vector<IllustrationsDef *>  IllustrationsCfg::getIllustrationsDefsbyType(IllustrationsType type)
{
    vector<IllustrationsDef *> datas;
    datas.clear();
    std::map<int, IllustrationsDef*>::iterator it;
    for(it = mIllustrationsMap.begin(); it!= mIllustrationsMap.end(); it ++)
    {
        if((IllustrationsType)it->second->type == type)
        {
            datas.push_back(it->second);
        }
    }
    return datas;
}

IllustrationsDef* IllustrationsCfg::getIllustrationsDef(int id)
{
    std::map<int, IllustrationsDef*>::iterator it = mIllustrationsMap.find(id);
    if(it == mIllustrationsMap.end())
    {
        return NULL;
    }
    return it->second;
}


// 装备共鸣系统配置
vector<int> EquipResonanceCfg::mDeepenRanges;
std::map<int, EquipResonanceDef*> EquipResonanceCfg::mDeepenMap;
std::map<int, EquipResonanceDef*> EquipResonanceCfg::mQualityMap;

void EquipResonanceCfg::load(std::string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);
        //lihengjin
        GameInifile MLInifile;
        if(!(Cfgable::getFullFilePath("equip_resonanceMultiLanguage.ini") == "equip_resonanceMultiLanguage.ini"))
        {
            MLInifile = GameInifile(Cfgable::getFullFilePath("equip_resonanceMultiLanguage.ini"));
        }
        //GameInifile MLInifile(Cfgable::getFullFilePath("equip_resonanceMultiLanguage.ini"));
        mDeepenRanges.clear();
        mDeepenMap.clear();
        mQualityMap.clear();
        
        {
            string ranges = inifile.getValue("deepen", "ranges", "").c_str();
            StringTokenizer range(ranges, ";");
            assert(range.count() > 0);
        
            for (int i = 0; i < range.count(); i++)
            {
                int val = Utils::safe_atoi(range[i].c_str());
                mDeepenRanges.push_back(val);
            }
        
            for (int i = 1; i < mDeepenRanges.size(); i++)
            {
//                char tmp[128] = {0};
//                sprintf(tmp, "deepen%d", i);
                
                string tmp = Utils::makeStr("deepen%d", i);
                EquipResonanceDef* def = new EquipResonanceDef;
            
                def->setAtkup(inifile.getValueT(tmp, "atkup", 0));
                def->setDefup(inifile.getValueT(tmp, "defup", 0));
                def->setHpup(inifile.getValueT(tmp, "hpup", 0));
                def->setHitup(inifile.getValueT(tmp, "hitup", 0.0f));
                def->setDodgeup(inifile.getValueT(tmp, "dodgeup", 0.0f));
                def->setCriup(inifile.getValueT(tmp, "criup", 0.0f));
                std::string DescID = inifile.getValue(tmp, "desc_id");
                //by lihengjin
                std::string strDesc;
                if(inifile.exists(tmp, "desc"))
                {
                    strDesc = inifile.getValue(tmp, "desc");
                }else
                {
                    strDesc = MLInifile.getValue(DescID, "desc", "");
                }
                def->desc = strDesc;//inifile.getValue(tmp, "desc", "");
                
                mDeepenMap.insert(make_pair(i, def));
            }
        }
        
        {
            string ranges = inifile.getValue("quality", "ranges", "").c_str();
            StringTokenizer range(ranges, ";");      
            assert(range.count() > 0);
        
            for (int i = 1; i < range.count(); i++)
            {
//                char tmp[128] = {0};
//                sprintf(tmp, "quality%d", i);
                string tmp = Utils::makeStr("quality%d", i);
                EquipResonanceDef* def = new EquipResonanceDef;
            
                def->setAtkup(inifile.getValueT(tmp, "atkup", 0));
                def->setDefup(inifile.getValueT(tmp, "defup", 0));
                def->setHpup(inifile.getValueT(tmp, "hpup", 0));
                def->setHitup(inifile.getValueT(tmp, "hitup", 0.0f));
                def->setDodgeup(inifile.getValueT(tmp, "dodgeup", 0.0f));
                def->setCriup(inifile.getValueT(tmp, "criup", 0.0f));
                std::string DescID = inifile.getValue(tmp, "desc_id");
                //by lihengjin
                std::string strDesc;
                if(inifile.exists(tmp, "desc"))
                {
                    strDesc = inifile.getValue(tmp, "desc");
                }else
                {
                    strDesc = MLInifile.getValue(DescID, "desc", "");
                }
                def->desc = strDesc;    //inifile.getValue(tmp, "desc", "");
                mQualityMap.insert(make_pair(i, def));
            }
        }
    }
    catch(...)
    {
        
    }
}

void EquipResonanceCfg::getEquipResonanceCount(EquipResonanceDef &def, int deepen, int quality)
{
    int index = 0;
    for (int i = 0; i < mDeepenRanges.size(); i++)
    {
        if (deepen < mDeepenRanges[i])
        {
            index = i;
            break;
        }
    }	
    std::map<int, EquipResonanceDef*>::iterator it;
    for (it = mDeepenMap.begin(); it != mDeepenMap.end(); it++)
    {
        if(it->first > index) break;
        def += *it->second;
    }
      
    for (it = mQualityMap.begin(); it != mQualityMap.end(); it++)
    {
        if(it->first > quality) break;
        def += *it->second;
    }
}


EquipResonanceDef* EquipResonanceCfg::getDeepenDef(int deepen)
{
    int index = 0;
    for (int i = 0; i < mDeepenRanges.size(); i++)
    {
        if (deepen < mDeepenRanges[i])
        {
            index = i;
            break;
        }
    }
    std::map<int, EquipResonanceDef*>::iterator it = mDeepenMap.find(index);
    if(it == mDeepenMap.end())
    {
        return NULL;
    }
    
    return it->second;
}

EquipResonanceDef* EquipResonanceCfg::getQualityDef(int quality)
{
    std::map<int, EquipResonanceDef*>::iterator it = mQualityMap.find(quality);
    if(it == mQualityMap.end())
    {
        return NULL;
    }
    
    return it->second;
}
int EquipResonanceCfg::getDeepenMapSize()
{
    return  mDeepenMap.size();
}
int EquipResonanceCfg::getQualityMapSize()
{
    return mQualityMap.size();
}




int PromotionActivityDef::ReadInt(const char* propname, int def, bool *suc)
{
    string strpropname = propname;
    map<string, string>::iterator iter = props.find(strpropname);
    if (iter == props.end())
    {
        if (suc)
        {
            *suc = false;
        }
        return def;
    }
    if (suc)
    {
        *suc = true;
    }
    
    return Utils::safe_atoi(iter->second.c_str());
}

string PromotionActivityDef::ReadStr(const char* propname, string def, bool *suc)
{
    map<string, string>::iterator iter = props.find(propname);
    if (iter == props.end())
    {
        if (suc)
        {
            *suc = false;
        }
        return def;
    }
    if (suc)
    {
        *suc = true;
    }
    return iter->second;
}


map<int, PromotionActivityDef*> PromotionActivityCfg::mMap;
void PromotionActivityCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    mMap.clear();
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
    {
        PromotionActivityDef *def = new PromotionActivityDef();

        vector<string> keys;
        inifile.getKeys(*it, inserter(keys, keys.begin()));
        for (int k = 0; k < keys.size(); k++)
        {
            string value = inifile.getValue(*it, keys[k]);
            def->props.insert(make_pair(keys[k], value));
        }
        
        mMap.insert(make_pair(Utils::safe_atoi((*it).c_str()), def));
    }
}

PromotionActivityDef * PromotionActivityCfg::getDef(int id)
{
    map<int, PromotionActivityDef*>::iterator it = mMap.find(id);
    if (it == mMap.end())
    {
        return NULL;
    }
    return it->second;
}

int PvpCfg::getCoolDownTime(int times)
{
    if (mCooldownTime.size() < 1) {
        return 0;
    }
    
    if (times >= mCooldownTime.size()) {
        return mCooldownTime[mCooldownTime.size() - 1];
    }
    
    if (times < 0) {
        return mCooldownTime[0];
    }
    
    return mCooldownTime[times];
}

int PvpCfg::getFreshCost(int times)
{
    check_min(times, 0);
    if (mFreshCost.size() == 0) {
        return 0;
    }
    if (times >= mFreshCost.size()) {
        return mFreshCost[mFreshCost.size() - 1];
    }
    return mFreshCost[times];
}


#pragma mark - 分享奖励配置
string ShareAwardCfg::sfirstAward = "";
bool ShareAwardCfg::sIsWeekly = false;
string ShareAwardCfg::sWeeklyAward = "";
int ShareAwardCfg::sCycleTime = 0;
string ShareAwardCfg::sDailyAward = "";
void ShareAwardCfg::load(std::string fullPath)
{
	GameInifile inifile(fullPath);
	sfirstAward = inifile.getValue("root", "award_firstTime");
	sIsWeekly = inifile.getValueT("root", "is_weekly", false);
	sWeeklyAward = inifile.getValue("root", "award_weekly");
	sCycleTime = inifile.getValueT("root", "cycleTime", 0);
	sDailyAward = inifile.getValue("root", "award_daily");
}

#pragma mark -

//**********************************  pet  ***********************************//



PetCfgDef::PetCfgDef()
{
    setMaxlvl(0);
    setExtlvl(0);
    setMonsterTpltID(0);
    setQua(0);
    setExp(0);
    
    setCrip(0.0);
    setCri(0.0);
    
    setMinGrowthRefine(0);
    setMaxGrowthRefine(0);
    setNeedGrowthPill(0);
    setGrowthSuccFactor(0.0);
    setCollectqua(-1);
    setGrowthRate(0.0);
    
    setStarBorn(0);
    setStageBorn(0);
    setMaxbaseprop(0);
    setExtraProp(0.0);
    setExtraPropPoint(0);
    
    
    setStre(0);
    setInte(0);
    setPhys(0);
    setCapa(0);
    
    
    setPhystohp(0.0);
    setCapatodef(0.0);
    setIntetoatk(0.0);
    setStretoatk(0.0);
    
    setMergeExp(0);
    setExtlvlAdd(0);
    setMaxlvlAdd(0);
    setGrowthAdd(0);
    setTypeForPetMerger(0);

	setCollectID(0);
}

std::map<int, PetCfgDef*> PetCfg::cfg;
std::map<int, PetGrowth*> PetCfg::growthcfg;
std::map<int, PetExp*> PetCfg::expcfg;

PetStarCfg PetCfg::petStarCfg;
vector<PetStageQua> PetCfg::petStageCfg;

bool PetCfg::load(string petPath, string petGrowthPath,std::string petStarPath)
{
    try
    {
        GameInifile ini(petPath);
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("petMultiLanguage.ini") == "petMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("petMultiLanguage.ini"));
        }
        //GameInifile MLIniFile(Cfgable::getFullFilePath("petMultiLanguage.ini"));

        std::vector<string> sections;
        ini.getSections(std::back_inserter(sections));

        for (int i = 0; i < sections.size(); i++)
        {
            PetCfgDef* petcfgdef = new PetCfgDef;
//            int id = ini.getValueT(sections[i], "id", 0);
//            if(id == 104023)
//                printf("");
            petcfgdef->setQua(ini.getValueT(sections[i], "qua", 0));
            
             petcfgdef->setMaxlvl(ini.getValueT(sections[i], "maxlvl", 0));
            petcfgdef->setExtlvl( ini.getValueT(sections[i], "extlvl", 0));

            petcfgdef->setCri(ini.getValueT(sections[i], "cri", 0.0));
            petcfgdef->setCrip(ini.getValueT(sections[i], "crip", 0.0));
      //      petcfgdef->setType(ini.getValueT(sections[i], "type", 0));
            petcfgdef->setExp(ini.getValueT(sections[i], "exp", 0));
            string nameId = ini.getValue(sections[i], "name_id");
            //by lihengjin
            std::string str_name;
            if(ini.exists(sections[i], "name"))
            {
                str_name = ini.getValue(sections[i], "name");
            }else
            {
                str_name = MLIniFile.getValue(nameId, "desc");
            }
            petcfgdef->name = str_name;//ini.getValue(sections[i], "name", "");
            petcfgdef->setCollectqua(ini.getValueT(sections[i], "collectqua", -1));
            petcfgdef->setMonsterTpltID(ini.getValueT(sections[i], "monster_id", 0));
            
            
            petcfgdef->setGrowthRate(ini.getValueT(sections[i], "growth_rate", 0.0f));
            
            petcfgdef->setMaxbaseprop(ini.getValueT(sections[i], "maxbaseprop", 0.0f));

            petcfgdef->setExtraProp(ini.getValueT(sections[i], "extraprop", 0.0f));
            petcfgdef->setExtraPropPoint(ini.getValueT(sections[i], "extrapoint", 0));
            petcfgdef->setStretoatk(ini.getValueT(sections[i], "strtoatk", 0.0f));
            petcfgdef->setPhystohp(ini.getValueT(sections[i], "phystohp", 0.0f));
            petcfgdef->setIntetoatk(ini.getValueT(sections[i], "intetoatk", 0.0f));
            petcfgdef->setCapatodef(ini.getValueT(sections[i], "capatodef", 0.0f));
            
            petcfgdef->setStre(ini.getValueT(sections[i], "stre", 0));
            petcfgdef->setInte(ini.getValueT(sections[i], "inte", 0));
            petcfgdef->setPhys(ini.getValueT(sections[i], "phys", 0));
            petcfgdef->setCapa(ini.getValueT(sections[i], "capa", 0));
            
            petcfgdef->setMergeExp(ini.getValueT(sections[i], "merge_exp", 0));
            petcfgdef->setExtlvlAdd(ini.getValueT(sections[i], "extlvl_add", 0));
            petcfgdef->setMaxlvlAdd(ini.getValueT(sections[i], "maxlvl_add", 0));
            petcfgdef->setGrowthAdd(ini.getValueT(sections[i], "growth_add", 0));
            petcfgdef->setTypeForPetMerger(ini.getValueT(sections[i], "typeForPetMerger", 0));
			
			petcfgdef->setCollectID(ini.getValueT(sections[i], "collectid", 0));
            petcfgdef->setSpiritSpeakCoef(ini.getValueT(sections[i], "spiritspeak_coef", 0.0f));
            
            string growth = ini.getValue(sections[i], "growth");
            sscanf(growth.c_str(), "%d-%d", petcfgdef->growth, petcfgdef->growth + 1);
            string atk = ini.getValue(sections[i], "atk");
            sscanf(atk.c_str(), "%d-%d", petcfgdef->batk, petcfgdef->batk + 1);
            string def = ini.getValue(sections[i], "def");
            sscanf(def.c_str(), "%d-%d", petcfgdef->bdef, petcfgdef->bdef+ 1);
            string hp = ini.getValue(sections[i], "hp");
            sscanf(hp.c_str(), "%d-%d", petcfgdef->bhp, petcfgdef->bhp + 1);
            string dodge = ini.getValue(sections[i], "dodge");
            sscanf(dodge.c_str(), "%f-%f", petcfgdef->bdodge, petcfgdef->bdodge + 1);
            string hit = ini.getValue(sections[i], "hit");
            sscanf(hit.c_str(), "%f-%f", petcfgdef->bhit, petcfgdef->bhit + 1);


            int skillnum = ini.getValueT(sections[i], "skillnum", 0);
            for (int j = 0; j < skillnum; j++)
            {
//                char skillbuf[32];
//                char propbuf[32];
//                sprintf(skillbuf, "skill%d", j + 1);
//                sprintf(propbuf, "prop%d", j + 1);
                string skillbuf = Utils::makeStr("skill%d", j + 1);
                string propbuf = Utils::makeStr("prop%d", j + 1);
                string skillLockPropStr = xystr("skill_lock_prop%d",j + 1);

                int skillid = ini.getValueT(sections[i], skillbuf, 0);
                float prop = ini.getValueT(sections[i], propbuf, 0.0);
                float lockporp = ini.getValueT(sections[i], skillLockPropStr.c_str(), 0.0);

                petcfgdef->skills.push_back(skillid);
                petcfgdef->skillprops.push_back(prop);
                petcfgdef->skilllockprops.push_back(lockporp);
            }

            //升星
            petcfgdef->setStarBorn(ini.getValueT(sections[i], "starBorn", 0));

            StringTokenizer starUpStepToken(ini.getValue(sections[i], "starUpStep"), ";");
            for (int j = 0; j < starUpStepToken.count(); j++)
            {
                petcfgdef->starUpStep.push_back(atoi(starUpStepToken[j].c_str()));
            }

            StringTokenizer starUpSuccRatioToken(ini.getValue(sections[i], "starUpSuccRatio"), ";");
            for (int j = 0; j < starUpSuccRatioToken.count(); j++)
            {
                petcfgdef->starUpSuccRatio.push_back(atof(starUpSuccRatioToken[j].c_str()));
            }

            StringTokenizer starUpConsumeFactorToken(ini.getValue(sections[i], "starUpConsumeFactor"), ";");
            for (int j = 0; j < starUpConsumeFactorToken.count(); j++)
            {
                petcfgdef->starUpConsumeFactor.push_back(atoi(starUpConsumeFactorToken[j].c_str()));
            }
            
            //升阶
            petcfgdef->setStageBorn(ini.getValueT(sections[i], "stageBorn", 0));
            petcfgdef->totalStageUpStep = 0;
            StringTokenizer stageUpStepToken(ini.getValue(sections[i], "stageUpstep"), ";");
            for (int j = 0; j < stageUpStepToken.count(); j++)
            {
                int stepStage = Utils::safe_atoi(stageUpStepToken[j].c_str());
                
                petcfgdef->stageUpStep.push_back(stepStage);
                
                petcfgdef->totalStageUpStep += stepStage;
            }
            
            //可合体幻兽id列表
            StringTokenizer petMergeToken(ini.getValue(sections[i], "mergeList"), ";");
            for(int j = 0; j < petMergeToken.count(); j++)
            {
                petcfgdef->petMergeList.push_back(atoi(petMergeToken[j].c_str()));
            }
            
            petcfgdef->stageInherit = ini.getTableT(sections[i], "StageInherit", 0);
            
            assert(petcfgdef->stageUpStep.size() == petcfgdef->stageInherit.size());
            
            //成长洗练
            string growthlimit = ini.getValue(sections[i], "growth_limit");
            
            int minGrowthRefine;
            int maxGrowthRefine;
            sscanf(growthlimit.c_str(), "%d-%d", & minGrowthRefine, & maxGrowthRefine);
            
            petcfgdef->setMinGrowthRefine(minGrowthRefine);
            petcfgdef->setMaxGrowthRefine(maxGrowthRefine);
            petcfgdef->setNeedGrowthPill(ini.getValueT(sections[i], "need_pill", (int)INT_MAX));
            petcfgdef->setGrowthSuccFactor(ini.getValueT(sections[i], "growth_succfactor", 1.0f));
            
            petcfgdef->setDisplayInCity(ini.getValueT(sections[i], "display_in_city", 0));
            
            //吞噬属性成长改变
            vector<float>* propsPoints[2] = {&petcfgdef->lvlPoints, &petcfgdef->ownPoints};
            string keyNames[2] = {"lvlprop_points", "ownprop_points"};
            for (int j = 0; j < 2; j++)
            {
                vector<float>& propPoints = *propsPoints[j];
            
                string pointsStr = ini.getValue(sections[i], keyNames[j]);
                StringTokenizer token(pointsStr, ";");
                StringTokenizer::Iterator lastIter = token.end();
                for (StringTokenizer::Iterator iter = token.begin(); iter != lastIter; ++iter)
                {
                    float value = atof(iter->c_str());
                    propPoints.push_back(value);
                }
            }
            
            
            //加入数据

            cfg.insert(std::make_pair<int, PetCfgDef*>(atoi(sections[i].c_str()), petcfgdef));
        }

    }
    catch (...)
    {
        log_error("load pet cfg ini file failed");
        return false;
    }

    if( ! loadPetGrowCfg(petGrowthPath.c_str())){
        return false;
    }

    if( ! loadPetStarCfg(petStarPath.c_str())){
        return false;
    }
    
    if( ! loadPetStageCfg(petStarPath.c_str()))
    {
        return false;
    }



    return true;
}

//加载宠物成长
bool PetCfg::loadPetGrowCfg(const char* path)
{
    if(path == NULL){
        log_error("loadPetGrowCfg error");
        return false;
    }

    try
    {
        GameInifile ini(path);

        int quanum = ini.getValueT("common", "quanum", 0);
        int typenum = ini.getValueT("common", "typenum", 0);

        for (int i = 1; i <= typenum; i++)
        {
//            char buf[32];
//            sprintf(buf, "type%d", i);
            string buf = Utils::makeStr("type%d", i);

            PetGrowth* growth = new PetGrowth;
            
            growth->petTypeImage = ini.getValue(buf,"pettypeimage");
            StringTokenizer atktoken(ini.getValue(buf, "atk"), ";");
            for (int j = 0; j < atktoken.count(); j++)
            {
                growth->atk.push_back(atoi(atktoken[j].c_str()));
            }
            StringTokenizer deftoken(ini.getValue(buf, "def"), ";");
            for (int j = 0; j < deftoken.count(); j++)
            {
                growth->def.push_back(atoi(deftoken[j].c_str()));
            }
            StringTokenizer hptoken(ini.getValue(buf, "hp"), ";");
            for (int j = 0; j < hptoken.count(); j++)
            {
                growth->hp.push_back(atoi(hptoken[j].c_str()));
            }
            StringTokenizer dodgetoken(ini.getValue(buf, "dodge"), ";");
            for (int j = 0; j < dodgetoken.count(); j++)
            {
                growth->dodge.push_back(atof(dodgetoken[j].c_str()));
            }
            StringTokenizer hittoken(ini.getValue(buf, "hit"), ";");
            for (int j = 0; j < hittoken.count(); j++)
            {
                growth->hit.push_back(atof(hittoken[j].c_str()));
            }
            StringTokenizer critoken(ini.getValue(buf, "cri"), ";");
            for (int j = 0; j < critoken.count(); j++)
            {
                growth->cri.push_back(atof(critoken[j].c_str()));
            }

            growthcfg.insert(make_pair<int, PetGrowth*>(i, growth));
        }

        for (int i = 1; i <= quanum; i++)
        {
//            char buf[32];
//            sprintf(buf, "qua%d", i);
            
            string buf = Utils::makeStr("qua%d", i);

            PetExp* exp = new PetExp;
            StringTokenizer exptoken(ini.getValue(buf, "exp"), ";");
            for (int j = 0; j < exptoken.count(); j++)
            {
                exp->exp.push_back(atoi(exptoken[j].c_str()));
            }

            StringTokenizer totalexptoken(ini.getValue(buf, "total_exp"), ";");
            for (int j = 0; j < totalexptoken.count(); j++)
            {
                exp->totalExp.push_back(atoi(totalexptoken[j].c_str()));
            }
            
            exp->maxCommonSkillCount = ini.getValueT(buf, "max_commonskill", 0);
            exp->maxSpecialskillCount = ini.getValueT(buf, "max_specialskill", 0);
            expcfg.insert(make_pair<int, PetExp*>(i, exp));
        }
    }
    catch (...)
    {
        log_error("load pet growth cfg ini file failed");
        return false;
    }

    return true;
}

//加载宠物升阶参数
bool PetCfg::loadPetStageCfg(const char* path)
{
    if(path == NULL){
        log_error("loadPetStageCfg error");
        return false;
    }
    
    petStageCfg.clear();
    
    try{
        GameInifile ini(path);
        int quanum = ini.getValueT("common", "quanum", 0);
        for (int i = 1; i <= quanum; ++ i)
        {
            PetStageQua quastage;
            quastage.quality = i - 1;
            
            string quastr = xystr("qua%d",i);
            StringTokenizer propertyRatioToken(ini.getValue(quastr, "stage_property_ratio"), ";");
            for (int j = 0; j < propertyRatioToken.count(); ++j)
            {
                float val = atof(propertyRatioToken[j].c_str());
                quastage.stagePropertyRatio.push_back(val);
            }
            
            StringTokenizer stageUpConsumeFactorToken(ini.getValue(quastr, "stageUpConsumeFactor"), ";");
            for (int j = 0; j < stageUpConsumeFactorToken.count(); ++j)
            {
                int val = atoi(stageUpConsumeFactorToken[j].c_str());
                quastage.stageUpConsumeFactor.push_back(val);
            }
            petStageCfg.push_back(quastage);
            
        }
    }catch(...){
        log_error("load pet star cfg ini file failed");
        return false;
    }
    return  true;
    
}


//加载宠物升星参数
bool PetCfg::loadPetStarCfg(const char* path)
{
    if(path == NULL){
        log_error("loadPetStarCfg error");
        return false;
    }

    try{
        GameInifile ini(path);
        int quanum = ini.getValueT("common", "quanum", 0);
        for (int i = 1; i <= quanum; ++ i) {

            do{
//                char buf[32];
//                sprintf(buf, "qua%d", i);
                string buf = Utils::makeStr("qua%d", i);
                vector<float> percent;

                StringTokenizer exptoken(ini.getValue(buf, "percent_conversion"), ";");
                for (int j = 0; j < exptoken.count(); j++)
                {
                    percent.push_back(atof(exptoken[j].c_str()));
                }
                petStarCfg.percentConversion.push_back(percent);
            }while(0);

            do{
//                char buf[32];
//                sprintf(buf, "qua%d", i);
                
                string buf = Utils::makeStr("qua%d", i);
                vector<float> vec;

                StringTokenizer exptoken(ini.getValue(buf, "property_ratio"), ";");
                for (int j = 0; j < exptoken.count(); j++)
                {
                    vec.push_back(atof(exptoken[j].c_str()));
                }
                petStarCfg.propertyRatio.push_back(vec);
            }while(0);
        }
    }catch(...){
        log_error("load pet star cfg ini file failed");
        return false;
    }
    return  true;
}

PetCfgDef* PetCfg::getCfg(int pettypeid)
{
    
    map<int, PetCfgDef*>::iterator iter = cfg.find(pettypeid);

    if (iter == cfg.end())
    {
        return NULL;
    }
  //  CCLog("pettypeid:%d,iter->second->skills.size:%x",pettypeid,iter->second->skills.size());
    return iter->second;
}

PetGrowthVal PetCfg::getGrowthVal(int type, int lvl)
{
    PetGrowthVal growthval;
    memset(&growthval, 0, sizeof(growthval));
    map<int, PetGrowth*>::iterator iter = growthcfg.find(type);
    if (iter != growthcfg.end())
    {
        PetGrowth* growth = iter->second;

        growthval.atk = growth->atk[lvl];
        growthval.def = growth->def[lvl];
        growthval.hp = growth->hp[lvl];
        growthval.dodge = growth->dodge[lvl];
        growthval.hit = growth->hit[lvl];
        growthval.cri = growth->cri[lvl];
    }

    return growthval;
}


int PetCfg::getLvlExp(int qua, int lvl)
{
    map<int, PetExp*>::iterator iter = expcfg.find(qua);
    if (iter == expcfg.end())
    {
        return -1;
    }

    return iter->second->exp[lvl];
}

int PetCfg::getLvlTotalExp(int qua, int lvl)
{
    map<int, PetExp*>::iterator iter = expcfg.find(qua);
    if (iter == expcfg.end())
    {
        return -1;
    }
    if (lvl >= iter->second->totalExp.size())
    {
        return iter->second->totalExp[iter->second->totalExp.size() - 1];  //要是没有配置更多宠物等级的EXP则取最后的经验。
    }
    return iter->second->totalExp[lvl];
}
int PetCfg::getMaxCommmonSkills(int qua)
{
    map<int, PetExp*>::iterator iter = expcfg.find(qua);
    if(iter == expcfg.end())
    {
        return -1;
    }
    return iter->second->maxCommonSkillCount; 
}
int PetCfg::getMaxSpecialSkills(int qua)
{
    map<int, PetExp*>::iterator iter = expcfg.find(qua);
    if(iter == expcfg.end())
    {
        return -1;
    }
    return iter->second->maxSpecialskillCount;
}
// 注意，配置表中是从1开始的
float PetCfg::getPercentConversion(int qua,int starlvl)
{
    if(qua <0 || qua >= petStarCfg.percentConversion.size()){
        log_error("getPercentConversion error , convert 100%");
        return 1;
    }

    if( starlvl <0 || starlvl >= petStarCfg.percentConversion[qua].size()){
        log_error("getPercentConversion error , convert 100%");
        return 1;
    }

    float ret = petStarCfg.percentConversion[qua][starlvl];
    if(ret > 1 || ret < 0){
        log_error("getPercentConversion error ,conver 100%");
        return 1;
    }
    return ret;
}

float PetCfg::getPropertyRatio(int qua,int starlvl)
{
    if(qua <0 || qua >= petStarCfg.propertyRatio.size()){
        log_error("getPropertyRatio error , Ratio 100%");
        return 1;
    }

    if( starlvl <0 || starlvl >= petStarCfg.propertyRatio[qua].size()){
        log_error("getPropertyRatio error , Ratio 100%");
        return 1;
    }

    float ret = petStarCfg.propertyRatio[qua][starlvl];
    if(ret < 0){
        log_error("getPropertyRatio error ,Ratio 100%");
        return 1;
    }
    return ret;
}

//升阶消耗金币系数// 注意，配置表中是从1开始的
int   PetCfg::getStageUpConsumeFactor(int qua,int stage)
{
    if(qua< 0 || stage<0 ||  qua >= petStageCfg.size())
    {
        log_error("getStageUpConsumeFactor error , use INT_MAX , qua ="<<qua<<" stage ="<<stage);
        return INT_MAX;
    }
    
    int index = std::min(stage, (int)petStageCfg[qua].stageUpConsumeFactor.size()-1);
    int ret = petStageCfg[qua].stageUpConsumeFactor[index];
    if(ret <0)
    {
        log_error("getStageUpConsumeFactor error ,use INT_MAX");
        return INT_MAX;
    }
    return ret;
}

//升阶战斗力加成
float PetCfg::getStagePropertyRatio(int qua,int stage)
{
    if(qua< 0 || stage<0 ||  qua >= petStageCfg.size())
    {
        log_error("getStagePropertyRatio error , use 1.0f , qua ="<<qua<<" stage ="<<stage);
        return 1.0f;
    }
    
    int index = std::min(stage, (int)petStageCfg[qua].stagePropertyRatio.size()-1);
    float ret = petStageCfg[qua].stagePropertyRatio[index];
    if(ret <0)
    {
        log_error("getStagePropertyRatio error ,use 1.0f");
        return 1.0f;
    }
    return ret;
}

int PetCfg::getMaxStageLvl(int qua)
{
   // return petStarCfg.propertyRatio[qua].size();
    return petStageCfg[qua].stagePropertyRatio.size() - 1;
}
//被吞宠物数量判断
bool PetCfg::checkAbsorbAmount(int amount)
{
    if(1<= amount && amount <= 6)
        return true;
    return false;
}
std::string PetCfg::getPetGrowthTypeImage(int petType)
{
    std::map<int, PetGrowth*>::iterator iter = growthcfg.find(petType);
    if(iter != growthcfg.end())
        return iter->second->petTypeImage;
    return "";
}

int PetCfg::calcAbsorbedPropPoints(int absorbedProp, const vector<float>& ownPoints, int mainProp, const vector<float>& lvlPoints)
{
    //absorbedProp 被吞噬宠物 单项属性值
    //mainProp     宠物 单项属性值
    //propPoints   属性点数
    //ownPoints    被吞噬宠物 提供的 属性点
    //lvlPoints    主宠物  升级所需 属性点
    if (absorbedProp <= 0)
    {
        return 0;
    }
    
    int newPoints = 0;
    float propPoints = absorbedProp < ownPoints.size() ? ownPoints[absorbedProp-1] : ownPoints.back();
    
    int i;
    for (i = mainProp; i < lvlPoints.size() && propPoints > 0; i++)
    {
        if (propPoints >= lvlPoints[i])
        {
            newPoints++;
            propPoints -= lvlPoints[i];
        }
    }
    
    if (propPoints > 0)
    {
        float rate = propPoints / lvlPoints[i];
        if (rate > range_randf(0, 1))
        {
            newPoints++;
        }
    }
    
    return newPoints;
}

float PetCfg::calcAbsorbedPropPointsNum(int absorbedProp, const vector<float> &ownPoints)
{
    if (absorbedProp <=0) {
        return 0;
    }
    return absorbedProp< ownPoints.size()? ownPoints[absorbedProp-1]:ownPoints.back();
}

int PetCfg::calcAbsorbedPropPoints(float absorbedPropsNum, int mainProp, const vector<float> &lvlPoints)
{
    int i;
    int newPoints = 0;

    for (i = mainProp; i < lvlPoints.size() && absorbedPropsNum > 0; i++)
    {
        if (absorbedPropsNum >= lvlPoints[i])
        {
            newPoints++;
            absorbedPropsNum -= lvlPoints[i];
            continue;
        }
        
        break;
    }
    
    
    if (i < lvlPoints.size() && absorbedPropsNum > 0)
    {
        float rate = absorbedPropsNum / lvlPoints[i];
        
        float randnum = range_randf(0, 1);
        if (rate > randnum)
        {
            newPoints++;
        }
    }
    
    return newPoints;
}

int PetCfg::calcMinAbsorbedPropPoints(float absorbedPropsNum, int mainProp, const vector<float> &lvlPoints)
{
    int i;
    int newPoints = 0;
    
    for (i = mainProp; i < lvlPoints.size() && absorbedPropsNum > 0; i++)
    {
        if (absorbedPropsNum >= lvlPoints[i])
        {
            newPoints++;
            absorbedPropsNum -= lvlPoints[i];
            continue;
        }
        
        break;
    }
     return newPoints;
}

int PetCfg::calcMaxRateAbsorbedPropPoints(float absorbedPropsNum, int mainProp, const vector<float> &lvlPoints)
{
    int i;
    int rate = 0;
    for (i = mainProp; i < lvlPoints.size() && absorbedPropsNum > 0; i++)
    {
        if (absorbedPropsNum >= lvlPoints[i])
        {
            absorbedPropsNum -= lvlPoints[i];
            continue;
        }
        
        break;
    }
    if (i < lvlPoints.size() && absorbedPropsNum > 0)
    {
        rate = 1;
    }

    return rate;
}


int PetAssistCfg::starinterval = 0;
string PetAssistCfg::propDec;
map<int, PetAssistDef*> PetAssistCfg::petAssistDefMap;

void PetAssistCfg::load(string path)
{
     try
     {
        GameInifile inifile(path);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
        {
            if (inifile.exists(*iter))
            {
                if (strcmp((*iter).c_str(), "common") == 0)
                {
                    propDec = inifile.getValue("common", "desc");
                    starinterval = inifile.getValueT("common", "star_interval", 1);
                    continue;
                }
                std::string propsStr = inifile.getValue(*iter, "props","");
                std::string namesStr = inifile.getValue(*iter, "names","");
                int lvl = inifile.getValueT(*iter, "lvl", 0);
                int awakelvl = inifile.getValueT(*iter, "awakelvl", 0);
                
                int position = Utils::safe_atoi((*iter).c_str());
                
                PetAssistDef *petAssistDef = new PetAssistDef();
                
                petAssistDef->position = position;
                petAssistDef->lvl = lvl;
                petAssistDef->awakeLvl = awakelvl;
                StringTokenizer proptoken( propsStr, ";");
                StringTokenizer nametoken( namesStr, ";");
                assert(proptoken.count() == nametoken.count());
                for( int i = 0; i < proptoken.count();i++ )
                {
                    PetAssistAddPropDef *petAssistAddPropDef = new PetAssistAddPropDef();
                    petAssistAddPropDef->prop = proptoken[i];
                    petAssistAddPropDef->name = nametoken[i];
                    petAssistDef->petAssistAddPropDefVec.push_back(petAssistAddPropDef);
                }
                int maxQua = inifile.getValueT(*iter, "maxqua", 1);
                for (int i = 0; i < maxQua; i++)
                {
                    string ratiodesc = strFormat("ratio%d", i + 1);
                    string ratiostr = inifile.getValue(*iter, ratiodesc.c_str());
                    StringTokenizer ratiotoken(ratiostr, ";");
                    vector<float > data;
                    for (int j = 0; j < ratiotoken.count(); j++)
                    {
                        char* end;
                        data.push_back(strtof(ratiotoken[j].c_str(), &end));
                    }
                    petAssistDef->ratioDatas.push_back(data);
                }
                
                petAssistDefMap[position] = petAssistDef;
             }
        }
     }catch(...)
    {
        log_error("PetAssistCfg ERROR !!!");
    }
    
}

float  PetAssistCfg::getStagePropRatio(int pos,int qua, int stage)
{
    int lvl = stage / starinterval;
    PetAssistDef* petAssistDef = getPetAssistDefByPos( pos );
    if (qua < 1 || qua > petAssistDef->ratioDatas.size())
    {
        return 0.0;
    }
    
    if (lvl < 0 || lvl >= petAssistDef->ratioDatas[qua - 1].size())
    {
        return 0.0;
    }
    return petAssistDef->ratioDatas[qua - 1][lvl];
}

string PetAssistCfg::getPetAssistDesc()
{
    return propDec;
}
int PetAssistCfg::getMaxPos()
{
    return petAssistDefMap.size();
}

PetAssistDef* PetAssistCfg::getPetAssistDefByPos(int pos)
{
    map<int, PetAssistDef*>::iterator iter = petAssistDefMap.find(pos);
    if(iter == petAssistDefMap.end())
    {
        return NULL;
    }
    return iter->second;
}

map<int, PetAssistDef*> &PetAssistCfg::getPetAssistDefMap()
{
   return petAssistDefMap;
}

int PetAssistCfg::getPetAssistDefLvlByPos(int pos)
{
    return getPetAssistDefByPos(pos)->lvl;
}

int PetAssistCfg::getPetAssistDefAwakeLvlByPos(int pos)
{
    return getPetAssistDefByPos(pos)->awakeLvl;
}

vector<PetAssistAddPropDef *> &PetAssistCfg::getPetAssistAddPropDefVecByPos(int pos)
{
    return getPetAssistDefByPos(pos)->petAssistAddPropDefVec;
}

vector<PetAssistDef *> PetAssistCfg::getOpenPetAssistDefVec(int lvl, int awakeLvl)
{
    vector<PetAssistDef*> openPetAssistDefVec;
    for(map<int, PetAssistDef *>::iterator iter = petAssistDefMap.begin(); iter !=petAssistDefMap.end(); iter++)
    {
        if(iter->second == NULL)
        {
            continue;
        }
        
        if (lvl >= iter->second->lvl && awakeLvl >= iter->second->awakeLvl)
        {
            openPetAssistDefVec.push_back(iter->second);
        }
    }
    return openPetAssistDefVec;
}

bool PetAssistCfg::isPositionOpen(int rolelvl, int awakeLvl, int checkPos)
{
    map<int, PetAssistDef *>::iterator iter = petAssistDefMap.find(checkPos);
    
    if(iter == petAssistDefMap.end() || iter->second == NULL)
    {
        return false;
    }
    
    if ( rolelvl >= iter->second->lvl && awakeLvl >= iter->second->awakeLvl) {
        return true;
    }
    
    return false;
}

std::map<string,string> QualityColorCfg::colorCfg;
bool QualityColorCfg::load(std::string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            colorCfg[iter->c_str()] = inifile.getValue(*iter, "colour");
        }
    } catch (...)
    {
        throw 1;
    }
    return true;
}
string  QualityColorCfg::getColor(string name)
{
    std::map<string,string>::iterator iter = colorCfg.find(name);
    if(iter == colorCfg.end())
    {
        return "255,255,255";
    }
    else
        return iter->second;
}

map<string, vector<DungeonCfgDef *> >DungeonCfg::cfg;
map<int, DungeonCfgDef * >DungeonCfg::cfgIdIndex;
bool DungeonCfg::load(std::string fullPath)
{
    cfg.clear();
    try
    {
        //--lihengjin---
        GameInifile inifile(fullPath);
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("dungeonMultiLanguage.ini") == "dungeonMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("dungeonMultiLanguage.ini"));
        }
        //GameInifile MLIniFile(Cfgable::getFullFilePath("dungeonMultiLanguage.ini"));
        std::string str[5] = {"Normal_Dungeons", "Firend_dungeons","pet_dungeons",  "God_Dungeons", "Guild_Dungeons"};
        for(int i = 0 ; i < sizeof(str)/sizeof(std::string); i++)
        {
            //获取不同的地下城的副本类型
            std::string activity_type =  inifile.getValue(str[i], "dungeon_list");
            vector<std::string> activityStr = StrSpilt(activity_type, ";");
            
            vector<DungeonCfgDef * > datas;
            for(int j = 0; j < activityStr.size(); j++)
            {
                DungeonCfgDef * data = new DungeonCfgDef;
                data->dungeonid =  inifile.getValueT(activityStr[j], "dungeonid", 0);
                data->difficulty = inifile.getValueT(activityStr[j], "difficulty", 0);
                string dungeonname_id = inifile.getValue(activityStr[j], "dungeonname_id");
                         
                string str_dungeonname;
                if(inifile.exists(activityStr[j], "dungeonname"))
                {
                    str_dungeonname = inifile.getValue(activityStr[j], "dungeonname");
                }else
                {
                    str_dungeonname = MLIniFile.getValue(dungeonname_id, "desc");
                }
                data->desc = str_dungeonname;
                data->floor = inifile.getValueT(activityStr[j], "floor", 0);
                data->energy_cosume = inifile.getValueT(activityStr[j], "energy_consumption", 0);
                data->firstmap = inifile.getValueT(activityStr[j], "firstmap", 0);
                data->minlvl = inifile.getValueT(activityStr[j], "minlvl", 0);
                string dropname_id = inifile.getValue(activityStr[j], "dropname_id");
                string str_dropname;
                if(inifile.exists(activityStr[j], "dropname"))
                {
                    str_dropname = inifile.getValue(activityStr[j], "dropname");
                }else
                {
                    str_dropname = MLIniFile.getValue(dropname_id, "desc");
                }
                data->award = str_dropname;
                string costrmb = inifile.getValue(activityStr[j], "rmbcost");
                StringTokenizer token(costrmb,";");
                for(int i=0;i< token.count();i++)
                {
                    data->rmbcost.push_back(Utils::safe_atoi(token[i].c_str()));
                }
                data->icon = inifile.getValue(activityStr[j],"icon");
                
                datas.push_back(data);
                cfgIdIndex[data->dungeonid] = data;
            }
            
            cfg.insert(make_pair(str[i], datas));
            
            
        }
        
        //--liehngjin---
        
//        GameInifile inifile(fullPath);
//        //lihengjin
//        GameInifile* MLIniFile;
//        if(!(Cfgable::getFullFilePath("dungeonMultiLanguage.ini") == "dungeonMultiLanguage.ini"))
//        {
//            MLIniFile = new GameInifile(Cfgable::getFullFilePath("dungeonMultiLanguage.ini"));
//        }
//        //GameInifile MLIniFile(Cfgable::getFullFilePath("dungeonMultiLanguage.ini"));
//        std::list<std::string> sections;
//        inifile.getSections(std::back_inserter(sections));
//        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
//        {
//            DungeonCfgDef * data = new DungeonCfgDef;
//            data->dungeonid = inifile.getValueT(*iter, "dungeonid", 0);
//            string dungeonname_id = inifile.getValue(*iter, "dungeonname_id");
//            //by lihengjin
//            string str_dungeonname;
//            if(inifile.exists(*iter, "dungeonname"))
//            {
//                str_dungeonname = inifile.getValue(*iter, "dungeonname");
//            }else
//            {
//                str_dungeonname = MLIniFile->getValue(dungeonname_id, "desc");
//            }
//            data->desc = str_dungeonname;//inifile.getValue(*iter, "dungeonname");
//            data->floor = inifile.getValueT(*iter, "floor", 0);
//            data->energy_cosume = inifile.getValueT(*iter, "energy_consumption", 0);
//            data->firstmap = inifile.getValueT(*iter, "firstmap", 0);
//            data->minlvl = inifile.getValueT(*iter, "minlvl", 0);
//            string dropname_id = inifile.getValue(*iter, "dropname_id");
//            //by lihengjin
//            string str_dropname;
//            if(inifile.exists(*iter, "dropname"))
//            {
//                str_dropname = inifile.getValue(*iter, "dropname");
//            }else
//            {
//                str_dropname = MLIniFile->getValue(dropname_id, "desc");
//            }
//            data->award = str_dropname;//inifile.getValue(*iter, "dropname");
//            string costrmb = inifile.getValue(*iter, "rmbcost");
//            StringTokenizer token(costrmb,";");
//            for(int i=0;i< token.count();i++)
//            {
//                data->rmbcost.push_back(Utils::safe_atoi(token[i].c_str()));
//            }
//            data->icon = inifile.getValue(*iter,"icon");
//            cfg.insert(std::make_pair<int, DungeonCfgDef*>(atoi(iter->c_str()), data));
//
//        }
    } catch (...)
    {
        throw 1;
    }
    return true;
}



DungeonCfgDef * DungeonCfg::getDungeonCfg(eSceneType sceneType, int index)
{
    std::string str = getSceneNameBySceneType(sceneType);
    std::map<string, vector<DungeonCfgDef*> >::iterator iter = cfg.find(str);
    if(iter == cfg.end()) {
        return NULL;
    }
    
    for (int i = 0; i < iter->second.size(); i++) {
        DungeonCfgDef* cfgDef = iter->second[i];
        if (cfgDef->dungeonid == index) {
            return cfgDef;
        }
    }
    
    return NULL;
}

DungeonCfgDef* DungeonCfg::getDungeonCfg(int index)
{
    std::map<int, DungeonCfgDef*>::iterator iter = cfgIdIndex.find(index);

    if (iter == cfgIdIndex.end()) {
        return NULL;
    }
    return iter->second;
}

vector<DungeonCfgDef*> DungeonCfg::getDungeonDatasByType(string name)
{
    vector<DungeonCfgDef *> data;
    std::map<string, vector<DungeonCfgDef*> >::iterator iter =  cfg.find(name);
    if(iter != cfg.end())
    {
        data = iter->second;
    }
    return data;
}


vector<DungeonCfgDef *> DungeonCfg::getDungeonDatasByType(eSceneType sceneType)
{
    std::string str = getSceneNameBySceneType(sceneType);
    return DungeonCfg::getDungeonDatasByType(str);
}

int DungeonCfg::getResetCost(int index, int times)
{
    DungeonCfgDef* cfg = getDungeonCfg(index);
    if (cfg == NULL) {
        return -1;
    }

    if (times >= cfg->rmbcost.size()) {
        return -1;
    }
    return cfg->rmbcost[times];
}

int DungeonCfg::getEnergyConsumption(int index)
{
    DungeonCfgDef* cfg = getDungeonCfg(index);
    if (cfg == NULL) {
        return -1;
    }
    
    return cfg->energy_cosume;
}
int DungeonCfg::getEnergyConsumption(string index, int tag)
{
    vector<DungeonCfgDef *>  vec = getDungeonDatasByType(index);
    if(tag < 0 )
    {
        return -1;
    }
    if(vec.size() <= tag)
    {
        return -1;
    }
    return  vec[tag]->energy_cosume;
}

int DungeonCfg::getEnergyConsumption(eSceneType sceneType, int index)
{
    std::string str = getSceneNameBySceneType(sceneType);
   vector<DungeonCfgDef *>  vec = DungeonCfg::getDungeonDatasByType(str);
    if(index < 0 )
    {
        return 0;
    }
    if(vec.size() <= index)
    {
        return 0;
    }
    return  vec[index]->energy_cosume;

}

map<int, ChargeCfgDef*> ChargeCfg::cfg;
bool ChargeCfg::load(std::string fullPath)
{
    cfg.clear();
    try
    {
        GameInifile inifile(fullPath);
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("chargeMultiLanguage.ini") == "chargeMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("chargeMultiLanguage.ini"));
        }
//        std::string MLFullPath = Cfgable::getFullFilePath("chargeMultiLanguage.ini");
//        GameInifile MLIniFile(MLFullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            ChargeCfgDef * data = new ChargeCfgDef;
            data->index = inifile.getValueT(*iter, "index", 0);
            // MultiLanguage adapt
            string product_name = inifile.getValue(*iter, "product_name_id");
            //by lihengjin
            string productNameStr;
            if(inifile.exists(*iter, "product_name"))
            {
                productNameStr = inifile.getValue(*iter, "product_name");
            }else
            {
                productNameStr = MLIniFile.getValue(product_name, "desc");
            }
            data->product_name = productNameStr;//inifile.getValue(*iter, "product_name");
            data->diamond_num = inifile.getValueT(*iter, "diamond_num", 0);
            data->rmb_num = inifile.getValueT(*iter, "rmb_num", 0.0f);
            data->is_hot = (inifile.getValueT(*iter, "is_hot",0)==1)?true:false;
            data->platform = inifile.getValue(*iter, "platform");
            data->productID = inifile.getValue(*iter, "product_id");
            cfg[data->index] = data;
        }
    } catch (...)
    {
        throw 1;
    }
    return true;
}

vector<ChargeCfgDef*> ChargeCfg::getChargeList(string _platform)
{
    vector<ChargeCfgDef*> _chargeList;
    for (map<int, ChargeCfgDef*>::iterator i = cfg.begin(); i != cfg.end(); i++) {
        if ((i->second)->platform  == _platform) {
            _chargeList.push_back(i->second);
        }
    }
    return _chargeList;
}
map <int,string> sysTipCfg::cfg;
bool sysTipCfg::load(std::string fullPath)
{
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("systemannouncementMultiLanguage.ini") == "systemannouncementMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("systemannouncementMultiLanguage.ini"));
    }
    //GameInifile MLIniFile(Cfgable::getFullFilePath("systemannouncementMultiLanguage.ini"));
    cfg.clear();
    try
    {
        GameInifile inifile(fullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            string str;
            string descript_id = inifile.getValue(*iter, "descript_id");
            //by lihengjin
            if(inifile.exists(*iter,"descript"))
            {
                str = inifile.getValue(*iter, "descript");
            }else
            {
                str = MLIniFile.getValue(descript_id, "desc");
            }
           // str = MLIniFile.getValue(descript_id, "desc");//inifile.getValue(*iter, "descript");
            cfg[atoi(iter->c_str())] = str;
        }
    } catch (...)
    {
        throw 1;
    }
    return true;
}
string sysTipCfg::getSysDesc(int index)
{
    if(cfg.find(index) != cfg.end())
        return cfg[index];
    return " ";
}




vector<IconCfgDef*> IconCfg::iconf;
bool IconCfg::load(std::string fullPath)
{
    iconf.clear();
    try
    {
        GameInifile inifile(fullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            IconCfgDef *icon=new IconCfgDef;

            icon->type=inifile.getValueT(*iter, "type",0);
            icon->IConImgName=inifile.getValue(*iter, "icon");
            iconf.push_back(icon);
        }
    } catch (...)
    {
        throw 1;
    }
    return true;
}
string IconCfg::getImgName(enLotteryItemType index)
{
    if (index>=0||index<iconf.size()) {
        return iconf[index]->IConImgName;
    }
    return "";
}

#pragma mark- 限时日常活动

std::vector<ActivityCfgDef*> ActivityCfg::cfg;//普通活动
std::vector<ActivityCfgDef*> ActivityCfg::guildcfg;//公会活动
std::vector<ActivityCfgDef*> ActivityCfg::synPvpcfg;    // 同步竞技场活动      
std::vector<int> ActivityCfg::guildActivityTotalIds;
std::vector<string> ActivityCfg::guildTreasureFightOpenDays;
bool ActivityCfg::load(std::string fullPath)
{
    cfg.clear();
    guildcfg.clear();
    synPvpcfg.clear();
    guildActivityTotalIds.clear();
    guildTreasureFightOpenDays.clear();
    try
    {
        GameInifile inifile(fullPath);
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("activityMultiLanguage.ini") == "activityMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("activityMultiLanguage.ini"));
        }
       // GameInifile MLIniFile(Cfgable::getFullFilePath("activityMultiLanguage.ini"));
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
        {
            ActivityCfgDef *data = new ActivityCfgDef;
            data->activityId = inifile.getValueT(*iter, "id", 0);
            
            // MultiLanguage adapt
            std::string nameID = inifile.getValue(*iter, "name_id");
            //by lihengjin
            std::string nameStr;
            if(inifile.exists(*iter, "name"))
            {
                nameStr = inifile.getValue(*iter, "name");
            }else
            {
                nameStr = MLIniFile.getValue(nameID, "desc");
            }
           // std::string nameStr = MLIniFile.getValue(nameID, "desc");
            data->activityName = nameStr;//inifile.getValue(*iter, "name");
            string topname_id = inifile.getValue(*iter, "topname_id");
            //by lihengjin
            string str_topname;
            if(inifile.exists(*iter, "topname"))
            {
                str_topname = inifile.getValue(*iter, "topname");
            }else
            {
                str_topname = MLIniFile.getValue(topname_id, "desc");
            }
            data->activityTopName = str_topname; //inifile.getValue(*iter, "topname");

            data->sort = inifile.getValueT(*iter, "sort", 0);
            data->is_flash = inifile.getValueT(*iter, "is_flash", 0);

            string time_info_id = inifile.getValue(*iter, "time_info_id");
            //by lihengjin
            string  str_time_info;
            if(inifile.exists(*iter, "time_info"))
            {
                str_time_info = inifile.getValue(*iter, "time_info");
            }else
            {
                str_time_info = MLIniFile.getValue(time_info_id, "desc");
            }
            data->activityTime = str_time_info;//inifile.getValue(*iter, "time_info");
            data->minLvl = inifile.getValueT(*iter, "lvl", 0);
            string content_info_id = inifile.getValue(*iter, "content_info_id");
            //by lihengjin
            string str_content_info;
            if(inifile.exists(*iter, "content_info"))
            {
                str_content_info = inifile.getValue(*iter, "content_info");
            }else
            {
                str_content_info = MLIniFile.getValue(content_info_id, "desc");
            }
            data->activityContent = str_content_info;//inifile.getValue(*iter, "content_info");
            string award_info_id = inifile.getValue(*iter, "award_info_id");
            //by lihengjin
            string str_award_info;
            if(inifile.exists(*iter, "award_info"))
            {
                str_award_info = inifile.getValue(*iter, "award_info");
            }else
            {
                str_award_info = MLIniFile.getValue(award_info_id, "desc");
            }
            data->ActivityAward = str_award_info;//inifile.getValue(*iter, "award_info");
            data->activityIcon = inifile.getValue(*iter, "icon");
            data->isblink = inifile.getValueT(*iter, "is_MainUIblink", 0);
            data->isTicket = inifile.getValueT(*iter, "is_ticket", 0);
            //------------公会部分----------------
            data->isguild=inifile.getValueT(*iter, "is_guild", 0);
            data->isNum=inifile.getValueT(*iter, "is_num", 0);
            data->isChooseMode=inifile.getValueT(*iter, "is_chooseMode", 0);
            data->guildLvl=inifile.getValueT(*iter, "guildlvl", 0);
            //-----------------------------------
            string daysStr = inifile.getValue(*iter, "day");
            StringTokenizer daysToken(daysStr,";");
            for (int i =0; i< daysToken.count(); i++) {
                if (data->activityId == 43) { //公会夺宝战 活动 id
                    guildTreasureFightOpenDays.push_back(daysToken[i]);
                }
            }
            
            if( inifile.getValueT(*iter, "enable", 0) == 1)
            {
                  //cfg.push_back(data);
                if (inifile.getValueT(*iter, "is_guild", 0)==1) {
                    guildcfg.push_back(data);
                    guildActivityTotalIds.push_back(data->activityId);
                }
                else if(inifile.getValueT(*iter, "is_guild", 0)==2){ // 限时活动 和 公会活动 入口 都要显示
                    guildcfg.push_back(data);
                    guildActivityTotalIds.push_back(data->activityId);
                    cfg.push_back(data);
                }
                else if (inifile.getValueT(*iter, "is_arena", 0)==1)
                {
                    synPvpcfg.push_back(data);
                }
                else{
                    cfg.push_back(data);
                }
            }
        }
    } catch (...)
    {
        throw 1;
    }
    return true;
}
ActivityCfgDef * ActivityCfg::getActivityData(int id)
{
    for(int i=0; i< cfg.size(); i++)
    {
        if(cfg[i]->activityId == id)
            return cfg[i];
    }
    return NULL;
}
ActivityCfgDef * ActivityCfg::getGuildActivityData(int id){
    for (int i=0; i<guildcfg.size(); i++) {
        if (guildcfg[i]->activityId==id) {
            return guildcfg[i];
        }
    }
    return NULL;
}

ActivityCfgDef* ActivityCfg::getCfg(int id)
{
    ActivityCfgDef* cfg =  getActivityData(id);
    if (cfg)
    {
        return cfg;
    }
    cfg = getGuildActivityData(id);
    if (cfg)
    {
        return cfg;
    }
    cfg = getSynPvpData(id);
    return cfg;
}

int  ActivityCfg::getMaxActivity()
{
    return cfg.size();
}
int ActivityCfg::getGuildMaxActivity(){
    return guildcfg.size();
}
bool cmp(ActivityCfgDef * data1, ActivityCfgDef * data2)
{
    if(data1->sort <= data2->sort)
        return true;
    else
        return false;
}

std::vector<ActivityCfgDef *> ActivityCfg::getAllDataBySort(std::vector<int> data)
{
    vector<ActivityCfgDef *> allDatas, lightDatas,nolightDatas,otherDatas;
    for(int i=0;i< data.size();i++)
    {
        ActivityCfgDef * def = getActivityData(data[i]);
        if(def)
        {
            if(def->is_flash == 1)
            {
                lightDatas.push_back(def);
            }
            else
            {
                nolightDatas.push_back(def);
            }
        }
    }
    sort(lightDatas.begin(), lightDatas.end(), cmp);
    sort(nolightDatas.begin(), nolightDatas.end(),cmp);

    for(int i=0;i< lightDatas.size();i++)
    {
        allDatas.push_back(lightDatas[i]);
    }

    for(int i=0;i< nolightDatas.size();i++)
    {
        allDatas.push_back(nolightDatas[i]);
    }

    for(int i=0;i< cfg.size();i++)
    {
        int j = 0;
        for(j= 0;j< data.size();j++)
        {
            if(cfg[i]->activityId == data[j])
                break;
        }
        if(j == data.size())
        {
            otherDatas.push_back(cfg[i]);
        }
    }
    sort(otherDatas.begin(), otherDatas.end(), cmp);
    for(int i=0;i< otherDatas.size();i++)
    {
        allDatas.push_back(otherDatas[i]);
    }
    return allDatas;
}

std::vector<ActivityCfgDef *> ActivityCfg::getAllDataBySort(std::vector<int> data, int lvl)
{
    vector<ActivityCfgDef *> allDatas, lightDatas, nolightDatas, otherDatas, limitLvlDatas;
    for(int i=0;i< data.size();i++)
    {
        ActivityCfgDef * def = getActivityData(data[i]);
        if(def)
        {
            if(def->is_flash == 1)
            {
                if(def->minLvl <= lvl){  //2.0 不够等级,活动不闪烁
                    lightDatas.push_back(def);
                }else{
                    limitLvlDatas.push_back(def);
                }
            }
            else
            {
                if (def->minLvl <= lvl) {
                    nolightDatas.push_back(def);
                }else{
                    limitLvlDatas.push_back(def);
                }
            }
        }
    }
    sort(lightDatas.begin(), lightDatas.end(), cmp);
    sort(nolightDatas.begin(), nolightDatas.end(),cmp);
    sort(limitLvlDatas.begin(), limitLvlDatas.end(),cmp);
    for(int i=0;i< lightDatas.size();i++)
    {
        allDatas.push_back(lightDatas[i]);
    }
    
    for(int i=0;i< nolightDatas.size();i++)
    {
        allDatas.push_back(nolightDatas[i]);
    }
    
    for (int i = 0; i<cfg.size(); i++) {
        vector<int>::iterator iter = std::find(data.begin(), data.end(), cfg[i]->activityId);
        if (iter == data.end()) {
            otherDatas.push_back(cfg[i]);
        }
    }
    sort(otherDatas.begin(), otherDatas.end(), cmp);
    for(int i=0;i< otherDatas.size();i++)
    {
        allDatas.push_back(otherDatas[i]);
    }
    for (int i =0; i<limitLvlDatas.size(); i++) {
        allDatas.push_back(limitLvlDatas[i]);
    }
    return allDatas;
}


std::vector<ActivityCfgDef *> ActivityCfg::getGuildAllDataBySort(std::vector<int> data)
{
    vector<ActivityCfgDef *> allDatas, lightDatas,nolightDatas,otherDatas;
    for(int i=0;i< data.size();i++)
    {
        ActivityCfgDef * def = getGuildActivityData(data[i]);
        if(def)
        {
            if(def->is_flash == 1)
            {
                lightDatas.push_back(def);
            }
            else
            {
                nolightDatas.push_back(def);
            }
        }
    }
    sort(lightDatas.begin(), lightDatas.end(), cmp);
    sort(nolightDatas.begin(), nolightDatas.end(),cmp);
    
    for(int i=0;i< lightDatas.size();i++)
    {
        allDatas.push_back(lightDatas[i]);
    }
    
    for(int i=0;i< nolightDatas.size();i++)
    {
        allDatas.push_back(nolightDatas[i]);
    }
    
    for(int i=0;i< guildcfg.size();i++)
    {
        int j = 0;
        for(j= 0;j< data.size();j++)
        {
            if(guildcfg[i]->activityId == data[j])
                break;
        }
        if(j == data.size())
        {
            otherDatas.push_back(guildcfg[i]);
        }
    }
    sort(otherDatas.begin(), otherDatas.end(), cmp);
    for(int i=0;i< otherDatas.size();i++)
    {
        allDatas.push_back(otherDatas[i]);
    }
    return allDatas;
}

vector<int> ActivityCfg::getGuildActivityTotalId()
{
    return guildActivityTotalIds;
}

vector<ActivityCfgDef*> ActivityCfg::getSynPvpDataBySort(std::vector<int> data)
{
    return synPvpcfg;
}
ActivityCfgDef* ActivityCfg::getSynPvpData(int id)
{
    for (int i = 0; i < (int)synPvpcfg.size(); ++i)
    {
        if (synPvpcfg[i]->activityId == id) {
            return synPvpcfg[i];
        }
    }
    return NULL;
}
vector<string> ActivityCfg::getGuildTreasureFightOpenDays(){
    return guildTreasureFightOpenDays;
}
#pragma mark- 普通装备幻兽活动

std::vector<OrdinaryActivityCfgDef*> OrdinaryActivityCfg::equipCfg;
std::vector<OrdinaryActivityCfgDef*> OrdinaryActivityCfg::petCfg;
std::vector<int> OrdinaryActivityCfg::equipCfgIds;
std::vector<int> OrdinaryActivityCfg::petCfgIds;

bool OrdinaryActivityCfg::load(std::string fullPath)
{
    equipCfg.clear();
    petCfg.clear();
    equipCfgIds.clear();
    petCfgIds.clear();
    try {
        GameInifile inifile(fullPath);
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("daily_activityMultiLanguage.ini") == "daily_activityMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("daily_activityMultiLanguage.ini"));
        }
        //GameInifile MLIniFile(Cfgable::getFullFilePath("daily_activityMultiLanguage.ini"));
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
            OrdinaryActivityCfgDef *data = new OrdinaryActivityCfgDef;
            data->activityId = inifile.getValueT(*iter, "id", 0);
            
            //多语言适配
//            std::string nameId = inifile.getValue(*iter, "name_id");
//            std::string nameStr = MLIniFile.getValue(nameId, "desc");
            string nameStr;
            if (inifile.exists(*iter, "name"))
            {
                nameStr = inifile.getValue(*iter, "name");
            }else{
                nameStr = MLIniFile.getValue(*iter, "desc");
            }
            data->activityName = nameStr;
            
            data->sort = inifile.getValueT(*iter, "sort", 0);
            data->type = inifile.getValueT(*iter, "type", 0);
            
            std::string ico = inifile.getValue(*iter, "icon");
            data->activityIcon = ico;
            
            std::string awardId = inifile.getValue(*iter, "award_info_id");
            //by lihengjin
            std::string awardStr;
            if(inifile.exists(*iter, "award_info"))
            {
                awardStr = inifile.getValue(*iter, "award_info");
            }else
            {
                awardStr = MLIniFile.getValue(awardId, "desc");
            }
           // std::string awardStr = MLIniFile.getValue(awardId, "desc");
            data->activityAward = awardStr;
            
            std::string contentId = inifile.getValue(*iter, "content_info_id");
            //by lihengjin
            std::string contentStr;
            if(inifile.exists(*iter, "content_info"))
            {
                contentStr = inifile.getValue(*iter, "content_info");
            }else
            {
                contentStr = MLIniFile.getValue(contentId, "desc");
            }
           // std::string contentStr = MLIniFile.getValue(contentId, "desc");
            data->activityContent = contentStr;
            
            data->minLvl = inifile.getValueT(*iter, "lvl", 0);
            data->is_ticket = inifile.getValueT(*iter, "is_ticket",0);
            //分离活动: 装备活动 和 幻兽活动
            if (inifile.getValueT(*iter, "type", 0) == 1)
            {
                //装备活动
                equipCfg.push_back(data);
                equipCfgIds.push_back(data->activityId);
            }
            else if (inifile.getValueT(*iter, "type", 0) == 2)
            {
                //幻兽活动
                petCfg.push_back(data);
                petCfgIds.push_back(data->activityId);
            }
        }
    } catch (...) {
        throw 1;
    }
    return true;
}

OrdinaryActivityCfgDef * OrdinaryActivityCfg::getEquiqActivityData(int id)
{
    for (int i = 0; i < equipCfg.size(); i++) {
        if (equipCfg[i]->activityId == id) {
            return equipCfg[i];
        }
    }
    return NULL;
}

OrdinaryActivityCfgDef * OrdinaryActivityCfg::getPetActivityData(int id)
{
    for (int i = 0; i < petCfg.size(); i++) {
        if (petCfg[i]->activityId == id) {
            return petCfg[i];
        }
    }
    return NULL;
}

OrdinaryActivityCfgDef * OrdinaryActivityCfg::getCfg(int id)
{
    OrdinaryActivityCfgDef* cfg = getEquiqActivityData(id);
    if(cfg) return cfg;
    cfg = getPetActivityData(id);
    return cfg;
}

int OrdinaryActivityCfg::getMaxEquipActivity()
{
    return equipCfg.size();
}

int OrdinaryActivityCfg::getMaxPetActivity()
{
    return petCfg.size();
}

bool cmpSort(OrdinaryActivityCfgDef * data1, OrdinaryActivityCfgDef *data2)
{
    if (data1->sort <= data2->sort) {
        return true;
    }else{
        return false;
    }
}
std::vector<OrdinaryActivityCfgDef *> OrdinaryActivityCfg::getSortedEquipActDatas(std::vector<int> data)
{
    vector<OrdinaryActivityCfgDef *> equipActDatas;
    
    for (int i=0 ; i< data.size(); i++) {
        OrdinaryActivityCfgDef *def =getEquiqActivityData(data[i]);
        if (def) {
            equipActDatas.push_back(def);
        }
    }
    sort(equipActDatas.begin(), equipActDatas.end(),cmpSort);
    return equipActDatas;
}

std::vector<OrdinaryActivityCfgDef *> OrdinaryActivityCfg::getSortedPetActDatas(std::vector<int> data)
{
    vector<OrdinaryActivityCfgDef *> petActDatas;
    
    for (int i=0 ; i< data.size(); i++) {
        OrdinaryActivityCfgDef *def =getPetActivityData(data[i]);
        if (def) {
            petActDatas.push_back(def);
        }
    }
    sort(petActDatas.begin(), petActDatas.end(),cmpSort);
    return petActDatas;
}
std::vector<int> OrdinaryActivityCfg::getTotalEquipActIds()
{
    return equipCfgIds;
}
std::vector<int> OrdinaryActivityCfg::getTotalPetActIds()
{
    return petCfgIds;
}

#pragma mark - WorldBoss

map<int,WorldBossCfgDef*> WorldBossCfg:: bosses;
vector<WorldBossBonusCfgDef*> WorldBossCfg:: ranges;
int WorldBossCfg:: firstBossID; int WorldBossCfg::lasthit;int WorldBossCfg::enter_lvl;
int WorldBossCfg::clientranknum; int WorldBossCfg::reborn_seconds;
float WorldBossCfg::syn_seconds; int WorldBossCfg::damage_bonus;
float WorldBossCfg::round_refresh_seconds = 0;
int WorldBossCfg::round_max_hurt_base = 0;
int WorldBossCfg::preparetime = 300;
map<int,int> WorldBossCfg::itemaward;

float WorldBossCfg::hurt_batpointaward = 0.0f; //伤害战功奖
float WorldBossCfg::hurt_goldaward = 0.0f;   //伤害金币奖


bool WorldBossCfg::load(std::string fullPath)
{
    bosses.clear();ranges.clear();
    try
    {
        GameInifile inifile(fullPath);

        damage_bonus = inifile.getValueT("root", "damage_bonus", 0);
        enter_lvl = inifile.getValueT("root", "enter_lvl", 20);
        firstBossID = inifile.getValueT("root", "first_boss", 0);
        lasthit = inifile.getValueT("root","last_hit",0);
        reborn_seconds = inifile.getValueT("root","reborn_seconds",3);
        clientranknum = inifile.getValueT("root","client_rank_num",10);
        syn_seconds = inifile.getValueT("root","syn_seconds",2);
        round_refresh_seconds = inifile.getValueT("root", "round_refresh_seconds", 30);
        round_max_hurt_base = inifile.getValueT("root", "period_maxhurt", 0);
        preparetime = inifile.getValueT("root", "prepare_time", 300);

        
        vector<int> boss_id =  inifile.getTableT("root", "boss_id", 0);
        vector<int> scene_id = inifile.getTableT("root", "scene_id", 0);
        vector<int> pass_time = inifile.getTableT("root", "pass_time", 0);
        vector<int> next_boss = inifile.getTableT("root", "next_boss", 0);
        vector<float> boss_award_factor = inifile.getTableT("root", "boss_award_factor", 0.0f);
        vector<int> rollBackTime = inifile.getTableT("root", "roll_back_time", 0);
        
        float batpoint_award = inifile.getValueT("root", "batpoint_award",.0f);
        float gold_award = inifile.getValueT("root", "gold_award",.0f);
        float exp_award = inifile.getValueT("root", "exp_award",.0f);
        float consval_award = inifile.getValueT("root", "consval_award",.0f);
        
        hurt_batpointaward = inifile.getValueT("root", "batpoint_period_award", .0f);
        hurt_goldaward = inifile.getValueT("root", "gold_period_award", .0f);
        
        assert(boss_id.size() == scene_id.size() &&
               scene_id.size() == pass_time.size() &&
               pass_time.size() == next_boss.size() &&
               next_boss.size() == boss_award_factor.size()&&
               boss_award_factor.size() == rollBackTime.size());
        
        
        map<int,int> beforeMap;
        
        for (int i=0; i<boss_id.size(); i++)
        {
            WorldBossCfgDef* wbcd = new WorldBossCfgDef;
            wbcd->bossid = boss_id[i];
            wbcd->sceneid = scene_id[i];
            wbcd->passtime = pass_time[i];
            wbcd->nextBoss = next_boss[i];
            wbcd->beforeBoss = wbcd->bossid; // before默认是自己
            wbcd->rollBackTime = rollBackTime[i];
            wbcd->level_factor = boss_award_factor[i];
                        
            wbcd->batpointaward = wbcd->level_factor * batpoint_award;
            wbcd->goldaward = wbcd->level_factor * gold_award;
            wbcd->expaward = wbcd->level_factor * exp_award;
            wbcd->constellaward = wbcd->level_factor * consval_award;
            
            bosses[wbcd->bossid] = wbcd;
            beforeMap.insert(make_pair(wbcd->nextBoss, wbcd->bossid));
            assert(wbcd->rollBackTime > wbcd->passtime);
        }
        
        // 通过next反推出 before是什么
        map<int,WorldBossCfgDef*>::iterator itBoss;
        for ( itBoss = bosses.begin(); itBoss != bosses.end() ; ++itBoss)
        {
            WorldBossCfgDef* wbcd = itBoss->second;
            assert(wbcd);
            map<int,int>::iterator itBefore = beforeMap.find(wbcd->bossid);
            if(itBefore != beforeMap.end() )
            {
                wbcd->beforeBoss = itBefore->second;
            }
        }

        
        vector<int> rank_bonus = inifile.getTableT("root", "rank_bonus", 0);
        vector<int> rank_range = inifile.getTableT("root", "rank_range", 0);

        assert(rank_bonus.size() == rank_range.size());
        for (int i=0; i<rank_bonus.size(); i++)
        {
            WorldBossBonusCfgDef* bonusdef = new WorldBossBonusCfgDef;
            bonusdef->rank_bonus = rank_bonus[i];
            bonusdef->rank_range = rank_range[i];
            ranges.push_back(bonusdef);
        }
        
        vector<int> items = inifile.getTableT("root", "ranking_item_award", 0);
        for (int i = 0; i< items.size(); ++i) {
            itemaward[i+1] = items[i];
        }

    } catch (...)
    {
        throw 1;
    }
    return true;
}

map<string, map<string, string > > MoneyMLCfg::_map;
void MoneyMLCfg::load(const string &filepath)
{
    GameInifile ini(filepath);
    vector<string> sections;
    vector<string> keys;
    ini.getSections(back_inserter(sections));
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        ini.getKeys(*iter, back_inserter(keys));
        for (vector<string>::iterator it = keys.begin(); it != keys.end(); ++it)
        {
            string tmp = ini.getValue(*iter, *it);
            //_map.insert(make_pair(*iter, make_pair(*it, tmp)));
            _map[*iter][*it] = tmp;
        }
        keys.clear();
    }
}

string MoneyMLCfg::getValue(const string &section, const string &key)
{
    MapIterator iter = _map.find(section);
    if (iter == _map.end())
    {
        return string("");
    }
    map<string, string>::iterator it = iter->second.find(key);
    if (it == iter->second.end())
    {
        return string("");
    }
    return it->second;
}


map<string, MoneyCfgDef> MoneyCfg::sMoneyNames;

void MoneyCfg::load(const string &filepath)
{
    GameInifile ini(filepath);
    vector<string> sections;
    ini.getSections(back_inserter(sections));
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        int id = ini.getValueT(*iter, "id", 0);
        string moneytype = ini.getValue(*iter, "moneytype");
        string name = ini.getValue(*iter, "name");
        int type = ini.getValueT(*iter, "type", 0);
        
        MoneyCfgDef def;// = {.id = id, .moneytype = moneytype, .name = name, .type = type};
        def.id = id;
        def.moneytype = moneytype;
        def.name = name;
        def.type = type;
        sMoneyNames[moneytype] = def;
    }
}

string MoneyCfg::getName(const char *money)
{
    map<string, MoneyCfgDef>::iterator iter = sMoneyNames.find(money);
    if (iter == sMoneyNames.end()) {
        return "";
    }
    return iter->second.name;
}

/*
map<string, map<string, string > > MailMLCfg::_map;
void MailMLCfg::load(const string &filepath)
{
    GameInifile ini(filepath);
    vector<string> sections;
    vector<string> keys;
    ini.getSections(back_inserter(sections));
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        ini.getKeys(*iter, back_inserter(keys));
        for (vector<string>::iterator it = keys.begin(); it != keys.end(); ++it)
        {
            string tmp = ini.getValue(*iter, *it);
            //_map.insert(make_pair(*iter, make_pair(*it, tmp)));
            _map[*iter][*it] = tmp;
        }
        keys.clear();
    }
}

string MailMLCfg::getValue(const string &section, const string &key)
{
    MapIterator iter = _map.find(section);
    if (iter == _map.end())
    {
        return string("");
    }
    map<string, string>::iterator it = iter->second.find(key);
    if (it == iter->second.end())
    {
        return string("");
    }
    return it->second;
}

*/


std::map<string, MailFormat*> MailCfg::formats;

void MailCfg::load(const char* fullPath)
{
    formats.clear();
    try {
        GameInifile inifile(fullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter=sections.begin(); iter!=sections.end(); iter++) {
            MailFormat* mailformat = new MailFormat;
            
            //mail modify
            /*
            string sender_id = inifile.getValue(*iter, "sender_id");
             mailformat->sendername = MailMLCfg::getValue(sender_id, "desc");
            
            string title_id = inifile.getValue(*iter, "title_id");
            mailformat->title = MailMLCfg::getValue(title_id, "desc");
            
            string content_id = inifile.getValue(*iter, "content_id");
            mailformat->content = MailMLCfg::getValue(content_id, "desc");
            */
            
            mailformat->sendername = inifile.getValue(*iter, "sender", "no");
            mailformat->title = inifile.getValue(*iter, "title", "no title");
            mailformat->content = inifile.getValue(*iter, "content", "");
            formats.insert(make_pair(*iter, mailformat));
        }
    } catch (...) {
        throw 1;
    }
}

MailFormat* MailCfg::getCfg(const char *name)
{
    std::map<string, MailFormat*>::iterator iter = formats.find(name);
    if (iter == formats.end()) {
        return NULL;
    }
    return iter->second;
}


//服务器提示信息
/*
void MLCfg::load(const char* fullpath)
{
    try
    {
        GameInifile ini(fullpath);
        
        string section = "common";
        vector<string> keys;
        ini.getKeys(section, inserter(keys, keys.begin()));
        
        for (int i = 0; i < keys.size(); i++)
        {
            string value = ini.getValue(section, keys[i]);
            strMapML[keys[i]] = value;
        }
    }
    catch (...)
    {
        
    }
}

string MLCfg::getStr(const char* key)
{
    map<string, string>::iterator iter = strMapML.find(key);
    if (iter == strMapML.end())
    {
        return "";
    }
    
    return iter->second;
}

void MLCfg::clear()
{
    strMapML.clear();
}*/


map<string, string>  StrMapCfg::strMap;
bool StrMapCfg::load(const char* fullPath)
{
    try
    {
        GameInifile ini(fullPath);
        string section = "common";
        
        vector<string> keys;
        ini.getKeys(section, inserter(keys, keys.begin()));
        
        for (int i = 0; i < keys.size(); i++)
        {
            string key = keys[i];
            string value = ini.getValue(section, key);
            //string value = mlCfg.getStr(id.c_str());
            
            strMap[key] = value;
        }
    }
    catch (...)
    {
        return false;
    }
    
    return true;
}

string StrMapCfg::getString(const char* name, const char* def)
{
    map<string, string>::iterator iter = strMap.find(name);
    if (iter == strMap.end())
    {
        return def;
    }
    
    return iter->second;
}



    

#pragma mark - 试炼 
map<int, ChallengeCfgDef*> ChallengeCfg::cfg;

ChallengeGuildCfg ChallengeCfg::guildcfg;

void ChallengeCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin() ; iter != sections.end(); iter++)
    {        
        //通用字段
        ChallengeCfgDef *def = new ChallengeCfgDef();
        def->wave = inifile.getValueT(*iter, "id", 0);
        def->time = inifile.getValueT(*iter, "time", 0);
        def->addContribute = inifile.getValueT(*iter, "addcontribute", 0);
        def->addFeats = inifile.getValueT(*iter, "addfeats", 0);
        def->addWealth = inifile.getValueT(*iter, "addwealth", 0);
        def->addOverBuild = inifile.getValueT(*iter, "addoverbuild", 0);
        def->failSum = inifile.getValueT(*iter, "failsNum", 0);
        def->minGuildLvl = inifile.getValueT(*iter, "min_guild_lvl", 0);
        string monsterinfo  = inifile.getValue(*iter, "monster");
        vector<monsterWave> _monsterWave;
        StringTokenizer reToken(monsterinfo,";");
        for (int i = 0; i < reToken.count(); i++)
        {
            StringTokenizer reTo(reToken[i],"*");
            monsterWave _m;
            _m.monsterid = atoi(reTo[0].c_str());
            _m.sum = atoi(reTo[1].c_str());
            _m.x = atoi(reTo[2].c_str());
            _m.y = atoi(reTo[3].c_str());
            _monsterWave.push_back(_m);
        }
        
        def->monsterWaves = _monsterWave;
        cfg[def->wave] = def;
        
    }

    guildcfg.traintNum = inifile.getValueT("common", "traintNum", 0);
    guildcfg.blessingNum = inifile.getValueT("common", "blessingNum", 0);
    guildcfg.blessedNum = inifile.getValueT("common", "blessedNum", 0);
    guildcfg.blessedAward = inifile.getValueT("common", "blessedAward", 0.0f);
    guildcfg.blessingRmb = inifile.getValueT("common", "blessingRmb", 0);
    guildcfg.inspireNum = inifile.getValueT("common", "inspireNum", 0);
    guildcfg.inspiredNum = inifile.getValueT("common", "inspiredNum", 0);
    guildcfg.inspiredfighting = inifile.getValueT("common", "inspiredfighting", 0.0f);
    guildcfg.inspiredRmb = inifile.getValueT("common", "inspireRmb", 0);
    guildcfg.blesser_get_award =  inifile.getValue("common", "blesser_get_award","");
}

const ChallengeGuildCfg& ChallengeCfg::getGuildDef()
{
    return guildcfg;
}

ChallengeCfgDef* ChallengeCfg::getChallengeCfg(int _lvl,int _wave)
{
    int _id = _lvl*100+_wave;
    if(cfg.find(_id) == cfg.end())
        return NULL;
    return cfg[_id];
}

int  ChallengeCfg::getWaveSumforChallengeLvl(int _lvl)
{
    int _id = _lvl*100;
    for (int i = 1;; i++)
    {
        if(cfg.find(_id+i) == cfg.end())
        {
            return i-1;
        }
    }
    return 0;
}


#pragma mark - 保卫神像配置表 defendstatue.ini
map<int, DefendStatueCfgDef*> DefendStatueCfg::cfg;

void DefendStatueCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin() ; iter != sections.end(); iter++)
    {
        //通用字段
        DefendStatueCfgDef *def = new DefendStatueCfgDef();
        def->wave = inifile.getValueT(*iter, "id", 0);
        def->desc = inifile.getValue(*iter, "dese");
        def->m_standarPoints = inifile.getValueT(*iter, "points", 0);
        def->m_standarTime = inifile.getValueT(*iter, "time", 0);
        def->m_coefficient = inifile.getValueT(*iter, "coefficient", 0);
        def->m_minPoints = inifile.getValueT(*iter, "minpoints", 0);
        def->m_maxPoints = inifile.getValueT(*iter, "maxpoints", 0);
        string monsterinfo  = inifile.getValue(*iter, "monster");
        
        //奖励
        def->mAward.exp_coef = inifile.getValueT(*iter, "exp_coef", 0);
        def->mAward.gold_coef = inifile.getValueT(*iter, "gold_coef", 0);
        def->mAward.batpoint_coef = inifile.getValueT(*iter, "batpoint_coef", 0);
        def->mAward.petexp_coef = inifile.getValueT(*iter, "petexp_coef", 0);
        
        vector<monsterWave> _monsterWave;
        StringTokenizer reToken(monsterinfo,";");
        for (int i = 0; i < reToken.count(); i++)
        {
            StringTokenizer reTo(reToken[i],"*");
            monsterWave _m;
            _m.monsterid = atoi(reTo[0].c_str());
            _m.sum = atoi(reTo[1].c_str());
            _m.x = atoi(reTo[2].c_str());
            _m.y = atoi(reTo[3].c_str());
            _monsterWave.push_back(_m);
        }
        def->monsterWaves = _monsterWave;
        cfg[def->wave] = def;
    }
}

DefendStatueCfgDef* DefendStatueCfg::getDefendStatueCfg(int _lvl, int _wave)
{
    int _id = _lvl*100+_wave;
    if(cfg.find(_id) == cfg.end())
        return NULL;
    return cfg[_id];
}

int DefendStatueCfg::getWaveSumforDefengLvl(int _lvl)
{
    int _id = _lvl*100;
    for (int i = 1;; i++)
    {
        if(cfg.find(_id+i) == cfg.end())
        {
            return i-1;
        }
    }
    return 0;
}
vector<FriendAdditAward*> FriendAwardCoefCfg::friend_coef;
vector<FriendAdditAward*> FriendAwardCoefCfg::intimacy_coef;
void FriendAwardCoefCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    
    int count = inifile.getValueT("global", "count", 0);
    int intimacyparts = inifile.getValueT("global", "intimacy_parts", 0);
    
    for (int i = 1; i <= count; i++) {
        
//        char tmp[128] = {0};
//        sprintf(tmp, "awardpart%d",i);
        
        string tmp = Utils::makeStr( "awardpart%d",i);
        string coefStr = inifile.getValue(tmp, "coef", "");
        
        StringTokenizer coef(coefStr, ";");
        FriendAdditAward* def = new FriendAdditAward;
        for (int j = 0; j < coef.count(); j++) {
            def->m_coef.push_back(atof(coef[j].c_str()));
        }
        
        def->mIsShuzu = Utils::safe_atoi(inifile.getValue(tmp, "is_shuzu", "").c_str());
        def->mMin = Utils::safe_atoi(inifile.getValue(tmp, "xiaxian", "").c_str());
        def->mMax = Utils::safe_atoi(inifile.getValue(tmp, "shangxian", "").c_str());
        
        friend_coef.push_back(def);
    }
    
    for (int i = 1; i <= intimacyparts; i++) {
        
//        char tmp[128] = {0};
//        sprintf(tmp, "intimacy_part%d",i);
        
        string tmp = Utils::makeStr("intimacy_part%d",i);
        
        string coefStr = inifile.getValue(tmp, "coef", "");
        
        StringTokenizer coef(coefStr, ";");
        FriendAdditAward* def = new FriendAdditAward;
        for (int j = 0; j < coef.count(); j++) {
            def->m_coef.push_back(atof(coef[j].c_str()));
        }
        
        def->mIsShuzu = Utils::safe_atoi(inifile.getValue(tmp, "is_shuzu", "").c_str());
        def->mMin = Utils::safe_atoi(inifile.getValue(tmp, "xiaxian", "").c_str());
        def->mMax = Utils::safe_atoi(inifile.getValue(tmp, "shangxian", "").c_str());
        
        intimacy_coef.push_back(def);
    }
}

float FriendAwardCoefCfg::getFriendsCountCoef(int friendcount)
{
    for (int i = 0; i < friend_coef.size(); i++) {
        if (friendcount <= friend_coef[i]->mMax) {
            if (friend_coef[i]->mIsShuzu) {
                return friend_coef[i]->m_coef[friendcount - friend_coef[i]->mMin];
            }
            else{
                return friend_coef[i]->m_coef[0];
            }
        }
    }
    
    return 0;
    
}

float FriendAwardCoefCfg::getFriendsIntimacyCoef(int intimacysum)
{
    for (int i = 0; i < intimacy_coef.size(); i++) {
        if (intimacysum <= intimacy_coef[i]->mMax) {
            if (intimacy_coef[i]->mIsShuzu) {
                return intimacy_coef[i]->m_coef[intimacysum - intimacy_coef[i]->mMin];
            }
            else{
                return intimacy_coef[i]->m_coef[0];
            }
        }
    }
    
    return 0;
}


#pragma mark - 神秘副本（双周副本）

vector<int> MysticalCopyListCfgDef::getOpenDays()
{
    vector<int> days;
    days.clear();
    if ( day.empty())
    {
        return days;
    }
    StringTokenizer openDays(day, ";");
    for(int i = 0; i < openDays.count(); i++)
    {
        int dayNum = Utils::safe_atoi(openDays[i].c_str());
        if(dayNum > 0)
        {
            days.push_back(dayNum);
        }
    }
    
    return days;
}

std::map<int, MysticalCopyListCfgDef*> MysticalCopyListCfg::m_copylist;
vector<int> MysticalCopyListCfg::m_noOpenCopyList;

int MysticalCopyListCfg::listcount = 0;
int MysticalCopyListCfg::replacePeriod = 0;
int MysticalCopyListCfg::activeGroup;
int MysticalCopyListCfg::copyCountPerGroup;
int MysticalCopyListCfg::existTime;

void MysticalCopyListCfg::load(const char *fullPath)
{
    GameInifile inifile(fullPath);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("mysticalcopylistMultiLanguage.ini") == "mysticalcopylistMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("mysticalcopylistMultiLanguage.ini"));
    }
    //GameInifile MLIniFile(Cfgable::getFullFilePath("mysticalcopylistMultiLanguage.ini"));
    m_copylist.clear();
    
    int count = inifile.getValueT("global", "listcount", 0);
    string replaceStr = inifile.getValue("global", "replace_period", "");
    
    assert(!replaceStr.empty());
    
    StringTokenizer replace(replaceStr, "*", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
    replacePeriod = 1;
    for (int i = 0; i < replace.count(); i++) {
        replacePeriod *= Utils::safe_atoi(replace[i].c_str());
    }
    listcount = count;
    assert(replacePeriod);
    
    activeGroup = inifile.getValueT("global", "active_group", 0);
    copyCountPerGroup = inifile.getValueT("global", "copys_pergroup", 0);
    existTime = inifile.getValueT("global", "copy_existsec", 0);
    
    for (int i = 1; i <= count; i++) {
        char buf[128] = "";
        string key = Utils::itoa(i, 10, buf);
        string listNameId = inifile.getValue(key.c_str(), "listname_id", "");
        //by lihengjin
        string listNameStr;
        if(inifile.exists(key.c_str(), "listname"))
        {
            listNameStr = inifile.getValue(key.c_str(), "listname", "");
        }else
        {
            listNameStr = MLIniFile.getValue(listNameId, "desc");
        }
        //string listNameStr = MLIniFile.getValue(listNameId, "desc");
        string listname = listNameStr;//inifile.getValue(key.c_str(), "listname", "");
        string copylist = inifile.getValue(key.c_str(), "copylist", "");
        string actIcon = inifile.getValue(key.c_str(), "icon", "");
        string openDays = inifile.getValue(key.c_str(), "day");
        int openLvl = inifile.getValueT(key.c_str(), "openLvl", 0);
        int copyType = inifile.getValueT(key.c_str(), "copyType", 0);
        
        if (listname.empty() || copylist.empty() || actIcon.empty()) {
            assert(0);
        }
        
        MysticalCopyListCfgDef* def = new MysticalCopyListCfgDef;
        def->act_icon = actIcon;
        def->copylist = copylist;
        def->listname = listname;
        def->day = openDays;
        def->openLvl = openLvl;
        def->copyType = copyType;
        
        m_copylist[i] = def;
    }
}

MysticalCopyListCfgDef* MysticalCopyListCfg::getCfg(int index)
{
    std::map<int, MysticalCopyListCfgDef*>::iterator iter = m_copylist.find(index);
    if (iter != m_copylist.end()) {
        return iter->second;
    }
    return NULL;
}

vector<int> MysticalCopyListCfg::getCopyID(int index)
{
    MysticalCopyListCfgDef* _cfg = getCfg(index);

    vector<int> _copyIDs;
    if (_cfg == NULL) {
        return _copyIDs;
    }
    
    StringTokenizer reToken(_cfg->copylist,";");
    for (int i = 0; i < reToken.count(); i++)
    {
        int _copyID = atoi(reToken[i].c_str());
        _copyIDs.push_back(_copyID);
    }
    return _copyIDs;
}

//--------处理幻兽大本营 未开放 list-----------
vector<int> MysticalCopyListCfg::getNoOpenList()
{
    return m_noOpenCopyList;
}
int MysticalCopyListCfg::getNoOpenListSize()
{
    return m_noOpenCopyList.size();
}

void MysticalCopyListCfg::findNoOpenList(vector<int> &vec)
{
//    for (int i=0; i<vec.size(); i++) {
//        printf("%d------+++------++++",vec[i]);
//    }
    m_noOpenCopyList.clear();
    vector<int>::iterator findTmp;
    map<int, MysticalCopyListCfgDef*>::iterator it;
    
    if (m_copylist.empty()) {
        return;
    }
    vector<int> tmpVec;
    tmpVec.clear();
    for (it=m_copylist.begin(); it!=m_copylist.end(); it++) {
        tmpVec.push_back(it->first);
    }
    if (vec.empty()) {
        m_noOpenCopyList=tmpVec;
        return;
    }
	int index = std::find(tmpVec.begin(), tmpVec.end(), vec[0]) - tmpVec.begin();
	int size = tmpVec.size();
	for (int i = 0; i < size; i++)
	{
		int temp = tmpVec[(i + index) % size];
		if (std::find(vec.begin(), vec.end(), temp) != vec.end())
		{
			continue;
		}
		m_noOpenCopyList.push_back(temp);
	}

    
//    printf("////%d-----+++-----+++\n",m_noOpenCopyList.size());
//    for (int i=0; i<m_noOpenCopyList.size(); i++) {
//        printf("///////-------\\\\\\%d------+++------++++",m_noOpenCopyList[i]);
//    }
//
}

//-------------------


std::map<int, MysticalCopyDef*> MysticalCopyCfg::m_MysticalCopyData;

void MysticalCopyCfg::load(const char *fullPath)
{
    GameInifile inifile(fullPath);
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    for (std::list<std::string>::iterator iter = sections.begin() ; iter != sections.end(); iter++)
    {
        int copyid = Utils::safe_atoi((*iter).c_str());
        if (copyid <= 0) {
            continue;
        }
        
        MysticalCopyDef* copydata = new MysticalCopyDef;
        
        copydata->miniLvl = inifile.getValueT(*iter,"minLv",0);
        
        copydata->bossExistProps = inifile.getValueT(*iter, "bossappearprops", 0);
        
        string bossdata = inifile.getValue(*iter, "mysticalboss", "");
        StringTokenizer boss(bossdata, ";");
        
        if (boss.count() == 3) {
            copydata->m_boss.m_Monsterid = Utils::safe_atoi(boss[0].c_str());
            copydata->m_boss.x = Utils::safe_atoi(boss[1].c_str());
            copydata->m_boss.y = Utils::safe_atoi(boss[2].c_str());
        }
        
        copydata->m_boss.m_Existtime = inifile.getValueT(*iter, "bossexisttime", 0);
        copydata->bossAward = inifile.getValue(*iter, "bossaward", "");
        copydata->m_boss.talkingid = inifile.getValueT(*iter, "dialog_id", 0);
        
        string bossDiscountStr = inifile.getValue(*iter, "boss_discount", "");
        StringTokenizer bossDiscount(bossDiscountStr, ";");
        for (int i = 0; i < bossDiscount.count(); i++) {
            copydata->m_BossApearDiscount.push_back(atof(bossDiscount[i].c_str()));
        }
        
        int monstercount = inifile.getValueT(*iter, "monstercount", 0);
        copydata->monsternum = monstercount;
        for (int i = 1; i <= monstercount; i++) {
            char buf[32] = "";
            snprintf(buf, 32, "monster%d", i);
            string monstersVal = inifile.getValue(*iter, buf, "");
            
            StringTokenizer monsters(monstersVal, ";");
            if(monsters.count() < 4)
            {
                assert(0);
            }
            
            int monsterid = Utils::safe_atoi(monsters[0].c_str());
            assert(monsterid > 0);
            
            int x = Utils::safe_atoi(monsters[1].c_str());
            int y = Utils::safe_atoi(monsters[2].c_str());
            int energe_consum = Utils::safe_atoi(monsters[3].c_str());
            
            snprintf(buf, 32, "existtime%d", i);
            int existtime = inifile.getValueT(*iter, buf, 0);
            
            MysticalMonsterDef monster;
            monster.m_Monsterid = monsterid;
            monster.m_Existtime = existtime;
            monster.x = x;
            monster.y = y;
            monster.energy_consum = energe_consum;
            
            snprintf(buf, 32, "appearprops%d", i);
            int appearprop = inifile.getValueT(*iter, buf, 0);
            
            copydata->m_AppearProps.push_back(appearprop);
            copydata->m_Monsters.push_back(monster);
        }
        
        string monsterDiscountStr = inifile.getValue(*iter, "monster_discount", "");
        StringTokenizer monsterDiscount(monsterDiscountStr, ";");
        for (int i = 0; i < monsterDiscount.count(); i++) {
            copydata->m_MonsterAppearDiscount.push_back(atof(monsterDiscount[i].c_str()));
        }
        
        int awardNum = inifile.getValueT(*iter, "awardnum", 0);
        for (int i = 1; i <= awardNum; i++) {
            char buf[32] = "";
            snprintf(buf, 32, "award%d", i);
            
            string award = inifile.getValue(*iter, buf, "");
            copydata->m_award.push_back(award);
            
            snprintf(buf, 32, "awardprop%d", i);
            int awardprops = inifile.getValueT(*iter, buf, 0);
            copydata->m_AwardProps.push_back(awardprops);
        }
        
        m_MysticalCopyData[copyid] = copydata;
    }
}

void MysticalCopyCfg::getMysticalMonsters(int copyid, int times, std::vector<MysticalMonsterDef>& monsters, MysticalMonsterDef& boss)
{
    std::map<int, MysticalCopyDef*>::iterator iter = m_MysticalCopyData.find(copyid);
    if (iter != m_MysticalCopyData.end()) {
        if (iter->second) {
            iter->second->getMysticalMonsters(times, monsters, boss);
        }
    }
}


//百分百出现隐藏 BOSS 与 哥布林
void MysticalCopyCfg::getMysticalMonsters(int copyid, std::vector<MysticalMonsterDef>& monsters, MysticalMonsterDef& boss)
{
    std::map<int, MysticalCopyDef*>::iterator iter = m_MysticalCopyData.find(copyid);
    if (iter != m_MysticalCopyData.end()) {
        if (iter->second) {
            iter->second->getMysticalMonsters(monsters, boss);
        }
    }
}

MysticalCopyDef* MysticalCopyCfg::getcfg(int copyid)
{
    std::map<int, MysticalCopyDef*>::iterator iter = m_MysticalCopyData.find(copyid);
    if (iter != m_MysticalCopyData.end()) {
        return iter->second;
    }
    return NULL;
}

#pragma mark - 小助手
vector<HelperCfgDef*> HelperCfg::mHelperCfgs;
vector<HelperMenuDef*> HelperCfg::mHelperMenus;
void HelperCfg::load(std::string fullPath)
{
    mHelperCfgs.clear();
    mHelperMenus.clear();
    
    try
    {
        //lihengjin
        GameInifile MLIniFile;
        if(!(Cfgable::getFullFilePath("HelperMultiLanguage.ini") == "HelperMultiLanguage.ini"))
        {
            MLIniFile = GameInifile(Cfgable::getFullFilePath("HelperMultiLanguage.ini"));
        }
        //GameInifile MLIniFile(Cfgable::getFullFilePath("HelperMultiLanguage.ini"));
        GameInifile inifile(fullPath);
        std::list<string> sections;
        
        string btn_name_id = inifile.getValue("Button", "Btn_name_id");
        //by lihengjin
        string menuName;
        if(inifile.exists("Button", "Btn_name" ))
        {
            menuName = inifile.getValue("Button", "Btn_name");
        }else
        {
            menuName =  MLIniFile.getValue(btn_name_id, "desc");
        }
        //string menuName = MLIniFile.getValue(btn_name_id, "desc");//inifile.getValue("Button", "Btn_name");
        string menuType = inifile.getValue("Button", "Btn_id");
        StringTokenizer tokens1(menuName, ";");
        StringTokenizer tokens2(menuType, ";");
        for(int i = 0; i < tokens1.count(); i++)
        {
            HelperMenuDef* data = new HelperMenuDef();
            data->menuName = tokens1[i];
            data->menuType = atoi(tokens2[i].c_str());
            mHelperMenus.push_back(data);
        }
        
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin() ; iter != sections.end(); iter++)
        {
            if (*iter == "Button") {
                continue;
            }
            HelperCfgDef* data = new HelperCfgDef();
            string explanation_id = inifile.getValue(*iter, "Explanation_id");
            //by lihengjin
            string str_explanation;
            if(inifile.exists(*iter, "Explanation"))
            {
                str_explanation = inifile.getValue(*iter, "Explanation", "");
            }else
            {
                str_explanation = MLIniFile.getValue(explanation_id, "desc", "");
            }
            data->eventDes = str_explanation;//inifile.getValue(*iter, "Explanation");
            data->eventId = inifile.getValueT(*iter, "id", 0);
            string activity_name_id = inifile.getValue(*iter, "Activity_name_id");
            //by lihengjin
            string str_activity_name;
            if(inifile.exists(*iter, "Activity_name"))
            {
                str_activity_name = inifile.getValue(*iter, "Activity_name", "");
            }else
            {
                str_activity_name = MLIniFile.getValue(activity_name_id, "desc");
            }
            data->eventName = str_activity_name;//inifile.getValue(*iter, "Activity_name");
            data->eventRes = inifile.getValue(*iter, "Activity_head");
            string open_time_id = inifile.getValue(*iter, "Open_time_id");
            //by lihengjin
            string str_open_time;
            if(inifile.exists(*iter, "Open_time"))
            {
                str_open_time = inifile.getValue(*iter, "Open_time", "");
            }else
            {
                str_open_time = MLIniFile.getValue(open_time_id, "desc");
            }
            data->eventTime = str_open_time;//inifile.getValue(*iter, "Open_time");
            string comments_name_id = inifile.getValue(*iter, "Comments_name_id");
            //by lihengjin
            string str_comments_name;
            if(inifile.exists(*iter, "Comments_name"))
            {
                str_comments_name = inifile.getValue(*iter, "Comments_name");
            }else
            {
                str_comments_name = MLIniFile.getValue(comments_name_id, "desc");
            }
            data->eventProp = str_comments_name;//inifile.getValue(*iter, "Comments_name");
            data->eventStars = inifile.getValueT(*iter, "Comments_lvl", 0);
            data->eventType = inifile.getValueT(*iter, "Btn_id", 0);
            data->eventGoto = inifile.getValueT(*iter, "Goto_Activity", 0);
            data->eventLvl = inifile.getValueT(*iter, "Limit_lvl", 0);
            data->eventSclae = inifile.getValueT(*iter, "scaling_head", 1);
            mHelperCfgs.push_back(data);
        }
    }catch(...)
    {
        throw 1;
    }
}

vector<HelperCfgDef*> HelperCfg::getHelperCfgListByType(int nType)
{
    vector<HelperCfgDef*> cfgs;
    cfgs.clear();
    
    vector<HelperCfgDef*>::iterator it;
    vector<HelperCfgDef*>::iterator itBegin = mHelperCfgs.begin();
    vector<HelperCfgDef*>::iterator itEnd = mHelperCfgs.end();
    
    for (it = itBegin; it != itEnd; ++it) {
        if ((*it)->eventType == nType) {
            cfgs.push_back((*it));
        }
    }
    
    return cfgs;
}

HelperMenuDef* HelperCfg::getHelperMenuById(int id)
{
    if (id < 0) {
        return NULL;
    }
    if (id >= mHelperMenus.size()) {
        return NULL;
    }
    return mHelperMenus[id];
}

int HelperCfg::getHelperMenuSize()
{
    return mHelperMenus.size();
}

#pragma mark - 本地推送配置表
map<int, LocalNotificationCfgDef*> LocalNotificationCfg::cfg;
void LocalNotificationCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("localNotificationMultiLanguage.ini") == "localNotificationMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("localNotificationMultiLanguage.ini"));
    }
   // GameInifile MLIniFile(Cfgable::getFullFilePath("localNotificationMultiLanguage.ini"));
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin() ; iter != sections.end(); iter++)
    {
        LocalNotificationCfgDef *def = new LocalNotificationCfgDef();
        def->ID = inifile.getValueT(*iter, "id", 0);
        def->name = inifile.getValue(*iter, "name");
        def->isInTime = inifile.getValueT(*iter, "isInTime", 0)==1?true:false;
        def->weekDay = inifile.getValueT(*iter, "weekday", 0);
        def->time = inifile.getValueT(*iter, "time", 0);
        string body_id = inifile.getValue(*iter, "body_id");
        //by lihengjin
        string str_body;
        if(inifile.exists(*iter, "body"))
        {
            str_body  = inifile.getValue(*iter, "body");
        }else
        {
            str_body = MLIniFile.getValue(body_id, "desc");
        }
        def->body =  str_body;//inifile.getValue(*iter, "body");
        string ps_id = inifile.getValue(*iter, "ps_id");
        //by lihengjin
        string str_ps;
        if(inifile.exists(*iter, "ps"))
        {
            str_ps = inifile.getValue(*iter, "ps");
        }else
        {
            str_ps = MLIniFile.getValue(ps_id, "desc");
        }
        def->ps = str_ps;//inifile.getValue(*iter, "ps");
        cfg[def->ID] = def;
    }
}

map<int, LocalNotificationCfgDef*>* LocalNotificationCfg::getCfgList()
{
    return &cfg;
}

LocalNotificationCfgDef* LocalNotificationCfg::getLocalNotificationCfg(int ID)
{
    return cfg[ID];
}

/******************************** petpvp *******************************/
string petPvpType2String(PetPvpType type)
{
    switch (type) {
        case eBronze:
            return "bronze";
        case eSilver:
            return "silver";
        case eGolden:
            return "golden";
        default:
            return "";
    }
}
PetPvpType string2PetPvpType(string str)
{
    return
    (str == "bronze") ? eBronze :
    (str == "silver") ? eSilver :
    (str == "golden") ? eGolden :
    eUnknown;
}
                              
std::vector<PetPvpAwardDef*> PetPvpCfg::mRankAward;
std::vector<PetPvpSectionCfg*> PetPvpCfg::mSectionsInfo;
PetPvpCfgDef PetPvpCfg::mCfgData;

void PetPvpCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("petpvpMultiLanguage.ini") == "petpvpMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("petpvpMultiLanguage.ini"));
    }
   // GameInifile MLIniFile(Cfgable::getFullFilePath("petpvpMultiLanguage.ini"));
    
    mCfgData.mWinGivePoints = inifile.getValueT("global", "win_points", 0);
    mCfgData.mLostGivePoints = inifile.getValueT("global", "lose_points", 0);
    mCfgData.mAwardTimes = inifile.getValueT("global", "award_times", 0);
    mCfgData.mEnemyGetPoints = inifile.getValueT("global", "enemy_points", 0);
    mCfgData.mOpenLvl = inifile.getValueT("global", "open_lvl", 0);
    mCfgData.mPetMaxHpCoef = inifile.getValueT("global", "pet_maxhp_coef", 1.0f);
    mCfgData.mLimitCups = inifile.getValueT("global", "limit_cups", 0);
    
    string petcountstr = inifile.getValue("global", "pets_count");
    StringTokenizer petsCount(petcountstr, ";");
    for (int i = 0; i < petsCount.count(); i++) {
        StringTokenizer pets(petsCount[i], ":");
        if (pets.count() < 2) {
            log_error("petpvp.ini pets_count formate error");
            assert(0);
        }
        
        int lvl = Utils::safe_atoi(pets[0].c_str());
        int petcount = Utils::safe_atoi(pets[1].c_str());
        
        mCfgData.mPetCounts.push_back(make_pair(lvl, petcount));
        
    }
    
    mCfgData.mEnterTime = inifile.getValueT("global", "enter_times", 0);

    //目前3个段
    for (int i = 1; i <= 3; i++) {
        string section = petPvpType2String((PetPvpType)i);
        PetPvpSectionCfg* sectionCfg = new PetPvpSectionCfg;
        sectionCfg->mType = i;
        
        sectionCfg->mFixAwardCups = inifile.getValueT(section, "fix_cups", 0);          //战胜同段位获得的基础奖杯
        sectionCfg->mFormationSize = inifile.getValueT(section, "formation_size", 0);   //阵型中宠物的个数
        
        sectionCfg->mWinHigherCups = inifile.getValueT(section, "win_higher_cups", 0);  //战胜高段位获得的奖杯
        sectionCfg->mWinLowerCups = inifile.getValueT(section, "win_lower_cups", 0);    //战胜低段位获得的奖杯
        sectionCfg->mLostCups = inifile.getValueT(section, "lose_cups", 0);             //战败获得的奖杯
        sectionCfg->mfixAwardPoints = inifile.getValueT(section, "fix_award_point", 0); //周结算的基础分，段位越高基础分越高
        sectionCfg->mIconName = inifile.getValue(section, "icon_name", "");             //图标名称，用于客户端
        sectionCfg->mMinCups = inifile.getValueT(section, "min_cups", 0);               //该段的最小cups（竞技积分）
        sectionCfg->mMinPerCent = inifile.getValueT(section, "min_percent", 0.0f);      //该段最小排名百分比(myrank/playercount)
        
        string name_id = inifile.getValue(section, "name_id");
        //by lihengjin
        string str_name;
        if(inifile.exists(section, "name"))
        {
            str_name = inifile.getValue(section, "name");
        }else
        {
            str_name = MLIniFile.getValue(name_id, "desc");
        }
        sectionCfg->mName = str_name;//inifile.getValue(section, "name");
        sectionCfg->mItemList = inifile.getValue(section, "item_list", "");             //可购买的物品
        
        string desc_id = inifile.getValue(section, "desc_id");
        //by lihengjin
        string str_desc;
        if(inifile.exists(section, "desc"))
        {
            str_desc = inifile.getValue(section, "desc");
        }else
        {
            str_desc = MLIniFile.getValue(desc_id, "desc");
        }
        sectionCfg->mDesc = str_desc;//inifile.getValue(section, "desc", "");
        sectionCfg->mSearchEnemyCost = inifile.getValueT(section, "search_enemy_cost", 0);
        sectionCfg->mRankAdditionCoef = inifile.getValueT(section, "rank_addition_coef", 1.0f);
        mSectionsInfo.push_back(sectionCfg);
    }
    
    int parts = inifile.getValueT("global", "award_parts", 0);
    assert(parts);
    for (int i = 1; i < parts; i++) {
        
        string tmp = Utils::makeStr("awardpart%d",i);
        
        string pointsStr = inifile.getValue(tmp, "give_points", "");
        
        StringTokenizer points(pointsStr, ";");
        
        PetPvpAwardDef* def = new PetPvpAwardDef;
        for (int j = 0; j < points.count(); j++) {
            int val = Utils::safe_atoi(points[j].c_str());
            def->mAwards.push_back(val);
        }
        
        def->mIsShuzu = Utils::safe_atoi(inifile.getValue(tmp, "is_shuzu", "").c_str());
        def->mMinRank = Utils::safe_atoi(inifile.getValue(tmp, "xiaxian", "").c_str());
        def->mMaxRank = Utils::safe_atoi(inifile.getValue(tmp, "shangxian", "").c_str());
        
        mRankAward.push_back(def);
    }
    assert(mRankAward.size());
}

int PetPvpCfg::getRankAward(int playerRank)
{
    int index = 0;

    if (mRankAward.size() <= 0) {
        log_error("PetPvpCfg error");
        return 0;
    }
    
    for (int i = 0; i < mRankAward.size(); i++) {
        index = i;
        if ( mRankAward[i]->mMinRank <= playerRank && playerRank <= mRankAward[i]->mMaxRank ) {
            break;
        }
    }
    
    if (index >= mRankAward.size()) {
        index = mRankAward.size() - 1;
    }
    
    PetPvpAwardDef* def = mRankAward[index];
    
    if (def->mIsShuzu) {
        int tmp = playerRank - def->mMinRank;
        if (tmp >= def->mAwards.size()) {
            tmp = def->mAwards.size() - 1;
        }
        return def->mAwards[tmp];
    }
    else
    {
        return def->mAwards[0];
    }
}

PetPvpSectionCfg* PetPvpCfg::getPetPvpSectionCfg(PetPvpType type)
{
    for (int i = 0; i < mSectionsInfo.size(); i++) {
        if (mSectionsInfo[i]->mType == type) {
            return mSectionsInfo[i];
        }
    }
    
    return NULL;
}

int PetPvpCfg::getLimitCups()
{
    return mCfgData.mLimitCups;
}

int PetPvpCfg::getPetFormSize(int playerLvl)
{
    vector<pair<int, int> >::reverse_iterator iter = mCfgData.mPetCounts.rbegin();
    for (;iter != mCfgData.mPetCounts.rend(); iter++) {
        if (playerLvl >= iter->first) {
            return iter->second;
        }
    }
    
    return 0;
}

int PetPvpCfg::getPetFormLvl(int petNum)
{
    vector<pair<int, int> >::reverse_iterator iter = mCfgData.mPetCounts.rbegin();
    for(; iter != mCfgData.mPetCounts.rend(); iter ++)
    {
        if(petNum == iter->second)
        {
            return iter->first;
        }
    }
    return 0;
}


PetPvpType PetPvpCfg::calPlayerPetpvpType(int playerPoint, int playerGlobalRank, int playerCounts)
{
    check_min(playerCounts, 1);
    
    int maxRank = 0;
    
    std::vector<PetPvpSectionCfg*>::reverse_iterator rIter = mSectionsInfo.rbegin();
    for (; rIter != mSectionsInfo.rend(); rIter++) {
        
        maxRank += playerCounts * (*rIter)->mMinPerCent;
        
        //排名在配置百分比之前的，所以要比它小
        if (playerGlobalRank > maxRank) {
            continue;
        }
        
        if (playerPoint >= (*rIter)->mMinCups) {
            return (PetPvpType)((*rIter)->mType);
        }
    }
    
    return eBronze;
}

/***************************************** 幻兽竞技场积分兑换 *******************************/
std::map<int, PetPvpItem*> PetPvpRedemptionCfg::mItems;
std::map<int, std::vector<PetPvpItem*> > PetPvpRedemptionCfg::mSectionItems;

void PetPvpRedemptionCfg::load(std::string fullpath)
{
    GameInifile inifile(fullpath);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("petpvpredemptionMultiLanguage.ini") == "petpvpredemptionMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("petpvpredemptionMultiLanguage.ini"));
    }
    //GameInifile MLIniFile(Cfgable::getFullFilePath("petpvpredemptionMultiLanguage.ini"));
    
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));

    std::map<int, std::vector<PetPvpItem*> >::iterator itemIter = mSectionItems.end();
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        PetPvpItem* item = new PetPvpItem;
        item->mItemid = inifile.getValueT(*iter, "id", 0);
        assert(item->mItemid > 0);
        string name_id = inifile.getValue(*iter, "name_id");
        //by lihengjin
        string str_name;
        if(inifile.exists(*iter, "name"))
        {
            str_name = inifile.getValue(*iter, "name");
        }else
        {
            str_name = MLIniFile.getValue(name_id, "desc");
        }
        item->mItemName = str_name; //inifile.getValue(*iter, "name", "");
        item->mConsumPoints = inifile.getValueT(*iter, "consum_points", 0);
        string desc_id = inifile.getValue(*iter, "desc_id");
        //by lihengjin
        string str_desc;
        if(inifile.exists(*iter, "desc"))
        {
            str_desc = inifile.getValue(*iter, "desc");
        }else
        {
            str_desc = MLIniFile.getValue(desc_id, "desc");
        }
        item->mDesc = str_desc; //inifile.getValue(*iter, "desc","");
        string needDan = inifile.getValue(*iter, "need_dan", "");
        item->mNeedDan = string2PetPvpType(needDan);
        mItems.insert(make_pair(item->mItemid , item));
        //itemIter  不为end()表示mSectionItems里有数据
        if (itemIter != mSectionItems.end()) {
            //在这个段，加进去
            if (itemIter->first == item->mNeedDan) {
                itemIter->second.push_back(item);
            }
            else //不是这个段，重新查找
            {
                itemIter = mSectionItems.find((int)item->mNeedDan);
                //找不到，就insert;
                if (itemIter == mSectionItems.end()) {
                    
                    vector<PetPvpItem*> newSection;
                    newSection.push_back(item);
                    std::pair<std::map<int, std::vector<PetPvpItem*> >::iterator, bool> ret;
                    ret = mSectionItems.insert(make_pair(item->mNeedDan, newSection));
                    itemIter = ret.first;
                }
                else{
                    itemIter->second.push_back(item);
                }
            }
        }
        else{
            vector<PetPvpItem*> newSection;
            newSection.push_back(item);
            std::pair<std::map<int, std::vector<PetPvpItem*> >::iterator, bool> ret;
            ret = mSectionItems.insert(make_pair((int)item->mNeedDan, newSection));
            itemIter = ret.first;
        }
    }
    
    return;
}

int PetPvpRedemptionCfg::consumPoints(int itemid)
{
    std::map<int, PetPvpItem*>::iterator iter = mItems.find(itemid);
    
    if (iter == mItems.end()) {
        return -1;
    }
    
    return iter->second->mConsumPoints;
}

string PetPvpRedemptionCfg::getItemName(int itemid)
{
    std::map<int, PetPvpItem*>::iterator iter = mItems.find(itemid);
    
    if (iter == mItems.end()) {
        return "";
    }
    
    return iter->second->mItemName;
}

std::map<int, PetPvpItem*>& PetPvpRedemptionCfg::getAllItems()
{
    return mItems;
}

std::vector<PetPvpItem*> PetPvpRedemptionCfg::getItems(PetPvpType type)
{
    std::map<int, std::vector<PetPvpItem*> >::iterator itemIter = mSectionItems.find((int)type);
    std::vector<PetPvpItem*> ret;
    ret.clear();
    if (itemIter != mSectionItems.end()) {
        ret = itemIter->second;
    }
    return ret;
}


map<int, TimeLimitKillCopyCfgDef*> TimeLimitKillCopyCfg::sCfg;

/*
 [500808]
 #持续时间
 lasttime=300
 #刷怪时间
 refreshtime=5
 #最大怪物数量
 maxmonsternum=20
 
 #奖励数量
 awardnum=2
 #奖励积分
 awardscore1=100
 #奖励
 awards1=item 199988*1;gold 100000*1;
 awardscore2=300
 awards2=item 199988*1;gold 100000*1;
 
 #怪物组数 
 groupnum=2
 
 
 #怪物种类
 needscore1=0
 monsternum1=3
 #怪物配置（优先产生触发数量大的怪物）  模版；触发数量；击杀积分；缩放比例；
 monster1_1=324234;1;1;1;
 monster1_2=324234;5;5;2;
 monster1_3=324234;10;10;3;
 
 #怪物种类
 needscore2=100
 monsternum1=3
 #怪物配置（优先产生触发数量大的怪物）  模版；触发数量；击杀积分；缩放比例；
 monster2_1=324234;1;1;1;
 monster2_2=324234;5;5;2;
 monster2_3=324234;10;10;3;
 
 */
bool TimeLimitKillCopyCfg::load(std::string fullpath)
{
    GameInifile ini(fullpath);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("time_limit_activityMultiLanguage.ini") == "time_limit_activityMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("time_limit_activityMultiLanguage.ini"));
    }
   // GameInifile MLIniFile(Cfgable::getFullFilePath("time_limit_activityMultiLanguage.ini"));
    vector<string> sections;
    ini.getSections(std::back_inserter(sections));
    
    for (int i = 0; i < sections.size(); i++)
    {
        TimeLimitKillCopyCfgDef* cfgDef = new TimeLimitKillCopyCfgDef;
        cfgDef->mLastTime = ini.getValueT(sections[i], "lasttime", 300);
        cfgDef->mRefreshTime = ini.getValueT(sections[i], "refreshtime", 5);
        cfgDef->mMaxMonsterNum = ini.getValueT(sections[i], "maxmonsternum", 5);
        
        int awardNum = ini.getValueT(sections[i], "awardnum", 0);
        for (int j = 0; j < awardNum; j++)
        {
            string awardScoreKey = strFormat("awardscore%d", j + 1);
            string awardsKey = strFormat("awards%d", j + 1);
            string awardDescId = strFormat("awarddesc%d_id", j +1);
            TimeLimitKillCopyAward award;
            award.score = ini.getValueT(sections[i], awardScoreKey, 0);
            award.award = ini.getValue(sections[i], awardsKey);
            string desc_id = ini.getValue(sections[i], awardDescId);
            //by lihengjin
            string awardDescId_no_mul = strFormat("awarddesc%d", j +1);
            string str_desc;
            if(ini.exists(sections[i], awardDescId_no_mul))
            {
                str_desc = ini.getValue(sections[i], awardDescId_no_mul);
            }else
            {
                str_desc = MLIniFile.getValue(desc_id, "desc");
            }
            award.desc  = str_desc;
            cfgDef->mAwards.push_back(award);
        }
        
        int groupNum = ini.getValueT(sections[i], "groupnum", 0);
        for (int j = 0; j < groupNum; j++)
        {
            string needScoreKey = strFormat("needscore%d", j + 1);
            int needScore = ini.getValueT(sections[i], needScoreKey, 0);
            
            string monsterNumKey = strFormat("monsternum%d", j + 1);
            int monsterNum = ini.getValueT(sections[i], monsterNumKey, 0);
            
            cfgDef->mGroupScores.push_back(needScore);
            cfgDef->mGroupMonsters.push_back(vector<TlkMonsterCfgDef>());
            for (int k = 0; k < monsterNum; k++)
            {
                string monsterKey = strFormat("monster%d_%d", j + 1, k + 1);
                string monsterStr = ini.getValue(sections[i], monsterKey);
                
                vector<string> detail = StrSpilt(monsterStr, ";");
                
                TlkMonsterCfgDef monster;
                monster.mMonsterMod = Utils::safe_atoi(detail[0].c_str(), 0);
                monster.mUnitNum = Utils::safe_atoi(detail[1].c_str(), 0);
                monster.mScore = Utils::safe_atoi(detail[2].c_str(), 0);
                monster.mScale = atof(detail[3].c_str());
                
                cfgDef->mGroupMonsters[j].push_back(monster);
            }
        }
        
        sCfg.insert(make_pair(Utils::safe_atoi(sections[i].c_str()), cfgDef));
    }
    
    return true;
}


vector<int> TimeLimitKillCopyCfg::getAllMonsterModByMap(int copyid)
{
    vector<int> monsterMod;
    monsterMod.clear();
    TimeLimitKillCopyCfgDef * def = getCfg(copyid);
    if(def != NULL)
    {
        for (int i = 0; i < def->mGroupMonsters.size(); i++)
        {
            for(int j = 0; j < def->mGroupMonsters[i].size(); j++)
            {
                monsterMod.push_back(def->mGroupMonsters[i][j].mMonsterMod);
            }
        }
    }
    return monsterMod;

}
TlkMonsterCfgDef * TimeLimitKillCopyCfg::getTlkMonsterCfg(int copyId, int mod)
{
    vector<int> monsterMod;
    monsterMod.clear();
    TimeLimitKillCopyCfgDef * def = getCfg(copyId);
    if(def != NULL)
    {
        for (int i = 0; i < def->mGroupMonsters.size(); i++)
        {
            for(int j = 0; j < def->mGroupMonsters[i].size(); j++)
            {
                if(def->mGroupMonsters[i][j].mMonsterMod == mod)
                {
                    return &(def->mGroupMonsters[i][j]);
                }
                
                monsterMod.push_back(def->mGroupMonsters[i][j].mMonsterMod);
            }
        }
    }
    return NULL;
}


#pragma mark - 名人堂配置表
map<int, FamousCfgDef*> FamousCfg::cfg;
map<int,FamousDungeCfgDef*> FamousCfg::famousdungecfg;

FamousCfgDef* FamousCfg::getFamousCfg(int _id){
    if( cfg.find(_id) == cfg.end() ){
        return NULL;
    }else{
        return cfg[_id];
    }
}

FamousDungeCfgDef* FamousCfg::getFamousDungeCfg(int _wave)
{
    if( famousdungecfg.find(_wave) == famousdungecfg.end() ){
        return NULL;
    }else{
        return famousdungecfg[_wave];
    }
}

FamousCfgDef* FamousCfg::getFamousCfgNew(int _job, int _startLayer, int _pos)
{
    
    int roleid = (_startLayer + (FamousDataCfg::mData.famesNeedPassLayers - _pos)) * 10 + _job;
    
    return FamousCfg::getFamousCfg(roleid);
    
    return NULL;
}

void FamousCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++) {
        int id = inifile.getValueT(*iter,"id",0);
//        if(id <1000) // 因为原来只有10层，现在要动态配置层数，但是最高还是只能1000层
        if(id < 3000)
        {
            FamousCfgDef* tmp = new FamousCfgDef;
            tmp->_id = inifile.getValueT(*iter,"id",0);
            tmp->default_name = inifile.getValue(*iter, "default_name", "");
            tmp->default_lvl = inifile.getValueT(*iter,"default_lvl",0);
            tmp->default_roletplt = inifile.getValueT(*iter,"default_roletplt",0);
            tmp->hp = inifile.getValueT(*iter,"hp",0);
            tmp->atk = inifile.getValueT(*iter,"atk",0);
            tmp->def = inifile.getValueT(*iter,"def",0);
            tmp->hit = inifile.getValueT(*iter, "hit", 0.0);  //命中
            tmp->dodge = inifile.getValueT(*iter, "dodge",0.0 );//闪避
            tmp->cri = inifile.getValueT(*iter, "cri", 0.0);//暴击
            tmp->default_weapon = inifile.getValueT(*iter, "weaponQua", 0);//武器品质
            for (int i = 0; i < SKILL_EQUIT_NUM; i++) {        
                string key = Utils::makeStr("skill%d", i);
                int skillid = Utils::safe_atoi(inifile.getValue(*iter,key,"").c_str());
                if (skillid) {
                    tmp->skills.push_back(skillid);
                }
            }
            cfg[tmp->_id] = tmp;
        }
//        else if(id>1000 && id <2000)
        else if(id > 10000)
        {
            FamousDungeCfgDef* tmp = new FamousDungeCfgDef;
            tmp->wave = inifile.getValueT(*iter,"id",0) - 10000;
            tmp->dungeName = inifile.getValue(*iter, "dunge_name", "");
            tmp->limitTime = inifile.getValueT(*iter,"limit_time",0);
            tmp->award = inifile.getValue(*iter, "award", "");
            famousdungecfg[tmp->wave] = tmp;
        }
    }
}

FamousDataCfgDef FamousDataCfg::mData;
map<int, FamousHeroBaseDef*> FamousDataCfg::mFamousHeroBaseDefs;

void FamousDataCfg::load(std::string fullpath)
{
    GameInifile inifile(fullpath);
    
    mData.fameHallList = inifile.getValue("global", "famesHall_list", "");
    mData.famesHallLayers = inifile.getValueT("global", "famesHall_layers", 0);
    mData.famesChallengeLayers = inifile.getValueT("global", "challenge_layers", 0);
    mData.famesNeedPassLayers = inifile.getValueT("global", "pass_layers", 0);
    mData.famesNextAddLayers = inifile.getValueT("global", "add_layers", 0);
    
    StringTokenizer token(mData.fameHallList, ";");
    if(token.count() == 0)
        assert(0);
    StringTokenizer fameHall(token[0], ",");
    if(fameHall.count() < mData.famesNeedPassLayers)
        assert(0);
    
    if(mData.famesHallLayers == 0 || mData.famesNeedPassLayers == 0
       || mData.famesNextAddLayers == 0)
        assert(0);
    
    mFamousHeroBaseDefs.clear();
    
    // 将配置表英雄的基本信息统一放在这里，其他有变化的属性在FamousCfg里读取
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        if(strcmp((*iter).c_str(), "global"))
        {
            FamousHeroBaseDef* def = new FamousHeroBaseDef();
            def->_id = inifile.getValueT(*iter, "id", 0);
            def->_name = inifile.getValue(*iter, "default_name");
            def->_roleTplt = inifile.getValueT(*iter, "default_roletplt", 0);
            def->_weaponQue = inifile.getValueT(*iter, "weaponQua", 0);
            mFamousHeroBaseDefs.insert(make_pair(def->_id, def));
        }
    }
}

FamousHeroBaseDef* FamousDataCfg::getFamousHeroBaseDef(int _job, int _pos)
{
    int needLayers = FamousDataCfg::mData.famesNeedPassLayers;
    
    string index = Utils::makeStr("%d%d", (needLayers - _pos + 1), _job);
    int roleid = Utils::safe_atoi(index.c_str(), 0);
    return getFamousHeroBaseDef(roleid);
    return NULL;
}

FamousHeroBaseDef* FamousDataCfg::getFamousHeroBaseDef(int _id)
{
    if(mFamousHeroBaseDefs.find(_id) != mFamousHeroBaseDefs.end())
        return mFamousHeroBaseDefs[_id];
    
    return NULL;
}

vector<TicketTypeDef *>  LimitActivityCfg::ticketTypeDatas;
map<int,vector<LimitActivityDef *> > LimitActivityCfg::limitActivityDatas;

void LimitActivityCfg::load(std::string fullpath)
{
    GameInifile inifile(fullpath);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("ticket_activityMultiLanguage.ini") == "ticket_activityMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("ticket_activityMultiLanguage.ini"));
    }
    //GameInifile MLIniFile(Cfgable::getFullFilePath("ticket_activityMultiLanguage.ini"));
    string activityType = inifile.getValue("ticket_activity","activity_id");
    vector<string > activityStr = StrSpilt(activityType, ";");
    for(int i=0;i < activityStr.size();i++)
    {
        TicketTypeDef * ticketData = new TicketTypeDef;
        ticketData->activityid = atoi(activityStr[i].c_str());
        string info_id = inifile.getValue(activityStr[i].c_str(),"info_id");
        //by lihengjin
        string str_info;
        if(inifile.exists(activityStr[i].c_str(), "info"))
        {
            str_info = inifile.getValue(activityStr[i].c_str(),"info");
        }else
        {
            str_info =  MLIniFile.getValue(info_id, "desc");
        }
        ticketData->activityDesc = str_info;//inifile.getValue(activityStr[i].c_str(),"info");
        
        string activity_name_id = inifile.getValue(activityStr[i].c_str(), "activity_name_id");
        //by lihengjin
        string str_activity_name;
        if(inifile.exists(activityStr[i].c_str(), "activity_name"))
        {
            str_activity_name = inifile.getValue(activityStr[i].c_str(), "activity_name");
        }else
        {
            str_activity_name = MLIniFile.getValue(activity_name_id, "desc");
        }
        ticketData->activityName = str_activity_name; //inifile.getValue(activityStr[i].c_str(), "activity_name");
        ticketTypeDatas.push_back(ticketData);
        
        string allMapIDs = inifile.getValue(activityStr[i].c_str(),"map_list");

        vector<string > mapIDStr = StrSpilt(allMapIDs, ";");
        
        //读每个地图的详细信息
        vector<LimitActivityDef *> activityDatas;
        for(int j=0;j< mapIDStr.size();j++)
        {
            LimitActivityDef * def = new LimitActivityDef;
            def->mapid = atoi(mapIDStr[j].c_str());
            def->name = inifile.getValue(mapIDStr[j].c_str(), "name");
            def->minLvl = inifile.getValueT(mapIDStr[j].c_str(), "minlvl", 0);
            string costStr = inifile.getValue(mapIDStr[j].c_str(), "cost");
            def->costItmem = StrSpilt(costStr,";");
            activityDatas.push_back(def);
        }
        
        limitActivityDatas[ticketData->activityid] = activityDatas;
        
    }
}

TicketTypeDef * LimitActivityCfg::getTicketTypeByActivityID(int activityID)
{
    for(int i=0;i < ticketTypeDatas.size();i++)
    {
        if(ticketTypeDatas[i]->activityid == activityID)
        {
            return ticketTypeDatas[i];
        }
    }
    return NULL;
}
vector<LimitActivityDef *> LimitActivityCfg::getAllLimitActivityDatas(int activityID)
{
    map<int, vector<LimitActivityDef *> >::iterator iter = limitActivityDatas.find(activityID);
    if(iter != limitActivityDatas.end())
    {
        return iter->second;
    }
    vector<LimitActivityDef *> datas;
    datas.clear();
    return datas;
}
LimitActivityDef * LimitActivityCfg::getLimitByMapID(int mapID)
{
    map<int, vector<LimitActivityDef *> >::iterator iter = limitActivityDatas.begin();
    
    for(; iter != limitActivityDatas.end(); iter ++)
    {
        vector<LimitActivityDef *> datas = iter->second;
        for(int i=0; i< datas.size() ;i++)
        {
            if(datas[i]->mapid == mapID)
            {
                return datas[i];
            }
        }
    }
    return NULL;
}
int LimitActivityCfg::getActivityIdByTicketId(int ticketId)
{
    map<int, vector<LimitActivityDef *> >::iterator iter = limitActivityDatas.begin();
    for(; iter != limitActivityDatas.end(); iter++)
    {
        vector<LimitActivityDef *> datas = iter->second;
        for (int i=0; i < datas.size(); i++) {
            std::vector<std::string> costItmem = datas[i]->costItmem;
            if( costItmem.size() > 0)
            {
                std::vector<std::string>  tickItmem =  StrSpilt(costItmem[0]," ");
                if( tickItmem.size() >1)
                {
                    std::vector<std::string>  tickValueItmem =  StrSpilt(tickItmem[1],"*");
                    if( tickValueItmem.size() > 0)
                    {
                        if (std::atoi(tickValueItmem[0].c_str()) == ticketId){
                            return iter->first;
                        }
                    }
                   
                }
               
            }
            
        }
        
    }
  return 0 ;
    
};

#pragma mark -------------------防作弊相关配置表------------------
float AntiClientCheatCfg::sHeatBeatIntervalTime = 5.0;
int AntiClientCheatCfg::sAcclerateKickSecond = 5;// 客户端比服务器快x秒被踢
int AntiClientCheatCfg::sFoceClientSyncSecond = 30; // 客户端时间比服务器小x秒,强制同步客户端时间
int AntiClientCheatCfg::sSyncClientInterval=60; // 同步客户端间隔时间
float AntiClientCheatCfg::sCommunicationTime = 3.0;
float AntiClientCheatCfg::sSyncNetworkStuckTime = 0.12f;

void AntiClientCheatCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        sHeatBeatIntervalTime = inifile.getValueT(*iter,"heart_beat_time",5.0f);
        sAcclerateKickSecond = inifile.getValueT(*iter,"accelerate_kick_second",5);
        sFoceClientSyncSecond = inifile.getValueT(*iter,"force_client_sync_second",30);
        sSyncClientInterval = inifile.getValueT(*iter,"sync_client_second",60);
        sCommunicationTime = inifile.getValueT(*iter, "communication_time", 3.0);
        sSyncNetworkStuckTime = inifile.getValueT(*iter, "sync_network_stuck_time", 0.12f);
        assert( sAcclerateKickSecond > 0 && sFoceClientSyncSecond > 0 && sSyncClientInterval>1);
    }
}

#pragma mark -----------------通关副本战斗力验证配置表-------------
std::vector<CompatPowerVerifyCfgDef*> CompatPowerVerifyCfg::mCfg;

void CompatPowerVerifyCfg::load(std::string fullpath)
{
    mCfg.clear();
    
    GameInifile inifile(fullpath);
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        CompatPowerVerifyCfgDef* def = new CompatPowerVerifyCfgDef;
        def->minPercent = inifile.getValueT(*iter, "min", 0);
        def->maxPercent = inifile.getValueT(*iter, "max", 0);
        def->timeMulti = inifile.getValueT(*iter, "time_multi", 1.0f);
        def->cheatPercent = inifile.getValueT(*iter, "cheat_prob", 0.0);
        mCfg.push_back(def);
    }
}

CompatPowerVerifyCfgDef* CompatPowerVerifyCfg::getCfg(int cmpPercent)
{
    for (int i = 0; i < mCfg.size(); i++) {
        if ( cmpPercent < mCfg[i]->maxPercent && cmpPercent >= mCfg[i]->minPercent) {
            return mCfg[i];
        }
    }
    
    return NULL;
}

#pragma mark -------------------虚拟物品资源配置------------------
std::map<std::string, moneyRes*> moneyResCfg::mMoneyResMap;
void moneyResCfg::load(std::string fullpath)
{
    mMoneyResMap.clear();
    
    GameInifile inifile(fullpath);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("moneyMultiLanguage.ini") == "moneyMultiLanguage.ini"))
    {
        MLIniFile = GameInifile(Cfgable::getFullFilePath("moneyMultiLanguage.ini"));
    }
    //GameInifile MLIniFile(Cfgable::getFullFilePath("moneyMultiLanguage.ini"));
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        moneyRes* res = new moneyRes();
        res->mId = inifile.getValueT(*iter, "id", 0);
        string name_id = inifile.getValue(*iter, "name_id");
        //by lihengjin
        string str_name;
        if(inifile.exists(*iter, "name"))
        {
            str_name = inifile.getValue(*iter, "name");
        }else
        {
            str_name = MLIniFile.getValue(name_id, "desc");
        }
        res->mName = str_name;//inifile.getValue(*iter, "name");
        res->mMoneyType = inifile.getValue(*iter, "moneytype");
        res->mType = inifile.getValueT(*iter, "type", 0);
        res->mIcon = inifile.getValue(*iter, "icon");
        
        mMoneyResMap.insert(std::make_pair(res->mMoneyType, res));
    }
}
std::string moneyResCfg::getIconByName(std::string sName)
{
    std::string iconRes = "";
    if (mMoneyResMap.find(sName) != mMoneyResMap.end())
    {
        iconRes = Utils::makeStr("%s%s", mMoneyResMap[sName]->mIcon.c_str(), ".png");
    }
    return iconRes;
}
moneyRes* moneyResCfg::getMoneyRes(std::string sName)
{
    if (mMoneyResMap.find(sName) != mMoneyResMap.end()) {
        return mMoneyResMap[sName];
    }
    return NULL;
}
std::string moneyResCfg::getMoneyName(std::string sName)
{
    if (mMoneyResMap.find(sName) != mMoneyResMap.end())
    {
        return mMoneyResMap[sName]->mName;
    }
    return "";
}
map<int,int> autoBattleCfg::autoBattleDatas;

void autoBattleCfg::load(std::string fullpath)
{
    GameInifile inifile(fullpath);
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        int copyType = atoi(iter->c_str());
        int vipLvl = inifile.getValueT(*iter, "need_viplvl", 0);
        autoBattleDatas.insert(std::make_pair(copyType, vipLvl));
    }

}
int autoBattleCfg::getOpenBattleVipLvl(int sceneType)
{
    map<int, int>::iterator iter = autoBattleDatas.find(sceneType);
    if(iter != autoBattleDatas.end())
    {
        return iter->second;
    }
    return -1;
}
GameFunctionType StringToFunctionType( std::string str )
{
    return
    (str == "oneEquipNoDrop") ? eOneEquipNoDrop :
    (str == "multipleEquipDrop") ? eMultipleEquipDrop :
    (str == "vipEffectShop") ? eVipEffectShop :
    (str == "vipNoEffectShop") ? eVipNoEffectShop :
    (str == "changeMysticalCopy") ? eChangeMysticalCopy :
    (str == "noChangeMysticalCopy") ? eNoChangeMysticalCopy :
    (str == "noKrLvReward") ? eNoKrLvlRewardTyp :
    (str == "KrLvReward") ? eKrLvlRewardTyp :
    (str == "maxlvlAdd")? eMaxlvlAddType:
    (str == "extlvAdd")? eExtlvAddType:
    (str == "oldSuperActivity")? eOldSuperActivityType:
    (str == "newSuperActivity")? eNewSuperActivityType:
    (str == "serverControl")? eServerControlBBS:
    (str == "ClientControl")? eClientControlBBS:
    (str == "normalCharge")? eNormalCharge:
    (str == "webViewCharge")? eWebViewCharge:
    (str == "broswerCharge")? eBroswerCharge:
    (str == "petMergeList")? ePetMergeList:
    (str == "typeForPetMerger")? eTypeForPetMerger:
	(str == "effectLimitTimeKilled")? eEffectLimitTimeKilled:
	(str == "noEffectLimitTimeKilled")? eNoEffectLimitTimeKilled:
    (str == "friendInvitationOpen") ? eFriendInvitationOpen:
    (str == "friendInvitationClose") ? eFriendInvitationClose:
    eNonType;
}

map<string, int> GameFunctionCfg::functionDatas;
GameFunctionCfgDef GameFunctionCfg::functionData;

void GameFunctionCfg::load(std::string fullpath)
{
    GameInifile inifile(fullpath);
    
    functionData.equipType = StringToFunctionType(inifile.getValue("equip", "state"));
    
    functionData.mysticalcopyType = StringToFunctionType(inifile.getValue("MysticalCopy", "state"));
    
    functionData.vipEffectShopType = StringToFunctionType(inifile.getValue("mySticMerchant", "state"));
    
    functionData.krLvlRewardType = StringToFunctionType(inifile.getValue("msg_award", "state"));
    
    functionData.leveldifferenceType = StringToFunctionType(inifile.getValue("leveldifference", "state"));
    functionData.activitychooseType = StringToFunctionType(inifile.getValue("activitychoose", "state"));
    functionData.bBSUsingType = StringToFunctionType(inifile.getValue("BBSUsingState", "state"));
    functionData.chargeInterface = StringToFunctionType(inifile.getValue("ChargeInterface", "state"));
    functionData.petCanMergeType = StringToFunctionType(inifile.getValue("petCanMerge", "state"));
    functionData.weChatShare = inifile.getValue("WeChatShare", "state");
    functionData.limitTimeKilled = StringToFunctionType(inifile.getValue("time_limit_activity", "state"));
    functionData.friendInvitation = StringToFunctionType(inifile.getValue( "FriendInvitation", "state" ));
    if (functionData.isEmpty()) {
        log_error("load ini error : Game FunctionCfg");
        //assert(0);
    }
}

bool GameFunctionCfg::isWeChatShare()
{
    StringTokenizer reToken(functionData.weChatShare,";");
    for (int i =0; i<reToken.count(); i++) {
        if (GameCfg::clientChannel == reToken[i])
        {
            return true;
        }
    }
    return false;
}

map<string, string> KoreaPlatformJump::platformUrls;
void KoreaPlatformJump::load(std::string fullpath)
{
    GameInifile inifile(fullpath);
    list<string> sections;
    inifile.getSections(back_inserter(sections));
    for (list<string>::iterator itr = sections.begin(); itr != sections.end(); ++itr) {
        string platform = itr->c_str();
        string url = inifile.getValue(*itr, "wangzhi");
        platformUrls.insert(make_pair(platform, url));
    }
}
string KoreaPlatformJump::getUrl(string platform)
{
    string url = "";
    if (platformUrls.find(platform) != platformUrls.end()) {
        url = platformUrls[platform];
    }
    return url;
}

#pragma mark- 主界面 UI 活动图标 位置配置

map<int, UiIconSortCfgDef*> UiIconSortCfg::uiIconData;

bool UiIconSortCfg::load(std::string fullpath)
{
    uiIconData.clear();
    try
    {
        GameInifile inifile(fullpath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter ++)
        {
            if (inifile.exists(*iter))
            {
                UiIconSortCfgDef *def = new UiIconSortCfgDef;
                def->id = inifile.getValueT(*iter, "ID", 0);
                def->img = inifile.getValue(*iter, "image");
                uiIconData[atoi(iter->c_str())] = def;
            }
        }
    }
    catch (...)
    {
        throw 1;
    }
    return true;
}

int UiIconSortCfg::getDataSize()
{
    return uiIconData.size();
}

bool UiIconSortCfg::findUiIconByString(std::string name)
{
    bool exit = false;
    for(std::map<int, UiIconSortCfgDef*>::iterator iter=uiIconData.begin();iter!=uiIconData.end();iter++)
    {
        if (iter->second->img == name) {
            exit = true;
        }
    }
    return exit;
}

std::map<int,UiIconSortCfgDef*> UiIconSortCfg::getUiIconMap()
{
    return uiIconData;
}

vector<NetWorkStateDef *> NetWorkStateCfg::cfg;
bool NetWorkStateCfg::load(std::string fullpath)
{
    GameInifile inifile(fullpath);
    //lihengjin
    GameInifile MLIniFile;
    if(!(Cfgable::getFullFilePath("network_statusMultiLanguage.ini") == "network_statusMultiLanguage.ini"))
    {
        MLIniFile = *new GameInifile(Cfgable::getFullFilePath("network_statusMultiLanguage.ini"));
    }
   // GameInifile MLIniFile(Cfgable::getFullFilePath("network_statusMultiLanguage.ini"));
    std::list<string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        NetWorkStateDef * def = new NetWorkStateDef();
        string valueStr = inifile.getValue(*iter, "delay_range");
        string name_id = inifile.getValue(*iter, "network_info_id");
        // by lihengjin
        string desc;
        if(!inifile.exists(*iter, "network_info"))
        {
            desc = inifile.getValue(*iter, "network_info");
        }else
        {
            desc = MLIniFile.getValue(name_id, "desc");
        }
        //string desc = MLIniFile.getValue(name_id, "desc");
        string colorStr = inifile.getValue(*iter, "info_rgb");
        StringTokenizer token(valueStr,";");
        def->minPing = atoi(token[0].c_str());
        def->maxPing = atoi(token[1].c_str());
        def->desc = desc;
        def->colorStr = colorStr;
        cfg.push_back(def);
    }
    return true;

}
NetWorkStateDef * NetWorkStateCfg::getNetWorkStateDesc(float value)
{
    NetWorkStateDef  *def;
    for (int i=0; i< cfg.size(); i++)
    {
        if((int)value >= cfg[i]->minPing && (int)value < cfg[i]->maxPing)
        {
            def = cfg[i];
            return def;
        }
    }
    return def;
}

#pragma mark- 娱乐活动UI配置
vector<FunnyActCfgDef*> FunnyActCfg::mFunnyActCfgs;

bool FunnyActCfg::load(std::string fullPath){
    
    mFunnyActCfgs.clear();
    try {
        GameInifile inifile(fullPath);
        //lihengjin
        GameInifile MLInifile;
        if(!(Cfgable::getFullFilePath("funny_activityMultiLanguage.ini") == "funny_activityMultiLanguage.ini"))
        {
            MLInifile = GameInifile(Cfgable::getFullFilePath("funny_activityMultiLanguage.ini"));
        }
        //GameInifile MLInifile(Cfgable::getFullFilePath("funny_activityMultiLanguage.ini"));
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter!=sections.end(); iter++) {
            FunnyActCfgDef *data = new FunnyActCfgDef;
            data->funnyActId = inifile.getValueT(*iter, "id", 0);
            //by lihengjin
            string name_id = inifile.getValue(*iter, "name_id");
            string str_name;
            if(inifile.exists(*iter, "name"))
            {
                str_name = inifile.getValue(*iter, "name");
            }else
            {
                str_name = MLInifile.getValue(name_id, "desc");
            }
            data->funnyActName = str_name;
            data->funnyActType = inifile.getValue(*iter, "type");
            data->funnyActImage = inifile.getValue(*iter, "image");
            data->minLvl = inifile.getValueT(*iter, "lvl", 0);
            data->funnyActDesc = inifile.getValue(*iter, "desc");
            mFunnyActCfgs.push_back(data);
        }
    } catch (...) {
        throw 1;
    }

    return true;
}

vector<FunnyActCfgDef*> FunnyActCfg::getFunnyActList(){
    return mFunnyActCfgs;
}

bool FunnyActCfg::checkMinLvl(string actType , int lvl)
{
    if( actType == " ")
    {
        return false;
    }
    vector<FunnyActCfgDef*>::iterator it;
    for (it = mFunnyActCfgs.begin(); it != mFunnyActCfgs.end(); it++)
    {
        if ((*it)->funnyActType == actType)
        {
            if (lvl >= (*it)->minLvl)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

int FunnyActCfg::getFunnyActListSize(){
    return mFunnyActCfgs.size();
}

FunnyActCfgDef* FunnyActCfg::getFunnyActByIndex(int index){
    if (index < mFunnyActCfgs.size()) {
        return mFunnyActCfgs[index];
    }else{
        return new FunnyActCfgDef;
    }
}

#pragma mark- Android加载fanti_config.ini配置文件
//********** android fanti config ************
std::string AndroidFantiCfg::googlePayURL = "";
std::string AndroidFantiCfg::clientPartnerName = "";   //合作商
int AndroidFantiCfg::openMyCardPayNeedLevel = 15;
void AndroidFantiCfg::load(const char* fullPath){
    GameInifile inifile(fullPath);
    googlePayURL = inifile.getValue("root", "googlePayURL", "").c_str();
    openMyCardPayNeedLevel = inifile.getValueT("root", "openMyCardPayNeedLevel", 15);
    clientPartnerName = inifile.getValue("root", GameCfg::clientChannel, "");
}

WardrobeCfgDef::WardrobeCfgDef()
{
    setNeedexp(0);
    setHp(0);
    setDef(0);
    setAtk(0);
    
    setHit(0.0);
    setDodge(0.0);
    setCri(0.0);
    
//    setStre(0);
//    setInte(0);
//    setPhys(0);
//    setCapa(0);
#ifndef CLIENT
    setAccHp(0);
    setAccDef(0);
    setAccAtk(0);
    
    setAccHit(0.0);
    setAccDodge(0.0);
    setAccCri(0.0);
    
    //setAccStre(0);
    //setAccInte(0);
    //setAccPhys(0);
    //setAccCapa(0);
#endif
}

vector<WardrobeCfgDef*> WardrobeCfg::cfg;
int WardrobeCfg::maxLvl;

bool WardrobeCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    maxLvl = inifile.getValueT("common", "maxlvl",0);
 
    string needexpTmp = inifile.getValue("common","needexp","");
    StringTokenizer needexpSt(needexpTmp,";");
    assert(needexpSt.count() == maxLvl);
    
//    string streTmp = inifile.getValue("common","stre","");
//    StringTokenizer streSt(streTmp,";");
//    assert(streSt.count() == maxLvl);
    
//    string inteTmp = inifile.getValue("common","inte","");
//    StringTokenizer inteSt(inteTmp,";");
//    assert(inteSt.count() == maxLvl);
//    
//    string physTmp = inifile.getValue("common","phys","");
//    StringTokenizer physSt(physTmp,";");
//    assert(physSt.count() == maxLvl);
//    
//    string capaTmp = inifile.getValue("common","capa","");
//    StringTokenizer capaSt(capaTmp,";");
//    assert(capaSt.count() == maxLvl);
    
    string atkTmp = inifile.getValue("common","atk","");
    StringTokenizer atkSt(atkTmp,";");
    assert(atkSt.count() == maxLvl);
    
    string defTmp = inifile.getValue("common","def","");
    StringTokenizer defSt(defTmp,";");
    assert(defSt.count() == maxLvl);
    
    string hpTmp = inifile.getValue("common","hp","");
    StringTokenizer hpSt(hpTmp,";");
    assert(hpSt.count() == maxLvl);
    
    string hitTmp = inifile.getValue("common","hit","");
    StringTokenizer hitSt(hitTmp,";");
    assert(hitSt.count() == maxLvl);
    
    string dodgeTmp = inifile.getValue("common","dodge","");
    StringTokenizer dodgeSt(dodgeTmp,";");
    assert(dodgeSt.count() == maxLvl);
    
    string criTmp = inifile.getValue("common","cri","");
    StringTokenizer criSt(criTmp,";");
    assert(criSt.count() == maxLvl);

    WardrobeCfgDef zeroDef;
    WardrobeCfgDef* prevDef = &zeroDef;
    for(int i=0; i< maxLvl;i++)
    {
        WardrobeCfgDef * def = new WardrobeCfgDef;
        def->setNeedexp(atoi(needexpSt[i].c_str()));
        
//        def->setStre(atoi(streSt[i].c_str()));
//        def->setInte(atoi(inteSt[i].c_str()));
//        def->setPhys(atoi(physSt[i].c_str()));
//        def->setCapa(atoi(capaSt[i].c_str()));
        def->setAtk(atoi(atkSt[i].c_str()));
        def->setDef(atoi(defSt[i].c_str()));
        def->setHp(atoi(hpSt[i].c_str()));
        def->setHit(atof(hitSt[i].c_str()));
        def->setDodge(atof(dodgeSt[i].c_str()));
        def->setCri(atof(criSt[i].c_str()));
#ifndef CLIENT
//        def->setAccStre(prevDef->getAccStre() + def->getStre());
//        def->setAccInte(prevDef->getAccInte() + def->getInte());
//        def->setAccPhys(prevDef->getAccPhys() + def->getPhys());
//        def->setAccCapa(prevDef->getAccCapa() + def->getCapa());
        def->setAccAtk(prevDef->getAccAtk() + def->getAtk());
        def->setAccDef(prevDef->getAccDef() + def->getDef());
        def->setAccHp(prevDef->getAccHp() + def->getHp());
        def->setAccHit(prevDef->getAccHit() + def->getHit());
        def->setAccDodge(prevDef->getAccDodge() + def->getDodge());
        def->setAccCri(prevDef->getAccCri() + def->getCri());
        prevDef = def;
#endif
        
        cfg.push_back(def);
    }
    return true;
}

WardrobeCfgDef * WardrobeCfg::getWardrobeCfg(int lvl)
{
    if(lvl <1 || lvl > maxLvl)
    {
        return NULL;
    }
    return cfg[lvl-1];
}
int WardrobeCfg::getMaxLvl()
{
    return maxLvl;
}


#pragma mark - 月卡系统
MonthCardCfgDef::MonthCardCfgDef()
{
    setIndex(0);
    setDayIdx(0);
    setAwardIdx(0);
}
MonthCardCfg::MonthCardCfg()
{
    setNeedRmb(0);
    setTotalCount(0);
    setAwardNum(0);
}
MonthCardCfgDef* MonthCardCfg::getMonthCardDefByIdx(int idx)
{
    if(idx < 0 || idx >= monthCardAwards.size())
        return NULL;
    
    return monthCardAwards[idx];
}

void MonthCardCfg::pushMonthCardDef(MonthCardCfgDef* def)
{
    if(def)
        monthCardAwards.push_back(def);
}

map<int, MonthCardCfg*> MonthCardCfgMgr::monthCardCfgs;

bool MonthCardCfgMgr::load(string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
        {
            if (inifile.exists(*iter))
            {
                MonthCardCfg* cfg = new MonthCardCfg();
                if(cfg == NULL)
                    break;
                cfg->setAwardNum(inifile.getValueT(*iter, "index", 0));
                cfg->setTotalCount(inifile.getValueT(*iter, "totalcount", 0));
                cfg->setNeedRmb(inifile.getValueT(*iter, "needrmb", 0));
                cfg->setStartTime(Utils::parseDate(inifile.getValue(*iter, "startdate").c_str()));
                cfg->setEndTime(Utils::parseDate(inifile.getValue(*iter, "overdate").c_str()));
                
                string awards = inifile.getValue(*iter, "awarditems");
                string idxs = inifile.getValue(*iter, "indexs");
                string days = inifile.getValue(*iter, "days");
                StringTokenizer awardToken(awards, ";");
                StringTokenizer idxToken(idxs, ";");
                StringTokenizer dayToken(days, ";");
                //assert(awardToken.count() == idxToken.count());
                assert(idxToken.count() == dayToken.count());
                int awardCounts = awardToken.count();
                for (int i = 0; i < idxToken.count(); ++i)
                {
                    MonthCardCfgDef* data = new MonthCardCfgDef;
                    data->setDayIdx(Utils::safe_atoi(dayToken[i].c_str()));
                    data->setIndex(Utils::safe_atoi(idxToken[i].c_str()));
                    int index = awardCounts <= i ? (awardCounts - 1) : i;
                    int awarditem = 0;
                    int awardcount = 0;
                    sscanf(awardToken[index].c_str(), "item %d*%d",&awarditem, &awardcount);
                    data->setAwardIdx(awarditem);
                    cfg->pushMonthCardDef(data);
                }
                
                //
                monthCardCfgs.insert(std::make_pair(cfg->getAwardNum(), cfg));
            }
        }
    }
    catch (...)
    {
        throw 1;
    }
    
    return true;
}

MonthCardCfg* MonthCardCfgMgr::getMonthCardCfgById(int id)
{
    map<int, MonthCardCfg*>::iterator itrFind = monthCardCfgs.find(id);
    if(itrFind != monthCardCfgs.end())
        return itrFind->second;
    
    return NULL;
}

MonthCardCfg* MonthCardCfgMgr::getMonthCardCfgByTime(int tm)
{
    map<int, MonthCardCfg*>::iterator itrBegin = monthCardCfgs.begin();
    map<int, MonthCardCfg*>::iterator itrEnd = monthCardCfgs.end();
    if(itrBegin == itrEnd)
        return NULL;
    //int curtime = time(0);
    int start = 0;
    int end = 0;
    MonthCardCfg* cfg = NULL;
    for (; itrBegin != itrEnd; ++itrBegin)
    {
        end = itrBegin->second->getEndTime();
        if(tm > end)
            continue;
        cfg =  abs(start - tm) < end - tm ? cfg : itrBegin->second;
        start = end;
    }
    
    return cfg;
}

map<int, SyncTeamCopyCfgDef*> SyncTeamCopyCfg::mSyncCopys;

void SyncTeamCopyCfg::load(string fullpath)
{
    try
    {
        GameInifile inifile(fullpath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
        {
            if (inifile.exists(*iter))
            {
                int scenemod = inifile.getValueT(*iter, "scene_mod", 0);
                if (scenemod) {
                    SyncTeamCopyCfgDef* def = new SyncTeamCopyCfgDef;
                    
                    def->scenemod = scenemod;
                    def->fortuneAward = inifile.getValueT(*iter, "fortune_award", 0);
                    def->constructionAward = inifile.getValueT(*iter, "construction_award", 0);
                    def->goldAward = inifile.getValueT(*iter, "gold_award", 0);
                    def->battlepointAward = inifile.getValueT(*iter, "battlepoint_award", 0);
                    mSyncCopys[scenemod] = def;
                }
            }
        }
    }
    catch (...)
    {
        
    }
}

SyncTeamCopyCfgDef* SyncTeamCopyCfg::getCfg(int sceneMod)
{
    map<int, SyncTeamCopyCfgDef*>::iterator iter = mSyncCopys.find(sceneMod);
    
    if (iter != mSyncCopys.end()) {
        return iter->second;
    }
    
    return NULL;
}

void SyncTeamCopyCfg::getAllCopyList(vector<int>& outdata)
{
    map<int, SyncTeamCopyCfgDef*>::iterator iter = mSyncCopys.begin();
    map<int, SyncTeamCopyCfgDef*>::iterator endIter = mSyncCopys.end();
    
    for (; iter != endIter; iter++) {
        if (iter->second) {
            outdata.push_back(iter->second->scenemod);
        }
    }
}

SyncTeamCopyFunctionCfgDef SyncTeamCopyFunctionCfg::mCfg;
vector<int> SyncTeamCopyFunctionCfg::mIntimacyBase;
vector<float> SyncTeamCopyFunctionCfg::mAwardMultiple;

void SyncTeamCopyFunctionCfg::load(string fullpath)
{
    GameInifile inifile(fullpath);
    
    try {
        
        mIntimacyBase.clear();
        mAwardMultiple.clear();
        
        mCfg.incomeTimes = inifile.getValueT("common", "income_times", 0);
        mCfg.teamMemberCount = inifile.getValueT("common", "team_member_count", 3);
        
        string intimacyBaseStr = inifile.getValue("common", "intimacy_base", "");
        string multipleStr = inifile.getValue("common", "award_multiple", "");
        
        StringTokenizer intitoken(intimacyBaseStr, ";");
        StringTokenizer multipleToken(multipleStr, ";");
        
        if (intitoken.count() != multipleToken.count()) {
            assert(0);
        }
        
        for (int i = 0; i < intitoken.count() && i < multipleToken.count(); i++) {
            
            int intibase = Utils::safe_atoi(intitoken[i].c_str());
            float multiple = atof(multipleToken[i].c_str());
            
            mIntimacyBase.push_back(intibase);
            mAwardMultiple.push_back(multiple);
            
        }
        
    } catch (...) {
        
    }
}

int SyncTeamCopyFunctionCfg::getIncomeTimes()
{
    return mCfg.incomeTimes;
}

int SyncTeamCopyFunctionCfg::getTeamMemberCount()
{
    return mCfg.teamMemberCount;
}

float SyncTeamCopyFunctionCfg::getIncomeMultiple(int intimacysum)
{
    int intiSize = mIntimacyBase.size();
    int multiSize = mAwardMultiple.size();
    
    int index = 0;
    
    for (int i = 0; i < intiSize; i++) {
        if (intimacysum < mIntimacyBase[i]) {
            index = i;
            break;
        }
    }
    
    if (index >= multiSize) {
        return 1.0f;
    }
    
    return mAwardMultiple[index];
}

set<string> ReservedNameMgr::sNames;
string ReservedNameMgr::sDefaultRobotFriend;

bool ReservedNameMgr::load(const char *fullpath)
{
    GameInifile ini(fullpath);
    
    vector<string> keys;
    string section = "system";
    ini.getKeys(section, inserter(keys, keys.begin()));
    for (int i = 0; i < keys.size(); i++) {
        string name = ini.getValue(section, keys[i]);
        add(name.c_str());
    }
    
    sDefaultRobotFriend = ini.getValue("friend", "robot");
    add(sDefaultRobotFriend.c_str());
    
    return true;
}

bool ReservedNameMgr::add(const char *name)
{
    sNames.insert(name);
    return true;
}

bool ReservedNameMgr::find(const char *name)
{
    return sNames.find(name) != sNames.end();
}

map<int, string> EquipmodeCorrespondedColor::datas;

void EquipmodeCorrespondedColor::load(string fullpath)
{

    GameInifile inifile(fullpath);
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        if (inifile.exists(*iter))
        {
            
            
            string content = inifile.getValue(*iter, "color");
            int key = atoi((*iter).c_str());
            datas[key] = content;
        }
    }

}
string EquipmodeCorrespondedColor::getColorStrByEquipmode(int mode)
{
    map<int, string>::iterator iter = datas.find(mode);
    if(iter == datas.end())
        return "";
    return iter->second;
}

//检验与服务器连接状态
int LinkStateCheckCfg::mIsCheckON;
float LinkStateCheckCfg::mCheckReqDelta;
float LinkStateCheckCfg::mRespondWaitDelta;
void LinkStateCheckCfg::load(string fullpath)
{
    GameInifile inifile(fullpath);
    mIsCheckON = inifile.getValueT("link_check", "link_check_ON", 1);
    mRespondWaitDelta = inifile.getValueT("link_check", "link_check_waittime", 0.0f);
    mCheckReqDelta = inifile.getValueT("link_check", "link_check_Reqtime", 0.0f);
}

/**********
 幻兽进化
 *********/

vector<pair<int,PetEvolutionNode::MaterialPair> > PetEvolutionNode::getAllEvolutionRelation()
{
    map<int, vector< MaterialPair > >::iterator iter = targets.begin();
    vector<pair<int,PetEvolutionNode::MaterialPair> > datas;
    for(;iter != targets.end(); iter ++)
    {
        vector< MaterialPair > data = iter->second;
        for(int i=0; i< data.size(); i++)
        {
            datas.push_back(make_pair(iter->first,data[i]));
        }
    }
    return datas;
}

vector<PetEvolutionNode::MaterialPair > PetEvolutionNode::getMaterialByTarget(int targetid)
{
    vector<PetEvolutionNode::MaterialPair > datas;
    map<int, vector< MaterialPair > >::iterator iter = targets.begin();
    if(iter == targets.end())
        return datas;
    return iter->second;
}


map<int, PetEvolutionMaterial*> PetEvolutionCfgMgr::mMaterial;
map<int, PetEvolutionNode*> PetEvolutionCfgMgr::mNodes;
vector<int> PetEvolutionCfgMgr::mCircuit;

void PetEvolutionCfgMgr::load(string nodeFile, string materialFile)
{
    GameInifile iniMaterial(materialFile);
    mMaterial.clear();
    
    std::list<std::string> sections;
    iniMaterial.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        if (iniMaterial.exists(*iter))
        {
            int materialId = Utils::safe_atoi((*iter).c_str());
            if (materialId <= 0) {
                continue;
            }
            
            PetEvolutionMaterial* def = new PetEvolutionMaterial;
            
            def->tmpid = iniMaterial.getValueT(*iter, "type_id", 0);
            def->lvl = iniMaterial.getValueT(*iter, "lvl", 0);
            def->starlvl = iniMaterial.getValueT(*iter, "starlvl", 0);
            def->prop = iniMaterial.getValueT(*iter, "prop", 0);
            def->stage = iniMaterial.getValueT(*iter, "stage", 0);
            def->restitution = iniMaterial.getValueT(*iter, "restitution", 0);
            def->quality = def->tmpid % 10;
            
            string skillStr = iniMaterial.getValue(*iter, "skill_lvl");
            
            StringTokenizer skills(skillStr, ";");
            for (int i = 0; i < skills.count(); i++) {
                def->skills.push_back(Utils::safe_atoi(skills[i].c_str()));
            }
            
            mMaterial.insert(make_pair(materialId, def));
        }
    }

    map<int, NodeTmp> nodesTmp;
    
    GameInifile iniNodes(nodeFile);
    
    sections.clear();
    iniNodes.getSections(std::back_inserter(sections));
    mNodes.clear();
    
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        if ( !iniMaterial.exists(*iter))
        {
//            continue;
        }
        
        NodeTmp tmp;
        tmp.tmpid = iniNodes.getValueT(*iter, "type_id", 0);
        tmp.getWayDesc = iniNodes.getValue(*iter, "source_desc");
        
        if (tmp.tmpid <= 0) {
            continue;
        }
        
        string needMaterial = iniNodes.getValue(*iter, "need_material");
        
        StringTokenizer materials(needMaterial, ";");
        
        for (int i = 0; i < materials.count(); i++) {
            string material = materials[i];
            int lastposition = 0;
            int index = material.find("+", lastposition);
            
            if (index >= 1) {
                string material1 = material.substr(0, index);
                string material2 = material.substr(index + 1, material.size());
                
                tmp.material.push_back(make_pair(Utils::safe_atoi(material1.c_str()), Utils::safe_atoi(material2.c_str())));
            }
        }
        
        string targetsStr = "";
        targetsStr = iniNodes.getValue(*iter, "target");
        StringTokenizer targets(targetsStr, ";");
        for (int i = 0; i < targets.count(); i++) {
            int targetid = Utils::safe_atoi(targets[i].c_str());
            
            PetCfgDef* def = PetCfg::getCfg(targetid);
            if (def == NULL) {
                log_info("can not find petcfg in pet.ini : pet_mod:"<<targetid);
                assert(0);
            }
            
            int targetEvolutionId = def->getMonsterTpltID() * 10 + def->getQua();
            
            if (tmp.tmpid == targetEvolutionId) {
                log_info("不能自己进化成自己，tmpid："<<targetid);
                assert(0);
            }
            
            tmp.targets.push_back(make_pair(targetid, targetEvolutionId));
        }
        
        nodesTmp.insert(make_pair(tmp.tmpid, tmp));
    }
    
    assert(checkEvolutionCircuit(nodesTmp) == false);
    
    makeNodes(nodesTmp);
    
}

bool PetEvolutionCfgMgr::checkEvolutionCircuit(map<int, PetEvolutionCfgMgr::NodeTmp> &nodesTmp)
{
    map<int, PetEvolutionCfgMgr::NodeTmp>::iterator iter = nodesTmp.begin();
    map<int, PetEvolutionCfgMgr::NodeTmp>::iterator endIter = nodesTmp.end();
    
    for (; iter != endIter; iter++) {
        if (iter->second.checked == false) {

            if(onCheckEvolutionCircuit(iter->second, nodesTmp))
            {
                return true;
            }
        }
    }
    mCircuit.clear();
    return false;
}

bool PetEvolutionCfgMgr::onCheckEvolutionCircuit(PetEvolutionCfgMgr::NodeTmp& node, map<int, PetEvolutionCfgMgr::NodeTmp> &nodesTmp)
{
    node.checked = true;
    
    bool isCircuit = false;
    
    for (int i = 0; i < mCircuit.size(); i++) {
        if (mCircuit[i] == node.tmpid) {
            isCircuit = true;
        }
    }
    
    if (isCircuit) {
        
        ostringstream ostr;
        ostr<<"circuit: ";
        for (int i = 0; i < mCircuit.size(); i++) {

            ostr<<mCircuit[i]<<" -> ";
        }
        ostr<<node.tmpid;
        mCircuit.clear();
        log_error(ostr.str().c_str());
        return true;
    }

    
    if (node.targets.size() == 0)
    {
        ostringstream ostr;
        ostr<<"circuit: ";
        for (int i = 0; i < mCircuit.size(); i++) {
            
            ostr<<mCircuit[i]<<" -> ";
        }
        ostr<<node.tmpid;
        mCircuit.clear();
        log_info(ostr.str().c_str());
        mCircuit.clear();
        return false;
    }
    
    mCircuit.push_back(node.tmpid);
    
    for (int i = 0; i < node.targets.size(); i++)
    {
        map<int, PetEvolutionCfgMgr::NodeTmp>::iterator iter = nodesTmp.find(node.targets[i].second);
        
        if (iter == nodesTmp.end())
        {
            log_error("PetEvolutionNode.ini can not find type_id:"<<node.targets[i].second);
            assert(0);
        }
        
        if (onCheckEvolutionCircuit(iter->second, nodesTmp)) {
            return true;
        }
    }
    
    return false;
}

void PetEvolutionCfgMgr::makeNodes(map<int, PetEvolutionCfgMgr::NodeTmp> &nodesTmp)
{
    map<int, PetEvolutionCfgMgr::NodeTmp>::iterator iter = nodesTmp.begin();
    map<int, PetEvolutionCfgMgr::NodeTmp>::iterator endIter = nodesTmp.end();
    
    for (; iter != endIter; iter++) {
        PetEvolutionNode* node = new PetEvolutionNode;
        node->tmpid = iter->second.tmpid;
        node->getWayDesc  = iter->second.getWayDesc;
        
        //设置当前节点的进化来源的材料
        for (int i = 0; i < iter->second.material.size(); i++) {
            PetEvolutionMaterial* material1= getMaterial(iter->second.material[i].first);
            PetEvolutionMaterial* material2= getMaterial(iter->second.material[i].second);
            
            node->material.push_back(make_pair(material1, material2));
        }
        
        for (int i = 0; i < iter->second.targets.size(); i++) {
            
            int targetmod = iter->second.targets[i].first;
//
//            PetCfgDef* def = PetCfg::getCfg(targetmod);
//            if (def == NULL) {
//                log_info("can not find petcfg in pet.ini : pet_mod:"<<targetmod);
//                assert(0);
//            }
            
            int targetEvolutionId = iter->second.targets[i].second;
            
            map<int, PetEvolutionCfgMgr::NodeTmp>::iterator targetIter = nodesTmp.find(targetEvolutionId);
            
            if (targetIter == endIter) {
                log_info("没有找到目标的信息，fileName: PetEvolutionNode.ini，tmpid："<<targetEvolutionId);
                assert(0);
            }
            
            //设置已当前节点为材料的所有目标的索引
            vector< PetEvolutionNode::MaterialPair > materialPairs;
            
            //在目标的所有进化材料中找到已当前node为材料的pair
            for (int j = 0; j < targetIter->second.material.size(); j++) {
                PetEvolutionMaterial* materialFirst = getMaterial(targetIter->second.material[j].first);
                PetEvolutionMaterial* materialSecond = getMaterial(targetIter->second.material[j].second);
                
                if (materialFirst == NULL || materialSecond == NULL) {
                    log_info("没有找到材料的信息，fileName: PetEvolutionMaterial.ini. materialId: "<<targetIter->second.material[j].first<<","<<targetIter->second.material[j].second);
                    assert(0);
                }
                
                if ( !checkOtherNodeExistTarget(materialFirst->tmpid, targetmod, nodesTmp) ) {
                    log_error("没有找到对应的target:"<<targetmod<<". Typeid1:"<<materialFirst->tmpid);
                    assert(0);
                }
                
                if ( !checkOtherNodeExistTarget(materialSecond->tmpid, targetmod, nodesTmp) ) {
                    log_error("没有找到对应的target:"<<targetmod<<". Typeid2:"<<materialSecond->tmpid);
                    assert(0);
                }
                
                if (materialFirst->tmpid == node->tmpid)
                {
                    materialPairs.push_back(make_pair(materialFirst, materialSecond));
                }
                else if (materialSecond->tmpid == node->tmpid)
                {
                    materialPairs.push_back(make_pair(materialSecond, materialFirst));
                }
            }
            node->targets.insert(make_pair(targetmod, materialPairs));
        }
        
        mNodes.insert(make_pair(node->tmpid, node));
    }
}

PetEvolutionNode* PetEvolutionCfgMgr::getNode(int tmpid)
{
    map<int, PetEvolutionNode*>::iterator iter = mNodes.find(tmpid);
    
    if (iter == mNodes.end()) {
        return NULL;
    }
    
    return iter->second;
}

PetEvolutionNode* PetEvolutionCfgMgr::getPetEvolutionNodeByPetTmpid(int petTmpid)
{
    PetCfgDef * def = PetCfg::getCfg(petTmpid);
    if(def == NULL)
        return  NULL;
    int tmpid = def->getMonsterTpltID() * 10 + def->getQua();
    return getNode(tmpid);
}

PetEvolutionMaterial* PetEvolutionCfgMgr::getMaterial(int materialId)
{
    map<int, PetEvolutionMaterial*>::iterator iter = mMaterial.find(materialId);
    if (iter == mMaterial.end()) {
        return NULL;
    }
    
    return iter->second;
}

PetEvolutionNode::MaterialPair PetEvolutionCfgMgr::getMaterials(int targetid, int firstTypeid, int secondTypeid)
{
    PetEvolutionNode* node = getNode(firstTypeid);
    PetEvolutionNode::MaterialPair pair;
    pair.first = NULL;
    pair.second = NULL;
    
    if (node) {
        vector< PetEvolutionNode::MaterialPair > materials;
        
        map<int, vector< PetEvolutionNode::MaterialPair > >::iterator iter = node->targets.find(targetid);
        if (iter == node->targets.end()) {
            return pair;
        }
        
        for (int i = 0; i < iter->second.size(); i++) {
            if (iter->second[i].second->tmpid == secondTypeid) {
                return iter->second[i];
            }
        }
    }
    
    return pair;
}

PetEvolutionRestitutionCfgDef PetEvolutionRestitutionCfg::mCfg;

void PetEvolutionRestitutionCfg::load(string fullPath)
{
    GameInifile inifile(fullPath);
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        if (inifile.exists(*iter))
        {
            if (strcmp((*iter).c_str(), "common") == 0) {
                mCfg.mMergeExpRestitutionPercent = inifile.getValueT(*iter, "exp_restitution", 0.0f);
                mCfg.mPagesRestitutionPercent = inifile.getValueT(*iter, "pages_restitution", 0.0f);
                mCfg.mGoldRestitutionPercent = inifile.getValueT(*iter, "gold_restitution", 0.0f);
                
                PetEvolutionRestitutionItem* common_book = new PetEvolutionRestitutionItem;
                
                string bookStr = inifile.getValue(*iter, "common_skill_book");
                
                if (bookStr.empty()) {
                    break;
                }
                
                StringTokenizer bookInfo(bookStr, ":");
                if (bookInfo.count() < 2) {
                    assert(0);
                }
                
                common_book->itemId = Utils::safe_atoi(bookInfo[0].c_str());
                common_book->refValue = Utils::safe_atoi(bookInfo[1].c_str());
                
                mCfg.mSkillBooks.insert(make_pair(0, common_book));
                
                continue;
            }
            
            int quality = Utils::safe_atoi((*iter).c_str());
            
            string eggStr = inifile.getValue(*iter, "pet_egg");
            StringTokenizer eggInfo(eggStr, ":");
            if (!eggStr.empty()) {
                PetEvolutionRestitutionItem* egg = new PetEvolutionRestitutionItem;
                
                egg->itemId = Utils::safe_atoi(eggInfo[0].c_str());
                egg->refValue = Utils::safe_atoi(eggInfo[1].c_str());
                
                assert(egg->refValue > 0);
                
                mCfg.mPetEgg.insert(make_pair(quality, egg));
            }
            

            
            string bookStr = inifile.getValue(*iter, "skill_book");
            
            StringTokenizer bookInfo(bookStr, ":");
            if (!bookStr.empty()) {
                
                PetEvolutionRestitutionItem* book = new PetEvolutionRestitutionItem;
                book->itemId = Utils::safe_atoi(bookInfo[0].c_str());
                book->refValue = Utils::safe_atoi(bookInfo[1].c_str());
                
                assert(book->refValue > 0);
                mCfg.mSkillBooks.insert(make_pair(quality, book));
            }
            

            
            string stagePetEggStr = inifile.getValue(*iter, "star_goblin");
            StringTokenizer stageEggs(stagePetEggStr, ":");
            if (!stagePetEggStr.empty()) {
                
                PetEvolutionRestitutionItem* stageEgg = new PetEvolutionRestitutionItem;
                stageEgg->itemId = Utils::safe_atoi(stageEggs[0].c_str());
                stageEgg->refValue = Utils::safe_atoi(stageEggs[1].c_str());
                
                assert(stageEgg->refValue > 0);
                mCfg.mStagePetEggs.insert(make_pair(quality, stageEgg));
            }
        }
    }
}

float PetEvolutionRestitutionCfg::getMergeExpPercent()
{
    return mCfg.mMergeExpRestitutionPercent;
}

float PetEvolutionRestitutionCfg::getPagesPercent()
{
    return mCfg.mPagesRestitutionPercent;
}

float PetEvolutionRestitutionCfg::getGoldPercent()
{
    return mCfg.mGoldRestitutionPercent;
}

PetEvolutionRestitutionItem* PetEvolutionRestitutionCfg::getPetEggCfg(int quality)
{
    map<int, PetEvolutionRestitutionItem*>::iterator iter = mCfg.mPetEgg.find(quality);
    if (iter != mCfg.mPetEgg.end()) {
        return iter->second;
    }
    
    return NULL;
}

PetEvolutionRestitutionItem* PetEvolutionRestitutionCfg::getSkillBookCfg(int quality)
{
    map<int, PetEvolutionRestitutionItem*>::iterator iter = mCfg.mSkillBooks.find(quality);
    if (iter != mCfg.mSkillBooks.end()) {
        return iter->second;
    }

    return NULL;
}

PetEvolutionRestitutionItem* PetEvolutionRestitutionCfg::getStagePetEggCfg(int quality)
{
    map<int, PetEvolutionRestitutionItem*>::iterator iter = mCfg.mStagePetEggs.find(quality);
    if (iter != mCfg.mStagePetEggs.end()) {
        return iter->second;
    }
    
    return NULL;
}

int WeaponEnchantCfgDef::getCost(int enchantLvl)
{
    if(enchantLvl <= 0 || enchantLvl > costVec.size()|| costVec.size() == 0)
    {
        return INT_MAX;
    }
    return costVec[enchantLvl-1];
}
int WeaponEnchantCfgDef::getResourcesId(int enchantLvl,ObjJob objJob)
{
    map<ObjJob, vector<int> >::iterator iter = resourceMap.find(objJob);
    if (iter == resourceMap.end())
    {
        return 0;
    }
    if(enchantLvl <=0 || enchantLvl > iter->second.size() || iter->second.size() == 0)
    {
        return 0;
    }
    return iter->second[enchantLvl-1];
}

int WeaponEnchantCfgDef::getSkillid(int enchantLvl)
{
    if (enchantLvl <=0 || enchantLvl > skillIds.size() || skillIds.size() == 0)
    {
        return 0;
    }
    return  skillIds[enchantLvl-1];
}
float WeaponEnchantCfgDef::getProbability(int enchantLvl)
{
    if(enchantLvl <=0 || enchantLvl > probabilitys.size() || probabilitys.size() == 0)
    {
        return 0.0f;
    }
    return probabilitys[enchantLvl-1];
}

EnchantPropertyAddDef* WeaponEnchantCfgDef::getEnchantPropertyAddDef(int enchantLvl)
{
    if(enchantLvl <=0 || enchantLvl > enchantPropertyAddDefVec.size() || enchantPropertyAddDefVec.size()== 0)
    {
        return NULL;
    }
    return enchantPropertyAddDefVec[enchantLvl-1];
}


map<int,WeaponEnchantCfgDef*> WeaponEnchantCfg::cfg;
int WeaponEnchantCfg::mTypeNum = 0;
int WeaponEnchantCfg::mEffectValue = 0;
float WeaponEnchantCfg::mLowestPro = 0.0f;
int WeaponEnchantCfg::mEnchantTimes = 0;

void WeaponEnchantCfg::load(std::string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
        {
            if (inifile.exists(*iter))
            {
                if (strcmp((*iter).c_str(), "common") == 0)
                {
                    WeaponEnchantCfg::setTypeNum(inifile.getValueT(*iter, "typenum", 0));
                    WeaponEnchantCfg::setEffectValue(inifile.getValueT(*iter, "effectValue", 0));
                    WeaponEnchantCfg::setLowestPro(inifile.getValueT(*iter, "lowestPro", 0.0f));
                    WeaponEnchantCfg::setEnchantTimes(inifile.getValueT(*iter, "enchantTimes", 0));
                    continue;
                }
                
                int typeId = inifile.getValueT(*iter, "typeId", 0);
                int maxLvl = inifile.getValueT(*iter, "lvl", 0);
                std::string costStr = inifile.getValue(*iter, "cost","");
                std::string warriorResIdStr = inifile.getValue(*iter, "warrior_resID","");
                std::string mageResIdStr = inifile.getValue(*iter, "mage_resID","");
                std::string assassinResIdStr = inifile.getValue(*iter, "assassin_resID","");
                std::string iconStr = inifile.getValue(*iter, "icon","");
                std::string skillIdStr = inifile.getValue(*iter, "skill_id","");
                std::string proStr = inifile.getValue(*iter, "probability","");
                std::string hpStr = inifile.getValue(*iter, "hp","");
                std::string atkStr = inifile.getValue(*iter, "atk","");
                std::string defStr = inifile.getValue(*iter, "def","");
                std::string dodgeStr = inifile.getValue(*iter, "dodge","");
                std::string desc = inifile.getValue(*iter, "desc","");
                
                WeaponEnchantCfgDef *weaponEnchantCfgDef = new WeaponEnchantCfgDef();
                weaponEnchantCfgDef->typeId = typeId;
                weaponEnchantCfgDef->maxLvl = maxLvl;
                weaponEnchantCfgDef->icon = iconStr;
                StringTokenizer costToken(costStr,";");
                assert(costToken.count() == maxLvl);
                for (int i = 0; i < costToken.count(); i++)
                {
                    weaponEnchantCfgDef->costVec.push_back(atoi(costToken[i].c_str()));
                }
                StringTokenizer warriorResIdToken(warriorResIdStr,";");
                assert(warriorResIdToken.count() == maxLvl);
                vector<int> warriorResIDVec;
                for(int i = 0; i < warriorResIdToken.count();i++)
                {
                    warriorResIDVec.push_back(atoi(warriorResIdToken[i].c_str()));
                }
                weaponEnchantCfgDef->resourceMap[ eWarrior ] =  warriorResIDVec;
                
                StringTokenizer mageResIdToken(mageResIdStr,";");
                assert(mageResIdToken.count() == maxLvl);
                vector<int> mageResIdVec;
                for(int i = 0; i < mageResIdToken.count();i++)
                {
                    mageResIdVec.push_back(atoi(mageResIdToken[i].c_str()));
                }
                weaponEnchantCfgDef->resourceMap[ eMage ] =  mageResIdVec;
                
                StringTokenizer assassinResIdToken(assassinResIdStr,";");
                assert(assassinResIdToken.count() == maxLvl);
                vector<int> assassinResIdVec;
                for(int i = 0; i < assassinResIdToken.count();i++)
                {
                    assassinResIdVec.push_back(atoi(assassinResIdToken[i].c_str()));
                }
                weaponEnchantCfgDef->resourceMap[ eAssassin ] =  assassinResIdVec;
                
                StringTokenizer skillIdToken(skillIdStr,";");
                assert(skillIdToken.count() == maxLvl);
                for (int i = 0; i < skillIdToken.count(); i++)
                {
                    weaponEnchantCfgDef->skillIds.push_back(atoi(skillIdToken[i].c_str()));
                }
                StringTokenizer proToken(proStr,";");
                assert(proToken.count() == maxLvl);
                for(int i = 0; i < proToken.count(); i++)
                {
                    weaponEnchantCfgDef->probabilitys.push_back(atof(proToken[i].c_str()));
                }
                StringTokenizer hpToken(hpStr,";");
                StringTokenizer atkToken(atkStr,";");
                StringTokenizer defToken(defStr,";");
                StringTokenizer dodgeToken(dodgeStr,";");
                assert(hpToken.count() == maxLvl);
                assert(atkToken.count() == maxLvl);
                assert(defToken.count() == maxLvl);
                assert(dodgeToken.count() == maxLvl);
                for (int i = 0; i < maxLvl; i++)
                {
                    EnchantPropertyAddDef *enchantPropertyAddDef = new EnchantPropertyAddDef();
                    enchantPropertyAddDef->setHp(atoi(hpToken[i].c_str()));
                    enchantPropertyAddDef->setAtk(atoi(atkToken[i].c_str()));
                    enchantPropertyAddDef->setDef(atoi(defToken[i].c_str()));
                    enchantPropertyAddDef->setDodge(atof(dodgeToken[i].c_str()));
                    weaponEnchantCfgDef->enchantPropertyAddDefVec.push_back(enchantPropertyAddDef);
                }
                weaponEnchantCfgDef->desc = desc;
                cfg[typeId] = weaponEnchantCfgDef;
            }
        }
        
    }
    catch(...)
    {
        log_error("weaponEnchant ERROR !!!");

    }

}

WeaponEnchantCfgDef* WeaponEnchantCfg::getWeaponEnchantCfgDef(int typeId)
{
    map<int, WeaponEnchantCfgDef*>::iterator iterFind = cfg.find(typeId);
    if(iterFind == cfg.end())
    {
        return NULL;
    }
    return iterFind->second;
}

int WeaponEnchantCfg::getCost(int typeId, int enchantLvl)
{
    map<int, WeaponEnchantCfgDef*>::iterator iterFind = cfg.find(typeId);
    if (iterFind == cfg.end())
    {
        return INT_MAX;
    }
    return  iterFind->second->getCost(enchantLvl);
}
int WeaponEnchantCfg::getResourcesId(int typeId, int enchantLvl,ObjJob objJob)
{
    map<int, WeaponEnchantCfgDef*>::iterator iterFind = cfg.find(typeId);
    if (iterFind == cfg.end())
    {
        return 0;
    }
    return iterFind->second->getResourcesId(enchantLvl,objJob);
}

int WeaponEnchantCfg::getSkillid(int typeId, int enchantLvl)
{
    map<int, WeaponEnchantCfgDef*>::iterator iterFind = cfg.find(typeId);
    if (iterFind == cfg.end())
    {
        return 0;
    }
    return iterFind->second->getSkillid(enchantLvl);
}
float WeaponEnchantCfg::getProbability(int typeId, int enchantLvl)
{
    map<int, WeaponEnchantCfgDef*>::iterator iterFind = cfg.find(typeId);
    if (iterFind == cfg.end())
    {
        return 0.0f;
    }
    return iterFind->second->getProbability(enchantLvl);
}
EnchantPropertyAddDef *WeaponEnchantCfg::getEnchantPropertyAddDef(int typeId, int enchantLvl)
{
    map<int, WeaponEnchantCfgDef*>::iterator iterFind = cfg.find(typeId);
    if (iterFind == cfg.end())
    {
        return NULL;
    }
    return iterFind->second->getEnchantPropertyAddDef(enchantLvl);

}
void WeaponEnchantCfg::getAllEnchantPropertyAddDef(int typeId, int enchantLvl,int &hp,int &atk,int &def, float &Dodge)
{
    map<int, WeaponEnchantCfgDef*>::iterator iterFind = cfg.find(typeId);
    if (iterFind == cfg.end())
    {
        return ;
    }
    for (int i = 1; i <= enchantLvl;i++ )
    {
        EnchantPropertyAddDef* enchantPropertyAddDef = iterFind->second->getEnchantPropertyAddDef(i);
        if (enchantPropertyAddDef)
        {
            hp = hp + enchantPropertyAddDef->getHp();
            atk = atk + enchantPropertyAddDef->getAtk();
            def = def + enchantPropertyAddDef->getDef();
            Dodge = Dodge + enchantPropertyAddDef->getDodge();
        }
        
    }
}
map<int,WeaponEnchantCfgDef *> &WeaponEnchantCfg::getEnchantCfgData()
{
    return cfg;
}


//--------------------------时装搜集-----------------------------
//int FashionCollectCfgDef::ReadInt(const char *propname, int def, bool *suc)
//{
//    string strPropName = propname;
//    map<string, string>::iterator it = props.find(strPropName);
//    if (it == props.end())
//    {
//        if (suc)
//        {
//            *suc = false;
//        }
//        return def;
//    }
//    else
//    {
//        if (suc)
//        {
//            *suc = true;
//        }
//    }
//    return Utils::safe_atoi(it->second.c_str());
//}
//
//float FashionCollectCfgDef::ReadFloat(const char *propname, float def, bool *suc)
//{
//    string strPropName = propname;
//    map<string, string>::iterator iter = props.find(strPropName);
//    if (iter == props.end())
//    {
//        if (suc)
//        {
//            *suc = false;
//        }
//        return def;
//    }
//    if (suc)
//    {
//        *suc = true;
//    }
//    return atof(iter->second.c_str());
//}
//
//string FashionCollectCfgDef::ReadStr(const char *propname, string def, bool *suc)
//{
//    map<string, string>::iterator iter = props.find(propname);
//    if (iter == props.end())
//    {
//        if (suc)
//        {
//            *suc = false;
//        }
//        return def;
//    }
//    if (suc)
//    {
//        *suc = true;
//    }
//    return iter->second;
//
//}


map<int, FashionCollectCfgDef*> FashionCollectCfg::cfg;
map<int, FashionCollectMaterialInfo> FashionCollectCfg::mMaterialInfo;
vector<FashionCollectCfgDef*> FashionCollectCfg::mFirstStageIds;

void FashionCollectCfg::load(std::string fullPath)
{
    try {
        GameInifile inifile(fullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            if (inifile.exists(*it))
            {
                FashionCollectCfgDef *def = new FashionCollectCfgDef();
                def->fashionType = (FashionCollectType)inifile.getValueT(*it, "type", 1);
                def->fashionObjectId = inifile.getValueT(*it, "id", 0);
                def->stage = inifile.getValueT(*it, "stage", 1);
                
                if (def->stage == 1) {
                    mFirstStageIds.push_back(def);
                }
                
                string awardFashion = inifile.getValue(*it, "award_fashion");
                string awardGift = inifile.getValue(*it, "award_giftPackage");
                string materialsStr = inifile.getValue(*it,"materials_id");
                string shareMaterialStr = inifile.getValue(*it, "shareMat_id");
                
                //奖励的时装
                StringTokenizer fashions(awardFashion, ";");
                
                if(fashions.count() < MAX_ROLE_JOB)
                {
                    log_error("fashioncollect award_fashion kinds error: "<<def->fashionObjectId);
                    assert(0);
                }
                
                for (int i = 0; i < fashions.count(); i++) {
                    
                    StringTokenizer fashion(fashions[i].c_str(), "*");
                    
                    if (fashion.count() < 1) {
                        log_error("fashioncollect award_fashion formate error: "<<def->fashionObjectId);
                        assert(0);
                    }
                    
                    int count = 1;
                    
                    int fashionid = Utils::safe_atoi(fashion[0].c_str());
                    
                    if (fashions.count() == 2) {
                        count = Utils::safe_atoi(fashion[1].c_str());
                    }
                    
                    
                    if (fashionid <= 0) {
                        log_error("fashioncollect award_fashion error: "<<def->fashionObjectId);
                        assert(0);
                    }
                    
                    ItemCell item(fashionid, count);
                    def->fashionProductId.push_back(item);
                }
                
                //奖励的礼包
                StringTokenizer gifts(awardGift, ";");
                for (int i = 0; i < gifts.count(); i++) {
                    
                    StringTokenizer gift(gifts[i].c_str(), "*");
                    
                    if (gift.count() < 1) {
                        log_error("fashioncollect award_gift formate error: "<<def->fashionObjectId);
                        assert(0);
                    }
                    
                    int giftid = Utils::safe_atoi(gift[0].c_str());
                    int count = 1;
                    
                    if (giftid <= 0) {
                        log_error("fashioncollect award_gift error: "<<def->fashionObjectId);
                        assert(0);
                    }
                    
                    if (gift.count() == 2) {
                        count = Utils::safe_atoi(gift[1].c_str());
                    }
                    
                    ItemCell item(giftid, count);
                    def->awardGift.push_back(item);
                }
                
                //材料id及数量
                StringTokenizer token( materialsStr,";");
                for (int i = 0; i< token.count(); i++)
                {
                    StringTokenizer item(token[i], "*");
                    
                    assert(item.count() >= 2);
                    int materialId = Utils::safe_atoi(item[0].c_str());
                    int count = Utils::safe_atoi(item[1].c_str());
                    
                    if (materialId <= 0) {
                        log_error("fashioncollect material error: "<<def->fashionObjectId);
                        assert(0);
                    }
                    
                    TargetOfFashionCollectMaterial* materialInfo = new TargetOfFashionCollectMaterial;
                    materialInfo->targetid = def->fashionObjectId;
                    materialInfo->count = count;
                    materialInfo->grid = def->materialIds.size();
                    
                    addMaterialInfo(materialId, materialInfo);
                    
                    ItemCell material(materialId, count);
                    def->materialIds.push_back(material);
                    
                    ItemCfgDef* cfg = ItemCfg::getCfg(materialId);
                    
                    if(cfg == NULL)
                    {
                        log_error("fashioncollect: "<<def->fashionObjectId<<"material: "<<materialId<<" cfg not find error: ");
                        assert(0);
                    }
                    
                    //计算材料部分的属性
                    def->mMaterialPropAdded.setAtk( def->mMaterialPropAdded.getAtk() + cfg->ReadInt("atk") * count);
                    def->mMaterialPropAdded.setMaxHp( def->mMaterialPropAdded.getMaxHp() + cfg->ReadInt("hp") * count);
                    def->mMaterialPropAdded.setDef(def->mMaterialPropAdded.getDef() + cfg->ReadInt("def") * count);
                    def->mMaterialPropAdded.setDodge(def->mMaterialPropAdded.getDodge() + cfg->ReadFloat("dodge") * count);
                    def->mMaterialPropAdded.setHit(def->mMaterialPropAdded.getHit() + cfg->ReadFloat("hit") * count);
                    def->mMaterialPropAdded.setCri(def->mMaterialPropAdded.getCri() + cfg->ReadFloat("cri") * count);
                }
                
                if (def->materialIds.size() > FASHIONCOLLECT_MAX_GRID_FOR_MATERIAL) {
                    log_error("fashioncollection kinds of material are more than count of grid");
                    assert(0);
                }
                
                //共享材料id及数量
                StringTokenizer shareMaterials( shareMaterialStr,";");
                for (int i = 0; i< shareMaterials.count(); i++)
                {
                    StringTokenizer item(shareMaterials[i], "*");
                    
                    assert(item.count() >= 2);
                    int shareMaterialId = Utils::safe_atoi(item[0].c_str());
                    int count = Utils::safe_atoi(item[1].c_str());
                    
                    if (shareMaterialId <= 0) {
                        log_error("fashioncollect sharematerial error: "<<def->fashionObjectId);
                        assert(0);
                    }
                    
                    ItemCell shareMaterial(shareMaterialId, count);
                    def->shareMaterialIds.push_back(shareMaterial);
                }
                
                //读属性加成
                def->mPropAward.setAtk(inifile.getValueT(*it, "atk", 0));
                def->mPropAward.setDef(inifile.getValueT(*it, "def", 0));
                def->mPropAward.setHp(inifile.getValueT(*it, "hp", 0));
                def->mPropAward.setMaxHp(inifile.getValueT(*it, "hp", 0));
                def->mPropAward.setHit(inifile.getValueT(*it, "hit", 0.0f));
                def->mPropAward.setDodge(inifile.getValueT(*it, "dodge", 0.0f));
                def->mPropAward.setCri(inifile.getValueT(*it, "cri", 0.0f));
                
                cfg.insert(make_pair(def->fashionObjectId, def));
            }
        }
    } catch (...) {
        
    }
}
FashionCollectCfgDef * FashionCollectCfg::getCfg(int _id)
{
    map<int, FashionCollectCfgDef*>::iterator iter = cfg.find(_id);
    if (cfg.end() != iter)
    {
        return (iter->second);
    }
    return NULL;
}

//int FashionCollectCfg::readInt(int fashionObjectId, const char *propname, int def)
//{
//    FashionCollectCfgDef* cfgDef = getCfg(fashionObjectId);
//    if (cfgDef)
//    {
//        return cfgDef->ReadInt(propname,def);
//    }
//    return def;
//    
//}

vector<int> FashionCollectCfg::getFashionCollectItemByType(int type)
{
    vector<int> vec;
    map<int,FashionCollectCfgDef*>::iterator it = cfg.begin();
    for (; it != cfg.end(); it++)
    {
        FashionCollectCfgDef *def = it->second;
        
        if (def == NULL) {
            return vec;
        }
        
        if (def->fashionType == type)
        {
            vec.push_back(it->first);
        }
    }
    return vec;
}

void FashionCollectCfg::addMaterialInfo(int materialId, TargetOfFashionCollectMaterial *material)
{
    map<int, FashionCollectMaterialInfo>::iterator iter = mMaterialInfo.find(materialId);
    
    if (iter == mMaterialInfo.end()) {
        FashionCollectMaterialInfo info;
        info.push_back(material);
        mMaterialInfo.insert(make_pair(materialId, info));
    }
    else
    {
        iter->second.push_back(material);
    }
}

FashionCollectMaterialInfo FashionCollectCfg::getMaterialTargets(int materialid)
{
    map<int, FashionCollectMaterialInfo>::iterator iter = mMaterialInfo.find(materialid);
    if (iter == mMaterialInfo.end()) {
        FashionCollectMaterialInfo info;
        return info;
    }
    else
    {
        return iter->second;
    }
}


TargetOfFashionCollectMaterial* FashionCollectCfg::getMaterialInfowithTarget(int materialid, int targetid)
{
    map<int, FashionCollectMaterialInfo>::iterator iter = mMaterialInfo.find(materialid);
    
    if (iter == mMaterialInfo.end()) {
        return NULL;
    }
    
    FashionCollectMaterialInfo& info = iter->second;
    for (int i = 0; i < info.size(); i++) {
        
        TargetOfFashionCollectMaterial* targetInfo = info[i];
        
        if (targetInfo == NULL) {
            continue;
        }
        
        if (targetid == targetInfo->targetid) {
            return targetInfo;
        }
    }
    
    return NULL;
}

vector<FashionCollectCfgDef*>& FashionCollectCfg::getFirstStageIds()
{
    return mFirstStageIds;
}

map<int, FashionMaterialRecycleCfgDef*> FashionMaterialRecycleCfg::mCfg;
void FashionMaterialRecycleCfg::load(std::string fullPath)
{
    try {
        GameInifile inifile(fullPath);
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            FashionMaterialRecycleCfgDef* def = new FashionMaterialRecycleCfgDef;
            def->lvl = inifile.getValueT(*it, "Lvl", 0);
            def->exp = inifile.getValueT(*it, "exp", 0);
            def->parameter = inifile.getValueT(*it, "improve_Attri", 0.0f);
            
            mCfg.insert(make_pair(def->lvl, def));
        }
    }catch(...)
    {
        
    }
}

FashionMaterialRecycleCfgDef* FashionMaterialRecycleCfg::getCfg(int lvl)
{
    map<int, FashionMaterialRecycleCfgDef*>::iterator iter = mCfg.find(lvl);
    if (iter == mCfg.end()) {
        return NULL;
    }
    
    return iter->second;
}

/****************************************************/


void SynthesisCfgDef::checkSameMaterial()
{
    if(materials.size() <= 1)
        return;
    for(int i=0; i< materials.size() -1; i++)
    {
        for(int j = i+1 ;j< materials.size(); j++)
        {
            if(materials[i].itemid == materials[j].itemid)
            {
                string  str = Utils::makeStr("在第%d列 材料里面有相同的id",synthesisID);
                printf("%s",str.c_str());
                assert(0);
            }
        }
    }

}
void SynthesisCfgDef::checkSameSynthetic()
{
    if(synthetics.size() <= 1)
        return;
    for(int i=0; i< synthetics.size() -1; i++)
    {
        for(int j = i+1 ;j< synthetics.size(); j++)
        {
            if(synthetics[i].itemid == synthetics[j].itemid)
            {
                string  str = Utils::makeStr("在第%d列 材料里面有相同的id",synthesisID);
                printf("%s",str.c_str());
                assert(1);
            }
        }
    }
}

vector<SynthesisCfgDef * > SynthesisCfg::synthesisCfg;
map<int, vector<SynthesisCfgDef *> > SynthesisCfg::cfg;

void SynthesisCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
    {
        SynthesisCfgDef *def = new SynthesisCfgDef();
        
        def->synthesisID = atoi((*it).c_str());
        def->needLvl = inifile.getValueT(*it, "synthesis_lvl", 0);
        def->needGold = inifile.getValueT(*it, "synthesis_gold", 0);
        string materialStr = inifile.getValue(*it, "materials", "");
        StringTokenizer materialToken(materialStr,";");
        for(int i=0; i< materialToken.count(); i++)
        {
            ItemCell cell;
            if(sscanf(materialToken[i].c_str(), "%d*%d",&cell.itemid,&cell.count) == 2)
            {
                def->materials.push_back(cell);
            }
        }
        
        string syntheticsStr = inifile.getValue(*it, "synthetics", "");
        StringTokenizer syntheticsToken(syntheticsStr,";");
        
        for(int i=0; i< syntheticsToken.count(); i++)
        {
            ItemCell cell;
            if(sscanf(syntheticsToken[i].c_str(), "%d*%d",&cell.itemid,&cell.count) == 2)
            {
                def->synthetics.push_back(cell);
            }
        }
        def->checkSameMaterial();
        def->checkSameSynthetic();
        checkSameMaterialAndSynthetic(synthesisCfg,def);
        synthesisCfg.push_back(def);
    }
    transformDatas(synthesisCfg);
}
bool  SynthesisCfg::SynthesisCfgIsSample(SynthesisCfgDef *def1 ,SynthesisCfgDef * def2)
{
    if(def1->materials.size() != def2->materials.size())
        return false;
    for(int i=0; i< def1->materials.size(); i++)
    {
        bool isFind = false;
        for(int j=0; j < def2->materials.size(); j++)
        {
            if(def1->materials[i].itemid == def2->materials[j].itemid && def1->materials[i].count == def2->materials[i].count)
            {
                isFind = true;
                break;
            }
        }
        if(isFind == false)
            return false;
    }
    return true;
}
void  SynthesisCfg::checkSameMaterialAndSynthetic(vector<SynthesisCfgDef *> datas, SynthesisCfgDef * def)
{
    
    //去掉检测
//    for(int i=0; i< datas.size(); i++)
//    {
//        
//        bool isSample = SynthesisCfgIsSample(datas[i],def);
//        if(isSample == true)
//        {
//            string str = Utils::makeStr("第%d列 和 第%d列 存在一样的材料", datas[i]->synthesisID, def->synthesisID);
//            printf("%s",str.c_str());
//            assert(0);
//        }
//            
//    }
}

void SynthesisCfg::checkAndAddMaterialID(vector<int> & materialids, int itemid)
{
    for(int i=0; i< materialids.size();i++)
    {
        if(materialids[i] == itemid)
            return;
    }
    materialids.push_back(itemid);
}

vector<ItemCell > SynthesisCfg::sortSynthesis(SynthesisCfgDef * data, int materialid)
{
    vector<ItemCell > materials;
    for (int i=0; i < data->materials.size(); i++)
    {
        if(data->materials[i].itemid == materialid)
        {
            materials.push_back(data->materials[i]);
            
            for(int j=0; j< data->materials.size(); j++)
            {
                if(data->materials[j].itemid != data->materials[i].itemid)
                {
                    materials.push_back(data->materials[j]);
                }
            }
            
            break;
            
        }
    }
    return materials;
}

bool SynthesisCfg::isFindMaterialid(SynthesisCfgDef * data, int materialid)
{
    for (int i=0; i < data->materials.size(); i++)
    {
        if(data->materials[i].itemid == materialid)
        {
            return true;
        }
    }
    return false;

}
void SynthesisCfg::transformDatas(vector<SynthesisCfgDef *> & datas)
{
    vector<int> materialids;  //找到所有的材料id
    for(int i=0; i< datas.size(); i++)
    {
        vector<ItemCell> material = datas[i]->materials;
        for(int j=0; j < material.size(); j++)
        {
            checkAndAddMaterialID(materialids,material[j].itemid);
        }
    }
    
    //以每种材料作为键值  存值
    for(int i=0; i< materialids.size(); i++)
    {
        vector<SynthesisCfgDef *> allRelations;
        for(int j=0; j < datas.size(); j++)
        {
            if(isFindMaterialid(datas[j],materialids[i]) > 0)
            {
                allRelations.push_back(datas[j]);
            }
        }
        cfg.insert(make_pair(materialids[i], allRelations));
    }
}

vector<SynthesisCfgDef *> SynthesisCfg::getCfgByMaterial(int materialid)
{
    vector<SynthesisCfgDef *> data;
    map<int, vector<SynthesisCfgDef *> >::iterator iter = cfg.find(materialid);
    if(iter != cfg.end())
    {
        data = iter->second;
    }
    return data;
}

vector<SynthesisCfgDef> SynthesisCfg::getSortCfgByMaterial(int materialid)
{
    vector<SynthesisCfgDef > data;
    map<int, vector<SynthesisCfgDef *> >::iterator iter = cfg.find(materialid);
    if(iter != cfg.end())
    {
        for(int i=0; i< iter->second.size(); i++)
        {
            vector<ItemCell > cell = sortSynthesis(iter->second[i],materialid);
            if(cell.size() >0)
            {
                SynthesisCfgDef def;
                def.needLvl = iter->second[i]->needLvl;
                def.synthesisID = iter->second[i]->synthesisID;
                def.needGold = iter->second[i]->needGold;
                def.materials = cell;
                def.synthetics = iter->second[i]->synthetics;
                data.push_back(def);
            }
        }
    }
    return data;
}

SynthesisCfgDef * SynthesisCfg::getSynthesisCfg(int synthesisid)
{
    for(int i=0; i< synthesisCfg.size(); i++)
    {
        if(synthesisCfg[i]->synthesisID == synthesisid)
        {
            return synthesisCfg[i];
        }
    }
    return NULL;
}
vector<int>SynthesisCfg::getAllMaterialIDs()
{
    vector<int> allMaterialIDs;
    for(int i=0; i< synthesisCfg.size(); i++)
    {
        for(int j=0; j < synthesisCfg[i]->materials.size(); j++)
        {
            checkAndAddMaterialID(allMaterialIDs,synthesisCfg[i]->materials[j].itemid);
        }
    }
    return allMaterialIDs;
}

vector<int> SynthesisCfg::getAllSynthesisByMaterialID(int materialid)
{
    vector<int> allSynthesis;
    vector<SynthesisCfgDef *> data = SynthesisCfg::getCfgByMaterial(materialid);
    for(int i=0; i< data.size(); i++)
    {
        for(int j=0; j < data[i]->synthetics.size(); j++)
        {
            checkAndAddMaterialID(allSynthesis,data[i]->synthetics[j].itemid);
        }
    }
    return allSynthesis;
}

//--------------------------------------------------
#pragma mark - 幻兽大冒险
std::map<int, PetAdventureCfgDef*> PetAdventureCfg::cfg;

void PetAdventureCfg::load(std::string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);
        cfg.clear();
        
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            PetAdventureCfgDef *def = new PetAdventureCfgDef();
            
            def->setSceneid(inifile.getValueT(*it, "sceneid", 0));
            
            string adventure_type = inifile.getValue(*it, "pet_adventure_type");

            def->pet_adventure_type = getPetAdventureType(adventure_type);

            int starTime = Utils::parseDate( inifile.getValue(*it, "begin_time").c_str());
            def->begin_time = starTime;
            int endTime = Utils::parseDate(inifile.getValue(*it, "end_time").c_str());
            def->end_time = endTime;
            
            if (def->pet_adventure_type == ePetAdventure_temporary_activity) {
                assert(starTime > 0 && endTime > 0);
            }
                
            string recommended_pet_list = inifile.getValue(*it, "recommended_pet");
            StringTokenizer token(recommended_pet_list,";");
            for(int i=0;i< token.count();i++)
            {
                def->recommended_pet.push_back(Utils::safe_atoi(token[i].c_str()));
            }
            def->setHp(inifile.getValueT(*it, "hp", 0));
            def->setHpTimes(inifile.getValueT(*it, "hp_times", 1.0));
            def->setAttack(inifile.getValueT(*it, "attack", 0));
            def->setAttackTimes(inifile.getValueT(*it, "attack_times", 1.0));
            def->setDefence(inifile.getValueT(*it, "defence", 0));
            def->setDefenceTimes(inifile.getValueT(*it, "defence_times", 1.0));
            def->setHit(inifile.getValueT(*it, "hit", 0));
            def->setHitTimes(inifile.getValueT(*it, "hit_times", 1.0));
            def->setDodge(inifile.getValueT(*it, "dodge", 0));
            def->setDodgeTimes(inifile.getValueT(*it, "dodge_times", 1.0));
            def->setSkill(inifile.getValueT(*it, "skill", 0));
            def->setSkillTimes(inifile.getValueT(*it, "skill_times", 1.0));
                
            def->extra_gold = inifile.getValueT(*it, "extra_gold", 0);
            def->extra_exp = inifile.getValueT(*it, "extra_exp", 0);
            def->extra_battlepoint = inifile.getValueT(*it, "extra_battlepoint", 0);
            def->extra_consval = inifile.getValueT(*it, "extra_consval", 0);
            def->extra_petexp = inifile.getValueT(*it, "extra_petexp", 0);

            def->reward_rate = inifile.getValueT(*it, "reward_rate", 1.0);
            def->extra_fall_items = inifile.getValue(*it, "extra_fall_items", "");

            cfg.insert(make_pair(def->getSceneid(), def));
        }
    }
    catch(...)
    {
        
    }
}

PetAdventureCfgDef* PetAdventureCfg::getPetAdventureCfgDef(int index) {
    
    std::map<int, PetAdventureCfgDef*>::iterator iter = cfg.find(index);
    
    if (iter == cfg.end()) {
        return NULL;
    }
    return iter->second;
}

void PetAdventureCfg::getPetAdventureNormalCfgDef()
{
    
}
vector<int> PetAdventureCfg::getCurPetAdventureData(int sceneId)
{
    vector<int> vec;
    map<int, PetAdventureCfgDef*> ::iterator iter = cfg.find(sceneId);
    if(iter != cfg.end())
    {
        for(int i = 0; i< iter->second->recommended_pet.size(); i++)
        {
            vec.push_back(iter->second->recommended_pet[i]);
        }
    }
    return vec;
}

vector<PetAdventureCfgDef*> PetAdventureCfg::getNormalCopy()
{
    
    vector<PetAdventureCfgDef*> datas;
    
    map<int, PetAdventureCfgDef*>::iterator iter = cfg.begin();
    for(; iter != cfg.end(); iter++)
    {
        int sceneid = iter->second->getSceneid();
        
        SceneCfgDef * def = SceneCfg::getCfg(sceneid);
        
        if(def->petAdventureType == ePetAdventure_normal)
        {
            datas.push_back(iter->second);
        }
    }
    return datas;
}

vector<PetAdventureCfgDef *> PetAdventureCfg::getActivityCopy()
{
    vector<PetAdventureCfgDef*> datas;
    
    map<int, PetAdventureCfgDef*>::iterator iter = cfg.begin();
    for(; iter != cfg.end(); iter++)
    {
        int sceneid = iter->second->getSceneid();
        
        SceneCfgDef * def = SceneCfg::getCfg(sceneid);
        
        if(def->petAdventureType == ePetAdventure_permanent_activity || def->petAdventureType == ePetAdventure_temporary_activity)
        {
            datas.push_back(iter->second);
        }
    }
    return datas;
}


void PetAdventureCfg::getPetAdventureCopy()
{
    vector<PetAdventureCfgDef*> datas;
    map<int, PetAdventureCfgDef*>::iterator iter = cfg.begin();
    for(; iter != cfg.end(); iter++)
    {
        int sceneId =  iter->second->getSceneid();
        SceneCfgDef* def = SceneCfg::getCfg(sceneId);
        
        if(def != NULL)
        {
            datas.push_back(iter->second);
        }
    }
}

//---------------------------------------------------------
//幻兽大冒险机器幻兽配置
#pragma mark - 幻兽大冒险机器幻兽配置
std::map<int, PetAdventureRobotPetsForRentCfgDef*> PetAdventureRobotPetsForRentCfg::cfg;

void PetAdventureRobotPetsForRentCfg::load(std::string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);
        cfg.clear();
        
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            PetAdventureRobotPetsForRentCfgDef *def = new PetAdventureRobotPetsForRentCfgDef();
            
            def->sceneId       = inifile.getValueT(*it, "sceneId", 0);
            def->growth        = inifile.getValueT(*it, "growth", 0);
            string szSkillsLvl = inifile.getValue(*it, "skills", "");
            StringTokenizer token(szSkillsLvl,";");
            for(int i=0; i< token.count(); i++)
            {
                def->skillsLvl.push_back(Utils::safe_atoi(token[i].c_str()));
            }
            
            def->batprop.setLvl  (inifile.getValueT(*it, "lvl", 0));
            def->batprop.setMaxHp(inifile.getValueT(*it, "bhp", 0));
            def->batprop.setAtk  (inifile.getValueT(*it, "batk", 0));
            def->batprop.setDef  (inifile.getValueT(*it, "bdef", 0));
            def->batprop.setHit  (inifile.getValueT(*it, "bhit", 0));
            def->batprop.setDodge(inifile.getValueT(*it, "bdodge", 0));
            
            cfg.insert(make_pair(def->sceneId, def));
        }
    }
    catch(...)
    {
        
    }
    
}

PetAdventureRobotPetsForRentCfgDef* PetAdventureRobotPetsForRentCfg::getPetAdventurePetRobotCfgDef(int index)
{
    std::map<int, PetAdventureRobotPetsForRentCfgDef*>::iterator iter = cfg.find(index);
    
    if (iter == cfg.end()) {
        return NULL;
    }
    return iter->second;
}

vector<int> PetAdventureRobotPetsForRentCfg::getPetAdventureRobotPetSceneIdList()
{
    vector<int> robotPetSceneIdListTemp;
    std::map<int, PetAdventureRobotPetsForRentCfgDef*>::iterator iter = cfg.begin();
    for (; iter != cfg.end(); iter++) {
        robotPetSceneIdListTemp.push_back(iter->first);
    }
    
    return robotPetSceneIdListTemp;
    
}

//-----------------------------------------
//膜拜
std::map<int, CrossServiceWarWorshipCfgDef*> CrossServiceWarWorshipCfg::worshipcfg;
int CrossServiceWarWorshipCfg::openLvl = 0;
float CrossServiceWarWorshipCfg::sCommonWorshipPlus = 0.0f;
float CrossServiceWarWorshipCfg::sSpecialWorshipPlus = 0.0f;
CsPvpPeriod CrossServiceWarWorshipCfg::sWorshipPeriod;
CsPvpPeriod CrossServiceWarWorshipCfg::sBattlePeriod;
vector<string> CrossServiceWarWorshipCfg::sAwards;


int CrossServiceWarWorshipCfg::getOpenLvl()
{
    return openLvl;
}

bool CrossServiceWarWorshipCfg::canWorship(int wday, int hour, int min) {
    int cmpVal = genWeekTimeCmp(wday, hour, min);
    return cmpVal >= sWorshipPeriod.mWeekBegin && cmpVal < sWorshipPeriod.mWeekEnd;
}


bool CrossServiceWarWorshipCfg::isWorshipEnd(int wday, int hour, int min) {
    int cmpVal = genWeekTimeCmp(wday, hour, min);
    return cmpVal > sWorshipPeriod.mWeekEnd;
}


bool CrossServiceWarWorshipCfg::canBattle(int wday, int hour, int min) {
    int cmpVal = genWeekTimeCmp(wday, hour, min);
    return cmpVal >= sBattlePeriod.mWeekBegin && cmpVal < sBattlePeriod.mWeekEnd;
}


bool CrossServiceWarWorshipCfg::isBattleEnd(int wday, int hour, int min) {
    int cmpVal = genWeekTimeCmp(wday, hour, min);
    return cmpVal > sBattlePeriod.mWeekEnd;
}


int CrossServiceWarWorshipCfg::genWeekTimeCmp(int wday, int hour, int min) {
    if (wday == 0) {
        wday = 7;
    }
    return wday * 10000 + hour * 100 + min;
}

void CrossServiceWarWorshipCfg::load(std::string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);
        worshipcfg.clear();
        
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            sCommonWorshipPlus = inifile.getValueT(*it, "commonbattlepointPlus", 0.0f);
            sSpecialWorshipPlus = inifile.getValueT(*it, "specialbattlepointPlus", 0.0f);
            vector<string> worshipPeriod = inifile.getTable(*it, "worshiptimes");
            vector<string> battlePeriod = inifile.getTable(*it, "cspvptimes");
            assert(worshipPeriod.size() == 2);
            assert(battlePeriod.size() == 2);

            int wday, hour, min;
            sscanf(worshipPeriod[0].c_str(), "%d-%d:%d", &wday, &hour, &min);
            sWorshipPeriod.mWeekBegin = genWeekTimeCmp(wday, hour, min);
            sscanf(worshipPeriod[1].c_str(), "%d-%d:%d", &wday, &hour, &min);
            sWorshipPeriod.mWeekEnd = genWeekTimeCmp(wday, hour, min);
            sscanf(battlePeriod[0].c_str(), "%d-%d:%d", &wday, &hour, &min);
            sBattlePeriod.mWeekBegin = genWeekTimeCmp(wday, hour, min);
            sscanf(battlePeriod[1].c_str(), "%d-%d:%d", &wday, &hour, &min);
            sBattlePeriod.mWeekEnd = genWeekTimeCmp(wday, hour, min);

            sAwards = inifile.getTable(*it, "awards");


            CrossServiceWarWorshipCfgDef *def = new CrossServiceWarWorshipCfgDef();
            def->setCrossServiceWarId(inifile.getValueT(*it, "id", 0));
            def->awards = inifile.getValue(*it, "awards", "");
            def->specialbattlepointplus = sSpecialWorshipPlus;
            def->commonbattlepointplus = sCommonWorshipPlus;
            def->cspvptimes = inifile.getValue(*it, "cspvptimes", "");
            def->worshiptimes = inifile.getValue(*it, "worshiptimes", "");
            openLvl = inifile.getValueT(*it, "openlvl", 0);
            worshipcfg.insert(make_pair(def->getCrossServiceWarId(), def));


        }
    }
    catch(...)
    {
        
    }
}


//服务器排名
std::map<string, CrossServiceWarRankingserviceCfgDef*> CrossServiceWarRankingserviceCfg::rankingservicecfg;
vector<pair<int, int> > CrossServiceWarRankingserviceCfg::sRankIntervals;
vector<string> CrossServiceWarRankingserviceCfg::sAwards;


void CrossServiceWarRankingserviceCfg::load(std::string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);
        rankingservicecfg.clear();
        
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        
        int expectRank = 1;
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            string interval = inifile.getValue(*it, "index");
            string award = inifile.getValue(*it, "awards", "");
            vector<string> rankStrArr = StrSpilt(interval, "-");
            int firstRank = Utils::safe_atoi(rankStrArr[0].c_str());
            if (expectRank != firstRank) {
                assert(0);
            }
            
            int secondRank;
            if (rankStrArr.size() == 1) {
                secondRank = firstRank;
            } else if (rankStrArr.size() == 2) {
                secondRank = Utils::safe_atoi(rankStrArr[1].c_str());
                if (secondRank <= firstRank) {
                    assert(0);
                }
            } else {
                assert(0);
            }
            sRankIntervals.push_back(make_pair<int, int>(firstRank, secondRank));
            sAwards.push_back(award);
            expectRank = secondRank + 1;
            
            
            
            CrossServiceWarRankingserviceCfgDef *def = new CrossServiceWarRankingserviceCfgDef();
            def->index = interval;
            def->awards = award;
            rankingservicecfg.insert(make_pair(def->index, def));
        }
    }
    catch(...)
    {
        
    }

}


string CrossServiceWarRankingserviceCfg::getAwardByRank(int rank)
{
    for (int i = 0; i < sRankIntervals.size(); i++) {
        if (rank >= sRankIntervals[i].first && rank <= sRankIntervals[i].second) {
            return sAwards[i];
        }
    }
    return "";
}

int CrossServiceWarRankingserviceCfg::getAwardsCount()
{
    int page = 0;
    map<string,  CrossServiceWarRankingserviceCfgDef*>::iterator iter = rankingservicecfg.begin();
    for(; iter != rankingservicecfg.end(); iter++)
    {
        int num = 0;
        CrossServiceWarRankingserviceCfgDef*  def = iter->second;
        
        string item_ward = def->awards;
        StringTokenizer tokenExtra(item_ward, ";");
        
        if (tokenExtra.count() % 6 == 0) {
            page = page + num/6;
        }
        else
        {
            page = page + num/6 + 1;
        }
    }
    return page;
}




//个人排名
std::map<string, CrossServiceWarRankingsimpleCfgDef*>  CrossServiceWarRankingsimpleCfg::rankingsimplecfg;
vector<pair<int, int> > CrossServiceWarRankingsimpleCfg::sRankIntervals;
vector<string> CrossServiceWarRankingsimpleCfg::sAwards;


string CrossServiceWarRankingsimpleCfg::getAwardByRank(int rank)
{
    for (int i = 0; i < sRankIntervals.size(); i++) {
        if (rank >= sRankIntervals[i].first && rank <= sRankIntervals[i].second) {
            return sAwards[i];
        }
    }
    return "";
}

void CrossServiceWarRankingsimpleCfg::load(std::string fullPath)
{
    try
    {
        GameInifile inifile(fullPath);
        rankingsimplecfg.clear();
        
        std::list<string> sections;
        inifile.getSections(std::back_inserter(sections));
        int expectRank = 1;
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
        {
            
            string interval = inifile.getValue(*it, "index");
            string award = inifile.getValue(*it, "awards", "");
            vector<string> rankStrArr = StrSpilt(interval, "-");
            int firstRank = Utils::safe_atoi(rankStrArr[0].c_str());
            if (expectRank != firstRank) {
                assert(0);
            }
            
            int secondRank;
            if (rankStrArr.size() == 1) {
                secondRank = firstRank;
            } else if (rankStrArr.size() == 2) {
                secondRank = Utils::safe_atoi(rankStrArr[1].c_str());
                if (secondRank <= firstRank) {
                    assert(0);
                }
            } else {
                assert(0);
            }
            sRankIntervals.push_back(make_pair<int, int>(firstRank, secondRank));
            sAwards.push_back(award);
            expectRank = secondRank + 1;
            
            
            
            CrossServiceWarRankingsimpleCfgDef *def = new CrossServiceWarRankingsimpleCfgDef();
            def->index = interval;
            def->awards = award;
            
            rankingsimplecfg.insert(make_pair(def->index, def));
        }
    }
    catch(...)
    {
        
    }

}

int CrossServiceWarRankingsimpleCfg::getRankingSimpleAwardCount()
{
    int page = 0;
    map<string,  CrossServiceWarRankingsimpleCfgDef*>::iterator iter = rankingsimplecfg.begin();
    for(; iter != rankingsimplecfg.end(); iter++)
    {
        int num = 0;
        CrossServiceWarRankingsimpleCfgDef*  def = iter->second;
        
        string item_ward = def->awards;
        StringTokenizer tokenExtra(item_ward, ";");
        
        if (tokenExtra.count() % 6 == 0) {
            page = page + num/6;
        }
        else
        {
            page = page + num/6 + 1;
        }
    }
    return page;

}


//跨服战
vector<int> CrossServiceWarCfg::mRanges;
std::map<int, CrossServiceWarAwardDef*> CrossServiceWarCfg::mRankAward;
vector<CrossServiceWarAward*> CrossServiceWarCfg::mCrossServiceWarResultAward;
int CrossServiceWarCfg::mfreetimes = 0;
int CrossServiceWarCfg::mOpenlvl = 0;
int CrossServiceWarCfg::mTimeout = 0;
vector<CrossServiceWarRankDataDef*> CrossServiceWarCfg::mRankDataRule;
vector<int> CrossServiceWarCfg::mCooldownTime;
vector<int> CrossServiceWarCfg::mFreshCost;
int CrossServiceWarCfg::mRobotCount = 0;
int CrossServiceWarCfg::mAwardPointPeriod = 0;

void CrossServiceWarCfg::load(std::string fullPath)
{
    mRankDataRule.clear();
    try
    {
        GameInifile inifile(fullPath);
        
        string ranges = inifile.getValue("rankranges", "ranges", "").c_str();
        StringTokenizer range(ranges, ";");
        mRanges.clear();
        mCrossServiceWarResultAward.clear();
        mRankAward.clear();
        
        assert(range.count() > 0);
        
        for (int i = 0; i < range.count(); i++) {
            int val = Utils::safe_atoi(range[i].c_str());
            mRanges.push_back(val);
        }
        
        for (int i = 1; i < mRanges.size(); i++) {
            
            string tmp = Utils::makeStr("awardpart%d",i);
            
            string protossStr = inifile.getValue(tmp, "protoss", "");
            string goldPartStr = inifile.getValue(tmp, "gold", "");
            string itemAwardStr = inifile.getValue(tmp, "awarditem", "");
            
            StringTokenizer protoss(protossStr, ";");
            StringTokenizer gold(goldPartStr, ";");
            StringTokenizer awardItems(itemAwardStr, ";");
            
            CrossServiceWarAwardDef* def = new CrossServiceWarAwardDef;
            for (int j = 0; j < protoss.count() && j < gold.count() && j < awardItems.count(); j++) {
                
                CrossServiceWarAward* awards = new CrossServiceWarAward;
                awards->protoss = Utils::safe_atoi(protoss[j].c_str());
                awards->gold = Utils::safe_atoi(gold[j].c_str());
                awards->itemAward = awardItems[j];
                
                def->mAwards.push_back(awards);
            }
            
            def->mIsShuzu = Utils::safe_atoi(inifile.getValue(tmp, "is_shuzu", "").c_str());
            def->mMinRank = Utils::safe_atoi(inifile.getValue(tmp, "xiaxian", "").c_str());
            def->mMaxRank = Utils::safe_atoi(inifile.getValue(tmp, "shangxian", "").c_str());
            
            mRankAward.insert(make_pair(i, def));
        }
        
        string protossStr = inifile.getValue("immediatereward", "protoss", "");
        string goldPartStr = inifile.getValue("immediatereward", "gold", "");
        
        StringTokenizer protoss(protossStr, ";");
        StringTokenizer gold(goldPartStr, ";");
        
        assert(protoss.count() == 2 && gold.count() == 2);
        
        for (int i = 0; i < protoss.count() && i < gold.count(); i++) {
            CrossServiceWarAward* immedAward = new CrossServiceWarAward;
            immedAward->protoss = Utils::safe_atoi(protoss[i].c_str());
            immedAward->gold = Utils::safe_atoi(gold[i].c_str());
            mCrossServiceWarResultAward.push_back(immedAward);
        }
        
        string freetimes = inifile.getValue("free", "freetimes");
        mfreetimes = Utils::safe_atoi(freetimes.c_str());
        
        string openlvl = inifile.getValue("free", "openlvl");
        mOpenlvl = Utils::safe_atoi(openlvl.c_str());
        
        mTimeout = inifile.getValueT("free", "timeout", 0);
        mRobotCount = inifile.getValueT("free", "robot_count", 40);
        mAwardPointPeriod = inifile.getValueT("free", "award_point_period", 0);
        
        int parts = inifile.getValueT("rankparts", "parts", 0);
        for (int i = 0;  i < parts; i++) {
            //            char tmp[128] = {0};
            //            sprintf(tmp, "rank%d",i + 1);
            
            string tmp = Utils::makeStr("rank%d",i + 1);
            int min = inifile.getValueT(tmp, "min", 0);
            int max = inifile.getValueT(tmp, "max", 0);
            int space = inifile.getValueT(tmp, "space", 0);
            
            if (min <= 0 || max <= 0 || space <= 0) {
                printf("Pvpcfg formate error [rank rule]\n");
                continue;
            }
            
            CrossServiceWarRankDataDef* data = new CrossServiceWarRankDataDef;
            data->mMin = min;
            data->mMax = max;
            data->space = space;
            
            mRankDataRule.push_back(data);
        }
        
        mFreshCost.clear();
        mCooldownTime.clear();
        string cooldowntimeStr = inifile.getValue("cooldown", "time", "");
        string freshcostStr = inifile.getValue("cooldown", "cost", "");
        
        StringTokenizer times(cooldowntimeStr, ";");
        StringTokenizer costs(freshcostStr, ";");
        
        for (int i = 0; i < times.count(); i++) {
            mCooldownTime.push_back(Utils::safe_atoi(times[i].c_str()));
        }
        
        for (int i = 0; i < costs.count(); i++) {
            mFreshCost.push_back(Utils::safe_atoi(costs[i].c_str()));
        }
        
    }
    catch(...)
    {
        
    }
}

int CrossServiceWarCfg::getTimeout()
{
    return mTimeout;
}

int CrossServiceWarCfg::getRobotCount()
{
    return mRobotCount;
}

CrossServiceWarAward* CrossServiceWarCfg::getRankAward(int rank)
{
    int index = 0;
    for (index = 0; index < mRanges.size(); index++) {
        if (rank <= mRanges[index]) {
            break;
        }
    }
    
    if (index >= mRanges.size()) {
        index = mRanges.size() - 1;
    }
    
    index = index < 1 ? 1 : index;
    CrossServiceWarAwardDef* def = mRankAward[index];
    
    if (def->mIsShuzu) {
        int tmp = rank - def->mMinRank;
        if (tmp >= def->mAwards.size()) {
            tmp = def->mAwards.size() - 1;
        }
        return def->mAwards[tmp];
    }
    else
    {
        return def->mAwards[0];
    }
    
}

CrossServiceWarAward* CrossServiceWarCfg::getResultAward(bool win)
{
    if (win) {
        return mCrossServiceWarResultAward[1];
    }
    return mCrossServiceWarResultAward[0];
}

int CrossServiceWarCfg::getfreeTimes()
{
    return mfreetimes;
}

int CrossServiceWarCfg::getOpenLvl()
{
    return mOpenlvl;
}

void CrossServiceWarCfg::getRankRule(int myrank, CrossServiceWarRankDataDef &output)
{
    for (int i = 0; i < mRankDataRule.size(); i++) {
        if (myrank > mRankDataRule[i]->mMax) {
            continue;
        }
        else if(myrank >= mRankDataRule[i]->mMin)
        {
            output.mMin = mRankDataRule[i]->mMin;
            output.mMax = mRankDataRule[i]->mMax;
            output.space = mRankDataRule[i]->space;
        }
    }
}




map<int, CsPvpGroupCfgDef*> CsPvpGroupCfg::sGroups;
map<int, CsPvpGroupCfgDef*> CsPvpGroupCfg::sServerIdIndex;
bool CsPvpGroupCfg::sAllServer = false;


void CsPvpGroupCfg::load(const char* fullpath) {
    try
    {
        GameInifile inifile(fullpath);
        vector<string> sections;
        inifile.getSections(std::back_inserter(sections));
        std::vector<string>::iterator itr = sections.begin();
        std::vector<string>::iterator itrEnd = sections.end();
        sAllServer = true;
        for (; itr != itrEnd; ++itr)
        {
            int groupId = Utils::safe_atoi(itr->c_str());
            string serverIdStr = inifile.getValue(*itr, "server_list");
            if (getCfgDef(groupId)) {
                assert(0);
            }

            if (serverIdStr == "-1") {
                sAllServer = true;
                break;
            }

            sAllServer = false;

            CsPvpGroupCfgDef* cfgDef = new CsPvpGroupCfgDef;
            cfgDef->mGroupId = groupId;

            sGroups[groupId] = cfgDef;
            vector<string> serverIdStrArr = StrSpilt(serverIdStr, ";");
            for (int i = 0; i < serverIdStrArr.size(); i++) {
                int serverId = Utils::safe_atoi(serverIdStrArr[i].c_str());
                if (getCfgDefByServerId(serverId)) {
                    assert(0);
                }

                cfgDef->mServerIds.push_back(serverId);
                sServerIdIndex[serverId] = cfgDef;
            }


            vector<string> closeTimeArr = inifile.getTable(*itr, "close_time");
            for (int i = 0; i < closeTimeArr.size(); i++) { 
                int year, month, day;
                string closeTimeStr = closeTimeArr[i];
                sscanf(closeTimeStr.c_str(), "%d-%d-%d", &year, &month, &day);
                struct tm ttm;
                memset(&ttm, 0, sizeof(ttm));
                ttm.tm_year = year - 1900;
                ttm.tm_mon = month - 1;
                ttm.tm_mday = day;
                time_t closeTime = mktime(&ttm);

                localtime_r(&closeTime, &ttm);
                int wday = ttm.tm_wday == 0 ? 7 : ttm.tm_wday;
                pair<int, int> closeWeek;
                closeWeek.first = closeTime - (wday - 1) * 24 * 3600;
                closeWeek.second = closeTime + (8 - wday) * 24 * 3600;
                cfgDef->mCloseWeeks.push_back(closeWeek);
            }
        }
    }
    catch(...)
    {
        log_error("CsPvpGroupCfg ERROR !!!");
    }
}

bool 
CsPvpGroupCfg::isGroupClose(int groupId, int testTime) {
    if (sAllServer) {
        return false;
    }
    CsPvpGroupCfgDef* cfgDef = getCfgDef(groupId);
    if (cfgDef == NULL) {
        return true;
    }

    for (int i = 0; i < cfgDef->mCloseWeeks.size(); i++) {
        pair<int, int>& closeWeek = cfgDef->mCloseWeeks[i];
        if (testTime >= closeWeek.first && testTime < closeWeek.second) {
            return true;
        }
    }

    return false;
}

bool CsPvpGroupCfg::isClose(int serverId, int testTime) {
    if (sAllServer) {
        return false;
    }
    CsPvpGroupCfgDef* cfgDef = getCfgDefByServerId(serverId);
    if (cfgDef == NULL) {
        return true;
    }

    for (int i = 0; i < cfgDef->mCloseWeeks.size(); i++) {
        pair<int, int>& closeWeek = cfgDef->mCloseWeeks[i];
        if (testTime >= closeWeek.first && testTime < closeWeek.second) {
            return true;
        }
    }

    return false;
}

CsPvpGroupCfgDef* 
CsPvpGroupCfg::getCfgDefByServerId(int serverId) {
    map<int, CsPvpGroupCfgDef*>::iterator iter = sServerIdIndex.find(serverId);
    if (iter == sServerIdIndex.end()) {
        return NULL;
    }

    return iter->second;
}

CsPvpGroupCfgDef* 
CsPvpGroupCfg::getCfgDef(int groupId) {
    map<int, CsPvpGroupCfgDef*>::iterator iter = sGroups.find(groupId);
    if (iter == sGroups.end()) {
        return NULL;
    }

    return iter->second;
}








//------------------------------
vector<LoadingTipsDef*> LoadingTipsDefCfg::cfg;

void LoadingTipsDefCfg::load(std::string fullPath)
{
    GameInifile inifile(fullPath);
    cfg.clear();
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++)
    {
        LoadingTipsDef *def = new LoadingTipsDef();
        def->mId = inifile.getValueT(*it, "id", 0);
        def->mTips = inifile.getValue(*it, "tips","");
        cfg.push_back(def);
    }
}

string LoadingTipsDefCfg::getRandomTips()
{
#if defined(CLIENT)
    if(cfg.size() == 0)
        return  " ";
    int ranNum = arc4random()%10000;
    ranNum = ranNum % cfg.size();
    if (cfg[ranNum])
    {
        return cfg[ranNum]->mTips;
    }
#endif
    return " ";
}



//---------------------------------------------------------

int DailyScheduleCfgDef::getPoint(int day)
{
    if (day < 1 || points.size() == 0) {
        return 0;
    }
    
    day  = day -1;
    
    if (day >= points.size()) {
        return 0;
    }
    
    return points[day];
}


map<int, DailyScheduleAwardCfgDef*> DailyScheduleCfg::awards;
map<int, DailyScheduleCfgDef*> DailyScheduleCfg::cfg;
vector<DailyScheduleCfgDef*> DailyScheduleCfg::cfgTypeIdx;

void DailyScheduleCfg::load(const char* fullPath, const char* awardPath)
{
    cfgTypeIdx.resize(getDailyScheduleTypeCount(), NULL);
    
    GameInifile inifile(fullPath);
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        string section = *iter;
        DailyScheduleCfgDef *def = new DailyScheduleCfgDef();
        def->cfgId = inifile.getValueT(*iter, "id", 0);
        def->name = inifile.getValue(*iter, "name","");
        def->desc= inifile.getValue(*iter, "desc","");
        def->title= inifile.getValue(*iter, "title","");
        vector<int> points;
        StringTokenizer token(inifile.getValue(*iter, "point",""),";");
        for (int i = 0; i < token.count(); i++)
        {
            points.push_back(atoi(token[i].c_str()));
        }
        def->icon = inifile.getValue(*iter, "icon","");
        def->iconScle = inifile.getValueT(*iter, "iconScle", 1.0f);
        def->points = points;
        def->activityID = inifile.getValueT(*iter, "activityID", 0);
        def->progress = inifile.getValueT(*iter, "progress",0);
        def->target = inifile.getValueT(*iter, "target", 0);
        def->openCfg =inifile.getValue(*iter, "openCfg","");
        def->openType = inifile.getValue(*iter, "openType","");
        
        int scheduleType = getDailyScheduleTypeByName(def->name.c_str());
        if (scheduleType == -1) {
            delete def;
            continue;
        }
        def->type = scheduleType;
        cfg[def->cfgId] = def;
        cfgTypeIdx[scheduleType] = def;
    }
    
    GameInifile awardIni(awardPath);
    std::list<std::string> awardSections;
    awardIni.getSections(std::back_inserter(awardSections));
    for (std::list<std::string>::iterator iter = awardSections.begin(); iter != awardSections.end(); iter++) {
        string section = *iter;
        
        int id = awardIni.getValueT(section, "id", -1);
        int point = awardIni.getValueT(section, "point", -1);
        string awardstr = awardIni.getValue(section, "award");
        int index = awardIni.getValueT(section, "index", -1);
        if (id < 0 || point < 0 || index < 0) {
            continue;
        }
        int startLvl, endLvl;
        string lvlStr = awardIni.getValue(section, "lvl");
        if (sscanf(lvlStr.c_str(), "%d~%d", &startLvl, &endLvl) != 2) {
            continue;
        }
        
        DailyScheduleAwardCfgDef* awardCfgDef = new DailyScheduleAwardCfgDef;
        awardCfgDef->mPoint = point;
        awardCfgDef->mAward = awardstr;
        awardCfgDef->mId = id;
        awardCfgDef->mIndex = index;
        awardCfgDef->mStartLvl = startLvl;
        awardCfgDef->mEndLvl = endLvl;
        awards[id] = awardCfgDef;
    }

}

DailyScheduleAwardCfgDef* DailyScheduleCfg::getAwardCfg(int id)
{
    map<int, DailyScheduleAwardCfgDef*>::iterator iter = awards.find(id);
    if (iter == awards.end()) {
        return NULL;
    }
    
    return iter->second;
}

DailyScheduleCfgDef* DailyScheduleCfg::getCfgByType(int type)
{
    if (type < 0 || type >= getDailyScheduleTypeCount()) {
        return NULL;
    }
    
    return cfgTypeIdx[type];
}

DailyScheduleCfgDef* DailyScheduleCfg::getCfg(int id)
{
    map<int, DailyScheduleCfgDef*>::iterator iter = cfg.find(id);
    if (iter == cfg.end()) {
        return NULL;
    }
    
    return iter->second;
}

int DailyScheduleCfg::getDailySheduleCfgListSize()
{
    return cfg.size();
}

DailyScheduleCfgDef* DailyScheduleCfg::getDailyScheduleCfgByName(string name)
{
    for (map<int,DailyScheduleCfgDef*>::iterator it = cfg.begin(); it != cfg.end() ; it++)
    {
        if(it->second->name == name)
        {
            return it->second;
        }
    }
    return NULL;
}

vector< DailyScheduleAwardCfgDef*> DailyScheduleCfg::getAwardListByLv(int lv)
{
    vector< DailyScheduleAwardCfgDef*> awardList;
    for (map<int, DailyScheduleAwardCfgDef*>::iterator it = awards.begin(); it != awards.end(); it++)
    {
        if (it->second->mStartLvl<=lv && it->second->mEndLvl>=lv)
        {
            awardList.push_back(it->second);
        }
    }
    return awardList;
}

vector<int> ActivateBagGridsCfg::activateCost;
void ActivateBagGridsCfg::load(const string &filepath)
{
    GameInifile inifile(filepath);
    int maxPage = inifile.getValueT("root","maxpage",0);
    string needRmbStr = inifile.getValue("root","needrmb");
    StringTokenizer token(needRmbStr,";");
    assert(maxPage == token.count());
    
    for(int i=0; i< token.count();i++)
    {
        int cost = atoi(token[i].c_str());
        activateCost.push_back(cost);
    }
}
int ActivateBagGridsCfg::getMaxPage()
{
    return  activateCost.size();
}
int ActivateBagGridsCfg::getCfg(int page)
{
    if(page <1 || page > activateCost.size())
        return 0x7FFFFFFF;
    return activateCost[page - 1];
}

#pragma mark----------------角色奖励-------------------------

RoleAwardCfgDef::RoleAwardCfgDef()
{
    
}
RoleAwardCfgDef::~RoleAwardCfgDef()
{
    
}
RoleAwardItemDef* RoleAwardCfgDef::getRoleAwardItemDefByid(int itemid)
{
    vector<RoleAwardItemDef*>::iterator itrFind = items.begin();
    for(; itrFind != items.end(); ++itrFind)
    {
        if((*itrFind)->itemid == itemid)
        {
            return (*itrFind);
        }
    }
    return NULL;
}

vector<RoleAwardItemDef*> &RoleAwardCfgDef::getRoleAwardItems()
{
    return items;
}

vector<RoleAwardCfgDef*> RoleAwardCfgMgr::awards;

void RoleAwardCfgMgr::load(const char* fullPath)
{
    try
    {
        GameInifile inifile(fullPath);
        awards.clear();
        
        set<int> tmpAwardId;
        
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
        {
            RoleAwardCfgDef* award = new RoleAwardCfgDef();
            award->setCfgid(inifile.getValueT(*iter, "pageId", 0));
            award->setDays(inifile.getValueT(*iter, "days", 0));
            string contents = inifile.getValue(*iter, "awards");
            string viplvs = inifile.getValue(*iter, "vipLv");
            string ids = inifile.getValue(*iter, "ids");
            StringTokenizer itemTokens(contents,";");
            StringTokenizer vipTokens(viplvs, ";");
            StringTokenizer idTokens(ids, ";");
            assert(itemTokens.count() == vipTokens.count());
            assert(vipTokens.count() == idTokens.count());
            
            for(int i = 0; i < itemTokens.count(); ++i)
            {
                RoleAwardItemDef* def = new RoleAwardItemDef();
                def->item = itemTokens[i];
                def->viplv = Utils::safe_atoi(vipTokens[i].c_str());
                
                int awardId = Utils::safe_atoi(idTokens[i].c_str());
                
                //既然不能有重复的id，就在程序里加个检查，总比策划肉眼去检查靠谱 by wangzhigang
                if (tmpAwardId.find(awardId) != tmpAwardId.end()) {
                    log_error(" roleaward.ini 有重复的奖励id: "<<awardId<<" in day: "<<award->getDays());
                    assert(0);
                }
                
                def->itemid = awardId;
                
                award->items.push_back(def);
                
                tmpAwardId.insert(awardId);
            }
            awards.push_back(award);
        }
        
        if (awards.size() <= 0) {
            log_info("not exist config ------ "<<fullPath);
            assert(0);
        }
        
    }
    catch(...)
    {
        
    }
}

RoleAwardCfgDef* RoleAwardCfgMgr::getRoleAwardCfgDefByid(int cfgid)
{
    Iterator itrFind = awards.begin();
    for(; itrFind != awards.end(); ++itrFind)
    {
        if((*itrFind)->getCfgid() == cfgid)
        {
            return (*itrFind);
        }
    }
    return NULL;
}

RoleAwardItemDef* RoleAwardCfgMgr::getRoleAwardItemDefByRoleAwardItemId(int roleAwardItemId)
{
    Iterator itrFind = awards.begin();
    for(; itrFind != awards.end(); ++itrFind)
    {
        vector<RoleAwardItemDef*>::iterator roleAwardItemitrFind = (*itrFind)->items.begin();
        for (; roleAwardItemitrFind != (*itrFind)->items.end(); roleAwardItemitrFind++)
        {
            if((*roleAwardItemitrFind)->itemid == roleAwardItemId)
            {
                return (*roleAwardItemitrFind);
            }
        }
    }
    return NULL;
}

vector<RoleAwardCfgDef*> &RoleAwardCfgMgr::getRoleAwardCfgDefs()
{
    return awards;
}


map<int, string> ColorCfg::sColors;
string ColorCfg::sDefaultColor = "255,255,255";

void ColorCfg::load(const char* fullpath)
{
    GameInifile inifile(fullpath);
    
    vector<string> sections;
    inifile.getSections(back_inserter(sections));
    
    for (int i = 0; i < sections.size(); i++) {
        int colorTag = inifile.getValueT(sections[i], "tag", 0);
        string colorStr = inifile.getValue(sections[i], "color", sDefaultColor);
        if (colorTag == 0) {
            break;
        }
        
        sColors[colorTag] = colorStr;
    }
}
string ColorCfg::getColor(int tag)
{
    map<int, string>::iterator iter = sColors.find(tag);
    if (iter == sColors.end()) {
        return sDefaultColor;
    }
    
    return iter->second;
}

#pragma mark----------------觉醒-------------------------

void RoleAwakeCfgDef::addSkillReplace(int job, string& skills, string& skillDes)
{
    StringTokenizer skillArray(skills, ";");
    StringTokenizer skillDesArray(skillDes,";");
    
    for (int i = 0; i < skillArray.count(); i++) {
        StringTokenizer skillInfo(skillArray[i].c_str(), ":");
        if (skillInfo.count() < 2) {
            log_error("roleAwake newSkill cfg error");
            assert(0);
        }
        
        AwakeSkillReplace* skill = new AwakeSkillReplace;
        
        skill->preSkillId = Utils::safe_atoi(skillInfo[0].c_str());
        skill->newSkillId = Utils::safe_atoi(skillInfo[1].c_str());
        
        if(skillDesArray.count() > i)
        {
            skill->skillDescrip = skillDesArray[i];
        }
        
//        SkillCfgDef* preSkillCfg = SkillCfg::getCfg(skill->preSkillId);
//        SkillCfgDef* newSkillCfg = SkillCfg::getCfg(skill->newSkillId);
//        
//        if (preSkillCfg == NULL || newSkillCfg == NULL) {
//            log_error("awake SkillReplace skillCfg not exist: preSkillid - "<<skill->preSkillId<<", newSkillid - "<<skill->newSkillId);
//            assert(0);
//        }
//        
//        newSkillCfg->awakeLvl = awakeLvl;
        
        skillReplace[job].push_back(skill);
    }
}

void RoleAwakeCfgDef::addPropAddtion(int job, string& propsInfo)
{
    StringTokenizer propInfoArray(propsInfo, ";");
    for (int i = 0; i < propInfoArray.count(); i++) {
        StringTokenizer propInfo(propInfoArray[i].c_str(), ":");
        if (propInfo.count() < 2) {
            log_error("roleAwake propAdd cfg error");
            assert(0);
        }
        
        AwakePropAddtion* prop = new AwakePropAddtion;
        
        prop->propName = propInfo[0];
        prop->addRatio = atof(propInfo[1].c_str());
        
        propAddtion[job].push_back(prop);
    }
}

void RoleAwakeCfgDef::addNeedItemInfo(string& itemStr, string& externDes)
{
    StringTokenizer itemArray(itemStr, ";");
    StringTokenizer externDesArray(externDes, ";");
    
    for (int i = 0; i < itemArray.count(); i++) {
        
        StringTokenizer itemInfo(itemArray[i], "*");
        if (itemInfo.count() < 2) {
            log_error("roleAwake needItem cfg error");
            assert(0);
        }
        
        ItemCell itemcell;
        itemcell.itemid = Utils::safe_atoi(itemInfo[0].c_str());
        itemcell.count = Utils::safe_atoi(itemInfo[1].c_str());
        
        if (i < externDesArray.count()) {
            itemcell.externDes = externDesArray[i];
        }
        
        this->needItem.push_back(itemcell);
    }
}

map<int, RoleAwakeCfgDef*> RoleAwakeCfg::mAwakeCfgData;
void RoleAwakeCfg::load(const char* fullPath)
{
    try
    {
        mAwakeCfgData.clear();
        
        GameInifile inifile(fullPath);
        mAwakeCfgData.clear();
    
        std::list<std::string> sections;
        inifile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
        {
            if (!inifile.exists(*iter)) {
                continue;
            }
            
            RoleAwakeCfgDef* def = new RoleAwakeCfgDef;
            
            if (def == NULL) {
                log_error("create RoleAwakeCfgDef error");
                assert(0);
            }
            
            int awakeLvl = Utils::safe_atoi((*iter).c_str());
            def->awakeLvl = awakeLvl;
            def->name = inifile.getValue(*iter, "names");
            def->needLvl = inifile.getValueT(*iter, "lvl", 0);
            def->resid = inifile.getValueT(*iter, "resid", 0);
            
            def->fashionCollectAdd = inifile.getValueT(*iter, "fashionCollectAdd", 0.0f);
            def->weaponEnchantsAdd = inifile.getValueT(*iter, "weaponEnchantsAdd", 0.0f);
            
            string mageSkillsInfo = inifile.getValue(*iter, "mSkill");
            string warriorSkillsInfo = inifile.getValue(*iter, "wSkill");
            string assassinSkillsInfo = inifile.getValue(*iter, "aSkill");
            
            string mageNewSkillDes = "";
            string warriorNewSkillDes = "";
            string assassinNewSkillDes = "";
            
#if defined(CLIENT)
            mageNewSkillDes = inifile.getValue(*iter, "mSkilldesc");
            warriorNewSkillDes = inifile.getValue(*iter, "wSkilldesc");
            assassinNewSkillDes = inifile.getValue(*iter, "aSkilldesc");
#else
#endif
            def->addSkillReplace(eMage, mageSkillsInfo, mageNewSkillDes);
            def->addSkillReplace(eWarrior, warriorSkillsInfo, warriorNewSkillDes);
            def->addSkillReplace(eAssassin, assassinSkillsInfo, assassinNewSkillDes);
            
            string magePropInfo = inifile.getValue(*iter, "mStatistics");
            string warriorPropInfo = inifile.getValue(*iter, "wStatistics");
            string assassinPropInfo = inifile.getValue(*iter, "aStatistics");

            def->addPropAddtion(eMage, magePropInfo);
            def->addPropAddtion(eWarrior, warriorPropInfo);
            def->addPropAddtion(eAssassin, assassinPropInfo);
            
            string needItems = inifile.getValue(*iter, "needitem");
            string needItemsDes = "";
#if defined(CLIENT)
            needItemsDes = inifile.getValue(*iter, "goodsdesc");
#else
#endif
            def->addNeedItemInfo(needItems, needItemsDes);
            
            def->lightResIds[eMage] = inifile.getValueT(*iter, "mLightRes", 0);
            def->lightResIds[eWarrior] = inifile.getValueT(*iter, "wLightRes", 0);
            def->lightResIds[eAssassin] = inifile.getValueT(*iter, "aLightRes", 0);
            
            mAwakeCfgData[awakeLvl] = def;
        }
        
        RoleAwakeAddtionInfo outData;
        getRoleAwakeInfo(eMage, 2, outData);
    }
    catch(...)
    {
        
    }
}

bool RoleAwakeCfg::getRoleAwakeInfo(int job, int awakeLvl, RoleAwakeAddtionInfo& outData)
{
    map<int, RoleAwakeCfgDef*>::iterator iter = mAwakeCfgData.find(awakeLvl);
    
    if (iter == mAwakeCfgData.end() || iter->second == NULL) {
        return false;
    }
    
    //读取对应职业的替换技能
    map<int, RoleAwakeSkillReplace>::iterator skillIter = iter->second->skillReplace.find(job);
    if (skillIter != iter->second->skillReplace.end()) {
        
        for (int i = 0; i < skillIter->second.size(); i++) {
            AwakeSkillReplace skill;
            skill.preSkillId = skillIter->second[i]->preSkillId;
            skill.newSkillId = skillIter->second[i]->newSkillId;
            skill.skillDescrip = skillIter->second[i]->skillDescrip;
            outData.skillsReplace.push_back(skill);
        }
    }
    
    //读取对应职业的属性加成
    map<int, RoleAwakePropAddtion>::iterator propIter = iter->second->propAddtion.find(job);
    if (propIter != iter->second->propAddtion.end()) {
        
        for (int i = 0; i < propIter->second.size(); i++) {
            AwakePropAddtion propInfo;
            propInfo.propName = propIter->second[i]->propName;
            propInfo.addRatio = propIter->second[i]->addRatio;
            outData.propAddtion.push_back(propInfo);
        }
    }
    map<ObjJob, int>::iterator lightIter = iter->second->lightResIds.find((ObjJob)job);
    if (lightIter != iter->second->lightResIds.end())
    {
        outData.lightResId = lightIter->second;
    }
    outData.needItem = iter->second->needItem;
    outData.needLvl = iter->second->needLvl;
    outData.resid = iter->second->resid;
    outData.fashionCollectAdd = iter->second->fashionCollectAdd;
    outData.weaponEnchantsAdd = iter->second->weaponEnchantsAdd;
    outData.awakeName = iter->second->name;
    outData.awakeLvl = iter->second->awakeLvl;
    return true;
}

void  RoleAwakeCfg::getAllAwakePropAddtion(ObjJob job, map< string, AwakePropAddtion>&awakePropAddtionMap)
{
    map<int, RoleAwakeCfgDef*>::iterator iter = mAwakeCfgData.begin();
    for (; iter != mAwakeCfgData.end(); iter++)
    {
        if (iter->second != NULL)
        {
            map<int, RoleAwakePropAddtion>::iterator propIter = iter->second->propAddtion.find(job);
            for(int i=0; i < propIter->second.size(); i++)
            {
                map<string, AwakePropAddtion>::iterator iterFind = awakePropAddtionMap.find(propIter->second[i]->propName);
                if(iterFind != awakePropAddtionMap.end())
                {
                    AwakePropAddtion propInfo;
                    propInfo.propName = propIter->second[i]->propName;
                    propInfo.addRatio = propIter->second[i]->addRatio + iterFind->second.addRatio;
                    iterFind->second = propInfo;
                }else
                {
                    AwakePropAddtion propInfo;
                    propInfo.propName = propIter->second[i]->propName;
                    propInfo.addRatio = propIter->second[i]->addRatio;
                    awakePropAddtionMap.insert(pair<string, AwakePropAddtion>(propIter->second[i]->propName,propInfo));
                }
            }
        }
    }
}

void  RoleAwakeCfg::getAllAwakeSkillReplace(ObjJob job,vector<AwakeSkillReplace> &skillsReplace)
{
    map<int, RoleAwakeCfgDef*>::iterator iter = mAwakeCfgData.begin();
    for (; iter != mAwakeCfgData.end(); iter++)
    {
        if (iter->second != NULL)
        {
          map<int, RoleAwakeSkillReplace>::iterator skillIter = iter->second->skillReplace.find(job);
            if (skillIter != iter->second->skillReplace.end())
            {
                for (int i = 0; i < skillIter->second.size(); i++)
                {
                    AwakeSkillReplace skill;
                    skill.preSkillId = skillIter->second[i]->preSkillId;
                    skill.newSkillId = skillIter->second[i]->newSkillId;
                    skill.skillDescrip = skillIter->second[i]->skillDescrip;
                    skillsReplace.push_back(skill);
                }
            }
        }
    }
}

void RoleAwakeCfg::getAlreadySkillReplace(ObjJob job,int roleAwakeLvl,vector<AwakeSkillReplace> &skillsReplace)
{
    for(int i = 1;i <= roleAwakeLvl; i++)
    {
        map<int, RoleAwakeCfgDef*>::iterator iter = mAwakeCfgData.find(i);
        if (iter != mAwakeCfgData.end() && iter->second != NULL)
        {
            map<int, RoleAwakeSkillReplace>::iterator skillIter = iter->second->skillReplace.find(job);
            if (skillIter != iter->second->skillReplace.end())
            {
                for (int i = 0; i < skillIter->second.size(); i++) {
                    AwakeSkillReplace skill;
                    skill.preSkillId = skillIter->second[i]->preSkillId;
                    skill.newSkillId = skillIter->second[i]->newSkillId;
                    skill.skillDescrip = skillIter->second[i]->skillDescrip;
                    skillsReplace.push_back(skill);
                }
            }
            
        }
    }

}

int RoleAwakeCfg::getRoleAwakeLightResId(ObjJob job,int awakeLvl)
{
    map<int, RoleAwakeCfgDef*>::iterator iter = mAwakeCfgData.find(awakeLvl);
    if (iter == mAwakeCfgData.end() ||  iter->second == NULL)
    {
        return 0;
    }
    map< ObjJob, int>::iterator iterFind = iter->second->lightResIds.find(job);
    if (iterFind == iter->second->lightResIds.end())
    {
        return 0;
    }
    return iterFind->second;
}


int EquipBaptizeCfg::mBatizeStoneId =0;
int EquipBaptizeCfg::mBatizeCost = 0;
int EquipBaptizeCfg::mRecoveryCost = 0;
map<int,EquipBaptizeSlot*>  EquipBaptizeCfg::equipBaptizeSlots;
vector<EquipBaptizeAttriDef*> EquipBaptizeCfg::equipBaptizeAttriDefData;
void EquipBaptizeCfg::load(const char* equipBatizeSlotPath,const char* equipBatizeAttriPath)
{
    try {
        equipBaptizeSlots.clear();
        GameInifile equipBatizeSlotfile(equipBatizeSlotPath);
        std::list<std::string> sections;
        equipBatizeSlotfile.getSections(std::back_inserter(sections));
        for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
        {
            if (equipBatizeSlotfile.exists(*iter))
            {
                if (strcmp((*iter).c_str(), "common") == 0)
                {
                    EquipBaptizeCfg::setBatizeStoneId(equipBatizeSlotfile.getValueT(*iter, "batizeStoneId", 0));
                    EquipBaptizeCfg::setBatizeCost(equipBatizeSlotfile.getValueT(*iter, "batizeCost", 0));
                    EquipBaptizeCfg::setRecoveryCost(equipBatizeSlotfile.getValueT(*iter, "recoveryCost", 0));
                    continue;
                }
                int qua = equipBatizeSlotfile.getValueT(*iter, "qua", 0);
                string quaName = equipBatizeSlotfile.getValue(*iter, "quaName");
                int slotNum = equipBatizeSlotfile.getValueT(*iter, "slotNum", 0);
                string slotDesc = equipBatizeSlotfile.getValue(*iter, "slotDesc");
                EquipBaptizeSlot *equipBaptizeSlot = new EquipBaptizeSlot();
                equipBaptizeSlot->qua = qua;
                equipBaptizeSlot->quaName = quaName;
                equipBaptizeSlot->slotNum = slotNum;
                equipBaptizeSlot->slotDesc = slotDesc;
                equipBaptizeSlots[ qua ] = equipBaptizeSlot;
            }
            
        }
        
        equipBaptizeAttriDefData.clear();
        GameInifile equipBatizeAttrifile(equipBatizeAttriPath);
        std::list<std::string> equipBatizeAttriSections;
        equipBatizeAttrifile.getSections(std::back_inserter(equipBatizeAttriSections));
        for (std::list<std::string>::iterator iter = equipBatizeAttriSections.begin(); iter != equipBatizeAttriSections.end(); iter++)
        {
            if (equipBatizeAttrifile.exists(*iter))
            {
                int id =  Utils::safe_atoi((*iter).c_str());
                string attriCnName = equipBatizeAttrifile.getValue(*iter,"attriCnName");
                string attriName = equipBatizeAttrifile.getValue(*iter,"attriName");
                int pro = equipBatizeAttrifile.getValueT(*iter,"pro",0);
                string attriValuesStr = equipBatizeAttrifile.getValue(*iter,"attriValues");
                string attriProsStr = equipBatizeAttrifile.getValue(*iter,"attriPros");
                
                EquipBaptizeAttriDef *equipBaptizeAttriDef = new EquipBaptizeAttriDef();
                equipBaptizeAttriDef->attriCnName = attriCnName;
                equipBaptizeAttriDef->attriName = attriName;
                equipBaptizeAttriDef->pro = pro;
                
                StringTokenizer attriValuesToken(attriValuesStr,";");
                StringTokenizer attriProsToken(attriProsStr,";");
                assert(attriValuesToken.count() == attriProsToken.count());
                for (int i = 0; i < attriValuesToken.count(); i++)
                {
                    EquipBaptizeAttri *equipBaptizeAttri = new EquipBaptizeAttri();
                    equipBaptizeAttri->attriValue = attriValuesToken[i];
                    equipBaptizeAttri->attriPro = atoi(attriProsToken[i].c_str());
                    equipBaptizeAttriDef->equipBaptizeAttris.push_back(equipBaptizeAttri);
                }
                equipBaptizeAttriDefData.push_back(equipBaptizeAttriDef);
            }
        }
        
    } catch (...) {
        log_error("EquipBaptizeCfg ERROR !!!");
    }

}

EquipBaptizeSlot* EquipBaptizeCfg::getEquipBaptizeSlotByQua(int qua)
{
    map<int,EquipBaptizeSlot*>::iterator iter = equipBaptizeSlots.find(qua);
    if (iter == equipBaptizeSlots.end() || iter->second == NULL)
    {
        return NULL;
    }
    return iter->second;
}

pair<string, string> EquipBaptizeCfg::makeBaptizeProp()
{
    int vsize = equipBaptizeAttriDefData.size();
    
    int pros = 0;
    for (int i = 0; i < vsize; i++) {
        pros += equipBaptizeAttriDefData[i]->pro;
    }
    
    if (pros <= 0) {
        
        return make_pair("", "");
    }
    
    int randnum = rand() % pros;
    pros = 0;
    for (int i = 0; i < vsize; i++) {
        pros += equipBaptizeAttriDefData[i]->pro;
        if (randnum < pros) {
            return equipBaptizeAttriDefData[i]->getProp();
        }
    }
    
    return make_pair("", "");
}

string getJsonValue(string& str, string key)
{
    Json::Reader jsonreader;
    Json::Value jsonval;
    
    jsonreader.parse(str, jsonval);
    return jsonval[key.c_str()].asString();
}

pair<string, string> EquipBaptizeCfg::getBaptizeProp(string& data, int qua, int index)
{
    string baptizeData = getJsonValue(data, getBaptizeKeyName());
    
    string quaStr = Utils::makeStr("%d", qua);
    
    if (quaStr.empty()) {
        return make_pair("", "");
    }
    
    string quaBaptizeprop = getJsonValue(baptizeData, quaStr);
    if (quaBaptizeprop.empty()) {
        return make_pair("", "");
    }
    
    Json::Reader propJsonreader;
    Json::Value propJsonval;
    propJsonreader.parse(quaBaptizeprop, propJsonval);
    
    if (!propJsonval.isArray() || propJsonval.size() <= index) {
        return make_pair("", "");
    }
    
    string propname = propJsonval[index]["name"].asString();
    string val = propJsonval[index]["val"].asString();
    
    return make_pair(propname, val);
}

pair<string, string> EquipBaptizeCfg::getBackupBaptizeProp(string& data, int qua, int index)
{
    string baptizeData = getJsonValue(data, getBackupBaptizeKeyName());
    
    string quaStr = Utils::makeStr("%d", qua);
    
    if (quaStr.empty()) {
        return make_pair("", "");
    }
    
    string quaBaptizeprop = getJsonValue(baptizeData, quaStr);
    if (quaBaptizeprop.empty()) {
        return make_pair("", "");
    }
    
    Json::Reader propJsonreader;
    Json::Value propJsonval;
    propJsonreader.parse(quaBaptizeprop, propJsonval);
    
    if (!propJsonval.isArray() || propJsonval.size() <= index) {
        return make_pair("", "");
    }
    
    string propname = propJsonval[index]["name"].asString();
    string val = propJsonval[index]["val"].asString();
    
    return make_pair(propname, val);
}


EquipBaptizeAttriDef* EquipBaptizeCfg::getEquipBaptizeAttriDefbyattriName(string _attriName)
{
    EquipBaptizeAttriDef *equipBaptizeAttriDef = NULL;
     vector<EquipBaptizeAttriDef*>::iterator iter = equipBaptizeAttriDefData.begin();
    for (; iter != equipBaptizeAttriDefData.end(); iter++)
    {
        if ((*iter)->attriName == _attriName)
        {
            equipBaptizeAttriDef = (*iter);
            break;
        }
    }
    
    return equipBaptizeAttriDef;
}

void EquipBaptizeCfg::calcBaptizeProp(const char* propName, const char* propVal, BattleProp& battleProp)
{
    if (strcmp(propName, "") == 0) {
        return;
    }
    
    BattleProp prop;
    float floatVal = atof(propVal);
    if (strcmp(propName, "atk") == 0)
    {
        prop.setAtk(prop.getAtk() + floatVal);
    }
    else if(strcmp(propName, "def") == 0)
    {
        prop.setDef(prop.getDef() + floatVal);
    }
    else if(strcmp(propName, "hp") == 0)
    {
        prop.setMaxHp(prop.getMaxHp() + floatVal);
        prop.setHp(prop.getMaxHp());
    }
    else if(strcmp(propName, "hit") == 0)
    {
        prop.setHit(prop.getHit() + floatVal);
    }
    else if(strcmp(propName, "dodge") == 0)
    {
        prop.setDodge(prop.getDodge() + floatVal);
    }
    
    battleProp += prop;
}

RetinueOutline * RetinueModDef::getOutlineData(int lvl)
{
    for(int i=0; i< outlineData.size(); i++)
    {
        if(outlineData[i].lvl >= lvl)
        {
            return &(outlineData[i]);
        }
    }
    return NULL;
}
string RetinueSkillDef::getSkillDescBySkillLvl(int lvl)
{
    string canshu1,canshu2,canshu3,canshu4,canshu5;
    
    StringTokenizer canshu1Tokenizer(canshu1_s,";");
    StringTokenizer canshu2Tokenizer(canshu2_s,";");
    StringTokenizer canshu3Tokenizer(canshu3_s,";");
    StringTokenizer canshu4Tokenizer(canshu4_s,";");
    StringTokenizer canshu5Tokenizer(canshu5_s,";");
    
    set_skill_level_fields_number(canshu1, canshu1Tokenizer, lvl-1);
    set_skill_level_fields_number(canshu2, canshu2Tokenizer, lvl-1);
    set_skill_level_fields_number(canshu3, canshu3Tokenizer, lvl-1);
    set_skill_level_fields_number(canshu4, canshu4Tokenizer, lvl-1);
    set_skill_level_fields_number(canshu5, canshu5Tokenizer, lvl-1);
    
    string descVal = Utils::makeStr(commonDesc.c_str(), canshu1.c_str(), canshu2.c_str(), canshu3.c_str(),canshu4.c_str(),canshu5.c_str());
    return descVal;

}

bool RetinueModDef::checkRetinueLvlupMaterial(int materialId)
{
    for (int i = 0; i < specificMaterialForLvlup.size(); i++) {
        
        if (materialId == specificMaterialForLvlup[i]) {
            return true;
        }
    }
    
    for (int i = 0; i < uniMaterialForLvlup.size(); i++) {
        
        if (materialId == uniMaterialForLvlup[i]) {
            return true;
        }
    }
    
    return false;
}

bool RetinueModDef::checkRetinueStarupMaterial(int materialId)
{
    for (int i = 0; i < mNeedMaterials.size(); i++)
    {
        if (materialId == mNeedMaterials[i].itemid) {
            return true;
        }
    }
    
    for (int i = 0; i < uniMaterialForStarup.size(); i++) {
        
        if (materialId == uniMaterialForStarup[i]) {
            return true;
        }
    }
    
    return false;
}

map<int, RetinueModDef *> RetinueCfg::mRetinueMods;
map<int, RetinueSkillDef *> RetinueCfg::mRetinueSkills;
map<int, RetinueSummonDef *> RetinueCfg::mRetinueSummon;
map<int, SpiritSpeakDef *> RetinueCfg::mSpiritSpeak;

void RetinueCfg::load(string modPath, string skillPath, string summonPath, string spiritPath)
{
    loadMod(modPath);
    loadSkills(skillPath);
    loadSummon(summonPath);
    loadSpiritSpeak(spiritPath);
}

void RetinueCfg::loadMod(string& path)
{
    GameInifile inifile(path);
    vector<string> sections;
    inifile.getSections(back_inserter(sections));
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
    {
        RetinueModDef * def = new RetinueModDef;
        
        def->retinueId = Utils::safe_atoi((*iter).c_str());
        
        def->name = inifile.getValue(*iter, "name");
        def->position = inifile.getValueT(*iter, "position", 0);
        def->steps = inifile.getValueT(*iter, "steps", 0);
        
        def->maxLvl = inifile.getValueT(*iter, "maxlvl", 0);
        def->maxStar = inifile.getValueT(*iter, "maxstar", 0);
        
        def->bExp = inifile.getValueT(*iter, "bexp", 0);
        def->bStarExp = inifile.getValueT(*iter, "bstarexp", 0);
        
        def->bAtk = inifile.getValueT(*iter, "batk", 0);
        def->bDef = inifile.getValueT(*iter, "bdef", 0);
        def->bHp = inifile.getValueT(*iter, "bhp", 0);
        def->bHit = inifile.getValueT(*iter, "bhit", 0.0f);
        def->bDodge = inifile.getValueT(*iter, "bdodge", 0.0f);
        def->bCri = inifile.getValueT(*iter, "bcri", 0.0f);
        
        def->dropDesc = inifile.getValue(*iter,"dropdesc");
        
        string materialStr = inifile.getValue(*iter, "materials", "");
        string uniMaterialForStarup = inifile.getValue(*iter, "uni_star_up_material", "");
        StringTokenizer materialToken(materialStr,";");
        StringTokenizer uniMaterialForStarupToken(uniMaterialForStarup,";");
       
        for(int i=0; i< materialToken.count(); i++)
        {
            ItemCell cell;
            if(sscanf(materialToken[i].c_str(), "%d*%d",&cell.itemid,&cell.count) == 2)
            {
                def->mNeedMaterials.push_back(cell);
            }
        }
        for(int i=0; i< uniMaterialForStarupToken.count(); i++)
        {
            int materialId = Utils::safe_atoi(uniMaterialForStarupToken[i].c_str());
            def->uniMaterialForStarup.push_back(materialId);
        }
        
        string specificMaterial = inifile.getValue(*iter, "specific_lvlup_material", "");
        StringTokenizer specificMaterialToken(specificMaterial, ";");
        string uniMaterialForLvlup = inifile.getValue(*iter, "uni_lvlup_material", "");
        StringTokenizer uniLvlupMaterialTmp(uniMaterialForLvlup, ";");
        
        for(int i=0; i< specificMaterialToken.count(); i++)
        {
            int materialId = Utils::safe_atoi(specificMaterialToken[i].c_str());
            def->specificMaterialForLvlup.push_back(materialId);
        }
        for(int i=0; i< uniLvlupMaterialTmp.count(); i++)
        {
            int materialId = Utils::safe_atoi(uniLvlupMaterialTmp[i].c_str());
            def->uniMaterialForLvlup.push_back(materialId);
        }
        
        string lvlsString = inifile.getValue(*iter, "lvls","");
        string residsString = inifile.getValue(*iter, "resids","");
        string monstermgsString = inifile.getValue(*iter, "monstermgs","");
        string monsterheadsString = inifile.getValue(*iter,"monsterheads","");
        
        StringTokenizer lvlsToken(lvlsString,";");
        StringTokenizer residsToken(residsString,";");
        StringTokenizer monstermgsToken(monstermgsString,";");
        StringTokenizer monsterheadsToken(monsterheadsString,";");
        
        bool isEquality = (lvlsToken.count()== residsToken.count()
                           && lvlsToken.count() == monstermgsToken.count()
                           && lvlsToken.count() == monsterheadsToken.count());
        assert(isEquality);
        for(int i=0; i< lvlsToken.count(); i++)
        {
            RetinueOutline data;
            data.lvl = atoi(lvlsToken[i].c_str());
            data.resID = atoi(residsToken[i].c_str());
            data.retinueIamge =monstermgsToken[i];
            data.retinueHeadImage = monsterheadsToken[i];
            def->outlineData.push_back(data);
        }
        mRetinueMods.insert(make_pair(def->retinueId, def));
    }
}

void RetinueCfg::loadSkills(string& path)
{
    GameInifile inifile(path);
    vector<string> sections;
    inifile.getSections(back_inserter(sections));
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
    {
        RetinueSkillDef *def = new RetinueSkillDef;
        
        int retinueSkillId = Utils::safe_atoi((*iter).c_str());
        def->skillId = inifile.getValueT(*iter, "skillid", 0);
        def->name = inifile.getValue(*iter, "name");
        def->maxLvl = inifile.getValueT(*iter, "max_lvl", 0);
        //def->desc = inifile.getValue(*iter, "desc");
        def->powerConsum = inifile.getValueT(*iter, "power_consum", 0);
        def->setCoolingTime(atof(inifile.getValue(*iter, "coolingtime").c_str()));
        //def->setRol(atoi(inifile.getValue(*iter, "rol").c_str()));
        def->skillIcon = inifile.getValue(*iter, "skillicon");
        
        string skillsString = inifile.getValue(*iter, "skillids");

        StringTokenizer skillToken(skillsString,";");
        for(int i=0; i< skillToken.count(); i++)
        {
            def->skillIds.push_back(atoi(skillToken[i].c_str()));
        }
        
        int universalUpgMaterId = inifile.getValueT(*iter, "uniUpg_material", 0);
        int upgradeMaterialId = inifile.getValueT(*iter, "upg_material", 0);
        
        string uniUpgMaterCountStr = inifile.getValue(*iter, "uniUpg_mater_count", "");
        string upgMaterCountStr = inifile.getValue(*iter, "upg_mater_count");
        
        StringTokenizer uniUpgMaterCount(uniUpgMaterCountStr, ";");
        StringTokenizer upgMaterCount(upgMaterCountStr, ";");
        
        if ((def->maxLvl + 1) != uniUpgMaterCount.count() || (def->maxLvl + 1) != upgMaterCount.count()) {
            
            log_info("Retinue skill ini error -- "<<"RetinueSkillid :"<< def->skillId<<"maxLvl:"<<def->maxLvl<<"uniMaterCount:"<<uniUpgMaterCount.count()<<"upMaterCount:"<<upgMaterCount.count());
           assert(0);
        }
        
        for (int i = 0; i < def->maxLvl; i++)
        {
            int upCount = Utils::safe_atoi(upgMaterCount[i].c_str());
            ItemCell item(upgradeMaterialId, upCount);
            
            int uniCount = Utils::safe_atoi(uniUpgMaterCount[i].c_str());
            ItemCell uniItem(universalUpgMaterId, uniCount);
            
            def->upgradeMater.push_back(item);
            def->uniUpgradeMater.push_back(uniItem);
        }
        
        mRetinueSkills.insert(make_pair(retinueSkillId, def));
        //技能描述
        def->canshu1_s = inifile.getValue(*iter, "canshu1", "");
        def->canshu2_s = inifile.getValue(*iter, "canshu2", "");
        def->canshu3_s = inifile.getValue(*iter, "canshu3", "");
        def->canshu4_s = inifile.getValue(*iter, "canshu4", "");
        def->canshu5_s = inifile.getValue(*iter, "canshu5", "");
        def->commonDesc = inifile.getValue(*iter, "commondesc", "");
    }
}

void RetinueCfg::loadSummon(string& path)
{
    GameInifile inifile(path);
    vector<string> sections;
    inifile.getSections(back_inserter(sections));
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
    {
        RetinueSummonDef *def = new RetinueSummonDef;
        
        int index = Utils::safe_atoi((*iter).c_str());
        
        def->index = inifile.getValueT(*iter, "id", 0);
        def->ordinarySummon = inifile.getValueT(*iter, "ordinary_summon", 0);
        def->rmbSummon = inifile.getValueT(*iter, "rmb_summon", 0);
        def->rmbSummonCost = inifile.getValueT(*iter, "rmb_cost", 0);
        def->roleLvl = inifile.getValueT(*iter, "openLvl", 0);
        def->cooldown = inifile.getValueT(*iter, "cool_down", 0);
        def->resetTimes = inifile.getValueT(*iter, "reset_times", 0);
        def->resetCost = inifile.getValueT(*iter, "reset_cost", 0);
        def->backgroundSpr = inifile.getValue(*iter, "backgoundSpr");
        def->res_id_stand = inifile.getValueT(*iter, "res_id_stand", 0);
        def->res_id_run = inifile.getValueT(*iter, "res_id_run", 0);
        def->name = inifile.getValue(*iter, "name");
        string aWardString = inifile.getValue(*iter, "awards");
        
        std::vector<std::string> awards = StrSpilt(aWardString, ";");
        
        rewardsCmds2ItemArray(awards,def->mItems,NULL,NULL,NULL,NULL,NULL,NULL);

        mRetinueSummon.insert(make_pair(index, def));
    }
}

void RetinueCfg::loadSpiritSpeak(string& path)
{
    GameInifile inifile(path);
    vector<string> sections;
    inifile.getSections(back_inserter(sections));
    for (vector<string>::iterator iter = sections.begin(); iter != sections.end(); ++iter)
    {
        SpiritSpeakDef * def = new SpiritSpeakDef;
        int index = Utils::safe_atoi((*iter).c_str());
        def->spiritSpeakid = index;
        def->retinueStep = inifile.getValueT(*iter, "retinue_step", 0);
        def->gribCount = inifile.getValueT(*iter, "grib_count", 0);
        def->desc = inifile.getValue(*iter, "des");
        
        mSpiritSpeak.insert(make_pair(index, def));
    }
}

void RetinueCfg::getAllSpiritSpeakCfg(vector<SpiritSpeakDef *>& data)
{
    SpiritSpeakMap::iterator iter = mSpiritSpeak.begin();
    
    data.clear();
    
    for (;iter != mSpiritSpeak.end(); iter++)
    {
        data.push_back(iter->second);
    }
}

void RetinueCfg::getAllRetinueSummonCfg(vector<RetinueSummonDef *>& data)
{
    RetinueSummonMap::iterator iter = mRetinueSummon.begin();
    
    data.clear();
    
    for (;iter != mRetinueSummon.end(); iter++)
    {
        data.push_back(iter->second);
    }
}
RetinueSummonDef * RetinueCfg::getRecentlySummonDefByLvl(int rolelvl, int *summonIndex)
{
    vector<RetinueSummonDef *>data;
    getAllRetinueSummonCfg(data);
    int index = 0;
    for(int i=0;i < data.size(); i++)
    {
        if(data[i]->roleLvl <= rolelvl)
        {
            index = i;
        }
    }
    if(summonIndex)
    {
        *summonIndex = index;
    }
    return data[index];
}



map<int, RetinueSkillDef *> RetinueCfg::getAllSkillsCfg()
{
    return mRetinueSkills;
}

SpiritSpeakDef * RetinueCfg::getSpiritSpeakCfg(int index)
{
    SpiritSpeakMap::iterator iter = mSpiritSpeak.find(index);
    
    if (iter == mSpiritSpeak.end()) {
        return NULL;
    }
    
    return iter->second;
}

RetinueSummonDef* RetinueCfg::getRetinueSummonCfg(int retinueIndex)
{
    RetinueSummonMap::iterator iter = mRetinueSummon.find(retinueIndex);
    
    if (iter == mRetinueSummon.end()) {
        return NULL;
    }
    
    return iter->second;
}

RetinueModDef * RetinueCfg::getRetinueModCfg(int retinueId)
{
    RetinueModMap::iterator iter = mRetinueMods.find(retinueId);
    
    if (iter == mRetinueMods.end()) {
        return NULL;
    }
    
    return iter->second;
}

RetinueSkillDef * RetinueCfg::getRetinueSkillCfg(int skillId)
{
    RetinueSkillMap::iterator iter = mRetinueSkills.find(skillId);
    
    if (iter == mRetinueSkills.end()) {
        return NULL;
    }
    
    return iter->second;
}


vector<RetinueModDef *> RetinueCfg::getAllRetinueModDefsByStepsAndPos(int steps, int postions)
{
    vector<RetinueModDef *> tempDatas;
    tempDatas.clear();
    map<int, RetinueModDef *> ::iterator iter = mRetinueMods.begin();
    for(; iter != mRetinueMods.end(); iter ++)
    {
        
        if(iter->second-> steps == steps && iter->second->position == postions)
        {
            tempDatas.push_back(iter->second);
        }
    }
    return tempDatas;
}

vector<RetinueModDef *> RetinueCfg::getAllRetinueModDefsByConfig()
{
    vector<RetinueModDef *>tempDatas;
    map<int, RetinueModDef *> ::iterator iter = mRetinueMods.begin();
    for(; iter != mRetinueMods.end(); iter ++)
    {
        tempDatas.push_back(iter->second);
    }
    return tempDatas;
}

int RetinueCfg::getRetinueSummonResetCost(int index)
{
    map<int, RetinueSummonDef *> ::iterator iter = mRetinueSummon.begin();
    
    if (iter != mRetinueSummon.end())
    {
        return iter->second->resetCost;
    }
    
    return 0;
}

int RetinueCfg::getRetinueSummonResetTimes(int index)
{
    map<int, RetinueSummonDef *> ::iterator iter = mRetinueSummon.begin();
    
    if (iter != mRetinueSummon.end())
    {
        return iter->second->resetTimes;
    }
    
    return 0;
}


void
ServerGroupCfg::load(string config, int type)
{
    IniFile inifile(config);
    
    int rankListType = type;

    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    set<int> allServers;
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        int groupid = Utils::safe_atoi(iter->c_str());
        
        int beginTime = Utils::parseDate(inifile.getValue(*iter, "beginTime").c_str());
        int endTime   = Utils::parseDate(inifile.getValue(*iter, "endTime").c_str());
        assert(!(beginTime == -1 || endTime == -1 || beginTime >= endTime));
        
        int sendAwardTime = Utils::parseDate(inifile.getValue(*iter, "sendAwardTime").c_str());
        assert(!(sendAwardTime == -1));
        
		int awardNum  = inifile.getValueT(*iter, "awardNum", 0);
        vector<string> awards;
		for (int i = 1; i <= awardNum; i++)
		{
            char awardName[32];
			sprintf(awardName, "Num%dAward", i);
			awards.push_back(inifile.getValue(*iter, awardName));
		}
        
        int nRankListFreshPeriod = inifile.getValueT(*iter, "rankListFreshPeriod", DEFAULT_RANKLIST_REFRESHTIME);
        check_min(nRankListFreshPeriod, 60);
        
        
        
        string sServerList = inifile.getValue(*iter, "serverList", "");
        StringTokenizer token(sServerList, ";");
        
        set<int> serverList;
        for(int i = 0; i < token.count(); i++)
        {
            int serverid = Utils::safe_atoi(token[i].c_str());
            assert(allServers.end() == allServers.find(serverid)); //一个 serverid 不能出现在多个 group 里面
            
            serverList.insert(serverid);
            mServer2Group.insert(make_pair(serverid, groupid));
            allServers.insert(serverid);
        }
        
        
        ServerGroupCfgDef* tmp = new ServerGroupCfgDef;
        tmp->mServerIds             = serverList;
        tmp->mGroupId               = groupid;
        tmp->mRankListFreshPeriod   = nRankListFreshPeriod;
        tmp->mBeginTime             = beginTime;
        tmp->mEndTime               = endTime;
        tmp->mAwards                = awards;
        tmp->mRankListType          = rankListType;
        tmp->mSendAwardTime         = sendAwardTime;
        
        mServerGroupCfgMap.insert(make_pair(groupid, tmp));
        mGroupidList.push_back(groupid);
    }
    
    if (allServers.end() != allServers.find(-1)) {
        assert(allServers.size() == 1);                            //有一个全服 group 不能有其他 group
    }
    if (allServers.empty()) {
        return;
    }
    mLoaded = true;
}

ServerGroupCfgDef*
ServerGroupCfg::getServerGroupCfgDefByGroupid(int groupid)
{
    Iterator iter = mServerGroupCfgMap.find(groupid);
    
    if (mServerGroupCfgMap.end() == iter) {
        return NULL;
    }
    
    return iter->second;
}

ServerGroupCfgDef*
ServerGroupCfg::getServerGroupCfgDefByServerid(int serverId)
{
    int groupid = getServerGroupId(serverId);
    
    ServerGroupCfgDef* tmp = getServerGroupCfgDefByGroupid(groupid);
    if (NULL == tmp) {
        return NULL;
    }
    return tmp;
}



int
ServerGroupCfg::getServerGroupId(int serverId)
{
    map<int, int>::iterator iter = mServer2Group.find(serverId);
    if (mServer2Group.end() == iter) {
        iter = mServer2Group.find(-1);
        if (mServer2Group.end() == iter) {
            return  -1;
        }
    }
    
    return iter->second;
}

int
ServerGroupCfg::getGroupFreshPeriod(int serverId)
{
    ServerGroupCfgDef* tmp = getServerGroupCfgDefByServerid(serverId);
    if (tmp == NULL) {
        return -1;
    }
    
    return tmp->mRankListFreshPeriod;
}

void
ServerGroupCfg::getGroupList(vector<int> &output)
{
    output = mGroupidList;
}

ServerGroupCfg g_RechargeRankListGroupCfg;
ServerGroupCfg g_ConsumeRankListGroupCfg;

