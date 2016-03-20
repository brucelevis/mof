//
//  ip_filter.cpp
//  GameSrv
//
//  Created by prcv on 14-2-21.
//
//

#include "ip_filter.h"
IpFilterMgr g_IpFilterMgr;


bool IpAddress::init(const char* address)
{
    const char* pos = address;
    while (*pos)
    {
        if (*pos == '/')
        {
            break;
        }
        
        pos++;
    }
    
    string ip = string(address, pos);
    mAddress = address;
    if (*pos)
    {
        mMask = atoi(pos + 1);
    }
    else
    {
        mMask = 32;
    }
    mNetAddr = toNetAddr(ip.c_str());
    
    return true;
}

uint32_t IpAddress::toNetAddr(const char* addr)
{
    uint32_t netAddr;
    const char* pEnd = addr;
    const char* pStart = addr;
    int secMask = 24;
    while (*pEnd && secMask >= 0)
    {
        if (!isdigit(*pEnd))
        {        
            if (*pEnd == '.')
            {
                string ssec = string(pStart, pEnd);
                uint32_t sec = atoi(ssec.c_str());
                netAddr += (sec << secMask);
                
                secMask -= 8;
                pStart = pEnd + 1;
            }
            else
            {
                return 0;
            }
        }
        pEnd++;
    }
    
    if (pEnd != pStart)
    {
        string ssec = string(pStart, pEnd);
        uint32_t sec = atoi(ssec.c_str());
        netAddr += (sec << secMask);
    }
    
    return netAddr;
}

bool IpAddress::invovle(uint32_t addr)
{
    if (mMask < 0 || mMask > 32)
    {
        return false;
    }
    
    int shiftBits = 32 - mMask;
    if ((mNetAddr >> shiftBits) == (addr >> shiftBits))
    {
        return true;
    }
    
    return false;
}