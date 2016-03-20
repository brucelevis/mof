#pragma once

#include <string>
#include "cyclemsg.h"
#include "Role.h"
using namespace std;

class Role;

#define CSPVP_SCENEID 501238

class CsPvpState
{
public:
    static CsPvpState* create(Role* role);
    
    enum {
        kCsPvpNone,
        kSelectChallengeRole,
        kGetTargetBattle,
        kInBattle,
        kSubmitBattleResult,
    };
    
    CsPvpState();
    
    bool haveChallengeTime();
    int getChallengeTime();
    
    int getState();
    void pendSelectTarget(int serverId, int roleId);
    void getTargetBattle();
    void beginBattle();
    void submitBattleResult();
    void endPvp();

private:
    int mState;
    int mTargetServerId;
    int mTargetRoleId;
    int mTime;
    Role* mOwner;
};
