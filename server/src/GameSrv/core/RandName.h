//
//  RandName.h
//  GameSrv
//
//  Created by cxy on 13-1-30.
//

#ifndef __GameSrv__RandName__
#define __GameSrv__RandName__

#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include "Utils.h"

using namespace std;

typedef bitset<1024> NameMask;

class RandFirstName
{
public:
    int mIndex;      //对应的first name的索引
    int mFreeNum;    //剩余的second name的数量
    NameMask mMask;  //second name的状态，每个second name对应一位，0表示未被使用过，
};

class RandNameMgr
{
public:
    //加载配置文件
    bool loadConfig(const char* cfgFile);
    
    //加载数据，配置文件（所有随即名的姓和名）和数据库数据（已经使用过的随机名）
    bool load(const string& cfgFile);
    
    //获取随机名
    string getRandName(bool isMale);
    
    //更新随机名，从随机名管理器种去掉已经使用过的名字
    void update(bool isMale, const string& name, bool save = true);
    
    //
    void save();
    
private:
    
    //根据性别获取key
    static string getRandNameKeyByGender(bool isMale) {
        return isMale ? "randname:male" : "randname:female";
    }
    
    //加载已经使用过的随机名数据
    bool loadMaleData();
    bool loadFemaleData();
    
    void saveMaleData();
    void saveFemaleData();
    
    int                   mMaleFreeNum;    //剩余男性名字
    int                   mFemaleFreeNum;  //剩余女性名字
    vector<RandFirstName> mMaleFirstNames;   //男性名字的状态
    vector<RandFirstName> mFemaleFirstNames; //女性名字的状态
    
    vector<string> mFirstNamesList;         //姓列表
    vector<string> mMaleSecondNamesList;    //男性名列表
    vector<string> mFemaleSecondNamesList;  //女性名列表
};

extern RandNameMgr g_RandNameMgr;

#endif /* defined(__GameSrv__RandName__) */
