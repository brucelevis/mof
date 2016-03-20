//
//  RandomEvents.h
//  GameSrv
//
//  Created by jin on 13-9-4.
//
// 

#ifndef GameSrv_RandomEvents_h
#define GameSrv_RandomEvents_h
#include <set>
#include <map>
#include <string>
#include "Defines.h"
#include "Singleton.h"
#include "../global/Activity.h"
#include "redis_helper.h"

using namespace std;

class Role;
class ItemCfgDef;
class PromotionActivityDef;
class PromotionActivityDef;

class RandomEventsConfig
{
public:
    RandomEventsConfig() {
        itemId = 0;
        dayNum = 0;
        allNum = 0;
        sysDayNum = 0;
        sysAllNum = 0;
    }
    RandomEventsConfig(int id):itemId(id) {
        dayNum = 0;
        allNum = 0;
        sysDayNum = 0;
        sysAllNum = 0;
    }
    bool insertData(string data);
    string formatData();
    
    int itemId;
    int dayNum;
    int allNum;
    // 系统上限数
    int sysDayNum;
    int sysAllNum;
};

class RandomEvents
{
public:
    RandomEvents(ItemCfgDef *icDef, PromotionActivityDef *paDef);
    void refresh();
    string randGoods();
    
private:
    // 是否达到上限
    bool isLimit(int itemId);
    
    void checkItemSave(string item);
    void saveRandomEventsConfig(RandomEventsConfig *rec);
    RandomEventsConfig * CreateGoodsRecord(int itemId);
    RandomEventsConfig * getGoodsRecord(int itemId);
private:
    ItemCfgDef *mIcDef;
    map<int, RandomEventsConfig> mGoodsRecord;
};

typedef std::map<int, RandomEvents> RandomEventsTd;
class RandomEventsSys
{
public:
    RandomEventsSys()
    {
        mRandomEventsMap.clear();
    }
    // 打开随机礼包
    string getRandBox(ItemCfgDef *icDef, Role *role);
    // 每天更新
    void refresh();
private:
    void physicalProc(Role *role, string items, ItemCfgDef *box);
    bool timeIsExpired(int endTime);
    RandomEvents * getRandonEvents(int id);
private:
    RandomEventsTd mRandomEventsMap;
};
#define SRandomEventsSys Singleton<RandomEventsSys>::Instance()

// ###################################################



#endif
