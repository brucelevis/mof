//
//  ip_filter.h
//  GameSrv
//
//  Created by prcv on 14-2-21.
//
//

#ifndef __GameSrv__ip_filter__
#define __GameSrv__ip_filter__

#include <iostream>

#include "net.h"
#include <vector>
#include <map>
#include <string.h>
#include <fstream>
#include <stdlib.h>
using namespace std;


class IpAddress
{
public:
    string      mAddress;
    uint32_t    mNetAddr;
    uint8_t     mMask;
    
    bool init(const char* address);
    
    static uint32_t toNetAddr(const char* addr);
    
    bool invovle(uint32_t addr);
};

class IpFilter
{
public:
    bool filter(const char* ip)
    {
        uint32_t addr = IpAddress::toNetAddr(ip);
        for (vector<IpAddress>::iterator iter = mAddresses.begin(); iter != mAddresses.end(); iter++)
        {
            if (iter->invovle(addr))
            {
                return true;
            }
        }
        return false;
    }
    
    void addAddresses(IpAddress& ipAddress)
    {
        mAddresses.push_back(ipAddress);
    }
    
    vector<IpAddress> mAddresses;
};

class IpFilterMgr
{
public:
    void load(const string& path)
    {
        fstream cfgStream(path.c_str());
        if (!cfgStream.is_open())
        {
            return;
        }

        string lineData;
        string section;
        IpFilter* ipFilter = NULL;
        do{
            lineData = "";
            cfgStream >> lineData;
            
            size_t posStart = lineData.find("[");
            size_t posEnd = lineData.find("]");
            if (posStart != std::string::npos && posEnd != std::string::npos && posEnd > posStart)
            {
                section = string(lineData, posStart + 1, posEnd - posStart - 1);
                ipFilter = new IpFilter;
                mFilters.insert(make_pair(section, ipFilter));
                continue;
            }
            
            if ((lineData.empty()) ||
                (lineData[0]==';'&&lineData[1]==';') ||
                (lineData[0]=='/'&&lineData[1]=='/') ||
                (lineData[0]=='#')
                )
            {
                continue;
            }
            
            IpAddress address;
            address.init(lineData.c_str());
            ipFilter->addAddresses(address);
        }
        while (!cfgStream.eof());
    }
    
    bool isFiltered(const char* name, const char* ip)
    {
        map<string, IpFilter*>::iterator iter = mFilters.find(name);
        if (iter == mFilters.end())
        {
            return false;
        }
        
        IpFilter* filter = iter->second;
        return filter->filter(ip);
    }
    
    map<string, IpFilter*> mFilters;
};

extern IpFilterMgr g_IpFilterMgr;

#endif /* defined(__GameSrv__ip_filter__) */
