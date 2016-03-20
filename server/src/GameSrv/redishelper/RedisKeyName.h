//  Created by Huang Kunchao on 13-8-26.
//
//  Redis Key Name

#pragma once

#ifndef RedisNameTableBegin
#define RedisNameTableBegin() enum rnRedisNameTable {
#endif

#ifndef RedisNameDef
#define RedisNameDef(name, value)  name,
#endif

#ifndef RedisNameTableEnd
#define RedisNameTableEnd()  rnRedisNameCount, };
#endif

RedisNameTableBegin()

RedisNameDef(rnGuildAllId,guildallid)                   //全部公会id
RedisNameDef(rnGuild,guild)                             //公会
RedisNameDef(rnGuildMember,guildmember)                 //公会成员
RedisNameDef(rnRoleGuild,roleguild)                     //角色公会属性
RedisNameDef(rnRoleApplyGuildList,roleapplyguild)       //角色申请公会列表
RedisNameDef(rnGuildApplicant,guildapplicant)           //公会被申请列表
RedisNameDef(rnGuildLog,guildlog)                       //公会日志
RedisNameDef(rnGuildChat,guildchat)                     //公会聊天

RedisNameDef(rnPet,pet)                                 //宠物

RedisNameDef(rnPetCasinoWager,petcasinowager)           //幻兽大比拼押注

RedisNameDef(rnHonor, honor)     		//称号
RedisNameDef(rnHonorCond, honorcond)    //称号条件值记录

RedisNameDef(rnRole, role)								// 角色属性

RedisNameDef(rnFameChallData, famechalldata)              // 名人堂角色战斗信息


RedisNameDef(rnOnceRechardAwardAct, once_rechard_award_act)  // 一次性充值活动
RedisNameDef(rnMonthRechardAwardAct, month_rechard_award_act)  // 月卡充值活动
RedisNameDef(rnInviteAct, invite_code_act)  // 邀请码

RedisNameDef(rnAccumulateRechardAwardAct, acc_rechard_award_act)  // 累计充值活动
RedisNameDef(rnAccumulateConsumeAwardAct, acc_consume_award_act)  // 累计消费活动
RedisNameDef(rnAccumulateFatAwardAct, acc_consumefat_award_act)   // 消耗精力换奖励活动

RedisNameDef(rnMailCustom, mailcustom)  // 定制邮件

RedisNameDef(rnRealPvpData, realpvpdata)    //实时竞技场数据，用在Global线程

RedisNameDef(rnWardrobeData, wardrobedata)
RedisNameDef(rnDailySchduleData, dailyscheduledata)

RedisNameDef(rnEnchantDBData,  enchantdbdata)   // 武器附魔
RedisNameDef(rnRoleLoginDaysAward, enRoleLoginDaysAward)          // 角色奖励

RedisNameDef(rnRetinue, retinue)    //侍魂

RedisNameTableEnd()
extern const char* g_GetRedisName(int type);


