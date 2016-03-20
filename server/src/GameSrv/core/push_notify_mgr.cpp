//
//  push_notify_mgr.cpp
//  GameSrv
//
//  Created by 丁志坚 on 11/21/14.
//
//

#include "push_notify_mgr.h"
#include "Game.h"


void PushNotifyMgr::update()
{
    time_t curTime = Game::tick;
    int daySec = curTime % (3600 * 24);
    
    for (vector<PushNotify>::iterator iter = mNotifies.begin(); iter != mNotifies.end(); ) {
        PushNotify pushNotify = *iter;
        
        int secDiff = pushNotify.mDaySec - daySec;
        if (secDiff > 5) {
            iter++;
            continue;
        }
        
        mNotifies.erase(iter++);
        if (secDiff < -5) {
            continue;
        }
        
        //push notify
        
    }
}