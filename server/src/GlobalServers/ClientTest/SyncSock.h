 /*-------------------------------------------------------------------------
	created:	2010/10/09  13:42
	filename: 	e:\SVN\·þÎñÆ÷¶Ë\Source\ClientTest\SyncSock.h
	author:		Deeple
	
	purpose:	
---------------------------------------------------------------------------*/

#pragma  once

#include "winsock.h"
#pragma comment(lib, "ws2_32.lib")


static bool InitSocket()
{
	WSADATA wsaData;
	if (WSAStartup( MAKEWORD( 1, 1 ), &wsaData) != 0)
	{
		return false;
	}
	return true;
}

static SOCKET ConnectServer(const char* addr, u_short uPort)
{
	SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) 
	{
		return INVALID_SOCKET;
	}

	sockaddr_in clientService; 
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = addr ? inet_addr(addr) : 0;
	clientService.sin_port = ::htons(uPort);

	if (connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService)) != 0) 
	{
		closesocket(ConnectSocket);
		return INVALID_SOCKET;
	}

	return ConnectSocket;
}

static void CloseConn(SOCKET& s)
{
	closesocket(s);
	s = INVALID_SOCKET;
}

static bool SendData(SOCKET& s, void* pData, int nLen)
{
	if (!pData)
		return false;

	char * chData = (char*) pData;

	int nSent = 0;

	while (nSent < nLen)
	{
		int n = ::send(s, chData + nSent, nLen - nSent, 0);
		if (n < 1)
		{
			CloseConn(s);
			return false;
		}
		nSent += n;
	}

	return true;
}

static bool RecvData(SOCKET& s, void* pData, int nLen)
{
	if (!pData)
		return false;

	char * chData = (char*) pData;

	int nRecvd = 0;

	while (nRecvd < nLen)
	{
		int n = ::recv(s, chData + nRecvd, nLen - nRecvd, 0);
		if (n < 1)
		{
			CloseConn(s);
			return false;
		}
		nRecvd += n;
	}

	return true;
}

static bool SendMsg(SOCKET& s, void* pData, int nLen)
{
	if (SendData(s, &nLen, sizeof(int))
		&& SendData(s, pData, nLen))
	{
		return true;
	}
	return false;
}

static bool RecvMsg(SOCKET& s, char* pRecvBuf, size_t& nBufLen)
{
	size_t nMsgLen = 0;

	if (!RecvData(s, &nMsgLen, sizeof(int)))
	{
		return false;
	}

	if (nMsgLen > nBufLen)
	{
		CloseConn(s);
		return false;
	}

	if (!RecvData(s, pRecvBuf, (int)nMsgLen))
	{
		return false;
	}

	nBufLen = nMsgLen;

	return true;
}

