//
//  PushNotifyMgr.h
//  GameSrv
//
//  Created by prcv on 14-10-31.
//
//

#ifndef __GameSrv__PushNotifyMgr__
#define __GameSrv__PushNotifyMgr__

void pushNotifyToRole(int roleId, const char* ticker, const char* title, const char* text);
void pushNotifyToAllRoles(const char* ticker, const char* title, const char* text);

#endif /* defined(__GameSrv__PushNotifyMgr__) */
