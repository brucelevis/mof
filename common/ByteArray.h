/*==============================================================================
 Copyright (c) 2009 D.W All rights reserved.

 filename: 	BaseType.h
 created:	D.W 2009-9-23 19:00
 purpose:	字节流类 （网络协议序列化）
 ==============================================================================*/
#pragma once

#include <vector>
#include <string>
#include <stdint.h>
#include "BaseType.h"
using namespace std;

const int MAXMSGSIZE    = 32*1024;                    // 32K  单个网络消息最大长度，（超过极易导致物理服务器收发队列阻塞）

class ByteArray
{
public:
	ByteArray(char* content = NULL, int size = 0);
	virtual ~ByteArray();

	void print();
	void reuse(){ m_nRdptr = 0; m_nWrPtr = 0; m_nTotalSize = int(MAXMSGSIZE); }
public:

    char           read_char();
	int            read_int();
	int64_t            read_int64();
	float          read_float();
	char*          read_string(char* info, int len);
    vector<char>   read_blob();
	std::string    read_string();
    std::wstring   read_wstring();

	void 		   write_char(char);
	void 		   write_int(int);
	void 		   write_int64(int64_t);
	void 		   write_float(float);
    void           write_blob(vector<char>& data);
	void 		   write_string(const char* str);
	void 		   write_string(const std::string& str);
	void 		   write_wstring(const std::wstring& str);

public:
	int 		   length() const;

private:
	char*          base() const;     //字节流内部实现
	char*          end(void) const;
	char*          rd_ptr(void) const;
	char*          wr_ptr(void) const;
	void           rd_ptr(int n);
	void           wr_ptr(int n);
	int            space();
	int            copy(const char *buf, int n);

private:
	template<typename T> friend ByteArray& operator>>(ByteArray& msg, T& v)//运算符重载
	{
		msg.Read(&v);
		return msg;
	}

	friend ByteArray& operator>>(ByteArray& msg, std::string& str)
	{
		str = msg.read_string();
		return msg;
	}

	template<typename T> friend ByteArray& operator<<(ByteArray& msg, const T& v)
	{
		msg.Write(v);
		return msg;
	}

	friend ByteArray& operator<<(ByteArray& msg, const std::string& str)
	{
		msg.write_string(str);
		return msg;
	}

	template<typename T>
	T& Read(T* pVar,bool readonly=false)
	{
		*pVar = *((T*)rd_ptr());
		if(!readonly)//添加只读，不移指针支持，为了调试，Mike
			rd_ptr(sizeof(T));
		return *pVar;
	}

	template<typename T>
	void Write(const T& value)
    {
		T* pV = (T*)wr_ptr();
    	*pV = value;
		wr_ptr(sizeof(T));
	}

public:
    char* m_pOwnContent;
	char* m_pContent;                               //字节流内容指针
	int   m_nTotalSize;                             //字节流允许大小
	int   m_nRdptr;                                 //读取位置
	int   m_nWrPtr;                                 //写入位置
};
