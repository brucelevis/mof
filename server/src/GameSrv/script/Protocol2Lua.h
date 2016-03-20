//
//  ProtocolExport.h
//  GameSrv
//
//  Created by 丁志坚 on 15/5/11.
//
//

#ifndef __GameSrv__ProtocolExport__
#define __GameSrv__ProtocolExport__

#include "lua_helper.h"

LuaPack getProtoLuaPacker(int type, int id);
LuaUnpack getProtoLuaUnpacker(const char* name);


#endif /* defined(__GameSrv__ProtocolExport__) */
