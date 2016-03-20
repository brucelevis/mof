//
//  LogBase.h
//  GameSrv
//
//  Created by prcv on 14-6-10.
//
//

#ifndef __GameSrv__LogBase__
#define __GameSrv__LogBase__

#include <iostream>
#include <string>
#include "Utils.h"
#include "MsgBuf.h"
#include "log_server.h"
using namespace std;

class BaseLog
{
protected:
    BaseLog(){}
    virtual void specSave() {}
    virtual const char* getName() {return "xylog";}
    void addField(const string& val);
    void addField(int val);
    void addField(unsigned int val);
    void addField(double val);
    void addField(size_t val);
    void addField(time_t val);
    virtual ~BaseLog(){save();}
    
private:
    void save();
    CMsgTyped mMsg;
};

#endif /* defined(__GameSrv__LogBase__) */
