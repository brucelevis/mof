

/******************************************************************************
 * 这个后续版本建议不要用了，用RedisHashJson  2013-10-21
 ******************************************************************************/


/******************************************************************************
 
   RedisDhash.h
   GameSrv
 
   Created by Huang Kunchao on 13-8-23.
 
   Redis Dynamic Hash Class Maker Macro
 
 功能： 动态哈希类构造宏
     
     使用： 用于动态哈希集，其域是不固定的关键字，例如公会被申请列表
     guildapplyguild:123 ->  9527 -> 男，坤朝，60级，战士，很帅
                             1024 -> 男，坚哥，58级，法师，V5
                             2048 -> 女，开军，57级，剑士，根号4
     
     其中被申请列表的信息是不固定的,随时都可能增加新申请者，删掉申请者，或者修改申请者
     该类可以自动读写 redis hash 并且同步哈希集
     
 函数说明：
     void load(int id);
     从数据库加载关键字为 xxx:id 的哈希集，并且自动填充哈希集
     
     void set(const TK& key , const TV& val)
     设置并同步数据库
     
     void remove(const TK& key)
     删除并同步数据库
     
     const char* dbkey()
     返回数库库关键字 xxx:id
 
     int getId();
     返回唯一标记 id
 
    void deleteFromDatabase()
    删了数据库数据
 
    void clear()
    删了内存数据，同时删了数据库数据
 
     一般情况下key为int ,val 可能是 int ,float ,string ，
     更多情况下是自定义类型,如果使用自定义类型,
     自定义类要实现以下两个函数
        std::ostream& operator<<(std::ostream& out, const ClassType& obj)
        std::istream& operator>>(std::istream& in,ClassType& obj)
 
     void active( int id)
     激活读写数据库，具体请看成员说明
     
 成员说明：
     int mId ;
     保存的唯一标记，同时作为数据库关键字后缀
     
     string mDbname;
     保存数据库关键字前缀，用于拼数据库关键字，具体取值来自于 RedisKeyTable
     
     bool mActive;
     是否激活读写数据库，末激活是不能读写数据库的。该功能用于dummy对象，防止dummy误写数据库
     可以通过active函数（load内置了此函数）来激活对数据库读写
     
*******************************************************************************/
#pragma once
#include <map>
#include <string>
#include <sstream>
#include "RedisResultHelper.h"
#include "RedisKeyName.h"
#include "main.h"
#include "json/json.h"
#include <list>
#include <set>
#include <iostream>
using namespace std;



template<class TK,class TV,int dbNameIndex>
class RedisDhash
{
public:
    typedef std::map<TK,TV> Map;
    typedef typename Map::iterator iterator;
    typedef typename Map::reverse_iterator reverse_iterator;
    
    RedisDhash(){
        mId = 0;
        mActive = false;
        mDbname = g_GetRedisName(dbNameIndex);
    }
    virtual ~RedisDhash(){}
    
    int      getId()    {   return mId;             }
    iterator begin()    {   return mData.begin();   }
    iterator end()      {   return mData.end();     }
    reverse_iterator rbegin()   {   return mData.rbegin();  }
    reverse_iterator rend()     {   return mData.rend();    }
    int      size()     {   return mData.size();    }
    iterator find(const TK& key) {  return mData.find(key); }
    bool     exist(const TK& key) {   return mData.count(key) == 1;}
    
    void     active (int id)
    {
        mActive = true;
        mId = id;
    }
    
    string   dbkey()
    {                                                               
        stringstream ss;                                            
        ss<<mDbname<<":"<<mId;                                      
        return ss.str();                                            
    }
    
    void deleteFromDatabase()
    {
        if(mActive)                                                         
            doRedisCmd("DEL %s",dbkey().c_str());                           
    }
    
    void load(int id)
    {
        active(id);
        mData.clear();
        RedisHGETALL result("HGETALL %s",dbkey().c_str());
        RedisHGETALL::iterator iter;
        
        stringstream ss;
        
        for (iter = result.begin(); iter != result.end(); ++iter)
        {
            TK key; TV val;
            ss.clear();
            ss.str("");
            ss<<iter->first;
            ss>>key;
            
            ss.clear();
            ss.str("");
            ss<<iter->second;
            ss>>val;
            
            mData.insert(std::make_pair(key,val));
        }
    }
    
    void set(const TK& key , const TV& val)
    {
        if( ! mActive )
            return;
        mData[key] = val;

        stringstream keyss , valss;
        keyss << key;
        valss << val;
        
        doRedisCmd("HMSET %s %s %s", dbkey().c_str(), keyss.str().c_str(), valss.str().c_str());
    }
    
    void remove(const TK& key)
    {
        if( ! mActive )
            return;
        
        if(mData.erase(key) == 0)
            return;
        
        stringstream keyss;
        keyss<<key;
        
        doRedisCmd("HDEL %s %s", dbkey().c_str(), keyss.str().c_str());
    }
    
    
    void clear()
    {
        if( ! mActive )
            return;
        
        deleteFromDatabase();
        mData.clear();
    }
    
    const TV& operator[](const TK& key) const
    {
        return mData[key];
    }

protected:
    Map     mData;
    string  mDbname;
    int     mId;
    bool    mActive;
};




