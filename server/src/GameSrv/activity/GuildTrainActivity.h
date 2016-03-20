//
//  GuildTrainActivity.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-12-16.
//
//  公会试练活动

#pragma once
#include "msg.h"

class Role;
void on_req_get_guild_train_data(Role* role , req_get_guild_train_data& req);
void on_req_enter_guild_train_room(Role* role, req_enter_guild_train_room& req);
void on_req_leave_guild_train_room(Role* role, req_leave_guild_train_room& req);
void on_req_guild_train_room_next_wave(Role* role, req_guild_train_room_next_wave& req);








