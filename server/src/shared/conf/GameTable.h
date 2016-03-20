//
//  GameTable.h
//  GameSrv
//
//  Created by pireszhi on 15-5-12.
//
//
#ifndef _GAMETABLE_H_
#define _GAMETABLE_H_
#include "std_includes.h"

using namespace std;

typedef void(*LOADTABLEFUNC)(string configName);

typedef std::map<std::string, std::map<std::string, std::string> > MapType;

class ConfigTable
{
public:
    void setData(const MapType& inData);
    
    /// Returns true, if section exists.
    bool exists(const std::string& section) const;
    
    /// Returns associated value from section-key-pair or default-value.
    std::string getValue(const std::string& section,
                         const std::string& token, const std::string& def = std::string()) const;
    
    bool exists(const std::string& section, const std::string& token) const;
    
    
    template <typename OutputIterator>
    void getSections(OutputIterator oi);
    
    template <typename OutputIterator>
    void getKeys(const std::string& section, OutputIterator oi);
    
    /// Get the value and convert it with istream-operator.
    /// The return-type is identified by the default-value-type.
    template <typename T>
    T getValueT(const std::string& section,
                const std::string& token,
                const T& def) const;
    
    std::vector<std::string> getTable(const std::string& section,
                                      const std::string& token) const;
    
    template <typename T>
    std::vector<T> getTableT(const std::string& section,
                             const std::string& token ,const T& def) const;
private:
    MapType mData;
};

template <typename OutputIterator>
void ConfigTable::getKeys(const std::string& section, OutputIterator oi)
{
    MapType::const_iterator si = mData.find(section);
    if (si != mData.end())
    {
        for (MapType::mapped_type::const_iterator it = si->second.begin();
             it != si->second.end(); ++it, ++oi)
            *oi = it->first;
    }
}

template <typename OutputIterator>
void ConfigTable::getSections(OutputIterator oi)
{
    for (MapType::const_iterator it = mData.begin(); it != mData.end(); ++it, ++oi)
        *oi = it->first;
}

template <typename T>
T ConfigTable::getValueT(const std::string& section,
            const std::string& token,
            const T& def) const
{
    // find section
    MapType::const_iterator si = mData.find(section);
    
    if (si != mData.end())
    {
        // find token
        MapType::mapped_type::const_iterator token_it = si->second.find(token);
        
        if (token_it != si->second.end())
        {
            // extract value with stream
            T value;
            std::istringstream s(token_it->second);
            s >> value;
            
            if (s)
                return value;
        }
    }
    return def;
}

template <typename T>
std::vector<T> ConfigTable::getTableT(const std::string& section,
                         const std::string& token ,const T& def) const
{
    std::vector<T> result;
    std::string str = getValue(section,token,"");
    if(str.empty())
        return  result;
    
    std::vector<std::string> _tokens;
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
    
    for (int i = 0 , ni = _tokens.size(); i< ni ; ++i)
    {
        T value;
        std::istringstream s(_tokens[i]);
        s >> value;
        result.push_back(value);
    }
    return result;
}

class ConfigTableMgr
{
public:
    
    ConfigTable* getTable(const string& tableName);
    
    void clear();
    
    void removeTable(const string& tableName);
    
    string getValue(const string& tableName, string& section, string& key, const std::string& def);
    
    template <typename T>
    T getValueT(const std::string& tableName,
                const std::string& section,
                const std::string& key,
                const T& def) const;
    
    
    void addSearchPath(const string& path);
    
    void loadAllTable();
    
    void loadDir(const char* path);
    
private:
    bool addTable(string tableName, const MapType& tableData);
    
    
    
    void loadCfg(const string& filename);
    
    
private:
    map<string, ConfigTable*> mTables;
    vector<std::string> mSearchPath;
    ConfigTable mNilTable;
};

template <typename T>
T ConfigTableMgr::getValueT(const std::string& tableName,
            const std::string& section,
            const std::string& key,
            const T& def) const
{
    map<string, ConfigTable*>::const_iterator iter = mTables.find(tableName);
    
    if (iter != mTables.end()) {
        
        const ConfigTable* table = iter->second;
        
        if (table) {
            return table->getValueT(section, key, def);
        }
    }
    
    return def;
}

typedef void (*CFGLOADFUNC)(std::string fullpath);

class ConfigInterpreter
{
public:
    void registerLoaderFunc(string name, CFGLOADFUNC p_func)
    {
        mFuncMap[name] = p_func;
    }
    
    bool callFunc(string name)
    {
        map<string, CFGLOADFUNC>::iterator iter = mFuncMap.find(name);
        
        if (iter == mFuncMap.end()) {
            return false;
        }
        
        CFGLOADFUNC func = iter->second;
        
        func(name);
        return true;
    }
private:
    map<string, CFGLOADFUNC> mFuncMap;
};


extern ConfigTableMgr g_ConfigTableMgr;
extern ConfigInterpreter g_ConfigInterpreter;

#endif