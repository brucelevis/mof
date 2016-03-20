//
//  push_notify_mgr.h
//  GameSrv
//
//  Created by 丁志坚 on 11/21/14.
//
//

#ifndef __GameSrv__push_notify_mgr__
#define __GameSrv__push_notify_mgr__

#include <stdio.h>

#include "push_notify_cfg.h"

class PushNotify
{
public:
    
    int    mDaySec;
    
    string mTicker;
    string mTitle;
    string mContent;
};

class PushNotifyMgr
{
public:
    
    void update();
    
    vector<PushNotify> mNotifies;
};

#endif /* defined(__GameSrv__push_notify_mgr__) */
