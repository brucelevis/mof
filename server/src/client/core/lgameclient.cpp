//
//  lgameclient.cpp
//  client
//
//  Created by xinyou on 14-5-28.
//
//

#include "lgameclient.h"

#include <string>

#include "gameclient.h"

using namespace std;



static int l_GameClient_constructor(lua_State* L)
{
    GameClient** client = (GameClient**)lua_newuserdata(L, sizeof(GameClient*));
    *client = new GameClient;
    luaL_getmetatable(L, "mof_GameClient");
    lua_setmetatable(L, -2);
    return 1;
}

static GameClient* l_checkGameClient(lua_State* L, int n)
{
    return *(GameClient**)luaL_checkudata(L, n, "mof_GameClient");
}

static int l_GameClient_login(lua_State* L)
{
    GameClient* client = l_checkGameClient(L, 1);
    client->login();
    return 0;
}

static int l_GameClient_worldChat(lua_State* L)
{
    GameClient* client = l_checkGameClient(L, 1);
    string message = luaL_checkstring(L, 2);
    client->worldChat(message.c_str());
    return 0;
}

static int l_GameClient_destructor(lua_State* L)
{
    GameClient* client = l_checkGameClient(L, 1);
    delete client;
    
    printf("destructor\n");
    
    return 0;
}

void registerGameClient(lua_State* L)
{
    luaL_Reg gameClientRegs[] = {
        {"login", l_GameClient_login},
        {"worldChat", l_GameClient_worldChat},
        {NULL, NULL}
    };
    
    luaL_newmetatable(L, "mof_GameClient");
    luaL_setfuncs(L, gameClientRegs, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    lua_setglobal(L, "GameClient");
}

void pushGameClient(lua_State *L, GameClient* client)
{
    GameClient** userdata = (GameClient**)lua_newuserdata(L, sizeof(GameClient*));
    *userdata = client;
    luaL_getmetatable(L, "mof_GameClient");
    lua_setmetatable(L, -2);
}

