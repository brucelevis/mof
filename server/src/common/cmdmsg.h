//
//  cmdmsg.h
//  GameSrv
//
//  Created by 丁志坚 on 12/26/14.
//
//

#ifndef GameSrv_cmdmsg_h
#define GameSrv_cmdmsg_h

#include <stdio.h>
class MsgObject;
struct ICmdMsg
{
public:
    virtual void handle(void* obj = NULL) = 0;
    virtual ~ICmdMsg(){}
};

#endif
