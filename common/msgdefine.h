#include "msgdefine_login.h"
#include "msgdefine_world.h"
#ifndef CLIENT
#include "msgdefine_server.h"
#endif

begin_msg(req_heartbeat, 1, 1)

end_msg()

//随机名字
begin_msg(req_randname,1,3)
    def_int(male)
end_msg()

begin_msg(ack_randname,1,4)
    def_err(errorcode)
    def_string(name)
end_msg()

//创建角色
begin_msg(req_createrole,1,5)
    def_int(roletypeid)
    def_string(rolename)
    def_string(proplist)
end_msg()

begin_msg(ack_createrole,1,6)
    def_err(errorcode)
    def_int(roleid)
end_msg()

//选择角色
begin_msg(req_select_role,1,7)
    def_int(roleid)
end_msg()

begin_msg(ack_select_role,1,8)
    def_err(errorcode)
    def_int(roleid)
end_msg()

//删除角色
begin_msg(req_delete_role,1,9)
    def_int(roleid)
end_msg()

begin_msg(ack_delete_role,1,10)
    def_err(errorcode)
end_msg()

//获取角色
begin_msg(req_getroles, 1, 11)
end_msg()

begin_msg(ack_getroles,1,12)
    def_err(errorcode)
    def_object_arr(obj_role,roles)
end_msg()


begin_msg(req_getrolesave, 1, 13)
end_msg()

begin_msg(ack_getrolesave, 1, 14)
    def_int(errorcode)
    def_int(exp)
    def_int(fat)
    def_int(gold)
    def_int(rmb)
    def_int(battlePoint)
    def_int(constellval)
    def_int(lastFinishCopy)
    def_int(dungTimes)
    def_int(eliteTimes)
    def_int(fatbuytime)
    def_int(elitebuytimes)
    def_int(pvpAward)
    def_int(pvptimes)
    def_int(pvpbuytimes)
    def_int(teamresettimes)
    def_int(dunglevel1)
    def_int(dunglevel2)
    def_int(dunglevel3)
    def_int(dungresetlvl1)
    def_int(dungresetlvl2)
    def_int(dungresetlvl3)
    def_int(dungsweepinglevel1)
    def_int(dungsweepinglevel2)
    def_int(dungsweepinglevel3)
    def_int(dungissweeped1)
    def_int(dungissweeped2)
    def_int(dungissweeped3)
    def_int(petadventurebuytimes) //幻兽大冒险副本次数购买次数
    def_int(petadventuretimes) //幻兽大冒险副本次数
    def_int(petadventurerobotpetrenttimesremain)                   //幻兽大冒险机器幻兽租用次数
    def_int(petadventurerobotpetbuyingrenttimesperday)               //幻兽大冒险机器幻兽当天购买次数

    def_int(crossservicewarworshiptimes) //跨服战膜拜次数
    def_int(crossservicewarworshipbuytimes) //跨服战膜拜购买次数
    def_int(crossservicewartimes) //跨服战次数
    def_int(crossservicewarbuytimes) //跨服战购买次数

    def_int(viplvl)
    def_int(vipexp)
    def_int(viplvlaward)
    def_int_arr(finishElitecopy)
    def_int(dailyQuestRefreshTimes)
	def_int(prestige) // 声望
    def_int(petEliteTimes)  // 幻兽试炼场剩余次数
    def_int(petEliteBuytimes)   // 已买次数
    def_int(roleAnger)  //角色怒气值
    def_int(doublePotionNum)   //用于好友副本 剩余 双倍药水次数
    def_int(enchantDust)      // 附魔尘, 普通的
    def_int(enchantGold)      // 附魔钻
    def_int(roleAwakeLvl)    //觉醒等级
end_msg()

begin_msg(req_login_super, 1, 23)
    def_string(account)
    def_string(platform)
    def_string(password)
end_msg()

begin_msg(ack_login_super, 1, 24)
end_msg()



////////////
begin_msg(notify_disconnect_tip, 1, 27)
    def_string(tip)
end_msg()


begin_msg(req_verify_server_info, 1, 28)
    def_int(serverid)
    def_string(version)
end_msg()

begin_msg(ack_verify_server_info, 1, 29)
    def_int(serverid)
    def_string(servername)
    def_string(version)
end_msg()

//恢复角色
begin_msg(req_restore_role,1,30)
def_int(roleid)
end_msg()

begin_msg(ack_restore_role,1,31)
def_err(errorcode)
end_msg()

begin_msg(req_get_role_data, 1, 32)
    def_int(roleid)
    def_int(isrobot)
    def_int(serverid)
end_msg()

begin_msg(ack_get_role_data, 1, 33)
    def_int(errorcode)
    def_int(roleid)
    def_int(isrobot)
    def_string(rolename)
    def_int(lvl)
    def_int(roletype)
    def_int(lastlogin)
    def_string(guildname)
    def_int(atk)
    def_int(def)
    def_float(hit)
    def_float(dodge)
    def_int(hp)
    def_float(cri)
    def_int(stre)
    def_int(inte)
    def_int(phys)
    def_int(capa)
    def_int(constellid)
    def_int(constellstep)
    def_int(activepet)
    def_object_arr(obj_playerItem, equips)
    def_int(guildLvl)
    def_int(guildposition) // 职位
	def_int(prestige) // 声望
    def_int(bodyfashion)
    def_int(weaponfashion)
    def_int(cityBodyFashion)
    def_int_arr(fashionList)
    def_int(playerWardrobeLvl);
	def_int(guildRank) // 公会排名
    def_int(enchantId)  // 当前附魔id
    def_int(enchantLvl) // 当前附魔等级
    def_int(awakeLvl)

    def_int(onReqServerid)
end_msg()


//登出
begin_msg(req_logout,1,43)
end_msg()

begin_msg(ack_logout,1,44)
	def_err(errorcode)
end_msg()

//断开连接的模拟包，纯客户端用，收到断开链接命令后模拟这个包压到消息队列里
begin_msg(notify_server_break_socket, 1, 45)
end_msg()

//发送角色属性，宠物属性
begin_msg(notify_role_prop_info, 1, 46)

    def_int(Phys)
    def_int(Capa)
    def_int(Inte)
    def_int(Stre)

    def_int(maxHp)
    def_int(atk)            //攻击
    def_int(def)            //防御
    def_float(hit)          //命中
    def_float(dodge)        //闪避
    def_float(cri)          //暴击

    def_float(verifyPropCode)
    def_int(isBattleBuf)
end_msg()

begin_msg(notify_pet_prop_info, 1, 47)
    def_int(petid)
    def_int(isActive)
    def_int(Phys)
    def_int(Capa)
    def_int(Inte)
    def_int(Stre)

    def_int(maxHp)
    def_int(atk)            //攻击
    def_int(def)            //防御
    def_float(hit)          //命中
    def_float(dodge)        //闪避
    def_float(cri)          //暴击

    def_int_arr(skill)      //技能

    def_float(verifyPropCode)
    def_int(isBattleBuf)
end_msg()

begin_msg(notify_petAdventurerRobotPet_prop_info, 1, 48)
    def_object(obj_petBattleProp, robotpetinfo)
    def_float(verifyPropCode)
end_msg()

begin_msg(req_enterroom,2,0)
	def_int(roomtype)
	def_int(threadid)
	def_string(password)
	def_int(harden)
end_msg()

begin_msg(ack_enterroom,2,1)
	def_err(errorcode)
	def_int(seceneid)
	def_int(secenetype)
end_msg()

begin_msg(req_enter_city, 2, 2)
    def_int(cityid)
    def_int(threadid)
end_msg()

begin_msg(ack_enter_city, 2, 3)
    def_int(errorcode)
    def_int(cityid)
    def_int(threadid)
end_msg()

begin_msg(notify_enter_city, 2, 4)
    def_object(obj_roleinfo, roleinfo)
end_msg()

begin_msg(notify_leave_city, 2, 5)
    def_int(roleid)
    def_string(rolename)
end_msg()


begin_msg(req_enter_world_scene, 2, 6)
    def_int(sceneMod)
    def_int(sceneId)
end_msg()

begin_msg(ack_enter_world_scene, 2, 7)
    def_int(errorcode)
    def_int(sceneMod)
    def_int(sceneId)
end_msg()


begin_msg(req_leave_world_scene, 2, 8)

end_msg()

begin_msg(ack_leave_world_scene, 2, 9)
    def_int(errorcode)
end_msg()

begin_msg(notify_player_state_refresh, 2, 10)
    def_int(roleid)
    def_int(rolestate)      // sceneID
end_msg()


begin_msg(notify_friend_state_refresh, 2, 11)
    def_int(friendid)
    def_int(sceneid)
end_msg()

// 应用指令提示
begin_msg(notify_app_cmd, 2, 12)
	def_string(data)
end_msg()

begin_msg(notify_backToCity, 2, 13)
    def_int(cityid)
end_msg()


begin_msg(req_get_scene_threads, 2, 14)
    def_int(cityid)
end_msg()

begin_msg(ack_get_scene_threads, 2, 15)
    def_int(cityid)
    def_int_arr(threadroles)
end_msg()

begin_msg(req_auto_testtool, 2, 16)
end_msg()

begin_msg(ack_auto_testtool, 2, 17)
	def_int(state)	// 0关闭 1开启
end_msg()
/////////////////场景&玩家///////////////////////

//获取玩家背包
begin_msg(req_playerbag,3,0)
end_msg()

begin_msg(ack_playerbag,3,1)
    def_int(errorcode)
    def_int(maxbagsize)
    def_int(bagsize)
    def_int(rmbextendedsize)
    def_object_arr(obj_bagItem, items)
end_msg()

begin_msg(notify_playerbag,3,2)
    def_int(errorcode)
    def_object_arr(obj_addordelItem,items)
end_msg()

//丢弃背包内装备
begin_msg(req_dropBagEquip,3,3)
    def_int(itemIndex)
end_msg()


begin_msg(ack_dropBagEquip,3,4)
    def_int(errorcode)
end_msg()

//获取身上装备
begin_msg(req_PlayerEquip, 3, 5)
end_msg()

begin_msg(ack_PlayerEquip, 3, 6)
    def_int(errorcode)
    def_object_arr(obj_playerItem, equips)
end_msg()

begin_msg(notify_PlayerEquip, 3, 7)
    def_int(errorcode)
    def_object_arr(obj_updateEquip, equips)
end_msg()

//卸下装备
begin_msg(req_unloadEquip,3,8)
    def_int(itemindex)
end_msg()

begin_msg(ack_unloadEquip,3,9)
    def_int(errorcode)
end_msg()

//穿上装备
begin_msg(req_loadEquip,3,10)
    def_int(itemindex)
end_msg()

begin_msg(ack_loadEquip,3,11)
    def_int(errorcode)
end_msg()

//整理背包
begin_msg(req_sortbag,3,12)
end_msg()

begin_msg(ack_sortbag,3,13)
    def_err(errorcode)
end_msg()


//服务发放物品
begin_msg(notify_addbagItem,3,14)
    def_int(reason) //1:任务发放 2:翻牌获取 3:副本奖励 4:gm发放
    def_object_arr(obj_bagItem,itemlist)
end_msg()

//查看其它玩家
begin_msg(req_lookoplayer,3,15)
    def_string(playername)
end_msg()

begin_msg(ack_lookoplayer,3,16)
    def_err(errorcode)
    def_int(playerid)
    def_int(playertype)
    def_int(playerjob)
    def_object_arr(obj_playerItem,items)
    def_string(strJson)
end_msg()

//翻牌
begin_msg(notify_flopcard,3,17)
    def_int(errorcode)
    def_object(obj_copyaward, award)
end_msg()

///装备改造
begin_msg(req_upgEquip, 3, 30)
    def_int(position)   //1表示装备栏，0表示背包
    def_int(index)
    def_int(luckystoneid)   //幸运石id**
    def_int(upgEquipCount)        //强化的次数
    def_int(materialid)        //  如果使用强化石强化装备,暂时定为1(以后强化石类型增多 传值为id, 代码逻辑也需要更改), 如果为0则使用金币强化装备
end_msg()

begin_msg(ack_upgEquip, 3, 31)
    def_int(errorcode)
    def_int(position)
    def_int(index)
    def_int(starlvl)
    def_int(starexp)
    def_int(upgEquipCount)
end_msg()


begin_msg(req_promoteEquip, 3, 32)
    def_int(index)
    def_int(makeid)
end_msg()

begin_msg(ack_promoteEquip, 3, 33)
    def_int(errorcode)
    def_int(index)
    def_int(makeid)
	def_int(returnGold)
end_msg()

begin_msg(req_attachEquip, 3, 34)
    def_int(equipindex)
    def_int(stoneid)
end_msg()

begin_msg(ack_attachEquip, 3, 35)
    def_int(errorcode)
    def_int(equipindex)
    def_int(stoneid)
end_msg()

begin_msg(req_deattachEquip, 3, 36)
    def_int(equipindex)
    def_int(stoneindex)
end_msg()

begin_msg(ack_deattachEquip, 3, 37)
    def_int(errorcode)
    def_int(equipindex)
    def_int(stoneindex)
end_msg()

begin_msg(req_composeItem, 3, 38)
    def_int(composeid)
    def_int(count)
end_msg()

begin_msg(ack_composeItem, 3, 39)
    def_int(errorcode)
    def_int(composeid)
    def_int(newcount)
end_msg()

begin_msg(req_useitem, 3, 40)
    def_int(index)
end_msg()

begin_msg(ack_useitem, 3, 41)
    def_int(errorcode)
    def_int(index)
    def_int(itemid)
    def_string(items)
end_msg()

begin_msg(req_sellitem, 3, 42)
    def_int(index)
    def_int(num)
end_msg()

begin_msg(ack_sellitem, 3, 43)
    def_int(errorcode)
    def_int(itemid)
    def_int(sgold)
end_msg()

begin_msg(req_use_allitems, 3, 44)
    def_int(index)
end_msg()

begin_msg(ack_use_allitems, 3, 45)
    def_int(errorcode)
    def_int(index)  //背包位置
    def_int(itemid) //物品id
    def_int(count)  //使用物品数量
    def_object_arr(obj_item, items) //得到的物品
end_msg()

begin_msg(req_fusionEquip, 3, 46)   //装备融合
    def_int(equipindex)
end_msg()

begin_msg(ack_fusionEquip, 3, 47)
    def_int(errorcode)
end_msg()

begin_msg(notify_bagfull_and_send_mail, 3, 48)
end_msg()

begin_msg(req_activate_baggrids, 3, 49)
     def_int(activatepage)
end_msg()

begin_msg(ack_activate_baggrids, 3, 50)
     def_int(errorcode)
     def_int(bagsize)
     def_int(rmbextendedsize)
end_msg()

begin_msg(req_equip_baptize, 3, 51)
    def_int(equipIndex)     //装备部位
    def_int(baptizeQua)     //洗练格子的颜色
    def_int(baptizeIndex)   //某颜色的格子的序号
end_msg()

begin_msg(ack_equip_baptize, 3, 52)
    def_int(errorcode)
    def_int(equipIndex)
    def_int(baptizeQua)     //洗练格子的颜色
    def_int(baptizeIndex)   //某颜色的格子的序号
end_msg()

begin_msg(req_equip_baptize_recover, 3, 53)
    def_int(equipIndex)
    def_int(baptizeQua)
    def_int(baptizeIndex)
end_msg()

begin_msg(ack_equip_baptize_recover, 3, 54)
    def_int(errorcode)
    def_int(equipIndex)
    def_int(baptizeQua)     //洗练格子的颜色
    def_int(baptizeIndex)   //某颜色的格子的序号
end_msg()

////////////////技能相关//////////////////////
//请求可学习的技能列表
begin_msg(req_studyable_skill_list,4,0)
end_msg()

//接收可学习的技能列表
begin_msg(ack_studyable_skill_list,4,1)
    def_err(errorcode)
    def_object_arr(obj_studyable_skill, skillList)
end_msg()

//请求学习技能
begin_msg(req_study_skill,4,2)
    def_int(skillID)
end_msg()

//接收学习技能的结果
begin_msg(ack_study_skill,4,3)
    def_err(errorcode)
    def_int(skillID)
end_msg()

//请求装备技能
begin_msg(req_equip_skill,4,4)
    def_int(skillID)
    def_int(index)
    def_int(bindType)
end_msg()

//接收装备技能的结果
begin_msg(ack_equip_skill,4,5)
    def_err(errorcode)
    def_int(skillID)
    def_int(index)
end_msg()

//请求卸载技能
begin_msg(req_unequip_skill,4,6)
    def_int(index)
end_msg()

//接收卸载技能的结果
begin_msg(ack_unequip_skill,4,7)
    def_err(errorcode)
    def_int(index)
end_msg()

//请求已经装备的技能列表
begin_msg(req_equiped_skill_list,4,8)
end_msg()

//接收已经装备的技能列表
begin_msg(ack_equiped_skill_list,4,9)
    def_err(errorcode)
    def_object_arr(obj_equiped_skill, skillList)
end_msg()

//请求施放技能
begin_msg(req_emit_skill,4,10)
    def_int(skillID)
    def_int(targetID)
    def_int(orient)
    def_int(bindType)
end_msg()

//通告施放技能成功
begin_msg(ack_emit_skill,4,11)
    def_err(errorcode)
    def_int(skillID)
    def_int(casterID)
    def_int(distance)
    def_int(hitCount)
end_msg()

//已经学习的所有技能列表
begin_msg(req_get_player_skill_table, 4, 12)
end_msg()

begin_msg(ack_get_player_skill_table, 4, 13)
    def_int(errorcode)
    def_int_arr(skillTable)
end_msg()
///////////////任务模块//////////////////////

//获取进行中的任务
begin_msg(req_getQuests, 5, 0)
end_msg()

begin_msg(ack_getQuests, 5, 1)
    def_err(errorcode)
    def_int(dailycount) //当前日常任务次数
    def_object_arr(obj_questProgress,questProgress)
    def_int_arr(finishQuestIds)
end_msg()

//接受任务 npc,主,支线
begin_msg(req_recvQuest,5,2)
    def_int(questid)
end_msg()

begin_msg(ack_recvQuest,5,3)
    def_int(questid)
    def_int(err)
end_msg()


//添加任务 日常任务
begin_msg(notify_addQuest,5,4)
    def_int_arr(questids)
end_msg()

//通知删除任务
begin_msg(notify_delQuest,5,5)
    def_int_arr(questids)
end_msg()

//放弃任务
begin_msg(req_giveupQuest, 5, 6)
    def_int(questid)
end_msg()

begin_msg(ack_giveupQuest, 5, 7)
    def_int(questid)
    def_int(errorcode)
end_msg()

//完成任务 领取奖励
begin_msg(req_finishQuest, 5, 8)
    def_int(questid)
end_msg()

begin_msg(ack_finishQuest,5,9)
    def_int(questid)
    def_int(errorcode) //1:背包已经满了 2:服务还没完成该任务
end_msg()

//提交对话 同步任务进度
begin_msg(req_dialog,5,10)
    def_int(npdid)
    def_int(indexid)
end_msg()

//同步日常任务计数
begin_msg(notify_syn_dailycount,5,11)
    def_int(count)
end_msg()

begin_msg(notify_dunglevelrecord, 5, 12)
    def_object_arr(obj_dungeonstate, states)
    def_int(dunglevel1)
    def_int(dunglevel2)
    def_int(dunglevel3)
end_msg()

begin_msg(notify_dungResetTimes, 5, 13)
    def_object_arr(obj_dungeonstate, states)
    def_int(dungResetTimes1)
    def_int(dungResetTimes2)
    def_int(dungResetTimes3)
end_msg()

begin_msg(req_refresh_daily_quest, 5, 14)
end_msg()

begin_msg(ack_refresh_daily_quest, 5, 15)
    def_int(errorcode)
    def_int(times)
end_msg()

begin_msg(notify_dungMaxSweepingLevel, 5, 17)
    def_object_arr(obj_dungeonstate, states)
    def_int(dungMaxSweepingLevel1)
    def_int(dungMaxSweepingLevel2)
    def_int(dungMaxSweepingLevel3)
end_msg()

/////////////////////////////////////////
//


/////////////////////////////////////////



//进入副本
begin_msg(req_enter_general_copy, 6, 3)
    def_int(copyid)
end_msg()

begin_msg(ack_enter_general_copy, 6, 4)
    def_int(errorcode) //0:成功 1:失败
    def_int(copyid)
end_msg()

//完成副本
begin_msg(req_finishcopy, 6, 5)
    def_int(copyid)
    def_int(star)
    def_int(maxCombo)       //最大连击
    def_int(deadMonsterNum) //死亡的小怪数
    def_int(finishstate)    //1:挑战失败 2:通过
end_msg()

begin_msg(ack_finishcopy, 6, 6)
    def_int(rescode) //1:成功 0:失败
end_msg()


//翻牌奖励消息
begin_msg(req_treasurebox,6,7)
    def_int(timeout)
    def_int(isfree)
    def_int(index)
end_msg()

begin_msg(ack_treasurebox,6,8)
    def_err(errorcode)
    def_string(szname)
    def_int(job)
    def_int(isVIP)
    def_int(isFree)
    def_int(itemId1)
    def_string(itemName1)
    def_int(count1)
    def_int(index1)
    def_int(itemId2)
    def_string(itemName2)
    def_int(count2)
    def_int(index2)
end_msg()


//限时击杀副本 time limit kill copy
begin_msg(req_enter_tlk_copy, 6, 9)
    def_int(activityId)
    def_int(copyId)
end_msg()

begin_msg(ack_enter_tlk_copy, 6, 10)
    def_int(errorcode)
    def_int(copyId)
    def_int(gametimes)
    def_object_arr(obj_tlk_monster, monsters)
end_msg()

begin_msg(notify_refresh_tlk_monster, 6, 11)
    def_object_arr(obj_tlk_monster, monsters)
end_msg()

begin_msg(req_kill_tlk_monster, 6, 12)
    def_object_arr(obj_tlk_monster, monsters)
end_msg()

begin_msg(ack_kill_tlk_monster, 6, 13)
    def_int(errorcode)
    def_int(score)
    def_int(killscount)
end_msg()

begin_msg(req_leave_tlk_copy, 6, 14)
    def_int(type)  //1表示领取奖励，0表示不领取奖励
end_msg()

begin_msg(ack_leave_tlk_copy, 6, 15)
    def_int(errorcode)
    def_int(type)
    def_string(awards)
end_msg()

begin_msg(req_add_anger, 6, 16)
    def_int(addPoint)
end_msg()

begin_msg(req_enter_scene, 6, 17)
    def_int(sceneid)
end_msg()

begin_msg(ack_enter_scene, 6, 18)
    def_int(sceneid)
end_msg()

begin_msg(req_enter_elitecopy, 6, 19)
    def_int(copyid)
    def_int(oncemore)
end_msg()

begin_msg(ack_enter_elitecopy, 6, 20)
    def_int(errorcode)
    def_int(copyid)
end_msg()

begin_msg(req_pass_tlk_copy, 6, 21)
end_msg()

begin_msg(ack_pass_tlk_copy, 6, 22)
    def_int(errorcode)
    def_string(awards)
end_msg()

begin_msg(notify_timeout_tlk_copy, 6, 23)

end_msg()

/******************vip 相关**********************/
//获取vip奖励
begin_msg(req_getvipgift,6,50)
end_msg()

begin_msg(ack_getvipgift,6,51)
    def_err(errorcode)
    //def_object_arr(obj_vipGiftItems,nItemKinds)    //多个道具
end_msg()

begin_msg(req_getviplvlAward, 6, 52)
    def_int(viplvl)
end_msg()

begin_msg(ack_getviplvlAward, 6, 53)
    def_int(errorcode)
end_msg()

////////////////签到系统//////////////////////
begin_msg(req_loginaward,7,0)
end_msg()

begin_msg(ack_loginaward,7,1)
    def_err(errorcode)
    def_int(days)
end_msg()

begin_msg(req_loginawardstate,7,2)
end_msg()

begin_msg(ack_loginawardstate,7,3)
    def_int(days)
    def_int(todaygift)
    def_int(vipgift)
    def_int_arr(giftsFor7Days)
end_msg()

begin_msg(req_vipaward, 7, 4)
end_msg()

begin_msg(ack_vipaward, 7, 5)
    def_int(errorcode)
    def_int(lvl)
end_msg()

begin_msg(req_moneytreestate, 7, 6)

end_msg()


begin_msg(ack_moneytreestate, 7, 7)
    def_int(errorcode)
    def_int(times)
end_msg()

begin_msg(req_moneytreeop, 7, 8)

end_msg()


begin_msg(ack_moneytreeop, 7, 9)
    def_int(errorcode)
    def_int(times)
end_msg()


begin_msg(req_lotterystate,7,10)        //抽奖状态
end_msg()

begin_msg(ack_lotterystate,7,11)
    def_int(freelottery)                //今日免费已抽奖次数，道具使用次数为0时，自增
    def_int(buylottery)                 //今日收费已抽奖次数
    def_int(incr_lottery_times)         //使用道具免费抽奖次数，自减
    def_object_arr(obj_lotteryhistory,history)//今日抽奖历史
end_msg()

begin_msg(req_lottery,7,12)             //请求抽奖

end_msg()

begin_msg(ack_lottery,7,13)             //抽奖结果
    def_int(errorcode)                  //enLotteryReuslt
    def_int(costrmb)                    //消耗经济
    def_int(costType)                   //CostType
    def_int_arr(items)                  //中奖物品
    def_int(itemtype)                   //物品对应转盘格子类型
end_msg()

begin_msg(req_activation,7,14)          //激活码请求
    def_string(code)
end_msg()

begin_msg(ack_activation,7,15)          // 激活码结果
    def_int(errorcode)                  // enActivationReuslt
    def_string(startDate)               // 开始时间
	def_string(endDate)                 // 结束时间
end_msg()

begin_msg(req_n_lottery,7,16)         //一次请求n次抽奖，最大10
end_msg()

begin_msg(ack_n_lottery, 7, 17)
    def_int( err )                    // enLotteryReuslt
    def_int( succTime )               // 如果成功， 抽了几次
    def_int( lastItemType)            // 最后一件物品的类型
    def_int_arr( items )              // 物品
end_msg()

begin_msg(req_dice_lottery_state,7,18)     //色子抽奖状态
end_msg()

begin_msg(ack_dice_lottery_state,7,19)
    def_int(lotterytime)              // 抽奖次数
    def_int(position)                 // 位置
    def_object_arr(obj_lotteryhistory,history)//今日抽奖历史
end_msg()

begin_msg(req_dice_lottery,7,20)         // 请求色子抽奖
end_msg()

begin_msg(ack_dice_lottery,7,21)        // 抽奖结果
    def_int(errorcode)                  // eDiceLotteryReuslt
    def_int(newposition)                // 新位置
end_msg()

begin_msg( req_KrLvAward, 7, 32 )       // 领取等级奖励
end_msg()

begin_msg( ack_KrLvAward, 7, 33 )   //  奖励领取结果
def_int( errorcode )                //  eKrLvAwardResult
end_msg()

begin_msg( req_KrLvAwardState, 7, 34 )  // 等级奖励是否可领
end_msg()

begin_msg( ack_KrLvAwardState, 7, 35 )  // 等级奖励是否可领返回
    def_int( errorcode )          //0,可领；1，不可领
end_msg()

begin_msg( req_shareaward_state, 7, 36 )  // 分享奖励
end_msg()

begin_msg( ack_shareaward_state, 7, 37 )
	def_int( weekly )	// 周奖励(1获得,０未获得)
	def_int( day )		// 天奖励(1获得,０未获得)
	def_int( first )	// 第一次奖励(1获得,０未获得)
end_msg()

/////////聊天//////////////////////
//私聊
begin_msg(req_privatechat,8,0)
    def_int(receiverId)
    def_string(message)
    def_int(messageType)
    def_int(voiceDuration)
    def_string(uuid)
    def_blob(voiceData)
    def_int(translated)                 //是否翻译成功
end_msg()


begin_msg(notify_privatechat,8,1)
    def_int(senderId)
    def_int(senderSex)
    def_int(senderRoleType)
    def_int(senderLvl)
    def_string(senderName)
    def_string(message)
    def_string(uuid)
    def_int(messageType)
    def_int64(voiceId)
    def_int(voiceDuration)
    def_int(createDate)
    def_int(translated)                 //是否翻译成功
end_msg()

begin_msg(req_getsceneroles, 8, 2)
    def_int(maxnum)
end_msg()

begin_msg(ack_getsceneroles, 8, 3)
    def_int(errorcode)
    def_int(maxnum)
    def_object_arr(obj_roleinfo, roleinfos)
end_msg()

begin_msg(req_getfriendlist, 8, 4)
end_msg()

begin_msg(ack_getfriendlist, 8, 5)
    def_int(errorcode)
    def_object_arr(obj_friendRoleinfo, roleinfos)
end_msg()


begin_msg(req_addfriend, 8, 6)
    def_int(roleid)
end_msg()

begin_msg(ack_addfriend, 8, 7)
    def_int(errorcode)
    def_int(roleid)
end_msg()


begin_msg(notify_addfriend, 8, 8)
    def_object(obj_friendRoleinfo, role)
end_msg()

begin_msg(req_agreefriend, 8, 9)
    def_int(agree)
    def_int(roleid)
end_msg()

begin_msg(ack_agreefriend, 8, 10)
    def_int(errorcode)
    def_int(agree)
    def_object(obj_friendRoleinfo, role)
end_msg()

begin_msg(notify_agreefriend, 8, 11)
    def_int(agree)
    def_object(obj_friendRoleinfo, role)
end_msg()

begin_msg(notify_friendonline, 8, 12)
    def_int(roleid)
end_msg()

begin_msg(notify_friendoffline, 8, 13)
    def_int(roleid)
end_msg()

begin_msg(ack_private_chat, 8, 14)
    def_int(errorcode)
    def_int(receiverId)
    def_string(message)
    def_int(messageType)
    def_string(uuid)
    def_int64(voiceId)
    def_int(voiceDuration)
    def_int(senderRoleType)
    def_int(createDate)
    def_int(translated)                 //是否翻译成功
end_msg()

begin_msg(req_del_friend, 8, 15)
    def_int(friendid)
end_msg()

begin_msg(ack_del_friend, 8, 16)
    def_int(errorcode)
    def_int(friendid)
end_msg()

begin_msg(notify_del_friend, 8, 17)
    def_int(friendid)
end_msg()



//发送世界消息
begin_msg(req_worldchat,8,18)
    def_string(message)
    def_int(messageType)
    def_int(voiceDuration)
    def_string(uuid)
    def_blob(voiceData)
    def_int(translated)                 //是否翻译成功
end_msg()

begin_msg(notify_worldchat,8,19)
    def_int(senderId)
    def_int(senderSex)
    def_string(senderName)
    def_int(senderRoleType)     //发送者的角色类型，用于聊天时显示发送者的头像
    def_int(senderLvl)          //发送者等级
    def_string(message)
    def_int(sceneTemplateId) // 场景模板id
	def_int(honorId)		// 称号id, 用于世界聊天个性显示 by jin
    def_string(uuid)
    def_int(messageType)
    def_int64(voiceId)
    def_int(voiceDuration)
    def_int(createDate)
    def_int(translated)                 //是否翻译成功
end_msg()

// 系统消息通知
begin_msg(notify_systemchat,8,20)
    def_err(errorcode)
    def_string(message)
end_msg()


begin_msg(req_queryrole, 8, 21)
    def_string(rolename)
    def_int(type)
end_msg()

begin_msg(ack_queryrole, 8, 22)
    def_int(errorcode)
    def_string(rolename)
    def_object_arr(obj_roleinfo, roleinfos)
end_msg()

begin_msg(req_getTeamCopyData, 8, 23)
end_msg()

begin_msg(ack_getTeamCopyData, 8, 24)
    def_int(errorcode)
    def_int_arr(finishedCopys)
    def_int_arr(frightedfriends)
end_msg()

begin_msg(req_beginTeamCopy, 8, 25)
    def_int(copyid)
    def_int(friendid)
end_msg()

begin_msg(ack_beginTeamCopy, 8, 26)
    def_int(errorcode)
    def_int(copyid)
    def_object(obj_roleBattleProp, friendinfo)
    def_object(obj_petBattleProp, petinfo)
    def_object(obj_retinueBattleProp, retinueinfo)
end_msg()

begin_msg(notify_sys_reset_teamcopy, 8, 27)
end_msg()

begin_msg(notify_broadcast, 8, 29)
    def_int(type)
    def_string(content)
	def_int(noViewSystem)	// 显示在系统聊天框　０显示　１不显示
    def_int(createDate)
end_msg()

begin_msg(notify_forbitchat, 8, 30)
    def_int(starttime)
    def_int(endtime)
    def_string(reason)
end_msg()

begin_msg(notify_forbitrole, 8, 31)
    def_int(starttime)
    def_int(endtime)
    def_string(reason)
end_msg()

begin_msg(notify_forbidaccount, 8, 32)
    def_int(starttime)
    def_int(endtime)
    def_string(reason)
end_msg()


begin_msg(notify_sys_reset_printcopy, 8, 33)
end_msg()

begin_msg(notify_sys_reset_petcamp, 8, 34)
end_msg()

//发送世界消息返回错误码
begin_msg(ack_worldchat,8,35)
    def_int(errorcode)
    def_int(messageType)
    def_string(uuid)
end_msg()


begin_msg(req_private_chat_history,8,36)                        //旧的聊天消息
    def_int(start)//0开始
    def_int(num)
end_msg()

begin_msg(ack_private_chat_history,8,37)
    def_int(errorcode)
    def_int(have_offline_msg)   //0表示没有， 1表示有好友离线留言
    def_int(start)//0开始
    def_object_arr(obj_private_chat,chats)
end_msg()

//发送世界消息
begin_msg(req_system_chat, 8, 38)
	def_string(message)
end_msg()

begin_msg(ack_system_chat, 8, 39)
	def_int(errorcode)		// enSystemChatResult
end_msg()


begin_msg(req_scene_displayed_pets, 8, 40)
    
end_msg()

begin_msg(ack_scene_displayed_pets, 8, 41)
    def_object_arr(obj_pet_base_info, petinfos)
end_msg()

begin_msg(req_get_voice_chat_data, 8, 42)
    def_int(channel)
    def_int64(voiceId)
    def_string(uuid)
    def_int(roleId)     //请求哪个角色的roleId
end_msg()

begin_msg(ack_get_voice_chat_data, 8, 43)
    def_int(errorcode)
    def_int64(voiceId)
    def_blob(voiceData)
    def_int(channel)
    def_string(uuid)
    def_int(createDate)
    def_int(roleId)     //请求哪个角色的roleId
end_msg()

begin_msg(req_scene_displayed_retinues, 8, 44)

end_msg()

begin_msg(ack_scene_displayed_retinues, 8, 45)
def_object_arr(obj_Retinue_base_info, retinueinfos)
end_msg()

/******************************************************************************/

//获取星座系统状态
begin_msg(req_constellstate, 9, 1)
end_msg()

begin_msg(ack_constellstate, 9, 2)
    def_int(constellval)
    def_object_arr(obj_constellTimes, constellTimes)
    def_object_arr(obj_constelldata, constellpro)
end_msg()

//星座升级
begin_msg(req_constellupg, 9, 3)
    def_int(constellid)
end_msg()


begin_msg(ack_constellupg, 9, 4)
    def_err(errorcode)								// eConstellupgResult
    def_int(constellid)								// 星座ID
    def_int(constellstep)							// 星级
    def_int(constellval)							// 星灵
	def_object_arr(obj_constellTimes, constellTimes)
end_msg()


//////////////////////////  lvl玩家等级相关 ////////////////////////////
begin_msg(notify_lvl_up, 10, 1)
    def_int(lvl)
    def_int(exp)
end_msg()

begin_msg(syn_exp, 10, 2)
    def_int(exp)
end_msg()

begin_msg(notify_syn_gold, 10, 3)
    def_int(gold)
end_msg()

begin_msg(notify_syn_fat, 10, 4)
    def_int(fat)
end_msg()

begin_msg(notify_syn_batpoint, 10, 5)
    def_int(batpoint)
end_msg()

begin_msg(notify_syn_constellval, 10, 6)
    def_int(constval)
end_msg()

begin_msg(notify_syn_rmb, 10, 7)
    def_int(rmb)
end_msg()

begin_msg(notify_syn_elitetimes, 10, 8)
    def_int(times)
end_msg()

begin_msg(notify_syn_dungtimes, 10, 9)
    def_int(times)
end_msg()

begin_msg(notify_syn_pvptimes, 10, 10)
    def_int(times)
end_msg()

begin_msg(notify_syn_teamresettimes, 10, 11)
    def_int(times)
end_msg()

begin_msg(notify_syn_vipexp, 10, 12)
    def_int(vipexp)
end_msg()

begin_msg(notify_syn_viplvl, 10, 13)
    def_int(viplvl)
    def_int(vipaward)
end_msg()

begin_msg(notify_syn_viplvlaward, 10, 14)
    def_int(viplvlaward)
end_msg()

begin_msg(notify_syn_petelitetimes, 10, 15) // 同步幻兽试炼场次数
    def_int(times) // 剩余次数
end_msg()
begin_msg(notify_syn_enchantDust, 10, 16)   // 附魔尘, 普通的
def_int(enchantDust)
end_msg()
begin_msg(notify_syn_enchantGold, 10, 17)   // 附魔钻
def_int(enchantGold)
end_msg()

begin_msg(notify_syn_petadventuretimes, 10, 18)
    def_int(times)
end_msg()

begin_msg(notify_syn_petadventurerobotpetrenttimesremain, 10, 19)
    def_int(times)
end_msg()

begin_msg(notify_syn_cspvptimes, 10, 20) //跨服战通知协议
    def_int(times)
end_msg()
//////////////////////////  玩家挂机相关 ///////////////////////////////
begin_msg(req_hang_up,11, 1)
    def_int(copyId)
    def_int(hangTimes)
end_msg()


begin_msg(ack_hang_up,11, 2)
    def_int(errorCode)
    def_int(copyid)
    def_int(hangTimes)
    def_object_arr(obj_copyaward, copyawards)
end_msg()

////////////////////////////////
begin_msg(ack_freshprop_byday, 12, 1)
    def_int(dungbuytimes)
    def_int(elitebuytimes)
    def_int(fatbuytimes)
    def_int(pvpbuytimes)
    def_int(pvpaward)

    def_object_arr(obj_dungeonstate, dungeonstates)
    def_int(petadventurebuytimes)
    def_int(petadventurerobotpetbuyingrentedtimesperday)
    def_int(crossservicewarbuytimes)
    def_int(crossservicewarworshipbuytimes)


    def_int(dunglevel1)
    def_int(dunglevel2)
    def_int(dunglevel3)
    def_int(dungResetTimes1)
    def_int(dungResetTimes2)
    def_int(dungResetTimes3)
    def_int(dungMaxsweepingLevel1)
    def_int(dungMaxsweepingLevel2)
    def_int(dungMaxsweepingLevel3)

    def_int(moneytreetimes)
    def_int(freelottery)
    def_int(buylottery)
end_msg()


//////////////////////////// shoping ////////////////////////////

begin_msg(req_buy_fat, 13, 1)
end_msg()

begin_msg(ack_buy_fat, 13, 2)
    def_int(errorCode)
    def_int(costrmb)
    def_int(buyTime)
    def_int(costType)
end_msg()

begin_msg(req_buy_elitetimes, 13, 3)
    def_int(buytime)
end_msg()

begin_msg(ack_buy_elitetimes, 13, 4)
    def_int(errorCode)
    def_int(addtime)
    def_int(costrmb)
    def_int(buyTime)
    def_int(costType)
end_msg()

begin_msg(req_teamcopy_reset, 13, 5)
def_int(copyid)
end_msg()

begin_msg(ack_teamcopy_reset, 13, 6)
def_int(errorcode)
def_int(copyid)
def_int(costrmb)
def_int(costType)
end_msg()

begin_msg(req_dungcopy_reset, 13, 7)
    def_int(dungcopytype)
end_msg()

begin_msg(ack_dungcopy_reset, 13, 8)
    def_int(errorcode)
    def_int(index)
    def_int(cost)
    def_int(costType)
    def_object_arr(obj_dungeawawrd, awards)
end_msg()



//begin_msg(notify_syn_fatbuytimes, 13, 3)
//    def_int(buytimes)
//end_msg()

// 购买幻兽试炼场
begin_msg(req_buy_petelitetimes, 13, 9)
    def_int(buytime)    // 购买次数
end_msg()

begin_msg(ack_buy_petelitetimes, 13, 10)
    def_int(errorcode)
    def_int(addtime)
    def_int(costrmb)
    def_int(buyTime)
    def_int(costType)
end_msg()



begin_msg(req_dungcopy_sweeping,13,11)
     def_int(dungcopytype)
end_msg()

begin_msg(ack_dungcopy_sweeping, 13, 12)
    def_int(errorcode)
    def_int(index)
    def_int(sweepinglvl)
def_object_arr(obj_dungeawawrd, awards)
end_msg()

begin_msg(req_get_dungeon_state, 13, 14)
end_msg()

begin_msg(ack_get_dungeon_state, 13, 15)
    def_object_arr(obj_dungeonstate, dungeonstates)
end_msg()

begin_msg(notify_dungeon_state, 13, 16)
    def_object_arr(obj_dungeonstate, states)
end_msg()

//幻兽大冒险副本次数购买
begin_msg(req_buy_petadventuretimes, 13, 17)
    def_int(buytime)
end_msg()

begin_msg(ack_buy_petadventuretimes, 13, 18)
    def_int(errorCode)
    def_int(addtime)
    def_int(costrmb)
    def_int(buyTime)
    def_int(costType)
end_msg()

begin_msg(req_buy_petAdventureRobotPetRentedTimes, 13, 19)
    def_int(addRentTimes)
    def_int(petadventuretype)
end_msg()

begin_msg(ack_buy_petAdventureRobotPetRentedTimes, 13, 20)
    def_int(errorCode)
    def_int(addtime)
    def_int(costrmb)
    def_int(rentedtimes)
    def_int(costType)
end_msg()

//跨服战
begin_msg(req_buy_crossservicewartimes, 13, 21)
    def_int(buytime)
end_msg()

begin_msg(ack_buy_crossservicewartimes, 13, 22)
    def_int(errorCode)
    def_int(addtime)
    def_int(costrmb)
    def_int(buyTime)
    def_int(costType)
end_msg()

begin_msg(req_buy_crossservicewarworshiptimes, 13, 23)
    def_int(buytime)
end_msg()

begin_msg(ack_buy_crossservicewarworshiptimes, 13, 24)
    def_int(errorCode)
    def_int(addtime)
    def_int(costrmb)
    def_int(buyTime)
    def_int(costType)
end_msg()


////////////////////////////公会 始////////////////////////////////////////////////
begin_msg(req_guild_list, 14, 1)                            //全部公会列表
    def_int(beginrank)//从0开始
    def_int(num)
end_msg()

begin_msg(ack_guild_list, 14, 2)
    def_int(beginrank)
    def_object_arr(obj_guild_info, guilds)
end_msg()

begin_msg(req_create_guild, 14, 3)                          //创建公会
    def_string(guildname)
end_msg()

begin_msg(ack_create_guild, 14, 4)
    def_int(errorcode)			//CreateGuildResult
end_msg()

begin_msg(req_dismiss_guild, 14, 5)                         //解散公会
end_msg()

begin_msg(ack_dismiss_guild, 14, 6)
    def_int(errorcode)			// eDismissGuildResult
    def_int(dissmissdate)		//解散时间
end_msg()

begin_msg(req_cancle_dismiss_guild, 14, 7)                  //取消解散
end_msg()

begin_msg(ack_cancle_dismiss_guild, 14, 8)
    def_int(errorcode)//0成功 1失败
end_msg()

begin_msg(req_search_guild,14,9)                            //搜索公会
    def_string(guildname)
end_msg()

begin_msg(ack_search_guild,14,10)
    def_int(errorcode)//0成功 1失败
    def_object(obj_guild_info,guild)
end_msg()

begin_msg(req_modify_guild_notice,14,11)                    //修改公告
    def_string(notice)
end_msg()

begin_msg(ack_modify_guild_notice,14,12)
    def_int(errorcode)
end_msg()


begin_msg(req_transfer_guild_master,14,14)                  //转让会长
    def_int(memberid)
end_msg()

begin_msg(ack_transfer_guild_master,14,15)
	def_int(errorcode)	// eTransferMasterResult
end_msg()

begin_msg(notify_transfer_guild_master,14,16)
    def_int(oldmaster)
    def_int(newmaster)
end_msg()

begin_msg(req_cancle_transfer_master, 14, 17)                  //取消转让会长
end_msg()

begin_msg(ack_cancle_transfer_master, 14, 18)
	def_int(errorcode)	// eTransferMasterResult
end_msg()


begin_msg(req_guild_info, 14, 20)                           //公会信息
end_msg()

//这个很耗流量，有空再把它分开来发
begin_msg(notify_guild_info, 14, 21)
    def_int(guildid)                                        //0表示没公会
    def_string(guildname)                                   //公会名
    def_int(createdate)                                     //创建时间
    def_int(dissmissdate)                                   //解散时间
    def_int(membernum)                                      //人数
    def_int(master)                                         //会长
    def_string(mastername)
    def_string(notice)                                      //公告
    def_int(fortune)                                        //财富
    def_int(construction)                                   //建设度
    def_int(donatermbnum)                                   //日捐献金钻次数
    def_int(inspired)                                       //被鼓舞次数
	def_int(transferMasterDate)                             //转让会长时间
end_msg()


begin_msg(req_guild_member_list, 14, 22)                    //公会成员列表
    def_int(start)                                          //0开始
    def_int(num)
end_msg()

begin_msg(ack_guild_member_list, 14, 23)
    def_int(start)
    def_object_arr(obj_guild_member, members)
end_msg()

begin_msg(req_player_guild_detail, 14, 24)                  //个人公会属性
end_msg()

begin_msg(notify_player_guild_detail,14,25)
    def_int(guildid)                                        //0表示没公会
    def_object_arr(obj_guild_skill,skills)                  //角色公会技能
    def_int(constrib)                                       //公会贡献值
    def_int(exploit)                                        //功勋
    def_int(dnoateGlodBatNum)                               //日捐献战功、金币次数
    def_int(position)                                       //职位
    def_int(itemTrainBossTime)                              //今日道具培养boss次数
    def_int(rmbTrainBossTime)                               //今日金钻培养boss次数
    def_string(guildname)                                   //公会名
    def_int_arr(blessmate)                                  //祝福过的成员
    def_int(blessed)                                        //被祝福次数
	def_int(guildRank)										//公会排名
end_msg()

begin_msg(syn_player_guild_position,14,26)                  //职位同步
    def_int(position)
end_msg()

begin_msg(syn_player_guild_constrib,14,27)                  //贡献值
    def_int(constrib)
end_msg()

begin_msg(notify_guild_constrib,14,28)                //垃圾贡献值
    def_int(constrib)
end_msg()

// 公会申请
begin_msg(req_role_apply_guild_list,14,35)                  //玩家申请的公会列表
end_msg()

begin_msg(ack_role_apply_guild_list,14,36)
    def_int_arr(guilds)
end_msg()

begin_msg(req_guild_applicant_list, 14, 37)                 //公会被申请列表
    def_int(start)//从0开始
    def_int(num)
end_msg()

begin_msg(ack_guild_applicant_list, 14,38)
    def_int(start)
    def_object_arr(obj_guild_applicant, applicants)
end_msg()

begin_msg(notify_delete_apply,14,39)                        //同步删除申请信息
    def_int(guildid)
    def_int(roleid)
end_msg()


begin_msg(notify_new_applicant, 14, 43)                     //通知管理员有新的申请了
end_msg()


begin_msg(req_apply_guild, 14, 44)                          //申请加入公会
    def_int(guildid)
end_msg()

begin_msg(ack_apply_guild, 14, 45)
    def_int(errorcode)//eApplyGuild
    def_int(guildid)
end_msg()

begin_msg(req_accept_apply_guild, 14, 46)                   //管理员接受申请
    def_int(applicantid)
    def_string(applicantname)
end_msg()

begin_msg(ack_accept_apply_guild, 14, 47)
    def_int(errorcode)//eGuildMgrAcceptApply
    def_int(applicantid)
    def_string(applicantname)
end_msg()

begin_msg(notify_guild_new_member,14,48)                    //通知新会员加入
    def_int(guildid)
    def_string(guildname)
    def_int(memberid)
    def_string(membername)
end_msg()

begin_msg(req_reject_apply_guild, 14, 49)                   //管理员拒绝申请
    def_int(applicantid)
    def_string(applicantname)
end_msg()

begin_msg(ack_reject_apply_guild, 14, 50)
    def_int(errorcode)//eGuildMgrRejectApply
    def_int(applicantid)
    def_string(applicantname)
end_msg()

begin_msg(notify_reject_apply_for_applicant, 14, 51)        //通知玩家被公会拒绝
    def_int(guildid)
    def_string(guildname)
end_msg()


begin_msg(req_guild_leave, 14, 52)                          //离开公会
end_msg()

begin_msg(ack_guild_leave, 14, 53)
    def_int(errorcode) //0成功 1失败
end_msg()

begin_msg(notify_guild_leave,14,54)                         //通知有成员离开
    def_int(roleid)//如果roleid 是自己，说明自己离开，如果是其它人，就是成员有离开
    def_string(rolename)
    def_int(guildid)
    def_string(guildname)
end_msg()

begin_msg(req_guild_kick, 14, 55)                           //踢人
    def_int(memberid)
end_msg()

begin_msg(ack_guild_kick, 14, 56)
    def_int(errorcode)										//eGuildKickMember
    def_int(memberid)
end_msg()

begin_msg(req_cancel_apply_guild, 14, 57)                          //　取消加入公会
	def_int(guildid)
end_msg()

begin_msg(ack_cancel_apply_guild, 14, 58)                          //　取消加入公会
	def_int(errorcode) //0成功 1失败
	def_int(guildid)
end_msg()

begin_msg(req_guild_donate,14,60)                           //捐献公会
    def_int(type)//GuildDonateType
end_msg()

begin_msg(ack_guild_donate,14,61)
    def_int(errorcode)//eGuildDonateResult
    def_int(type)
end_msg()

begin_msg(req_guild_log,14,70)                              //公会日志
    def_int(start)//0开始
    def_int(num)
end_msg()

begin_msg(ack_guild_log,14,71)
    def_int(start)//0开始
    def_object_arr(obj_guild_log_info,logs)
end_msg()

begin_msg(req_upgrade_guild_skill,14,76)                    //公会技能
    def_int(skillid)//440200这种不用带等级
end_msg()

begin_msg(ack_upgrade_guild_skill,14,76)                    
    def_int(errcode)//0成功 1失败
    def_int(skillid)
    def_int(newlevel)
end_msg()

begin_msg(req_guild_chat,14,78)                             //公会聊天
    def_string(msg)
    def_int(messageType)
    def_int(time)
    def_string(uuid)
    def_blob(voiceData)
    def_int(voiceDuration)
    def_int(translated)                 //是否翻译成功
end_msg()

begin_msg(notify_guild_chat,14,78)
    def_int(senderRoleid)
    def_int(senderRoleType)
    def_string(senderRoleName)
    def_int(senderRoleLvl)

    def_string(msg)
    def_string(uuid)
    def_int(messageType)
    def_int64(voiceid)
    def_int(voiceDuration)
    def_int(createDate)
    def_int(translated)                 //是否翻译成功
end_msg()

begin_msg(req_past_guild_chat,14,79)                        //旧的聊天消息
    def_int(start)//0开始
    def_int(num)
end_msg()

begin_msg(ack_past_guild_chat,14,80)
    def_int(start)//0开始
    def_object_arr(obj_guild_chat,chats)
end_msg()


//公会商店

// 商品列表
begin_msg(req_guild_store, 14, 81)
end_msg()

begin_msg(ack_guild_store, 14, 82)
	def_int(errorcode)                                // StoreListReuslt
	def_int(nextTm)                                   // 下次更新倒计时（秒）
	def_int(updateNum)                                // 已手动更新次数
	def_object_arr(obj_store_goods_info, info_arr)    // 商品列表
end_msg()

// 刷新商品列表
begin_msg(req_guild_storeUpdate, 14, 83)
end_msg()

begin_msg(ack_guild_storeUpdate, 14, 84)
	def_int(errorcode)                              // StoreManuallyListReuslt
	def_int(costrmb)                                // 消耗金钻
	def_int(updateNum)                              // 已手动更新次数
end_msg()

// 购买商品
begin_msg(req_guild_storeBuy, 14, 85)
	def_int(indexId)                                // 索引ID
end_msg()

begin_msg(ack_guild_storeBuy, 14, 86)
	def_int(errorcode)                              // StoreBuyReuslt
	def_int(cost)                                   // 消耗资源
	def_int(costType)                               // 消耗类型
	def_int(indexId)                                // 索引ID
end_msg()

// 公会boss
begin_msg(req_guild_boss_exp,14,90)                 //公会boss经验
end_msg()

begin_msg(nty_guild_boss_exp,14,91)
    def_int(exp)
end_msg()

begin_msg(req_train_guild_boss,14,92)               //培养公会boss
    def_int(train_type)                             //eTrainGuildBossType
    def_int_arr(items)                              //道具id
    def_int(rmb)
end_msg()

begin_msg(ack_train_guild_boss,14,93)
    def_int(err)                                    //eTrainGuildBoss
    def_int(train_exp)                              //喂养成功，增加的个人公会经验值
end_msg()


begin_msg(req_my_guild_rank,14,100)                 // 取本公会排名
end_msg()

begin_msg(ack_my_guild_rank,14,101)
    def_int(rank)                                   // 从0开始
end_msg()

begin_msg(req_guild_bless,14,110)                   // 祝福
    def_int( target )                               // 祝福对象
end_msg()

begin_msg(ack_guild_bless,14,111)
    def_int( errorcode )                            // eGuildBless
    def_int( target )                               // 祝福对象
end_msg()

begin_msg(req_guild_inspire,14,112)                 // 鼓舞
end_msg()

begin_msg(ack_guild_inspire,14,113)
    def_int( errorcode )                            // eGuildInspire
end_msg()

begin_msg(req_guild_impeach_info, 14, 115)  //获取弹劾信息
end_msg()

begin_msg(ack_guild_impeach_info, 14, 116)
    def_int(errorcode)      //eGuildImpeachInfoError_Error
    def_string(applicant_name)
    def_int(applicant_contrib)
end_msg()

begin_msg(req_guild_impeach, 14, 117)   //会长弹劾
end_msg()

begin_msg(ack_guild_impeach, 14, 118)
def_int(errorcode)      //eGuildImpeachError_Error
end_msg()

// 指派职位
begin_msg(req_guild_appoint_position, 14, 119)
	def_int(roleId)
	def_int(pos)	// GuildMemberPosition	职位
end_msg()

begin_msg(ack_guild_appoint_position, 14, 120)
	def_int(errorcode)      // eGuildAppointPosition
end_msg()

begin_msg(syn_player_guild_rank,14,121)                  //排名同步
	def_int(rank)
end_msg()

// 商店占城特权
begin_msg(req_guild_occupy_store_list, 14, 122)
end_msg()

begin_msg(ack_guild_occupy_store_list, 14, 123)
	def_int(point);
	def_object_arr(obj_occupy_store_goods_info, info_arr)    // 商品列表
end_msg()

begin_msg(req_guild_occupy_store_buy, 14, 124)
	def_int(index);
	def_int(num);
end_msg()

begin_msg(ack_guild_occupy_store_buy, 14, 125)
	def_int(errorcode)      // eGuildOccupyStoreResult
end_msg()
////////////////////////////公会 末//////////////////////////////////////////


//////////////////////////// pvp /////////////////////////////

begin_msg(req_getPvpRank, 15, 1)
    def_int(getalldata)
end_msg()

begin_msg(ack_getPvpRank, 15, 2)
    def_int(errorcode)
    def_int(getalldata)
    def_int(myrank)
    def_int(getaward)
    def_int(awardrank)
    def_int(restTime)
    def_int(costperminute)
    def_object_arr(obj_pvp_role, roles)
    def_object_arr(obj_pvp_log, logs)
    def_int(pointsGetPeriod)    //当前排名每段时间可以得到的积分
end_msg()

begin_msg(req_begin_pvp, 15, 3)
    def_int(bechallengeId)
    def_int(objType)                //0是真实角色， 1是机器人
    def_int(bechallengeRank)
    def_int(sceneid)
end_msg()

begin_msg(ack_begin_pvp, 15, 4)
    def_int(errorcode)
    def_int(sceneid)
    def_object(obj_roleBattleProp, enemyinfo)
    def_int(targetType)            //0是角色，1是机器人
    def_object(obj_petBattleProp, enemypetinfo)
    def_object(obj_retinueBattleProp, enemyretinueinfo)
    def_int(selfMaxhp)
    def_int(selfatk)
    def_int(selfdef)
    def_int(selfhit)
    def_int(selfdodge)
    def_int(selfcri)
end_msg()

begin_msg(req_PvpResult, 15, 5)
    def_int(iswin)
    def_int(bechallengeId)
end_msg()

begin_msg(ack_PvpResult, 15, 6)
    def_int(errorcode)
    def_int(cooltime)
    def_int(isWin)
    def_string(award)
end_msg()

begin_msg(req_getPvpAward, 15, 7)
end_msg()

begin_msg(ack_getPvpAward, 15, 8)
    def_int(errorcode)
    def_int(awardrank)
    def_string(awardItem)
end_msg()

begin_msg(req_buy_pvptimes, 15, 9)
end_msg()

begin_msg(ack_buy_pvptimes, 15, 10)
    def_int(errorcode)
    def_int(costrmb)
    def_int(buytimes)
    def_int(costType)
end_msg()

begin_msg(req_fresh_pvp_immediately, 15, 11)
end_msg()

begin_msg(ack_fresh_pvp_immediately, 15, 12)
    def_int(errorcode)
    def_int(cost)
    def_int(costType)
end_msg()

begin_msg(req_pvp_point_exchange, 15, 13)
    def_int(itemIndex)
    def_int(itemCount)
end_msg()

begin_msg(ack_pvp_point_exchange, 15, 14)
    def_int(errorcode)
    def_int(itemCount)
    def_int(myPvpPoint)
end_msg()

begin_msg(req_pvp_exchange_itemlist, 15, 15)
end_msg();

begin_msg(ack_pvp_exchange_itemlist, 15, 16)
    def_int(myPvpPoint)
    def_object_arr(obj_pvp_exchange_item, items)
end_msg()

begin_msg(req_pvp_point, 15, 17)
end_msg()

begin_msg(ack_pvp_point, 15, 18)
	def_int(pvpPoint)
end_msg()
/////////////////////////////////////////////////

//宠物列表
begin_msg(req_list_pet, 16, 1)

end_msg()

begin_msg(ack_list_pet, 16, 2)
    def_int(errorcode)
    def_object_arr(obj_petinfo, petinfos)
end_msg()


//宠物出战
begin_msg(req_use_pet, 16, 3)
    def_int(petid)
end_msg()

begin_msg(ack_use_pet, 16, 4)
    def_int(errorcode)
    def_int(petid)
end_msg()


//召回宠物
begin_msg(req_recall_pet, 16, 5)
    def_int(petid)
end_msg()


begin_msg(ack_recall_pet, 16, 6)
    def_int(errorcode)
    def_int(petid)
end_msg()


//吞噬宠物
begin_msg(req_absorb_pet, 16, 7)
    def_int(petid)
    def_int_arr(absorbedid)
end_msg()


begin_msg(ack_absorb_pet, 16, 8)
    def_int(errorcode)          //enPetAbsorbRetsult
    def_int(petid)
    def_int_arr(absorbedid)
end_msg()

//宠物经验等级更新
begin_msg(notify_sync_pet_exp, 16, 9)
    def_int(petid)
    def_int(lvl)
    def_int(exp)
end_msg()

//通知获得宠物
begin_msg(notify_get_pet, 16, 10)
    def_object_arr(obj_petinfo, petsinfo)
end_msg()

//宠物属性更新
begin_msg(notify_sync_pet_prop, 16, 11)
    def_int(petid)
    def_int(mPhys)
    def_int(mCapa)
    def_int(mInte)
    def_int(mStre)
end_msg()

//宠物星级更新
begin_msg(notify_sync_pet_star , 16,12)
    def_int(petid)
    def_int(star)
    def_int(starlvl)
end_msg()




// 图鉴列表
begin_msg(req_illustrations_list, 16, 13)
    def_int(type)               // IllustrationsType
end_msg()

begin_msg(ack_illustrations_list, 16, 14)
    def_object_arr(obj_illustrationsInfo, info_arr)
end_msg()

// 获取图鉴奖励
begin_msg(req_illustrations_reward, 16, 15)
    def_int(illId)              // 图鉴ID
    def_int(qua)                // IllustrationsQuality
end_msg()

begin_msg(ack_illustrations_reward, 16, 16)
    def_int(illId)              // 图鉴ID
    def_int(qua)                // IllustrationsQuality
    def_int(errorcode)          // enIllustrationsRewardReuslt
end_msg()

// 图鉴奖励消息
begin_msg(notify_illustrations_reward, 16, 17)
    def_int(illId)              // 图鉴ID
    def_int(qua)                // IllustrationsQuality
end_msg()


begin_msg(req_assist_pet_list, 16, 20)
end_msg()

begin_msg(ack_assist_pet_list, 16, 21)
    def_int(errorcode)
    def_int_arr(assistPets)
end_msg()

begin_msg(req_set_assist_pet, 16, 22)
    def_int(petid)
    def_int(pos)
end_msg()

begin_msg(ack_set_assist_pet, 16, 23)
    def_int(errorcode)
    def_int(petid)
    def_int(pos)
end_msg()

begin_msg(req_get_pet_data, 16, 24)
    def_int(petid)
    def_int(serverid)
end_msg()

begin_msg(ack_get_pet_data, 16, 25)
    def_int(errorcode)
    def_int(owner)
    def_object(obj_check_pet_prop, petinfo)

    def_int(onReqServerid)
end_msg()

//宠物升阶更新
begin_msg(notify_sync_pet_stage , 16,26)
    def_int(petid)
    def_int(stage)
end_msg()


//宠物使用技能书 激活或者升级技能
begin_msg(req_pet_study_skill,16,30)
    def_int(petid)          //宠
    def_int(skillid)        //技能ID
    def_int_arr(bookid)         //技能书
end_msg()

begin_msg(ack_pet_study_skill, 16, 31)
    def_int(errcode)        //ePetStudySkill
    def_int(petid)
    def_int(skillid)
    def_int(failtimes)         // 技能升级失败次数
end_msg()

// 宠物吃成长丸洗练成长值
begin_msg(req_pet_refine_growth,16,32)
    def_int( petid )
end_msg()

begin_msg(ack_pet_refine_growth,16,33)
    def_int(errcode)    //ePetRefineGrowth
    def_int(petid)
    def_int(growth)     //成长值
end_msg()

//宠物合并功能
begin_msg(req_pet_merge, 16, 34)
    def_int(petid)      //  主宠物的id
    def_int_arr(merge_petid) //要合并的其他宠物id
end_msg()

begin_msg(ack_pet_merge, 16, 35)
    def_int(errorcode)      //ePetMergeRetsult
    def_int(petid)
    def_int_arr(merge_petid)
end_msg()

begin_msg(notify_sync_pet_mergelvl, 16, 36)
    def_int(petid)          //宠物ID
    def_int(merge_lvl)      //宠物阶级
    def_int(current_exp)    //当前经验
    def_int(extlvladd)      //等级差增加值
    def_int(maxlvladd)      //等级上限增加值
    def_int(growthadd)      //成长值上限增加值
end_msg()

begin_msg(req_pet_unlock_skill, 16, 37)
def_int(petid)          //宠
def_int(skillid)        //技能ID
def_int_arr(bookid)         //技能书
end_msg()

begin_msg(ack_pet_unlock_skill, 16, 38)
def_int(errcode)        // ePetUnlockSkill
def_int(petid)
def_int(skillid)
end_msg()

//通知删掉宠物
begin_msg(notify_delete_pet, 16, 39)
    def_int_arr(pets)
end_msg()

//幻兽进化相关
begin_msg(req_petevolution_begin_evolution, 16, 40)
    def_int(firstpetid)
    def_int(secondpetid)
    def_int(targetpetmod)
end_msg()

begin_msg(ack_petevolution_begin_evolution, 16, 41)
    def_int(errorcode)
    def_int(petid)
end_msg()

begin_msg(req_set_pet_unnew, 16, 42)
    def_int(petid)
end_msg()

// 获取幻兽技能升级概率
begin_msg(req_pet_studyskill_prob, 16, 43)
    def_int(petid)              // 幻兽id
    def_int(skillid)            // 技能id
    def_int_arr(bookid)         // 技能书
end_msg()
begin_msg(ack_pet_studyskill_prob, 16, 44)
    def_int(petid)
    def_int(skillid)
    def_int(failtimes)         // 技能升级失败次数
end_msg()


//幻兽吞噬锁
begin_msg(req_set_petswallowlock, 16, 45)
def_int(petid)
end_msg()

begin_msg(ack_set_petswallowlock, 16, 46)
def_int(errorcode)
def_int(isLocked)
def_int(petid)
end_msg()

/*******************************************************/

//地下城结算
begin_msg(req_finish_dungecopy, 17, 1)
    def_int(result)
    def_int(sceneid)
    def_int(deadMonsterNum)
end_msg()

begin_msg(ack_finish_dungecopy, 17, 2)
    def_int(errorcode)
    def_int(result)
    def_int(sceneid)
    def_object_arr(obj_dungeawawrd, awards)
end_msg()

begin_msg(req_finish_friend_dunge, 17, 3)
    def_int(result)
    def_int(sceneid)
    def_int(deadMonsterNum)
end_msg()


begin_msg(ack_finish_friend_dunge, 17, 4)
    def_int(errorcode)
    def_int(result)
    def_int(sceneid)
    def_object_arr(obj_dungeawawrd, awards)
end_msg()

begin_msg(req_getfrienddunge_data, 17, 5)
end_msg()

begin_msg(ack_getfrienddunge_data, 17, 6)
    def_int(errorcode)
    def_int(leftTimes)
    def_int_arr(frightedfriends)
    def_object_arr(obj_friendDunge_record, record)          //每个等级的记录，如果没有，表示该等级没打过
    def_int(friend_lasttime)
end_msg()

begin_msg(req_begin_friend_dunge, 17, 7)
    def_int(sceneid)
    def_int(friendid)
end_msg()

begin_msg(ack_begin_friend_dunge, 17, 8)
    def_int(errorcode)
    def_int(copyid)
    def_object(obj_roleBattleProp, friendinfo)
    def_object(obj_petBattleProp, petinfo)
    def_object(obj_retinueBattleProp, retinueinfo)
end_msg()

//独立处理地下城进入条件的判断
begin_msg(req_begin_normal_dunge, 17, 9)
    def_int(sceneid)
end_msg()

begin_msg(ack_begin_normal_dunge, 17, 10)
    def_int(errorcode)
    def_int(sceneid)
end_msg()

begin_msg(req_friendDunge_reset, 17, 11)
    def_int(index)
end_msg()

begin_msg(ack_friendDunge_reset, 17, 12)
    def_int(errorcode)
    def_object_arr(obj_friendDunge_record, record)          //每个等级的记录，如果没有，表示该等级没打过
end_msg()

//幻兽大冒险进入条件判断
begin_msg(req_begin_petAdventure, 17, 13)
    def_int(sceneid)
    def_int(petid)      //租用幻兽的时候这个值是幻兽模板Id,使用玩家幻兽列表的时候这个值是数据库保存的幻兽id
    def_int(oncemore)   //0:不是再来一次, 1:再来一次
    def_int(isrented)   //0:没有租借,    1:租借幻兽

end_msg()

begin_msg(ack_begin_petAdventure, 17, 14)
    def_int(errorcode)
    def_int(sceneid)
    def_int(oncemore)
end_msg()

//完成幻兽大冒险副本
begin_msg(req_finish_petAdventure, 17, 15)
    def_int(sceneid)
    def_int(isrented)
    def_int(petid)
    def_int(star)
    def_int(deadMonsterNum) //死亡的小怪数
    def_int(finishstate)    //0:挑战失败 1:通过
end_msg()

begin_msg(ack_finish_petAdventure, 17, 16)
    def_int(errorcode) //1:成功 0:失败
end_msg()

//获取临时副本所同步的服务器时间
begin_msg(req_petadventure_temporaryactcopy_time, 17, 17)
end_msg()

begin_msg(ack_petadventure_temporaryactcopy_time, 17, 18)
    def_int(errorcode) //1:成功 0:失败
    def_int(nowTime)
end_msg()


/***********************************************************************/

//苹果充值
begin_msg(req_apple_recharge,18,1)
    def_int(serverid)                       //服务器id
    def_int(roleid)
    def_string(receipt_data)                // base64 encode 苹果小票
	def_string(coin_type)					// 货币类型
end_msg()

begin_msg(ack_apple_recharge,18,2)
    def_int(result)                         //enAppleRechargeRet
    def_int(roleid)
    def_int(rmb)                            //金钻
    def_string(receipt_data)                //原单
end_msg()

//充值
begin_msg(req_pps_recharge, 18, 11)
    def_int(itemid)     //充值id
    def_int(num)        //购买数量
end_msg()

begin_msg(ack_pps_recharge, 18, 12)
    def_int(errorcode)
    def_string(billno)
    def_int(zoneid)
    def_int(ppcoin)
end_msg()


begin_msg(req_tb_recharge, 18, 16)

end_msg()

begin_msg(ack_tb_recharge, 18, 17)

end_msg()


begin_msg(notify_recharge_success, 18, 18)
    def_string(billno)
    def_int(rmb)
end_msg()

begin_msg(req_recharge_360, 18, 19)

end_msg()

begin_msg(ack_recharge_360, 18, 20)
    def_int(errorcode)
    def_string(accesstoken)
    def_string(notifyurl)
end_msg()

//　申请订单号
begin_msg(req_register_order, 18, 21)
	def_int(channelId)			// 接入平台对应的ID, 服务端提供
	def_float(amount)			// 订单金额 单位(分) 不能指定金额填0
end_msg()

begin_msg(ack_register_order, 18, 22)
	def_int(errorcode)			// 错误定义看枚举 enRegisterOrderReuslt 
	def_string(orderId)			// 订单号
	def_float(amount)			// 订单金额 单位(分)
end_msg()

//获取请求ID
begin_msg(req_get_request_id, 18, 23)
end_msg()

begin_msg(ack_get_request_id, 18, 24)
    def_int(errorcode)      
    def_string(id)  //  请求的ID
end_msg()

// google充值
begin_msg(req_google_pay, 18, 25)
	def_string(data)
	def_string(sign)
	def_string(packageName) // 包名, 不对的话, 充值会出问题
end_msg()

begin_msg(ack_google_pay, 18, 26)
	def_int(errorcode) // 0为正常
end_msg()

// google充值_客户端本地充值信息
begin_msg(req_google_pay_localinfo, 18, 27)
	def_string(data)
	def_string(sign)
end_msg()

begin_msg(ack_google_pay_localinfo, 18, 28)
	def_int(errorcode) // 0为正常
end_msg()

// winner ios 充值
begin_msg(req_winner_ios_pay, 18, 29)
	def_string(sn_pin)
	def_string(price)
end_msg()

begin_msg(ack_winner_ios_pay, 18, 30)
	def_int(errorcode) // 0为正常
end_msg()

// winner Google Wallet 充值
begin_msg(req_winner_google_wallet_pay, 18, 31)
def_string(gg_data)
def_string(gg_sign)
end_msg()

begin_msg(ack_winner_google_wallet_pay, 18, 32)
def_int(errorcode) // 0为正常
end_msg()

// winner fortumo 充值
begin_msg(req_winner_fortumo_pay, 18, 33)
def_string(payment_code)
def_string(price)
def_string(currency)
def_string(point)
end_msg()

begin_msg(ack_winner_fortumo_pay, 18, 34)
def_int(errorcode) // enAppleRechargeRet
end_msg()

begin_msg(req_logout_winner, 18, 35)
def_string(clientMac)
end_msg()

begin_msg(ack_logout_winner, 18, 36)
def_int(errorcode)   // 返回0表示成功
end_msg()
/***********************************************************************/

//排行榜
begin_msg(req_getPaihangData, 19, 1)
    def_int(paihangType)
    def_int(job)
    def_int(isCrossService) //1表示拿跨服榜0表示拿当前服榜
    def_int(beginRank)
    def_int(endRank)
end_msg()


begin_msg(ack_getPaihangData, 19, 2)
    def_int(paihangType)
    def_int(job)
    def_int(beginRank)
    def_int(endRank)
    def_object_arr(obj_paihangdata, data)
    def_int(isCrossService) //1表示拿跨服榜0表示拿当前服榜
    def_int(mypvp)
    def_int(myRank)
    def_int(rankListIsOpened)
end_msg()

begin_msg(ack_getPetPaihangData, 19, 3)
    def_int(paihangType)
    def_int(job)
    def_int(beginRank)
    def_int(endRank)
    def_object_arr(obj_Petpaihangdata, data)
    def_int(isCrossService) //1表示拿跨服榜0表示拿当前服榜
    def_int(myPetRank)
    def_int(petmod)
end_msg()

begin_msg(notify_sync_fresh_paihang, 19, 4)
end_msg()

begin_msg(req_checkCrossServiceRankListOpen, 19, 5)
    def_int_arr(types)
end_msg()

begin_msg(ack_checkCrossServiceRankListOpen, 19, 6)
    def_int_arr(results)
end_msg()

begin_msg(req_crossServiceRankListTopThreeFashion, 19, 7)
    def_object_arr(obj_crossServiceRoleFashion_info, roleFashionInfos)
    def_int(rankListType)
end_msg()

begin_msg(ack_crossServiceRankListTopThreeFashion, 19, 8)
    def_object_arr(obj_crossServiceRoleFashion_info, roleFashionInfos)
    def_int(rankListType)
end_msg()

begin_msg(notify_sync_fresh_crossServiceRankList, 19, 9)
end_msg()

///////////////////防外挂相关验证////////////////////////////////---linshsuen
//服务器认为客户端有开挂
begin_msg(ack_client_cheat, 20, 1)
    def_int(errorcode)
    def_string(info)
end_msg()

//旧的角色属性验证
begin_msg(req_old_verify_role_prop, 20, 2)
    def_int(roleID)         //角色id
    def_int(rolelvl)
    def_int(verifyType)     //验证的类型(时间点) 参看枚举定义：VerifyPropType
    def_int(verifyHP)       //更改的血量--如果是刚进入副本，就是满血。如果战斗中过段，是更改的血量。
    def_int(atk)            //攻击
    def_int(def)            //防御
    def_float(hit)          //命中
    def_float(dodge)        //闪避
    def_float(cri)          //暴击
    def_int(roleBattle)      //角色战斗力
    def_int(petBattle)       //宠物的战斗力
end_msg()

//验证角色对场景中怪物的平均普通攻击值
begin_msg(req_verify_role_atk_monsters, 20, 3)
    def_object_arr(obj_monster_average_hurt, monsterHurts)      //普通攻击的平均伤害
    def_object_arr(obj_monster_average_hurt, monsterCriHurts)   //暴击的平均伤害
end_msg()


//技能或者伤害的验证
begin_msg(req_verify_skill_hurt, 20, 4)
def_int(skillType)                                       //伤害类型（技能 1 或普攻 0）
def_int(skillID)                                         //技能ID
def_object(obj_type_and_id, caster)                      //释放者（ObjType 和 ID）
def_object_arr(obj_skill_effect_info,casterSkilleffectList)                         //释放者身上的技能id列表
def_object(obj_current_battleprop_info, casterProp)      //释放者的当前属性（加buff后的6个核心属性）
def_object(obj_type_and_id, hurter)                      //受害者（ObjType 和 ID
def_object_arr(obj_skill_effect_info,hurterSkilleffectList) //受害者身上的技能id效果列表
def_object(obj_current_battleprop_info, hurterProp)       //受害者的当前属性（加buff后的6个核心属性)
def_int(hurtType)                                          //是否为暴击 1 暴击，0 普通攻击
def_int(hurtNumber)                                        //技能造成的伤害数值
end_msg()

//心跳包，暂时不用传递数据---linshusen
begin_msg(req_client_heart_beat, 20, 5)
    def_int(clientTime)         // 客户端当前时间秒
end_msg()

begin_msg(syn_server_time, 20,6) // 同步时间
    def_int(time)                // 服务器当前时间
end_msg()

//新的角色属性验证
begin_msg(req_verify_role_prop, 20, 7)
    def_object(obj_role_verify_info, mainrole)
    def_object(obj_role_verify_info, other)
end_msg()

//告诉服务器客户端有内存修改作弊
begin_msg(req_client_memory_cheat, 20, 8)
    def_string(fileName)
    def_string(functionName)
end_msg()

//告诉服务器客户端有修改配置文件作弊
begin_msg(req_client_config_cheat, 20, 9)
def_string(fileName)
end_msg()


//客户端触发了 触发技能 告诉服务器

begin_msg(notify_role_release_triggerskill, 20, 10)
def_int(roleid)
def_int(skillid)
end_msg()

begin_msg(notify_role_stop_triggerskill, 20, 11)
def_int(roleid)
def_int(skillid)
end_msg()



///////////////////邮件////////////////////////////////
begin_msg(req_send_mail, 21, 1)
end_msg()
begin_msg(ack_send_mail, 21, 2)

end_msg()

begin_msg(req_get_mail_list, 21, 3)
    def_int(from)
    def_int(count)
end_msg()
begin_msg(ack_get_mail_list, 21, 4)
    def_int(errorcode)
    def_int(from)
    def_int(count)
    def_object_arr(obj_mail, mails)
end_msg()

begin_msg(req_get_mail, 21, 5)
    def_string(mailid)
    def_string(mailtitle)
end_msg()

begin_msg(ack_get_mail, 21, 6)
    def_int(errorcode)
    def_string(mailid)
    def_string(content)
    def_string(attach)
end_msg()

begin_msg(req_delete_mail, 21, 7)
    def_string(mailid)
end_msg()
begin_msg(ack_delete_mail, 21, 8)
    def_int(errorcode)
    def_string(mailid)
end_msg()

begin_msg(req_get_attach, 21, 9)
    def_string(mailid)
    def_string(attach)
    def_string(mailtitle)
end_msg()
begin_msg(ack_get_attach, 21, 10)
    def_int(errorcode)
    def_string(mailid)
end_msg()

begin_msg(notify_new_mail, 21, 11)
    def_object(obj_mail, mail)
end_msg()

///////////////////////////////// 活动状态/////////////////////////////////////////////
begin_msg(req_get_act_stats, 22, 1)

end_msg()

begin_msg(ack_get_act_stats, 22, 2)
def_int(servertime)
def_object_arr(obj_actstat, stats)
end_msg()

begin_msg(ack_update_act_stats, 22, 3)
def_int(actid)
def_int(isopen)
end_msg()

//**********************************   幻兽大比拼(PetCasino)   ***********************************//


begin_msg(req_activity_pet_casino_status, 23, 1)       //当前赌场台面状态
end_msg()

//history格式,  id1:gmb;id2:gmb;...;id8:gmb;
begin_msg(ack_activity_pet_casino_status,23,2)
    def_int(status)                           //enPetCasinoStatus
    def_object_arr(obj_casino_pet,allpets)    //全部参赛宠
    def_string_arr(history)                   //历史，最后一条是当前可押宠物列表
    def_int(thief)                            //1 可用小偷
    def_int(petid)                            //你押的宠
    def_int(wager)                            //你的赌注,金币
    def_int_arr(matchtime)                    //全部时间，倒计时
end_msg()

begin_msg(req_activity_pet_casino_wager,23,3) //下注
    def_int(petid)                            //宠物
    def_int(wager)                            //赌注
    def_int(wager_type)                       //CostType
end_msg()

begin_msg(ack_activity_pet_casino_wager,23,4) //下注结果
    def_int(errorcode)                        //enPetCasinoWagerResult
    def_int(petid)                            //你的赌宠
    def_int(wager)                            //你的赌注,金币
end_msg()

begin_msg(req_activity_pet_casino_thief,23,5) //使用小偷之手
end_msg()

begin_msg(ack_activity_pet_casino_thief,23,6)
    def_int(errorcode)                        //0成功,1没钱,2其它
end_msg()

begin_msg(req_activity_pet_casino_current_history,23,7) //请求当前轮押注情况
end_msg()

begin_msg(ack_activity_pet_casino_current_history,23,8)
    def_string(history)
end_msg()

// 神秘商人列表
begin_msg(req_mysterious_list, 23, 9)
end_msg()

begin_msg(ack_mysterious_list, 23, 10)
    def_int(errorcode)                              // enMysteriousListReuslt
    def_int(nextTm)                                 // 下次更新倒计时(秒)
    def_int(updateNum)                              // 已手动更新次数(金钻)
	def_int(updateGoldNum)                          // 已手动更新次数(金币)
	def_int(time)									// 当前时间戳
    def_object_arr(obj_mysteriousInfo, info_arr)    // 商品列表
end_msg()

// 手动刷新神秘商人列表
begin_msg(req_upMysterious_list, 23, 11)
	def_int(type)	// enStoreGoodsType
end_msg()

begin_msg(ack_upMysterious_list, 23, 12)
    def_int(errorcode)                              // enManuallyMysteriousReuslt
	def_int(updateNum)                              // 已手动更新次数(金钻)
	def_int(updateGoldNum)                          // 已手动更新次数(金币)
end_msg()

// 购买神秘商人物品
begin_msg(req_buyMysterious_goods, 23, 13)
    def_int(indexId)                                // 索引ID
	def_int(time)									// 当前时间戳
end_msg()

begin_msg(ack_buyMysterious_goods, 23, 14)
    def_int(errorcode)                              // enBuyMysteriousReuslt
    def_int(indexId)                                // 索引ID
end_msg()

// 购买神秘商店特卖商品
begin_msg(req_buyMysterious_special_goods, 23, 15)
	def_int(indexId)                                // 索引ID
	def_int(goodsId)                                // 物品ID
    def_int(goodsNum)                               // 批量购买的物品数量
end_msg()

begin_msg(ack_buyMysterious_special_goods, 23, 16)
	def_int(errorcode)                              // enBuyMysteriousReuslt
	def_int(indexId)                                // 索引ID
    def_int(goodNum)                                // 批量购买购买成功 数量
end_msg()

// 兑换神秘商店道具
begin_msg(req_buyMysterious_exchange_goods, 23, 17)
	def_int(indexId)                                // 索引ID
	def_int(goodsId)                                // 物品ID
    def_int(goodsNum)                               // 批量购买的物品数量
end_msg()

begin_msg(ack_buyMysterious_exchange_goods, 23, 18)
	def_int(errorcode)                              // enBuyMysteriousExchange
	def_int(indexId)                                // 索引ID
    def_int(goodNum)                                // 批量购买购买成功 数量
end_msg()

begin_msg(req_buyMysterious_exchange_list, 23, 19)
end_msg()

begin_msg(ack_buyMysterious_exchange_list, 23, 20)
	def_int(tm)										// 服务器时间
	def_object_arr(obj_mysterious, list)
end_msg()

begin_msg(req_buyMysterious_special_list, 23, 21)
end_msg()

begin_msg(ack_buyMysterious_special_list, 23, 22)
	def_int(tm)										// 服务器时间
	def_object_arr(obj_mysterious, list)
end_msg()

begin_msg(req_buyMysterious_special_goods_list, 23, 23)
end_msg()

begin_msg(ack_buyMysterious_special_goods_list, 23, 24)
	def_object_arr(obj_mysterious_special_goods, list)
end_msg()

begin_msg(req_buyMysterious_exchange_goods_list, 23, 25)
end_msg()

begin_msg(ack_buyMysterious_exchange_goods_list, 23, 26)
	def_object_arr(obj_mysterious_exchange_goods, list)
end_msg()
//**********************************   图纸副本   ***********************************//
begin_msg(req_get_printcopy_data, 24, 1)
end_msg()

begin_msg(ack_get_printcopy_data, 24, 2)
def_int(errorcode)
def_int_arr(finishedCopys)
def_object_arr(obj_copy_resettimes, resettimes);
end_msg()

begin_msg(req_printcopy_reset, 24, 3)
def_int(copyid)
end_msg()

begin_msg(ack_printcopy_reset, 24, 4)
def_int(errorcode)
def_int(copyid)
def_int(cost)
def_int(costType)
end_msg()

begin_msg(req_begin_printcopy, 24, 5)
def_int(copyid)
end_msg()

begin_msg(ack_begin_printcopy, 24, 6)
def_int(errorcode)
def_int(copyid)
end_msg()


// 活动场景同步
// 服务器提醒角色已经进入场景
begin_msg(notify_enter_act, 25, 1)
def_string(actname) //房间名称
def_object_arr(obj_roleinfo, roleinfos) // 已经进入活动场景的角色列表
def_object_arr(obj_posiinfo, posiinfos) // 每个对象的位置信息(坐标，方向)
end_msg()

// 服务器通知有玩家加入活动场景
begin_msg(notify_act_add_player, 25, 2)
def_object(obj_roleinfo, roleinfo) // 进入场景的角色对象
def_object(obj_posiinfo, posiinfo) // 对象的位置信息
end_msg()

// 服务器通知玩家离开了活动场景
begin_msg(notify_act_remove_player, 25, 3) //有角色离开了活动房间
def_int(roleid)
end_msg()

// 服务器通知同步角色移动
begin_msg(notify_act_player_move,25, 4)
def_object(obj_posiinfo, posiinfo)
end_msg()

// 发送角色同步消息
begin_msg(req_act_player_move, 25,5)
def_object(obj_posiinfo, posiinfo)
end_msg()

// 角色同步消息回应, 当服务器认为客户端的移动有错误时发送
// 将角色拖回服务器上的位置
begin_msg(respon_act_player_move, 25, 6) //
def_int(errorcode)
def_object(obj_posiinfo, posiinfo)
end_msg()

/************************* pet camp *********************/
begin_msg(req_get_petcamp_data, 26, 1)
end_msg()

begin_msg(ack_get_petcamp_data, 26, 2)
def_int(errorcode)
def_int_arr(finishedCopys)
def_object_arr(obj_copy_resettimes, resettimes);
end_msg()

begin_msg(req_petcamp_reset, 26, 3)
def_int(copyid)
end_msg()

begin_msg(ack_petcamp_reset, 26, 4)
def_int(errorcode)
def_int(copyid)
def_int(cost)
def_int(costType)
end_msg()

begin_msg(req_begin_petcamp, 26, 5)
def_int(copyid)
end_msg()

begin_msg(ack_begin_petcamp, 26, 6)
def_int(errorcode)
def_int(copyid)
end_msg()

/***************************  pet dunge  ******************/
begin_msg(req_finish_pet_dunge, 27, 1)
    def_int(result)
    def_int(sceneid)
    def_int(deadMonsterNum)
end_msg()


begin_msg(ack_finish_pet_dunge, 27, 2)
    def_int(errorcode)
    def_int(result)
    def_object_arr(obj_dungeawawrd, awards)
end_msg()

begin_msg(req_getpetdunge_data, 27, 3)
end_msg()

begin_msg(ack_getpetdunge_data, 27, 4)
    def_int(errorcode)
    def_int_arr(frightedfriends)
end_msg()

begin_msg(req_begin_pet_dunge, 27, 5)
    def_int(sceneid)
    def_int(friendid)
end_msg()

begin_msg(ack_begin_pet_dunge, 27, 6)
    def_int(errorcode)
    def_int(copyid)
    def_int(friendid)
    def_int(maxhp)
    def_int(atk)
    def_int(def)
    def_int(hit)
    def_int(dodge)
    def_int(cri)
    def_int_arr(skills)
    def_object(obj_petBattleProp, petinfo)
end_msg()


////////////////////BOSS （世界、公会）////////////////////////////////////////
begin_msg(req_enter_boss_scene,30,1)        // 申请进入
    def_int(bossType)                        // enumBossType
end_msg()

begin_msg(ack_enter_boss_scene,30,2)        // 确认进入
    def_int(err)                            // 1表示无错
    def_int(sceneid)                        // 由服务器通知sceneid
    def_int(bossid)                         // bossid == monsterid
    def_int(bossHP)                         // boss 当前 HP
    def_int(reborn_seconds)                 // 复活秒数
    def_float(elaps_seconds)                  // 活动开始多少秒
    def_int(remain_seconds)                        // 创建boss剩余时间
end_msg()

begin_msg(req_leave_boss_scene,30,3)        // 请求离开
    def_int(sceneid)
end_msg()

begin_msg(ack_leave_boss_scene,30,4)        // 确认离开
    def_int(err)                            // 1表示无错
    def_int(sceneid)
end_msg()

begin_msg(req_attack,30,6)                  // 普攻
    def_int(sceneid)
    def_int(objtype)                        // pet or role
    def_int(isCir)
    def_int(hurts)
    def_int_arr(args)                       // 附加参数
    def_float(elaps_seconds)                  // 客户端时间，防作弊 
end_msg()

begin_msg(req_skill_hurt,30,7)              //技能攻击
    def_int(sceneid)
    def_int(casterType)                     //技能施放者类型 （主角，角色，宠物，怪物）
    def_int(casterID)                       //技能施放者ID
    def_int(hurterType)                     //受伤者类型 （主角，角色，宠物，怪物）
    def_int(hurterID)                       //受伤者ID
    def_int(skillID)                        //技能ID
    def_int(skillEffectIndex)               //技能效果索引（本技能的第几个效果）
    def_int(hurtNumber)                     //技能造成的伤害数值
    def_int(isCriHurt)                      //是否为暴击 1 暴击，0 普通攻击
    def_float(elaps_seconds)                  // 客户端时间，防作弊 
end_msg()

begin_msg(notify_hp_boss,30,8)              // 血量同步
    def_int(hp)                             // 剩余
end_msg()

begin_msg(req_role_battle_info, 30, 11)     // 请求玩家战斗信息
    def_int(roleid)
end_msg()

begin_msg(notify_role_battle_info, 30, 12)  // 玩家战斗信息
    def_int(ID)
    def_string(name)
    def_int(typeID)
    def_int(lvl)
    def_int(maxhp)
    def_int(atk)
    def_int(def)
    def_int(hit)
    def_int(dodge)
    def_int(cri)
    def_int_arr(skills)
    def_object(obj_petinfo, petinfo)
end_msg()


begin_msg(syn_hurtboss_rank,30,13)          // boss伤害榜
    def_object_arr(obj_hurts,hurts)         // 伤害
    def_int(total_hurts)                    // 总伤害
    def_int(myhurts)                        // 自己的
    def_int(hurtAwardBatpoint)              // 伤害奖励战功
    def_int(hurtAwardGold)                  // 伤害奖励金币
end_msg()

begin_msg(notify_dead_boss,30,16)           // 通知杀死者，发放奖品，结束活动，退出场景
    def_string(rolewhokill)                 // 最后一击角色名

    //排名奖励
    def_int(gold)                           // 金币奖
    def_int(exp)                            // 经验奖
    def_int(bat_point)                      // 战功奖
    def_int(constell)                       // 星灵奖
    def_int(item)                           // 道具奖

    //伤害奖励
    def_int(hurt_bat_point)                 // 战功奖
    def_int(hurt_gold)                      // 金币奖

    //公会奖励
    def_int(guildFortune)                   // 公会财富
    def_int(guildConstruction)              // 公会建设度

    def_int(constribute)                    // 公会个人贡献奖
    def_int(exploit)                        // 公会功勋奖

    def_int(hurts)                          // 你的
end_msg()

begin_msg(notify_boss_fail,30,18)
    def_int(gold)                           // 金币奖
    def_int(exp)                            // 经验奖
    def_int(bat_point)                      // 战功奖
    def_int(constell)                       // 星灵奖
end_msg()

begin_msg(notify_boss_created, 30, 19)
end_msg()


//////////////////// end 世界ＢＯＳＳ////////////////////////////////////////




/**************************  begin towerdefense  *************************/

begin_msg(req_get_towerdefense_data, 31, 1)
end_msg()

begin_msg(ack_get_towerdefense_data, 31, 2)
    def_int(times)
end_msg()

begin_msg(req_enter_tower_room, 31, 3)
    def_int(towerlvl)
    def_int(sceneid)
    def_int(oncemore)
end_msg()

begin_msg(ack_enter_tower_room, 31, 4)
    def_int(sceneid)
    def_int(errorcode)
    def_int(oncemore)
end_msg()

begin_msg(req_leave_tower_room, 31, 5)
end_msg()

begin_msg(ack_leave_tower_room, 31, 6)
    def_int(errorcode)
end_msg()

begin_msg(req_tower_room_next_wave, 31, 7)
    def_int(towerlvl)
    def_int(usetime)
    def_int(monstercount)
    def_int(wave)
end_msg()
/**************************  end  towerdefense  ************************/

/**************************  公会试炼 始*************************/

begin_msg(req_get_guild_train_data, 31, 51)
end_msg()

begin_msg(ack_get_guild_train_data, 31, 52)
    def_int(leftTimes)      // 剩余次数
end_msg()

begin_msg(req_enter_guild_train_room, 31, 53)
    def_int(towerlvl)
    def_int(sceneid)
end_msg()

begin_msg(ack_enter_guild_train_room, 31, 54)
    def_int(errorcode)
    def_float(inspireAddBattleForce)        // 鼓舞的战斗力加成 0~ 0.5
    def_object(obj_batforce_brief, bat)     // 我的战斗力(加成后)
end_msg()

begin_msg(req_leave_guild_train_room, 31, 55)
end_msg()

begin_msg(ack_leave_guild_train_room, 31, 56)
    def_int(errorcode)
    def_int(contribute) // 个人贡献
    def_int(exploit) // 个人功勋
    def_int(fortune) // 公会财富
    def_int(construction) // 公会建设度
    def_float(bless) // 被祝福加成 0 ~ 0.3
end_msg()

begin_msg(req_guild_train_room_next_wave, 31, 57)
    def_int(killwave)   // 已经成功守的波数
end_msg()




/**************************  公会试炼 末************************/



/**************************  mysticalcopy  ************************/

begin_msg(req_get_mystical_copylist, 32, 1)
end_msg()

begin_msg(ack_get_mystical_copylist, 32, 2)
    def_int(errorcode)
    def_object_arr(obj_mysticalcopy, indexs)
    def_object_arr(obj_mysticalcopy, cycleIndexs)
    def_int(times)
    def_int(consumabletimes)
    def_int(surplusSec) //列表刷新的剩余秒数
end_msg()

begin_msg(req_enter_mysticalcopy, 32, 3)
    def_int(listIndex)
    def_int(copyid)
    def_int(oncemore)
end_msg()

begin_msg(ack_enter_mysticalcopy, 32, 4)
    def_int(errorcode)
    def_int(copyid)
    def_object_arr(obj_mysticalmonster, monsters)
    def_int(oncemore)
end_msg()

begin_msg(req_pass_mysticalcopy, 32, 5) //玩家死了，发过来。如果玩家点退出，在发req_leave_mysticalcopy之前，发过来
    def_int(ispass) //0－no pass, 1-pass
    def_int(copyid)
    def_int_arr(monsters)
end_msg()

begin_msg(req_leave_mysticalcopy, 32, 6)
    def_int(copyid)
    def_int(bossid)
end_msg()

begin_msg(ack_leave_mysticalcopy, 32, 6)
    def_int(errorcode)
    def_object(obj_copyaward, award)  //正常翻牌奖励
    def_string_arr(monsterKillAward)    //隐藏小怪的奖励
    def_string(bossAward)   //隐藏boss的奖励
end_msg()

begin_msg(ack_pass_mysticalcopy, 36, 7)
    def_object(obj_mysticalmonster, boss)
    def_int(bosstalkingid)
end_msg()

/************************** end mysticalcopy  ************************/




/****************************** petpvp ********************************/

begin_msg(req_get_petpvp_data, 33, 1)
end_msg()

begin_msg(ack_get_petpvp_data, 33,2)
    def_int(errorcode)  //
    def_int(randking)   //排名
    def_int(cups)       //所谓的竞技等级
    def_int(points)     //可用积分
    def_int(pvptype)    //黄金，白银， 青铜段
    def_int_arr(petlist)    //按阵型的位置顺序发，如果该位置没宠物则petid为0
    def_int(pvptimes)   //今天一进行的pvp次数
    def_int(wintimes)   //胜利场次
    def_int(losetimes)  //不胜利场次
    def_int(weeklyType) //上周结算时的竞技场段位
    def_int(weeklyRank) //上周结算排名,为0表示已经发放过积分不提示，非零表示刚领取了积分，客户端提示一下
    def_int(awardpoint) //上周结算积分
    def_int_arr(historyRank)    //历史战绩
end_msg()

begin_msg(req_edit_petpvp_formation, 33, 3)
    def_int_arr(newformation)
end_msg()

begin_msg(ack_edit_petpvp_formation, 33, 4)
    def_int(errorcode)
    def_int_arr(petlist)    //按阵型的位置顺序发，如果该位置没宠物则petid为0
end_msg()

begin_msg(req_search_petpvp_enemy, 33, 5)
    def_int(firstSearch)
end_msg()

begin_msg(ack_search_petpvp_enemy, 33, 6)
    def_int(errorcode)
    def_int(roleid)
    def_int(roletype)
    def_int(rolelvl)
    def_string(rolename)
    def_int(pvptype)    //段位
    def_int(rank)
    def_object_arr(obj_petBattleProp, petsinfo)   //按阵型的位置顺序发，如果该位置没宠物则petid为0
end_msg()

begin_msg(req_begin_petpvp, 33, 7)
    def_int(roleid)     //我的id
    def_int(enemyid)    //敌人的id
    def_int(sceneid)
end_msg()

begin_msg(ack_begin_petpvp, 33, 8)
    def_int(errorcode)
    def_int(sceneid)
end_msg()

begin_msg(req_commit_petpvp_battle_report, 33, 9)
    def_int(iswin)
end_msg()

begin_msg(ack_commit_petpvp_battle_report, 33, 10)
    def_int(errorcode)
    def_int(iswin)
    def_int(getPoints)
    def_int(getCups)
end_msg()

begin_msg(req_end_search_petpvp_enemy, 33, 11)
end_msg()

begin_msg(req_get_petpvp_formation, 33, 12)
end_msg()

begin_msg(ack_get_petpvp_formation, 33, 13)
    def_int_arr(petlist)
end_msg()

begin_msg(req_get_item_by_consum_petpvp_point, 33, 14)
    def_int(item)
    def_int(count)
end_msg()

begin_msg(ack_get_item_by_consum_petpvp_point, 33, 15)
    def_int(errorcode)
    def_int(consumPoints)
end_msg()
/******************************* end petpvp ****************************/

//****************************** honor ********************************/

//　获取当前使用的称号
begin_msg(req_honor, 34, 1)
end_msg()

begin_msg(ack_honor, 34, 2)
	def_int(honorId)
end_msg()

//　获取已得到的称号
begin_msg(req_honor_info, 34, 3)
	def_int(type)
end_msg()

begin_msg(ack_honor_info, 34, 4)
    def_int(type)
	def_object_arr(obj_honorInfo, obj)
end_msg()

//　获取未获得称号的条件值
begin_msg(req_honor_cond_info, 34, 5)
	def_int(type)
end_msg()

begin_msg(ack_honor_cond_info, 34, 6)
	def_object_arr(obj_honorCondInfo, obj)
end_msg()

//　使用称号
begin_msg(req_honor_use, 34, 7)
	def_int(honorId)
end_msg()

begin_msg(ack_honor_use, 34, 8)
	def_int(honorId)
	def_int(errorcode)	// enUseHonroReuslt
end_msg()

//　称号消息
begin_msg(notify_honor_get, 34, 9)
	def_int(honorId)
	def_int(isLimited)			// 是否限时称号
	def_int(remainingTime)		// 剩余时间(秒)
end_msg()

//　取消称号新状态
begin_msg(req_honor_readed, 34, 10)
	def_int(honorId)
end_msg()

begin_msg(ack_honor_readed, 34, 11)
end_msg()

//　取消使用称号
begin_msg(req_honor_cancel, 34, 12)
end_msg()

begin_msg(ack_honor_cancel, 34, 13)
	def_int(errorcode)	// 0成功 1失败
end_msg()

//　同步声望消息
begin_msg(notify_edit_prestige, 34, 14)
	def_int(prestige)
end_msg()

//　过期称号消息
begin_msg(notify_honor_expiration, 34, 15)
	def_int(honorId)
end_msg()

// ====================================
// 获取答题信息
begin_msg(req_answerinfo_get, 35, 1)
end_msg()

begin_msg(ack_answerinfo_get, 35, 2)
    def_int(errorcode)              // eAnswerInfoGetResult
    def_int(question_number)        // 第几题（从1开始）
    def_int(question_id)            // 问题的系统ID
    def_int(change_question_count)  // 换题次数
    def_int(score)                  // 已获得的分数
    def_int(right_questions_count)  // 答对题目个数
    def_int(endTime)                // 剩余倒计（秒）
    def_int(isOver)                 // 游戏是否完成
    def_int(surplus_num)            // 剩余 题目个数
end_msg()

// 答题
begin_msg(req_answer_select, 35, 3)
    def_int(answer)                 //客户端 超时 答题 默认 自动发 0
end_msg()

begin_msg(ack_answer_select, 35, 4)
    def_int(errorcode)              // eAnswerSelectResult
    def_int(next_question_id)       // 下一题系统ID
    def_int(isOver)                 // 游戏是否完成
    def_int(isTrue)                 // 是否正确
    def_int(score)                  // 已获得的分数
    def_int(right_questions_count)  // 答对题目个数
    def_int(endTime)                // 倒计时 时间
    def_int(surplus_num)            // 剩余 题目个数
end_msg()

//换题
begin_msg(req_answer_questionchange, 35, 5)
end_msg()

begin_msg(ack_answer_questionchange, 35, 6)
    def_int(errorcode)              // eAnswerQuestionChangeResult
    def_int(question_id)            // 更换的题目ID
    def_int(endTime)                // 倒计时 时间
    def_int(change_question_count)  // 换题次数
end_msg()

// 答题结束消息
begin_msg(notify_answer_over, 35, 7)
    def_int(errorcode)              // eAnswerNotifyResult
    def_int(total_score)            // 总分
    def_int(total_time)             // 消耗的总时间(秒)
    def_int(right_questions_count)  // 答对题目个数
end_msg()

// 累计奖励信息
begin_msg(req_answer_totalrewards, 35, 8)
end_msg()

begin_msg(ack_answer_totoalrewards, 35, 9)
def_int(is_reward1) //是否领奖
def_int(day_count1) //天次数
def_int(is_reward2) 
def_int(day_count2) 
def_int(is_reward3) 
def_int(day_count3) 
end_msg()

// 领取奖励
begin_msg(req_answer_getrewards, 35, 9)
def_int(rewards_type) //eAnswerResultRewardsType
end_msg()


begin_msg(ack_answer_getrewards, 36, 9)
def_int(errorcode)  //eGetRewardsResult
end_msg()
//
//

/********************************** 名人堂相关协议 ******************************/
begin_msg(req_fameshall_player_data, 37, 1)
    def_int(famesVersion)       // 名人堂的版本，为了让客户端比较更新数据
end_msg()

//名人堂，玩家的个人信息
begin_msg(ack_fameshall_player_data, 37, 2)
    def_int(currLayer)              //当前要挑战的层数
    def_int(topLayer)               //已经挑战的层数
    def_int(startlayer)             //起始层数
    def_int(errorcode)
    def_int(famesVersion)
end_msg()

//名人堂，名人列表
begin_msg(req_fameshall_fameslist, 37, 3)
end_msg()

begin_msg(ack_fameshall_fameslist, 37, 4)
    def_int(errorcode)
    def_object_arr(obj_fames_info, warriorFames)    //剑士名人，牛B的下标越小
    def_object_arr(obj_fames_info,mageFames)       //法师名人，牛B的下标越小
    def_object_arr(obj_fames_info,AssassinFames)   //刺客名人，牛B的下标越小
end_msg()

//挑战结果结算
begin_msg(req_finish_famesHall, 37, 5)
    def_int(result)                 //挑战结果，参考枚举：DungeResult
    def_int(sceneid)                //当前层id
    def_int(layer)                  //层数
end_msg()

//服务器返回，当result == eDungeNext 时不返回
begin_msg(ack_finish_famesHall, 37, 6)
    def_int(errorcode)              //
    def_int(result)                 //挑战结果，参考枚举：DungeResult
    def_int_arr(awards)             //奖励，如果不需要可去掉
end_msg()

begin_msg(req_fameshall_beginBattle, 37, 7)
    def_int(layer)
    def_int(sceneid)
end_msg()

begin_msg(ack_fameshall_beginbattle, 37, 8)
    def_int(errorcode)
    def_int(sceneid)
end_msg()

begin_msg(notify_fresh_fameshall_list, 37, 9)
end_msg()


/****************************** 累计充值返利活动 始 **************************/
begin_msg(req_acc_recharge_status, 38, 1)
end_msg()

begin_msg(ack_acc_recharge_status, 38, 2)
    def_int(actindex)           // 活动id
    def_int_arr(awardrmb)       // 已领奖励
    def_int(accrmb)             // 当前活动累计的金钻
end_msg()

begin_msg(req_acc_recharge_get_award, 38, 3)
    def_int(rmb)                // 请求领奖的rmb
end_msg()

begin_msg(ack_acc_recharge_get_award, 38, 4)
    def_int(errorcode)            // eAccuRechargeAwardResult
    def_int(rmb)                // 请求领奖的rmb
end_msg()


begin_msg(req_accumulate_recharge_awardact, 38, 37)
end_msg()

begin_msg(ack_accumulate_recharge_awardact, 38, 38)
def_object_arr(obj_accumulate_recharge, accumulateRecharge)
def_int(errorcode)
end_msg()
/****************************** 累计充值返利活动 末 **************************/

/****************************** 累计消费返利活动 始 **************************/
begin_msg(req_acc_consume_status, 38, 11)
end_msg()

begin_msg(ack_acc_consume_status, 38, 12)
    def_int(actindex)           // 活动id
    def_int_arr(awardrmb)       // 已领奖励
    def_int(accrmb)             // 当前活动累计的金钻
end_msg()

begin_msg(req_acc_consume_get_award, 38, 13)
    def_int(rmb)                // 请求领奖的rmb
end_msg()

begin_msg(ack_acc_consume_get_award, 38, 14)
    def_int(errorcode)            // eAccuRechargeAwardResult
    def_int(rmb)                  // 请求领奖的rmb
end_msg()

begin_msg(req_redirect_name, 38, 15)
    def_int(itemIndex)              //改名卡所在的背包格子索引
    def_int(type)                   //修改的类型，现有角色，公会,查看 eRenameType
    def_string(newname)             //新名字
end_msg()

begin_msg(ack_redirect_name, 38, 16)
    def_int(type)                   //修改的类型，现有角色，公会,查看 eRenameType
    def_int(errorcode)              //错误码参照 GlobalErrorCode
end_msg()

begin_msg(req_pre_redirect_name, 38, 17)
    def_int(type)               //更改类型
    def_string(newname)         //检查一下名字是否重复
end_msg()

begin_msg(ack_pre_redirect_name, 38, 18)
    def_int(type)               
    def_int(errorcode)
end_msg()


begin_msg(req_accumulate_consume_awardact, 38, 39)
end_msg()

begin_msg(ack_accumulate_consume_awardact, 38, 40)
def_object_arr(obj_accumulate_consume, accumulateConsume)
def_int(errorcode)
end_msg()

/****************************** 单笔充值返利活动 始 **************************/
//不可叠加的充值活动返利*********************** 开始
begin_msg(req_once_recharge_status, 38, 19)   
end_msg()

begin_msg(ack_once_recharge_status, 38, 20)
def_object_arr(obj_recharge_info,recharges)
end_msg()
begin_msg(req_once_recharge_get_award, 38, 21)
def_int(time)                // 请求领奖，TIME表示充值时的时间，由服务器发给客户端
end_msg()

begin_msg(ack_once_recharge_get_award, 38, 22)
def_int(errorcode)            // eAccuRechargeAwardResult
def_int(time)                // 请求领奖，TIME表示充值时的时间，由服务器发给客户端
end_msg()
//不可叠加的充值活动返利*********************** 开始

//可叠加的充值活动返利*********************** 开始
begin_msg(req_re_recharge_status, 38, 23)
end_msg()

begin_msg(ack_re_recharge_status, 38, 24)
def_object_arr(obj_recharge_info,recharges)
end_msg()

begin_msg(req_re_recharge_get_award, 38, 25)
def_int(time)                // 请求领奖，TIME表示充值时的时间，由服务器发给客户端
end_msg()

begin_msg(ack_re_recharge_get_award, 38, 26)
def_int(errorcode)            // eAccuRechargeAwardResult
def_int(time)                // 请求领奖，TIME表示充值时的时间，由服务器发给客户端
end_msg()
//可叠加的充值活动返利*********************** 结束

// 月卡
begin_msg(req_month_recharge_status, 38, 27)
end_msg()

begin_msg(ack_month_recharge_status,38,28)
	def_int(id)						// 对应活动id
	def_int_arr(alreadGet)          // 已经领取的索引
	def_int_arr(canGet)             // 可以领取的索引
	def_int(errorcode)              // eMonthRechargeAwardResult
def_int(tm)	
end_msg()

begin_msg(req_month_recharge_get_award, 38, 29)
	def_int(index)                  //请求领取的索引
end_msg()

begin_msg(ack_month_recharge_get_award, 38, 30)
	def_int(index)                  //请求领取的索引
	def_int(errorcode)              // eMonthRechargeAwardResult
end_msg()

// 邀请码
begin_msg(req_invite_status, 38, 31)
end_msg()

begin_msg(ack_invite_status,38,32)
def_object_arr(obj_invite_code_num,totalNum)           //输入此人的邀请码的各个阶段人数
def_string(code)                //邀请码
def_int_arr(canGet)             // 
def_int(errorcode)              //
end_msg()

begin_msg(req_invite_get_award, 38, 33)
def_int(index)                  //请求领取的索引
end_msg()

begin_msg(ack_invite_get_award, 38, 34)
def_int(index)                  //请求领取的索引
def_int(errorcode)              //
end_msg()

begin_msg(req_invite_input_code,38,35)
def_string(code)
end_msg()

begin_msg(ack_invite_input_code,38,36)
def_string(code)
def_int(errorcode)
end_msg()

begin_msg(req_operate_act_recharge_award, 38, 41)
end_msg()

begin_msg(ack_operate_act_recharge_award, 38, 42)
def_object_arr(obj_operate_recharge, operateRecharge)
def_int(errorcode)
end_msg()

begin_msg(req_operate_act_once_recharge_award, 38, 43)
end_msg()

begin_msg(ack_operate_act_once_recharge_award, 38, 44)
def_object_arr(obj_operate_once_recharge, operateOnceRecharge)
def_int(errorcode)
end_msg()

//--首冲奖励
begin_msg(req_gift_activity, 38, 45)
end_msg()

begin_msg(ack_gift_activity, 38, 46)
def_object_arr(obj_gift_activity, giftActivity)
def_int(errorcode)
end_msg()

/****************************** 单笔充值返利活动 末 **************************/

/******************************获取同步服务器的时间***************************/
begin_msg(req_server_time, 38, 47)
end_msg()

begin_msg(ack_server_time, 38, 48)
def_int(time)
def_int(errorcode)
end_msg()

// 通知月卡状态更新
begin_msg(notify_month_recharge_open, 38, 49)
end_msg()

/****************************** 消耗精力领奖利 开始 ****************************/
begin_msg(req_fat_award_activity, 38, 50)
end_msg()

begin_msg(ack_fat_award_activty, 38, 51)
def_int(errorcode)
def_object_arr(obj_fat_award_activity, fat_activity)
end_msg()

begin_msg(req_consumefat_award_status, 38, 52)
end_msg()

begin_msg(ack_consumefat_award_status, 38, 53)
def_int(activityid)          // 活动id
def_int_arr(awardfat)        // 已领奖励
def_int(consumefat)          // 当前活动累计消费的精力
end_msg()

begin_msg(req_consumefat_get_award, 38, 54)
def_int(fat)                // 请求领奖的精力
end_msg()

begin_msg(ack_consumefat_get_award, 38, 55)
def_int(errorcode)          // eConsumeFatAwardResult
def_int(fat)                // 请求领奖的精力
end_msg()


begin_msg(req_month_recharge_get_rmb, 38, 56)
end_msg()

begin_msg(ack_month_recharge_get_rmb, 38, 57)
	def_int(id)						// 对应活动id
	def_int(rmb)					// 对应的金钻
end_msg()
/****************************** 消耗精力领奖利 结束 ****************************/

/****************************** 幻兽试炼场**************************/

begin_msg(req_petelite_fightpetlist, 39, 1)     // 已经参加过幻兽试炼场的幻兽列表
end_msg()

begin_msg(ack_petelite_fightpetlist, 39, 2)
    def_int_arr(petlist)
end_msg()

begin_msg(req_enter_petelite_copy, 39, 3)
    def_int_arr(list_petid)                              // 选择宠来完成幻兽试炼
    def_int(copyid)
    def_int(oncemore)
end_msg()

begin_msg(ack_enter_petelite_copy, 39, 4)
    def_int(errorcode)
    def_int(copyid)
end_msg()

begin_msg(req_finish_petelite_copy, 39, 5)
    def_int(ispass)
    def_int(star)
    def_int(copyid)
    def_int(deadMonsterNum)
end_msg()

begin_msg(ack_finish_petelite_copy, 39, 6)
    def_int(ispass)
    def_int(errorcode)
    def_int(copyid)
    def_object(obj_copyaward, award)
end_msg()

//获取幻兽出战队列
begin_msg(req_get_petidlist_copy, 39, 7)   
end_msg()

begin_msg(ack_get_petidlist_copy, 39, 8)
    def_int_arr(list_petid)
end_msg()

/****************************** 累计消费返利活动 末 **************************/
//领取精力,精灵女王的祝福
begin_msg(req_daily_get_fat_status, 40, 1)
end_msg()

begin_msg(ack_daily_get_fat_status, 40, 2)
    def_int(errorcode)      //  eQueenBlessStatusType
    def_int(status)         //  1表示可领取
    def_string(index_str)    //示例1;2;3 已经领取的index
    def_int(award)
end_msg()

begin_msg(req_daily_get_fat, 40, 3)
end_msg()

begin_msg(ack_daily_get_fat, 40, 4)
    def_int(errorcode)  // eQueenBlessGetType
    def_int(index)
end_msg()

begin_msg(notify_get_fat_status, 40, 5)
end_msg()

/***************************** 同步竞技场 ********************************/

//获取功能相关信息
begin_msg(req_get_realpvp_data, 41, 1)
end_msg()

begin_msg(ack_get_realpvp_data, 41, 2)
    def_int(errorcode)
    def_int(roleid)
    def_int(awardTimes)
    def_object(obj_realpvp_info, pvpinfo)
end_msg()

//准备匹配
begin_msg(req_bereadyto_realpvp_matching, 41, 3)
end_msg()

begin_msg(ack_bereadyto_realpvp_matching, 41, 4)
    def_int(errorcode)
    def_int(matchCountDown)                             //匹配的倒计时
end_msg()

//取消匹配
begin_msg(req_cancel_realpvp_matching, 41, 5)
end_msg()

begin_msg(ack_cancel_realpvp_matching, 41, 6)
    def_int(errorcode)
end_msg()

//匹配失败
begin_msg(notify_realpvp_matching_fail, 42, 7)
    def_int(errorcode)
    def_int(awardMedal)                                 //获得的勋章、荣誉
    def_int(awardHonor)
end_msg()

//返回匹配对手的信息
begin_msg(notify_realpvp_matching_info, 41, 8)
    def_int(enemyid)
    def_object(obj_realpvp_info, enemyinfo)
    def_int(fightCountDown)                             //进入战斗的倒计时
end_msg()

//发送战斗结果
begin_msg(notify_realpvp_fighting_result, 41, 9)
    def_int(errorcode)
    def_int(result)                         // 1 = win, 0 = lose
    def_int(awardMedal)
    def_int(awardHonor)
end_msg()

begin_msg(req_realpvp_medal_exchange, 41, 10)
    def_int(itemIndex)
    def_int(itemCount)
end_msg()

begin_msg(ack_realpvp_medal_exchange, 41, 11)
    def_int(errorcode)
    def_int(itemCount)
end_msg()

begin_msg(req_realpvp_exchange_itemlist, 41, 12)
end_msg();

begin_msg(ack_realpvp_exchange_itemlist, 41, 13)
    def_object_arr(obj_realpvp_item, items)
end_msg()

/**
 *  决斗相关
 **/

//发出邀请
begin_msg(req_duel_invite, 41, 14)
    def_int(targetid)                           //被邀请的id
    def_int(duelType)                           //决斗类型， 暂时不需要用到
end_msg()

begin_msg(ack_duel_invite, 41, 15)
    def_int(errorcode)
end_msg()

//通知被邀请者
begin_msg(notify_duel_invite_notice, 41, 16)
    def_int(inviterid)                          //邀请者的id
    def_string(inviterName)                     //邀请者的名字
    def_int(inviterRoletype)                    //邀请者的角色类型
    def_int(inviterLvl)                         //邀请者的等级
    def_int(inviteTime)                         //邀请时间， 作为判断是不是同一个邀请的标识
end_msg()

//被邀请者把是否接受邀请返回服务器
begin_msg(req_duel_invite_respond, 41, 17)
    def_int(inviterid)                          //邀请者id
    def_int(inviteTime)                         //邀请时间,作为判断是不是同一个邀请的标识
    def_int(agree)                              //是否同意， 1 = 同意, 0 = 不同意
end_msg()

begin_msg(ack_duel_invite_respond, 41, 18)
    def_int(agree)
    def_int(errorcode)                          //
end_msg()

begin_msg(notify_duel_invite_respond, 41, 19)
    def_int(targetid)
    def_int(agree)
end_msg()

//通知双方准备进入战斗
begin_msg(notify_duel_be_ready, 41, 20)
    def_int(errorcode)
end_msg()

begin_msg(req_duel_invite_cancel, 41, 21)
end_msg()

begin_msg(ack_duel_invite_cancel, 41, 22)
    def_int(errorcode)
end_msg()

begin_msg(notify_duel_result, 41, 23)
    def_int(isWin)                              //1 = win, 0 = lose
    def_string(enemyName)
end_msg()

begin_msg(notify_duel_invite_cancel, 41, 24)
    def_int(inviterId)
    def_string(inviterName)
end_msg()
//同步场景相关消息从100开始


/*************************************************************/
//获取扭蛋信息
begin_msg(req_capsuletoy_status, 42, 1)
end_msg()

begin_msg(ack_capsuletoy_status, 42, 2)
def_int(errorcode)  //该活动开关为1的时候返回0，否则其他值
def_object_arr(obj_capsuletoy_info, info)   //各扭蛋类型的信息
end_msg()

//免费领取扭蛋
begin_msg(req_capsuletoy_get_egg, 42, 3)
def_int(qua_type)       // 扭蛋类型
end_msg()

begin_msg(ack_capsuletoy_get_egg, 42, 4)
def_int(errorcode)      // 枚举 eCapsuletoyResult
def_int(qua_type)
def_int(item_id)
def_int(item_count)
def_int(remain_time)        // 剩余时间
def_int(remain_free_times) // 剩余免费领取次数，-1表示不存在每天免费次数，例如qua2,3,4
end_msg()

//购买扭蛋
begin_msg(req_capsuletoy_buy_egg, 42, 5)
def_int(qua_type)
end_msg()

begin_msg(ack_capsuletoy_buy_egg, 42, 6)
def_int(errorcode)     // 枚举 eCapsuletoyResult
def_int(qua_type)
def_int(item_id)
def_int(item_count)
def_int(remain_buy_times)   // 剩余购买次数,-1无限制购买次数
end_msg()

//买X送Y优惠活动
begin_msg(req_capsuletoy_buy_n_egg, 42, 7)
def_int(qua_type)
end_msg()

begin_msg(ack_capsuletoy_buy_n_egg, 42, 8)
def_int(errorcode)          // 枚举 eCapsuletoyResult
def_int(qua_type)
def_int_arr(item_id)        // item_id与item_count下标对应。
def_int_arr(item_count)     //
def_int(buy_times)          // 已经购买的次数
def_int(special_offers_buy_count)   //买X送Y活动买的个数，没有这个活动此值为0，   例如qua4,这里为10
def_int(special_offers_present_count)//买X送Y活动送的个数，                      例如qua4,这里为1
def_int(remain_buy_times)   // 剩余购买次数,-1无限制购买次数
def_int(award_mark)			// 买了超过10次要送一次,存1 2 3 4分别表示领取1次，2次

end_msg()


// 新扭蛋
begin_msg(req_twistegg_info, 42, 9)
end_msg()

begin_msg(ack_twistegg_info, 42, 10)
	def_int(tm)									// 服务器时间
	def_object_arr(obj_twistegg_info, infoArr)	// 各扭蛋项信息
end_msg()

begin_msg(req_twistegg_get, 42, 11)
	def_int(index)								// 扭蛋项
	def_int(activityId)							// 扭蛋的活动Id
	def_int(free)								// 免费扭（0或1）
end_msg()

begin_msg(ack_twistegg_get, 42, 12)
	def_int(errorcode)							// 枚举 eTwisteggGetResult
	def_object(obj_twistegg_info, info)			// 扭蛋项信息
	def_object(obj_twistegg_result, result)		// 结果
end_msg()

begin_msg(req_twistegg_eventwist, 42, 13)
	def_int(index)								// 连扭
end_msg()

begin_msg(ack_twistegg_eventwist, 42, 14)
	def_int(errorcode)							// 枚举 eTwisteggGetResult
	def_object_arr(obj_twistegg_result, resultArr)
end_msg()

// 有配置活动时间的扭蛋项消息
begin_msg(notify_twistegg_activity, 42, 16)
	def_int(tm)									// 服务器时间
	def_int(index)								// 扭蛋项
end_msg()

#pragma mark - 公会夺宝战

//获取副本列表信息
begin_msg(req_get_treasure_copy_data, 43, 1)
    def_int(beginIndex)                         //开始的下标，从0开始
    def_int(endIndex)                           //结束的下标
end_msg()

begin_msg(ack_get_treasure_copy_data, 43, 2)
    def_int(errorcode)
    def_int(beginIndex)                         //开始的下标，从0开始
    def_int(endIndex)                           //结束的下标
    def_int(isGetall)                           //是否已获取全部
    def_object_arr(obj_treasure_scene, scenelist)
end_msg()

//打开活动界面
begin_msg(req_enter_guild_treasurefight_activity, 43, 3)
end_msg()

begin_msg(ack_enter_guild_treasurefight_activity, 43, 4)
    def_int(errorcode)
    def_int(myPoint)
    def_int(guildPoint)
    def_int(guildRank)
    def_int(recoverCountdown)
    def_int(actDuration)                        //活动剩余的秒数
end_msg()

//关闭活动界面
begin_msg(req_leave_guild_treasure_activity, 43, 5)
    
end_msg()

begin_msg(ack_leave_guild_treasure_activity, 43, 6)
    def_int(errorcode)
end_msg()

//进入公会宝藏副本
begin_msg(req_enter_guild_treasurecopy, 43, 7)
    def_int(sceneid)
end_msg()

begin_msg(ack_enter_guild_treasurecopy, 43, 8)
    def_int(errorcode)
    def_int(sceneid)
end_msg()

//下发发生变化的副本信息
begin_msg(notify_guild_treasurecopy_change, 43, 9)
    def_object(obj_treasure_scene, copy)
end_msg()

//快速进入战场
begin_msg(req_quick_enter_guild_treasurecopy, 43, 10)
end_msg()

begin_msg(ack_quick_enter_guild_treasurecopy, 43, 11)
    def_int(errorcode)
    def_int(sceneid)
    def_string(sceneName)
end_msg()

//有变化的时候下发给前端
begin_msg(notify_guild_treasurefight_data_change, 43, 12)
    def_int(myPoint)
    def_int(guildPoint)
    def_int(guildRank)
end_msg()

begin_msg(notify_guild_new_treasurecopy, 43, 13)
    def_object(obj_treasure_scene, copy)
end_msg()

//在副本里查看积分
begin_msg(req_treasurecopy_get_fighting_points, 43, 14)
end_msg()

begin_msg(ack_treasurecopy_get_fighting_points, 43, 15)
    def_int(myPoints)
    def_int(guildPoints)
    def_int(guildRank)
end_msg()

//副本内获取玩家的积分及副本内当前玩家的排名
begin_msg(req_treasurecopy_get_fighting_rank, 43, 16)
end_msg()

begin_msg(ack_treasurecopy_get_fighting_rank, 43, 17)
    def_object_arr(obj_treasurecopy_rankdata, datas)
end_msg()

//活动结束后的结算界面
begin_msg(notify_treasurecopy_activity_result, 43, 18)
    def_int(myPoints)
    def_int(enemyKilled)
    def_int(monsterKilled)
    def_int(assists)
    def_int(boxesKilled)
    def_object_arr(obj_treasurecopy_guildresult, datas)
    def_int(myGuildPoints)
    def_int(myGuildRank)
end_msg()

//获取占领的协议
begin_msg(req_treasurecopy_get_manors, 43, 19)
end_msg()

begin_msg(ack_treasurecopy_get_manors, 43, 20)
    def_object_arr(obj_treasurecopy_manor, manors)
end_msg()

//领取公会占领的奖励
begin_msg(req_treasurecopy_get_guildmanor_award, 43, 21)
    def_int(index)                          //领哪个奖励，具体就是哪一周的奖励
end_msg()

begin_msg(ack_treasurecopy_get_guildmanor_award, 43, 22)
    def_int(index)
    def_int(errorcode)
    def_string(award)
end_msg()

/***************************** 图腾 ********************************/

// 获取图腾
begin_msg(req_totem_group, 44, 1)
end_msg()

begin_msg(ack_totem_group, 44, 2)
	def_object_arr(obj_totem_info, info)
end_msg()

// 图腾献祭
begin_msg(req_totem_immolation, 44, 3)
	def_int(totemId)	// 图腾id
	def_int_arr(pets)	// 献祭的宠物
end_msg()

begin_msg(ack_totem_immolation, 44, 4)
	def_int(errorcode)		// eTotemImmolation
	def_object(obj_totem_info, info)
	def_int_arr(pets)
end_msg()


//衣柜
begin_msg(req_wardrobe_info, 45, 1)

end_msg()

begin_msg(ack_wardrobe_info, 45, 2)
    def_int(lvl)
    def_int(exp)
    def_int(activebody)
    def_int(activeweapon)
    def_int(activehome)
    def_object_arr(obj_fashion_info, fashions)
end_msg()


begin_msg(notify_add_fashion, 45, 3)
    def_object(obj_fashion_info, fashion)
end_msg()

begin_msg(notify_fashion_expire, 45, 4)
    def_int_arr(fashions)
end_msg()

begin_msg(notify_refresh_fashion, 45, 5)
    def_object(obj_fashion_info, fashion)
end_msg()

begin_msg(req_puton_fashion, 45, 6)
    def_int(fashionid)
    def_int(type)
end_msg()

begin_msg(ack_puton_fashion, 45, 7)
    def_int(errorcode)
    def_int(fashionid)
    def_int(type)
end_msg()

begin_msg(req_takeoff_fashion, 45, 8)
    def_int(type)
end_msg()

begin_msg(ack_takeoff_fashion, 45, 9)
    def_int(errorcode)
    def_int(type)
end_msg()

begin_msg(notify_wardrobe_exp, 45, 10)
    def_int(lvl)
    def_int(exp)
end_msg()

begin_msg(req_doublePotionNum, 46, 1)   //好友副本 剩余双倍药水奖励次数

end_msg()

begin_msg(ack_doublePotionNum, 46, 2)
    def_int(nums)
end_msg()

/*********
 组队 by wangzhigang
 ********/

//创建队伍
begin_msg(req_fighting_team_create, 47, 1)
    def_int(sceneMod)
end_msg()

//
begin_msg(ack_fighting_team_create, 47, 2)
    def_int(errorcode)
    def_int(teamid)
    def_int(mySyncCopyleftTimeOfAward)
end_msg()

//申请加入队伍
begin_msg(req_fighting_team_aplly_join, 47, 3)
    def_int(teamid)
end_msg()

//
begin_msg(ack_fighting_team_aplly_join, 47, 4)
    def_int(errorcode)
    def_int(teamid)
    def_int(sceneMod)
    def_object_arr(obj_fighting_team_member, members)
end_msg()

//通知队员有新队员加入
begin_msg(notify_fighting_team_someone_join, 47, 5)
    def_string(newMemberName)
    def_object(obj_fighting_team_member, member)
end_msg()

//离开队伍
begin_msg(req_fighting_team_leave, 47, 6)
end_msg()

//
begin_msg(ack_fighting_team_leave, 47, 7)
    def_int(errorcode)
end_msg()

//通知其他队员有人离开
begin_msg(notify_fighting_team_someone_leave, 47, 8)
    def_int(leaveId)
    def_string(leaveName)
    def_int(position)
end_msg()

//踢人
begin_msg(req_fighting_team_kick_someone_out, 47, 9)
    def_int(someoneBeKick)
end_msg()

begin_msg(ack_fighting_team_kick_someone_out, 47, 10)
    def_int(errorcode)
    def_string(someoneBeKickName)
end_msg()

//通知队员（包括自己）有人被踢，如果被踢的是自己，客户端做不同的反应
begin_msg(notify_fighting_team_be_kick_out, 47, 11)
    def_int(beKickId)                                   //被踢的人的id
    def_string(someoneBeKickName)                       //被踢的人的名字
    def_string(captainName)                             //队长名字
end_msg()

//通知其他队员，当某个队员状态变化时
begin_msg(notify_fighting_team_change, 47, 12)
    def_object(obj_fighting_team_member, member)        //
end_msg()

//任命新的队长
begin_msg(notify_fighting_team_new_captain, 47, 13)
    def_int(teamid)
    def_int(position)
    def_int(newCaptain)
    def_string(newCaptainName)
end_msg()

//准备，如果是队长就相当于开始
begin_msg(req_fighting_team_set_ready, 47, 14)
end_msg()

begin_msg(ack_fighting_team_set_ready, 47, 15)
    def_int(errorcode)
end_msg()

//取消准备      
begin_msg(req_fighting_team_cancel_ready, 47, 16)
end_msg()

begin_msg(ack_fighting_team_cancel_ready, 47, 17)
    def_int(errorcode)
end_msg()

//获取自己队伍的信息，目前用于同步组队副本结束后回到组队界面时用到
begin_msg(req_fighting_team_get_teamInfo, 47, 18)
    def_int(teamid)
end_msg()

begin_msg(ack_fighting_team_get_teamInfo, 47, 19)
    def_int(errorcode)
    def_int(teamid)
    def_int(sceneMod)
    def_int(mySyncCopyleftTimeOfAward)                //角色的组队副本剩余收益次数
    def_object_arr(obj_fighting_team_member, members)
end_msg()

//邀请玩家加入队伍， 批量邀请
begin_msg(req_fighting_team_batch_invite_player, 47, 20)
    def_int_arr(targetsId)
end_msg()


begin_msg(ack_fighting_team_batch_invite_player, 47, 21)
    def_int(errorcode)
end_msg()

//下发邀请通知给目标玩家
begin_msg(notify_fighting_team_invitation, 47, 22)
    def_int(inviterId)
    def_string(inviterName)
    def_int(teamid)
    def_int(sceneMod)
end_msg()

//玩家收到通知的反应
begin_msg(req_fighting_team_invitation_respond, 47, 23)
    def_int(teamid)
    def_int(inviterId)
    def_int(accept)                     //1 = 接受，0 = 拒绝
end_msg()

//如果接受并且errorcode = CE_OK，就返回队伍信息 , 如果accept = 0，就不返回ack了
begin_msg(ack_fighting_team_invitation_respond, 47, 24)
    def_int(errorcode)
    def_int(accept)
    def_int(teamid)
    def_int(sceneMod)
    def_object_arr(obj_fighting_team_member, members)
end_msg()

//发挥给邀请者，接受的前端可以不处理，拒绝的话，前端可提示xxx拒绝了你的邀请
begin_msg(notify_fighting_team_invitation_respond, 47, 25)
    def_int(accept)                     //1 = 接受 ， 0 = 拒绝
    def_int(targetId)
    def_string(targetName)
end_msg()

begin_msg(req_fighting_team_teamtalking, 47, 26)
    def_int(teamId)
    def_string(message)
    def_int(messageType)
    def_blob(voiceData)
    def_int(voicdeDuration)
    def_int(translated)
end_msg()

begin_msg(notify_fighting_team_teamtalking,47,27)
    def_int(senderId)
    def_int(senderRoleType)
    def_string(senderName)
    def_string(message)
    def_int(createDate)
    def_int(messageType)
    def_int64(voiceId)
    def_int(voicdeDuration)
    def_int(translated)
end_msg()

//单个邀请
begin_msg(req_fighting_team_single_invite_player, 47, 28)
    def_int(targetsId)
end_msg()


begin_msg(ack_fighting_team_single_invite_player, 47, 29)
    def_int(errorcode)
end_msg()

//通知队员场景变化
begin_msg(notify_fighting_team_scenemod_change, 47, 30)
    def_int(newscenemod)
end_msg()

/*-----------------------------------------*/
/**********
 同步组队副本 by wangzhigang
 *********/

//请求队伍列表
begin_msg(req_sync_teamcopy_list, 48, 1)
end_msg()

begin_msg(ack_sync_teamcopy_list, 48, 2)
    def_int(leftTimeOfAward)                        //剩余收益次数
    def_object_arr(obj_sync_team_info, teams)
end_msg()

//关闭界面
begin_msg(req_sync_teamcopy_close_interface, 48, 3)
end_msg()

//可以组队的副本列表, 客户端根据这些场景模板发送req_fighting_team_create到服务器创建队伍
begin_msg(req_sync_teamcopy_creatable_sceneModList, 48, 4)
end_msg()

begin_msg(ack_sync_teamcopy_creatable_sceneModList, 48, 5)
    def_int_arr(sceneMods)
end_msg()

//更改副本(只允许队长操作)
begin_msg(req_sync_teamcopy_change_scenemod, 48, 6)
    def_int(scene)
end_msg()

begin_msg(ack_sync_teamcopy_change_scenemod, 48, 7)
    def_int(errorcode)
    def_int(scene)
end_msg()

//副本的列表有变化，如果team.memberCount==0，客户端把该行去掉
begin_msg(notify_sync_teamcopy_listchange, 48, 8)
    def_object(obj_sync_team_info, team)
end_msg()

//有玩家新建了个房间副本
begin_msg(notify_sync_teamcopy_increase, 48, 9)
    def_object(obj_sync_team_info, team)
end_msg()

//把队伍从列表中去掉，例如队伍开始进入战斗的时候
begin_msg(notify_sync_teamcopy_remove, 48, 10)
    def_int(teamid)
end_msg()

begin_msg(notify_sync_teamcopy_award, 48, 11)
    def_int(isPass)
    def_int(hasAward)
    def_int(errorcode)
    def_object(obj_copyaward, normalaward)
end_msg()

begin_msg(notify_edit_intimacy, 49, 1)
def_int(friendid)
def_int(intimacy)
end_msg()


begin_msg(gm_addTop50Firend,49,2)
def_int(roleID)
def_object(obj_friendRoleinfo, role)
end_msg()

//客户端检查和服务器的连接状态
begin_msg(req_linkStateCheck, 50, 1)
end_msg()
begin_msg(ack_linkStateCheck, 50, 2)
end_msg()

/**********************武器附魔**************************/
// 获取列表
begin_msg(req_enchant_list, 51, 1)
end_msg()
begin_msg(ack_enchant_list, 51, 2)
    def_int(errorcode)
    def_object_arr(obj_enchant_info, enchantList)
end_msg()

// 随机附魔
begin_msg(req_random_enchant, 51, 3)
def_int(continueNum)   // 连续执行次数
end_msg()
begin_msg(ack_random_enchant, 51, 4)
    def_int(errorcode)
    def_object_arr(obj_enchant_info, enchantList)
end_msg()

// 指定附魔
begin_msg(req_sure_enchant, 51, 5)
    def_int(enchantId)
    def_int(continueNum)   // 连续执行次数
end_msg()
begin_msg(ack_sure_enchant, 51, 6)
    def_int(errorcode)
    def_object_arr(obj_enchant_info, enchantList)
end_msg()

// 通知最高等级变化
begin_msg(notify_current_enchant, 51, 7)
    def_int(enchantId)
    def_int(enchantLvl)
end_msg()

// 可以自由选择 满级附魔
begin_msg(req_choose_enchant, 51, 8)
    def_int(enchantId)
end_msg()
begin_msg(ack_choose_enchant, 51, 9)
    def_int(errorcode)
    def_int(enchantId)
end_msg()


//-------------------时装武器搜集---------------
// 请求 时装搜集 列表id信息
begin_msg(req_fashionCollect_list, 52, 1)

end_msg()

begin_msg(ack_fashionCollect_list, 52, 2)
    def_int(errorcode)
    def_object_arr(obj_fashionCollect_info, listInfo);  //正在进行中的id
    def_int_arr(latestCollect)          //各条线已经完成的id
end_msg()

//请求搜集合成 时装
begin_msg(req_collectFashion, 52, 3)
    def_int(objectId)
end_msg()

begin_msg(ack_collectFashion, 52, 4)
    def_int(errocode)
    def_int(isMaxStage)                  //是否最高阶
    def_int(objectId)                    //合成成功的id
    def_object_arr(obj_item, items)      //奖励物品：武器/时装，礼包
end_msg()

//合成成功，同步 下一阶 数据    暂时不用
begin_msg(notify_nextstage_collectInfo, 52, 5)
    def_int(errocode)
    def_int(preCollectId)
    def_object(obj_fashionCollect_info, newCollectInfo)
end_msg()

begin_msg(req_fashioncollect_get_materialbox, 52, 6) //材料背包
end_msg()

begin_msg(ack_fashioncollect_get_materialbox, 52, 7)
    def_int(errorcode)
    def_int(boxsize)
    def_object_arr(obj_bagItem,items)
end_msg()

//通知范围：1神秘商店 产出新热门时装类型： 2.购买神秘商店时装武器类型，直接完成 某一阶时装合成。也会通知。
begin_msg(notify_fashioncollect_add_newprogress, 52, 8) 
    def_int(preProgressid)
    def_object(obj_fashionCollect_info, newCollectInfo)
end_msg()

begin_msg(notify_fashioncollect_delete_progress, 52, 9)
    def_int(progressid)
end_msg()

//手动放置材料
begin_msg(req_puton_fashioncollect_material, 52, 10)
    def_int(objectId)
    def_int(materialId)
    def_int(materialCount)
    def_int(position) //第几个 材料
end_msg()

begin_msg(ack_puton_fashioncollect_material, 52, 11)
    def_int(errorcode)
end_msg()

//已经放置的材料 数据 更新
begin_msg(notify_fashioncollect_material_change, 52, 12)
    def_object(obj_fashionCollect_info, collectinfo)   //客户端根据这个协议判断 是否已经满足合成 条件，若满足则 自动请求合成材料
end_msg()

begin_msg(notify_fashioncollect_materialbox, 52, 13)
    def_object_arr(obj_addordelItem,items)
end_msg()

begin_msg(req_fashionCollect_recycle_material, 52, 14)
    def_int_arr(grids)
end_msg()

begin_msg(ack_fashionCollect_recycle_material, 52, 15)
    def_int(errorcode)
end_msg()

begin_msg(req_fashionCollect_prop_added, 52, 16)
end_msg()

begin_msg(ack_fashionCollect_prop_added, 52, 17)
    def_int(maxHp)
    def_int(atk)                //攻击
    def_int(def)                //防御
    def_float(hit)              //命中
    def_float(dodge)            //闪避
    def_float(cri)              //暴击

    def_int(recycleLvl)         //材料回收系统等级
    def_int(recycleExp)         //材料回收系统经验
    def_float(recyclePropParam)   //全属性加成百分比
end_msg()

begin_msg(req_fashionCollect_recycle_single_material, 52, 18)
    def_int(gridIndex)
    def_int(count)
end_msg()

begin_msg(ack_fashionCollect_recycle_single_material, 52, 19)
    def_int(errorcode)
end_msg()
//-------------------时装武器搜集---------------

//--------------每日日程----------------
begin_msg(req_daily_schedule_list, 53, 1)

end_msg()

begin_msg(ack_daily_schedule_list, 53, 2)
    def_int(errorcode)
    def_int(point)
    def_int_arr(gotaward)
    def_object_arr(obj_dailyschedule_info, dailyschedules)
end_msg()

begin_msg(notify_daily_schedule_point, 53, 3)
    def_int(point)
    def_int(type)
end_msg()

begin_msg(req_get_daily_schedule_award, 53, 4)
    def_int(awardid)
end_msg()

begin_msg(ack_get_daily_schedule_award, 53, 5)
    def_int(errorcode)
    def_int(awardid)
    def_string(award)
end_msg()

//--------------每日日程----------------

//--------------角色奖励----------------
begin_msg(req_role_award_state, 54, 1)
end_msg()

begin_msg(ack_role_award_state, 54, 2)
    def_err(errorcode)
    def_int(days)               // 当前可以领取的页面id，非天数
    def_string(alreadyget)
    def_string(canget)
end_msg()

begin_msg(req_role_award, 54, 3)
    def_int(days)               // 申请领取的页面id
end_msg()

begin_msg(ack_role_award, 54, 4)
    def_err(errorcode)
    //def_string(awards)          // 领取成功的奖励
end_msg()
//--------------角色奖励----------------

/********************* 觉 醒 *******************/

begin_msg(req_role_awake_skill_replace, 55, 1)
    def_int(preSkillId)
end_msg()

begin_msg(ack_role_awake_skill_replace, 55, 2)
    def_int(errorcode)
end_msg()

begin_msg(req_role_awake_lvlup, 55, 3)
end_msg()

begin_msg(ack_role_awake_lvlup, 55, 4)
    def_int(errorcode)
    def_int(awakelvl)
end_msg()

/********************* 点石成金 *******************/

begin_msg(req_midastouch_info, 56, 1)
end_msg()

begin_msg(ack_midastouch_info, 56, 2)
	def_object(obj_midastouch_info, info)
end_msg()

begin_msg(req_midastouch, 56, 3)
end_msg()

begin_msg(ack_midastouch, 56, 4)
    def_int(errorcode)						// eMidastouchResult
	def_object(obj_midastouch_info, info)
end_msg()



///膜拜
begin_msg(req_get_beworship_data, 57, 1)
    def_int(start_rank)
    def_int(end_rank)
end_msg()

begin_msg(ack_get_beworship_data, 57, 2)
    def_int(errorcode)
    def_int(start_rank)
    def_int(end_rank)
    def_object_arr(obj_beworship_member, info)
end_msg()

begin_msg(req_worship, 57, 3)
    def_int(targetId)
    def_string(type)
end_msg()

begin_msg(ack_worship, 57, 4)
    def_int(errorcode)
    def_int(targetId)
    def_string(type)
end_msg()


//服务器排名
begin_msg(req_get_cspvp_serverrank, 57, 5)
    def_int(start_rank)
    def_int(end_rank)
end_msg()

begin_msg(ack_get_cspvp_serverrank, 57, 6)
    def_int(errorcode)
    def_int(start_rank)
    def_int(end_rank)
    def_object_arr(obj_cspvp_serverrank_member, info)
end_msg()

//个人排行
begin_msg(req_get_cspvp_rolerank, 57, 7)
    def_int(start_rank)
    def_int(end_rank)
end_msg()

begin_msg(ack_get_cspvp_rolerank, 57, 8)
    def_int(errorcode)
    def_int(start_rank)
    def_int(end_rank)
    def_object_arr(obj_cspvp_rolerank_member, info)
end_msg()

//跨服战



begin_msg(req_cspvp_challengedata, 57, 9)
end_msg()

begin_msg(ack_cspvp_challengedata, 57, 10)
def_int(errorcode)
def_object_arr(obj_crossservicewar_role, roles)
end_msg()


begin_msg(req_cspvp_log, 57, 11)
def_int(num)
end_msg()

begin_msg(ack_cspvp_log, 57, 12)
def_int(errorcode)
def_object_arr(obj_crossservicewar_log, logs)
end_msg()



begin_msg(req_cspvp_roleinfo, 57, 13)
end_msg()

begin_msg(ack_cspvp_roleinfo, 57, 14)
def_int(errorcode)
def_int(myrank)
def_int(getaward)
def_int(awardrank)
def_int(costperminute)
def_int(collingtime)
def_int(score)
end_msg()

begin_msg(req_begin_cspvp, 57, 15)
def_int(serverid)
def_int(bechallengeId)
def_int(bechallengeRank)
def_int(sceneid)
end_msg()

begin_msg(ack_begin_cspvp, 57, 16)
def_int(errorcode)
def_int(sceneid)
def_object(obj_roleBattleProp, enemyinfo)
def_object(obj_petBattleProp, enemypetinfo)
def_object(obj_retinueBattleProp, enemyRetinueinfo)
def_int(selfMaxhp)
def_int(selfatk)
def_int(selfdef)
def_int(selfhit)
def_int(selfdodge)
def_int(selfcri)
end_msg()

begin_msg(req_cspvp_result, 57, 17)
def_int(iswin)
def_int(bechallengeId)
end_msg()

begin_msg(ack_cspvp_result, 57, 18)
def_int(errorcode)
def_int(cooltime)
def_int(isWin)
def_int(rank)
def_int(score)
end_msg()


begin_msg(req_worship_info, 57, 19)
end_msg()

begin_msg(ack_worship_info, 57, 20)
def_int(battlepoint)
def_int(specialworshiptimes)  //使用过的豪华次数
def_int(commonworshiptimes)  //使用过的普通次数
def_int(beworshiptimes)
def_float(addproperty)
end_msg()


begin_msg(req_fresh_cspvp_immediately, 57, 21)
end_msg()

begin_msg(ack_fresh_cspvp_immediately, 57, 22)
    def_int(errorcode)
    def_int(cost)
    def_int(costType)
end_msg()

/*
 *侍魂协议
 */

//获取所以侍魂信息
begin_msg(req_get_retinues_info, 58, 1)
end_msg()

begin_msg(ack_get_retinues_info, 58, 2)
    def_object_arr(obj_retinue_info, retinues)
end_msg()

//获取已有的侍魂技能
begin_msg(req_get_retinue_skills, 58, 3)
end_msg()

begin_msg(ack_get_retinue_skills, 58, 4)
    def_object_arr(obj_retinue_skillInfo, skills)
end_msg()

//侍魂合成
begin_msg(req_compose_retinue, 58, 5)
def_int(targetId)
end_msg()

begin_msg(ack_compose_retinue, 58, 6)
def_int(errorcode)
def_object(obj_retinue_info, newRetinue)
end_msg()

//获取装备的侍魂技能
begin_msg(req_get_retinue_equiped_skills, 58, 7)
end_msg()

begin_msg(ack_get_retinue_equiped_skills, 58, 8)
    def_object_arr(obj_retinue_skillInfo, equipedSkills)
end_msg()

begin_msg(req_study_retinue_skill, 58, 9)
    def_int(skillid)
    def_int(skillLvl)
end_msg()

begin_msg(ack_study_retinue_skill, 58, 10)
    def_int(errorcode)
    def_int(skillid)
    def_int(skillLvl)
end_msg()

//获取通灵信息
begin_msg(req_get_spiritspeak_info, 58, 11)
end_msg()

begin_msg(ack_get_spiritspeak_info, 58, 12)
    def_object_arr(obj_spiritspeak_info, spiritspeaks)
end_msg()

//通灵
begin_msg(req_do_spiritspeak, 58, 13)
    def_int(petId)
    def_int(spiritspeakId)
    def_int(position)
    def_int(retinueId)
end_msg()

begin_msg(ack_do_spiritspeak, 58, 14)
    def_int(errorcode)
    def_object(obj_spiritspeak_info, newSpiritspeak)
end_msg()

//聚魂
begin_msg(req_retinue_summon, 58, 15)
    def_int(summonId)
    def_int(summonType)         //0为普通聚魂， 1为高级聚魂
end_msg()

begin_msg(ack_retinue_summon, 58, 16)
    def_int(errorcode)
    def_string(retinuePieces)
end_msg()

begin_msg(req_get_retinue_summon_info, 58, 17)
end_msg()

begin_msg(notify_retinue_summon_info, 58, 18)
    def_int(cooldown)       //现在只有冷却时间，可能以后会由更多的信息
    def_int(remianRemoveTimes)    //可用的次数  
end_msg()

//侍魂材料背包相关
begin_msg(req_get_retinue_bag, 58, 19)
end_msg()

begin_msg(ack_get_retinue_bag, 58, 20)
    def_int(errorcode)
    def_int(boxsize)
    def_object_arr(obj_bagItem,items)
end_msg()

begin_msg(notify_retinue_bag,58, 21)
    def_int(errorcode)
    def_object_arr(obj_addordelItem,items)
end_msg()

begin_msg(req_use_retinue, 58, 22)
    def_int(retinueid)
    def_int(isuse)
end_msg()

begin_msg(ack_use_retinue, 58, 23)
   def_int(errorcode)
   def_int(retinueid)
   def_int(isuse)
end_msg()

//侍魂升级
begin_msg(req_retinue_lvlup, 58, 24)
    def_int(retinueId)
    def_int(materialId)
end_msg()

begin_msg(ack_retinue_lvlup, 58, 25)
    def_int(errorcode)
end_msg()

//侍魂升星
begin_msg(req_retinue_starup, 58, 26)
    def_int(retinueId)
    def_int(materialId)
end_msg()

begin_msg(ack_retinue_starup, 58, 27)
    def_int(errorcode)
end_msg()

//下发侍魂信息
begin_msg(notify_retinue_info, 58, 28)
    def_object(obj_retinue_info, retinue)
end_msg()

begin_msg(req_remove_retinue_summon_cooldown, 58, 29)
    def_int(summonId)
end_msg()

begin_msg(ack_remove_retinue_summon_cooldown, 58, 30)
    def_int(errorcode)          //成功之后服务器发送 notify_retinue_summon_info
end_msg()

begin_msg(req_equip_retinue_skill, 58, 31)
    def_int(index)
    def_int(retinueSkillId)         //0 时为脱装备
end_msg()

begin_msg(ack_equip_retinue_skill, 58, 32)
    def_int(errrorcode)
    def_int(index)
    def_int(retinueSkillId)
end_msg()

//累计充值
begin_msg(notify_syn_accumulateRechargeRecord, 58, 33)
    def_int(record)
end_msg()

//累计消费
begin_msg(notify_syn_accumulateRmbConsumedRecord, 58, 34)
def_int(record)
end_msg()
