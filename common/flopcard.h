//
//  flopcard.h
//  GameSrv
//
//  Created by cxy on 13-3-15.
//  翻牌配置文件
//

#ifndef __GameSrv__flopcard__
#define __GameSrv__flopcard__

#include <map>
#include <string>
#include <set>
#include <vector>
#include "Singleton.h"

class flopcard {
public:
    int id; //对应场景
    std::string name; //场景名称
//    std::string rewards1; //奖励物品1
//    int pro1; //权重
//    std::string rewards2; //奖励物品2
//    int pro2; //权重
//    std::string rewards3; //奖励物品3
//    int pro3; //权重
//    std::string rewards4; //奖励物品4
//    int pro4; //权重

    int cardnum;
    std::vector<int> pros;
    std::vector<std::string> rewards;

    int dropnum;
    std::vector<std::string> dropitems;
    std::vector<float> dropprops;
    
    std::set<std::string> allItemIds;

    std::vector<std::string> getAwards(int outindex, const std::vector<int>& drops);
};

class flopcardMgr {
public:
    /*
        随机发放奖励
        ratio:加成比例, 默认为0 (2013-10-10修改默认值为0)
    */
    std::string RandomCard(int id,int& outindex, std::vector<int>& drops, std::string& dropStr, float ratio=0);
    flopcard* Find(int id);
    bool LoadFile(const char* FileName);
    std::map<int,flopcard*> _flopcards;

};

#define SflopcardMgr Singleton<flopcardMgr>::Instance()

#endif /* defined(__GameSrv__flopcard__) */
