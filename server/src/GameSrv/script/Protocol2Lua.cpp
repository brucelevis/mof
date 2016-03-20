//
//  ProtocolExport.cpp
//  GameSrv
//
//  Created by 丁志坚 on 15/5/11.
//
//

#include "Protocol2Lua.h"
#include "NetPacket.h"
#include "msg.h"

static map<int, LuaPack> sProtoLuaPackers;
static map<string, LuaUnpack> sProtoLuaUnpackers;

static int
registerLuaPacker(int type, int id, LuaPack func) {
    int index = (type << 16) | id;
    sProtoLuaPackers[index] = func;
    return 0;
}

static int
registerLuaUnpacker(const char* name, LuaUnpack func) {
    sProtoLuaUnpackers[name] = func;
    return 0;
}

LuaPack
getProtoLuaPacker(int type, int id) {
    int index = (type << 16) | id;
    map<int, LuaPack>::iterator iter = sProtoLuaPackers.find(index);
    if (iter == sProtoLuaPackers.end()) {
        return NULL;
    }
    return (iter->second);
}

LuaUnpack
getProtoLuaUnpacker(const char* name) {
    map<string, LuaUnpack>::iterator iter = sProtoLuaUnpackers.find(name);
    if (iter == sProtoLuaUnpackers.end()) {
        return NULL;
    }
    return (iter->second);
}

//init
#define begin_cyclemsg(name) void luaPack_cyclemsg_##name(lua_State* state, void* param) {\
ByteArray& byteArray = *((ByteArray*)param); (void)byteArray; \
lua_newtable(state);

#define end_cyclemsg() }

#define def_int(name) lua_pushnumber(state, byteArray.read_int()); lua_setfield(state, -2, #name);
#define def_int64(name) lua_pushnumber(state, byteArray.read_int64()); lua_setfield(state, -2, #name);
#define def_float(name) lua_pushnumber(state, byteArray.read_float()); lua_setfield(state, -2, #name);

#define def_string(name) {\
string val = byteArray.read_string(); \
lua_pushstring(state, val.c_str()); \
lua_setfield(state, -2, #name);\
}

#define def_int_arr(name) do{\
lua_newtable(state); \
int len = byteArray.read_int();\
for(int i=0;i<len;++i){\
int temp = byteArray.read_int(); \
lua_pushinteger(state,temp);\
lua_rawseti(state, -2, i+1); \
} \
lua_setfield(state, -2, #name);\
}while(false);

#define def_float_arr(name) do{\
lua_newtable(state); \
int len = byteArray.read_int();\
for(int i=0;i<len;++i){\
float temp = byteArray.read_float(); \
lua_pushnumber(state,temp);\
lua_rawseti(state, -2, i+1); \
} \
lua_setfield(state, -2, #name);\
}while(false);

#define def_string_arr(name) do{\
lua_newtable(state); \
int len = byteArray.read_int(); \
for(int i = 0; i < len; ++i){ \
string temp = byteArray.read_string(); \
lua_pushstring(state, temp.c_str()); \
lua_rawseti(state, -2, i+1); \
} \
lua_setfield(state, -2, #name); \
}while(false);

#define def_object_arr(obj,name) do{ \
lua_newtable(state); \
int len = byteArray.read_int();\
for(int i=0;i<len;++i){\
luaPack_cycylemsg_##obj(state, &byteArray);\
lua_rawseti(state, -2, i+1);\
} \
lua_setfield(state, -2, #name);\
}while(false);

#include "cyclemsgdefine.h"

#undef begin_cyclemsg
#undef end_cyclemsg


#undef def_int
#undef def_int64
#undef def_float
#undef def_string

#undef def_int_arr
#undef def_float_arr
#undef def_string_arr
#undef def_object_arr





#define begin_msg(name,proto_type,msgid)  \
void name##_lua_packer(lua_State* state, void* param);\
static int register_ret_for_##name = registerLuaPacker(proto_type, msgid, name##_lua_packer);\
void \
name##_lua_packer(lua_State* state, void* param) {\
ByteArray& byteArray = *((ByteArray*)param);\
(void)byteArray;\
lua_pushstring(state, #name);\
lua_createtable(state, 0, 0);
#define end_msg() } \

#define def_err(name) {\
int val = byteArray.read_int(); \
lua_pushinteger(state, val); \
lua_setfield(state, -2, #name);\
}
#define def_int(name) {\
int val = byteArray.read_int(); \
lua_pushinteger(state, val); \
lua_setfield(state, -2, #name);\
}

#define def_int64(name) {\
int val = byteArray.read_int64(); \
lua_pushinteger(state, val); \
lua_setfield(state, -2, #name);\
}

#define def_float(name) {\
float val = byteArray.read_float(); \
lua_pushinteger(state, val); \
lua_setfield(state, -2, #name);\
}

#define def_string(name) {\
string val = byteArray.read_string(); \
lua_pushstring(state, val.c_str()); \
lua_setfield(state, -2, #name);\
}

#define def_blob(name)
#define def_object(type, name) {\
luaPack_cyclemsg_##type(state, &byteArray);\
lua_setfield(state, -2, #name);\
}

#define def_int_arr(name) do{\
lua_newtable(state); \
int len = byteArray.read_int();\
for(int i=0;i<len;++i){\
int temp = byteArray.read_int(); \
lua_pushinteger(state,temp);\
lua_rawseti(state, -2, i+1); \
} \
lua_setfield(state, -2, #name);\
}while(false);

#define def_int64_arr(name) do{\
lua_newtable(state); \
int len = byteArray.read_int();\
for(int i=0;i<len;++i){\
    int temp = byteArray.read_int64(); \
    lua_pushinteger(state,temp);\
    lua_rawseti(state, -2, i+1); \
} \
lua_setfield(state, -2, #name);\
}while(false);

#define def_float_arr(name) do{\
lua_newtable(state); \
int len = byteArray.read_int();\
for(int i=0;i<len;++i){\
float temp = byteArray.read_float(); \
lua_pushnumber(state,temp);\
lua_rawseti(state, -2, i+1); \
} \
lua_setfield(state, -2, #name);\
}while(false);


#define def_string_arr(name) do{\
lua_newtable(state); \
int len = byteArray.read_int(); \
for(int i = 0; i < len; ++i){ \
string temp = byteArray.read_string(); \
lua_pushstring(state, temp.c_str()); \
lua_rawseti(state, -2, i+1); \
} \
lua_setfield(state, -2, #name); \
}while(false);

#define def_object_arr(obj,name) do{ \
lua_newtable(state); \
int len = byteArray.read_int();\
for(int i=0;i<len;++i){\
luaPack_cyclemsg_##obj(state, &byteArray);\
lua_rawseti(state, -2, i+1);\
} \
lua_setfield(state, -2, #name);\
}while(false);

#include "msgdefine.h"

#undef begin_msg
#undef end_msg

#undef def_err
#undef def_int
#undef def_int64
#undef def_float
#undef def_string
#undef def_blob
#undef def_object

#undef def_int_arr
#undef def_int64_arr
#undef def_float_arr
#undef def_string_arr
#undef def_object_arr



#define begin_msg(name,proto_type,msgid)  \
void name##_lua_unpacker(lua_State* state, int index, void* param);\
static int register_popret_for_##name = registerLuaUnpacker(#name, name##_lua_unpacker);\
void \
name##_lua_unpacker(lua_State* state, int index, void* param) {\
ByteArray& byteArray = *((ByteArray*)param);\
(void)byteArray;\
static name emptyObj;\
emptyObj.INetPacket::build(byteArray);
#define end_msg() }

#define def_err(name) {\
lua_getfield(state, index, #name); \
int val = lua_tonumber(state, -1);\
lua_pop(state, 1);\
byteArray.write_int(val);\
}
#define def_int(name) {\
lua_getfield(state, index, #name); \
int val = lua_tonumber(state, -1);\
lua_pop(state, 1);\
byteArray.write_int(val);\
}

#define def_int64(name) {\
lua_getfield(state, index, #name); \
int val = lua_tonumber(state, -1);\
lua_pop(state, 1);\
byteArray.write_int(val);\
}

#define def_float(name) {\
lua_getfield(state, index, #name); \
float val = lua_tonumber(state, -1);\
lua_pop(state, 1);\
byteArray.write_float(val);\
}

#define def_string(name) {\
lua_getfield(state, index, #name); \
string val = lua_tostring(state, -1);\
lua_pop(state, 1);\
byteArray.write_string(val);\
}

#define def_blob(name)

#define def_object(type, name)

#define def_int_arr(name)
#define def_int64_arr(name)
#define def_float_arr(name)
#define def_string_arr(name)
#define def_object_arr(obj,name)

#include "msgdefine.h"

#undef begin_msg
#undef end_msg

#undef def_err
#undef def_int
#undef def_int64
#undef def_float
#undef def_string
#undef def_blob
#undef def_object

#undef def_int_arr
#undef def_int64_arr
#undef def_float_arr
#undef def_string_arr
#undef def_object_arr