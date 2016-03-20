//
//  log_server.h
//  GameSrv
//
//  Created by pireszhi on 13-7-22.
//
//

#ifndef __GameSrv__log_server__
#define __GameSrv__log_server__

#include <iostream>
#include "psmgr.h"
class CMsgTyped;

class LogServer : public PublicServer
{
public:
    virtual void onResponse(int receiver, int cmdId, CMsgTyped* msg);
};
#endif /* defined(__GameSrv__log_server__) */
