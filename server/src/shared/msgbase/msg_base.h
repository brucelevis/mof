#ifndef SHARED_MSG_BASE_H
#define SHARED_MSG_BASE_H

#include "msg_variant.h"
#include "ByteArray.h"
#include <stdio.h>

class MsgObject
{
public:
    MsgObject()
    {
        
    }
    
    virtual ~MsgObject()
    {
        
    }
    
    string toString();
    string variantToString(const MsgVariant& variant);
    
    void encode(ByteArray& byteArray);
    void encodeVariant(MsgVariant& variant, ByteArray& byteArray);
    
    void decode(ByteArray& byteArray);
    void decodeVariant(MsgVariant& variant, ByteArray& byteArray);
    
    virtual MsgObject* clone() const = 0;
    virtual MsgObject* clone(void* addr) const = 0;
    
    void addMember(const MsgVariant& variant)
    {
        mArray.push_back(variant);
    }
    vector<MsgVariant> mArray;
};

class MsgBase : public MsgObject
{
public:
    int mType;
    int mId;
    
    virtual MsgObject* clone() const {return NULL;}
    virtual MsgObject* clone(void* addr) const {return NULL;}
    
    void encode(ByteArray& byteArray)
    {
        byteArray.write_int(mType);
        byteArray.write_int(mId);
        MsgObject::encode(byteArray);
    }
    
    void encodeVariant(MsgVariant& variant, ByteArray& byteArray)
    {
        
    }
    
    void decode(ByteArray& byteArray)
    {
        
    }
    
    void decodeVariant(MsgVariant& variant, ByteArray& byteArray)
    {
        
    }
};

#endif