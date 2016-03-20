#ifndef SHARED_CONFIG_SET_H
#define SHARED_CONFIG_SET_H

#include <map>
#include <string>

#include "reference.h"

using namespace std;

class PropertySet : public RefCountedObject
{
public:
    string get(const char* field);
    void set(const char* field, const char* value) {}
    
private:
    map<string, string> mFields;
};

class ConfigData : public RefCountedObject
{
public:
    static ConfigData* createFromIni(const char* filename);
    PropertySet* getPropertySet(const char* setName);
    void addPropertySet(const char* setName, PropertySet* propertySet) {}
    
private:
    
    map<string, PropertySet*> mRows;
};

class ConfigSet
{
public:
    bool addConfigData(const char* confName, ConfigData* configData);
    ConfigData* getConfigData(const char* confName);
    
private:
    map<string, ConfigData*> mDatas;
};

#endif