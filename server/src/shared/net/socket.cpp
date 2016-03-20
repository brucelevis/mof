//
//  socket.cpp
//  libgate
//
//  Created by prcv on 13-7-7.
//  Copyright (c) 2013年 prcv. All rights reserved.
//

#include "socket.h"

#include <string.h>

#define RECV_BUF_SIZE (32 * 1024)
#define SEND_BUF_SIZE (64 * 1024)

Socket::Socket()
{
    _fd = -1;
    
    _sendbuf = new char[SEND_BUF_SIZE];
    _sendbufsize = SEND_BUF_SIZE;
    _sendbufstart = _sendlen = 0;
    
    _recvbuf = new char[RECV_BUF_SIZE];
    _recvbufsize = RECV_BUF_SIZE;
    _recvlen = 0;
    _packlen = 0;
    
    mUserData = 0;
}


Socket::Socket(int recvBufSize, int sendBufSize)
{
    _fd = -1;
    
    _sendbuf = new char[sendBufSize];
    _sendbufsize = sendBufSize;
    _sendbufstart = _sendlen = 0;
    
    _recvbuf = new char[recvBufSize];
    _recvbufsize = recvBufSize;
    _recvlen = 0;
    _packlen = 0;
}


void Socket::setSendBufSize(int sendBufSize) 
{
    _sendbufsize = sendBufSize;
    delete _sendbuf;
    _sendbuf = new char[sendBufSize];
}

void Socket::setRecvBufSize(int recvBufSize) 
{
    _recvbufsize = recvBufSize;
    delete _recvbuf;
    _recvbuf = new char[recvBufSize];
}

Socket::~Socket()
{
    delete[] _sendbuf;
    delete[] _recvbuf;
}

void Socket::setFd(int fd)
{
    _fd = fd;
}

void Socket::resetFd()
{
    _fd = -1;
    _sendbufstart = _sendlen = 0;
    _recvlen = _packlen = 0;
}

int Socket::readImp(char* data, int len)
{
    return Net::read(_fd, data, len);
}

int Socket::writeImp(char* data, int len)
{
    int ret;
    do
    {
        if ((ret = process_sendbuf()) == -1)
        {
            break;
        }
        
        if (_sendlen > 0)
        {
            ret = append_sendbuf(data, len);
        }
        else
        {
            ret = process_senddata(data, len);
        }
    }
    while (0);
    
    
    return ret;
}

int Socket::process_sendbuf()
{
    while (_sendlen > 0)
    {
        int tosend;
        if (_sendbufstart + _sendlen > _sendbufsize)
        {
            tosend = _sendbufsize - _sendbufstart;
        }
        else
        {
            tosend = _sendlen;
        }
        
        int sendedlen = Net::write(_fd, _sendbuf + _sendbufstart, tosend);
        if (sendedlen == -1)
        {
            return -1;
        }
        
        _sendlen -= sendedlen;
        _sendbufstart = (sendedlen + _sendbufstart) % _sendbufsize;
        
        if (sendedlen < tosend)
        {
            break;
        }
    }
    
    return _sendlen;
}

int Socket::append_sendbuf(void* data, int len)
{
    if (len + _sendlen > _sendbufsize)
    {
        return -1;
    }
    
    int appended = 0;
    while (len > 0)
    {
        int toappend;
        int tailindex = (_sendlen + _sendbufstart) % _sendbufsize;
        
        if (tailindex + len > _sendbufsize)
        {
            toappend = _sendbufsize - tailindex;
        }
        else
        {
            toappend = len;
        }
        
        memcpy(_sendbuf + tailindex, (char*)data + appended, toappend);
        
        appended += toappend;
        _sendlen += toappend;
        len -= toappend;
    }
    
    return 0;
}

int Socket::process_senddata(void* data, int len)
{
    int ret = Net::write(_fd, data, len);
    if (ret == -1)
    {
        return -1;
    }
    
    if (ret < len)
    {
        return append_sendbuf((char*)data + ret, len - ret);
    }
    
    return 0;
}