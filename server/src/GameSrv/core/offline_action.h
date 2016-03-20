//
//  offline_action.h
//  GameSrv
//
//  Created by prcv on 13-12-12.
//
//

#ifndef __GameSrv__offline_action__
#define __GameSrv__offline_action__

#include <iostream>

#include "Role.h"

void procRoleOfflineAction(Role* role);
void addRoleOfflineAction(int roleid, const char* cmd, vector<string>& params);

#endif /* defined(__GameSrv__offline_action__) */
