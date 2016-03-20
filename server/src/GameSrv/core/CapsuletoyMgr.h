//
//  CapsuletoyMgr.h
//  GameSrv
//
//  Created by nothing on 14-4-25.
//
//  扭蛋功能


#ifndef __GameSrv__CapsuletoyMgr__
#define __GameSrv__CapsuletoyMgr__

#include <iostream>
#include <map>
#include <vector>

#include "msg.h"
using namespace std;



class Role;

// 保存到数据库的json
struct CapsuletoyItem
{
    int qua_type;       //品质类型（1，2，3，4）
    int refresh_time;   //领取的时间
    int free_times;     //已经用过的免费次数，0递增
    int buy_times;      //已经购买次数，0递增
    int award_mark;     //买了超过10次要送一次,存1 2 3 4分别表示领取1次，2次。。。
    CapsuletoyItem()
    {
        qua_type = 0;
        refresh_time = 0;
        free_times = 0;
        buy_times = 0;
        award_mark = 0;
    }
};

//人物身上的管理类
class CapsuletoyMgr
{
public:
    //          qua  item
    typedef map<int, CapsuletoyItem> DataMap;
    typedef DataMap::iterator DataMapIterator;
    
    
    CapsuletoyMgr();
    ~CapsuletoyMgr();
    
    // 创建管理类
    static CapsuletoyMgr *create();
    
    // 从数据库加载人物数据
    void load(Role *role);
    
    // 发送扭蛋状态
    void sendCapsuletoyStatus();
    
    // 免费领取扭蛋
    void getFreeCapsuletoy(req_capsuletoy_get_egg &req);
    
    // 购买扭蛋
    void buyCapsuletoy(req_capsuletoy_buy_egg &req);
    
    // 买10次送1次免费，走这个协议，诶悲剧搞错需求了（qua4购买走这条协议）
    void buyNCapsuletoy(req_capsuletoy_buy_n_egg &req);
    
    void dayRefresh();

    void resetData();
    
    // 发送扭蛋改版后的补偿
    void sendEggCompensation();
    
	// 计算剩余的购买次数
	int remainBuyTimes(int qua_type);
private:
    // save all data
    void __saveall();
    // load one record from db
    void __load_one_record_from_db(const int &key, string &str);
    // save one record to db
    void __save_one_record_to_db(const int &key, const string &value);
    // string to item
    bool __parse(const string &str, CapsuletoyItem &item);
    // item to string
    bool __tostring(const CapsuletoyItem &item, string &str);
    
    
    bool __check_base_condition(const int &quatype, int &errorcode, const int &buycnt = 1);
    // get free egg
    bool __check_get_free_capsuletoy_conditions(const int &quatype, int &errorcode);
    
    bool __check_buy_capsuletoy_conditions(const int &quatype, int &errorcode , const int &buycnt, int &needrmb, bool isfree = false);
    //  随机出物品
	void __random_n_item(const int &quatype,
						 const int &lvl,
						 const int &nrepeat,
						 vector<int> &itemids,
						 vector<int> &itemcounts);
    // 添加物品到背包
    void __add_n_item_to_bag(vector<int> &itemids, vector<int> &itemcounts, const char *desc);
    //
    void __add_log(const int &qua_type, vector<int> &itemids, vector<int> &itemcounts, const char *desc);

    
	int __get_buy_time(int qua_type);
	
    
private:
    DataMap mData;
    Role *mRole;
};


#endif /* defined(__GameSrv__CapsuletoyMgr__) */
