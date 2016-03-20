/******************************************************************************
  RedisHash.h
  GameSrv

  Created by Huang Kunchao on 13-8-19.

  功能： RedisHash类构造宏
 
  使用： 用于哈希集，其域是多个固定的关键字，例如 role 的 多个属性值
        role:9527 ->    name -> snake
                        job  -> 1
                        lvl  -> 60
        其中name , job , lvl ... 这些属性关键字是固定的,
        该类可以自动读写 redis hash 并且同步类成员
 
  函数说明：
        void load(int id);
        从数据库加载关键字为 xxx:id 的哈希集，并且自动填充类成员
        
        void save();
        保存被修改过的类成员立刻写进数据库
 
        const char* dbkey();
        返回数库库关键字 xxx:id
 
        int getId();
        返回唯一标记 id
 
        type& getXXX();
        void setXXX(type& val);
        读写类成员,其中type 预实现了 RedisHashInt,RedisHashFloat, RedisHashString
        
        也可以使用自定义类型,继承RedisHashObject，然后把头文件包含到此文件的头部
        编写自定义类型的修改值函数时要注意记得把函数 dirty(true);加上，否则不回写数据库
 
        void active( int id)
        激活读写数据库，具体请看成员说明
 
        void deleteFromDatabase()
        删了数据库数据
 
  成员说明：
        int mId;
        保存的唯一标记，同时作为数据库关键字后缀，例如可应用于以下场景：roleid , guildid , petid ...
 
        string mDbname;
        保存数据库关键字前缀，用于拼数据库关键字，具体取值来自于 RedisKeyTable
 
        bool mActive;
        是否激活读写数据库，末激活是不能读写数据库的。该功能用于dummy对象，防止dummy误写数据库
        可以通过active函数（load 内置了此函数）来激活对数据库读写
 
        type YYY;
        类成员
 
*******************************************************************************/
#pragma once
#include <string>
#include "RedisHashBaseObj.h"
#include "RedisHashObj.h"
#include "RedisKeyName.h"

//包含自定义类型头文件

using namespace std;


#define begin_redis_hash_class(name,dbname)                         \
class name{                                                        \
public:                                                             \
    name();                                                         \
    virtual ~name(){}                                               \
    void load(int id);                                        \
    void save();                                              \
    string dbkey();                                           \
    string joinAllKeyString();                                \
protected:                                                          \
    int    mId;                                                \
    string mDbname;                                                \
    bool    mActive;                                           \
public:                                                             \
    int  getId()             {   return mId;    }      \
    void active( int id)                                   \
    {                                                               \
        mId = id;                                              \
        mActive = true;                                        \
    }                                                           \
    void deleteFromDatabase();                                   

#define redis_hash_int(name,dbname)                                 \
private:                                                          \
    RedisHashInt    name;                                           \
public:                                                             \
    int  get##name()            {   return name.get();  }         \
    void set##name(int val)     {   name.set(val);      }

#define redis_hash_float(name,dbname)                               \
private:                                                          \
    RedisHashFloat  name;                                           \
public:                                                             \
    float get##name()           {   return name.get();  }         \
    void  set##name(float val)  {   name.set(val);      }

#define redis_hash_double(name,dbname)                               \
private:                                                          \
    RedisHashDouble  name;                                           \
public:                                                             \
    double get##name()           {   return name.get();  }         \
    void  set##name(double val)  {   name.set(val);      }

#define redis_hash_string(name,dbname)                              \
private:                                                          \
    RedisHashString  name;                                          \
public:                                                             \
    string& get##name()         {   return name.get();  }       \
    void  set##name(const char* val) {   name.set(val);      }  \
    void  set##name(const string& val) {   name.set(val);      }

#define redis_hash_obj(type,name,dbname)                            \
public:                                                             \
    type        name;                                               \
    type&       get##name()     {   return name;        }


#define end_redis_hash_class()                                    \
};

#include "RedisHashDefine.h"

#undef begin_redis_hash_class
#undef end_redis_hash_class

#undef redis_hash_int
#undef redis_hash_float
#undef redis_hash_double
#undef redis_hash_string
#undef redis_hash_obj

