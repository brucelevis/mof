//
//  mailprotocol.cpp
//  GameSrv
//
//  Created by prcv on 13-7-19.
//
//

#include "mailserver.h"
#include "MsgBuf.h"
#include "main.h"
#include "psmgr.h"
#include "mail_imp.h"
#include "Game.h"

void MailServer::onRegister()
{
    onRegisterMailServerSuccess();
}

void MailServer::onResponse(int receiver, int cmdId, CMsgTyped* msg)
{
    switch (cmdId) {
        case GM_SEND_MAIL:
        {
            onSendMail(receiver, msg);
            break;
        }
        case GM_MAIL_LIST:
        {
            onGetMailList(receiver, msg);
            break;
        }
        case GM_READ_MAIL:
        {
            onGetMail(receiver, msg);
            break;
        }
        case GM_GET_MAIL_ATTACHMENT:
        {
            onGetAttach(receiver, msg);
            break;
        }
        case GM_DELETE_MAIL:
        {
            onDeleteMail(receiver, msg);
            break;
        }
            
        default:
            break;
    }
}
