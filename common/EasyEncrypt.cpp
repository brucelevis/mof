//
//  EasyEncrypt.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-12-19.
//
//

#include "EasyEncrypt.h"
#include "md5.h"
#include <stdint.h>

string genEncryptKey(int serverId, int sessionId, int sec, int usec)
{
    return strFormat("server_id:%d-session_id:%d-sec:%d-usec:%d", serverId, sessionId, sec, usec);
}


//数据包结构信息
struct CMD_Info
{
	WORD								wDataSize;							//数据大小
	BYTE								cbCheckCode;						//效验字段
	BYTE								cbMessageVer;						//版本标识
};

//数据包命令信息
struct CMD_Command
{
	WORD								wMainCmdID;							//主命令码
	WORD								wSubCmdID;							//子命令码
};

//数据包传递包头
struct CMD_Head
{
	CMD_Info							CmdInfo;							//基础结构
	CMD_Command							CommandInfo;						//命令信息
};

//发送字节映射表
const BYTE g_SendByteMap[256]=
{
	0x70,0x2F,0x40,0x5F,0x44,0x8E,0x6E,0x45,0x7E,0xAB,0x2C,0x1F,0xB4,0xAC,0x9D,0x91,
	0x0D,0x36,0x9B,0x0B,0xD4,0xC4,0x39,0x74,0xBF,0x23,0x16,0x14,0x06,0xEB,0x04,0x3E,
	0x12,0x5C,0x8B,0xBC,0x61,0x63,0xF6,0xA5,0xE1,0x65,0xD8,0xF5,0x5A,0x07,0xF0,0x13,
	0xF2,0x20,0x6B,0x4A,0x24,0x59,0x89,0x64,0xD7,0x42,0x6A,0x5E,0x3D,0x0A,0x77,0xE0,
	0x80,0x27,0xB8,0xC5,0x8C,0x0E,0xFA,0x8A,0xD5,0x29,0x56,0x57,0x6C,0x53,0x67,0x41,
	0xE8,0x00,0x1A,0xCE,0x86,0x83,0xB0,0x22,0x28,0x4D,0x3F,0x26,0x46,0x4F,0x6F,0x2B,
	0x72,0x3A,0xF1,0x8D,0x97,0x95,0x49,0x84,0xE5,0xE3,0x79,0x8F,0x51,0x10,0xA8,0x82,
	0xC6,0xDD,0xFF,0xFC,0xE4,0xCF,0xB3,0x09,0x5D,0xEA,0x9C,0x34,0xF9,0x17,0x9F,0xDA,
	0x87,0xF8,0x15,0x05,0x3C,0xD3,0xA4,0x85,0x2E,0xFB,0xEE,0x47,0x3B,0xEF,0x37,0x7F,
	0x93,0xAF,0x69,0x0C,0x71,0x31,0xDE,0x21,0x75,0xA0,0xAA,0xBA,0x7C,0x38,0x02,0xB7,
	0x81,0x01,0xFD,0xE7,0x1D,0xCC,0xCD,0xBD,0x1B,0x7A,0x2A,0xAD,0x66,0xBE,0x55,0x33,
	0x03,0xDB,0x88,0xB2,0x1E,0x4E,0xB9,0xE6,0xC2,0xF7,0xCB,0x7D,0xC9,0x62,0xC3,0xA6,
	0xDC,0xA7,0x50,0xB5,0x4B,0x94,0xC0,0x92,0x4C,0x11,0x5B,0x78,0xD9,0xB1,0xED,0x19,
	0xE9,0xA1,0x1C,0xB6,0x32,0x99,0xA3,0x76,0x9E,0x7B,0x6D,0x9A,0x30,0xD6,0xA9,0x25,
	0xC7,0xAE,0x96,0x35,0xD0,0xBB,0xD2,0xC8,0xA2,0x08,0xF3,0xD1,0x73,0xF4,0x48,0x2D,
	0x90,0xCA,0xE2,0x58,0xC1,0x18,0x52,0xFE,0xDF,0x68,0x98,0x54,0xEC,0x60,0x43,0x0F
};

//接收字节映射表
const BYTE g_RecvByteMap[256]=
{
	0x51,0xA1,0x9E,0xB0,0x1E,0x83,0x1C,0x2D,0xE9,0x77,0x3D,0x13,0x93,0x10,0x45,0xFF,
	0x6D,0xC9,0x20,0x2F,0x1B,0x82,0x1A,0x7D,0xF5,0xCF,0x52,0xA8,0xD2,0xA4,0xB4,0x0B,
	0x31,0x97,0x57,0x19,0x34,0xDF,0x5B,0x41,0x58,0x49,0xAA,0x5F,0x0A,0xEF,0x88,0x01,
	0xDC,0x95,0xD4,0xAF,0x7B,0xE3,0x11,0x8E,0x9D,0x16,0x61,0x8C,0x84,0x3C,0x1F,0x5A,
	0x02,0x4F,0x39,0xFE,0x04,0x07,0x5C,0x8B,0xEE,0x66,0x33,0xC4,0xC8,0x59,0xB5,0x5D,
	0xC2,0x6C,0xF6,0x4D,0xFB,0xAE,0x4A,0x4B,0xF3,0x35,0x2C,0xCA,0x21,0x78,0x3B,0x03,
	0xFD,0x24,0xBD,0x25,0x37,0x29,0xAC,0x4E,0xF9,0x92,0x3A,0x32,0x4C,0xDA,0x06,0x5E,
	0x00,0x94,0x60,0xEC,0x17,0x98,0xD7,0x3E,0xCB,0x6A,0xA9,0xD9,0x9C,0xBB,0x08,0x8F,
	0x40,0xA0,0x6F,0x55,0x67,0x87,0x54,0x80,0xB2,0x36,0x47,0x22,0x44,0x63,0x05,0x6B,
	0xF0,0x0F,0xC7,0x90,0xC5,0x65,0xE2,0x64,0xFA,0xD5,0xDB,0x12,0x7A,0x0E,0xD8,0x7E,
	0x99,0xD1,0xE8,0xD6,0x86,0x27,0xBF,0xC1,0x6E,0xDE,0x9A,0x09,0x0D,0xAB,0xE1,0x91,
	0x56,0xCD,0xB3,0x76,0x0C,0xC3,0xD3,0x9F,0x42,0xB6,0x9B,0xE5,0x23,0xA7,0xAD,0x18,
	0xC6,0xF4,0xB8,0xBE,0x15,0x43,0x70,0xE0,0xE7,0xBC,0xF1,0xBA,0xA5,0xA6,0x53,0x75,
	0xE4,0xEB,0xE6,0x85,0x14,0x48,0xDD,0x38,0x2A,0xCC,0x7F,0xB1,0xC0,0x71,0x96,0xF8,
	0x3F,0x28,0xF2,0x69,0x74,0x68,0xB7,0xA3,0x50,0xD0,0x79,0x1D,0xFC,0xCE,0x8A,0x8D,
	0x2E,0x62,0x30,0xEA,0xED,0x2B,0x26,0xB9,0x81,0x7C,0x46,0x89,0x73,0xA2,0xF7,0x72
};

//数据加密密钥
//const static DWORD g_dwPacketKey=0xA55AA55A;
const static DWORD g_dwPacketKey=0xB44BB44B;	// 新游key

//数据包版本
#define SOCKET_VER						65

//随机映射
static WORD SeedRandMap(WORD wSeed)
{
	DWORD dwHold=wSeed;
	return (WORD)((dwHold=dwHold*241103L+2533101L)>>16);
}

void ClientEncrypt::Connect(const char* pwd)
{
    MD5 md5(pwd);
	md5.finalize();
    string pwdDigest = md5.hexdigest();
    memcpy(&mSerialNo, pwdDigest.c_str(), 4);
    mRc5.RC5_SetPWD(pwd);
    return;
    
	m_dwSendPacketCount = 0;
	m_dwRecvPacketCount = 0;
    
	m_cbSendRound = 0;
	m_cbRecvRound = 0;
	m_dwSendXorKey = 0;
	m_dwRecvXorKey = 0;
    
	//创建随机密钥
	DWORD dwXorKey = rand() ;
	dwXorKey=SeedRandMap((WORD)dwXorKey);
	dwXorKey|=((DWORD)SeedRandMap((WORD)(dwXorKey>>16)))<<16;
	dwXorKey^=g_dwPacketKey;
	m_dwSendXorKey=dwXorKey;
	m_dwRecvXorKey=dwXorKey;
}


BYTE ClientEncrypt::MapSendByte( BYTE const cbData )
{
	BYTE cbMap=g_SendByteMap[(BYTE)(cbData+m_cbSendRound)];
	m_cbSendRound+=3;
	return cbMap;
}

BYTE ClientEncrypt::MapRecvByte( BYTE const cbData )
{
	BYTE cbMap=g_RecvByteMap[cbData]-m_cbRecvRound;
	m_cbRecvRound+=3;
	return cbMap;
}

void ClientEncrypt::SendEncrypt(void * pData, WORD wDataSize)//, BYTE* & out, WORD& outSize)
{
    for (int i = 0; i < wDataSize / 4; i++)
    {
        char* pTemp = ((char*)pData) + i * 4;
        uint32_t data;
        memcpy(&data, pTemp, 4);
        data ^= mSerialNo;
        memcpy(pTemp, &data, 4);
    }
    mRc5.RC5_Encrypt(pData, wDataSize);
    
    MD5 md5;
    md5.update((char*)&mSerialNo, 4);
	md5.finalize();
    string pwdDigest = md5.hexdigest();
    memcpy(&mSerialNo, pwdDigest.c_str(), 4);

    return;
    
	//assert( pData != NULL &&  wDataSize <= ENCRYPT_NETPACK_BUF_SIZE && wDataSize > 0);
    
	//CMD_Head * pHead=(CMD_Head *)(&m_EncryptBuffer);
	//memcpy(pHead+1,pData,wDataSize);
	//out = (BYTE*)(&m_EncryptBuffer);
	//outSize = EncryptBuffer(m_EncryptBuffer,sizeof(CMD_Head)+wDataSize);
}

WORD ClientEncrypt::EncryptBuffer( BYTE * pcbDataBuffer, WORD wDataSize)
{
	assert(wDataSize>=sizeof(CMD_Head));
    
	//调整长度
	WORD wEncryptSize=wDataSize-sizeof(CMD_Info),wSnapCount=0;
	if ((wEncryptSize%sizeof(DWORD))!=0)
	{
		wSnapCount=sizeof(DWORD)-wEncryptSize%sizeof(DWORD);
		memset(pcbDataBuffer+sizeof(CMD_Info)+wEncryptSize,0,wSnapCount);
	}
    
	//效验码与字节映射
	BYTE cbCheckCode=0;
	for (WORD i=sizeof(CMD_Info);i<wDataSize;i++)
	{
		cbCheckCode+=pcbDataBuffer[i];
		pcbDataBuffer[i]=MapSendByte(pcbDataBuffer[i]);
	}
    
	//填写信息头
	CMD_Head * pHead=(CMD_Head *)pcbDataBuffer;
	pHead->CmdInfo.cbCheckCode=~cbCheckCode+1;
	pHead->CmdInfo.wDataSize=wDataSize;
	pHead->CmdInfo.cbMessageVer=SOCKET_VER;
    
	//创建密钥
	DWORD dwXorKey=m_dwSendXorKey;
    
	//加密数据
	WORD * pwSeed=(WORD *)(pcbDataBuffer+sizeof(CMD_Info));
	DWORD * pdwXor=(DWORD *)(pcbDataBuffer+sizeof(CMD_Info));
	WORD wEncrypCount=(wEncryptSize+wSnapCount)/sizeof(DWORD);
	for (int i=0;i<wEncrypCount;i++)
	{
		*pdwXor++^=dwXorKey;
		dwXorKey=SeedRandMap(*pwSeed++);
		dwXorKey|=((DWORD)SeedRandMap(*pwSeed++))<<16;
		dwXorKey^=g_dwPacketKey;
	}
    
	//插入密钥
	if (m_dwSendPacketCount==0)
	{
		memmove(pcbDataBuffer+sizeof(CMD_Head)+sizeof(DWORD),pcbDataBuffer+sizeof(CMD_Head),wDataSize-sizeof(CMD_Head));
		*((DWORD *)(pcbDataBuffer+sizeof(CMD_Head)))=m_dwSendXorKey;
		pHead->CmdInfo.wDataSize+=sizeof(DWORD);
		wDataSize+=sizeof(DWORD);
	}
    
	//设置变量
	m_dwSendPacketCount++;
	m_dwSendXorKey=dwXorKey;
    
	return wDataSize;
}

void ClientEncrypt::RecvDecrypt(void * pData, WORD wDataSize, BYTE* & out, WORD& outSize)
{
	//效验参数
	if( wDataSize < sizeof(CMD_Head) ||
       wDataSize > ENCRYPT_NETPACK_BUF_SIZE ) throw "包大小有误";
    
	CMD_Head * pHead=(CMD_Head *)pData;
	if ( pHead->CmdInfo.cbMessageVer != SOCKET_VER ) throw "数据包版本错误";
	if ( pHead->CmdInfo.wDataSize != wDataSize) throw "包头有误";
    
	//拷贝数据
	memcpy(m_DecryptBuffer,pData,wDataSize);
    
	//解密数据
	WORD wRealySize=CrevasseBuffer(m_DecryptBuffer,wDataSize);
	assert( wRealySize >= sizeof(CMD_Head) );
    
	//解释数据
	outSize = wRealySize - sizeof(CMD_Head);
	out = m_DecryptBuffer + sizeof(CMD_Head);
    
	m_dwRecvPacketCount++;
}

WORD ClientEncrypt::CrevasseBuffer(BYTE * pcbDataBuffer, WORD wDataSize)
{
    return 0;
}


//////////////////////////////////////////////////////////////////////////

//映射发送数据
BYTE ServerEncrypt::MapSendByte( BYTE const cbData )
{
	BYTE cbMap=g_SendByteMap[(BYTE)(cbData+m_cbSendRound)];
	m_cbSendRound+=3;
	return cbMap;
}

//映射接收数据
BYTE ServerEncrypt::MapRecvByte( BYTE const cbData )
{
	BYTE cbMap=g_RecvByteMap[cbData]-m_cbRecvRound;
	m_cbRecvRound+=3;
	return cbMap;
}

void ServerEncrypt::Connect(const char* pwd)
{
    MD5 md5(pwd);
	md5.finalize();
    string pwdDigest = md5.hexdigest();
    memcpy(&mSerialNo, pwdDigest.c_str(), 4);
    //mSerialNo = *((uint32_t*)pwdDigest.c_str());
    mRc5.RC5_SetPWD(pwd);
    
    return;
    
	m_dwSendPacketCount = 0;
	m_dwRecvPacketCount = 0;
	m_cbSendRound = 0;
	m_cbRecvRound = 0;
	m_dwSendXorKey = 0;
	m_dwRecvXorKey = 0;
}

void ServerEncrypt::SendEncrypt( void * pData, WORD wDataSize, BYTE* & out, WORD& outSize )
{
    
	assert( pData != NULL &&  wDataSize <= ENCRYPT_NETPACK_BUF_SIZE && wDataSize > 0);
    
	CMD_Head * pHead=(CMD_Head *)(&m_EncryptBuffer);
	memcpy(pHead+1,pData,wDataSize);
	out = (BYTE*)(&m_EncryptBuffer);
	outSize = EncryptBuffer(m_EncryptBuffer,sizeof(CMD_Head)+wDataSize);
}


WORD ServerEncrypt::EncryptBuffer( BYTE * pcbDataBuffer, WORD wDataSize )
{
    return 0;
}

void dumpBin(void* data, int len)
{
    static const char *st = "0123456789abcdef";
    string str;
    for (int i = 0; i < len; i++)
    {
        int a, b;
        a = ((unsigned char*)data)[i] >> 4;
        b = ((unsigned char*)data)[i] & 0xF;
        str = strFormat("%s%c%c", str.c_str(), st[a], st[b]);
    }
    printf("dump bin %s\n", str.c_str());
}

bool ServerEncrypt::RecvDecrypt( void * pData, WORD wDataSize)//, BYTE* & out, WORD& outSize )
{
    if (mRc5.RC5_Decrypt(pData, wDataSize))
    {
        
        for (int i = 0; i < wDataSize / 4; i++)
        {
            char* pTemp = ((char*)pData) + i * 4;
            uint32_t data;
            memcpy(&data, pTemp, 4);
            data ^= mSerialNo;
            memcpy(pTemp, &data, 4);
            
            //uint32_t* data = ((uint32_t*)pData) + i;
            //*data ^= mSerialNo;
        }
        
        MD5 md5;
        md5.update((char*)&mSerialNo, 4);
		md5.finalize();
        string pwdDigest = md5.hexdigest();
        memcpy(&mSerialNo, pwdDigest.c_str(), 4);
        //mSerialNo++;
        return true;
    }
    
    return false;
	//效验参数
	//if( wDataSize < sizeof(CMD_Head) ||
    //   wDataSize > ENCRYPT_NETPACK_BUF_SIZE ) throw "包大小有误";
    
	//CMD_Head * pHead=(CMD_Head *)pData;
	//if ( pHead->CmdInfo.cbMessageVer != SOCKET_VER ) throw "数据包版本错误";
	//if ( pHead->CmdInfo.wDataSize != wDataSize) throw "包头有误";
    
	//拷贝数据
	//memcpy(m_DecryptBuffer,pData,wDataSize);
    
	//解密数据
	//WORD wRealySize=CrevasseBuffer(m_DecryptBuffer,wDataSize);
	//assert( wRealySize >= sizeof(CMD_Head) );
    
	//解释数据
	//outSize = wRealySize - sizeof(CMD_Head);
	//out = m_DecryptBuffer + sizeof(CMD_Head);
    
	//m_dwRecvPacketCount++;
}


WORD ServerEncrypt::CrevasseBuffer( BYTE * pcbDataBuffer, WORD wDataSize )
{
	WORD i = 0;
	//效验参数
	assert(wDataSize>=sizeof(CMD_Head));
	assert(((CMD_Head *)pcbDataBuffer)->CmdInfo.wDataSize==wDataSize);
    
	//调整长度
	WORD wSnapCount=0;
	if ((wDataSize%sizeof(DWORD))!=0)
	{
		wSnapCount=sizeof(DWORD)-wDataSize%sizeof(DWORD);
		memset(pcbDataBuffer+wDataSize,0,wSnapCount);
	}
    
	//提取密钥
	if (m_dwRecvPacketCount==0)
	{
		assert(wDataSize>=(sizeof(CMD_Head)+sizeof(DWORD)));
		if (wDataSize<(sizeof(CMD_Head)+sizeof(DWORD))) throw "数据包解密长度错误";
		m_dwRecvXorKey=*(DWORD *)(pcbDataBuffer+sizeof(CMD_Head));
		m_dwSendXorKey=m_dwRecvXorKey;
		memmove(pcbDataBuffer+sizeof(CMD_Head),pcbDataBuffer+sizeof(CMD_Head)+sizeof(DWORD),
                wDataSize-sizeof(CMD_Head)-sizeof(DWORD));
		wDataSize-=sizeof(DWORD);
		((CMD_Head *)pcbDataBuffer)->CmdInfo.wDataSize-=sizeof(DWORD);
	}
    
	//解密数据
	DWORD dwXorKey=m_dwRecvXorKey;
	DWORD * pdwXor=(DWORD *)(pcbDataBuffer+sizeof(CMD_Info));
	WORD  * pwSeed=(WORD *)(pcbDataBuffer+sizeof(CMD_Info));
	WORD wEncrypCount=(wDataSize+wSnapCount-sizeof(CMD_Info))/4;
	for (i=0;i<wEncrypCount;i++)
	{
		if ((i==(wEncrypCount-1))&&(wSnapCount>0))
		{
			BYTE * pcbKey=((BYTE *)&m_dwRecvXorKey)+sizeof(DWORD)-wSnapCount;
			memcpy(pcbDataBuffer+wDataSize,pcbKey,wSnapCount);
		}
		dwXorKey=SeedRandMap(*pwSeed++);
		dwXorKey|=((DWORD)SeedRandMap(*pwSeed++))<<16;
		dwXorKey^=g_dwPacketKey;
		*pdwXor++^=m_dwRecvXorKey;
		m_dwRecvXorKey=dwXorKey;
	}
    
	//效验码与字节映射
	CMD_Head * pHead=(CMD_Head *)pcbDataBuffer;
	BYTE cbCheckCode=pHead->CmdInfo.cbCheckCode;;
	for (i=sizeof(CMD_Info);i<wDataSize;i++)
	{
		pcbDataBuffer[i]=MapRecvByte(pcbDataBuffer[i]);
		cbCheckCode+=pcbDataBuffer[i];
	}
    
    // 很可能是非法改包
	if (cbCheckCode!=0) throw "数据包效验码错误";
    
	return wDataSize;
}





