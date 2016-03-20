//
//  DBRolePropName.cpp
//  GameSrv
//
//  Created by 志坚 on 13-3-13.
//  Copyright (c) 2013年 __MyCompanyName__. All rights reserved.
//


#include <iostream>


#define PropTypeToValueBegin() static const char* s_propname[] = {

#define PropTypeToValue(name, value)  #value,

#define PropTypeToValueEnd()  };


#include "DBRolePropName.h"

const char* GetRolePropName(int type)
{
    if (type >= sizeof(s_propname) / sizeof(const char*) || type < 0)
    {
        return NULL;
    }
 
    return s_propname[type];
}