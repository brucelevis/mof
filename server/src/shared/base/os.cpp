//
//  os.cpp
//  GameSrv
//
//  Created by prcv on 13-8-30.
//
//

#include "os.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __APPLE__
#include <stdlib.h>
#include <mach-o/dyld.h>
#include <sys/param.h>
#else
#include <unistd.h>
#endif

#include "uuid/uuid.h"


string os::getAppPath()
{
    string ret;
    char path[1024];
    path[0] = 0;
    
    
#if __APPLE__
    
    uint32_t buflen = 1024;
    _NSGetExecutablePath(path, &buflen);
    
    char* realp = realpath(path, NULL);
    if (realp)
    {
        ret = realp;
        
        free( realp );
    }
    
#else
    
    char pidname[128];
    sprintf(pidname,"/proc/%d/exe", getpid());
    int retlen = readlink(pidname, path, sizeof(path));
    if (retlen > 0 ) path[retlen] = 0;
    
    ret = path;
#endif
    
    // 去掉路径
    int n = ret.rfind('/');
    if (n != -1)
    {
        ret = ret.substr(0, n);
    }
    
    return ret;
}



string fs::formatAbsPath(const char* path, const char* file)
{
    string abspath = path;
    if (*abspath.rbegin() != '/')
    {
        abspath.append("/");
    }
    abspath.append(file);
    return abspath;
}

void fs::listFiles(const char* path, vector<string>& files)
{
    DIR*     dir;
    dirent*  pdir;

    dir = opendir(path);
    if (!dir)
    {
        return;
    }
    while ((pdir = readdir(dir)))
    {
        if (pdir->d_type == DT_DIR)
        {
            if (pdir->d_name[0] == '.')
            {
                if (!pdir->d_name[1] || pdir->d_name[1] == '.')
                {
                    continue;
                }
            }

            listFiles(formatAbsPath(path, pdir->d_name).c_str(), files);
        }
        else
        {
            files.push_back(formatAbsPath(path, pdir->d_name));
        }
    }
    closedir(dir);
}

long fs::getModifyTime(const char *path)
{
    struct stat filestat;
    int ret = stat(path, &filestat);
    if (ret != 0){
        return 0;
    }
    #ifdef __APPLE__
    return filestat.st_mtimespec.tv_sec;
    #else
    return filestat.st_mtim.tv_sec;
    #endif
}

string fs::dirName(const char* file)
{
    const char* p = file;
    const char* endP = NULL;
    while (*p)
    {
        if (*p == '/' || *p == '\\')
        {
            endP = p;
        }
        p++;
    }

    if (endP == NULL)
    {
        return ".";
    }

    string ret = string(file, endP);
    if (ret.empty())
    {
        ret = "/";
    }

    return ret;
}

string fs::baseName(const char* file)
{
    const char* p = file;
    const char* beginP = NULL;
    while (*p)
    {
        if (*p == '/' || *p == '\\')
        {
            beginP = p;
        }
        p++;
    }

    string ret = file;
    if (beginP != NULL)
    {
        ret = string(beginP + 1);
    }
    return ret;
}
