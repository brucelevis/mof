/*==============================================================================
 Copyright (c) 2009 D.W All rights reserved.

 filename: 	BaseType.cpp
 created:	D.W 2009-9-23 19:00
 purpose:	字节流类 （网络协议序列化）
 ==============================================================================*/
#include "ByteArray.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ByteArray::ByteArray(char* content, int size)
{
    if (content == NULL){
        m_pOwnContent = m_pContent = new char[MAXMSGSIZE];
        m_nTotalSize = int(MAXMSGSIZE);
    }
    else{
        m_pOwnContent = NULL;
        m_pContent = content;
        m_nTotalSize = size;
    }
	m_nRdptr = 0;
	m_nWrPtr = 0;
}

ByteArray::~ByteArray()
{
    if (m_pOwnContent){
        delete[] m_pOwnContent;
    }
}

void ByteArray::print()
{
    if (m_nWrPtr == 0) {
        //read
        for (int i=0; i<m_nTotalSize; ++i) {
            printf("%02d ",*(uint8_t*)(m_pContent+i));
            if(i%10==9){printf("\n");}
        }
    }
    else
    {
        for (int i=0; i<m_nWrPtr; ++i) {
            printf("%02d ",*(uint8_t*)(m_pContent+i));
            if(i%10==9){printf("\n");}
        }
    }
    printf("\n");
}

char* ByteArray::base() const
{
	return m_pContent;
}

char* ByteArray::end(void) const
{
	return m_pContent + m_nTotalSize;
}

char* ByteArray::rd_ptr(void) const
{
	return m_pContent + m_nRdptr;
}

void ByteArray::rd_ptr(int n)
{
	m_nRdptr +=n;
}

char* ByteArray::wr_ptr(void) const
{
	return base() + m_nWrPtr;
}

void ByteArray::wr_ptr(int n)
{

	m_nWrPtr += n;
}

int ByteArray::space()
{
	return m_nTotalSize - m_nWrPtr;
}

int ByteArray::length() const
{
	return m_nWrPtr - m_nRdptr;
}

int   ByteArray::copy(const char *buf, int n)
{
	int len = this->space();

	if(len < n)
		return -1;

	memcpy(this->wr_ptr(), buf, n);
	this->wr_ptr(n);
	return 0;

}

int ByteArray::read_int()
{
    if (m_nRdptr + 4 > m_nTotalSize) throw "read out";
    
	int w = 0;
	Read(&w);

	return w;
}


int64_t ByteArray::read_int64()
{
    if (m_nRdptr + 8 > m_nTotalSize) throw "read out";
    
	int64_t w = 0;
	Read(&w);
    
	return w;
}

char ByteArray::read_char()
{
    if (m_nRdptr + 1 > m_nTotalSize) throw "read out";
	char w = 0;
	Read(&w);

	return w;
}

float ByteArray::read_float()
{
    if (m_nRdptr + 4 > m_nTotalSize) throw "read out";
	float w = 0;
    int i = read_int();
    memcpy(&w, &i, sizeof(float));
    //w = *((float*)&i);
    //rd_ptr(sizeof(float));
	return w;
}

void ByteArray::write_char(char value)
{
    if (m_nWrPtr + 1 > m_nTotalSize) throw "write out";
	Write(value);
}

void ByteArray::write_int(int value)
{
    if (m_nWrPtr + 4 > m_nTotalSize) throw "write out";
	Write(value);
}

void ByteArray::write_int64(int64_t value)
{
    if (m_nWrPtr + 8 > m_nTotalSize) throw "write out";
	Write(value);
}

void ByteArray::write_float(float value)
{
    if (m_nWrPtr + 4 > m_nTotalSize) throw "write out";
    //value = 0.0f;
    int a;
    memcpy(&a, &value, sizeof(float));
	//int *a = (int*)&value;
    write_int(a);
}

void  ByteArray::write_blob(vector<char>& data)
{
    if (m_nWrPtr + 4 + data.size() > m_nTotalSize) throw "write out";
    
    write_int(data.size());
    
    copy(data.data(), data.size() * sizeof(char));
}

vector<char>  ByteArray::read_blob()
{
    vector<char> result;
    int len = read_int();
    if (m_nRdptr + len < m_nRdptr || m_nRdptr + len > m_nTotalSize) throw "read string err";
    
    result.insert(result.begin(), rd_ptr(), rd_ptr() + len);
    
    rd_ptr(len);
    
    return result;
}

void  ByteArray::write_string(const char* str)
{
	write_string(std::string(str));
}

void  ByteArray::write_string(const std::string& str)
{
    if (m_nWrPtr + 4 + str.length() > m_nTotalSize) throw "write out";
    
    write_int(str.length());
	copy(str.data(), str.length());
}

char* ByteArray::read_string(char* info, int len)
{
    if (m_nRdptr + len < m_nRdptr || m_nRdptr + len > m_nTotalSize) throw "read string err";
    
	char* result = (char*)memcpy(info, rd_ptr(), len);
	rd_ptr(len);

	return result;
}

std::string ByteArray::read_string()
{
	int len = read_int();
    if (m_nRdptr + len < m_nRdptr || m_nRdptr + len > m_nTotalSize) throw "read string err";
	std::string strValue(rd_ptr(), len);
	rd_ptr(len);

	return strValue;
}

void  ByteArray::write_wstring(const std::wstring& str)
{
    write_int(str.length()*2);
    // for wchar_t ==4
//    int slen = str.length();
//    for (int i = 0; i < slen; ++i) {
//        copy((const char*)(&(str[i])), 2);
//        write_char(str[i]>>8);
//        write_char(str[i]&0xff);
//    }
    // for wchar_t == 2
	copy((const char*)str.data(), str.length()*2);
}

std::wstring ByteArray::read_wstring()
{
    int len = read_int();
	//std::wstring strValue(rd_ptr(), len);
    std::wstring strValue;
    strValue.resize(len/2);
    memcpy((char*)strValue.data(),rd_ptr(),len);
//    for (int i = 0; i< len/2; ++i) {
//        strValue[i] = (wchar_t)((char)(rd_ptr()[1+i*2]<<8|((char)rd_ptr()[0+i*2])));
//
//    }
	rd_ptr(len);
	return strValue;
}

