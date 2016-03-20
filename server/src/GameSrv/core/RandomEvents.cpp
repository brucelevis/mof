//
//  RandomEvents.cpp
//  GameSrv
//
//  Created by jin on 13-9-4.
//
//

#include "RandomEvents.h"
#include "main.h"
#include "Utils.h"
#include "GameLog.h"
#include "DBRolePropName.h"
#include "json/json.h"
#include "hander_include.h"
#include "mail_imp.h"
#include "BroadcastNotify.h"
#include "Role.h"
#include "DataCfg.h"
INSTANTIATE_SINGLETON(RandomEventsSys);

static void StrSpiltToInt(std::string str, const std::string& separators, vector<int> &vec)
{
    StringTokenizer token(str, separators);
    
    if (vec.size() < token.count()) {
        vec.resize(token.count(), 0);
    }
    
    vector<int>::iterator vecIt = vec.begin();
    StringTokenizer::Iterator it;
    for (it = token.begin(); it != token.end(); it++, vecIt++) {
        *vecIt = Utils::safe_atoi((*it).c_str(), 0);
    }
}

static ItemCfgDef * getItemCfg(string item)
{
    if (item.empty()) {
        return NULL;
    }
    int itemId = 0;
    if (sscanf(item.c_str(), "item %d*%*d", &itemId) != 1) {
        return NULL;
    }
    return ItemCfg::getCfg(itemId);
}


typedef void (*RandomEventsPropsCallback)(Role *role, ItemCfgDef *cfg, ItemCfgDef *box);
typedef std::map<int, RandomEventsPropsCallback> RandomEventsPropsCB;
class RandomEventsProps
{
public:
    void addCallBack(EnumItemPropsType attrType, RandomEventsPropsCallback cb);
    void handle(EnumItemPropsType attrType, Role *role, ItemCfgDef *cfg, ItemCfgDef *box);
    static void initRandomEventsProps();
private:
    RandomEventsPropsCB mRepMap;
};
RandomEventsProps _RandomEventsProps;

typedef int (*PropsStockCallback)(ItemCfgDef *arg);
typedef std::map<int, PropsStockCallback> PropsStockCB;
class PropsStock
{
public:
    void addCallBack(EnumItemPropsType attrType, PropsStockCallback cb);
    // 获取剩余量
    int handle(ItemCfgDef *cfg);
	// 检查是否需要库存处理
	bool isStock(ItemCfgDef *cfg);
	
    static void initPropsStock();
private:
    PropsStockCB mPsMap;
};
PropsStock _PropsStock;

bool RandomEventsConfig::insertData(string data)
{
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(data, value)) {
        return false;
    }
    itemId = value["id"].asInt();
    dayNum = value["day"].asInt();
    allNum = value["all"].asInt();
    
    return true;
}

string RandomEventsConfig::formatData()
{
    Json::Value value;
    value["id"] = itemId;
    value["day"] = dayNum;
    value["all"] = allNum;
    return Json::FastWriter().write(value);
}

void RandomEvents::refresh()
{
    map<int, RandomEventsConfig>::iterator it;
    for (it = mGoodsRecord.begin(); it != mGoodsRecord.end(); it++) {
        it->second.dayNum = 0;
        saveRandomEventsConfig(&it->second);
    }
}

RandomEvents::RandomEvents(ItemCfgDef *icDef, PromotionActivityDef *paDef)
{
    mIcDef = icDef;
    mGoodsRecord.clear();
    
    RedisResult result(redisCmd("hvals randomEvents:%d", mIcDef->itemid));
    int elementnum = result.getHashElements();
    for (int i = 0; i < elementnum; i++) {
        RandomEventsConfig rec;
        if (!rec.insertData(result.readHash(i, ""))) {
            continue;
        }
        mGoodsRecord.insert(make_pair(rec.itemId, rec));
    }
    
    // 加载每天上限配置
    {
        StringTokenizer token(paDef->ReadStr("toplimit", ""), ";");
        for (int i=0; i<token.count(); i++) {
            StringTokenizer temp(token[i], "=");
            if (temp.count() < 2) {
                xyerr("[随机礼包] ItemCfgDef:%d 配置的天上限规则有误", icDef->itemid);
                continue;
            }
            RandomEventsConfig *rec = CreateGoodsRecord(Utils::safe_atoi(temp[0].c_str()));
            rec->sysDayNum = Utils::safe_atoi(temp[1].c_str());
        }
    }
    // 加载活动总上限配置
    {
        StringTokenizer token(paDef->ReadStr("maxtoplimit", ""), ";");
        for (int i=0; i<token.count(); i++) {
            StringTokenizer temp(token[i], "=");
            if (temp.count() < 2) {
                xyerr("[随机礼包] ItemCfgDef:%d 配置的总上限规则有误", icDef->itemid);
                continue;
            }
            RandomEventsConfig *rec = CreateGoodsRecord(Utils::safe_atoi(temp[0].c_str()));
            rec->sysAllNum = Utils::safe_atoi(temp[1].c_str());
        }
    }
}

bool RandomEvents::isLimit(int itemId)
{
    RandomEventsConfig *rec = getGoodsRecord(itemId);
    if (NULL == rec) {
        return false;
    }
    if (rec->sysDayNum != 0 && rec->dayNum >= rec->sysDayNum) {
        return true;
    }
    if (rec->sysAllNum != 0 && rec->allNum >= rec->sysAllNum) {
        return true;
    }
    return false;
}

RandomEventsConfig * RandomEvents::getGoodsRecord(int itemId)
{
    map<int, RandomEventsConfig>::iterator it = mGoodsRecord.find(itemId);
    if (it != mGoodsRecord.end()) {
        return &it->second;
    }
    return NULL;
}

RandomEventsConfig * RandomEvents::CreateGoodsRecord(int itemId)
{
    map<int, RandomEventsConfig>::iterator it = mGoodsRecord.find(itemId);
    if (it != mGoodsRecord.end()) {
        return &it->second;
    }
    
    RandomEventsConfig rec(itemId);
    mGoodsRecord.insert(make_pair(itemId, rec));
    
    return &mGoodsRecord[itemId];
}

void RandomEvents::saveRandomEventsConfig(RandomEventsConfig *rec)
{
    RedisResult result(redisCmd("hmset randomEvents:%d %d %s", mIcDef->itemid, rec->itemId, rec->formatData().c_str()));
}

void RandomEvents::checkItemSave(string items)
{
	StringTokenizer token(items, ";");
	for (StringTokenizer::Iterator it = token.begin(); it != token.end(); it++)
	{
		ItemCfgDef *cfg = getItemCfg(*it);
		if (cfg == NULL) {
			return;
		}
		RandomEventsConfig *rec = getGoodsRecord(cfg->itemid);
		if (NULL == rec) {
			return;
		}
		// 不限制则忽略
		if (rec->sysDayNum == 0 && rec->sysAllNum == 0) {
			return;
		}
		rec->dayNum += 1;
		rec->allNum += 1;
		saveRandomEventsConfig(rec);
		
		Xylog log(eLogName_RandomLimitNum, 0);
		log<<rec->itemId<<rec->dayNum<<rec->allNum;
	}
}

string RandomEvents::randGoods()
{
    vector<int> props;
    vector<string> rewards;
    int randNum = mIcDef->ReadInt("rewardnum", 0);
    for (int i = 0; i < randNum; i++)
    {
        char rewardbuf[32];
        sprintf(rewardbuf, "rewards%d", i + 1);
        string reward = mIcDef->ReadStr(rewardbuf);
        rewards.push_back(reward);
        // 配置的奖励不存在(注意物品不存在,也要补0,因为占了一个位置)
        ItemCfgDef *cfg = getItemCfg(reward);
        if (NULL == cfg) {
            xyerr("[随机礼包] ItemCfgDef %s 配置的奖励不存在", reward.c_str());
            props.push_back(0);
            continue;
        }
        // 库存量
        if (_PropsStock.isStock(cfg) && 0 == _PropsStock.handle(cfg)) {
            props.push_back(0);
            continue;
        }
        // 达到上限
        if (isLimit(cfg->itemid)) {
            props.push_back(0);
            continue;
        }
        char propbuf[32];
        sprintf(propbuf, "pro%d", i + 1);
        int prop = mIcDef->ReadInt(propbuf);
        props.push_back(prop);
    }
    int index = getRand(props.data(), props.size());
	string items = rewards[index];
    // 检查保存抽中物品的天上限数与总上限数
    checkItemSave(items);
    return items;
}

// items:宝箱抽中的item串,　icDef:活动宝箱
void RandomEventsSys::physicalProc(Role *role, string items, ItemCfgDef *box)
{
	StringTokenizer token(items, ";");
	for (StringTokenizer::Iterator it = token.begin(); it != token.end(); it++)
	{
		ItemCfgDef *cfg = getItemCfg(*it);
		if (cfg == NULL) {
			return;
		}
		if (cfg->ReadInt("type") != KItemProps) {
			return;
		}
		_RandomEventsProps.handle((EnumItemPropsType)cfg->ReadInt("attr_type"), role, cfg, box);
	}
}

string RandomEventsSys::getRandBox(ItemCfgDef *icDef, Role *role)
{
    string items = "";
    PromotionActivityDef *paDef = PromotionActivityCfg::getDef(icDef->itemid);
    if (paDef == NULL) {
        xyerr("[随机礼包] %d 系统限制配置为空", icDef->itemid);
        return items;
    }
    // 检查过期
	int endTime = Utils::parseDate(paDef->ReadStr("validity","").c_str());
    if (timeIsExpired(endTime)) {
        ItemCfgDef* cfg = ItemCfg::getCfg(paDef->ReadInt("beyond_validity", 0));
        if (cfg == NULL) {
            return items;
        }
        items = getRandBoxItems(cfg);
    }
    else {
        RandomEvents *re = getRandonEvents(icDef->itemid);
        if (NULL == re)
        {
            RandomEvents temp(icDef, paDef);
            mRandomEventsMap.insert(make_pair(icDef->itemid, temp));
            re = getRandonEvents(icDef->itemid);
        }
        // 随机抽取
        items = re->randGoods();
        // 奖品处理
        physicalProc(role, items, icDef);
    }
	// 写奖励日志
	Xylog log(eLogName_RandomEvents, role->getInstID());
	log<<items;
	
    return items;
}

bool RandomEventsSys::timeIsExpired(int endTime)
{
    if (endTime - time(NULL) > 0) {
        return false;
    }
    return true;
}

void RandomEventsSys::refresh()
{
    RandomEventsTd::iterator it;
    for (it = mRandomEventsMap.begin(); it != mRandomEventsMap.end(); it++) {
        it->second.refresh();
    }
}

RandomEvents * RandomEventsSys::getRandonEvents(int id)
{
    RandomEventsTd::iterator it = mRandomEventsMap.find(id);
    if (it == mRandomEventsMap.end())
    {
        return NULL;
    }
    return &it->second;
}


// #####################################

bool sendPhoneCardMail(Role *role, ItemCfgDef *cfg, string mailStr)
{
	RedisResult result(redisCmd("lpop phoneCard:%d", cfg->itemid));
    string str = result.readStr();
    if (str.empty()) {
        xyerr("[随机礼包][电话卡] phoneCard:%d 为空 role:%d", cfg->itemid, role->getInstID());
        return false;
    }
    StringTokenizer token(str, ";");
    if (token.count() < 2) {
        xyerr("[随机礼包][电话卡] phoneCard:%d 存储值%s有误 role:%d", cfg->itemid, str.c_str(), role->getInstID());
        return false;
    }
	
	Xylog log(eLogName_RandomEvents,role->getInstID());
    log<<"[电话卡]"<<cfg->itemid<<str<<role->getInstID();

	char title[128] = "";
    char content[1024] = "";
    MailFormat *mailCfg = MailCfg::getCfg(mailStr.c_str());
	if (NULL == mailCfg) {
		xyerr("[随机礼包] %s 邮件配置有误", mailStr.c_str());
		return false;
	}
    
    string mail_title = mailCfg->title;
    string mail_content = mailCfg->content;
    
    find_and_replace(mail_title, 1, cfg->ReadStr("par_value").c_str());
    find_and_replace(mail_content, 2, token[0].c_str(), token[1].c_str());
	
    /*
    snprintf(title,
             sizeof(title),
             mailCfg->title.c_str(),
             cfg->ReadStr("par_value").c_str());
    
    snprintf(content,
             sizeof(content),
             mailCfg->content.c_str(),
             token[0].c_str(),
             token[1].c_str());
     */
    // 发邮件
    sendMail(0,
             mailCfg->sendername.c_str(),
             role->getRolename().c_str(),
             mail_title.c_str(),
             mail_content.c_str(),
             "",
             "");
	return true;
}

// 移动
static void RandomEventsPhoneCard(Role *role, ItemCfgDef *cfg, ItemCfgDef *box)
{
	if (!sendPhoneCardMail(role, cfg, "card_mail1")) {
		return;
	}
    
    // 发公告
	SystemBroadcast bc;
    /*
	bc.append(COLOR_WHITE, "恭喜");
	bc.append(COLOR_ROLE_NAME, role->getRolename().c_str());
	bc.append(COLOR_WHITE, "使用%s获得了[", box->ReadStr("name").c_str());
	bc.append(COLOR_QUA_GOLDEN, "%d元中国移动充值卡", cfg->ReadInt("par_value"));
	bc.append(COLOR_WHITE, "]，真是好运气啊!");
     */
}

//　电信
static void RandomEventsTelecomPhoneCard(Role *role, ItemCfgDef *cfg, ItemCfgDef *box)
{
	if (!sendPhoneCardMail(role, cfg, "card_mail2")) {
		return;
	}
    
    // 发公告
	SystemBroadcast bc;
    /*
	bc.append(COLOR_WHITE, "恭喜");
	bc.append(COLOR_ROLE_NAME, role->getRolename().c_str());
	bc.append(COLOR_WHITE, "使用%s获得了[", box->ReadStr("name").c_str());
	bc.append(COLOR_QUA_GOLDEN, "%d元中国电信充值卡", cfg->ReadInt("par_value"));
	bc.append(COLOR_WHITE, "]，真是好运气啊!");
     */
    
}

//　联通
static void RandomEventsUnicomPhoneCard(Role *role, ItemCfgDef *cfg, ItemCfgDef *box)
{
	if (!sendPhoneCardMail(role, cfg, "card_mail3")) {
		return;
	}
    
    // 发公告
	SystemBroadcast bc;
    /*
	bc.append(COLOR_WHITE, "恭喜");
	bc.append(COLOR_ROLE_NAME, role->getRolename().c_str());
	bc.append(COLOR_WHITE, "使用%s获得了[", box->ReadStr("name").c_str());
	bc.append(COLOR_QUA_GOLDEN, "%d元中国联通充值卡", cfg->ReadInt("par_value"));
	bc.append(COLOR_WHITE, "]，真是好运气啊!");
     */

}

static void RandomEventsMini(Role *role, ItemCfgDef *cfg, ItemCfgDef *box)
{
    MailFormat *mailCfg = MailCfg::getCfg("iphone_mail");
    if (mailCfg)
    {
    // 发邮件
    sendMail(0,
             mailCfg->sendername.c_str(),
             role->getRolename().c_str(),
             mailCfg->title.c_str(),
             mailCfg->content.c_str(),
             "",
             "");
    }
    // 发公告
	SystemBroadcast bc;
    /*
	bc.append(COLOR_WHITE, "恭喜");
	bc.append(COLOR_ROLE_NAME, role->getRolename().c_str());
	bc.append(COLOR_WHITE, "使用%s获得了[", box->ReadStr("name").c_str());
	bc.append(COLOR_QUA_GOLDEN, "ipad mini");
	bc.append(COLOR_WHITE, "]，真是好运气啊!");
     */
}


static void RandomEventsTickets(Role *role, ItemCfgDef *cfg, ItemCfgDef *box)
{
    MailFormat *mailCfg = MailCfg::getCfg("ticket_mail");
    if (mailCfg)
    {
    // 发邮件
    sendMail(0,
             mailCfg->sendername.c_str(),
             role->getRolename().c_str(),
             mailCfg->title.c_str(),
             mailCfg->content.c_str(),
             "",
             "");
    }
    // 发公告
	SystemBroadcast bc;
    /*
	bc.append(COLOR_WHITE, "恭喜");
	bc.append(COLOR_ROLE_NAME, role->getRolename().c_str());
	bc.append(COLOR_WHITE, "使用%s获得了[", box->ReadStr("name").c_str());
	bc.append(COLOR_QUA_GOLDEN, "回家飞机票");
	bc.append(COLOR_WHITE, "]，真是好运气啊!");
     */
}

void RandomEventsProps::initRandomEventsProps()
{
    _RandomEventsProps.addCallBack(KItemPropsPhoneCard, RandomEventsPhoneCard);
    _RandomEventsProps.addCallBack(KItemPropsPhone, RandomEventsMini);
	_RandomEventsProps.addCallBack(KItemPropsTelecomPhoneCard, RandomEventsTelecomPhoneCard);
	_RandomEventsProps.addCallBack(KItemPropsUnicomPhoneCard, RandomEventsUnicomPhoneCard);
	_RandomEventsProps.addCallBack(KItemPropsTickets, RandomEventsTickets);
}

void RandomEventsProps::addCallBack(EnumItemPropsType attrType, RandomEventsPropsCallback cb)
{
    assert(cb);
    
    RandomEventsPropsCB::iterator it = mRepMap.find(attrType);
    if (it != mRepMap.end()) {
        xyerr("[随机礼包][实物函数] attrType = %d 重复添加回调函数", attrType);
        return;
    }
    mRepMap.insert(make_pair(attrType, cb));
}

void RandomEventsProps::handle(EnumItemPropsType attrType, Role *role, ItemCfgDef *cfg, ItemCfgDef *box)
{
    RandomEventsPropsCB::iterator it = mRepMap.find(attrType);
    if(it == mRepMap.end() || NULL == it->second){
        xyerr("[随机礼包][实物函数] attrType = %d 没有回调函数", attrType);
        return;
    }
    it->second(role, cfg, box);
}


struct __RandomEventsProps
{
    __RandomEventsProps() {
        RandomEventsProps::initRandomEventsProps();
    }
}__tmp_RandomEventsProps;

// ##################################################

static int PropsStockPhoneCard(ItemCfgDef *cfg)
{
    RedisResult result(redisCmd("llen phoneCard:%d", cfg->itemid));
    return result.readInt();
}

void PropsStock::initPropsStock()
{
    _PropsStock.addCallBack(KItemPropsPhoneCard, PropsStockPhoneCard);
}

void PropsStock::addCallBack(EnumItemPropsType attrType, PropsStockCallback cb)
{
    assert(cb);
    PropsStockCB::iterator it = mPsMap.find(attrType);
    if (it != mPsMap.end()) {
        return;
    }
    mPsMap.insert(make_pair(attrType, cb));
}

int PropsStock::handle(ItemCfgDef *cfg)
{
    PropsStockCB::iterator it = mPsMap.find(cfg->ReadInt("attr_type"));
    if(it == mPsMap.end() || NULL == it->second){
        return 0;
    }
    return it->second(cfg);
}

bool PropsStock::isStock(ItemCfgDef *cfg)
{
	PropsStockCB::iterator it = mPsMap.find(cfg->ReadInt("attr_type"));
    if(it == mPsMap.end()){
        return false;
    }
	return true;
}

struct __PropsStock
{
    __PropsStock() {
        PropsStock::initPropsStock();
    }
}__tmp_PropsStock;


