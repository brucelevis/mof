//
//  threadpool.h
//  GameSrv
//
//  Created by prcv on 13-6-28.
//
//

#pragma once

#include "thread.h"
#include <stdio.h>

class ThreadPool
{
public:
    ThreadPool()
    {
    }
    
    void init(int num)
    {
        threads = new Thread[num];
        threadnum = num;
    }
    
    void start(::Runnable *tar)
    {
        for (int i = 0; i < threadnum; i++)
        {
            if (!threads[i].start(tar))
            {
                printf("create the %dth thread fail\n", i + 1);
            }
        }
    }
    
    void join()
    {
        for (int i = 0; i < threadnum; i++)
        {
            threads[i].join();
        }
    }
    
    
    int threadnum;
    Thread *threads;
    ::Runnable *runtar;
};
