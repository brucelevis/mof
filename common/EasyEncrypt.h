//
//  EasyEncrypt.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-12-19.
//
//

#pragma once
#include "Utils.h"
#include "rc5.h"

const int ENCRYPT_NETPACK_BUF_SIZE = 32 * 1024;

string genEncryptKey(int serverId, int sessionId, int sec, int usec);

class ClientEncrypt
{
public:
	void Connect(const char* pwd);
	void SendEncrypt(void * pData, WORD wDataSize);//, BYTE* & out, WORD& outSize);
	void RecvDecrypt(void * pData, WORD wDataSize, BYTE* & out, WORD& outSize);
protected:
	WORD EncryptBuffer(BYTE * pcbDataBuffer, WORD wDataSize);
	WORD CrevasseBuffer(BYTE * pcbDataBuffer, WORD wDataSize);
	inline BYTE MapSendByte(BYTE const cbData);
	inline BYTE MapRecvByte(BYTE const cbData);
protected:
	DWORD							m_dwSendPacketCount;				//发送计数
	DWORD							m_dwRecvPacketCount;				//接受计数
    
	BYTE							m_cbSendRound;						//字节映射
	BYTE							m_cbRecvRound;						//字节映射
	DWORD							m_dwSendXorKey;						//发送密钥
	DWORD							m_dwRecvXorKey;						//接收密钥
    
	BYTE							m_EncryptBuffer[ENCRYPT_NETPACK_BUF_SIZE+128];
	BYTE							m_DecryptBuffer[ENCRYPT_NETPACK_BUF_SIZE+128];
    
    RC5_Encrypt_Stream mRc5;
    uint32_t                mSerialNo;
};

class ServerEncrypt
{
public:
	void Connect(const char* pwd);
	void SendEncrypt(void * pData, WORD wDataSize, BYTE* & out, WORD& outSize);
    
	// 客户端造假包会抛异常
	bool RecvDecrypt(void * pData, WORD wDataSize);//, BYTE* & out, WORD& outSize);
protected:
	WORD EncryptBuffer(BYTE * pcbDataBuffer, WORD wDataSize);
	WORD CrevasseBuffer(BYTE * pcbDataBuffer, WORD wDataSize);
	inline BYTE MapSendByte(BYTE const cbData);
	inline BYTE MapRecvByte(BYTE const cbData);
protected:
	DWORD							m_dwSendPacketCount;				//发送计数
	DWORD							m_dwRecvPacketCount;				//接受计数
    
	BYTE							m_cbSendRound;						//字节映射
	BYTE							m_cbRecvRound;						//字节映射
	DWORD							m_dwSendXorKey;						//发送密钥
	DWORD							m_dwRecvXorKey;						//接收密钥
    
	BYTE							m_EncryptBuffer[ENCRYPT_NETPACK_BUF_SIZE+128];
	BYTE							m_DecryptBuffer[ENCRYPT_NETPACK_BUF_SIZE+128];
    
    RC5_Encrypt_Stream mRc5;
    uint32_t mSerialNo;
};
