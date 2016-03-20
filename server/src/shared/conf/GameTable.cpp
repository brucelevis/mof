//
//  GameTable.cpp
//  GameSrv
//
//  Created by pireszhi on 15-5-12.
//
//

#include <stdio.h>
#include "GameTable.h"
#include "DataCfg.h"
#include "os.h"

void ConfigTable::setData(const MapType& inData)
{
    mData.clear();
    std::copy(inData.begin(), inData.end(), std::inserter(mData, mData.begin()));
}

/// Returns true, if section exists.
bool ConfigTable::exists(const std::string& section) const
{
    return mData.find(section) != mData.end();
}

/// Returns associated value from section-key-pair or default-value.
std::string ConfigTable::getValue(const std::string& section, const std::string& token, const std::string& def) const
{
    // find section
    MapType::const_iterator si = mData.find(section);
    
    if (si != mData.end())
    {
        // find token
        
        MapType::mapped_type::const_iterator token_it = si->second.find(token);
        
        if (token_it != si->second.end())
            return token_it->second;
    }
    
    return def;
}

bool ConfigTable::exists(const std::string& section, const std::string& token) const
{
    MapType::const_iterator si = mData.find(section);
    return    si != mData.end() && si->second.find(token) != si->second.end();
}

std::vector<std::string> ConfigTable::getTable(const std::string& section,
                                  const std::string& token) const
{
    std::vector<std::string> _tokens;
    std::string str = getValue(section,token,"");
    if(str.empty())
        return  _tokens;
    
    const char* src = str.c_str();
    const char* sepstr = ";";
    int seplen = strlen(sepstr);
    
    while (*src){
        const char* pos = strstr(src, sepstr);
        if (pos == NULL){
            _tokens.push_back(src);
            break;
        }
        _tokens.push_back(std::string(src, pos - src));
        src = pos + seplen;
    }
    return _tokens;
}


ConfigTableMgr g_ConfigTableMgr;
ConfigInterpreter g_ConfigInterpreter;

void ConfigTableMgr::clear()
{
    map<string, ConfigTable*>::iterator iter = mTables.begin();
    map<string, ConfigTable*>::iterator endIter = mTables.end();
    
    for(; iter != endIter; iter++)
    {
        if (NULL != iter->second) {
            ConfigTable* tmp = iter->second;
            delete tmp;
            
            iter->second = NULL;
        }
    }
    
    mTables.clear();
}

void ConfigTableMgr::removeTable(const string& tableName)
{
    map<string, ConfigTable*>::iterator iter = mTables.find(tableName);
    
    if (iter != mTables.end()) {
        ConfigTable* tmp = iter->second;
        
        mTables.erase(iter);
        
        delete tmp;
    }
}

ConfigTable* ConfigTableMgr::getTable(const string& tableName)
{
    map<string, ConfigTable*>::iterator iter = mTables.find(tableName);
    if (iter != mTables.end()) {
        return iter->second;
    }
    printf("can not find %s \n", tableName.c_str());
    return NULL;
}

string ConfigTableMgr::getValue(const string& tableName, string& section, string& key, const std::string& def)
{
    ConfigTable* table = getTable(tableName);
    
    if (NULL == table) {
        return def;
    }
    
    return table->getValue(section, key, def);
}

bool ConfigTableMgr::addTable(string tableName, const MapType& tableData)
{
    map<string, ConfigTable*>::iterator iter = mTables.find(tableName);
    
    if (iter == mTables.end()) {
        
        ConfigTable* table = new ConfigTable();
        
        if (NULL == table) {
            return false;
        }
        
        table->setData(tableData);
        
        mTables.insert(make_pair(tableName, table));
        
        return true;
    
    }
    
    ConfigTable* table = iter->second;
    
    table->setData(tableData);
    
    return true;
}

void ConfigTableMgr::loadDir(const char *path)
{
    vector<string> files;
    fs::listFiles(path, files);
    
    int fileCount = files.size();
    for (int i = 0; i < fileCount; i++) {
        
        size_t pos = files[i].rfind(".ini");
        if (pos == string::npos || pos != files[i].length() - 4)
        {
            continue;
        }
        
        loadCfg(files[i].c_str());
    }
}

void ConfigTableMgr::loadCfg(const string& filename)
{
    IniFile inifile(filename.c_str());
    
    string fileBaseName = fs::baseName(filename.c_str());
    addTable(fileBaseName, inifile.getData());
}

void ConfigTableMgr::addSearchPath(const string& path)
{
    if (!path.empty())
    {
        mSearchPath.push_back(path);
    }
}

void ConfigTableMgr::loadAllTable()
{
    int pathCount = mSearchPath.size();
    
    for (int i = 0; i < pathCount; i++) {
        loadDir(mSearchPath[i].c_str());
    }
}