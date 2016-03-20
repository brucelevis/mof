//
//  GameIniFile.cpp
//  GameSrv
//
//  Created by pireszhi on 15-6-2.
//
//

#include <stdio.h>

#include "GameIniFile.h"

GameInifile::GameInifile(const std::string& fileName)
{
    string realFileName = fs::baseName(fileName.c_str());
    mConfigTable = g_ConfigTableMgr.getTable(realFileName);
}

GameInifile::~GameInifile()
{
}

GameInifile::GameInifile(const char* fileName)
{
    string realFileName = fs::baseName(fileName);
    mConfigTable = g_ConfigTableMgr.getTable(realFileName);
}

std::string GameInifile::getValue(const std::string& section, const std::string& token, const std::string& def) const
{
    if (mConfigTable == NULL)
    {
        return def;
    }
    return mConfigTable->getValue(section, token, def);
}

bool GameInifile::exists(const std::string& section) const
{
    if(mConfigTable == NULL)
    {
        return false;
    }
    return mConfigTable->exists(section);
}

/// Returns true, if key exists in section exists.
bool GameInifile::exists(const std::string& section, const std::string& token) const
{
    if(mConfigTable == NULL)
    {
        return false;
    }
    return mConfigTable->exists(section, token);
}

std::vector<std::string> GameInifile::getTable(const std::string& section, const std::string& token) const
{
    if(mConfigTable == NULL)
    {
        std::vector<std::string> ret;
        ret.clear();
        return ret;
    }
    return mConfigTable->getTable(section, token);
}
