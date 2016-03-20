//
//  DBRolePropName.h
//  GameSrv
//
//  Created by 志坚 on 13-3-13.
//  Copyright (c) 2013年 __MyCompanyName__. All rights reserved.
//
//  人物数据库字段名宏定义，推荐使用宏来取数据库字段名
//
//  如：定义了PropTypeToValue(eRolePropSceneType, scenetypeid)后
//  函数GetRolePropName(eRolePropSceneType)返回字符串”scenetypeid“

#ifndef GameSrv_DBRolePropName_h
#define GameSrv_DBRolePropName_h

#ifndef PropTypeToValueBegin
#define PropTypeToValueBegin() enum DBRolePropType {
#endif

#ifndef PropTypeToValue
#define PropTypeToValue(name, value)  name,
#endif

#ifndef PropTypeToValueEnd
#define PropTypeToValueEnd()  eRolePropCount, };
#endif

PropTypeToValueBegin()
//
PropTypeToValue(eRolePropSceneType, scenetypeid)
PropTypeToValue(eRolePropRoleType, roletype)
PropTypeToValue(eRolePropRoleName, rolename)
PropTypeToValue(eRolePropX, x)
PropTypeToValue(eRolePropY, y)
PropTypeToValue(eRolePropLvl, lvl)
PropTypeToValue(eRolePropHp, hp)
PropTypeToValue(eRolePropMp, mp)
PropTypeToValue(eRolePropGold, gold)
PropTypeToValue(eRolePropBattlePoint, battlePoint)
PropTypeToValue(eRolePropRmb, rmb)
PropTypeToValue(eRolePropExp, exp)
PropTypeToValue(eRolePropFat, fat)
PropTypeToValue(eRolePropConstellVal, constellval)
PropTypeToValue(eRoleLastFinishCopy, lastFinishCopy)
PropTypeToValue(eRoleDungTimes, dungtimes)
PropTypeToValue(eRoleEliteTimes, elitetimes)
PropTypeToValue(eRoleEliteBuyTimes, elitebuytimes)
PropTypeToValue(eRolefatFreshTime, fatFreshTime)
PropTypeToValue(eRolefatBuyTimes, fatbuytimes)

PropTypeToValue(eRoleLoginAward, loginaward)
PropTypeToValue(eRoleVipAward, vipaward)
PropTypeToValue(eRoleMoneyTreeTimes, moneytreetimes)

PropTypeToValue(eRolePvpAward, pvpaward)
PropTypeToValue(eRolePvpTimes, pvptimes)
PropTypeToValue(eRolePvpBuyTimes, pvpbuytimes)
PropTypeToValue(eRoleNextPvpResetTime, nexPvpResetTime)
PropTypeToValue(eRoleTeamCopyResetTimes, teamcopyresettimes)

PropTypeToValue(eRolePropPetList, RolePropPetList)
PropTypeToValue(eRolePropAssistPetList, roleassistpets)
PropTypeToValue(eRolePropCrossDay, crossday)
PropTypeToValue(eRolePropCrossWeek, crossweek)
PropTypeToValue(eRolePropVipLvl, viplvl)
PropTypeToValue(eRolePropVipExp, vipexp)
PropTypeToValue(eRolePropVipLvlAward, viplvlaward)
PropTypeToValue(eRolePropDungLevel1, dunglevel1)
PropTypeToValue(eRolePropDungLevel2, dunglevel2)
PropTypeToValue(eRolePropDungLevel3, dunglevel3)
PropTypeToValue(eRolePropDungResetTimes1, dungreset1)
PropTypeToValue(eRolePropDungResetTimes2, dungreset2)
PropTypeToValue(eRolePropDungResetTimes3, dungreset3)

PropTypeToValue(eRolePropFriendDungeLastFreshTime, frienddungefreshtime) //好友地下城的上次刷新时间
PropTypeToValue(eRolePropFriendDungeAttendTimes, FriendDungeAttendTimes)
PropTypeToValue(eRolePropFriendDungeRecord, frienddungerecord)  //好友地下城进度
PropTypeToValue(eRolePropFriendDungeFriendRecord, frienddungefriendrecord) //已使用的好友

PropTypeToValue(eRolePropPrintCopyFreshTime, printcopyfreshtime) //图纸副本的刷新时间

PropTypeToValue(eRolePropBanchat, banchat)
PropTypeToValue(eRoleIsInPvp, isinpvp)
PropTypeToValue(eRoleLottery, lottery)//已废
PropTypeToValue(eRoleLotteryHistory,lotteryhistory) //已废
PropTypeToValue(eRolePropPlayerAccount, playeraccount)
PropTypeToValue(eRolePvpCoolDown, pvpcooldown)

PropTypeToValue(eRoleGuild, guild)                          //公会
PropTypeToValue(eRoleGuildSkill,guildskill)

PropTypeToValue(eDailyQuestRefreshTimes, DailyQuestRefreshTimes)
PropTypeToValue(eDailyQuestCount, DailyQuestCount)
PropTypeToValue(eRoleActivePetId, actviepetid)
PropTypeToValue(eRoleTowerDefenseTimes, towertimes)
PropTypeToValue(eRoleTowerDefenseTimesFresh, towertimesfresh)   //次数刷新时间点

PropTypeToValue(eRoleMysteriousNum, mysteriousNum)
PropTypeToValue(eRoleMysteriousGoldNum, mysteriousGoldNum)
PropTypeToValue(eRoleMysteriousTm, mysteriousTm)

PropTypeToValue(eRoleMysterCopyTimes, mysticalcopytimes)
PropTypeToValue(eLastLogin, lastlogin)

PropTypeToValue(eRolePetPvpForm, petpvpform)    //幻兽竞技场阵型
PropTypeToValue(eRolePetPvpFormSize, petpvpformszie)    //阵型幻兽数目
PropTypeToValue(eRolePetPvpCups, petpvpcups)    //幻兽竞技奖杯数，用于排行
PropTypeToValue(eRolePetPvpPoints, petpvppoints)    //幻兽竞技场积分，用于兑换物品
PropTypeToValue(eRolePetPvpHistoryRank, petpvphistoryrank)  //历史最高排名
PropTypeToValue(eRolePetPvpTimes, petpvptimes)  //幻兽竞技场参加的次数，每天重设为零
PropTypeToValue(eRolePetPvpWinTimes, petpvpwintimes)    // 胜利的场次，机器人不必要
PropTypeToValue(eRolePetPvploseTimes, petpvplosetimes)  //失败场次，机器人不必要
PropTypeToValue(eRolePetPvpPeridAwardRank, petPvpPeriodAward)   //
PropTypeToValue(eRolePetPvpVersion, petpvpversion)    //幻兽竞技场版本

PropTypeToValue(eRoleWorldChatTime, worldchattime)

PropTypeToValue(eRolePropWeapQua, weaponqua)

PropTypeToValue(eRoleFriendAwardCount, friendawardcount)
PropTypeToValue(eRoleFriendAwards, friendawards)

PropTypeToValue(eRoleBattleForce, bat)  // 战斗力
PropTypeToValue(eRolePropAnger, roleAnger)  //怒气

PropTypeToValue(eRolePropEnchantDust, enchantdust)    // 附魔尘
PropTypeToValue(eRolePropEnchantGold, enchantgold)    // 附魔钻

PropTypeToValue(eRoleLoginDays, logindays)              // 玩家登陆的累计天数

PropTypeToValue(eRoleAwakeLvl, awakelvl)            //觉醒等级 

PropTypeToValueEnd()

extern const char* GetRolePropName(int type);

#endif
