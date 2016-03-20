/*-------------------------------------------------------------------------
	created:	2010/08/25  9:52
	filename: 	e:\Project_SVN\Server\ServerDemo\Game.cpp
	author:		Deeple

	purpose:
---------------------------------------------------------------------------*/

#include "Game.h"
#include "DataBaseFun.h"
#include <sys/time.h> //linux
#include <unistd.h> //linux

//#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// 获取tick
UI32 GetTickCount(void)
{
    UI32 currentTime;

    struct timeval current;
    gettimeofday(&current, NULL);
    currentTime = current.tv_sec * 1000 + current.tv_usec/1000;

    return currentTime;
}


CGame* _GAME;
CGameConfig* _CONFIG;
CAsyncProcMgr* _ASYNCPROC;
CServerList* _SERVERLIST;
CInterfaceManager* _IFMGR;


// 客户端消息输入(IO线程组输入) use linux.   so delete it
void NETEVENTCALLBACK(const PersistID& id,
					  IoAction action,
					  char* pData,
					  size_t nBytes,
					  char* pUserData);

// class CGame
// -------------------------------------------------------------------------
CGame::CGame(void)
{
	_GAME = this;
	_CONFIG = &m_gameConfig;
	_ASYNCPROC = &m_AsyncProcMgr;
	_SERVERLIST = &m_ServerList;
	_IFMGR = &m_IFMgr;

	// 注册处理方法
	OnRegisterIoMsgProc();
}

CGame::~CGame(void)
{
	for (size_t i=0; i < m_sysIoProc.size(); i++)
	{
		if (m_sysIoProc[i])
		{
			delete m_sysIoProc[i];
		}
	}

	for (size_t i=0; i < m_customIoProc.size(); i++)
	{
		if (m_customIoProc[i])
		{
			//delete m_customIoProc[i];
		}
	}
}

bool CGame::Initialize(const char* config)
{
	// 重置运行状态
	m_hInputThread = 0;

	m_bRunning = false;

	// 初始化随机函数
	InitRandomFunc();

	// 初始化COM
	//::CoInitialize(NULL);

	// 读取服务器配置文件
	Log(LOG_NORMAL, "Loading Config Data..");
	if (!m_gameConfig.LoadConfig(config))
	{
		Log(LOG_ERROR, "Load config error, exiting..");
		return false;
	}

	// 设置IO消息回调
	WShellAPI::SetMsgProc(NETEVENTCALLBACK);


	if (WShellAPI::Open(m_gameConfig.m_nPort,NULL,5000,1024,false))
	{
		Log(LOG_NORMAL, "Server Port: %d", m_gameConfig.m_nPort);
	}
	else
	{
		Log(LOG_ERROR, "Cannot Open Port %d, Exiting...", m_gameConfig.m_nPort);
		return false;
	}

	// 创建异步工作池
	Log(LOG_NORMAL, "Creating Async Processor..");

	// 设置服务器运行
	m_bRunning = true;

	// 创建命令行线程
	Log(LOG_NORMAL, "Creating Command Thread..");
	//m_hInputThreadExit = CreateEvent(NULL, TRUE, FALSE, NULL); //windows
	//m_hInputThread = CreateThread(0, 0, WaitCmdLine, this, 0, 0); //windows
    pthread_create( &m_hInputThread, NULL, WaitCmdLine, this ); //linux

	// 加载模块
	Log(LOG_NORMAL, "Loading Modules..");
	m_IFMgr.LoadModules(m_gameConfig.GetDllPath(), &m_ServerList);

	Log(LOG_NOTICE, "Server Started.");

	return true;
}

// 主线程循环, 等待命令行输入
void CGame::Run()
{
	//DWORD dwInitTick = timeGetTime(); //windows
	UI32 dwInitTick = GetTickCount(); //u sec;

	UI32 timeUsed; //linux
	UI32 HeartBeatNeed =  0; //linux
	UI32 HeartUsed = 0; //linux

	while (m_bRunning)
	{
		CMsgTyped* pmsg = GetMessage();
		if (!pmsg)
		{
			// 主逻辑心跳. 1 秒 1次。
			timeUsed = GetTickCount() - dwInitTick; //windows
			HeartBeatNeed = timeUsed / 1000; //linux, u _sec

			// 检测是否需要重置
			if (HeartBeatNeed < HeartUsed || HeartBeatNeed - HeartUsed > 10)
			{
			    dwInitTick = GetTickCount(); //u sec;
                HeartBeatNeed =  0;
                HeartUsed = 0;

			    continue;
			}

			for (UI32 i=0; i < HeartBeatNeed - HeartUsed; i++)
			{
				// 执行心跳
				OnHeartBeatDump(HeartUsed + i + 1);
			}

			HeartUsed = HeartBeatNeed;

			//::Sleep(10); //windows
			::usleep( 10000 ); //linux u _sec
			continue;
		}

		// 处理消息
		// 获取消息类型
		int nInnerMsgType = 0;
		try
		{
			nInnerMsgType = pmsg->IntVal();
		}
		catch (CExceptMsg e)
		{
			Log(LOG_ERROR, "CGame::Run()， Read Command Type Error: %s", e.GetMsg());
		}

		if (nInnerMsgType == INMSG_CLIENT_IO)
		{
			// IO 消息
			OnIOMessageDump(pmsg);
		}
		else if (nInnerMsgType == INMSG_CMD_INPUT)
		{
			// 命令行消息
			try
			{
				const char* szCmd = pmsg->StringVal();
				StringArray sa;
				split_path(sa, szCmd, ' ', false);

				OnCmdLineDump(sa);
			}
			catch (CExceptMsg e)
			{
				LogFilePos();
				Log(LOG_ERROR, e.GetMsg());
			}
		}
		else if (nInnerMsgType == INMSG_COMPONENT)
		{
			// 命令行输入
			// 内部组件通知
			OnComponentMessageDump(pmsg);
		}

		delete pmsg;
	}
}

void CGame::UnInitalize()
{
	Log(LOG_NOTICE, "Exiting...");

	// 关闭命令行线程
	if (m_hInputThread)
	{
		//::WaitForSingleObject(m_hInputThreadExit, INFINITE); //windows
		pthread_join( m_hInputThread, NULL ); //linux
		m_hInputThread = 0; //linux
		//::CloseHandle(m_hInputThread); //windows
		//::CloseHandle(m_hInputThreadExit); windows
	}

	m_bRunning = false;
	//m_hInputThreadExit = NULL; windows
	m_hInputThread = NULL;

	// 关闭网络
	WShellAPI::ShutDown();

	// 关闭各组件
	Log(LOG_NORMAL, "Closing ...");

	// 关闭异步处理器
	m_AsyncProcMgr.WaitForRequestEmpty();
	m_AsyncProcMgr.RemoveAllProcessor();

	// 关闭COM
	//::CoUninitialize(); //windows

	// 释放模块
	m_IFMgr.FreeModules();

	// 删除消息队列里的剩余消息
	CMsgTyped* pmsg;
	while ((pmsg = GetMessage()) != NULL)
	{
		delete pmsg;
	}
}

//DWORD WINAPI CGame::WaitCmdLine(LPVOID lpParam) //windows
void* CGame::WaitCmdLine( void* lpParam )
{
	CGame* pGame = (CGame*)lpParam;

	if (!pGame)
	{
		return 0;
	}

	char szInput[1024];

	while (pGame->m_bRunning && gets(szInput))
    {
		// 如果输入'quit' 'exit' 则退出服务器
		if (stricmp(szInput, "quit") == 0 || stricmp(szInput, "exit") == 0)
		{
			pGame->m_bRunning = false;
			break;
		}

		// 获取命令并发送到逻辑线程(长度 > 0)
		if (strlen(szInput) > 0)
		{
			CMsgTyped msg;
			msg.SetInt(INMSG_CMD_INPUT);
			msg.SetString(szInput);

			pGame->SendMessage(&msg);
		}

		usleep( 10000 );
	}

	//SetEvent(pGame->m_hInputThreadExit); windows
	return 0;
}

// 用于调用堆栈DUMP
void CGame::OnIOMessageDump(CMsgTyped* pmsg)
{
	OnIOMessage(pmsg);
}

void CGame::OnCmdLineDump(StringArray& sa)
{
	OnCmdLine(sa);
}

void CGame::OnComponentMessageDump(CMsgTyped* pmsg)
{
	OnComponentMessage(pmsg);
}

void CGame::OnHeartBeatDump(UI32 nBeat)
{
	OnHeartBeat(nBeat);
}

void CGame::OnCmdLine(StringArray& sa)
{
	if (sa.size() == 0)
	{
		return;
	}

	if (stricmp(sa[0].c_str(), "88") == 0)
	{
		SendQuitMessage();
	}
}

void CGame::SendQuitMessage()
{
	CMsgTyped msg;
	msg.SetInt(INMSG_COMPONENT);
	msg.SetInt(COMPMSG_SERVER_QUIT);
	SendMessage(&msg);
}

void CGame::RegisterIOMessageProc(bool system, UI32 msgId, IGlobalMessageProc* proc)
{
	vector<IGlobalMessageProc*>& msgcon = system ? m_sysIoProc : m_customIoProc;

	if (msgcon.size() <= msgId)
	{
		msgcon.resize(msgId + 10, NULL);
	}

	if (msgcon[msgId])
	{
		delete msgcon[msgId];
	}

	msgcon[msgId] = proc;
}

bool CGame::SendClientMessage(PersistID& id, CMsgTyped* pmsg)
{
	if (!pmsg)
	{
		return false;
	}

	//m_ResponseChecker.AddResponse(id);

	return WShellAPI::SendData(id, (void*)pmsg->GetData(), pmsg->GetLength()) == true;
}

bool CGame::Disconnect(PersistID& id)
{
	return WShellAPI::Disconnect(id) == true;
}

void CGame::OnIOMessage(CMsgTyped* pmsg)
{
	IO_MESSAGE nIoMsgType;
	PersistID id;
	void* pData = NULL;
	size_t nBytes = 0;

	try
	{
		nIoMsgType = (IO_MESSAGE)pmsg->IntVal();
		id.nIndex = pmsg->IntVal();
		id.nIdent = pmsg->IntVal();

		if (nIoMsgType == IOMSG_READ)
		{
			pmsg->BinaryVal(pData, nBytes);
		}
	}
	catch (CExceptMsg e)
	{
		LogFilePos();
		Log(LOG_ERROR, "Read Io Message Type Error: %s", e.GetMsg());
		return;
	}

	// 处理消息
	if (nIoMsgType == IOMSG_READ)
	{
		CMsgTyped msg(nBytes, pData, false);

		UI32 nReceiver;
		UI32 nCommandId;

		try
		{
			UI32 nReserv = msg.IntVal();
			nReceiver = msg.IntVal();
			nCommandId = msg.IntVal();
		}
		catch (CExceptMsg e)
		{
			LogFilePos();
			Log(LOG_ERROR, "Read Io Message Type Error: %s", e.GetMsg());
			return;
		}

		// 增加请求计数
		//m_ResponseChecker.AddRequest(id, nCommandId);

		bool bValidMessage = false;

		try
		{
			if (nReceiver == 0)
			{
				if (m_sysIoProc.size() > nCommandId)
				{
					if (m_sysIoProc[nCommandId])
					{
						m_sysIoProc[nCommandId]->OnIoMessage(id, nReceiver, nCommandId, &msg);
						bValidMessage = true;
					}
				}
			}

			if (!bValidMessage)
			{
				if (m_customIoProc.size() > nCommandId)
				{
					if (m_customIoProc[nCommandId])
					{
						m_customIoProc[nCommandId]->OnIoMessage(id, nReceiver, nCommandId, &msg);
						bValidMessage = true;
					}
				}
			}
		}
		catch (CExceptMsg e)
		{
			Log(LOG_ERROR, "iomsg, %d, read msg error: %s", nCommandId, e.GetMsg());
		}
		catch (...)
		{
			Log(LOG_ERROR, "iomsg, %d, Catch Exception", nCommandId);
		}

		// 保证io消息总有回应
		if (!bValidMessage)
		{
			Log(LOG_ERROR, "Invalid Message:%d", nCommandId);

			// 对于非法请求, 给予回应
			CMsgTyped msg;
			msg.SetInt(0);
			SendClientMessage(id, &msg);
		}
	}
	else if (nIoMsgType == IOMSG_CONNECT)
	{
		// 收到连接断开消息
		OnConnect(id);
	}
	else if (nIoMsgType == IOMSG_DISCONNECT)
	{
		// 收到连接断开消息
		OnDisconnect(id);
	}
}

void CGame::OnComponentMessage(CMsgTyped* pmsg)
{
	try
	{
		int msgId = pmsg->IntVal();

		if (msgId == COMPMSG_SERVER_QUIT)
		{
			OnQuitMessage(pmsg);
		}
		else
		{

		}
	}
	catch (CExceptMsg e)
	{
		Log(LOG_ERROR, e.GetMsg());
	}
}

void CGame::OnHeartBeat(UI32 nBeat)
{
	//m_ResponseChecker.Check(120);
	m_IFMgr.OnHeartBeat(nBeat);
	m_ServerList.OnHeartBeat(nBeat);

}

void CGame::OnConnect(PersistID& id)
{
	//m_ResponseChecker.AddConn(id);
}

void CGame::OnDisconnect(PersistID& id)
{
	//m_ResponseChecker.RemoveConn(id);
	ServerInfo* pServer = _SERVERLIST->FindServer( id );
	if ( pServer )
	{
	    _IFMGR->UnRegisterServer( pServer->serverid );
	    pServer->conn.Invalid();
	}
}

void CGame::OnQuitMessage(CMsgTyped* pmsg)
{
	m_bRunning = false;

	// 输入回车键
//	INPUT_RECORD ir[1];
//	ir[0].EventType = KEY_EVENT;
//	ir[0].Event.KeyEvent.bKeyDown = TRUE;
//	ir[0].Event.KeyEvent.dwControlKeyState = 0;
//	ir[0].Event.KeyEvent.uChar.UnicodeChar = '\r';
//	ir[0].Event.KeyEvent.wRepeatCount = 1;
//	ir[0].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
//	ir[0].Event.KeyEvent.wVirtualScanCode = MapVirtualKey(VK_RETURN, 0);
//
//	DWORD dwTmp;
//	WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), ir, 1, &dwTmp);
}

