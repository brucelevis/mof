//
//  server.cpp
//  libgate
//
//  Created by prcv on 13-7-5.
//  Copyright (c) 2013年 prcv. All rights reserved.
//

#include "tcp_listener.h"

#include <assert.h>

#include "tcp_connection.h"

TcpListener::TcpListener()
{
    _connections = NULL;
}

TcpListener::~TcpListener()
{
    if (_fd > 0)
    {
        close();
    }
}

bool TcpListener::init(Gate* gate, int id, const char* host, unsigned short port, int maxcon)
{
    _fd = -1;
    _gate = gate;
    strcpy(_host, host);
    _port = port;
    _id = id;
    _maxcon = maxcon;
    _index = 0;
    _curcon = 0;
    _cap = 1;
    while (_cap < _maxcon)
    {
        _cap <<= 1;
    }
    _connections = new TcpConnection* [_cap];
    for (int i = 0; i < _cap; i++)
    {
        _connections[i] = new TcpConnection;
    }
    
    return listen();
}

bool TcpListener::listen()
{
    int fd = Net::create_listener(_host, _port, 64);
    if (fd == -1)
    {
        return false;
    }
    _gate->get_poller()->add_fd(fd, POLLER_IN, this);
    _fd = fd;
    return true;
}

void TcpListener::close()
{
    for (int i = 0; i < _cap; i++)
    {
        int fd = _connections[i]->getFd();
        if (fd > 0)
        {
            _gate->get_poller()->rm_fd(fd, -1);
            _connections[i]->close();
        }
        delete _connections[i];
    }
    delete[] _connections;
    _connections = NULL;
    
    _gate->get_poller()->rm_fd(_fd, -1);
    ::close(_fd);
    _fd = -1;
}

void TcpListener::in_event()
{
    unsigned int sid = generate_id();
    if (sid == 0)
    {
        return;
    }
    
    sockaddr addr;
    socklen_t socklen = sizeof(sockaddr);
    int clientfd = accept(_fd, (sockaddr*)&addr, &socklen);
    Net::set_nonblocking(clientfd);
    
    char ip[64] = "";
    void* data = NULL;
    if (addr.sa_family == AF_INET){
        data = &((sockaddr_in*)&addr)->sin_addr;
    }
    else if(addr.sa_family == AF_INET6){
        data = &((sockaddr_in6*)&addr)->sin6_addr;
    }
    if (data){
        inet_ntop(addr.sa_family, data, ip, socklen);
    }
    
    create_connection(sid, clientfd);
    if (_reactor)
    {
        _reactor->onEvent(sid, IoEventAccept, ip, strlen(ip) + 1);
    }
}

unsigned int TcpListener::generate_id()
{
    if (_curcon >= _maxcon)
    {
        return 0;
    }
    
    int slot;
    for (int i = 0; i < _maxcon; i++)
    {
        slot = (_index + i) % _cap;
        if (_connections[slot]->getFd() == -1)
        {
            _index += i;
            _index &= 0xFFFFFF;
            
            unsigned int sid = (_id << 24);
            return (sid | _index++);
        }
    }
    
    assert(0);
}


TcpConnection* TcpListener::get_connection(int sid)
{
    int slot = sid & (_cap - 1);
    if (slot < _cap && _connections[slot]->get_sid() == sid)
    {
        return _connections[slot];
    }
    
    return NULL;
}


void TcpListener::create_connection(int sid, int fd)
{
    int slot = sid & (_cap - 1);
    TcpConnection* conn = _connections[slot];
    conn->init(sid, fd, this);
    get_poller()->add_fd(fd, POLLER_IN, conn);
    _curcon++;
}

void TcpListener::close_connection(int sid)
{
    int slot = sid & (_cap - 1);
    TcpConnection* conn = _connections[slot];
    if (conn->get_sid() == sid)
    {
        get_poller()->rm_fd(conn->getFd(), -1);
        conn->close();
        _curcon--;
    }
    
    _reactor->onEvent(sid, IoEventClose, NULL, 0);
}
