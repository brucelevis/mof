//
//  LogPropName.h
//  GameSrv
//
//  Created by jin on 13-12-18.
//
//

#ifndef __GameSrv__LogPropName__
#define __GameSrv__LogPropName__

#include <iostream>

#ifndef LogPropNameBegin
	#define LogPropNameBegin() enum eLogPropName {
#endif

#ifndef LogPropName
	#define LogPropName(id, name, value) name = id,
#endif

#ifndef LogPropNameEnd
	#define LogPropNameEnd() };
#endif

LogPropNameBegin()
#pragma mark - 日志文件名定义


/*
 mysql tb_log表log_name字段对应的id表, 是这里面的枚举值
 注意这个id使用后, 不能随意修改
 
 (对应id值, 枚举名, 文件日志名)
 */

LogPropName(0, eLogName_Non, Non)
LogPropName(1, eLogName_AppleCharge, AppleCharge) // 苹时充值
LogPropName(2, eLogName_PetCasion, PetCasion) // 幻兽大比拼
LogPropName(3, eLogName_Lottery, Lottery) // 抽奖
LogPropName(4, eLogName_PetAbsorb, PetAbsorb) // 吞宠
LogPropName(7, eLogName_HonorLog, HonorLog) // 称号
LogPropName(8, eLogName_HonorAction, HonorAction) // 称号离线动作
LogPropName(9, eLogName_ItemChange, ItemChange) // 物品转换
LogPropName(10, eLogName_PetStudySkill, PetStudySkill) // 宠物学习技能
LogPropName(11, eLogName_PetGrowthRefine, PetGrowthRefine) // 宠物成长洗练
LogPropName(12, eLogName_RoleUp, RoleUp) // 角色升级
LogPropName(13, eLogName_RoleVipUp, RoleVipUp) // vip升级
LogPropName(14, eLogName_EquipPromote, EquipPromote) // 装备提品
LogPropName(15, eLogName_StudySkill, StudySkill) // 学习技能
LogPropName(16, eLogName_Activation, Activation) // 激活码获得日志
LogPropName(17, eLogName_MysteriousBuy, MysteriousBuy) // 神秘商人购买日志
LogPropName(18, eLogName_AnswerReward, AnswerReward) // 答题活动奖励
LogPropName(19, eLogName_AnswerTitle, AnswerTitle) // 答题活动抽题
LogPropName(20, eLogName_DiceLottery, DiceLottery) // 色子抽奖日志
LogPropName(21, eLogName_RandomEvents, RandomEvents) // 活动宝箱日志
LogPropName(22, eLogName_RandomPresent, RandomPresent) // 活动宝箱日志(发实物日志)
LogPropName(23, eLogName_RandomLimitNum, RandomLimitNum) // 活动宝箱日志(实物限制次数上限记录日志)
LogPropName(24, eLogName_BossCheat, BossCheat) // 
LogPropName(25, eLogName_HttpRequest, HttpRequest) // HTTP请求失败
LogPropName(26, eLogName_OperateActLotteryAward, OperateActLotteryAward) // 抽奖返利活动
LogPropName(27, eLogName_PetCasionThief, PetCasionThief) // 幻兽大比拼小偷之手
LogPropName(28, eLogName_PetCasionWager, PetCasionWager) // 幻兽大比拼押注
LogPropName(29, eLogName_HeartBeatAntiCheating, HeartBeatAntiCheating) // 心跳防外挂
LogPropName(30, eLogName_RmbChange, RmbChange) // 金钻变化
LogPropName(31, eLogName_OnLineNumber, OnLineNumber)    // 每分钟在线人数
LogPropName(32, eLogName_GetFat, GetFat)    // 获取精力日志
LogPropName(33, eLogName_ConsumeFat, ConsumeFat)    // 消耗精力日志
LogPropName(34, eLogName_PetNumericConvert, PetNumericConvert)      // 宠物数值转换
LogPropName(35, eLogName_EquipNumericConvert, EquipNumericConvert)  // 装备数值转换
LogPropName(36, eLogName_BagItemToGold, BagItemToGold)  // 背包一些物品转换成金币
LogPropName(37, eLogName_MysteriousExchange, MysteriousExchange)  // 神秘商店　物品兑换
LogPropName(38, eLogName_QueenBless, QueenBless)  // 女神祝福（每天领取精力）
LogPropName(39, eLogName_PetMerge, PetMerge)        // 幻兽合并（升阶）
LogPropName(40, eLogName_UseItem, UseItem)          // 使用物品
LogPropName(41, eLogName_CapsuleToy, CapsuleToy)    //  扭蛋机功能
LogPropName(42, eLogName_UnlockPetSkill, UnlockPetSkill)    // 解锁宠物技能

LogPropName(43, eLogName_PetCasion_TraceBack, PetCasionTraceBack) // 幻兽大比拼
LogPropName(44, eLogName_EggCompensation, EggCompensation)   // 扭蛋改版补偿


LogPropName(100, eLogName_GuildlvlRankActivity, GuildlvlRankActivity) // 进击的公会活动
LogPropName(102, eLogName_GuildNationalDayActivity, GuildNationalDayActivity) // 公会大乱斗活动
LogPropName(103, eLogName_PetlvlRankActivity, PetlvlRankActivity) // 谁是最神兽活动
LogPropName(104, eLogName_RolelvlRankActivity, RolelvlRankActivity) // 稀有幻兽送斗皇活动
LogPropName(105, eLogName_OperateActRechargeAward, OperateActRechargeAward) // 充值返利活动
LogPropName(106, eLogName_OperateActOnceRechargeAward, OperateActOnceRechargeAward) // 一次性充值返利活动
LogPropName(107, eLogName_OperateActAccuRechargeAward, OperateActAccuRechargeAward) // 累计充值返利活动
LogPropName(108, eLogName_OperateActAccuConsumeAward, OperateActAccuConsumeAward) // 累计消费返利活动
LogPropName(109, eLogName_RoleFightPointActivity, RoleFightPointActivity)   // 角色战斗力排行活动
LogPropName(110, eLogName_FamousDungeActivity, FamousDungeActivity)     // 名人堂排行活动
LogPropName(111, eLogName_OperateActMonthRechargeAward, OperateActMonthRechargeAward) // 月卡返利活动
LogPropName(112, eLogName_Invite_Code, InviteCode) // 邀请码
LogPropName(113, eLogName_OperateActMonthOpen, OperateActMonthOpen) // 月卡充值开启日志
LogPropName(114, eLogName_OperateActMonthClose, OperateActMonthClose) // 月卡充值奖励领完了
LogPropName(115, eLogName_OperateActConsumeFatAward, OperateActConsumeFatAward)   // 消耗精力领奖励


LogPropName(200, eLogName_CustomMailLoad, CustomMailLoad)  // 定制邮件加载
LogPropName(201, eLogName_CustomMailClose, CustomMailClose)  // 定制邮件的发送结束记录
LogPropName(202, eLogName_CustomMailSend, CustomMailSend)  // 定制邮件的发送
LogPropName(203, eLogName_ReRoleName, ReRoleName)  // 修改角色名称
LogPropName(204, eLogName_ReGuildName, ReGuildName)  // 修改公会名称
LogPropName(205, eLogName_ShareAward, ShareAward)  // 分享奖励
LogPropName(206, eLogName_GooglePay, GooglePay)  // google交易
LogPropName(207, eLogName_OrderPayment, OrderPayment)  // 订单交易记录日志
LogPropName(208, eLogName_Payment, Payment)  // 充值记录日志
LogPropName(209, eLogName_AppCmd, AppCmd)  // 应用指令日志
LogPropName(210, eLogName_GooglePayLocalInfo, GooglePayLocalInfo)  // google交易本地信息
LogPropName(211, eLogName_TotemImmolation, TotemImmolation)  // 图腾献祭
LogPropName(212, eLogName_TotemNum, TotemNum)  // 图腾升级
LogPropName(213, eLogName_PetAbsorbNew, PetAbsorbNew) // 幻兽吞噬
LogPropName(214, eLogName_SdkLoginLog, SdkLoginLog)    // sdk登陆日志
LogPropName(215, eLogName_AddExpLog, AddExpLog)    // 经验变更
LogPropName(216, eLogName_RolePvpRankActivity_Mail, RolePvpRankActivity_Mail)    // 
LogPropName(217, eLogName_RolePvpRankActivity, RolePvpRankActivity)    //
LogPropName(218, eLogName_WinnerIosPay, WinnerIosPay)    //
LogPropName(219, eLogName_WinnerIosPayRet, WinnerIosPayRet)    //
LogPropName(220, eLogName_WinnerGoogleWalletPay, WinnerGoogleWalletPay)    //
LogPropName(221, eLogName_WinnerGoogleWalletPayRet, WinnerGoogleWalletPayRet)    //
LogPropName(222, eLogName_WinnerFortumoPay, WinnerFortumoPay)    //
LogPropName(223, eLogName_WinnerFortumoPayRet, WinnerFortumoPayRet)    //
LogPropName(224, eLogName_SdkChennelLogin, SdkChennelLogin)    //
LogPropName(225, eLogName_HonorProcLog, HonorProcLog)    // 称号应用日志
LogPropName(226, eLogName_HonorOfflineEffectiveTimeoutLog, HonorOfflineEffectiveTimeoutLog)    // 称号离线处理,　限时称号超时
LogPropName(227, eLogName_WebState, WebState)    // web服务器状态
LogPropName(228, eLogName_IpList, IpList)    // IP列表状态
LogPropName(229, eLogName_IllustrationsAdd, IllustrationsAdd)    // 图鉴日志
LogPropName(230, eLogName_IllustrationsAward, IllustrationsAward)    // 图鉴奖励获取
LogPropName(231, eLogName_OccupyBuy, OccupyBuy)    //　公会占领商店购买
LogPropName(232, eLogName_BattleChange, BattleChange)    //　战斗力变化
LogPropName(233, eLogName_CopyExtraAward, CopyExtraAward)    //　副本额外奖励
LogPropName(234, eLogName_TwistEggGet, TwistEggGet)    //　新扭蛋获得记录日志
LogPropName(235, eLogName_GetMailAttach, GetMailAttach)
LogPropName(236, eLogName_AddPetExp, AddPetExp)
LogPropName(237, eLogName_MidastouchAction, MidastouchAction)
LogPropName(238, eLogName_Midastouch, Midastouch)


LogPropName(300, eLogName_Login, Login)
LogPropName(301, eLogName_Logout, Logout)
LogPropName(302, eLogName_GetGold, GetGold)
LogPropName(303, eLogName_ConsumeGold, ConsumeGold)
LogPropName(304, eLogName_GetRmb, GetRmb)
LogPropName(305, eLogName_ConsumeRmb, ConsumeRmb)
LogPropName(306, eLogName_GetBattlePoint, GetBattlePoint)
LogPropName(307, eLogName_ConsumeBattlePoint, ConsumeBattlePoint)
LogPropName(308, eLogName_Getexp, Getexp)
LogPropName(309, eLogName_GetConstellVal, GetConstellVal)
LogPropName(310, eLogName_RoleUpGradeEquip, RoleUpGradeEquip)
LogPropName(311, eLogName_RoleGetinPvp, RoleGetinPvp)
LogPropName(312, eLogName_GetVipexp, GetVipexp)
LogPropName(313, eLogName_GetItems, GetItems)
LogPropName(314, eLogName_ComposeItems, ComposeItems)
LogPropName(315, eLogName_GetPet, GetPet)
LogPropName(316, eLogName_CreateRole, CreateRole)
LogPropName(317, eLogName_Moneytreeop, Moneytreeop)
LogPropName(318, eLogName_Loginaward, Loginaward)
LogPropName(319, eLogName_BuyFat, BuyFat)
LogPropName(320, eLogName_Vipaward, Vipaward)
LogPropName(321, eLogName_GetEnchantDust, GetEnchantDust)
LogPropName(322, eLogName_GetEnchantGold, GetEnchantGold)
LogPropName(323, eLogName_GetEnchantId, GetEnchantId)
LogPropName(324, eLogName_GetEnchantLvl, GetEnchantLvl)
LogPropName(325, eLogName_DailyScheduleGetAward, GetDailyScheduleAward)
LogPropName(326, eLogName_DailyScheduleUpdate,   DailyScheduleUpdate)
LogPropName(327, eLogName_DailyScheduleComplete, DailyScheduleComplete)

LogPropName(400, eLogName_ChangeSpeed, ChangeSpeed)


LogPropName(1000, eLogName_GuildOpertion, GuildOpertion) // 公会操作
LogPropName(1001, eLogName_GuildDonate, GuildDonate) // 公会捐献
LogPropName(1002, eLogName_GuildCommon, GuildCommon) // 公会普通日志
LogPropName(1003, eLogName_GuildConstribute, GuildConstribute) // 公会个人贡献日志
LogPropName(1004, eLogName_GuildExploit, GuildExploit) // 公会个人功勋日志
LogPropName(1005, eLogName_GuildFortCons, GuildFortCons) // 公会财富和建设度日志
LogPropName(1006, eLogName_GuildUpgradeSkill, GuildUpgradeSkill) // 公会升级技能日志
LogPropName(1007, eLogName_GuildCreateAndDissmiss, GuildCreateAndDissmiss) // 公会创建与解散
LogPropName(1008, eLogName_GuildJoinAndLeave, GuildJoinAndLeave) // 公会加入与离开
LogPropName(1009, eLogName_GuildBossTrain, GuildBossTrain) // 公会boss培养
LogPropName(1010, eLogName_GuildBossActivity, GuildBossActivity) // 公会boss活动
LogPropName(1011, eLogName_GuildBossAward, GuildBossAward) // 公会BOSS奖励
LogPropName(1012, eLogName_WorldBoss, WorldBoss) // 世界boss
LogPropName(1013, eLogName_GuildTrainAct, GuildTrainAct) // 公公试练活动
LogPropName(1014, eLogName_GuildTrainActAward, GuildTrainActAward) // 公公试练活动发奖
LogPropName(1015, eLogName_GuildBless, GuildBless) // 公会祝福
LogPropName(1016, eLogName_GuildInspire, GuildInspire) // 公会鼓舞
LogPropName(1017, eLogName_BossHpLog, BossHpLog) // boss血记录
LogPropName(1018, eLogName_GuildDonateBugFixLog, GuildDonateBugFix)// 公会捐修复日志
LogPropName(1019, eLogName_GuildImpeachLog, GuildImpeach)// 公会会长弹劾
LogPropName(1020, eLogName_GuildSkillIllegal, GuildSkillIllegal)  // 公会技能升级,　恶意修改日志
LogPropName(1021, eLogName_AddFashion, AddFashion)      //使用时装
LogPropName(1022, eLogName_GuildAppointPosition, GuildAppointPosition)  // 公会任职
LogPropName(1023, eLogName_TreasureFightPlayer, TreasureFightPlayer)    //　公会夺宝战参与的角色
LogPropName(1024, eLogName_OperateActAccuRechargeAwardMail, OperateActAccuRechargeAwardMail)    //

LogPropName(2000, eLogName_PetCampPassRecord, PetCampRecord) //幻兽大本营通关日志
LogPropName(2001, eLogName_CompatPowerError, CompatPowerError) //战斗力作弊
LogPropName(2002, eLogName_EnterCopy, EnterCopy) //进入副本
LogPropName(2003, eLogName_FinishCopy, FinishCopy) //完成副本
LogPropName(2004, eLogName_SendRoleMail, SendMail) //发邮件记录
LogPropName(2005, eLogNmae_FinishDungeException, FinishDungeException) //地下城和变异精英异常通关记录
LogPropName(2006, eLogName_TowerDefense_Pass_Condition, TowerDefense)   //守卫神像通关情况
LogPropName(2007, eLogName_SynPvpMedalChange, SynpvpMedal)              //同步竞技场勋章变化
LogPropName(2008, eLogName_TimeLimitKillCopy, TimeLimitKillCopy)        //显示击杀副本的日志
LogPropName(2009, eLogName_TowerDefenseAward, TowerDefenseAward)        //守卫神像奖励情况
LogPropName(2010, eLogName_FamesHallAward, FamesHallAward)              //名人堂奖励
LogPropName(2011, eLogName_FinishFriendDunge, FinishFriendDunge)        //好友地下城通关
LogPropName(2012, eLogName_FinishDunge, FinishDunge)                    //普通地下城
LogPropName(2013, eLogName_FinishEliteCopy, FinishEliteCopy)            //精英副本
LogPropName(2014, eLogName_FinishPetElite, FinishPetElite)              //幻兽试炼场
LogPropName(2015, eLogName_SkillHurtRecord, SkillHurtRecord)            //记录技能伤害
LogPropName(2016, eLogName_PetEvolution, PetEvolution)                  //幻兽进化
LogPropName(2017, eLogName_PetAdventureCopy, PetAdventureCopy)      //幻兽大冒险副本
LogPropName(2018, eLogName_PetAdventureBufOnPet, PetAdventureBufOnPet)      //幻兽大冒险幻兽属性加成
LogPropName(2019, eLogName_PetAdventureCopyBuyTimes, PetAdventureCopyBuyTimes)      //幻兽大冒险副本购买次数
LogPropName(2020, eLogName_DungeonSweeping, DungeonSweeping)      //地下城扫荡
LogPropName(2021, eLogName_MysticalCopyTimesAndIncrTimes, MysticalCopyTimesAndIncrTimes) //幻兽大本营完成副本消耗品与副本次数改变日志


LogPropName(2022, eLogName_FashionCollectFinish, FashionCollectFinish)                  //
LogPropName(2023, eLogName_FashionCollectPutOnMaterial, FashionCollectPutOnMaterial)    //
LogPropName(2024, eLogName_FashionCollectMaterialRecycle, FashionCollectMaterialRecycle)//
LogPropName(2025, eLogName_DelectPetFromDB, DelectPetFromDB)
LogPropName(2026, eLogName_PetAdventureRobotPetRentTimes, PetAdventureRobotPetRentTimes)      //幻兽大冒险机器幻兽租用次数
LogPropName(2027, eLogName_DuokuSdkLogin, DuokuSdkLogin)      //百度多酷平台登录
LogPropName(2028, eLogName_SwallowLockOperate, SwallowLockOperate)      //幻兽吞噬锁开关操作记录

LogPropName(2029, eLogName_PetBatlleForcePaihang, PetBattleForceSort)       //幻兽战斗力排行

LogPropName(2030, eLogName_RoleBatlleForcePaihang, RoleBattleForceSort)       //角色战斗力排行
LogPropName(2031, eLogName_RolePvpPaihang, RolePvpSort)       //角色竞技场排行
LogPropName(2032, eLogName_RoleLvlPaihang, RoleLvlSort)       //角色等级排行
LogPropName(2033, eLogName_RoleRechargePaihang, RoleRechargeSort)       //角色充值排行
LogPropName(2034, eLogName_RoleConsumePaihang, RoleConsumeSort)       //角色消费排行



LogPropName(2100, eLogName_BattleDataCollect, BattleDataCollect)              //战斗数据采集

LogPropName(2200, eLogName_WeaponEnchantErr, WeaponEnchantErr)          // 武器附魔异常
LogPropName(2201, eLogName_WeaponEnchantUpgrade, WeaponEnchantUpgrade)  // 武器附魔升级

LogPropName(2202, eLogName_EquipBaptize, EquipBaptize)              //装备洗练

LogPropName(2203, eLogName_RetinueSummon, RetinueSummon)
LogPropName(2204, eLogName_RetinueCompose, RetinueCompose)
LogPropName(2205, eLogName_RetinueAddExp, RetinueAddExp)
LogPropName(2206, eLogName_RetinueStarup, RetinueStarup)

LogPropName(2207, eLogName_RetinueSkillStudy, RetinueSkillStudy)

LogPropName(2208, eLogName_PvpPointExchange, PvpPointExchange)
LogPropName(2209, eLogName_PvpPointChange, PvpPointChange)
LogPropName(2210, eLogName_Worship, Worship)

LogPropName(3100, eLogName_GmCommand, GmCommand)
LogPropName(3101, eLogName_DupNameProblem, DupNameProblem)

LogPropName(3102, eLogName_CrossServiceBattleRankListAwards,    CrossServiceBattleRankListAwards)
LogPropName(3103, eLogName_CrossServicePetRankListAwards,       CrossServicePetRankListAwards)
LogPropName(3104, eLogName_CrossServiceConsumeRankListAwards,   CrossServiceConsumeRankListAwards)
LogPropName(3105, eLogName_CrossServiceRechargeRankListAwards,  CrossServiceRechargeRankListAwards)



#pragma mark -
LogPropNameEnd()

/*
 日志动作类型枚举
 (注意每个动作后面都需赋值)
 */
enum eLogAction
{
	eLogAction_Non = 0, // 无
    eLogAction_Test = 1, // 测试
};

extern const char* GetLogPropName(int eName);

#endif /* defined(__GameSrv__LogPropName__) */
