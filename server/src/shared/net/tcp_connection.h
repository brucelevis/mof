//
//  socket.h
//  libgate
//
//  Created by prcv on 13-7-5.
//  Copyright (c) 2013年 prcv. All rights reserved.
//

#pragma once

#include "net.h"
#include "poller.h"
#include "gate.h"
#include "socket.h"
#include "tcp_listener.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

class NetStatistic
{
public:
    NetStatistic()
    {
        mOpen = false;
        mSampleMaxNum = mSampleMaxAvg = 0;
    }
    
    void start(time_t nowTime, int sampleMaxNum, int sampleMaxAvg)
    {
        mOpen = true;
        
        mStartTime = nowTime;
        mBandWidth = 0;
        mPacketNum = 0;
        
        mSampleNum = 0;
        mSampleStartTime = nowTime;
        
        mSampleMaxNum = sampleMaxNum;
        mSampleMaxAvg = sampleMaxAvg;
    }
    
    void end()
    {
        mOpen = false;
    }
    
    void resetSample(time_t nowTime)
    {
        mSampleNum = 0;
        mSampleStartTime = nowTime;
    }
    
    
    bool update(int len, time_t nowTime)
    {
        if (!mOpen)
        {
            return true;
        }
        
        mPacketNum++;
        mBandWidth += len;
        
        mSampleNum++;
        if (mSampleNum < mSampleMaxNum)
        {
            return true;
        }
        
        int costTime = nowTime - mSampleStartTime;
        if (costTime <= 0)
        {
            return false;
        }
        
        int sampleAvg = mSampleNum / costTime;
        resetSample(nowTime);
        
        if (sampleAvg > mSampleMaxAvg)
        {
            return false;
        }
        return true;
    }
    
private:
    bool mOpen;
    
    int mSampleStartTime;
    int mSampleNum;
    
    int mSampleMaxNum;
    int mSampleMaxAvg;
    
    int mStartTime;
    int mBandWidth;
    int mPacketNum;
};


class TcpListener;

class TcpConnection : public IoEvent, public Socket
{
public:
    TcpConnection()
    {
        _sid = -1;
    }
    
    ~TcpConnection()
    {
        if (_sid != -1)
        {
            close();
        }
    }
    
    bool init(int sid, int fd, TcpListener* listener)
    {
        _recvlen = 0;
        _packlen = 0;
        
        setFd(fd);
        _listener = listener;
        _sid = sid;
        
        //mStatistic.start(Gate::sTick, 200, 20);
        
        return true;
    }
    
    void in_event()
    {
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
                    _listener->close_connection(_sid);
                    break;
                }
            }
            else
            {
                //if (!mStatistic.update(_packlen, Gate::sTick))
                //{
                    //_listener->close_connection(_sid);
                    //return;
                //}
                _listener->get_cb()->onEvent(_sid, IoEventRead, _recvbuf + 4, _packlen);
                
                _recvlen = 0;
                _packlen = 0;
            }
        }
        
        if (ret < 0)
        {
            _listener->close_connection(_sid);
        }
    }
    
    void out_event()
    {
        int ret = process_sendbuf();
        if (ret == -1)
        {
            _listener->close_connection(_sid);
        }
        else if (_sendlen == 0)
        {
            _listener->get_poller()->mod_fd(_fd, POLLER_IN, this);
        }
    }
    
    int close()
    {
        ::close(_fd);
        resetFd();
        _sid = -1;
        return 0;
    }
    
    void write(char* data, int len)
    {
        int ret = writeImp(data, len);
        if (ret == -1)
        {
            _listener->close_connection(_sid);
        }
        else if (_sendlen)
        {
            _listener->get_poller()->mod_fd(_fd, POLLER_IN | POLLER_OUT, this);
        }
    }
    
    int get_type()
    {
        return IoConnection;
    }
    
    unsigned int get_sid()
    {
        return _sid;
    }
    
    int getFd()
    {
        return _fd;
    }
    
private:
    TcpListener* _listener;
    int     _sid;
    
    NetStatistic mStatistic;
};