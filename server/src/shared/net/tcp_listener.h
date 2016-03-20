//
//  server.h
//  libgate
//
//  Created by prcv on 13-7-5.
//  Copyright (c) 2013年 prcv. All rights reserved.
//

#pragma once


#include "gate.h"
#include "net.h"
#include "socket.h"
#include "poller.h"

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

class TcpConnection;
class Gate;

class TcpListener : public IoEvent
{
public:
    TcpListener();
    ~TcpListener();
    
    bool init(Gate* gate, int id, const char* host, unsigned short port, int maxcon);
    
	bool listen();
    
    void close();
	
	void in_event();
    
    Poller* get_poller()
    {
        return _gate->get_poller();
    }
    
    void set_cb(IoEventReactor* reactor)
    {
        _reactor = reactor;
    }
    
    IoEventReactor* get_cb()
    {
        return _reactor;
    }
    
    int getIoType()
    {
        return IoListener;
    }
    
    TcpConnection* get_connection(int sid);
    void create_connection(int sid, int fd);
    void close_connection(int sid);

private:
    
    char             _host[64];
    unsigned short   _port;
    int              _fd;
    Gate*            _gate;
    unsigned int     _id;
    IoEventReactor*  _reactor;
    
    
    int              _index;
    int              _cap;
    int              _curcon;
    int              _maxcon;
    TcpConnection**  _connections;
    unsigned int generate_id();
};

