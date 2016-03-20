//
//  mof.cpp
//  GameSrv
//
//  Created by prcv on 13-6-15.
//
//

#include "process.h"

#include <unistd.h>

string Process::execpath;
string Process::respath;
Properties Process::env;

std::string Properties::getProperty(const std::string &key, const string& def)
{
    HashIterType iter = mProperties.find(key);
    if (iter == mProperties.end())
    {
        return def;
    }
    
    string property = def;
    switch (iter->second.type()) {
        case Json::stringValue:
            property = iter->second.asString();
            break;
        case Json::intValue:
            property = Utils::itoa(iter->second.asInt());
            break;
            
        default:
            break;
    }
    return property;
}

string Properties::getString(const KeyType& key, const string& def){
    HashIterType iter = mProperties.find(key);
    if (iter != mProperties.end() && iter->second.isString()){
        return iter->second.asString();
    }
    
    return def;
}

int Properties::getInt(const std::string &key, int def){
    HashIterType iter = mProperties.find(key);
    if (iter != mProperties.end() && iter->second.isInt()){
        return iter->second.asInt();
    }
    return def;
}

bool Properties::setInt(const KeyType& key, int val){
    HashIterType iter = mProperties.find(key);
    if (iter != mProperties.end()){
        
        if (!iter->second.isInt()){
            return false;
        }
        
        iter->second = ValueType(val);
    }
    else{
        mProperties[key] = ValueType(val);
    }
    
    return true;
}

bool Properties::setString(const KeyType& key, const string& val){
    
    HashIterType iter = mProperties.find(key);
    if (iter != mProperties.end()){
        
        if (!iter->second.isString()){
            return false;
        }
        
        iter->second = ValueType(val);
    }
    else{
        mProperties[key] = ValueType(val);
    }
    
    return true;
}

bool Properties::setProperty(const KeyType& key, const std::string &val)
{
    return setString(key, val);
}

bool Properties::setProperty(const KeyType& key, int val)
{
    return setInt(key, val);
}

int Properties::getPropertyType(const KeyType &key)
{
    HashIterType iter = mProperties.find(key);
    if (iter == mProperties.end()){
        return eNullValue;
    }
    
    int type = eNullValue;
    switch (iter->second.type()) {
        case Json::stringValue:
            type = eStringValue;
            break;
        case Json::intValue:
            type = eIntValue;
            break;
            
        default:
            break;
    }
    
    return type;
}