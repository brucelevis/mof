//
//  weapon_enchant_hand.cpp
//  GameSrv
//
//  Created by haifeng on 14-11-5.
//
//

#include "hander_include.h"
#include "WeaponEnchant.h"
#include "RedisHash.h"

// 获取武器附魔系统状态
hander_msg(req_enchant_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    role->getWeaponEnchantMgr()->SendWeaponEnchantState();
}}

// 随机附魔
hander_msg(req_random_enchant, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    role->getWeaponEnchantMgr()->SendRandomWeaponEnchantResult(req.continueNum);
}}

// 指定附魔
hander_msg(req_sure_enchant, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
	ack_sure_enchant ack;
    role->getWeaponEnchantMgr()->SendSureWeaponEnchantResult(req.enchantId, req.continueNum, ack);
	role->send(&ack);
}}

//可以自由选择 满级附魔
hander_msg(req_choose_enchant, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    role->getWeaponEnchantMgr()->ChooseWeaponEnchant(req.enchantId);
}}

