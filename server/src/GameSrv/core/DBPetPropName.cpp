//
//  DBPetPropName.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-7-8.
//
//
#include <iostream>

#define PetPropTypeToValueBegin() static const char* s_propname[] = {

#define PetPropTypeToValue(name, value)  #value,

#define PetPropTypeToValueEnd()  };


#include "DBPetPropName.h"

const char* g_GetPetPropName(int type)
{
    if (type >= sizeof(s_propname) / sizeof(const char*) || type < 0)
    {
        return NULL;
    }
    
    return s_propname[type];
}