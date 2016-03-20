//
//  net.cpp
//  GameSrv
//
//  Created by prcv on 13-6-13.
//
//

#include "net.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>


bool Net::set_nodelay(int fd)
{
    int flag = 1;
    //setsockopt(fd, IPPROTO_TCP, S, &flag, sizeof(flag));
    return 0; 
}

bool Net::set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

bool Net::set_reuse(int fd)
{
    int reuse = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

    return true;
}

bool Net::set_nosigpipe(int fd)
{
    int set = 1;
    //return 0 == setsockopt (fd, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof (int));
    return true;
}

bool Net::set_recvtimeout(int fd, struct timeval& tv)
{
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
    return true;
}


int Net::connect(const char* ip, unsigned short port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        return -1;
    }

    sockaddr_in addr;
    memset(&addr, sizeof(sockaddr_in), 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = query_ip(ip);

    int ret = ::connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1)
    {
        close(fd);
        fd = -1;
    }

    return fd;
}

int Net::async_connect(const char* ip, unsigned short port, bool& connected)
{
    connected = false;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        return -1;
    }

    set_nonblocking(fd);

    sockaddr_in addr;
    memset(&addr, sizeof(sockaddr_in), 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = query_ip(ip);

    int ret = ::connect(fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == 0)
    {
        connected = true;
    }
    else
    {
        if (errno != EINPROGRESS)
        {
            close(fd);
            fd = -1;
        }
    }

    return fd;
}

uint32_t Net::query_ip(const char* addr)
{
    unsigned int ip = inet_addr(addr);
    if (ip == INADDR_NONE)
    {
        struct hostent *host = NULL;
        host = gethostbyname(addr);
        if(host != NULL)
        {
            memcpy(&ip, host->h_addr_list[0], host->h_length);
        }
        else
        {
            return INADDR_NONE;
        }
    }

    return ip;
}

int Net::getIpType(unsigned int addr)
{
    if ((addr >> 31) == 0)
    {
        return eIpTypeA;
    }
    
    if ((addr >> 30) == 2)
    {
        return eIpTypeB;
    }
    
    if ((addr >> 29) == 6)
    {
        return eIpTypeC;
    }
    
    if ((addr >> 28) == 14)
    {
        return eIpTypeD;
    }
    
    if ((addr >> 27) == 30)
    {
        return eIpTypeE;
    }
    
    return eIpTypeOther;
}

int Net::create_listener(const char* ip, unsigned short port, int backlog)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        return -1;
    }

    Net::set_reuse(fd);

    sockaddr_in addr;
    memset(&addr, sizeof(sockaddr_in), 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (ip)
    {
        addr.sin_addr.s_addr = inet_addr(ip);
    }


    int ret;
    do
    {
        ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
        if (ret < 0)
        {
            break;
        }

        ret = listen(fd, backlog);
    }
    while (0);

    if (ret < 0)
    {
        close(fd);
        fd = -1;
    }

    return fd;
}

int Net::read(int fd, void* data, size_t size)
{
    ssize_t nbytes = recv (fd, data, size, 0);

    if (nbytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK ||
                         errno == EINTR))
        return 0;

    //  Signalise peer failure.
    if (nbytes == -1) {
        return -1;
    }

    if (nbytes == 0)
    {
        return -1;
    }

    return (size_t) nbytes;
}

int Net::write(int fd, void* data, size_t size)
{
    ssize_t nbytes = send (fd, data, size, 0);

    if (nbytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK ||
                         errno == EINTR))
        return 0;

    //  Signalise peer failure.
    if (nbytes == -1) {
        return -1;
    }

    if (nbytes == 0)
    {
        return -1;
    }

    return (size_t) nbytes;
}

int Net::read_all(int fd, void* data, size_t size)
{
	int totallen = 0, recvlen = 0;

	while((totallen += recvlen) < size)
	{
		recvlen = recv(fd, ((char*)data) + totallen, size - totallen, 0);
		if(0 >= recvlen)
		{
			return recvlen;
		}
	}

	return totallen;
}
