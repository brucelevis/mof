//
//  socket.h
//  libgate
//
//  Created by prcv on 13-7-7.
//  Copyright (c) 2013年 prcv. All rights reserved.
//

#pragma once

#include "net.h"
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>


class Socket
{
public:
    Socket();
    Socket(int recvBufSize, int sendBufSize);
    ~Socket();
    
    void setFd(int fd);
    void resetFd();
    
    virtual void write(char* data, int len) {}
    int getUserData() {return mUserData;}
    void setUserData(int userData) {mUserData = userData;}
    
    void setSendBufSize(int sendBufSize);
    void setRecvBufSize(int recvBufSize);
    
protected:
    int readImp(char* data, int len);
    int writeImp(char* data, int len);
    int append_sendbuf(void* data, int len);
    int process_sendbuf();
    int process_senddata(void* data, int len);
    
protected:
    int    _fd;
    
    
    char* _sendbuf;
    int   _sendlen;
    int   _sendbufstart;
    int   _sendbufsize;
    
    int     _recvbufsize;
    char*   _recvbuf;
    int     _recvlen;
    int     _packlen;
    
    int mUserData;
};
