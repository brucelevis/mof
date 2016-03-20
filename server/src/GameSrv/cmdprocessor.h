//
//  cmdprocessor.h
//  GameSrv
//
//  Created by prcv on 13-6-18.
//
//

#ifndef GameSrv_cmdprocessor_h
#define GameSrv_cmdprocessor_h

#include "thread.h"
#include "MQ.h"
#include "cmd_def.h"

class CmdProcessor : public Runnable
{
public:
    
    bool init(message_queue* MQ)
    {
        mq = MQ;
        return true;
    }
    
    void run()
    {
        isstop = false;
        while (!isstop)
        {
            skynet_message msg;
            while (skynet_mq_pop(mq, &msg) == 0)
            {
                ICmdMsg* cmd = (ICmdMsg*)msg.data;
                cmd->handle();
                destroy_cmd(cmd);
            }
            
            Thread::sleep(100);
        }
    }
    
    void stop()
    {
        isstop = true;
    }
    
    bool isstop;
    message_queue* mq;
};

#endif
