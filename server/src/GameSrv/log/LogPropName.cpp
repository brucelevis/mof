//
//  LogPropName.cpp
//  GameSrv
//
//  Created by jin on 13-12-18.
//
//
#include <map>
#include <string>
using namespace std;

static std::map<int, string> s_propname;
int registerLogType(int id, string name)
{
	s_propname[id] = name;
	return 0;
}

#define LogPropNameBegin() 
#define LogPropName(id, name, value) int name##ret = registerLogType(id, #value);
#define LogPropNameEnd() 

#include "LogPropName.h"

const char* GetLogPropName(int eName)
{
    return s_propname[eName].c_str();
}
