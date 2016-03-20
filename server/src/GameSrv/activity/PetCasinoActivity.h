//
//  PetCasinoActivity.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-12-16.
//
//  幻兽大比拼活动

#pragma once
#include "Role.h"


void on_req_activity_pet_casino_status(Role* role, req_activity_pet_casino_status& req);
void on_req_activity_pet_casino_wager(Role* role,req_activity_pet_casino_wager& req);
void on_req_activity_pet_casino_thief(Role* role,req_activity_pet_casino_thief& req);
void on_req_activity_pet_casino_current_history(Role* role,req_activity_pet_casino_current_history& req);
