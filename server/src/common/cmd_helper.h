//
//  cmd_helper.h
//  GameSrv
//
//  Created by prcv on 13-5-2.
//
//

#ifndef GameSrv_cmd_helper_h
#define GameSrv_cmd_helper_h

#include <map>
#include <stdio.h>
#include <vector>

#include "cmdmsg.h"

using namespace std;


#define begin_cmd(name) struct cmdmsg_##name : public ICmdMsg{ virtual void handle(void* obj = NULL);
#define cmd_member(type, name) type name;
#define cmd_array_member(type, name, num) type name[num];
#define end_cmd() };

#define create_cmd(name, var) cmdmsg_##name* var = new cmdmsg_##name
#define destroy_cmd(var) delete var;
#define handler_cmd(name)  void cmdmsg_##name::handle(void* obj)

#endif
