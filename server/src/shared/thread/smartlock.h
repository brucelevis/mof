//
//  smartlock.h
//  GameSrv
//
//  Created by prcv on 13-10-25.
//
//

#pragma once

#include <pthread.h>

#if __APPLE__
#define PTHREAD_MUTEX_RECURSIVE_NP PTHREAD_MUTEX_RECURSIVE
#endif

// 自动锁
// 线程安全对象
class CriticalObject
{
public:
	CriticalObject(void)
	{
	    pthread_mutexattr_t attr;
	    pthread_mutexattr_init(&attr);
	    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
        
        pthread_mutex_init(&mtx, &attr);
	}
	virtual ~CriticalObject(void)
	{
        pthread_mutex_destroy( &mtx );
	}
	void Lock()
	{
        pthread_mutex_lock( &mtx );
	}
	bool TryLock()
	{
        return ( pthread_mutex_trylock( &mtx ) == 0 );
	}
	void Unlock()
	{
        pthread_mutex_unlock( &mtx );
	}
private:
    pthread_mutex_t mtx;
};

class AtomCounter : public CriticalObject
{
public:
    AtomCounter() : m_counter(0){
        
    }
    
    int m_counter;
    
    int inc(){
        int val;
        Lock();
        val = ++m_counter;
        Unlock();
        
        return val;
    }
    
    int dec(){
        int val;
        Lock();
        val = --m_counter;
        Unlock();
        
        return val;
    }
    
    int val(){
        return m_counter;
    }
};

