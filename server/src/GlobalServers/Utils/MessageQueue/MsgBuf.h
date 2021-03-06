//--------------------------------------------------------------------
// 文件名:		MsgBuf.h
// 内  容:
// 说  明:
// 创建日期:
// 最后修改:
// 创建人:
// 版权所有:
//--------------------------------------------------------------------

#ifndef _MSGBUF_H
#define _MSGBUF_H

#ifdef LINUX_
#else
#include <crtdbg.h>
#include <wtypes.h>
#endif
#include <stddef.h>
#include <stdlib.h>
#include "../ThreadPool/ThreadPool.h"

#ifdef LINUX_
#define NEW ::new
#else
#ifndef NDEBUG
#define NEW ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW ::new
#endif // NDEBUG
#endif  // LINUX_


#ifndef NDEBUG
void __AssertionFail(const char * strFile, int nLine);
#define _Assert(p)	((p) ? (void)0 : (void)__AssertionFail(__FILE__, __LINE__))
#else
#define _Assert(p)	(void(0))
#endif

#ifdef LINUX_
typedef unsigned char BYTE;
#endif

class CExceptMsg
{
public:
	CExceptMsg();
	CExceptMsg(const char * msg);
	~CExceptMsg();

	const char * GetMsg() const;

private:
	const char * m_pMsg;
};

enum DATA_TYPES
{
	TYPE_NONE = 0,
	TYPE_INT  = 2,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_WIDESTR,
	TYPE_OBJECT,
	TYPE_BINARY,
	TYPE_POINTER,
	TYPE_BOOL,
	TYPE_BYTE,
    TYPE_WORD,
};

class IBufAllocator
{
public:
	IBufAllocator(){};
	virtual ~IBufAllocator(){};
	virtual char* Alloc(size_t size) = 0;
	virtual void Free(char* data, size_t size) = 0;
};

// 内存分配器
class MsgBufAllocItem : public CriticalObject
{
	friend class MsgBufAllocMgr;
public:
	MsgBufAllocItem();
	~MsgBufAllocItem();
private:
	void SetParam(size_t nCapacity, size_t nBufSize);
	char* Alloc(size_t size);
	void Free(char* data, size_t size);
	void Clear();
private:
	std::queue<char*> _FreeQueue;
	size_t m_nCapacity;
	size_t m_nBufSize;
};

class MsgBufAllocMgr : public IBufAllocator
{
public:
	MsgBufAllocMgr();
	virtual ~MsgBufAllocMgr();
	char* Alloc(size_t size);
	void Free(char* data, size_t size);
	void Dump();
private:
	MsgBufAllocItem m_Alloc_128;
	MsgBufAllocItem m_Alloc_256;
	MsgBufAllocItem m_Alloc_512;
	MsgBufAllocItem m_Alloc_1024;
	MsgBufAllocItem m_Alloc_2048;
	MsgBufAllocItem m_Alloc_4096;
};

extern void SetBufAlloctor(IBufAllocator* pAlloc);

// 自适应Buffer
// -----------------------------------------------------------------------
class CMsgTyped
{
public:
	CMsgTyped();
	CMsgTyped(size_t nBufSize);
	CMsgTyped(size_t len, const void * pdata, bool bAllocMem = true);
	CMsgTyped(const CMsgTyped& another);
	void operator =(const CMsgTyped& another);
	~CMsgTyped();

public:
	// 两个CMsgTyped 对象交换内存数据
	void Swap(CMsgTyped& another);

	void SetAsReadBuf(bool bReadBuf = true);
	void SeekToBegin();

	// 获取数据指针
	const void * GetData() const;

	// 获取数据长度
	size_t GetLength() const;

	// 设置值
	bool SetInt(int value);
	bool SetString(const char * value);
	bool SetFloat(float value);
	bool SetWideStr(const wchar_t * value);
	bool SetBinary(void* pData, size_t nLen);
	bool SetPointer(void* value);
	bool SetBool(bool value);
	bool SetByte(char  value);
	bool SetWord(short value);

	// 插入值(插在0位)
	bool InsertInt(int value);
	bool InsertInt(void* val1, void* val2 = NULL,
                 void * val3 = NULL, void* val4 = NULL);

    // 覆盖
    bool SetAt(int index, int value);
    bool SetAt(int index, float value);
    bool SetAt(int index, bool value);

	// 获取值
	int IntVal();
	const char * StringVal();
	float FloatVal();
	const wchar_t * WideStrVal();
	bool BinaryVal(void*& pData, size_t& nLen);
	void* GetPointer();
	bool BoolValue();
	char ByteVal();
	short WordVal();

    // 获取数据类型, 同时不移动数据指针
    DATA_TYPES TestType();
    void Dump();

private:
	bool SetType(DATA_TYPES value);
	DATA_TYPES GetType();
	bool Acquire(size_t nBytes);

public:
	char* m_pBuf;
	size_t m_nBufSize;
	char* m_pRWPtr;
	bool m_bWriteBuf;
	bool m_bAllocMem;
	bool m_bIsLuaRet;
};

// 消息缓冲申请器, 用来申请写缓冲
class CWriteBufferAllocator : public CriticalObject
{
public:
	CWriteBufferAllocator();
	~CWriteBufferAllocator();
public:
	CMsgTyped* Alloc();
	void Reuse(CMsgTyped* pmsg);
	void Clear();
private:
	vector<CMsgTyped*> m_List;
};

#endif // _MSGBUF_H

