//
//  thread.h
//  GameSrv
//
//  Created by prcv on 13-6-9.
//
//

#pragma once

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

class Thread;

class Runnable
{
public:
    Runnable() {}
    virtual ~Runnable(){}
    virtual bool beforeRun() {return true;}
    virtual void run() = 0;
    virtual void afterRun() {}
    void setThread(Thread* thread){
        _thread = thread;
    }
    Thread* getThread(){
        return _thread;
    }
    
protected:
    Thread* _thread;
};

class Thread
{
public:

    Thread ()
    {
        _started = false;
        _data = NULL;
        _target = NULL;
    }

    bool start(Runnable *tar)
    {
        _target = tar;
        _target->setThread(this);
        return pthread_create (&_handle, NULL, Thread::worker, this) == 0;
    }
    
    bool start(void* (*cworker)(void*), void* param)
    {
        return pthread_create (&_handle, NULL, cworker, param) == 0;
    }
    
    void setData(void* threaddata)
    {
        _data = threaddata;
    }
    
    void* getData()
    {
        return _data;
    }

    void join()
    {
        if (_started){
            pthread_join(_handle,  NULL);
        }
    }

    static void* worker(void* param)
    {
        Thread* thread = (Thread*)param;
        thread->_started = true;
        thread->_target->beforeRun();
        thread->_target->run();
        thread->_target->afterRun();
        return 0;
    }

    static void sleep(int ms)
    {
        timeval tv;
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
        select(0, NULL, NULL, NULL, &tv);
    }
    
    pthread_t getThreadId()
    {
        return _handle;
    }
    
    bool isStarted()
    {
        return _started;
    }

private:
    bool      _started;
    pthread_t _handle;
    ::Runnable *_target;
    void*     _data;

    Thread (const Thread&);
    const ::Thread &operator = (const ::Thread&);
};
