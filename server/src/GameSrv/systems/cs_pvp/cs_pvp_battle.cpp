#include "cs_pvp_battle.h"
#include "Role.h"
#include "Game.h"
#include "RedisHashObj.h"

CsPvpState*
CsPvpState::create(Role* role) {
    CsPvpState* state = new CsPvpState;
    state->mOwner = role;
    return state;
}

CsPvpState::CsPvpState() {
    mState = CsPvpState::kCsPvpNone;
}

int
CsPvpState::getState() {
    return mState;
}

int
CsPvpState::getChallengeTime() {
    RoleCfgDef& roleCfgDef = RoleCfg::getCfg(mOwner->getJob(), mOwner->getLvl());
    int cspvpUsedFreeTimes = mOwner->getCsPvpStateData().getUsedFreeTimes();
    int cspvpRemainFreeTimes = roleCfgDef.mCrossServiceWarFreeTimes - cspvpUsedFreeTimes;
    return cspvpRemainFreeTimes + mOwner->getCsPvpStateData().getRmbTimes();
}


void
CsPvpState::pendSelectTarget(int serverId, int roleId) {
    mState = CsPvpState::kSelectChallengeRole;
    mTargetRoleId = roleId;
    mTargetServerId = serverId;
    mTime = Game::tick;
}

void
CsPvpState::getTargetBattle() {
    mState = CsPvpState::kGetTargetBattle;
    mTime = Game::tick;
}

void
CsPvpState::beginBattle() {
    mState = CsPvpState::kInBattle;
    mTime = Game::tick;
    
    mOwner->getCsPvpStateData().consumeTime();
    mOwner->save();

    notify_syn_cspvptimes notify;
    notify.times = getChallengeTime();
    mOwner->send(&notify);
}

void
CsPvpState::submitBattleResult() {
    mState = CsPvpState::kSubmitBattleResult;
    mTime = Game::tick;
}

void
CsPvpState::endPvp() {
    mState = CsPvpState::kCsPvpNone;
}
