/*==============================================================================
 Copyright (c) 2009 D.W All rights reserved.
 
 filename: 	BaseType.h
 created:	D.W 2009-9-23 19:00
 purpose:	网络协议类型！（网络协议代码由自动协议生成工具生成，代码禁止手动修改）
 ==============================================================================*/
#pragma once

#include "ByteArray.h"

class INetPacket
{
protected:
	int _type;
    int _id;
    int mSession;
public:
	int getSession() {return mSession;}
	void setSession(int session) {mSession = session;}

    int getType(){return _type;}
    int getID(){return _id;}
    virtual ~INetPacket(){};
    virtual const char* PacketName() = 0;
	virtual void encode(ByteArray &byteArray)
	{
		byteArray.write_int(mSession);
		byteArray.write_int(_type);
        byteArray.write_int(_id);
	}
    
	virtual void decode(ByteArray &byteArray)
	{
		mSession = byteArray.read_int();
		_type = byteArray.read_int();
        _id = byteArray.read_int();
	}
    
	virtual void build(ByteArray &byteArray)
	{
        INetPacket::encode(byteArray);
	}
};