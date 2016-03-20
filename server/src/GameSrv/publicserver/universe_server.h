//
//  battle_server.h
//  GameSrv
//
//  Created by 丁志坚 on 15/1/13.
//
//

#ifndef __GameSrv__universe_server__
#define __GameSrv__universe_server__

#include <stdio.h>

#include "psmgr.h"

class UniverseServer : public PublicServer
{
public:
    virtual void onResponse(char* data, int len);
};

#endif /* defined(__GameSrv__battle_server__) */
