
#include "ClientTest.h"

#define SERVERIP "192.168.3.117"
//#define SERVERIP "127.0.0.1"
#define SERVERPORT 5009


std::string szServerIP;

CMyAsyncSock g_sock;
MsgMgr g_MsgMgr;


void CMyAsyncSock::OnReceive(CMsgTyped* pmsg)
{
	try
	{
		int nSysFlag = pmsg->IntVal();
		int recver = pmsg->IntVal();
		int nCmdId = pmsg->IntVal();

		g_MsgMgr.OnResponse(nCmdId, pmsg);
	}
	catch(CExceptMsg e)
	{

	}
}

bool g_bRunning = true;

//HANDLE g_hEventExit = CreateEvent(NULL,TRUE,FALSE,NULL);
sem_t g_sem_exit;
void SendHeartBeat();

void* DoMessages(void* param)
{
	int iCounter = 0;
	while (g_bRunning)
	{
		g_sock.ProcessMessages();

		::usleep(1000000); // 1sec

		iCounter ++;

		if (iCounter % 10 == 0) //10 sec
		{
			SendHeartBeat();
		}
	}

	//SetEvent(g_hEventExit);
	::sem_post( &g_sem_exit );

	return NULL;
}

int main(int argc, char** argv)
{
    char buf[1024];
    int cnt = readlink( "/proc/self/exe", buf, 1024 );
    buf[cnt] = 0;

    printf("%s\n", buf);

	szServerIP = SERVERIP;

	bool bSucc = g_sock.Open(szServerIP.c_str(), SERVERPORT);
	if ( !bSucc )
	{
	    printf("open server socket error!\n");
	    return 1;
	}

    pthread_t pid = NULL;
	if ( ::pthread_create( &pid, NULL, DoMessages, NULL ) )
	{
	    printf("create DoMessage Thread error!\n");
	    return 2;
	}

    if ( ::sem_init( &g_sem_exit, 0, 0 ) )
    {
        printf("init wait g_sem_exit error!\n");
        return 3;
    }

    printf("client test start succeed!\n");

	char input[1024];
	while (gets(input))
	{

		StringArray sa;
		split_path(sa, input, ' ');

		if (sa.size() == 0)
		{
			continue;
		}

		if (stricmp(input, "q") == 0) break;

        g_MsgMgr.OnRequest(sa);
	}

	g_bRunning = false;

	::sem_wait( &g_sem_exit );

	return 0;
}

