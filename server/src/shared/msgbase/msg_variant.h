//
//  variant.h
//  GameSrv
//
//  Created by 丁志坚 on 12/16/14.
//
//

#ifndef __GameSrv__msg_variant__
#define __GameSrv__msg_variant__

#include <stdio.h>
#include <vector>
#include <string>
#include "msg_object_array.h"
#include "Utils.h"
#include "ByteArray.h"
using namespace std;

enum MsgVariantType
{
    kMsgVariantNull,
    kMsgVariantInt,
    kMsgVariantIntArray,
    kMsgVariantInt64,
    kMsgVariantInt64Array,
    kMsgVariantFloat,
    kMsgVariantFloatArray,
    kMsgVariantString,
    kMsgVariantStringArray,
    kMsgVariantObject,
    kMsgVariantObjectArray,
};

class MsgObject;

class MsgVariant
{
public:
    
    
    MsgVariant()
    {
        mType = kMsgVariantNull;
    }
    
    MsgVariant(int* val)
    {
        mType = kMsgVariantInt;
        mValue = val;
    }
    
    MsgVariant(vector<int>* val)
    {
        mType = kMsgVariantIntArray;
        mValue = val;
    }
    
    MsgVariant(int64_t* val)
    {
        mType = kMsgVariantInt64;
        mValue = val;
    }
    
    MsgVariant(vector<int64_t>* val)
    {
        mType = kMsgVariantInt64Array;
        mValue = val;
    }
    
    MsgVariant(string* val)
    {
        mType = kMsgVariantString;
        mValue = val;
    }
    
    MsgVariant(vector<string>* val)
    {
        mType = kMsgVariantStringArray;
        mValue = val;
    }
    
    MsgVariant(float* val)
    {
        mType = kMsgVariantFloat;
        mValue = val;
    }
    
    MsgVariant(vector<float>* val)
    {
        mType = kMsgVariantFloatArray;
        mValue = val;
    }
    
    MsgVariant(MsgObject* val)
    {
        mType = kMsgVariantObject;
        mValue = val;
    }
    
    MsgVariant(void* val, MsgObject* prototype)
    {
        mType = kMsgVariantObjectArray;
        mValue = val;
        mPrototype = prototype;
    }
    
    ~MsgVariant()
    {
    }
    
    //void append(const MsgVariant& variant);
    //string toString();
    
    //void encode(ByteArray& byteArray);
    //bool decode(ByteArray& byteArray);
    
    int mType;
    void* mValue;
    MsgObject* mPrototype;
};

#endif /* defined(__GameSrv__variant__) */
