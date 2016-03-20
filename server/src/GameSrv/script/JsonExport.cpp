//
//  JsonExport.cpp
//  GameSrv
//
//  Created by 丁志坚 on 15/5/12.
//
//

#include "JsonExport.h"
#include "lua_helper.h"
#include "json/json.h"


void luaObject2Json(lua_State* l, Json::Value& value);
void luaType2Json(lua_State* l, Json::Value& value);
void luaArray2Json(lua_State* l, Json::Value& value);


void json2LuaType(Json::Value& value, lua_State* state);
void jsonObject2LuaType(Json::Value& value, lua_State* state);
void jsonArray2LuaType(Json::Value& value, lua_State* state);

bool
isArrayTable(lua_State* state) {
    int ret = true;
    int expectIndex = 1;
    int stackTop = lua_gettop(state);
    lua_pushnil(state);
    while (lua_next(state, -2)) {
        if (!lua_isnumber(state, -2)) {
            ret = false;
            break;
        }
        
        int curIndex = lua_tointeger(state, -2);
        if (curIndex != expectIndex) {
            ret = false;
            break;
        }
        
        expectIndex++;
        lua_pop(state, 1);
    }
    
    lua_settop(state, stackTop);
    return ret;
}

void
luaObject2Json(lua_State* l, Json::Value& value) {
    lua_pushnil(l);
    while (lua_next(l, -2)) {
        Json::Value childVal;
        luaType2Json(l, childVal);
        lua_pushvalue(l, -2);
        string key = lua_tostring(l, -1);
        value[key] = childVal;
        
        lua_pop(l, 2);
    }
}

void
luaArray2Json(lua_State* l, Json::Value& value) {
    lua_pushnil(l);
    while (lua_next(l, -2)) {
        Json::Value childVal;
        luaType2Json(l, childVal);
        value.append(childVal);
        lua_pop(l, 1);
    }
}

void
luaType2Json(lua_State* l, Json::Value& value) {
    switch (lua_type(l, -1)) {
        case LUA_TSTRING: {
            value = Json::Value(lua_tostring(l, -1));
            break;
        }
        case LUA_TNUMBER: {
            double dVal = lua_tonumber(l, -1);
            if (dVal == (int)dVal) {
                value = Json::Value(int(dVal));
            } else {
                value = Json::Value(dVal);
            }
            break;
        }
        case LUA_TBOOLEAN: {
            string strVal = lua_toboolean(l, -1) ? "true" : "false";
            value = Json::Value(strVal);
            break;
        }
        case LUA_TTABLE: {
            bool isArray = isArrayTable(l);
            if (isArray) {
                luaArray2Json(l, value);
            } else {
                luaObject2Json(l, value);
            }
            break;
        }
        default:
            break;
    }
}

void
json2LuaType(Json::Value& value, lua_State* state) {
    int type = value.type();
    switch (type) {
        case Json::intValue:
        case Json::uintValue:
            lua_pushinteger(state, value.asUInt());
            break;
        case Json::realValue:
            lua_pushnumber(state, value.asDouble());
            break;
        case Json::objectValue:
            lua_createtable(state, 0, 0);
            jsonObject2LuaType(value, state);
            break;
        case Json::arrayValue:
            lua_createtable(state, 0, 0);
            jsonArray2LuaType(value, state);
            break;
        default:
            break;
    }
}


void
jsonObject2LuaType(Json::Value& value, lua_State* state) {
    Json::Value::Members members( value.getMemberNames() );
    for ( Json::Value::Members::iterator it = members.begin();
         it != members.end();
         ++it )
    {
        const std::string &name = *it;
        json2LuaType(value[name], state);
        lua_setfield(state, -2, name.c_str());
    }
}

void
jsonArray2LuaType(Json::Value& value, lua_State* state) {
    int size = value.size();
    for (int i = 1; i <= size; ++i) {
        json2LuaType(value[i - 1], state);
        lua_rawseti(state, -2, i);
    }
}

int
lua_jsonEncode(lua_State* state) {
    if (!lua_istable(state, -1)) {
        lua_pushstring(state, "");
        return 1;
    }
    
    Json::Value value;
    luaType2Json(state, value);
    Json::FastWriter writer;
    string ret = writer.write(value);
    lua_pushstring(state, ret.c_str());
    
    return 1;
}

int
lua_jsonDecode(lua_State* state) {
    
    string jsonStr;
    Json::Value value;
    Json::Reader reader;
    
    if (!lua_isstring(state, -1)) {
        goto FAIL;
    }
    
    jsonStr = lua_tostring(state, -1);
    if (!reader.parse(jsonStr, value)) {
        goto FAIL;
    }
    
    if (!value.isObject() && !value.isArray()) {
        goto FAIL;
    }
    
    json2LuaType(value, state);
    
    return 1;
FAIL:
    lua_pushnil(state);
    return 1;
}

bool
exportJson(lua_State* state) {
    lua_register(state, "jsonEncode", lua_jsonEncode);
    lua_register(state, "jsonDecode", lua_jsonDecode);
    return true;
}