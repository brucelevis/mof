
#include "manage.h"
#include "http.h"
#include "tools.h"

void NETEVENTCALLBACK(const PersistID& id,
					  IoAction action,
					  char* pData,
					  size_t nBytes,
					  char* pUserData
				   );


// 用于定时关闭客户端链接
CriticalObject g_CloseConnLock;
vector< std::pair<PersistID, int> > g_ConnClose;
pthread_t g_CloseThread;
bool g_isRunning = false;

// 关闭垃圾连接
void* CloseBadConn(void* p)
{
    while (g_isRunning)
    {
        // 10毫秒检测一次
        usleep( 10000 );

        // 执行异步函数
        SyncProc();

        static time_t lasttm = time( NULL );
        time_t now = time( NULL );

        // 一秒执行一次
        if (now != lasttm)
        {
            lasttm = now;

            g_CloseConnLock.Lock();

            //////////////////////////
            vector< std::pair<PersistID, int> >::iterator it;
            for(it = g_ConnClose.begin();it != g_ConnClose.end(); )
            {
                it->second --;
                if (it->second == 0)
                {
                    WShellAPI::Disconnect( it->first );
                    it = g_ConnClose.erase(it);
                }
                else
                {
                    it++;
                }
            }

            //////////////////////////////
            g_CloseConnLock.Unlock();
        }
    }

    return 0;
}

void BeginServer(unsigned short port)
{
	if (!WShellAPI::Open(port, 0, 256))
	{
		Log(LOG_ERROR, "NanoWeds: Cannot open port %d, close", port);
		return;
	}

	WShellAPI::SetMsgProc(NETEVENTCALLBACK);
	Log(LOG_NOTICE, "NanoWeds: started, port %d", port);

	// 开启线程
	pthread_create(&g_CloseThread, NULL, CloseBadConn, NULL);
	g_isRunning = true;
}

void ShutDownServer()
{
    // 关闭线程
    g_isRunning = false;
    pthread_join(g_CloseThread, NULL);

    // 关闭服务器
    WShellAPI::ShutDown();

    // 清理定时器
    g_CloseConnLock.Lock();
    g_ConnClose.clear();
    g_CloseConnLock.Unlock();
}

void CloseConn(PersistID id, int secs)
{
    if (secs < 1) secs = 1;
    if (secs > 3600) secs = 3600;

    g_CloseConnLock.Lock();

    for (size_t i=0; i < g_ConnClose.size(); i++)
    {
        if (g_ConnClose[i].first == id)
        {
            g_ConnClose[i].second = secs;
            g_CloseConnLock.Unlock();
            return;
        }
    }

    g_ConnClose.push_back( std::make_pair(id, secs) );

    g_CloseConnLock.Unlock();
}

//////////////////////////////////////////////////
void NETEVENTCALLBACK(const PersistID& id,
					  IoAction action,
					  char* pData,
					  size_t nBytes,
					  char* pUserData
				   )
{
	switch(action)
	{
	case IoAccept:
		{
		}
		break;
    case IoRead:
        {
           HttpRequest * http = NULL;
           if ( WShellAPI::GetUserData(id,  (char**)&http ))
           {
               if (!http)
               {
                   http = new HttpRequest;
                   http->m_ConnId = id;
                   http->m_szIP = WShellAPI::GetAddr(id);

                   WShellAPI::SetUserData( id, (char*)http );
               }

               http->ReceiverBytes(pData, nBytes);
           }
        }
        break;
	case IoEnd:
		{
           HttpRequest * http = NULL;
           if ( WShellAPI::GetUserData(id,  (char**)&http ))
           {
                SAFE_DELETE( http );
                WShellAPI::SetUserData(id, NULL);

                // 删除回应记录
                g_ResonseManager.DeleteResonse( id );
           }
		}
		break;
	default:
		break;
	}
}

