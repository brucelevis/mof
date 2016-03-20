//
//  Global_def.h
//  GameSrv
//
//  Created by pireszhi on 13-7-1.
//
//

#ifndef GameSrv_Global_def_h
#define GameSrv_Global_def_h

#include "GlobalMsg_helper.h"
#include "Paihang.h"
#include "FamesHall.h"

begin_global(PaihangTest)
    global_member(int, name)
    global_arr_member(string, test);
end_global()

begin_global(updatePaihangDB)
    global_member(int, lvl)
    global_member(int, battle)
    global_member(int, recharge)
    global_member(int, consume)
    global_member(int, roleid)
    global_member(int, optionType)
end_global()

begin_global(updatePetPaihangDB)
    global_member(int, petid)
    global_member(int, optionType)
    global_member(int, petBattle)
    global_arr_member(PetPaihangUploadData, petData)
end_global()

begin_global(ClientGetPaihangData)
    global_member(void*, data)
    global_member(int, sessionid)
end_global()

begin_global(getPaihangData)
    global_member(PaihangMsgData, info)
    global_member(void*, retCallBackFunc)
end_global()

begin_global(getPaihangDataRet)
    global_member(int, type)
    global_member(int, roleid)
    global_member(vector<SortPetInfo>, petData)
    global_member(vector<SortInfo>, sortData)
    global_member(void*, retCallBackFunc)
end_global()

begin_global(CrossServiceGetPaihangDbData)
global_member(int, type)
global_member(int, sessionid)
end_global()

/******
 pvp
 *****/

begin_global(getPvpData)
    global_member(int64_t, roleid)
    global_member(int, sessionid)
    global_member(int, myAward)
    global_member(int, getalldata)
end_global()

begin_global(deletRoleInPvp)
    global_member(int64_t, roleid)
end_global()

begin_global(addRoleToPvp)
    global_member(int64_t , roleid)
end_global()

begin_global(onAddRoleToPvp)
    global_member(int64_t , roleid)
end_global()

begin_global(beginPvp)
    global_member(int64_t, roleid)
    global_member(int, bechallengeRank)
    global_member(int64_t, bechallengeid)
end_global()

begin_global(beginPvpret)
    global_member(int, errorcode)
    global_member(int64_t, roleid)
    global_member(int64_t, bechallengeid)
end_global()

begin_global(pvpResult)
    global_member(int64_t, roleid)
    global_member(int, isWin)
    global_member(int64_t, bechallengeid)
    global_member(int, battleTime)
end_global()

begin_global(pvpResultret)
    global_member(int, broacast)
    global_member(int, iswin)
    global_member(int64_t, roleid)
    global_member(int64_t, bechallengeid)
    global_member(int, myrank)
    global_member(int, prerank)
    global_member(int, cooldown)
    global_member(int, battleTime)
end_global()

begin_global(pvpEraseRoleInpvp)
    global_member(int64_t, roleid)
    global_member(int64_t, bechallengeid)
end_global()

begin_global(onSendPvpAward)
    global_member(vector<int64_t>, roles)
    global_member(vector<int>, ranks)
end_global()

/********************************************************/
begin_global(regActivityTimer)
    global_member(int, actid) // 活动id
    global_member(int, type)  // 定时器类型： before_open,ater_open,before_close,after_close
    global_member(int, relative_secs) // 相对时间（秒）
    global_member(void*, timerfun) // 回调函数
    global_member(string, params)  // 参数
end_global()

begin_global(regActivityOnTimer)
    global_member(int, actid) // 活动id
    global_member(void*, timerfun) // 回调函数
    global_member(int, number) // 活动在当天的序号， 0第一场，1第二场，2第三场 ...
    global_member(string, params) // 参数
end_global()

begin_global(onActivityInited)
end_global()

begin_global(reqActivityStats)
    global_member(int, sessionid) // 活动id
end_global()


/******************************* 宠物竞技场 ***********************************/

begin_global(getPetPvpData)
    global_member(obj_roleinfo, playerinfo)
end_global()

begin_global(onGetPetPvpData)
    global_member(int, errorcode)
    global_member(obj_roleinfo, playerinfo)
    global_member(vector<int>, petlist)

    global_member(int,randking)   //排名
    global_member(int, cups)       //所谓的竞技等级
    global_member(int, points)     //可用积分
    global_member(int, pvptype)    //黄金，白银， 青铜段
//    global_member(int, pvptimes)   //今天一进行的pvp次数
    global_member(int, wintimes)   //胜利场次
    global_member(int, losetimes)  //不胜利场次
    global_member(int, weeklyType) //上周结算时的竞技场段位
    global_member(int, weeklyRank) //上周结算排名,为0表示已经发放过积分不提示，非零表示刚领取了积分，客户端提示一下
    global_member(int, awardpoint) //上周结算积分
    global_member(vector<int>, historyRank)    //历史战绩
end_global()

begin_global(onGetWeeklyAwardPoints)
    global_member(int, roleid)
    global_member(int, getpoints)
    global_member(int, surplusPoints)
end_global()

begin_global(editPetPvpFormation)
    global_member(obj_roleinfo, playerinfo)
    global_member(bool, noAck)
    global_member(int, sessionid)
    global_member(std::vector<int>, newformation)
end_global()

begin_global(getPetPvpEnemy)
    global_member(int, firstSearch)
    global_member(obj_roleinfo, playerinfo)
    global_member(int, gold)
end_global()

begin_global(onGetPetPvpEnemy)
    global_member(int, roleid)
    global_member(int, needGold)
    global_member(int, errorcode)
    global_member(std::vector<int>, enemyPetformation)
    global_member(obj_roleinfo, enemyInfo)
    global_member(int, enemyPvptype)
    global_member(int, enemyRnak)
end_global()

begin_global(beginPetPvpBattle)
    global_member(int, roleid)
    global_member(int, enemyid)
    global_member(int, sceneid)
end_global()

begin_global(onBeginPetPvpBattle)
    global_member(int, errorcode)
    global_member(int, roleid)
    global_member(int, sceneid)
    global_member(vector<int>, petlist)
end_global()

begin_global(petPvpBattleHandle)
    global_member(obj_roleinfo, playerinfo)
    global_member(int, pvptimes)
    global_member(int, sessionid)
    global_member(int, iswin)
end_global()

begin_global(onPetPvpBattleHandle)
    global_member(int, errorcode)
    global_member(int, roleid)
    global_member(int, iswin)
    global_member(int, rank)    //段排名
    global_member(int, globalRank)  // 全局排名
    global_member(std::vector<int>, bestHistoryRanks)   //历史最佳排名
    global_member(int, pvptype)
    global_member(int, winTimes)
    global_member(int, getCups)
    global_member(int, givePoints)
    global_member(int, surplusPoints)
    global_member(int, enemyid)
    global_member(int, enemyGetPoints)
    global_member(int, enemySurplusPoints)
end_global()

begin_global(delPlayerInPetPvp)
    global_member(int, roleid)
end_global()

begin_global(endSearchPetPvpEnemy)
    global_member(obj_roleinfo, playerinfo)
end_global()

begin_global(removePetFromPetPvpFormation)
    global_member(int, roleid)
    global_member(std::vector<int>, petlist)
end_global()

begin_global(getPetPvpPetList)
    global_member(int, roleid)
    global_member(int, sessionid)
end_global()

begin_global(buyItemsUsingPetpvpPoints)
    global_member(int, roleid)
    global_member(int, sessionid)
    global_member(int, itemid)
    global_member(int, count)
end_global()

begin_global(onBuyItemsUsingPetpvpPoints)
    global_member(int, roleid)
    global_member(int, sessionid)
    global_member(int, errorcode)
    global_member(int, usingPoints)
    global_member(int, surplusPoints)
    global_member(int, itemid)
    global_member(int, count)
end_global()

begin_global(gmAddPetPvpPoints)
    global_member(int, roleid)
    global_member(int, addpoints)
end_global()

begin_global(gmSetPetPvpPoints)
global_member(int, roleid)
global_member(int, setpoints)
end_global()

begin_global(gmSetPetPvpCups)
    global_member(int, roleid)
    global_member(int, setCups)
end_global()

begin_global(onGmAddPetPvpPoints)
    global_member(int, roleid)
    global_member(int, getPoints)
    global_member(int, surplusPoints)
end_global()

/*********************************** 名人堂 ***********************************/
begin_global(getFamesHallPlayerData)
    global_member(obj_roleinfo, playerinfo)
    global_member(int, sessionid)
    global_member(int, versionid)
end_global()

begin_global(getFamesHallList)
    global_member(obj_roleinfo, playerinfo)
    global_member(int, sessionid)
end_global()

begin_global(beginFamesHallBattle)
    global_member(obj_roleinfo, playerinfo)
    global_member(int, checkLayer)
    global_member(int, sceneid)
end_global()

begin_global(onBeginFamesHallBattle)
    global_member(int, errorcode)
    global_member(int, roleid)
    global_member(int, sceneid)
end_global()

begin_global(famesHallBattleHandle)
    global_member(obj_roleinfo, playerinfo)
    global_member(FamesHallPointFactor, factor)
    global_member(int, result)
    global_member(int, layer)
    global_member(int, sceneid)
end_global()

begin_global(onFamesHallBattleHandle)
    global_member(int, errorcode)
    global_member(int, roleid)
    global_member(int, result)
    global_member(int, layer)
    global_member(int, sceneid)
end_global()

begin_global(GetFamesHallData)
    global_member(void*, cb)
end_global()

begin_global(onGetFamesHallData)
    global_member(AllJobFamesList, datas)
    global_member(void*, cb)
end_global()


/******************************** 实时竞技场 *******************************/

//获取自己的竞技场相关信息
begin_global(GetRealpvpInfo)
    global_member(int, roleid)
    global_member(int, matchingIndex)
    global_member(obj_realpvp_info, roleinfo)
end_global()

begin_global(onGetRealpvpInfo)
    global_member(int, roleid)
    global_member(int, CurrnetMatchingIndex)
    global_member(int, beginTimes)
    global_member(obj_realpvp_info, info)
end_global()
 
//玩家通知服务器进入匹配队列
begin_global(ReadyToRealpvpMatching)
    global_member(int, roleid)
    global_member(int, todayTimes)
    global_member(obj_realpvp_info, roleinfo)
end_global()

begin_global(onReadyToRealpvpMatching)
    global_member(int, roleid)
    global_member(int, errorcode)
end_global()

//玩家通知服务器取消匹配
begin_global(CancelMatching)
    global_member(int, roleid)
end_global()

begin_global(onCancelMatching)
    global_member(int, roleid)
    global_member(int, errorcode)
end_global()

//把匹配的结果通知Game线程，通知玩家及同步场景线程
begin_global(onNotifyRealpvpMatchingResult)
    global_member(int, firstRoleid)
    global_member(obj_realpvp_info, firstRoleInfo)
    global_member(int, secondeRoleid)
    global_member(obj_realpvp_info, secondRoleInfo)
end_global()

begin_global(onSendBeginMsgToSynScene)
    global_member(int, sceneid)
    global_member(int, sceneMod)
    global_member(int, firstRole)
    global_member(int, secondRole)
end_global()

//通知玩家匹配失败
begin_global(onNotifyRealpvpMatchingFail)
    global_member(int, roleid)
    global_member(SynPvpAward, award)
end_global()

//处理
begin_global(HandleRealpvpFightingResult)
    global_member(int, winnerid)
    global_member(int, loserid)
end_global()

begin_global(ResetRealpvpFightingState)
    global_member(int, firstRole)
    global_member(int, secondRole)
end_global()

begin_global(RealpvpFightingEnemyEscape)
    global_member(int, roleid)
    global_member(int, enemyid)
end_global()

begin_global(onHandleRealpvpFightingResult)
    global_member(int, roleid)
    global_member(int, result)                  //1=win, 0=lose
    global_member(SynPvpAward, getAward)
end_global()

begin_global(RealpvpMedalExchange)
    global_member(int, roleid)
    global_member(int, itemIndex)
    global_member(int, itemCount)
end_global()

begin_global(onRealpvpMedalExchange)
    global_member(int, roleid)
    global_member(int, itemIndex)
    global_member(int, itemCount)
    global_member(int, errorcode)
end_global()

begin_global(GmAddRealpvpMedal)
    global_member(int, roleid)
    global_member(int, addmedal)
end_global()

begin_global(GmSetRealpvpMilitary)
global_member(int, roleid)
global_member(int, index)
end_global()

begin_global(GmSetRealpvpMeds)
global_member(int, roleid)
global_member(int, setMeds)
end_global()

begin_global(GmSetRealpvpWinTimes)
global_member(int, roleid)
global_member(int, setWinTimes)
end_global()

begin_global(GmSetRealpvpLostTimes)
global_member(int, roleid)
global_member(int, setLostTimes)
end_global()

begin_global(GmSetRealpvpHons)
global_member(int, roleid)
global_member(int, setHons)
end_global()
             
begin_global(GmAddRealpvpWins)
    global_member(int, roleid)
    global_member(int, addWins)
end_global()

begin_global(onRealpvpMedalChangeLog)
    global_member(int, roleid)
    global_member(int, totalcount)
    global_member(int, changecount)
    global_member(string, type)
    global_member(string, changeFrom)
end_global()

begin_global(GmSetGuildLvl)
    global_member(int, roleid)
    global_member(int, num)

end_global()

begin_global(GmSetGuildFortune)
    global_member(int, roleid)
    global_member(int, num)
end_global()

begin_global(GmSetGuildDcons)
    global_member(int, roleid)
    global_member(int, num)
end_global()




#endif
