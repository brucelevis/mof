//
//  gmserver.h
//  GameSrv
//
//  Created by prcv on 13-7-23.
//
//

#ifndef __GameSrv__gmserver__
#define __GameSrv__gmserver__

#include "psmgr.h"
class GmServer : public PublicServer
{
public:
    virtual void onResponse(int receiver, int cmdId, CMsgTyped* msg);
};


#endif /* defined(__GameSrv__gmserver__) */
