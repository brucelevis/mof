//  Created by Huang Kunchao on 13-8-21.
//
//  Redish Hash 基础类型

#pragma once
#include <string>
#include <sstream>
#include <cstdlib>

using namespace std;

template<class T>
class RedisHashObjectBase
{
public:
    RedisHashObjectBase(){ mDirty = false;}
    virtual ~RedisHashObjectBase(){}
    virtual void load(const char* str) = 0;
    virtual string str() = 0;
    void setName(const char* str)
    {
        mName = str;
    }
    const char* getName() const
    {
        return mName.c_str();
    }
    T& get() { return mValue; }
    void set(const T& v)
    {
        dirty(true);
        mValue = v;
    }
    
    bool dirty() const  {   return mDirty; }
    void dirty(bool val) { mDirty = val;}
    
protected:
    bool    mDirty;
    string  mName;
    T       mValue;
};

class RedisHashInt : public RedisHashObjectBase<int>
{
public:
    RedisHashInt()
    {
        mDirty = false;
        mValue = 0;
    }
    virtual void load(const char* str)
    {
        mValue = atoi(str);
    }
    virtual string str()
    {
        stringstream ss;
        ss<<mValue;
        return ss.str();
    }
};

class RedisHashFloat : public RedisHashObjectBase<float>
{
public:
    RedisHashFloat()
    {
        mDirty = false;
        mValue = 0.0f;
    }
    virtual void load(const char* str)
    {
        mValue = atof(str);
    }
    virtual string str()
    {
        stringstream ss;
        ss<<mValue;
        return ss.str();
    }
};

class RedisHashDouble : public RedisHashObjectBase<double>
{
public:
    RedisHashDouble()
    {
        mDirty = false;
        mValue = 0;
    }
    virtual void load(const char* str)
    {
        stringstream ss;
        ss<< str;
        ss>> mValue;
    }
    virtual string str()
    {
        stringstream ss;
        ss<<mValue;
        return ss.str();
    }
};

class RedisHashString : public RedisHashObjectBase<string>
{
public:
    RedisHashString()
    {
        mDirty = false;
    }
    virtual void load(const char* str)
    {
        mValue = str;
    }
    virtual string str()
    {
        return mValue;
    }
};



