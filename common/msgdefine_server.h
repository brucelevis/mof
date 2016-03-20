//#include "cyclemsgdefine.h"

begin_msg(req_register_server, 400, 1)
	def_int(server_id)
	def_string(server_name)
end_msg()

begin_msg(ack_register_server, 400, 2)
	def_int(errorcode)
	def_string(errmsg)
end_msg()


begin_msg(req_keep_alive, 400, 3)

end_msg()



//取GameServer排行榜数据
begin_msg(req_cs_loadPaihangData, 400, 6)
    def_int(type)
end_msg()

begin_msg(ack_cs_loadPaihangRoleData, 400, 7)
    def_int(type)
    def_object_arr(obj_paihangdata, data)
    def_int(total)
end_msg()


begin_msg(ack_cs_loadPaihangPetData, 400, 8)
    def_int(type)
    def_object_arr(obj_Petpaihangdata, data)
    def_int(total)
end_msg()

begin_msg(req_cs_sendRankList, 400, 9)
    def_int(clientSession)
    def_int(beginRank)
    def_int(endRank)
    def_int(type)
    def_int(roleid)
    def_int(petid)
end_msg()

begin_msg(ack_cs_sendRankList, 400, 10)
    def_int(type)
    def_int(beginRank)
    def_int(endRank)
    def_object_arr(obj_paihangdata, data)
    def_int(myRank)
    def_int(clientSession)
end_msg()

begin_msg(req_cs_sendRankListAwards, 400, 11)
    def_string(awards)
    def_int(roleid)
    def_string(rolename)
    def_int(rank)
    def_int(rankListType)
end_msg()

begin_msg(req_get_csRoleData_through_cs, 400, 12)
    def_int(roleid)
    def_int(clientSession)
    def_int(doReqServerid)
    def_int(onReqServerid)
end_msg()

begin_msg(req_getCrossServiceRankListStatus, 400, 13)
    def_int_arr(types)
    def_int(clientSession)
end_msg()

begin_msg(req_cs_get_role_Data, 400, 14)
    def_int(roleid)
    def_int(clientSession)
    def_int(doReqServerid)
    def_int(onReqServerid)
end_msg()

begin_msg(ack_cs_get_role_Data, 400, 15)
    def_int(errorcode)
    def_int(roleid)
    def_int(clientSession)
    def_int(doReqServerid)
    def_int(onReqServerid)

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
end_msg()

begin_msg(req_get_csPetData_through_cs, 400, 16)
    def_int(petid)
    def_int(clientSession)
    def_int(doReqServerid)
    def_int(onReqServerid)
end_msg()

begin_msg(req_cs_get_pet_Data, 400, 17)
    def_int(petid)
    def_int(clientSession)
    def_int(doReqServerid)
    def_int(onReqServerid)
end_msg()

begin_msg(ack_cs_get_pet_Data, 400, 18)
    def_int(errorcode)
    def_int(owner)
    def_object(obj_check_pet_prop, petinfo)

    def_int(clientSession)
    def_int(doReqServerid)
    def_int(onReqServerid)
end_msg()

begin_msg(req_get_csRoleFashion_through_cs, 400, 19)
    def_object_arr(obj_crossServiceRoleFashion_info, roleFashionInfos)

    def_int(rankListType)
    def_int(clientSession)
    def_int(doReqServerid)
    def_int(onReqServerid)
end_msg()

begin_msg(ack_get_csRoleFashion_through_cs, 400, 20)
    def_object_arr(obj_crossServiceRoleFashion_info, roleFashionInfos)

    def_int(rankListType)
    def_int(clientSession)
    def_int(doReqServerid)
    def_int(onReqServerid)
end_msg()

begin_msg(notify_sync_refresh_crossServiceRankList, 400, 21)
end_msg()

begin_msg(req_cs_get_role_battle, 401, 1)
    def_int(src_server_id)
    def_int(src_role_id)
    def_int(target_server_id)
    def_int(target_role_id)
    def_string(custom)
end_msg()


begin_msg(ack_cs_get_role_battle, 401, 2)
    def_int(errorcode)
    def_int(src_server_id)
    def_int(src_role_id)
    def_int(target_server_id)
    def_int(target_role_id)
    def_string(custom)

    def_float(worshipPlus)
    def_object(obj_roleBattleProp, role_battle)
    def_object(obj_petBattleProp, pet_battle)
    def_object(obj_retinueBattleProp, retinue_battle)
end_msg()

begin_msg(req_cs_get_cspvp_challenge_data, 402, 1)
    def_int(server_id)
    def_int(role_id)
end_msg()

begin_msg(req_cs_get_cspvp_roleinfo, 402, 3)
    def_int(server_id)
    def_int(role_id)
    def_int(colddown)
    def_int(costperminute)
end_msg()

begin_msg(req_cs_get_cspvp_log, 402, 5)
    def_int(server_id)
    def_int(role_id)
    def_int(num)
end_msg()

begin_msg(req_cs_begin_cspvp, 402, 7)
    def_int(src_server_id)
    def_int(src_role_id)
    def_int(target_server_id)
    def_int(target_role_id)
    def_int(target_rank)
end_msg()

begin_msg(ack_cs_begin_cspvp, 402, 8)
    def_int(errorcode)
    def_int(src_server_id)
    def_int(src_role_id)
    def_int(target_server_id)
    def_int(target_role_id)
end_msg()

begin_msg(req_cs_cspvp_result, 402, 9)
    def_int(src_server_id)
    def_int(src_role_id)
    def_int(target_server_id)
    def_int(target_role_id)
    def_int(iswin)
end_msg()

begin_msg(ack_cs_cspvp_result, 402, 10)
    def_int(errorcode)
    def_int(src_server_id)
    def_int(src_role_id)
    def_int(target_server_id)
    def_int(target_role_id)
    def_int(iswin)
    def_int(score)
    def_int(rank)
    def_int(isrankup)
end_msg()

begin_msg(req_cs_cspvp_add_role, 402, 11)
    def_int(server_id)
    def_int(role_id)
    def_string(role_name)
    def_int(lvl)
    def_int(bat)
    def_int(role_type)
end_msg()

begin_msg(req_cs_cspvp_get_rolerank, 402, 13)
    def_int(start_rank)
    def_int(end_rank)
end_msg()


begin_msg(req_cs_cspvp_get_serverrank, 402, 15)
    def_int(start_rank)
    def_int(end_rank)
end_msg()

begin_msg(req_cs_cancel_cspvp, 402, 17)
    def_int(server_id)
    def_int(role_id)
end_msg()

begin_msg(notify_cs_cspvp_serverrank_award, 402, 19)
    def_int(rank)
    def_string(award)
    def_int(week_time)
end_msg()

begin_msg(notify_cs_cspvp_rolerank_award, 402, 21)
    def_int(role_id)
    def_int(rank)
    def_string(award)
end_msg()



