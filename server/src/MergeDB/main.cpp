//
//  main.cpp
//  MergeDB
//
//  Created by pireszhi on 14-1-13.
//  Copyright (c) 2014年 pireszhi. All rights reserved.
//

#include <iostream>
#include "dbconnector.h"
#include "log.h"
#include "Utils.h"
#include "Worker.h"
int main(int argc, const char * argv[])
{

    // insert code here...
    log_init("logPath.ini", GetAppPath());
    
    log_info("合并初始化");

    if(!g_DBMgr.initMgr())
    {
        return 0;
    }

    if (!_WORKERMGR.init()) {
        return 0;
    }
    string cmd = "";

    log_info("开始合并操作");
    time_t begintime = time(NULL);

    _WORKERMGR.doMerge();

    time_t costtime = time(NULL) - begintime;
    log_info("耗时:"<<(int)costtime<<"s");


    return 0;
}

