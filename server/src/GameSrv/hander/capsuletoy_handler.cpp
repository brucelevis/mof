//
//  capsuletoy_handler.cpp
//  GameSrv
//
//  Created by nothing on 14-4-29.
//
//

#include "hander_include.h"
#include "Role.h"

#include "CapsuletoyMgr.h"

hander_msg(req_capsuletoy_status, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    role->getCapsuletoyMgr()->sendCapsuletoyStatus();
}}

hander_msg(req_capsuletoy_get_egg, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    role->getCapsuletoyMgr()->getFreeCapsuletoy(req);
}}

hander_msg(req_capsuletoy_buy_egg, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    role->getCapsuletoyMgr()->buyCapsuletoy(req);
}}


hander_msg(req_capsuletoy_buy_n_egg, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    role->getCapsuletoyMgr()->buyNCapsuletoy(req);
}}




