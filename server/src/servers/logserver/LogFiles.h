//
//  LogFiles.h
//  GameSrv
//
//  Created by pireszhi on 13-6-15.
//
//

#ifndef __GameSrv__LogFiles__
#define __GameSrv__LogFiles__

#include <iostream>
#include <stdio.h>
#include <fcntl.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <string.h>
#include <vector>
using namespace std;

class LogFileMgr
{
public:
    int pathInit();
    FILE* openFile(const char* fileName);
    bool ClearFilesMap();
    
private:
    std::map<string, FILE*> mFiles;
    string mPath;
    string mCurrDate;
};

#endif /* defined(__GameSrv__LogFiles__) */

