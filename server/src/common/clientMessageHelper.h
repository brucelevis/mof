#pragma once

bool c_handleClientMsg(int session, void* data, int len);
bool lua_handleClientMsg(int session, void* data, int len);