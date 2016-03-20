//--------------------------------------------------------------------
// 文件名:		MsgBuf.cpp
// 内  容:
// 说  明:
//--------------------------------------------------------------------
#include <stdio.h>
#include "MsgBuf.h"

#define INITIAL_BUF_SIZE 128
char* __NEW(int size)
{
    char* ptr = new char[size];
    memset(ptr, 0, size);
    return ptr;
}
#define __FREE(p,size) delete [] p

#pragma warning(disable:4996)

void __AssertionFail(const char * strFile, int nLine)
{
	fflush(stdout);
	fprintf(stderr, "Asssertion failed: file %s, line %d", strFile, nLine);
	fflush(stderr);
	abort();
}

//////////////////////////////////////////////////////////////////////
// CExceptMsg

CExceptMsg::CExceptMsg(const char * msg)
{
	_Assert(msg != NULL);

	m_pMsg = msg;
}

CExceptMsg::~CExceptMsg()
{
}

const char * CExceptMsg::GetMsg() const
{
	return m_pMsg;
}

// class 自适应Buf
// ----------------------------------------------------
CMsgTyped::CMsgTyped()
{
	m_pBuf = NULL;
	m_nBufSize = 0;
	m_pRWPtr = NULL;
	m_bWriteBuf = true;
	m_bAllocMem = true;
	m_bIsLuaRet = false;

	try
	{
		m_pBuf = __NEW(INITIAL_BUF_SIZE);

		if (m_pBuf)
		{
			m_nBufSize = INITIAL_BUF_SIZE;
			m_pRWPtr = m_pBuf;
		}
	}
	catch (...)
	{
		throw CExceptMsg("No Memory");
	}
}

CMsgTyped::CMsgTyped(size_t nBufSize)
{
	m_pBuf = NULL;
	m_nBufSize = 0;
	m_pRWPtr = NULL;
	m_bWriteBuf = true;
	m_bAllocMem = true;
	m_bIsLuaRet = false;

	if (nBufSize <= 0)
	{
		return;
	}

	try
	{
		m_pBuf = __NEW(nBufSize);

		if (m_pBuf)
		{
			m_nBufSize = nBufSize;
			m_pRWPtr = m_pBuf;
		}
	}
	catch (...)
	{
		throw CExceptMsg("No Memory");
	}
}

CMsgTyped::CMsgTyped(const CMsgTyped& another)
{
	m_pBuf = NULL;
	m_nBufSize = 0;
	m_bIsLuaRet = false;

	if (another.m_pBuf && another.m_nBufSize > 0)
	{
		try
		{
			m_pBuf = __NEW(another.m_nBufSize);
			memcpy(m_pBuf, another.m_pBuf, another.m_nBufSize);
			m_nBufSize = another.m_nBufSize;
		}
		catch (...)
		{
			throw CExceptMsg("No Memory");
		}
	}

	m_pRWPtr = m_pBuf + ( another.m_pRWPtr - another.m_pBuf);
	m_bWriteBuf = another.m_bWriteBuf;
	m_bAllocMem = true;
}

void CMsgTyped::operator =(const CMsgTyped& another)
{
	// 删除原来的BUFFER
	if (m_pBuf)
	{
		__FREE(m_pBuf, m_nBufSize);
	}

	m_pBuf = NULL;
	m_nBufSize = 0;
	m_bIsLuaRet = false;

	if (another.m_pBuf && another.m_nBufSize > 0)
	{
		try
		{
			m_pBuf = __NEW(another.m_nBufSize);
			memcpy(m_pBuf, another.m_pBuf, another.m_nBufSize);
			m_nBufSize = another.m_nBufSize;
		}
		catch (...)
		{
			throw CExceptMsg("No Memory");
		}
	}

	m_pRWPtr = m_pBuf + ( another.m_pRWPtr - another.m_pBuf);
	m_bWriteBuf = another.m_bWriteBuf;
	m_bAllocMem = true;
}

CMsgTyped::CMsgTyped(size_t len, const void * pdata, bool bAllocMem)
{
	m_pBuf = NULL;
	m_nBufSize = 0;
	m_pRWPtr = NULL;
	m_bWriteBuf = false;
	m_bAllocMem = bAllocMem;
	m_bIsLuaRet = false;

	if (pdata && len > 0)
	{
		if (bAllocMem)
		{
			try
			{
				m_pBuf = __NEW(len);

				if (m_pBuf)
				{
					m_nBufSize = len;
					m_pRWPtr = m_pBuf;
					memcpy(m_pBuf, pdata, len);
				}
			}
			catch (...)
			{
				throw CExceptMsg("No Memory");
			}
		}
		else
		{
			m_pBuf = (char*)pdata;
			m_nBufSize = len;
			m_pRWPtr = m_pBuf;
		}
	}
}

CMsgTyped::~CMsgTyped()
{
	if (m_pBuf && m_bAllocMem)
	{
		__FREE(m_pBuf,m_nBufSize);
	}

	m_pBuf = NULL;
	m_nBufSize = 0;
	m_pRWPtr = NULL;
	m_bWriteBuf = false;
	m_bAllocMem = false;
}

void CMsgTyped::Swap(CMsgTyped& another)
{
	char* m_pBuf__ = m_pBuf;
	size_t m_nBufSize__ = m_nBufSize;
	char* m_pRWPtr__ = m_pRWPtr;
	bool m_bWriteBuf__ = m_bWriteBuf;

	m_pBuf = another.m_pBuf;
	m_nBufSize = another.m_nBufSize;
	m_pRWPtr = another.m_pRWPtr;
	m_bWriteBuf = another.m_bWriteBuf;

	another.m_pBuf = m_pBuf__;
	another.m_nBufSize = m_nBufSize__;
	another.m_pRWPtr = m_pRWPtr__;
	another.m_bWriteBuf = m_bWriteBuf__;
}

void CMsgTyped::SetAsReadBuf(bool bReadBuf)
{
	m_bWriteBuf = !bReadBuf;
}

void CMsgTyped::SeekToBegin()
{
	m_pRWPtr = m_pBuf;
}

const void * CMsgTyped::GetData() const
{
	return m_pBuf;
}

size_t CMsgTyped::GetLength() const
{
	if (m_bWriteBuf)
	{
		return (size_t)(m_pRWPtr - m_pBuf);
	}
	else
	{
		return m_nBufSize;
	}
}

// 设置
bool CMsgTyped::SetInt(int value)
{
	if (!SetType(TYPE_INT))
	{
		return false;
	}

	if (!Acquire(sizeof(int)))
	{
		return false;
	}

	(*(int*)m_pRWPtr) = value;
	m_pRWPtr += sizeof(int);

	//*m_pRWPtr = 0;

	return true;
}

bool CMsgTyped::SetString(const char * value)
{
	if (!value)
	{
		value = "";
	}

	if (!SetType(TYPE_STRING))
	{
		return false;
	}

	// 消息长度
	int nByteNeed = (int)strlen(value) + 1;

	// 插入一个int
	if (!Acquire(sizeof(int)))
	{
		return false;
	}

	(*(int*)m_pRWPtr) = nByteNeed;
	m_pRWPtr += sizeof(int);

	if (!Acquire(nByteNeed))
	{
		return false;
	}

	memcpy(m_pRWPtr, value, nByteNeed);
	m_pRWPtr += nByteNeed;

	//*m_pRWPtr = 0;

	return true;
}

bool CMsgTyped::SetFloat(float value)
{
	if (!SetType(TYPE_FLOAT))
	{
		return false;
	}

	if (!Acquire(sizeof(float)))
	{
		return false;
	}

	(*(float*)m_pRWPtr) = value;
	m_pRWPtr += sizeof(float);

	//*m_pRWPtr = 0;

	return true;
}

size_t  mywcslen(
        const wchar_t * wcs
        )
{
        const wchar_t *eos = wcs;

        while( *eos++ ) ;

        return( (size_t)(eos - wcs - 1) );
}

bool CMsgTyped::SetWideStr(const wchar_t * value)
{
	if (!value)
	{
		value = L"";
	}

	if (!SetType(TYPE_WIDESTR))
	{
		return false;
	}

	int nByteNeed = (int)((mywcslen(value) + 1) * 2);

	// 插入一个int
	if (!Acquire(sizeof(int)))
	{
		return false;
	}

	(*(int*)m_pRWPtr) = nByteNeed;
	m_pRWPtr += sizeof(int);

	if (!Acquire(nByteNeed))
	{
		return false;
	}

	//::wcscpy((wchar_t *)m_pRWPtr, value);
	memcpy(m_pRWPtr, value, nByteNeed);
	m_pRWPtr += nByteNeed;

	//*m_pRWPtr = 0;

	return true;
}

bool CMsgTyped::SetBinary(void* pData, size_t nLen)
{
	if (!pData)
	{
		nLen = 0;
	}

	if (!SetType(TYPE_BINARY))
	{
		return false;
	}

	// 插入一个int
	if (!Acquire(sizeof(int)))
	{
		return false;
	}

	(*(int*)m_pRWPtr) = (int)nLen;
	m_pRWPtr += sizeof(int);

	if (!Acquire(nLen))
	{
		return false;
	}

	if (pData && nLen > 0)
	{
		memcpy(m_pRWPtr, pData, nLen);
		m_pRWPtr += nLen;
	}

	//*m_pRWPtr = 0;

	return true;
}

bool CMsgTyped::SetPointer(void* value)
{
	if (!SetType(TYPE_POINTER))
	{
		return false;
	}

	if (!Acquire(sizeof(void*)))
	{
		return false;
	}

	(*(void**)m_pRWPtr) = value;
	m_pRWPtr += sizeof(void*);

	//*m_pRWPtr = 0;

	return true;
}

bool CMsgTyped::SetBool(bool value)
{
	if (!SetType(TYPE_BOOL))
	{
		return false;
	}

	// 写入整形数1
	if (!Acquire(sizeof(int)))
	{
		return false;
	}

	(*(int*)m_pRWPtr) = (value ? 1 : 0);
	m_pRWPtr += sizeof(int);

	//*m_pRWPtr = 0;

	return true;
}

bool CMsgTyped::SetByte(char  value)
{
    if (!SetType(TYPE_BYTE))
	{
		return false;
	}

	if (!Acquire(sizeof(char)))
	{
		return false;
	}

	(*(char*)m_pRWPtr) = value;
	m_pRWPtr += sizeof(char);

	//*m_pRWPtr = 0;

	return true;
}

bool CMsgTyped::SetWord(short value)
{
    if (!SetType(TYPE_WORD))
	{
		return false;
	}

	if (!Acquire(sizeof(short)))
	{
		return false;
	}

	(*(short*)m_pRWPtr) = value;
	m_pRWPtr += sizeof(short);

	//*m_pRWPtr = 0;

	return true;
}

bool CMsgTyped::InsertInt(int value)
{
	if (!SetInt(value))
	{
		return false;
	}

	char* pTemp = m_pRWPtr - 5;

	// 拷贝新增加的字节到临时空间
	char tmp[5];
	memcpy(tmp, pTemp, 5);

	// 移动原来的空间
	for (char* p = pTemp-1; p >= m_pBuf; p --)
	{
		*(p+5) = *p;
	}

	// 拷贝临时空间到前部
	memcpy(m_pBuf, tmp, 5);

	return true;
}

bool CMsgTyped::InsertInt(void* val1, void* val2, void * val3, void* val4)
{
    // 计算有效参数个数
    int cnt = 0;
    do
    {
        if (!val1) break;
        cnt ++;

        if (!val2) break;
        cnt ++;

        if (!val3) break;
        cnt ++;

        if (!val4) break;
        cnt ++;
    }
    while (false);

    if (cnt == 0)
    {
        return false;
    }

    // 需要的内存大小
    size_t newSize = cnt * ( sizeof(int) + sizeof(BYTE) );

    // 开辟空间
    if (!Acquire(  newSize ))
    {
        return false;
    }

    // 移动原来的内存
	for (char* p = m_pRWPtr-1; p >= m_pBuf; p --)
	{
		*(p + newSize) = *p;
	}

	// 记录写指针
	char* ptr = m_pRWPtr + newSize;

	// 移动到前部
	m_pRWPtr = m_pBuf;

	// 写数据
    do
    {
        if (!val1) break;
        SetInt( *(int*)val1 );

        if (!val2) break;
        SetInt( *(int*)val2 );

        if (!val3) break;
        SetInt( *(int*)val3 );

        if (!val4) break;
        SetInt( *(int*)val4 );
    }
    while (false);

    m_pRWPtr = ptr;

    return true;
}

#define setValueAddReturn(type,type2)\
            if (idx == index)\
            {\
                *read.m_pRWPtr = type;\
                read.m_pRWPtr ++;\
                (*(type2*)read.m_pRWPtr) = value;\
                read.m_pRWPtr += sizeof(type2);\
                return true;\
            }\

bool CMsgTyped::SetAt(int index, int value)
{
    if (index < 0) return false;

    CMsgTyped read(GetLength(),  m_pBuf, false);

    DATA_TYPES type;
    int idx = 0;
    while ( ( type = read.TestType() ) != TYPE_NONE)
    {
        if (type == TYPE_INT)
        {
            // 找到了
            setValueAddReturn(TYPE_INT, int);

            read.IntVal();
        }
        else if (type == TYPE_STRING)
        {
            read.StringVal();
        }
        else if (type == TYPE_FLOAT)
        {
            // 找到了
            setValueAddReturn(TYPE_INT, int);

            read.FloatVal();
        }
        else if (type == TYPE_BYTE)
        {
            read.ByteVal();
        }
        else if (type == TYPE_WORD)
        {
            read.WordVal();
        }
        else if (type == TYPE_BOOL)
        {
            // 找到了
            setValueAddReturn(TYPE_INT, int);

            read.BoolValue();
        }
        else if (type == TYPE_WIDESTR)
        {
            read.WideStrVal();
        }
        else if (type == TYPE_BINARY)
        {
            void* pData;
            size_t nLen;
            read.BinaryVal(pData, nLen);
        }
        else if (type == TYPE_POINTER)
        {
            read.GetPointer();
        }
        else break;

        idx ++;
    }

    return false;
}

bool CMsgTyped::SetAt(int index, float value)
{
  if (index < 0) return false;

    CMsgTyped read(GetLength(),  m_pBuf, false);

    DATA_TYPES type;
    int idx = 0;
    while ( ( type = read.TestType() ) != TYPE_NONE)
    {
        if (type == TYPE_INT)
        {
            // 找到了
            setValueAddReturn(TYPE_FLOAT, float);

            read.IntVal();
        }
        else if (type == TYPE_STRING)
        {
            read.StringVal();
        }
        else if (type == TYPE_FLOAT)
        {
            // 找到了
            setValueAddReturn(TYPE_FLOAT, float);

            read.FloatVal();
        }
        else if (type == TYPE_BYTE)
        {
            read.ByteVal();
        }
        else if (type == TYPE_WORD)
        {
            read.WordVal();
        }
        else if (type == TYPE_BOOL)
        {
            // 找到了
            setValueAddReturn(TYPE_FLOAT, float);

            read.BoolValue();
        }
        else if (type == TYPE_WIDESTR)
        {
            read.WideStrVal();
        }
        else if (type == TYPE_BINARY)
        {
            void* pData;
            size_t nLen;
            read.BinaryVal(pData, nLen);
        }
        else if (type == TYPE_POINTER)
        {
            read.GetPointer();
        }
        else break;

        idx ++;
    }

    return false;
}

bool CMsgTyped::SetAt(int index, bool value)
{
  if (index < 0) return false;

    CMsgTyped read(GetLength(),  m_pBuf, false);

    DATA_TYPES type;
    int idx = 0;
    while ( ( type = read.TestType() ) != TYPE_NONE)
    {
        if (type == TYPE_INT)
        {
            // 找到了
            setValueAddReturn(TYPE_BOOL, bool);

            read.IntVal();
        }
        else if (type == TYPE_STRING)
        {
            read.StringVal();
        }
        else if (type == TYPE_FLOAT)
        {
            // 找到了
            setValueAddReturn(TYPE_BOOL, bool);

            read.FloatVal();
        }
        else if (type == TYPE_BYTE)
        {
            read.ByteVal();
        }
        else if (type == TYPE_WORD)
        {
            read.WordVal();
        }
        else if (type == TYPE_BOOL)
        {
            // 找到了
            setValueAddReturn(TYPE_BOOL, bool);

            read.BoolValue();
        }
        else if (type == TYPE_WIDESTR)
        {
            read.WideStrVal();
        }
        else if (type == TYPE_BINARY)
        {
            void* pData;
            size_t nLen;
            read.BinaryVal(pData, nLen);
        }
        else if (type == TYPE_POINTER)
        {
            read.GetPointer();
        }
        else break;

        idx ++;
    }

    return false;
}

// 获取
int CMsgTyped::IntVal()
{
	DATA_TYPES dataType = GetType();
	if (dataType != TYPE_INT)
	{
	    // Lua 回调可以在float 和 int 之间转换
	    if (m_bIsLuaRet && dataType == TYPE_FLOAT)
	    {
	        m_pRWPtr --;
	        return (int)FloatVal();
	    }

		throw CExceptMsg("DataType Error, Require TYPE_INT");
		return 0;
	}

	if (!Acquire(sizeof(int)))
	{
		throw CExceptMsg("Out Of range");
		return 0;
	}

	int iRet = (*(int*)m_pRWPtr);

	m_pRWPtr += sizeof(int);

	return iRet;
}

const char * CMsgTyped::StringVal()
{
	DATA_TYPES dataType = GetType();
	if (dataType != TYPE_STRING)
	{
		throw CExceptMsg("DataType Error, Require TYPE_STRING");
		return 0;
	}

	// 读取长度
	if (!Acquire(sizeof(int)))
	{
		throw CExceptMsg("Out Of range");
		return 0;
	}

	int nBytes = (*(int*)m_pRWPtr);
	m_pRWPtr += sizeof(int);

	if (!Acquire(nBytes))
	{
		throw CExceptMsg("Out Of range");
		return 0;
	}

	// 读取数据
	const char* ret = m_pRWPtr;

	m_pRWPtr += nBytes;

	return ret;
}

float CMsgTyped::FloatVal()
{
	DATA_TYPES dataType = GetType();
	if (dataType != TYPE_FLOAT)
	{
		throw CExceptMsg("DataType Error, Require TYPE_FLOAT");
		return 0.0f;
	}

	if (!Acquire(sizeof(float)))
	{
		throw CExceptMsg("Out Of range");
		return 0.0f;
	}

	float iRet = (*(float*)m_pRWPtr);

	m_pRWPtr += sizeof(float);

	return iRet;
}

const wchar_t * CMsgTyped::WideStrVal()
{
	DATA_TYPES dataType = GetType();
	if (dataType != TYPE_WIDESTR)
	{
		throw CExceptMsg("DataType Error, Require TYPE_WIDESTR");
		return 0;
	}

	// 读取长度
	if (!Acquire(sizeof(int)))
	{
		throw CExceptMsg("Out Of range");
		return 0;
	}

	int nBytes = (*(int*)m_pRWPtr);
	m_pRWPtr += sizeof(int);

	if (!Acquire(nBytes))
	{
		throw CExceptMsg("Out Of range");
		return 0;
	}

	wchar_t* ret = (wchar_t*)m_pRWPtr;

	m_pRWPtr += nBytes;

	return ret;
}

bool CMsgTyped::BinaryVal(void*& pData, size_t& nLen)
{
	DATA_TYPES dataType = GetType();
	if (dataType != TYPE_BINARY)
	{
		throw CExceptMsg("DataType Error, Require TYPE_BINARY");
		return false;
	}

	// 读取长度
	if (!Acquire(sizeof(int)))
	{
		throw CExceptMsg("Out Of range");
		return false;
	}

	int nBytes = (*(int*)m_pRWPtr);
	m_pRWPtr += sizeof(int);

	if (!Acquire(nBytes))
	{
		throw CExceptMsg("Out Of range");
		return false;
	}

	if (nBytes > 0)
	{
		// 读取数据
		pData = m_pRWPtr;
		nLen = nBytes;

		m_pRWPtr += nBytes;
	}
	else
	{
		pData = NULL;
		nLen = 0;
	}

	return true;
}

void* CMsgTyped::GetPointer()
{
	DATA_TYPES dataType = GetType();
	if (dataType != TYPE_POINTER)
	{
		throw CExceptMsg("DataType Error, Require TYPE_POINTER");
		return NULL;
	}

	if (!Acquire(sizeof(void*)))
	{
		throw CExceptMsg("Out Of range");
		return NULL;
	}

	void* pRet = (*(void**)m_pRWPtr);

	m_pRWPtr += sizeof(void*);

	return pRet;
}

bool CMsgTyped::BoolValue()
{
	DATA_TYPES dataType = GetType();
	if (dataType != TYPE_BOOL)
	{
		throw CExceptMsg("DataType Error, Require TYPE_BOOL");
		return 0;
	}

	if (!Acquire(sizeof(int)))
	{
		throw CExceptMsg("Out Of range");
		return 0;
	}

	int iRet = (*(int*)m_pRWPtr);

	m_pRWPtr += sizeof(int);

	return (bool)(iRet == 1);
}

char CMsgTyped::ByteVal()
{
	DATA_TYPES dataType = GetType();
	if (dataType != TYPE_BYTE)
	{
	    // Lua 回调可以在float 和 int 之间转换
	    if (m_bIsLuaRet && dataType == TYPE_FLOAT)
	    {
	        m_pRWPtr --;
	        return (char)FloatVal();
	    }

		throw CExceptMsg("DataType Error, Require TYPE_BYTE");
		return 0;
	}

	if (!Acquire(sizeof(char)))
	{
		throw CExceptMsg("Out Of range");
		return 0;
	}

	char cRet = (*(char*)m_pRWPtr);

	m_pRWPtr += sizeof(char);

	return cRet;
}

short CMsgTyped::WordVal()
{
	DATA_TYPES dataType = GetType();
	if (dataType != TYPE_WORD)
	{
	    // Lua 回调可以在float 和 int 之间转换
	    if (m_bIsLuaRet && dataType == TYPE_FLOAT)
	    {
	        m_pRWPtr --;
	        return (short)FloatVal();
	    }

		throw CExceptMsg("DataType Error, Require TYPE_INT");
		return 0;
	}

	if (!Acquire(sizeof(short)))
	{
		throw CExceptMsg("Out Of range");
		return 0;
	}

	short iRet = (*(short*)m_pRWPtr);

	m_pRWPtr += sizeof(short);

	return iRet;
}

// private funcs
// ---------------------------------------------
bool CMsgTyped::SetType(DATA_TYPES value)
{
	if (!Acquire(sizeof(BYTE)))
	{
		return false;
	}

	(*(BYTE*)m_pRWPtr) = value;
	m_pRWPtr += sizeof(BYTE);

	return true;
}

DATA_TYPES CMsgTyped::TestType()
{
	if (!Acquire(sizeof(BYTE)))
	{
		return TYPE_NONE;
	}

	DATA_TYPES type = (DATA_TYPES)(*(BYTE*)m_pRWPtr);

	return type;
}

void CMsgTyped::Dump()
{
    CMsgTyped read(GetLength(),  m_pBuf, false);

    DATA_TYPES type;
    int cnt = 0;
    printf("[ ");
    while ( ( type = read.TestType() ) != TYPE_NONE)
    {
        cnt ++;

        if (cnt > 1) printf(" , ");

        if (type == TYPE_INT)
        {
                printf("%d", read.IntVal() );
        }
        else if (type == TYPE_STRING)
        {
            printf("'%s'", read.StringVal() );
        }
        else if (type == TYPE_FLOAT)
        {
            printf("%f", read.FloatVal() );
        }
        else if (type == TYPE_BYTE)
        {
            printf("%d", read.ByteVal() );
        }
        else if (type == TYPE_WORD)
        {
            printf("%d", read.WordVal() );
        }
        else if (type == TYPE_BOOL)
        {
            printf("%s", read.BoolValue() ? "true":"false");
        }
        else if (type == TYPE_WIDESTR)
        {
			printf("widestrval");
        }
        else if (type == TYPE_BINARY)
        {
			printf("binval");
        }
        else if (type == TYPE_POINTER)
        {
            printf("%p", read.GetPointer() );
        }
        else break;
    }
    printf(" ]\n");
}

DATA_TYPES CMsgTyped::GetType()
{
	if (!Acquire(sizeof(BYTE)))
	{
		return TYPE_NONE;
	}

	DATA_TYPES type = (DATA_TYPES)(*(BYTE*)m_pRWPtr);

	m_pRWPtr += sizeof(BYTE);

	return type;
}

bool CMsgTyped::Acquire(size_t nBytes)
{
	if (!m_pBuf)
	{
		return false;
	}

	if (m_bWriteBuf)
	{
		size_t nBytesUsed = GetLength();
		size_t nFree = m_nBufSize - nBytesUsed;

		if (nFree >= nBytes)
		{
			return true;
		}

		// 再分配空间 * 2
		size_t nNewSize = m_nBufSize * 2;

		if (nNewSize < nBytes + nBytesUsed)
		{
			nNewSize += nBytes;
		}

		char* pBuf = NULL;
		try
		{
			pBuf = __NEW(nNewSize);
		}
		catch (...)
		{
			throw CExceptMsg("No Memory");
		}

		if (!pBuf)
		{
			return false;
		}

		// 拷贝数据
		memcpy(pBuf, m_pBuf, nBytesUsed);

		__FREE(m_pBuf, m_nBufSize);
		m_pBuf = pBuf;
		m_pRWPtr = m_pBuf + nBytesUsed;
		m_nBufSize = nNewSize;

		return true;
	}
	else
	{
		size_t spaceRead = (size_t)(m_pRWPtr - m_pBuf);

		if (m_nBufSize - spaceRead >= nBytes)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
