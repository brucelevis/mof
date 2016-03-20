//
//  LogFiles.cpp
//  GameSrv
//
//  Created by pireszhi on 13-6-15.
//
//

#include "LogFiles.h"
#include "log.h"
#include "Utils.h"
#include "inifile.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "Utils.h"
#include "process.h"
#include <unistd.h>

int LogFileMgr::pathInit()
{
    mCurrDate = Utils::currentDate();
    mFiles.clear();
    mPath = Process::env.getString("logpath");

    if (mPath.empty()) {
        mPath = Process::execpath;
        mPath.append("/log");
    }
    mPath.append("/");

    mPath += Process::env.getProperty("server_id");
    StringTokenizer path(mPath, "/",StringTokenizer::TOK_TRIM | StringTokenizer::TOK_IGNORE_EMPTY);
    //创建日志文件的存放路径
    mPath = "/";
    chdir("/");
    for (int i = 0; i < path.count(); i++) {
        string subpath= path[i];
        mPath.append(subpath);
        mPath.append("/");
        if (chdir(subpath.c_str()) < 0) {
            int r = mkdir(subpath.c_str(), S_IRWXU | S_IRWXO);
            if (r < 0) {
                char error[128] = "";
                snprintf(error, 128, "logfile direction mkdir %s error : perhaps Permission denied\n", subpath.c_str());
                chdir(Process::execpath.c_str());
                printf("%s", error);
                return -1;
            }
            chdir(subpath.c_str());
        }
    }
    chdir(Process::execpath.c_str());
    printf("GameLog dir: %s \n", mPath.c_str());

    return 0;
}

FILE* LogFileMgr::openFile(const char *fileName)
{
    string newDate = Utils::currentDate();
    if (strcmp(newDate.c_str(), mCurrDate.c_str()) != 0) {
        mCurrDate = newDate;
        ClearFilesMap();
    }

    std::map<string, FILE*>::iterator iter = mFiles.find(fileName);
    if (iter == mFiles.end()) {

        //进入日志文件存放目录
        chdir(mPath.c_str());

        if (chdir(mCurrDate.c_str()) < 0) {
            int r = mkdir(mCurrDate.c_str(), S_IRWXU | S_IRWXO);
            if (r < 0) {
                char error[128] = "";
                sprintf(error, "mkdir %s error\n", mCurrDate.c_str());
                log_error(error);
                return NULL;
            }
            chdir(mCurrDate.c_str());
        }

        FILE* newfile = fopen(fileName, "a+");
        mFiles.insert(make_pair(fileName, newfile));
        chdir(Process::execpath.c_str());
        return newfile;
    }

    return iter->second;
}

bool LogFileMgr::ClearFilesMap()
{
    std::map<string, FILE*>::iterator iter;
    for (iter = mFiles.begin(); iter != mFiles.end();iter++) {
        fclose(iter->second);
    }

    mFiles.clear();
    return true;
}
