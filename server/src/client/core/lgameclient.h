//
//  lgameclient.h
//  client
//
//  Created by xinyou on 14-5-28.
//
//

#ifndef __client__lgameclient__
#define __client__lgameclient__

#include <iostream>


#ifdef __cplusplus
extern "C"
{
#endif
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#ifdef __cplusplus
}
#endif

class GameClient;

void registerGameClient(lua_State* L);

void pushGameClient(lua_State* L, GameClient* client);

#endif /* defined(__client__lgameclient__) */
