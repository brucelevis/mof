/*-------------------------------------------------------------------------
	created:	2010/08/16  16:10
	filename: 	e:\Project_SVN\Server\Utils\ThreadPool\ThreadPool.h
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/
#pragma once

#ifdef LINUX_
    #include <pthread.h>
    #include "../FileSys/SearchOps.h"

    #include <mysql_connection.h>
    #include <mysql_driver.h>
    #include <cppconn/statement.h>
    #include <cppconn/prepared_statement.h>
    #include <cppconn/driver.h>
    #include <cppconn/exception.h>
    #include <cppconn/resultset.h>
#else

#endif // LINUX_
#include <queue>
#include <string>
#include <unistd.h>
using namespace std;

#define MAX_ASYNC_PROCESSER_THREADS_COUNT 32

// 自动锁
// 线程安全对象
class CriticalObject
{
public:
	CriticalObject(void)
	{
	    pthread_mutexattr_t attr;
	    pthread_mutexattr_init(&attr);
	    pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);

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

class WEvent : public CriticalObject
{
public:
    WEvent(){ m_nRef = 1; }
    ~WEvent(){}

    bool WaitEvent(size_t msecs = -1)
    {
        while (msecs --)
        {
            Lock();

            if (m_nRef == 0)
            {
                Unlock();
                return true;
            }

            Unlock();

            usleep(1000);
        }

        return false;
    }

    void SetEvent()
    {
        Lock();

        if (m_nRef > 0)
        {
            m_nRef -- ;
        }

        Unlock();
    }

    void ResetEvent()
    {
        Lock();

        m_nRef = 1;

        Unlock();
    }

private:
    int32_t m_nRef;
};

// 自动锁
class SmartLock
{
public:
	SmartLock(CriticalObject* pObj)
	{
		m_pObject = pObj;
		if (m_pObject)
		{
			m_pObject->Lock();
		}
	}
	~SmartLock()
	{
		if (m_pObject)
		{
			m_pObject->Unlock();
		}
	}
	CriticalObject* m_pObject;
};

#define SMART_LOCK(obj) SmartLock lock(obj)
#define SMART_LOCK_THIS() SmartLock lock_this(this)

class CMsgTyped;
class CAsyncProcesser;

// 异步处理函数
typedef
void (*LPASYNCPROCFUNC)(CMsgTyped* pmsg, // 输入数据
						sql::Statement* pConn// 数据库连接
						);

class MysqlResult
{
public:
    MysqlResult(){dataSet = NULL;}
    ~MysqlResult(){if (dataSet){delete dataSet; dataSet = NULL;} }
    sql::ResultSet * dataSet;
};

struct AsyncProcTask
{
	LPASYNCPROCFUNC func;
	CMsgTyped* pParamList;
};

enum PROCESSOR_ERROR
{
	ERROR_CONNDB = 1,
};

struct AsyncThreadItem
{
public:
	AsyncThreadItem()
	{
		m_hThread = 0;
		m_AsyncProc= NULL;
		m_nLastError = 0;
	}
	~AsyncThreadItem(){}
public:
	pthread_t m_hThread;
	WEvent m_hExitEvent;
	WEvent m_hConnDBEvent;
	CAsyncProcesser* m_AsyncProc;
	int m_nLastError;
};

// 异步处理机
class CAsyncProcesser : public CriticalObject
{
public:
	CAsyncProcesser();
	~CAsyncProcesser();

public:
	// 获取处理机id
	int GetProcessorID();

	// 获取处理机名字
	const char* GetProcessorName();

	// 设置处理机id
	void SetProcessorID(int nProcessorId);

	// 设置处理机名字
	void SetProcessorName(const char* szProcessorName);

	// 获取任务数量
	size_t GetTaskCount();

public:
	// 初始化处理池。如果提供了数据库连接串， 则连接池的每个线程都将维护一个数据库连接。
	bool Open(size_t nThreadCount, const char* szADOConnStr = NULL);

	// 关闭处理池
	void ShutDown();

	// 调用
	void Call(LPASYNCPROCFUNC func, CMsgTyped* pParamList);

protected:
	static void *
	#ifndef LINUX_
	__cdecl
	#endif
	WorkProc(void* lpParam);

private:
	// 工作线程组
	AsyncThreadItem m_WorkThreads[MAX_ASYNC_PROCESSER_THREADS_COUNT];

	// 任务列表
	std::queue<AsyncProcTask> m_TaskList;

	// 数据库连接串
	string m_szADOConnStr;

	// 是否在运行中
	bool m_bRunning;

	// 运行线程个数
	size_t m_nThreadCount;

	// 处理器名字
	string m_szProcessorName;

	// 处理器id
	int m_nProcessorId;
};

class CAsyncProcMgr : public CriticalObject
{
public:
	CAsyncProcMgr();
	~CAsyncProcMgr();
public:
	// 添加处理机
	bool AddProcessor(int id, const char* szProcessorName, size_t nThreadCount, const char* szADOConnStr = NULL);

	// 查找处理机
	CAsyncProcesser* FindProcessor(int nProcessorID);

	// 查找处理机
	CAsyncProcesser* FindProcessor(const char* szProcessorName);

	// 注册工作
	bool Call(int nProcessorId, LPASYNCPROCFUNC func, CMsgTyped* pParamList);

	// 注册工作
	bool Call(const char* szProcessorName, LPASYNCPROCFUNC func, CMsgTyped* pParamList);

	// 删除处理机
	bool RemoveProcessor(int nProcessorID);

	// 删除处理机
	bool RemoveProcessor(const char* szProcessorName);

	// 删除所有处理机
	void RemoveAllProcessor();

	// 等待所有处理机处理完毕
	void WaitForRequestEmpty();

private:
	vector<CAsyncProcesser*> m_processorList;
};
