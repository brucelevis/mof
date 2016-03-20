//
//  NetExport.cpp
//  GameSrv
//
//  Created by 丁志坚 on 15/5/8.
//
//

#include "NetExport.h"
#include "lua_helper.h"
#include "Protocol2Lua.h"
#include "main.h"

int
lua_sendNetPacket(lua_State* state) {
    ByteArray byteArray;
    
    int sessionId = lua_tointeger(state, 1);
    string packetName = lua_tostring(state, 2);
    try {
        LuaUnpack unpacker = getProtoLuaUnpacker(packetName.c_str());
        LuaState::getValue(state, 3, unpacker, (void*)&byteArray);
        sendNetData(sessionId, byteArray.m_pContent, byteArray.m_nWrPtr);
        lua_pushinteger(state, 0);
    } catch (...) {
        lua_pushinteger(state, -1);
    }
    return 1;
}


void
exportNetLibrary(lua_State* L) {
    lua_register(L, "sendNetPacket", lua_sendNetPacket);
}