//
//  net.h
//  GameSrv
//
//  Created by prcv on 13-6-13.
//
//

#ifndef GameSrv_net_h
#define GameSrv_net_h

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>


enum IpType {
    eIpTypeA = 0,
    eIpTypeB,
    eIpTypeC,
    eIpTypeD,
    eIpTypeE,
    eIpTypeOther
};

class Net
{
public:
    static bool set_nodelay(int fd);
    static bool set_reuse(int fd);
    static bool set_nonblocking(int fd);
    static bool set_nosigpipe(int fd);
    static bool set_recvtimeout(int fd, struct timeval& tv);
    static int create_listener(const char* ip, unsigned short port, int backlog);
    static int connect(const char* ip, unsigned short port);
    static int async_connect(const char* ip, unsigned short port, bool& connected);
    static uint32_t query_ip(const char* addr);
    
    static int read(int fd, void* data, size_t size);
    static int write(int fd, void* data, size_t size);
    
    static int read_all(int fd, void* data, size_t size);
    
    static int getIpType(unsigned int addr);
    
    static sockaddr conv_addr(const char* addr);
};

#endif
