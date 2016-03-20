//
//  msgdefine_world.h
//  GameSrv
//
//  Created by prcv on 14-3-12.
//
//  同步战斗消息

begin_msg(req_ping, 100, 0)
    def_int(sec)
    def_int(usec)
    def_int(ping)
end_msg()


begin_msg(ack_ping, 100, 1)
    def_int(sec)
    def_int(usec)
end_msg()

begin_msg(req_get_scenes, 100, 2)
    def_int(sceneMod)
end_msg()


begin_msg(ack_get_scenes, 100, 3)
    def_int_arr(scenes)
end_msg()

begin_msg(req_get_scene_objects, 101, 1)

end_msg()

begin_msg(ack_get_scene_objects, 101, 2)
    def_object_arr(obj_pos_info, roleposinfos)
    def_object_arr(obj_worldrole_info, roles)

    def_object_arr(obj_pos_info, petposinfos)
    def_object_arr(obj_worldpet_info, pets)

    def_object_arr(obj_pos_info, npcposinfos)
    def_object_arr(obj_worldnpc_info, npcs)

    def_object_arr(obj_pos_info, flyposinfos)
    def_object_arr(obj_worldfly_info, flies)

    def_object_arr(obj_pos_info, retinueposinfors)
    def_object_arr(obj_worldretinue_info, retinues)



    // added by jianghan for 增加宠物的控制方式定义：eServerCtrl or eClientServerCtrl
    def_int(petctrlmode)
    // end add
end_msg()


begin_msg(notify_new_roles, 101, 3)
    def_object_arr(obj_pos_info, roleposinfos)
    def_object_arr(obj_worldrole_info, roles)

    def_object_arr(obj_pos_info, petposinfos)
    def_object_arr(obj_worldpet_info, pets)

    def_object_arr(obj_pos_info, retiuneposinfors)
    def_object_arr(obj_worldretinue_info, retinues)

end_msg()


begin_msg(notify_objects_pos, 101, 4)
    def_object_arr(obj_pos_info, posinfos)
end_msg()

///-------------移动和站立--------------
begin_msg(req_sync_motion, 101, 5)
    def_int(instid)
    def_float(posx)
    def_float(posy)
    def_float(speedx)
    def_float(speedy)
    //目的点
    def_float(destx)
    def_float(desty)
end_msg()

//只有错误的情况才给反馈，如果成功了则走notify的消息
begin_msg(ack_sync_motion, 101, 6)
    def_int(errorcode)
    def_int(instid)
    def_float(posx)
    def_float(posy)
    def_float(speedx)
    def_float(speedy)
end_msg()

begin_msg(notify_sync_motion, 101, 7)
    def_float(time)
    def_int(instid)
    def_float(posx)
    def_float(posy)
    def_float(speedx)
    def_float(speedy)
    //目的点
    def_float(destx)
    def_float(desty)
end_msg()

begin_msg(req_sync_stop, 101, 8)
    def_int(instid)
    def_float(posx)
    def_float(posy)
end_msg()

begin_msg(ack_sync_stop, 101, 9)
    def_int(errorcode)
    def_int(instid)
    def_float(posx)
    def_float(posy)
end_msg()

begin_msg(notify_sync_stop, 101, 10)
    def_float(time)
    def_int(instid)
    def_float(posx)
    def_float(posy)
end_msg()

///-------------普通攻击--------------
begin_msg(req_sync_attack, 101, 11)
    def_int(instid)
    def_int(atkIndex)//普通攻击的序号。从1开始

    def_int(x)
    def_int(y)
    def_int(orient)
end_msg()

//只有错误的情况才给反馈，如果成功了则走notify的消息
begin_msg(ack_sync_attack, 101, 12)
    def_int(errorcode)
    def_int(instid)

    def_int(posx)
    def_int(posy)
    def_int(orient)
end_msg()

begin_msg(notify_sync_attack, 101, 13)
    def_int(sourceID)
    def_int(atkIndex)//普通攻击的序号。从1开始
    def_int(x)
    def_int(y)
    def_int(orient)
end_msg()

// modified by jianghan
// 允许一次传递多个被打信息
begin_msg(notify_sync_beatk, 101, 14)
def_float(time)
def_object_arr(obj_beAttacked, beAttacked)
end_msg()
// end modify

///-----这个消息只通过伤害，不通告任何角色行为相关的信息
begin_msg(notify_sync_damage, 101, 15)
    def_float(time)
    def_object_arr(obj_attackedTarget, attacked)
end_msg()

///-------------技能--------------
begin_msg(req_sync_skill, 101, 16)
    def_int(sourceID)
    def_int(skill_id)
    def_int(x)
    def_int(y)
    def_int(orient)
end_msg()

begin_msg(notify_sync_skill, 101, 17)
    def_float(time)
    def_int(sourceID)
    def_int(skill_id)

    def_int(x)
    def_int(y)
    def_int(orient)
end_msg()

// modified by jianghan
// 将多个被技能击中的效果封入一个包
begin_msg(notify_sync_beskill, 101, 18)
    def_float(time)
    def_object_arr(obj_beSkilled, beSkilled)
end_msg()
// end modify

begin_msg(notify_sync_changespeed, 101, 19)
    def_int(instid)
    def_float(percent)
    def_float(value)
end_msg()

///-------------角色场景管理--------------
begin_msg(notify_worldplayer_enter_scene, 101, 22)
    def_object_arr(obj_pos_info, roleposinfos)
    def_object_arr(obj_worldrole_info, roles)

    def_object_arr(obj_pos_info, petposinfos)
    def_object_arr(obj_worldpet_info, pets)
end_msg()

begin_msg(notify_worldplayer_leave_scene, 101, 23)
    def_int(roleInstId)
    def_int(retinueInstId)
    def_int_arr(petInstIds)
end_msg()

begin_msg(notify_worldrole_enter_scene, 101, 24)
    def_object(obj_pos_info, posinfo)
    def_object(obj_worldrole_info, roleinfo)
end_msg()

begin_msg(notify_worldpet_enter_scene, 101, 25)
    def_object(obj_pos_info, posinfo)
    def_object(obj_worldpet_info, petinfo)
end_msg()

begin_msg(notify_worldfly_enter_scene, 101, 26)
    def_object(obj_pos_info, posinfo)
    def_object(obj_worldfly_info, flyinfo)
end_msg()


begin_msg(notify_worldobject_leave_scene, 101, 27)
    def_int(instid)
end_msg()


//只有错误的情况才给反馈，如果成功了则走notify的消息
begin_msg(ack_sync_skill, 101, 28)
def_int(errorcode)
def_int(sourceID)
def_int(skill_id)

def_int(posx)
def_int(posy)
def_int(orient)
end_msg()

begin_msg(notify_worldnpc_enter_scene, 101, 29)
    def_object_arr(obj_pos_info, posinfos)
    def_object_arr(obj_worldnpc_info, npcinfos)
end_msg()

begin_msg(notify_next_scene_area, 101, 30)
    def_float(posx)
    def_float(posy)
end_msg()

begin_msg(req_sync_scene_ready, 101, 31)

end_msg()


///----------------场景逻辑相关-------------------------
begin_msg(notify_sync_pvp_start, 102, 1)
    def_int(battime)
end_msg()

begin_msg(notify_sync_pvp_end, 102, 2)
    def_int(reason)
end_msg()

///--------------夺宝战

begin_msg(req_sync_treasurefight_rank_data, 102, 20)
end_msg()

begin_msg(ack_sync_treasurefight_rank_data, 102, 21)
    def_object_arr(obj_treasurecopy_rankdata, datas)
end_msg()

begin_msg(notify_sync_treasurefight_kill_player, 102, 22)
    def_int(victim)
    def_int(killerid)
    def_int(score)
    def_int_arr(assists)
    def_int_arr(assistscore)
end_msg()

begin_msg(notify_sync_treasurefight_kill_npc, 102, 23)
    def_int(killerid)
    def_int(score)
    def_int_arr(assists)
    def_int_arr(assistscore)
end_msg()

begin_msg(notify_sync_treasurefight_player_leave, 102, 24)
    def_int(instid)
    def_object_arr(obj_treasurecopy_getscore, datas)
end_msg()


//组队战
begin_msg(notify_sync_teamcopy_start, 102, 25)

end_msg()

begin_msg(notify_sync_teamcopy_end, 102, 26)

end_msg()
