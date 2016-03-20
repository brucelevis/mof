//
//  poller.h
//  libgate
//
//  Created by prcv on 13-6-30.
//  Copyright (c) 2013年 prcv. All rights reserved.
//

#ifndef __libgate__poller__
#define __libgate__poller__

#include "net.h"
#include <string.h>
#include <vector>
#include <list>
#include <map>
using namespace std;

enum PollerEvent
{
    POLLER_IN = 1,
    POLLER_OUT = 2,
};

enum IoType
{
    IoListener,
    IoConnection,
    IoConnector,
};

class Poller;
class IoEvent;

class IoEvent
{
public:
	virtual void in_event() {}
	virtual void out_event() {}
    virtual void timer_event(int id) {}
    
    virtual ~IoEvent() {}
};

class Poller
{
public:
    
	virtual void poll() = 0;
	virtual bool init() = 0;
    virtual bool add_fd(int fd, int mask, IoEvent* udata)= 0;
	virtual void rm_fd(int fd, int mask)= 0;
    virtual bool mod_fd(int fd, int mask, IoEvent* udata) = 0;
    virtual ~Poller() { }
    static Poller* create();
    
    void add_timer(int timeout, int timerid, IoEvent* udata);
    void cancel_timer(int timeout, int timerid, IoEvent* udata);
    int execute_timers();
    struct timer_info_t
    {
        IoEvent* ioevent;
        int id;
        uint64_t timeout;
    };

    typedef std::list <timer_info_t> timers_t;

    timers_t timers;
};


#endif /* defined(__libgate__poller__) */
