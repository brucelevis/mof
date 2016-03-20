/*-----------------------------------------------------------------------------
filename:	Common.h
auther  :	Deeple
purpose :
create  :	2007/11/22 22:11:2007   13:16
-----------------------------------------------------------------------------*/
#ifndef _Common_h_
#define _Common_h_

#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <dlfcn.h>
#include "../FileSys/FileSys.h"

using namespace std;

#define IN
#define OUT
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))

typedef int SOCKET;

// 最小缓冲大小
#define MIN_BUFFER_LEN 128

// 最大缓冲大小
#define MAX_BUFFER_LEN 65536

// 最大客户端数量
#define MAX_CONNECTIONS 25600

// 最大逻辑包长度
#define MAX_PACKAGE_LEN (1024 * 256)

// 检查参数的范围
#define CheckMinValue(VALUE, MIN)\
	if (VALUE < MIN) {\
		VALUE = MIN;\
	}\

#define CheckMaxValue(VALUE, MAX) \
	if (VALUE > MAX){\
		 VALUE = MAX; \
	}\

#define CheckValueRange(VALUE,MIN,MAX) \
  if (VALUE < MIN) { VALUE = MIN; }\
  if (VALUE > MAX) { VALUE = MAX; }\


#define SO_EXCLUSIVEADDRUSE ((int)(~SO_REUSEADDR))

// 网络连接ID
// 网络连接ID
#define MAKEINT64(low,high) ((int64_t)(((int)(low)) | ((int64_t)((int)(high))) << 32))

struct PersistID
{
public:
    PersistID()
    {
       nIndex = 0;
       nIdent = 0;
    }
	PersistID(size_t nIdx, size_t nIdt)
	{
		nIndex = nIdx;
		nIdent = nIdt;
	}

	int32_t nIndex;	// socket 序号
    int32_t nIdent;	// 身份标记

	bool operator < (const PersistID& another) const
	{
		return MAKEINT64(nIndex, nIdent) < MAKEINT64(another.nIndex, another.nIdent);
	}

	int64_t ToInt64()
	{
		return MAKEINT64(nIndex, nIdent);
	}

	void operator = (const PersistID& another)
	{
		nIndex = another.nIndex;
		nIdent = another.nIdent;
	}

	bool operator == (const PersistID& another)
	{
		return (nIndex == another.nIndex && nIdent == another.nIdent);
	}

	bool Valid() const
	{
		return (nIdent != -1 && nIndex != -1);
	}

	void Invalid()
	{
		nIndex = -1;
		nIdent = -1;
	}
};

#define INIT_PERSISTID(dest)\
	dest.nIndex = 0;\
	dest.nIdent = 0;\

// Io操作定义
enum IoAction
{
	IoRead = 0,
	IoWrite,
	IoAccept,
	IoEnd,
};

// 服务器选项
struct WShellOption
{
public:
    WShellOption()
    {
        ExclusiveAddrUse = 1;
        KeepAlive = -1;
        SendBufSize = -1;
        RecvBufSize = -1;
        TcpNoDelay = 1;
        LingerOnOff = 0;
        LingerSeconds = -1;
    }
	int ExclusiveAddrUse;
	int KeepAlive;
	int SendBufSize;
	int RecvBufSize;
	int TcpNoDelay;
	int LingerOnOff;
	int LingerSeconds;
};

// 网络事件回调
typedef
void (*LPNETEVENTCALLBACK)(
	const PersistID& id,
	IoAction action,
	char* pData,
	size_t nBytes,
	char* pUserData
	);


namespace WShellAPI
{
	class DllLoader
	{
	public:
		DllLoader()
		{
		    string app_path;
		    W_GetAppPath(app_path);
		    app_path += "/eWshell.so";

			m_hMod = dlopen(app_path.c_str(), RTLD_LAZY);
			if (!m_hMod)
			{
				printf("无法加载eWShell.so[err: %s]\n", dlerror());
			}
		}
		~DllLoader()
		{
			if (m_hMod)
				dlclose(m_hMod);
			m_hMod = NULL;
		}
		static DllLoader& Inst(){
			static DllLoader load;
			return load;
		}
		void* GetProcAddress(const char* lpProcName)
		{
			if (!m_hMod)
				return 0;
			return dlsym(m_hMod,lpProcName);
		}
	public:
		void* m_hMod;
	};

	/*
	 *	打开服务
	 */
	static bool				Open(
						IN unsigned short nListenPort,
						IN const char* szAddr = NULL,
						IN int nMaxConnections = 5000,
						IN int nRecvBufferSize = 1024,
						IN bool	bUseRawPacket = true,
						IN int nWorkThreads = 1,
						IN WShellOption* option = NULL
						)
	{
		typedef bool (*FUNC)(
						IN unsigned short nListenPort,
						IN const char* szAddr,
						IN int nMaxConnections,
						IN int nRecvBufferSize,
						IN bool	bUseRawPacket,
						IN int nWorkThreads,
						IN WShellOption* option
						);

		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("Open");
		if (func)
		{
			return func(nListenPort, szAddr, nMaxConnections, nRecvBufferSize
				, bUseRawPacket, nWorkThreads, option);
		}
		return false;
	}

	/*
	 *	停止服务
	 */
	static bool				ShutDown()
	{
		typedef bool (*FUNC)();
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("ShutDown");
		if (func)
		{
			return func();
		}
		return false;
	}

	/*
	 *	设置回调
	 */
	static void				SetMsgProc(IN LPNETEVENTCALLBACK lpMsgProc)
	{
		typedef bool (*FUNC)(LPNETEVENTCALLBACK lpMsgProc);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("SetMsgProc");
		if (func)
		{
			func(lpMsgProc);
		}
	}

	/*
	 *	设置用户数据
	 */
	static bool				SetUserData(IN const PersistID& id, IN char* pUserData)
	{
		if (!id.Valid())
		{
			return false;
		}

		typedef bool (*FUNC)(IN const PersistID& id, IN char* pUserData);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("SetUserData");
		if (func)
		{
			return func(id, pUserData);
		}
		return false;
	}

	/*
	*	获取用户数据
	*/
	static bool				GetUserData(IN const PersistID& id, OUT char** ppUserData)
	{
		if (!id.Valid())
		{
			return false;
		}

		typedef bool (*FUNC)(IN const PersistID& id, OUT char** ppUserData);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("GetUserData");
		if (func)
		{
			return func(id, ppUserData);
		}
		return false;
	}

	/*
	 *	发送数据
	 */
	static bool			SendData(
						IN const PersistID& id,
						IN void* pData,
						IN size_t nBytes)
	{
		if (!id.Valid())
		{
			return false;
		}

		typedef bool (*FUNC)(IN const PersistID& id,
						IN void* pData,
						IN size_t nBytes);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("SendData");
		if (func)
		{
			return func(id, pData, nBytes);
		}
		return false;
	}
	/*
	 *	关闭指定连接
	 */
	static bool			Disconnect(IN const PersistID& id)
	{
		if (!id.Valid())
		{
			return false;
		}

		typedef bool (*FUNC)(IN const PersistID& id);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("Disconnect");
		if (func)
		{
			return func(id);
		}
		return false;
	}

	/*
	 *	获取客户端地址
	 */
	static const char*	GetAddr(IN const PersistID& id)
	{
		if (!id.Valid())
		{
			return NULL;
		}

		typedef const char* (*FUNC)(const PersistID& id);
		static FUNC func = (FUNC)DllLoader::Inst().GetProcAddress("GetAddr");
		if (func)
		{
			return func(id);
		}
		return NULL;
	}
};

#endif// end of _Common_h
