//
//  guild_hand.cpp
//  GameSrv
//
//  Created by prcv on 13-4-18.
//
//

#include "hander_include.h"

#include "Guild.h"
#include "GuildMgr.h"
#include "GuildApply.h"
#include "GuildLog.h"
#include "TreasureFight.h"
#include "GameScript.h"

hander_msg(req_guild_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_list(role,req);
}}

hander_msg(req_create_guild,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_create_guild(role,req);
}}

hander_msg(req_dismiss_guild, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_dismiss_guild(role,req);
}}

hander_msg(req_cancle_dismiss_guild, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_cancle_dismiss_guild(role,req);
}}

hander_msg(req_cancle_transfer_master, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_cancle_transfer_master(role, req);
}}

hander_msg(req_search_guild, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_search_guild(role,req);
}}

hander_msg(req_modify_guild_notice, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_modify_guild_notice(role,req);
}}

hander_msg(req_transfer_guild_master, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_transfer_guild_master(role,req);
}}

hander_msg(req_guild_info, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_info(role,req);
}}

hander_msg(req_guild_member_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_member_list(role,req);
}}

hander_msg(req_player_guild_detail, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_player_guild_detail(role);
}}

hander_msg(req_role_apply_guild_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildApplyMgr.on_req_role_apply_guild_list(role);
}}

hander_msg(req_guild_applicant_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildApplyMgr.on_req_guild_applicant_list(role,req);
}}

hander_msg(req_apply_guild, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildApplyMgr.on_req_apply_guild(role,req);
}}

hander_msg(req_cancel_apply_guild, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
	SGuildApplyMgr.on_req_ancel_apply_guild(role, req);
}}

hander_msg(req_accept_apply_guild, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_accept_apply_guild(role,req);
}}

hander_msg(req_reject_apply_guild, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_reject_apply_guild(role,req);
}}

hander_msg(req_guild_leave, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_leave(roleid);
}}

hander_msg(req_guild_kick, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_kick(role,req);
}}

hander_msg(req_guild_donate, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_donate(role,req);
}}


hander_msg(req_guild_log, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);

    Guild& guild = SGuildMgr.getRoleGuild(roleid);
    if(guild.isNull())
        return;
    guild.sendLog(roleid, req.start, req.num);
}}

hander_msg(req_upgrade_guild_skill, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_upgrade_guild_skill(role,req);
}}

// 商店
hander_msg(req_guild_store, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_store(role, req);
}}

hander_msg(req_guild_storeUpdate, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_storeUpdate(role,req);
}}

hander_msg(req_guild_storeBuy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_storeBuy(role, req);
}}

hander_msg(req_guild_chat, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_chat(role, req);
}}

hander_msg(req_guild_boss_exp,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_boss_exp(role);
}}

hander_msg(req_train_guild_boss,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_train_guild_boss(role,req);
}}


hander_msg(req_my_guild_rank,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_my_guild_rank(role);

}}

hander_msg(req_past_guild_chat, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_past_guild_chat(role,req);
}}

hander_msg(req_guild_inspire, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_inspire(role,req);
}}

hander_msg(req_guild_bless, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_bless(role,req);
}}

hander_msg(req_guild_impeach_info, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_impeach_info(role);
}}

hander_msg(req_guild_impeach, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_impeach(role);
}}

hander_msg(req_guild_appoint_position, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    SGuildMgr.on_req_guild_appoint_position(role, req);
}}

// 公会商店(占城特权)
hander_msg(req_guild_occupy_store_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_guild_occupy_store_list ack;
	ack.point =  g_GuildTreasureFightMgr.getOccupyPoints(role);
	map<int, ChampaGoodsDef>::iterator it;
	for (it = GuildChampaGoodsCfg::mChampaGoods.begin(); it != GuildChampaGoodsCfg::mChampaGoods.end(); it++) {
		obj_occupy_store_goods_info obj;
		obj.index = it->second.index;
		obj.goodsId = it->second.goodsId;
		obj.goodsNum = it->second.goodsNum;
		obj.consume = 0;
		
		CMsgTyped msg;
        msg.SetInt(it->second.consume);
        msg.SetInt(ack.point);
		if (GameScript::instance()->call("guildStore_occupy_buy", msg) == 1) {
			obj.consume = msg.FloatVal();
		}

		obj.occupied = it->second.occupied;
		obj.goodsName = it->second.goodsName;
		obj.goodsDesc = it->second.goodsDesc;
		obj.pic = it->second.pic;
		
		ack.info_arr.push_back(obj);
	}
	role->send(&ack);
}}

hander_msg(req_guild_occupy_store_buy, req)
{
	hand_Sid2Role_check(sessionid, roleid, role);
	
    ack_guild_occupy_store_buy ack;
	int point = g_GuildTreasureFightMgr.getOccupyPoints(role);
	ack.errorcode = eGuildOccupyStoreResult_UnknowError;
	do {
		ChampaGoodsDef *def = GuildChampaGoodsCfg::getChampaGoods(req.index);
		if (NULL == def) {
			ack.errorcode = eGuildOccupyStoreResult_NoIndex;
			break;
		}
		// 检查占据分
		if (point < def->occupied) {
			ack.errorcode = eGuildOccupyStoreResult_NoOccupyPoints;
			break;
		}
		// 范围1-65535
		if (req.num <= 0 || req.num > 0xFFFF) {
			ack.errorcode = eGuildOccupyStoreResult_BuyNumRangeErr;
			break;
		}
		int buyNum = def->goodsNum * req.num;
		// 检查背包
		ItemArray items;
		GridArray effgrids;
		items.push_back(ItemGroup(def->goodsId, buyNum));

        if(role->preAddItems(items, effgrids) != CE_OK){
			ack.errorcode = eGuildOccupyStoreResult_BagFull;
			break;
		}
		// 扣费
		CMsgTyped msg;
        msg.SetInt(def->consume);
        msg.SetInt(point);
		if (GameScript::instance()->call("guildStore_occupy_buy", msg) != 1) {
			ack.errorcode = eGuildOccupyStoreResult_OccupyBuyLuaErr;
			break;
		}
		int value = msg.FloatVal();
		if (value <= 0) {
			ack.errorcode = eGuildOccupyStoreResult_ConsumeError;
			break;
		}
		int consume = value * req.num;
		RoleGuildProperty& prop = SRoleGuild(roleid);
		if (prop.noGuild()) {
			ack.errorcode = eGuildOccupyStoreResult_NoGuild;
			break;
		}
		if (!prop.addConstrib(-consume, "guild_occupy_store")) {
			ack.errorcode = eGuildOccupyStoreResult_NoConstrib;
			break;
		}
		
		// 添加物品
        role->playerAddItemsAndStore(effgrids, items, "guild_occupy_store", true);
		
		Xylog log(eLogName_OccupyBuy, roleid);
		log << point << value << req.num << consume << def->index << def->goodsId << def->goodsNum << def->consume;
		
		ack.errorcode = eGuildOccupyStoreResult_Success;		
	} while (false);

	role->send(&ack);
}}





