/****************************************************************************
 * 合并 RedisHash 和 RedisDhash 变为一个 RedisHashJson
 * 特点:
 * 1、为了扩展性，value 全是按json格式保存
 * 2、支持动作分析，不用每次操作都回写数据库
 * 3、Api更少，短小精悍
 *
 * 使用说明:
 * 必须调用的三个函数：1、读取 2、回写 3、销毁
 *
 *                  请各位同学不要修改 黄坤朝 2013-10-21
 *
 ****************************************************************************/
#pragma once
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <cassert>
#include "main.h"
#include "json/json.h"
#include "RedisResultHelper.h"

template<class KEY>
class RedisHashJson
{
public:
    //  1、读取
    //  此函数要第一个调用，如果没有调用，任何操作不会影响数据库
    //  dbname:id 构成一个关键字去读取内容到内存
    void load(const char* dbname , int id);

    //  2、回写
    //  将已修改的内存数据同步回数据库，上层应用可以定时调用回写或者立刻调用回写
    void save();
    
    //  3、销毁
    //  从数据库移除这个key，一个完整的应用基本上都会出现的
    void clear();
    
    typedef std::map<KEY,Json::Value> Map;
    typedef typename Map::iterator iterator;
    typedef typename Map::reverse_iterator reverse_iterator;
    

    int      getId()    {   return mId;             }
    
    iterator begin()    {   return mData.begin();   }
    iterator end()      {   return mData.end();     }
    
    reverse_iterator rbegin()   {   return mData.rbegin();  }
    reverse_iterator rend()     {   return mData.rend();    }
    
    int size() { return mData.size(); }
    bool empty() { return mData.empty(); }
    
    
    Json::Value& operator[](const KEY& key);
    const Json::Value& operator[](const KEY& key) const;
    
    bool exist(const KEY& key);
    void erase(const KEY& key);
    
    RedisHashJson();
    ~RedisHashJson();
    void release();  // 手工回收内存，有时用在全局单例主动回收内存，因为json库用在全局单例析构时有问题，此时要主动回收内存
    
    KEY nextIntKey();

private:
    
    void active (const char* dbname , int id);
    std::string   dbkey();
    
    enum eActionType
    {
        eRemove,    //删除
        eModify,    //修改
    };
    
private:
   
    std::string mDatabaseName;
    int mId;
    Map mData;
    
    typedef std::map<KEY,eActionType> ActionMap;
    ActionMap mActions;
};


template<class KEY>
RedisHashJson<KEY>::RedisHashJson():mId(0)
{
}

template<class KEY>
RedisHashJson<KEY>::~RedisHashJson()
{
}

template<class KEY>
void RedisHashJson<KEY>::release()
{
    mDatabaseName.clear();
    mData.clear();
    mActions.clear();
}

template<class KEY>
void RedisHashJson<KEY>::active (const char* dbname , int id)
{
    assert(dbname);
    mDatabaseName = dbname;
    mId = id;
}

template<class KEY>
std::string RedisHashJson<KEY>::dbkey()
{
    std::stringstream ss;
    ss<<mDatabaseName<<":"<<mId;
    return ss.str();
}

template<class KEY>
void RedisHashJson<KEY>::load(const char* dbname , int id)
{
    active(dbname, id);
    
    mData.clear();
    mActions.clear();
    
    RedisHGETALL result("HGETALL %s",dbkey().c_str());
    RedisHGETALL::iterator iter;
    
    
    for (iter = result.begin(); iter != result.end(); ++iter)
    {
        KEY key;
        stringstream ss;
        ss<< iter->first;
        ss>> key;
        
        Json::Value val;
        Json::Reader reader;
        
        //有些老字段不是json的
        if ( ! reader.parse(iter->second, val))
        {
            continue;
        }
        
        // 保证一定是obj类型的
        if ( val.type() != Json::objectValue )
        {
            log_error("[json err]"<<dbkey()<<" "<<iter->first<<"->"<<iter->second);
            continue;
        }
        
        mData.insert(std::make_pair(key, val));
        
    }
}

/*
 注：
 因为是使用的是redis hash ,所以每个操作独立的，忽略先后顺序
 所以动作分析器，可以用一个map来实现即可。
 
 而一般的通作动作分析，两个动作是有关联的，有先后顺序的，就要使用一个队列来存所有动作，
 然后反序从尾到头来遍历动作，同样操作只取第一个动作，这样就可以简化动作分析。
 */

template<class KEY>
void RedisHashJson<KEY>::save()
{
    if( mDatabaseName.empty() || mActions.empty() )
        return;
    
    if( empty() )
    {
        clear();//回收key
        return;
    }
    
    std::set<KEY> modAction;
    std::set<KEY> remAction;
    
    for(typename ActionMap::iterator it = mActions.begin() ; it != mActions.end(); ++it)
    {
        if( eModify ==  it->second )
        {
            modAction.insert( it->first);
            continue;
        }
        
        if( eRemove == it->second)
        {
            remAction.insert( it->first);
            continue;
        }
    }
    
    mActions.clear();
    
    //save modify action
    {
        RedisArgs args;
        RedisHelper::beginHmset(args,dbkey().c_str());
        for(typename std::set<KEY>::iterator iter = modAction.begin(); iter != modAction.end(); ++  iter)
        {
            if(mData.find(*iter) == mData.end())
                continue;
            
            stringstream sskey;
            sskey << *iter;
            
            RedisHelper::appendHmset(args,
                                     sskey.str().c_str(),
                                     Json::FastWriter().write(  mData[*iter] ).c_str()
                                     );
        }
        if(args.size() > 2)
            RedisHelper::commitHmset(get_DbContext(), args);
    }
    
    //save del action
    {
        RedisArgs delArgs;
        RedisHelper::beginHdel(delArgs,dbkey().c_str());
        for (typename std::set<KEY>::iterator iter = remAction.begin(); iter != remAction.end(); ++ iter)
        {
            stringstream sskey;
            sskey << *iter;
            
            RedisHelper::appendHdel(delArgs, sskey.str().c_str());
        }
        if(delArgs.size() > 2)
            RedisHelper::commitHdel(get_DbContext(), delArgs);
    }
}

template<class KEY>
void RedisHashJson<KEY>::clear()
{
    if( mDatabaseName.empty() )
        return;
    
    doRedisCmd("DEL %s",dbkey().c_str());
    mData.clear();
    mActions.clear();
}


template<class KEY>
bool RedisHashJson<KEY>::exist(const KEY& key)
{
    return mData.count(key) == 1;
}


template<class KEY>
void RedisHashJson<KEY>::erase(const KEY& key)
{
    if( mDatabaseName.empty() )
        return;
    
    if(mData.erase(key) == 0)
        return;
    
    mActions[key] = eRemove;
}

template<class KEY>
Json::Value& RedisHashJson<KEY>::operator[](const KEY& key)
{
    static Json::Value tmp;
    if( mDatabaseName.empty() )
        return tmp;
    
    mActions[key] = eModify;
    
    return mData[key];
}


template<class KEY>
const Json::Value& RedisHashJson<KEY>::operator[](const KEY& key) const
{
    static Json::Value tmp;
    if( mDatabaseName.empty() )
        return tmp;
    
    typename Map::const_iterator iter = mData.find(key);
    if(iter == mData.end() )
        return tmp;
    
    return iter->second;
}

template<class KEY>
KEY RedisHashJson<KEY>::nextIntKey()
{
    if(mData.empty() )
        return 1;
    
    KEY  key = mData.rbegin()->first ;
    
    return key + 1;
}
    

typedef RedisHashJson<int> IntMapJson;

