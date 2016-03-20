//
//  DefConf.h
//  GameSrv
//
//  Created by cxy on 13-2-5.
//
//

#ifndef __GameSrv__DefConf__
#define __GameSrv__DefConf__

#include <iostream>
#include "inifile.h"
#include "Singleton.h"

class DefConf {
public:
    void LoadFile(const char* filename);
    int def_scene;
    int def_x;
    int def_y;
};

#define SDefConf Singleton<DefConf>::Instance()

#endif /* defined(__GameSrv__DefConf__) */
