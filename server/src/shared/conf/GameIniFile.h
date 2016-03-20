//
//  GameIniFile.h
//  GameSrv
//
//  Created by pireszhi on 15-6-2.
//
//

#ifndef __Common__GameIniFile__
#define __Common__GameIniFile__

#include "inifile.h"
#include "GameTable.h"
#include "os.h"

class GameInifile
{
public:
    GameInifile():mConfigTable(NULL)
    {
        
    }
    
    GameInifile(const std::string& fileName);
    
    GameInifile(const char* fileName);
    
    ~GameInifile();
    
    std::string getValue(const std::string& section, const std::string& token, const std::string& def = std::string()) const;
    
    template <typename T>
    T getValueT(const std::string& section, const std::string& token, const T& def) const;
    
    bool exists(const std::string& section) const;
    
    /// Returns true, if key exists in section exists.
    bool exists(const std::string& section, const std::string& token) const;
    
    template <typename OutputIterator>
    void getSections(OutputIterator oi);
    
    template <typename OutputIterator>
    void getKeys(const std::string& section, OutputIterator oi);
    
    std::vector<std::string> getTable(const std::string& section, const std::string& token) const;
    
    template <typename T>
    std::vector<T> getTableT(const std::string& section, const std::string& token ,const T& def) const;
private:
    
    ConfigTable* mConfigTable;
    
};

template <typename T>
T GameInifile::getValueT(const std::string& section, const std::string& token, const T& def) const
{
    if (mConfigTable == NULL)
    {
        return def;
    }
    return mConfigTable->getValueT(section, token, def);
}

template <typename OutputIterator>
void GameInifile::getSections(OutputIterator oi)
{
    if (NULL != mConfigTable)
    {
        mConfigTable->getSections(oi);
    }
}

template <typename OutputIterator>
void GameInifile::getKeys(const std::string& section, OutputIterator oi)
{
    if (NULL != mConfigTable) {
        mConfigTable->getKeys(section, oi);
    }
}

template <typename T>
std::vector<T> GameInifile::getTableT(const std::string& section, const std::string& token ,const T& def) const
{
    if (mConfigTable == NULL) {
        return std::vector<T>();
    }
    
    return mConfigTable->getTableT(section, token, def);

}


#endif