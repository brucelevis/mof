//
//  CapsuletoyMgr.cpp
//  GameSrv
//
//  Created by nothing on 14-4-25.
//
//

#include "CapsuletoyMgr.h"

#include "DataCfg.h"
#include "Role.h"
#include "GameLog.h"
#include "daily_schedule_system.h"
#include "mail_imp.h"
#include "EnumDef.h"

static const char *DB_NAME = "caspuletoy";



CapsuletoyMgr::CapsuletoyMgr()
{
    mRole = NULL;
}

CapsuletoyMgr::~CapsuletoyMgr()
{
}

CapsuletoyMgr *CapsuletoyMgr::create()
{
    return new CapsuletoyMgr();
}

void CapsuletoyMgr::load(Role *role)
{
    if (!role)
    {
        return ;
    }
    
    mRole = role;
    
    bool needsave = false;
    
    int nowtime = time(NULL);

    //有多少种品质扭蛋，则重复拿几次数据
    int qua_cnt = GashaponMachinesCfg::getQuaCount();
    for (int i = 1; i <= qua_cnt; ++i)
    {
        CapsuletoyItem item;
        string str;
        __load_one_record_from_db(i, str);
        if (str.empty() || !__parse(str, item))
        {
            //没有该条记录，但是也得记录插入到mData中
            item.qua_type = i;
            item.refresh_time = nowtime;
            mData.insert(make_pair(i, item));
            
            //用于第一次保存数据
            needsave = true;
            continue ;
        }
        
        mData.insert(make_pair(i, item));
    }
    
    if (needsave)
    {
        __saveall();
    }
}

int CapsuletoyMgr::remainBuyTimes(int qua_type)
{
	GashaponMachinesQuaItem item = GashaponMachinesCfg::getGashaponMachinesQuaItem(qua_type);
    DataMapIterator iter = mData.find(qua_type);
	
	int sum = 0;
	int viptimes = GashaponMachinesCfg::getViplvlBuyTimesLimit(item.qua_type, mRole->getVipLvl());
    if (item.buy_times_limit < 0 || viptimes < 0)
    {
        sum = -1;
    }
    else
    {
        sum = item.buy_times_limit + viptimes - iter->second.buy_times;
    }
	return sum;
}


void CapsuletoyMgr::sendCapsuletoyStatus()
{
    ack_capsuletoy_status ack;
    ack.errorcode = 0;
    
    if (!GashaponMachinesCfg::isOpen())
    {
        return ;
    }
    
    int nowtime = time(NULL); 
    
    for (DataMapIterator iter = mData.begin(); iter != mData.end(); ++iter)
    {
        obj_capsuletoy_info tmp;
        
        GashaponMachinesQuaItem item = GashaponMachinesCfg::getGashaponMachinesQuaItem(iter->first);
        if (!item.is_struct_enable)
        {
            //不应该走这一步啊
            continue ;
        }
        
        tmp.qua = item.qua_type;
        tmp.enable_get = 0;
        tmp.sell_price = item.sell_price;
        tmp.can_everyday_free_get = item.can_everyday_free_get;
        tmp.is_have_special_offers = item.is_have_special_offers;
        tmp.special_offers_buy_count = item.special_offers_buy_count;
        tmp.special_offers_present_count = item.special_offers_buy_present;
        tmp.only_sell = item.only_sell;
        tmp.buy_times = iter->second.buy_times;  //已经购买次数
		tmp.remain_buy_times = this->remainBuyTimes(item.qua_type);
		tmp.award_mark = iter->second.award_mark;

        
        // can_everyday_free_get等于1表示qua1类型，把剩余次数发回去
        if (item.can_everyday_free_get == 1)
        {
            tmp.remain_free_times = item.everyday_free_times - iter->second.free_times;
        }
        else
        {
            tmp.remain_free_times = -1; 
        }
        
        
        // only_sell等于1表示单独售卖，没有刷新时间，例如qua4
        if (item.only_sell == 1)
        {
            tmp.remain_time = -1;
        }
        else  // qua1,2,3
        {
            //判断时间有没有过刷新固定时间
            int remaintime = nowtime - iter->second.refresh_time;
            
            //已经超过了刷新时间，领取次数还有的话
            if (remaintime >= item.refresh_time)
            {
                tmp.remain_time = 0;
                tmp.enable_get = 1;
                
                //有免费领取次数，qua1免费次数用完了
                if (item.can_everyday_free_get == 1 && tmp.remain_free_times <= 0)
                {
                    tmp.enable_get = 0;
                }
            }
            else  //CD没过，则把剩余时间发过去
            {
                if (item.can_everyday_free_get == 1 && tmp.remain_free_times <= 0)
                {
                    tmp.remain_time = 0;
                }
                else
                {
                    tmp.remain_time = item.refresh_time - remaintime;
                }
            }
        }
        ack.info.push_back(tmp);
    }
    mRole->send(&ack);
}



// 免费领取扭蛋
void CapsuletoyMgr::getFreeCapsuletoy(req_capsuletoy_get_egg &req)
{
    ack_capsuletoy_get_egg ack;
    ack.qua_type = req.qua_type;
    
    if (!__check_base_condition(ack.qua_type, ack.errorcode))
    {
        mRole->send(&ack);
        return ;
    }
    
    int nowtime = time(NULL);

    if (!__check_get_free_capsuletoy_conditions(req.qua_type, ack.errorcode))
    {
        mRole->send(&ack);
        return ;
    }
    
    // 前面已经做了检查，可以直接随机出一件物品
    GashaponMachinesQuaItem item = GashaponMachinesCfg::getGashaponMachinesQuaItem(req.qua_type);
    DataMapIterator iter = mData.find(req.qua_type);

    
    vector<int> itemids;
    vector<int> itemcounts;
    // 随机出一样物品,添加到背包
    __random_n_item(req.qua_type, mRole->getLvl(), 1, itemids, itemcounts);
    __add_n_item_to_bag(itemids, itemcounts, "capsuletoy free get");
    __add_log(ack.qua_type, itemids, itemcounts, "capsuletoy free get");
    
    if (mRole->getDailyScheduleSystem()) {
        mRole->getDailyScheduleSystem()->onEvent(kDailyScheduleCapsuleToy, 0, 1);
    }
    
    // 不应该没有物品
    if (itemids.empty())
    {
        ack.item_id = ack.item_count = 0;
    }
    else
    {
        ack.item_id = itemids[0];
        ack.item_count = itemcounts[0];
    }
    
    
    // qua1类型
    if (item.can_everyday_free_get == 1)
    {
        iter->second.free_times += 1;
        ack.remain_free_times = item.everyday_free_times - iter->second.free_times;
    }
    else
    {
        ack.remain_free_times = -1;
    }
    
    // 领完后，返回刷新时间
    
    ack.remain_time = item.refresh_time;
    
    iter->second.refresh_time = nowtime;
    
    // 保存更改的数据到数据库
    string str;
    if (__tostring(iter->second, str))
    {
        __save_one_record_to_db(iter->first, str);
    }
    else
    {
        // 保证一定不走这一步
        log_warn("save record to db error.");
    }

    
    mRole->send(&ack);
}

// 购买扭蛋
void CapsuletoyMgr::buyCapsuletoy(req_capsuletoy_buy_egg &req)
{
    ack_capsuletoy_buy_egg ack;
    ack.qua_type = req.qua_type;
    
    if (!__check_base_condition(ack.qua_type, ack.errorcode))
    {
        mRole->send(&ack);
        return ;
    }
    
    int needrmb = 0;
    if (!__check_buy_capsuletoy_conditions(ack.qua_type, ack.errorcode, 1, needrmb))
    {
        mRole->send(&ack);
        return ;
    }
    
    // 扣金钻
    mRole->addRmb(-needrmb, "buy capsuletoy", true);
    
    // 随机物品到背包
    vector<int> itemids;
    vector<int> itemcounts;
    __random_n_item(ack.qua_type, mRole->getLvl(), 1, itemids, itemcounts);
    __add_n_item_to_bag(itemids, itemcounts, "capsuletoy buy");
    __add_log(ack.qua_type, itemids, itemcounts, "capsuletoy buy");
    if (mRole->getDailyScheduleSystem()) {
        mRole->getDailyScheduleSystem()->onEvent(kDailyScheduleCapsuleToy, 0, 1);
    }
    
    GashaponMachinesQuaItem item = GashaponMachinesCfg::getGashaponMachinesQuaItem(ack.qua_type);
    DataMapIterator iter = mData.find(ack.qua_type);
    
    // 物品购买次数加1
    iter->second.buy_times += 1;
    
    string str;
    if (__tostring(iter->second, str))
    {
        __save_one_record_to_db(iter->first, str);
    }
    
    // 配置表应该有问题
    if (itemids.empty())
    {
        ack.item_id = ack.item_count = 0;
    }
    else
    {
        ack.item_id = itemids[0];
        ack.item_count = itemcounts[0];
    }
    
	ack.remain_buy_times = this->remainBuyTimes(req.qua_type);
    
    mRole->send(&ack);
}

void CapsuletoyMgr::buyNCapsuletoy(req_capsuletoy_buy_n_egg &req)
{
    ack_capsuletoy_buy_n_egg ack;
    ack.qua_type = req.qua_type;
    
    if (!__check_base_condition(ack.qua_type, ack.errorcode))
    {
        mRole->send(&ack);
        return ;
    }
    
    
    GashaponMachinesQuaItem item = GashaponMachinesCfg::getGashaponMachinesQuaItem(req.qua_type);
    DataMapIterator iter = mData.find(req.qua_type);
    
    int needrmb = 0;
    bool freeget = false;
    // 有买X送Y这类活动
    if (item.is_have_special_offers == 1)
    {
        if (item.special_offers_buy_count > 0)  // 除数不应该是0
        {
            //说明还没有领取
            if (iter->second.buy_times / item.special_offers_buy_count > iter->second.award_mark)
            {
                freeget = true;
            }
        }
    }
    
    // 不是免费领取，则要检查购买次数
    if (!freeget)
    {
        if (!__check_buy_capsuletoy_conditions(req.qua_type, ack.errorcode, 1, needrmb))
        {
            mRole->send(&ack);
            return ;
        }
    }
    
    // 扣金钻
    mRole->addRmb(-needrmb, "buy n capsuletoy", true);
    
    // 记录免费领取mark
    if (freeget)
    {
        iter->second.award_mark = iter->second.buy_times / item.special_offers_buy_count;
    }
    
    vector<int> itemids;
    vector<int> itemcounts;
    __random_n_item(req.qua_type, mRole->getLvl(), 1, itemids, itemcounts);
    string desc;
    if (freeget)
    {
        desc = "buy n capsuletoy free get";
    }
    else
    {
        desc = "buy n capsuletoy";
    }
    __add_n_item_to_bag(itemids, itemcounts, desc.c_str());
    __add_log(ack.qua_type, itemids, itemcounts, desc.c_str());
    if (mRole->getDailyScheduleSystem()) {
        mRole->getDailyScheduleSystem()->onEvent(kDailyScheduleCapsuleToy, 0, 1);
    }
    
    // 不是免费领
    if (!freeget)
    {
        iter->second.buy_times += 1;
    }
    
    
    string str;
    if (__tostring(iter->second, str))
    {
        __save_one_record_to_db(iter->first, str);
    }
    
    ack.item_id = itemids;
    ack.item_count = itemcounts;
    ack.buy_times = iter->second.buy_times;
    ack.special_offers_buy_count = item.special_offers_buy_count;
    ack.special_offers_present_count = item.special_offers_buy_present;
	ack.remain_buy_times = this->remainBuyTimes(req.qua_type);
	ack.award_mark = iter->second.award_mark;
    
    mRole->send(&ack);
}


void CapsuletoyMgr::resetData()
{
    doRedisCmd("del %s:%d", DB_NAME, mRole->getInstID());
    mData.clear();
}

void CapsuletoyMgr::sendEggCompensation()
{
    // 对之前388金钻的类型进行补偿
    DataMapIterator iter = mData.find(4);
    if (iter != mData.end() && iter->second.buy_times != 0) {
        // 补偿粉色技能书个数 = 3 * 购买次数
        int compensationNum = 3 * iter->second.buy_times;
        string mailsender = "系统";
        string rolename = mRole->getRolename();
        string mailtitle = "扭蛋改版补偿";
        string mailcontent = "因之前扭蛋最高金额为388金钻一次，新版降至288金钻一次，特设此补偿，祝您游戏愉快！";
        string mailitems = xystr("item 400004*%d;", compensationNum);
        sendMail(mRole->getInstID(), mailsender.c_str(), rolename.c_str(),
                 mailtitle.c_str(), mailcontent.c_str(), mailitems.c_str(), "");
        Xylog log(eLogName_EggCompensation, mRole->getInstID());
        log << compensationNum;
    }
}

void CapsuletoyMgr::dayRefresh()
{
    int nowtime = time(NULL);
    for (DataMapIterator iter = mData.begin(); iter != mData.end(); ++iter)
    {
        GashaponMachinesQuaItem item = GashaponMachinesCfg::getGashaponMachinesQuaItem(iter->first);
        if (!item.is_struct_enable)
        {
            //一定不能走这里
            continue;
        }
        
        // 只有每天免费领N次的才需要刷新时间
        if (item.can_everyday_free_get == 1)
        {
            iter->second.refresh_time = nowtime + 1;  //加1秒好了
        }
        if (item.only_sell != 1) // 不是单卖次数才清0，=。=
        {
            iter->second.buy_times = iter->second.free_times = 0;
        }
    }
    __saveall();
}


/* private functions */

bool CapsuletoyMgr::__parse(const string &str, CapsuletoyItem &item)
{
    Json::Value value;
    Json::Reader reader;
    if (!reader.parse(str, value) || value.type() != Json::objectValue)
    {
        return false;
    }
    item.qua_type = value["type"].asInt();      
    item.refresh_time = value["rtime"].asInt();
    item.free_times = value["ftimes"].asInt();
    item.buy_times = value["btimes"].asInt();
    item.award_mark = value["amark"].asInt();
    return true;
}

bool CapsuletoyMgr::__tostring(const CapsuletoyItem &item, string &str)
{
    Json::Value value;
    value["type"] = item.qua_type;
    value["rtime"] = item.refresh_time;
    value["ftimes"] = item.free_times;
    value["btimes"] = item.buy_times;
    value["amark"] = item.award_mark;
    str = Json::FastWriter().write(value);
    if (str.size() > 0)
    {
        str.resize(str.size() - 1);
    }
    return true;
}

void CapsuletoyMgr::__saveall()
{
    if (!mRole)
    {
        return ;
    }
    
    for (map<int, CapsuletoyItem>::iterator iter = mData.begin(); iter != mData.end(); ++iter)
    {
        string str;
        if (__tostring(iter->second, str))
        {
            __save_one_record_to_db(iter->first, str);
        }
    }
}

void CapsuletoyMgr::__load_one_record_from_db(const int &key, string &str)
{
    RedisResult result(redisCmd("hget %s:%d %d", DB_NAME, mRole->getInstID(), key));
    str = result.readStr();
}

void CapsuletoyMgr::__save_one_record_to_db(const int &key, const string &value)
{
    doRedisCmd("hset %s:%d %d %s", DB_NAME, mRole->getInstID(), key, value.c_str());
}

bool CapsuletoyMgr::__check_base_condition(const int &quatype, int &errorcode, const int &buycnt)
{
    // 活动开关没开
    if (!GashaponMachinesCfg::isOpen())
    {
        errorcode = eCapsuletoyResult_ActivityIsNotOpen;
        return false;
    }
    
    // qua type有错误
    if (quatype <= 0 || quatype > GashaponMachinesCfg::getQuaCount())
    {
        errorcode = eCapsuletoyResult_UnknowError;
        return false;
    }
    
    GashaponMachinesQuaItem item = GashaponMachinesCfg::getGashaponMachinesQuaItem(quatype);
    DataMapIterator iter = mData.find(quatype);
    
    //没有找到qua type的item
    if (!item.is_struct_enable || iter == mData.end())
    {
        errorcode = eCapsuletoyResult_UnknowError;
        return false;
    }
    
    // 背包太满了，至少预留购买数量个格子
    if (mRole->getBackBag()->EmptyGridAmount() < buycnt)
    {
        errorcode = eCapsuletoyResult_BagIsFull;
        return false;
    }
    
    return true;
}

bool CapsuletoyMgr::__check_get_free_capsuletoy_conditions(const int &quatype, int &errorcode)
{
    GashaponMachinesQuaItem item = GashaponMachinesCfg::getGashaponMachinesQuaItem(quatype);
    DataMapIterator iter = mData.find(quatype);
    
    int nowtime = time(NULL);
    
    do
    {
        // only_sell等于1表示单卖，不能免费领取,要是走免费领取这个协议，有问题
        if (item.only_sell == 1)
        {
            errorcode = eCapsuletoyResult_UnknowError;
            break;
        }

        // 免费领取CD还没过
        if (nowtime - iter->second.refresh_time < item.refresh_time)
        {
            errorcode = eCapsuletoyResult_NotTimeToGetEgg;
            break;
        }

        // 今日免费次数已经用完了
        if (item.can_everyday_free_get == 1)
        {
            if (item.everyday_free_times - iter->second.free_times <= 0)
            {
                errorcode = eCapsuletoyResult_FreeTimesIsUseUp;
                break;
            }
        }
        
        errorcode = eCapsuletoyResult_Ok;
        return true;
        
    } while (0);
    
    return false;
}



void CapsuletoyMgr::__random_n_item(const int &quatype,
									const int &lvl,
									const int &nrepeat,
									vector<int> &itemids,
									vector<int> &itemcounts)
{
    vector<GashaponMachinesAwardItem> vec =  GashaponMachinesCfg::getGashaponMachinesAwardItemByLvl(quatype, lvl);
    if (vec.empty())  //奖励不应该是空。
    {
        log_warn("Not found award...  " << __FILE__);
    }
    else
    {
		int timeNum = __get_buy_time(quatype);
		
		// 计算当前次数, 获取次数对应的权限区间位置
		int propIndex = GashaponMachinesCfg::getTimeNumIndex(timeNum);
		
        vector<int> props;
        for (int i = 0; i < vec.size(); ++i)
        {
			// 加载的时候已经保证item_prop有内容
            props.push_back(vec[i].item_prop[propIndex]);
        }
        
        for (int i = 0; i < nrepeat; ++i)
        {
            int index = getRand(props.data(), props.size());
            itemids.push_back(vec[index].item_id);
            itemcounts.push_back(vec[index].item_count);
        }
    }
}


void CapsuletoyMgr::__add_n_item_to_bag(vector<int> &itemids, vector<int> &itemcounts, const char *desc)
{
    vector<string> items;
    ItemArray itemarray;
    for (int i = 0; i < itemids.size(); ++i)
    {
        string itemstr = Utils::makeStr("item %d*%d", itemids[i], itemcounts[i]);
        items.push_back(itemstr);
    }
    mRole->addAwards(items, itemarray, desc);
}

bool CapsuletoyMgr::__check_buy_capsuletoy_conditions(const int &quatype, int &errorcode , const int &buycnt, int &needrmb, bool isfree)
{
    GashaponMachinesQuaItem item = GashaponMachinesCfg::getGashaponMachinesQuaItem(quatype);
    //DataMapIterator iter = mData.find(quatype);
    
    do
    {
        // 不是免费领取，则判断次数和金钻问题
        if (!isfree)
        {
            // 大于等于0表示有次数购买限制
            int remain_buy_times = this->remainBuyTimes(quatype);
            if (remain_buy_times >= 0)
            {
                if (remain_buy_times < buycnt)
                {
                    errorcode = eCapsuletoyResult_BuyTimesIsLessThanNTimes;
                    break;
                }
            }
            
            // 金钻不够
            if (mRole->getRmb() < item.sell_price * buycnt)
            {
                errorcode = eCapsuletoyResult_NotEnoughRmb;
                break;
            }
        }
        
        errorcode = eCapsuletoyResult_Ok;
        
        if (!isfree)
        {
            needrmb = item.sell_price * buycnt;
        }
        else
        {
            needrmb = 0;
        }
        
        return true;
        
    } while (0);
    
    return false;
}

void CapsuletoyMgr::__add_log(const int &qua_type, vector<int> &itemids, vector<int> &itemcounts, const char *desc)
{
    Xylog log(eLogName_CapsuleToy, mRole->getInstID());
    log << desc << qua_type;
    for (int i = 0; i < itemids.size(); ++i)
    {
        log << itemids[i] << itemcounts[i];
    }
}

int CapsuletoyMgr::__get_buy_time(int qua_type)
{
	DataMapIterator iter = mData.find(qua_type);
	if (iter == mData.end()) {
		return 0;
	}
	return iter->second.buy_times + iter->second.award_mark;
}