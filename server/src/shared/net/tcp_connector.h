//
//  connector.h
//  libgate
//
//  Created by prcv on 13-7-7.
//  Copyright (c) 2013年 prcv. All rights reserved.
//

#pragma once

#include "net.h"
#include "poller.h"
#include "gate.h"
#include "socket.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

const int CONNECTOR_SBUF_SIZE = (8 * 1024 * 1024);
const int CONNECTOR_RBUF_SIZE = (8 * 1024 * 1024);

class TcpConnector : public IoEvent, public Socket
{
public:
    enum{reconnect_timer_id, };
    
    TcpConnector() : Socket(CONNECTOR_RBUF_SIZE, CONNECTOR_SBUF_SIZE)
    {}
    
    bool init(Gate*  gate, int sid,  const char* host, unsigned short port)
    {
        strcpy(_host, host);
        _port = port;
        _gate = gate;
        _sid = sid;
        
        return connect();
    }
    
    ~TcpConnector()
    {
        if (_fd > 0)
        {
            close();
        }
    }
    
    bool connect()
    {
        int fd = Net::async_connect(_host, _port, _connected);
        if (fd == -1)
        {
            reconnect();
            return false;
        }
        
        setFd(fd);
        if (!_connected)
        {
            _gate->get_poller()->add_fd(_fd, POLLER_OUT, this);
        }
        else
        {
            _gate->get_poller()->add_fd(_fd, POLLER_IN, this);
            _reactor->onEvent(_sid, IoEventConnect, (char*)1, sizeof(char*));
        }
        
        return true;
    }
    
    void in_event()
    {
        if (_fd == -1 || !_connected)
        {
            return;
        }
        
        int ret;
        int needlen;
        
        for (;;)
        {
            if (_packlen == 0)
            {
                needlen = 4 - _recvlen;
            }
            else
            {
                needlen = _packlen + 4 - _recvlen;
            }
            
            ret = readImp(_recvbuf + _recvlen, needlen);
            if (ret <= 0)
            {
                break;
            }
            
            _recvlen += ret;
            if (ret < needlen)
            {
                break;
            }
            
            if (_packlen == 0)
            {
                memcpy(&_packlen, _recvbuf, 4);
                if (_packlen > _recvbufsize || _packlen < 8)
                {
                    close();
                }
            }
            else
            {
                _reactor->onEvent(_sid, IoEventRead, _recvbuf + 4, _packlen);
                _recvlen = 0;
                _packlen = 0;
            }
        }

        if (ret < 0)
        {
            reconnect();
            _reactor->onEvent(_sid, IoEventClose, NULL, 0);
        }
    }
    
    void reconnect()
    {
        close();
        _gate->get_poller()->add_timer(5000, reconnect_timer_id, this);
    }
    
    int close()
    {
        _gate->get_poller()->rm_fd(_fd, -1);
        ::close(_fd);
        resetFd();
        _connected = false;
        return 0;
    }
    
    void out_event()
    {
        if (_fd == -1)
        {
            return;
        }
        
        if (!_connected)
        {
            sockaddr_in addr;
            socklen_t socklen = sizeof(addr);
            if (getpeername(_fd, (sockaddr*)&addr, &socklen) == 0)
            {
                _gate->get_poller()->mod_fd(_fd, POLLER_IN, this);
                _connected = true;
                _reactor->onEvent(_sid, IoEventConnect, (char*)1, sizeof(char*));
            }
            else
            {
                reconnect();
                _reactor->onEvent(_sid, IoEventConnect, (char*)0, sizeof(char*));
            }
            return;
        }
        
        if (process_sendbuf() == -1)
        {
            _reactor->onEvent(_sid, IoEventClose, (char*)0, sizeof(char*));
            reconnect();
        }
        else if (_sendlen == 0)
        {
            _gate->get_poller()->mod_fd(_fd, POLLER_IN, this);
        }
    }
    
    
    void write(char* data, int len)
    {
        if (!_connected)
        {
            return;
        }
        
        int ret = writeImp(data, len);
        if (ret == -1)
        {
            _reactor->onEvent(_sid, IoEventClose, (char*)0, sizeof(char*));
            reconnect();
        }
        else if (_sendlen > 0)
        {
            _gate->get_poller()->mod_fd(_fd, POLLER_IN | POLLER_OUT, this);
        }
    }

    void timer_event(int timerid)
    {
        if (timerid == reconnect_timer_id)
        {
            connect();
        }
    }
    
    void set_cb(IoEventReactor* reactor)
    {
        _reactor = reactor;
    }
    
    IoEventReactor* get_cb()
    {
        return _reactor;
    }
 
private:
    char              _host[64];
    unsigned short    _port;
    int               _sid;
    
    bool              _connected;
    Gate*             _gate;
    
    IoEventReactor*   _reactor;
};

