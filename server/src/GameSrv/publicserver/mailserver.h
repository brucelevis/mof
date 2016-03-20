//
//  mailprotocol.h
//  GameSrv
//
//  Created by prcv on 13-7-19.
//
//

#ifndef __GameSrv__mailprotocol__
#define __GameSrv__mailprotocol__

#include "psmgr.h"

class MailServer : public PublicServer
{
public:
    virtual void onResponse(int receiver, int cmdId, CMsgTyped* msg);
    virtual void onRegister();
};


#endif /* defined(__GameSrv__mailprotocol__) */
