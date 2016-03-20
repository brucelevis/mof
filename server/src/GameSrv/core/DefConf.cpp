//
//  DefConf.cpp
//  GameSrv
//
//  Created by cxy on 13-2-5.
//
//

#include "DefConf.h"
#include "inifile.h"
#include "GameIniFile.h"

INSTANTIATE_SINGLETON(DefConf);

void DefConf::LoadFile(const char* filename)
{
    GameInifile ini(filename);
    def_scene = ini.getValueT("root", "def_scene", 0);
    def_x = ini.getValueT("root", "def_x", 0);
    def_y = ini.getValueT("root", "def_y", 0);
}