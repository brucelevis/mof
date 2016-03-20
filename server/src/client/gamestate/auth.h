#ifndef CLIENT_GAMESTATE_AUTH_H_
#define CLIENT_GAMESTATE_AUTH_H_

#include "base_state.h"
#include "msg.h"
#include "INetPacket.h"


class AuthState : public BaseState
{
public:
    virtual void exec(const char* cmd)
    {
        StringArray sa = strSplit(cmd, " ");
        
        if (sa.size() == 0)
        {
            return;
        }
        
        string platform = sa[0];
        sa.erase(sa.begin());
        
        if (platform == "raw")
        {
            loginRaw(sa);
        }
        else if (platform == "xinyou")
        {
            loginXinyou(sa);
        }
    }
    
    void loginRaw(StringArray& sa);
    void loginXinyou(StringArray& sa);
};

#endif