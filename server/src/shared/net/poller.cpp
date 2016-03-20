//
//  poller.cpp
//  GameSrv
//
//  Created by prcv on 13-6-13.
//
//

#include "poller.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>


static uint64_t now_ms()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void Poller::add_timer(int timeout, int timerid, IoEvent* udata)
{
    uint64_t expiration = now_ms() + timeout;
    timer_info_t info;
    info.ioevent = udata;
    info.id = timerid;
    info.timeout = expiration;
    timers.push_back(info);
}

void Poller::cancel_timer(int timeout, int timerid, IoEvent* udata)
{
    //  Complexity of this operation is O(n). We assume it is rarely used.
    for (timers_t::iterator it = timers.begin (); it != timers.end ();)
    {
        if (it->ioevent == udata && it->id == timerid)
        {
            it = timers.erase (it);
        }
        else
        {
            ++it;
        }
    }
}

int Poller::execute_timers()
{
    if (timers.empty ())
        return 0;

    uint64_t current = now_ms();

    timers_t::iterator it = timers.begin ();
    while (it != timers.end ()) {

        if (it->timeout > current){
            ++it;
            continue;
        }
        
        it->ioevent->timer_event (it->id);
        it = timers.erase (it);
    }

    return 0;
}

#ifdef __APPLE__
#include <sys/event.h>
//
class Kqueue : public Poller
{
public:
    Kqueue()
    {
        kqfd = -1;
    }

    ~Kqueue()
    {
        if (kqfd > 0)
        {
            close();
        }
    }

	bool init()
	{
		kqfd = kqueue();
		return kqfd != -1;
	}

    void close()
    {
        ::close(kqfd);
    }

	void poll()
	{
        int retval;
		if (timeout != 0)
		{
	        struct timespec ts;
	        ts.tv_sec = timeout / 1000;
	        ts.tv_nsec = (timeout % 1000) * 1000000;
	        retval = kevent(kqfd, NULL, 0, events, MAX_EVENT, &ts);
	    }
	    else
	    {
	        retval = kevent(kqfd, NULL, 0, events, MAX_EVENT, NULL);
	    }

        for(int j = 0; j < retval; j++)
        {
            IoEvent* io = (IoEvent*)(events[j].udata);
            if (events[j].filter == EVFILT_READ)
            {
            	io->in_event();
            }
            if (events[j].filter == EVFILT_WRITE)
            {
            	io->out_event();
            }
        }
	}

	bool add_fd(int fd, int mask, IoEvent* udata)
	{
	    struct kevent ke;
	    if (mask & POLLER_IN)
	    {
			EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, udata);
	        if (kevent(kqfd, &ke, 1, NULL, 0, NULL) == -1)
	        {
	        	return false;
	        }
	    }
	    if (mask & POLLER_OUT)
	    {
			EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, udata);
	        if (kevent(kqfd, &ke, 1, NULL, 0, NULL) == -1)
	        {
	        	return false;
	        }
	    }

	    return true;
	}


    bool mod_fd(int fd, int mask, IoEvent* udata)
    {
        struct kevent ke;
	    if (mask & POLLER_IN)
	    {
			EV_SET(&ke, fd, EVFILT_READ, EV_ADD, 0, 0, udata);
	    }
        else
        {
			EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, udata);
        }
        if (kevent(kqfd, &ke, 1, NULL, 0, NULL) == -1)
        {
            return false;
        }

	    if (mask & POLLER_OUT)
	    {
			EV_SET(&ke, fd, EVFILT_WRITE, EV_ADD, 0, 0, udata);
	    }
        else
        {
			EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, udata);
        }
        if (kevent(kqfd, &ke, 1, NULL, 0, NULL) == -1)
        {
            return false;
        }

        return true;
    }

	void rm_fd(int fd, int mask)
	{
	    struct kevent ke;
		{
	        EV_SET(&ke, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	        kevent(kqfd, &ke, 1, NULL, 0, NULL);
	    }
	    {
	        EV_SET(&ke, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	        kevent(kqfd, &ke, 1, NULL, 0, NULL);
	    }
	}

private:
	const static int MAX_EVENT = 32;
	const static int timeout = 10;

    int kqfd;
    struct kevent events[MAX_EVENT];
};


Poller* Poller::create()
{
    Kqueue* poller = new Kqueue();
    poller->init();
    return (Poller*)poller;
}

#elif __linux__
#include <sys/epoll.h>

class Epoller : public Poller
{
public:
    Epoller()
    {
        epollfd = -1;
    }

    ~Epoller()
    {
        if (epollfd > 0)
        {
            close();
        }
    }

	bool init()
	{
		epollfd = epoll_create(0xFFFF);
		return epollfd != -1;
	}

    void close()
    {
        ::close(epollfd);
    }

	void poll()
	{
		int nfd = epoll_wait(epollfd, evs, MAX_EVENT, timeout);
		for (int i = 0; i < nfd; i++)
		{
			IoEvent* io = (IoEvent*)evs[i].data.ptr;
			if (evs[i].events & EPOLLIN)
			{
				io->in_event();
			}

			if (evs[i].events & EPOLLOUT || evs[i].events & EPOLLERR)
			{
				io->out_event();
			}
		}
	}

	bool add_fd(int fd, int mask, IoEvent* udata)
	{
		struct epoll_event ev;
		ev.events = 0;
		if (mask & POLLER_IN)
		{
			ev.events |= EPOLLIN;
		}
		if (mask & POLLER_OUT)
		{
			ev.events |= EPOLLOUT;
		}
		ev.data.ptr = udata;

		int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);

		return ret == 0;
	}

	void rm_fd(int fd, int mask)
	{
		epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
	}


    bool mod_fd(int fd, int mask, IoEvent* udata)
    {
        struct epoll_event ev;
		ev.events = 0;
		if (mask & POLLER_IN)
		{
			ev.events |= EPOLLIN;
		}
		if (mask & POLLER_OUT)
		{
			ev.events |= EPOLLOUT;
		}
		ev.data.ptr = udata;

		int ret = epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev);
        return ret == 0;
    }

private:
	const static int MAX_EVENT = 32;
	const static int timeout = 10;

	int epollfd;
	struct epoll_event evs[MAX_EVENT];
};

Poller* Poller::create()
{
    Poller* poller = (Poller*)new Epoller();
    poller->init();
    return poller;
}

#endif
