//
//  Paihang_helper.h
//  GameSrv
//
//  Created by pireszhi on 13-7-1.
//
//

#ifndef __GameSrv__Paihang_helper__
#define __GameSrv__Paihang_helper__

#include <iostream>
#include <map>
#include "Pvp.h"

struct IGlobalMsg
{
public:
    virtual void handle(void* obj = NULL){};
    virtual ~IGlobalMsg(){}
};


#define begin_global(name) struct globalmsg_##name : public IGlobalMsg{ virtual void handle(void* obj = NULL);
#define global_member(type, name) type name;
#define global_arr_member(type, name) vector<type> name;
#define end_global() };

#define create_global(name, var) globalmsg_##name* var = new globalmsg_##name
#define destroy_global(var) delete var;
#define handler_global(name)  void globalmsg_##name::handle(void* obj)

#endif /* defined(__GameSrv__Paihang_helper__) */
