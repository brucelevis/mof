//
//  mail_handler.cpp
//  GameSrv
//
//  Created by prcv on 13-7-19.
//
//

#include "hander_include.h"
#include "mail_imp.h"

handler_msg(req_get_mail_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (!getMailList(roleid, role->getRolename().c_str(), req.from, req.count))
    {
        ack_get_mail_list ack;
        ack.errorcode = 1;
        sendNetPacket(sessionid, &ack);
    }
}}

handler_msg(req_get_mail, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (!getMail(roleid, req.mailid.c_str(), role->getRolename().c_str(), req.mailtitle.c_str()))
    {
        ack_get_mail ack;
        ack.errorcode = 1;
        sendNetPacket(sessionid, &ack);
    }
}}

handler_msg(req_delete_mail, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    if (!deleteMail(roleid, req.mailid.c_str(), role->getRolename().c_str()))
    {
        ack_delete_mail ack;
        ack.errorcode = 1;
        sendNetPacket(sessionid, &ack);
    }
}}

handler_msg(req_get_attach, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    string attach = req.attach;
    string mailid = req.mailid;
    
    if (attach == "")
    {
        if (getMail(roleid, req.mailid.c_str(), role->getRolename().c_str(), req.mailtitle.c_str()))
        {
            role->setProperty("waitmail", req.mailid);
            return;
        }
    }
    else
    {
        vector<string> itemstrs = StrSpilt(attach, ";");
        ItemArray items;
        RewardStruct reward;
        rewardsCmds2ItemArray(itemstrs, items, reward);
        GridArray grids;
        if(role->preAddItems(items, grids) == CE_OK)
        {
            if (getAttach(roleid, req.mailid.c_str(), role->getRolename().c_str(), req.mailtitle.c_str()))
            {
                return;
            }
        }
    }
    
    ack_get_attach ack;
    ack.errorcode = CE_BAG_FULL;
    sendNetPacket(sessionid, &ack);
}}
