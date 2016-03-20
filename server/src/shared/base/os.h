//
//  os.h
//  GameSrv
//
//  Created by prcv on 13-6-17.
//
//

#ifndef shared_os_h
#define shared_os_h

#include <string>
#include <vector>
using namespace std;


namespace os
{
    string getAppPath();
    
};

namespace fs
{
    string dirName(const char* file);
    string baseName(const char* file);
    string formatAbsPath(const char* path, const char* file);
    long getModifyTime(const char* path);
    void listFiles(const char* path, vector<string>& files);
};

#endif
