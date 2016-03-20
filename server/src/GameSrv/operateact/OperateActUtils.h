//
//  OperateActUtils.h
//  GameSrv
//
//  Created by Huang Kunchao on 14-2-17.
//
//

#ifndef GameSrv_OperateActUtils_h
#define GameSrv_OperateActUtils_h


#define OperateActNotForThisServer_ThenContinue() \
int serverid = inifile.getValueT(*iter, "server_id", 0);\
int thisServerId = Process::env.getInt("server_id"); \
if( ALL_SERVER_ID != serverid && serverid != thisServerId) \
{\
    continue;\
}


#endif
