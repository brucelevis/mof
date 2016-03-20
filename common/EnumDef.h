#pragma once

#include "define_helper.h"

enum VER_VALIDATE_RESULT
{
	ver_ok = 1,
	ver_fail = 2
};

const int PROTO_VER = 1;

enum RankChangeType
{
    eNonChange,
    eRankUp,
    eRankDown,
};

enum DungCityLevel
{
    eDungLvl_one = 1,
    eDungLvl_two,
    eDungLvl_three,
};

enum DungeResult
{
    eDungeNext,
    eDungeFail,
    eDungeFinish,
    eDungGetout,
};

enum SortType
{
    eSortBegin = 0,
    eSortBat = 1,
    eSortLvl,
    eSortPet = 3,
    eSortPvp,
    eSortRecharge = 5,
    eSortConsume = 6,
    eSortIndex = 7,
    eSortServerid = 8,
    eSortRank,
    eSortEnd,
};
enum MailType
{
    eSysMail = 1,
    eMyMail,
};

enum ChatContentType{
    kChatContentTypeNone = -1,
    kChatContentTypeText = 0,          //文字聊天
    kChatContentTypeVoice              //语音聊天
};

enum CheatType
{
    CHEAT_NOT = 0,
    
    CHEAT_YOU_ARENOT_IN_THE_TOWN_OF_THIS_COPY,
    CHEAT_YOU_ARENOT_IN_THIS_COPY,
    CHEAT_ATK_ERROR,
    CHEAT_DEF_ERROR,
    CHEAT_DODGE_ERROR = 5,
    
    CHEAT_CRI_ERROR,
    CHEAT_HP_ERROR,
    CHEAT_HIT_ERROR,
    CHEAT_ERROR_COPYID,
    CHEAT_ERROR_BATTLE = 10,
    
    CHEAT_PETBATTLE_ERROR,
    CHEAT_ENEMY_MAXHP_ERROR,
    CHEAT_ENEMY_ATK_ERROR,
    CHEAT_ENEMY_DEF_ERROR,
    CHEAT_ENEMY_CRI_ERROR = 15,
    
    CHEAT_ENEMY_DODGE_ERROR,
    CHEAT_ENEMY_HIT_ERROR,
    CHEAT_ENEMY_PET_BATTLE_ERROR,
    CHEAT_FRIEND_ATK_ERROR,
    CHEAT_FRIEND_DEF_ERROR = 20,
    
    CHEAT_FRIEND_DODGE_ERROR,
    CHEAT_FRIEND_CRI_ERROR,
    CHEAT_FRIEND_HP_ERROR,
    CHEAT_FRIEND_HIT_ERROR,
    CHEAT_FRIEND_PETBATTLE_ERROR = 25,
    
    CHEAT_MODIFY_MEMORY,
    CHEAT_MODIFY_CONFIG,
};
enum GlobalErrorCode
{
    CE_UNKNOWN = -1,
    CE_OK = 0,
    //login  1 ~ 10
    CE_ACCOUNT_UNAVAILABLE = 1,
    CE_ACCOUNT_NOT_EXIST,
    CE_PASSWORD_ERROR,
    CE_GET_ROLE_SAVE_ERROR,
    CE_CREATE_ROLE_ERROR,
    CE_SELECT_ROLE_ERROR,
    CE_DELETE_ROLE_ERROR,
    CE_ROLE_FULL,
    
    //other 11 ~ 30
    CE_GET_STUDYABLE_SKILLS_ERROR = 11,
    CE_STUDY_SKILL_ERROR,
    CE_GET_EQUIP_SKILLS_ERROR,
    CE_SHOP_RMB_NOT_ENOUGH,
    CE_SHOP_GOLD_NOT_ENOUGH = 15,
    
    CE_FAT_NOT_ENOUGH = 16,
    CE_BAG_FULL,
    CE_YOUR_LVL_TOO_LOW,
    CE_CAN_NOT_FIND_CFG,
    CE_YOU_SEND_WORLD_MSG_TOO_OFTEN,
    
    CE_YOUR_ENTER_TIMES_USEUP = 21, //你的进入次数已满
    CE_CAGE_IS_FULL,            //你的兽笼已满
    CE_READ_CFG_ERROR,          //读配置表错误
    CE_YOU_HAVE_NOT_STUDY_THIS_SKILL,
    CE_YOU_EQUIP_SKILL_POSITION_NOT_RIGHT,
    
    CE_BAG_FULL_AND_SEND_MAIL = 26,
    CE_EQUIP_FUSION_IS_MAX,
    CE_QUEST_NOT_FINISH,
    CE_BAG_CAPACITY_LESS_THAN_N_CELL,   //背包格子少于10格
    CE_USE_DUNGEITEM_ERROR,              //使用地下城扫荡物品，但是玩家还没有打地下城。
    
    ///pvp 31 ~ 40
    CE_PVP_GET_RANK_ROLES_ERROR = 31,
    CE_PVP_SLECTED_ROLE_IS_IN_PVP,
    CE_PVP_GET_ROLE_DATA_ERROR,
    CE_PVP_YOU_HAD_GETAWARD_TODAY,
    CE_PVP_YOU_HAVE_NO_TIMES_TO_PVP,
    CE_PVP_ROLE_IS_NOT_REAL_RANK,
    CE_PVP_YOU_ARE_IN_COOLING_DOWN,
    CE_PVP_ACTIVITY_TIMEOUT,
    
    
    //teamcopy 41 ~ 50
    CE_TEAMCOPY_YOU_HAD_FINISH_THISCOPY = 41,
    CE_TEAMCOPY_YOU_HAD_EMPLOYED_THIS_FRIEND,
    CE_GET_FRIEND_DATA_ERROR,
    CE_ELITETIMES_USEUP,
    CE_THE_TARGETS_FRIENDCOUNT_IS_FULL,
    CE_YOUR_FRIENDCOUNT_IS_FULL,
    
    //vip 51 ~ 60
    CE_READ_VIPCFG_ERROR = 51,
    CE_YOUR_VIPLVL_IS_LOWER,
    CE_YOU_ARE_NOT_VIP,
    CE_YOU_HAVE_GET_THIS_VIPLVL_AWARD,
    CE_MONEYTREE_RMB_OPEN_TIMES_USEUP,//33
    

    //copy 61 ~ 80
    CE_SCENE_TYPE_ERROR = 61,
    CE_YOUR_BATTLEFORCE_IS_TOO_LOW,
    CE_COPY_NOT_PASS,
    CE_READ_SCENE_CFG_ERROR,
    CE_YOU_HAVE_NOT_PASS_DUNGE_LVL,
    
    CE_TARGET_IS_DISONLINE = 66,                //目标不在线
    
    //幻兽大冒险
    CE_PET_ADVENTURE_TIMES_EXHAUSTED = 67,          //副本次数用完
    CE_PET_ADVENTURE_CONFIG_NOT_FOUND,              // 在petadventurecopy.ini 找不到对应副本
    CE_PET_ADVENTURE_SCENE_NOT_FOUND,               //在 scene.ini 找不到对应副本
    CE_PET_ADVENTURE_PLAYERLVL_OUT_RANGE,           //玩家等级不在副本限制等级范围内
    CE_PET_ADVENTURE_FIGHTPOINT_INSUFFICIENT,       //玩家战斗力不足
    CE_PET_ADVENTURE_TEMPORARY_ACTIVITY_CLOSE,      //临时活动副本时间结束已经关闭
    CE_PET_ADVENTURE_WRONG_DEADED_MONSTER_NUM,      //怪物死亡数量不对
    CE_PET_ADVENTURE_WRONG_CARRYPETID,              //携带的幻兽不对
    CE_PET_ADVENTURE_NOT_ENOUGH_GOLD_FOR_RENTPET,   //租用幻兽金钻不足
    CE_PET_ADVENTURE_RENTPET_TIMES_EXHAUSTED,       //租用幻兽次数耗尽
    CE_PET_ADVENTURE_CREATE_ROBOTPET_ERROR,         //租用幻兽的创建出错

    

    //地下城扫荡
    CE_DUNGEON_SWEEPING_MAXSWEEPLVL_PASSED,
    CE_DUNGEON_SWEEPING_PLAYER_FIGHTPOINT_INSUFFICIENT,
    
    //shop 81 ~ 90
    CE_READ_BUY_CFG_ERROR = 81,
    CE_BUY_TIMES_USEUP,//18
    
    //active 91 ~ 100
    CE_ACTIVE_IS_NOT_OPEN = 91,
    CE_YOUR_ACTIVITION_TIMES_USEUP,
    CE_CREATE_ACTIVITION_ROOM_FAIL,
    CE_CAN_NOT_FIND_ACTIVITION_ROOM,
    CE_MYSTICALCOY_LIST_HAD_CHANGE,
    
    CE_YOU_HAVE_IN_THIS_SCENE = 96,
    CE_ACTIVITY_IS_OPENNING,
    
    //petpvp 101~110
    CE_CAN_NOT_FIND_PLAYER_DATA = 101,
    CE_FIND_NO_ENEMY = 102,
    CE_YOUR_FORMATION_IS_EMPTY = 103,
    CE_YOUR_POINTS_NOT_ENOUGH,
    CE_PVP_PET_ERROR = 105,
    CE_PETPVP_TIMES_UP = 106,
    
    //famesHall 111 ~ 120
    CE_FAMEHALL_YOU_HAVE_PASS_THIS_LAYER = 111,
    CE_FAMEHALL_THIS_CHECKLAYER_IS_NOT_RIGHT = 112,
    CE_FAMEHALL_YOU_HAVE_PASS_ALLLAYERS,
    
    //130
    CE_ITEM_NOT_ENOUGH = 130,   //物品不足或没有
    CE_GUILD_NOT_EXIST = 131,   //公会不存在
    CE_GUILD_NAME_EXIST = 132,  //公会的名称已存在
    CE_ROLE_NAME_EXIST = 133,   //角色的名字已存在
    CE_RENAME_CARD_ERROR = 134, //改名卡错误
    CE_RENAME_SYSTEM_ERROR = 135,
    CE_RENAME_ILLEGAL_CHAR_ERROR = 136,  // 用改名卡改名时，存在非法字符
    CE_GUILD_LEVEL_TOO_LOW = 137,  // 公会等级不足
    
    //140 幻兽试炼场
    CE_PET_ELITE_ALREADY_USED = 140,  // 宠用过了
    CE_PET_ELITE_PET_ID_ERROR = 141,  // 没有这个宠
    
    //150 创建角色
    CE_INVALID_ROLE_NAME = 150,
    CE_INVALID_NAME = 151,
    
    //160
    CE_ITEM_EXPIRED = 160,
    
    //实时PVP 170 ~ 180
    CE_REALPVP_ENTER_MATCHING_ERROR = 170,
    CE_REALPVP_EXCHANGE_MEDAL_NOT_ENOUGH,
    CE_REALPVP_CAN_NOT_FIND_THIS_ITEM,
    CE_DUEL_TARGET_DISONLINE,
    CE_DUEL_TARGET_NOT_IN_TOWN,
    
    CE_DUEL_INVITER_HAD_CANCEL_DUEL = 175,
    CE_DUEL_TARGET_IS_IN_DUEL,
    CE_DUEL_TARGET_LVL_IS_TOO_LOW,
    CE_REALPVP_EXCHANGE_HONOR_NOT_ENOUGH,
    
    //公会宝藏战181~189
    CE_TREASURECOPY_PLAYER_FULL = 181,
    CE_TREASURECOPY_GUILD_MEM_FULL = 182,
    CE_TREASURECOPY_COPY_NOT_OPEN = 183,
    CE_TREASURECOPY_COPY_NOT_EXIST = 184,
    CE_TREASURECOPY_YOU_ARE_NOT_RECOVER = 185,              //苏醒时间还没到
    
    CE_TREASURECOPY_YOU_NOT_TAKE_PARTIN_ACT = 186,          //你没有餐加活动
    CE_TREASURECOPY_YOU_ARE_INCOPY_NOW = 187,
    CE_TREASURECOPY_YOU_CAN_NOT_GET_MANOR_AWARD = 188,      //你已经领过奖励或不可领
    
    //190~200
    CE_ROLE_LVL_TOO_LOW = 190,
    CE_CANT_USE_THIS_ITEM = 191,
    CE_USE_FASHION_ERROR = 192,
    CE_PUTON_FASHION_ERROR = 193,
    CE_FASHION_EXPIRED = 194,
    CE_FASHION_TYPE_NOT_EXIST = 195,
    
    
    //201~230
    CE_SYNC_TEAM_YOU_HAVE_IN_A_TEAM = 201,
    CE_SYNC_TEAM_CREATE_TEAM_ERROR = 202,
    CE_SYNC_TEAM_CAN_NOT_FIND_THIS_TEAM = 203,
    CE_SYNC_TEAM_MEMBER_OF_THIS_TEAM_FULL = 204,
    CE_SYNC_TEAM_YOU_ARE_NOT_IN_A_TEAM = 205,
    CE_SYNC_TEAM_YOU_ARE_NOT_CAPTAIN_OF_THIS_TEAM = 206,
    CE_SYNC_TEAM_YOU_ARE_THE_CAPTAIN = 207,
    CE_SYNC_TEAM_SOME_ONE_OF_THIS_TEAM_ARE_NOT_READY = 208,
    CE_SYNC_TEAM_SET_READY_ERROR = 209,
    CE_SYNC_TEAM_CANCEL_READY_ERROR = 210,
    CE_SYNC_TEAM_TEAM_MATE_LVL_IS_TOO_LOW = 211,              //更换副本时，队员的等级太低
    CE_SYNC_TEAM_TARGET_SCENEMOD_IS_INVALID = 212,
    
    CE_SYNC_TEAM_TARGET_INVITED_IS_IN_A_TEAM = 213,
    CE_SYNC_TEAM_INVITATION_TARGET_TEAM_MAYBE_IN_FIGHTING = 214,
    CE_SYNC_TEAM_KICKOUT_ERROR = 215,
    CE_SYNC_TEAM_SOME_ONE_OF_THIS_TEAM_ARE_IN_COPY = 216,
    CE_SYNC_TEAM_INVITATION_TARGET_TEAM_MAYBE_DISMISS = 217,
    
    //231~250
    CE_PET_EVOLUTION_MATERIAL_ERROR = 231,
    CE_PET_EVOLUTION_MATERIAL_STARLVL_TOO_LOW = 232,
    CE_PET_EVOLUTION_MATERIAL_LVL_TOO_LOW,
    CE_PET_EVOLUTION_MATERIAL_SKILLLVL_TOO_LOW,
    CE_PET_EVOLUTION_MATERIAL_PROP_TOO_LOW,
    
    CE_PET_EVOLUTION_MATERIAL_STAGE_TOO_LOW = 236,
    CE_PET_EVOLUTION_CREATE_TARGET_PET_ERROR,
    
    CE_HAVE_GOT_AWARD,
    CE_POINT_NOT_ENOUGH,
    

    //300~350 时装收集
    CE_FASHIONCOLLECT_READ_CFG_ERROR = 300,
    CE_FASHIONCOLLECT_MATERIAL_NOT_ENOUGH = 301,
    CE_FASHIONCOLLECT_MATERIAL_GRID_ERROR = 302,
    CE_FASHIONCOLLECT_MATERIAL_ERROR_FOR_THIS_GRID = 303,
    CE_FASHIONCOLLECT_NOT_FIND_THIS_PROGRESS = 304,
    
    CE_FASHIONCOLLECT_MATERIAL_FOR_THIS_PROGRES_WITHOUT_COMLETION = 305, //收集的材料不完整
    CE_FASHIONCOLLECT_MATERIAL_RECYCLE_SYSTEM_LVL_FULL = 306,
    CE_FASHIONCOLLECT_MATERIAL_NOT_EXIST = 307,
    
    //语音聊天 351 ~ 360
    CE_VOICECHAT_NOT_FIND_VOICEDATA = 351,
    CE_PRIVATE_FRIEND_CHAT_DATA_OVER = 352,
    
    // 角色奖励 361 ~ 365
    CE_ROLE_AWARD_NOT_EXIST = 361,      //不存在该奖励
    CE_ROLE_AWARD_NOT_MATCH,            //不符合领取条件
    
    //幻兽吞噬锁 366 ~ 370
    CE_PET_NOT_EXIST,          //玩家没有这个幻兽
    
    //觉醒 371 ~ 380
    CE_ROLE_AWAKE_SKILLREPLACE_ERROR = 371,
    CE_ROLE_AWAKE_FULL_LVL = 372,
    CE_ROLE_AWAKE_NEED_ITEM_NOT_ENOUGH = 373,
    
    //装备强化石强化装备错误码381 ~ 390
    CE_UPGRADEEQUIP_INTENSIFYSTONE_NOT_ENOUGH = 381,
    CE_UPGRADEEQUIP_INTENSIFYSTONE_ID_ERROR = 382,
    
    //竞技场积分兑换 391 ~ 340
    CE_PVP_EXCHANGE_POINTS_NOT_ENOUGH = 391,
    
    //侍魂相关 341 ~ 370
    CE_RETINUE_NOT_EXIST = 341,                     //侍魂不存在
    CE_RETINUE_MATERIAL_NOT_ENOUGH = 342,           //材料不足
    CE_RETINUE_MATERIAL_NOT_RIGHT = 343,            //材料不对
    CE_RETINUE_IS_MAXLVL = 344,                     //达到最大等级
    CE_RETINUE_IS_MAXSTAR =345,                     //达到最大星级
    
    CE_RETINUE_MOD_NOT_EXIST = 346,                 //侍魂模板配置不存在
    CE_RETINUE_COMPOSE_FAILD = 347,                 //合成失败
    CE_RETINUE_SKILL_CFG_NOT_EXIST = 348,           //侍魂技能配置不存在
    CE_RETINUE_SKILL_IS_MAXLVL = 349,               //技能达到最大等级
    CE_SPIRITSPEAK_CFG_NOT_EXIST = 350,             //通灵配置不存在
    
    CE_SPIRITSPEAK_POSITION_NOT_RIGHT = 351,        //通灵位置不对
    CE_SPIRITSPEAK_PARAM_NOT_RIGHT = 352,           //通灵参数不对
    CE_SUMMON_CFG_NOT_EXIST = 353,                  //聚魂配置不存在
    CE_SUMMON_COOLING_DOWN = 354,                   //处于冷却中
    CE_SUMMON_RESETTIME_USEUP = 355,                //消除冷却次数已用完
    
    CE_SPIRITSPEAK_RETINUE_BEING_USED = 356,
    CE_SUMMON_NOT_COOLING_DOWN = 357,
    
    //洗练错误码 371 ~ 375
    CE_EQUIP_BAPTIZE_FAILD = 371,                   //洗练失败
    CE_EQUIP_BAPTIZE_BACKUPPROP_NOT_EXIST = 372,    //备份属性不存在

    
/*--------------------------------以下留用, 别瞎jb写-------------------------------------*/
    
    //同步场景错误
    CE_REQ_MOTION_STATE_ERROR = 1000,
    CE_REQ_STOP_STATE_ERROR,
    CE_QUICK_ENTER_WORLD_SCENE_FAIL,
    CE_INVALID_OP_REQUEST,
    CE_INVALID_DEST_COORD,
    
    //通用错误
    CE_REQ_PARAM_ERROR = 2000,
    CE_SYSTEM_ERROR = 2001,
/*------------------------------ 枚举到这里结束，不要添加在下面-----------------------------*/
};

//大比拼恢复时间所处的状态
enum PetRecorveState
{
    NO_START = 0,
    NO_ROUND_1,
    NO_ROUND_2,
    NO_ROUND_3,
    NO_END,
    READY_END,
    //活动恢复过程中，当前比赛已经结束的情况
    RECORVESTATEERROR ,
    MATCHERROR ,
};


//配制的参赛宠信息
enum PetConfigurePetMatch
{
    //参赛宠的主人id
    CONFIGUREPETOWNEID = 0,
    //参赛宠的成长值
    CONFIGUREGROUTH,
    //参赛宠的排行
    CONFIGURERANK,
    //参赛宠的起始id
    VIRTUALPETMATCHID = 999999999,
};

// 武器附魔系统
enum WeaponEnchantErrorCode
{
    CE_ENCHANT_UNKNOWN = -1,
    CE_ENCHANT_OK = 0,
    CE_ENCHANT_DUST_LACK = 1,  // 附魔尘不足
    CE_ENCHANT_GOLD_LACK = 2,  // 附魔钻不足
    CE_ENCHANT_FULL = 3,       // 所有等级已满
    CE_ENCHANT_CUR_FULL = 4,	// 当前的附魔项满级了
};
//
//int getDailyScheduleTypeByName("fsdfas")
//const char* getNameByDailyScheduleType(type)
//int getDailyScheduleCount();

/*
  1: 地下城：通关1层
  2: 公会首捐
  3: 答题（20个不分对错）
  4: 神秘商店：购买任意物品或者指定物品一次
  5: 日常任务：完成10个（一轮）
  6: 好友副本：通关3次副本
  7: 幻兽大本营：参加5次
  8: 精英副本：通关3次
  9: 世界BOSS：参加1次
 10: 扭蛋：开任意10次
 11: 幻兽大比拼：下注1次
 12: 幻兽大冒险：完成3次
 13: 一到五.神像保卫战：参加1次
 14: 周二四.变异精英：通关1次
 15: 周六周日.好友地下城：通关任意层
 16: 公会试炼：守过第一波
 17: 个人竞技场：胜利一次
 18: 30级解锁组队副本：通关1次
 19: 28解锁幻兽试炼：通关1次
 20: 30级解锁名人堂：≥五层
 21: 公会二级解锁：公会BOSS参加一次
 22: 26级解锁幻兽竞技场：胜利一次
 23: 公会夺宝战：获得得分
 24: 匹配战：胜利1次
 */

BeginNamedEnum(DailyScheduleType)
    NamedEnumMember(kDailyScheduleDungeon, dungeon)
    NamedEnumMember(kDailyScheduleGuildDonate, guildDonate)
    NamedEnumMember(kDailyScheduleAnswer, answer)
    NamedEnumMember(kDailyScheduleMysteriousShop, mysteriousShop)
    NamedEnumMember(kDailyScheduleDailyQuest, dailyQuest)
    NamedEnumMember(kDailyScheduleTeamCopy, teamCopy)
    NamedEnumMember(kDailyScheduleMysteriousCopy, mysteriousCopy)
    NamedEnumMember(kDailyScheduleEliteCopy, eliteCopy)
    NamedEnumMember(kDailyScheduleWorldBoss, worldBoss)
    NamedEnumMember(kDailyScheduleCapsuleToy, capsuleToy)
    NamedEnumMember(kDailySchedulePetMatch, petMatch)
    NamedEnumMember(kDailySchedulePetAdventure, petAdventure)
    NamedEnumMember(kDailyScheduleDefendStatue, defendStatue)
    NamedEnumMember(kDailySchedulePrintCopy, printCopy)
    NamedEnumMember(kDailyScheduleFriendDungeon, friendDungeon)
    NamedEnumMember(kDailyScheduleGuildChallenge, guildChallenge)
    NamedEnumMember(kDailySchedulePvp, pvp)
    NamedEnumMember(kDailyScheduleSyncTeamCopy, syncTeamCopy)
    NamedEnumMember(kDailySchedulePetElite, petElite)
    NamedEnumMember(kDailyScheduleFamousHall, famousHall)
    NamedEnumMember(kDailyScheduleGuildBoss, guildBoss)
    NamedEnumMember(kDailySchedulePetArena, petArena)
    NamedEnumMember(kDailyScheduleSyncTreasureFight, syncTreasureFight)
    NamedEnumMember(kDailyScheduleSyncPvp, syncPvp)
EndNamedEnum(DailyScheduleType)


BeginNamedEnum(TargetType)
    NamedEnumMember(eTargetType_Recharge, recharge)
    NamedEnumMember(eTargetType_ComposeLvlStone, composeLvlStone)
EndNamedEnum(TargetType)


BeginNamedEnum(QuestTargetType)
    NamedEnumMember(eQuestTargetType_EnterPetCamp,           enterpetcamp)
    NamedEnumMember(eQuestTargetType_EnterCopy,              entercopy)
    NamedEnumMember(eQuestTargetType_ConstellUpg,            constellupg)
    NamedEnumMember(eQuestTargetType_ArenaBattle,            arenabattle)
    NamedEnumMember(eQuestTargetType_FinishFamouseHall,      finishfamoushall)
    NamedEnumMember(eQuestTargetType_FinishPetEliteCopy,     finishpetelitecopy)
    NamedEnumMember(eQuestTargetType_PetEvolution,           petevolution)
    NamedEnumMember(eQuestTargetType_FashionCollectMaterial, fashioncollectmaterial)
    NamedEnumMember(eQuestTargetType_UpgEquip,               upequip)
    NamedEnumMember(eQuestTargetType_FinishCopy,             finishcopy)
    NamedEnumMember(eQuestTargetType_FinishEliteCopy,        finishelitecopy)
    NamedEnumMember(eQuestTargetType_StudySkill,             studyskill)
    NamedEnumMember(eQuestTargetType_StudyPassiveSkill,      studypassiveskill)
    NamedEnumMember(eQuestTargetType_Dialog,                 dialog)
    NamedEnumMember(eQuestTargetType_CopyCombo,              copycombo)
    NamedEnumMember(eQuestTargetType_CopyStar,               copystar)
    NamedEnumMember(eQuestTargetType_FinishPetAdventure,     finishpetadventure)
    NamedEnumMember(eQuestTargetType_EnterFriendCopy,        enterfriendcopy)
EndNamedEnum(QuestTargetType)
