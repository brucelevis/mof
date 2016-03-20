//
//  gameclient.cpp
//  client
//
//  Created by xinyou on 14-5-28.
//
//

#include "gameclient.h"
#include "msg.h"


void GameClient::login()
{
    req_login req;
    req.account = mAccount;
    req.password = mPassword;
    sendRequest(&req);
}



void GameClient::callScript(lua_State *L, const char* func)
{
    lua_getglobal(L, func);
    GameClient** userdata = (GameClient**)lua_newuserdata(L, sizeof(GameClient*));
    *userdata = this;
    luaL_getmetatable(L, "mof_GameClient");
    lua_setmetatable(L, -2);
    int ret = lua_pcall(L, 1, LUA_MULTRET, 0);
    if (ret != LUA_OK){
        printf("exec lua func:%s with error: %s\n", "doLogic", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
}

void GameClient::worldChat(const char *message)
{
    printf("world chat\n");
    
    req_worldchat req;
    req.message = message;
    sendRequest(&req);
}

void GameClient::onMessage(const char *message)
{
}