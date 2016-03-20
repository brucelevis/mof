//
//  sign_hand.cpp
//  GameSrv
//
//  Created by cxy on 13-1-10.
//
//

#include "hander_include.h"
#include "DBRolePropName.h"
#include "RoleLoginDaysAward.h"
#include "HttpRequestConfig.h"
#include "datetime.h"

const int  MAX_LOTTERY_HISTORY  = 30;

hander_msg(req_loginaward, req)
{

    hand_Sid2Role_check(sessionid, roleid, role)
    ack_loginaward ack;
    ack.errorcode = CE_BAG_FULL;
    do
    {
        
        int loginaward = role->getLoginAward();

        time_t now = time(NULL);
        tm nowtm;
        localtime_r(&now, &nowtm);

        int mask = 1 << nowtm.tm_wday;
        if (loginaward & mask)
        {
            log_error("req_loginaward already get the award");
            break;
        }

        //Get Award
        loginaward |= mask;
        int days = Utils::numOfOne(loginaward, 0, 7);
        
        int secs = LoginAwardCfg::getAfterOpenServerDays() * SECONDS_PER_DAY;
        int openServerTime = Process::env.getInt("starttime", 0); //开服时间
        int benchmarkTime = secs + openServerTime;
        int current = time(NULL);
        
        int packageid;
        
        if(current > benchmarkTime){
            packageid = LoginAwardCfg::getAwardId(days); //有运营活动时的每日登陆奖励
        } else {
            packageid = LoginAwardCfg::getNormalAwardId(days); //没有运营活动时的每日登陆奖励

        }
        
        if (packageid > 0)
        {
            ItemArray items;
            GridArray effgrids;
            items.push_back(ItemGroup(packageid, 1));
            
            //wangzhigang 2014-11-19
//            if (!BackBag::PreAddItems(roleid, items, effgrids))
            if (role->preAddItems(items, effgrids) != CE_OK)
            {
                break;
            }

            char buf[32] = "";
            sprintf(buf, "login_award:%d", days);
            //LogMod::addLogGetItem(roleid, role->getRolename().c_str(), role->getSessionId(), items, buf);
//            role->updateBackBag(effgrids, items, true, buf);
            role->playerAddItemsAndStore(effgrids, items, buf, true);
        }

        role->setLoginAward(loginaward);
        role->saveProp(GetRolePropName(eRoleLoginAward), Utils::itoa(loginaward).c_str());

        ack.errorcode = CE_OK;
        ack.days = days;
        
        Xylog log(eLogName_Loginaward, role->getInstID());
		log << days;
    }
    while (0);

    sendNetPacket(sessionid, &ack);
}}

void sendLoginAwardState(Role* role)
{
    int loginstate = role->getLoginAward();
    int vipstate = role->getVipAward();

    int days = Utils::numOfOne(loginstate, 0, 7);

    int todaygift = 0;
    time_t now = time(NULL);
    tm nowtm;
    localtime_r(&now, &nowtm);
    if (loginstate & (1 << nowtm.tm_wday))
    {
        todaygift = 1;
    }
    
  
    int openServerDays = Process::env.getInt("starttime", 0);
    int afterOpenServerDays = LoginAwardCfg::getAfterOpenServerDays();
    int benchmarkTime = openServerDays + afterOpenServerDays * SECONDS_PER_DAY;
    vector<int> items;
    
    if(now > benchmarkTime){
        items = LoginAwardCfg::getActivityAwards();
    } else {
        items = LoginAwardCfg::getNormalAwards();
    }
    
    ack_loginawardstate ack;
    ack.days = days;
    ack.todaygift = todaygift;
    ack.vipgift = vipstate;
    ack.giftsFor7Days = items;
    sendNetPacket(role->getSessionId(), &ack);
}

hander_msg(req_loginawardstate, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    sendLoginAwardState(role);
}}

hander_msg(req_vipaward, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    ack_vipaward ack;
    ack.errorcode = CE_OK;
    do
    {
        int viplvl = role->getVipLvl();
        if (viplvl <= 0)
        {
//            log_error("req_vipaward not vip");
            ack.errorcode = CE_YOU_ARE_NOT_VIP;
            break;
        }

        int isgot = role->getVipAward();
        if (!isgot)
        {
            log_error("already get the vip gift");
            ack.errorcode = CE_YOU_HAVE_GET_THIS_VIPLVL_AWARD;
            break;
        }

        //Get Award
        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(viplvl);
        if (vipcfg == NULL)
        {
            ack.errorcode = CE_READ_VIPCFG_ERROR;
            break;
        }

        GridArray effgrids;
        ItemArray items;
        items.push_back(ItemGroup(vipcfg->giftPackage, 1));
        
        //wangzhigang 2014-11-19
//            if (!BackBag::PreAddItems(roleid, items, effgrids))
        if (role->preAddItems(items, effgrids) == CE_OK)
        {
            ack.errorcode = CE_OK;
//            role->updateBackBag(effgrids, items, true, "vipdayaward");
            role->playerAddItemsAndStore(effgrids, items, "vipdayaward", true);
            role->setVipAward(0);
            role->saveProp(GetRolePropName(eRoleVipAward), "0");
            ack.lvl = viplvl;
			
			Xylog log(eLogName_Vipaward, role->getInstID());
			log << viplvl;
        }
        else{
            ack.errorcode = CE_BAG_FULL;
        }
    }
    while (0);

    sendNetPacket(sessionid, &ack);
}}

handler_msg(req_getviplvlAward, req)
{
    hand_Sid2Role_check(sessionid, roleid, self)
    ack_getviplvlAward ack;
    ack.errorcode = CE_OK;
    do
    {
        if (self->getVipLvl() == 0) {
            ack.errorcode = CE_YOU_ARE_NOT_VIP;
            break;
        }
        if (self->getVipLvl() < req.viplvl) {
            ack.errorcode = CE_YOUR_VIPLVL_IS_LOWER;
            break;
        }

        if ( !self->checkVipLvlAward(req.viplvl) ) {
            ack.errorcode = CE_YOU_HAVE_GET_THIS_VIPLVL_AWARD;
            break;
        }
        //Get Award
        VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(req.viplvl);
        if (vipcfg == NULL)
        {
            ack.errorcode = CE_READ_VIPCFG_ERROR;
            break;
        }

        GridArray effgrids;
        ItemArray items;
        items.push_back(ItemGroup(vipcfg->viplvlAward, 1));
        
        //wangzhigang 2014-11-19
//            if (!BackBag::PreAddItems(roleid, items, effgrids))
        if (self->preAddItems(items, effgrids) == CE_OK)
        {

            char buf[32] = "";
            sprintf(buf, "vip_lvl_award:%d", req.viplvl);
            //LogMod::addLogGetItem(roleid, self->getRolename().c_str(), self->getSessionId(), items, buf);
//            self->updateBackBag(effgrids, items, true, buf);
            self->playerAddItemsAndStore(effgrids, items, buf, true);
        }
        else
        {
            ack.errorcode = CE_BAG_FULL;
            break;
        }
    }while (false);

    sendNetPacket(sessionid, &ack);

    if (ack.errorcode == CE_OK) {
        self->updateVipLvlAward(req.viplvl, false);
    }
}}

hander_msg(req_moneytreestate, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    int optimes = role->getMoneyTreeTimes();

    ack_moneytreestate ack;
    ack.errorcode = 0;
    ack.times = optimes;
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_moneytreeop, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    int optimes = role->getMoneyTreeTimes();
    int freetimes = MoneyTreeCfg::getFreeTimes(role->getLvl());
    ack_moneytreeop ack;
    ack.errorcode = CE_OK;
    //int viplvl = role->getVipLvl();
    //VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(viplvl);
    //if (vipcfg != NULL)
    //{
    //    freetimes += vipcfg->moneyTreeTimes;
    //}

    do
    {
        if (optimes >= freetimes)
        {
            int rmboptimes = optimes - freetimes;
            VipPrivilegeCfgDef* vipData = VipPrivilegeCfg::getCfg(role->getVipLvl());
            if (vipData != NULL) {
                if (rmboptimes >= vipData->moneyTreeTimes) {
                    ack.errorcode = CE_MONEYTREE_RMB_OPEN_TIMES_USEUP;
                    break;
                }
            }
            int costrmb = MoneyTreeCfg::getConsRmb(optimes - freetimes + 1);
            if (role->getRmb() >= costrmb)
            {
                role->addRmb(-costrmb, "开宝箱");
            }
            else
            {
                ack.errorcode = CE_SHOP_RMB_NOT_ENOUGH;
                break;
            }
        }
    }while(false);

    if (ack.errorcode == CE_OK) {
        optimes++;

        int gold = MoneyTreeCfg::getGold(role->getLvl(), optimes);
        role->addGold(gold, "moneytree");

        role->setMoneyTreeTimes(optimes);
        role->saveProp(GetRolePropName(eRoleMoneyTreeTimes), Utils::itoa(optimes).c_str());
		
		Xylog log(eLogName_Moneytreeop, role->getInstID());
		log << optimes;
    }

    ack.times = optimes;
    sendNetPacket(sessionid, &ack);
}}

#pragma mark -
#pragma mark 色子抽奖

static void sendDiceLotteryState(Role* role ,bool sendlatest = false)
{
    ack_dice_lottery_state ack;
    ack.position = role->getDiceLotteryPosition();
    ack.lotterytime = role->getDiceLotteryTime();
    
    obj_lotteryhistory obj;

    IntMapJson::reverse_iterator iter = role->mDiceLotteryHistory.rbegin();
    for (; iter != role->mDiceLotteryHistory.rend(); ++iter)
    {
        obj.id = iter->first;
        const Json::Value& val = iter->second;
        
        obj.itemid = val["item"].asInt();
        obj.amount = val["amount"].asInt();
        obj.rmb = val["rmb"].asInt();
        obj.date = val["date"].asInt();
        
        ack.history.push_back(obj);
        
        if(sendlatest)
            break;
    }
    
    sendNetPacket(role->getSessionId(), &ack);
}

hander_msg(req_dice_lottery_state, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    sendDiceLotteryState(role);
}}


static void broadcastLottery(const char* activity, Role* role , int itemid , int amount);
static int randNextPosition(int position, int lvl);
static const DiceItem& randDiceItem(int position, int lvl);
static void createLotteryItem(Role* role, int itemtype , int itemid , int amount);
static void resetDiceStart(Role* role ,int needRmb);

hander_msg(req_dice_lottery, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_dice_lottery ack;
    ack.errorcode = eDiceLotteryReuslt_Success;
    
    // 前2次免费的
    int buytime = role->getDiceLotteryTime()  - DiceLotteryCfgMgr::getFreeTimes();
    
    int needCost = 0;   // 需要的钱，金钻或者金币，具体看配置表 
    if( buytime>=0 )
    {        
        needCost = role->hasEnoughMoney(eDiceLotteryBuy, buytime);//返回0表示不够钱
        send_if( 0 == needCost, eDiceLotteryReuslt_NeedMoreRmb);
    }
    
//    int emptyslot = BackBag::EmptyGridAmount(role->getInstID());
    int emptyslot = role->getBackBag()->EmptyGridAmount();
    send_if(emptyslot < 1 , eDiceLotteryReuslt_BagFull);
    
    
    ack.newposition = randNextPosition( role->getDiceLotteryPosition(), role->getLvl());
    
    string desc = GameTextCfg::getString("2003");
    
    if( buytime >= 0)
    {
        role->consumeMoney(eDiceLotteryBuy, buytime, desc.c_str());//"骰子游戏");
    }
    
    if(ack.newposition > DICE_MAX_GRID)
    {
        resetDiceStart(role, needCost);
        return;
    }
    
    DiceItem diceitem = randDiceItem(ack.newposition, role->getLvl());
    
    ItemCfgDef* cfg = ItemCfg::getCfg( diceitem.item );
    xyassertf(cfg,"%d",diceitem.item);
    
    createLotteryItem(role, diceitem.type , diceitem.item , diceitem.amount);
    
    if(diceitem.broadcast)
    {   //"骰子游戏"
        broadcastLottery(desc.c_str() ,role,diceitem.item,diceitem.amount);
    }
    
    role->setDiceLotteryPosition( ack.newposition );
    role->setDiceLotteryTime( role->getDiceLotteryTime() + 1);
    role->pushLotteryHistory(diceitem.item , diceitem.amount, needCost ,true);
    role->saveNewProperty();
    
    sendDiceLotteryState(role,true);
    sendNetPacket(sessionid, & ack );
    
    Xylog log(eLogName_DiceLottery,role->getInstID());
    log<<"rmb:"<<needCost<<"item:"<<cfg->getFullName()<<"newposition:"<<ack.newposition;
}}

static void resetDiceStart(Role* role , int needRmb)
{
    role->setDiceLotteryPosition( 0 );
    role->setDiceLotteryTime( 0 );
    role->saveNewProperty();
    sendDiceLotteryState(role,true);
    
    ack_dice_lottery ack;
    ack.errorcode = eDiceLotteryReuslt_Success;
    ack.newposition = 0;
    sendNetPacket(role->getSessionId(), & ack );
    
    Xylog log(eLogName_DiceLottery,role->getInstID());
    log<<"rmb:"<<needRmb<<"item:"<<0<<"newposition:"<<0;
}



// 发走马灯
void broadcastDiceLottery(Role* role , int item )
{
    ItemCfgDef* cfg = ItemCfg::getCfg( item );
    xyassert(cfg);
    
    int itemqua = cfg->ReadInt("qua", -1);
    string itemname = cfg->ReadStr("name");
    
    //恭喜xxxx在色子抽奖中获得 物品名(根据品质决定颜色)
    SystemBroadcast bc;
    /*
    bc.append(COLOR_WHITE, "恭喜");
    bc.append(COLOR_ROLE_NAME, role->getRolename().c_str());
    bc.append(COLOR_WHITE, "在色子抽奖中获得 ");
    bc.append(itemqua, itemname.c_str());
    */
    string str = SystemBroadcastCfg::getValue("get_lotterywin", "content");
    bc << str.c_str() << role->getRolename().c_str() << itemqua << itemname.c_str();
    bc.send();
}

// 随机出下一个位置
int randNextPosition(int position, int lvl)
{
    assert(position >= 0);
    vector<int> props;
    for (int i = 1; i <= 6; ++i)
    {
        props.push_back(DiceLotteryCfgMgr::getGrid(position + i, lvl).prop);
    }
    int index = getRand(props.data(), props.size());
    return position + index + 1;
}

// 根据格子随机出物品
const DiceItem& randDiceItem(int position, int lvl)
{
    int gridid = DiceLotteryCfgMgr::getGrid(position, lvl).gridid;
    const DiceGridItems& gridItems = DiceLotteryCfgMgr::getGridItems(gridid);
    
    vector<int> props;
    for (DiceGridItems::const_iterator it = gridItems.begin(); it != gridItems.end(); ++it)
    {
		if (it->lvlmin <= lvl && lvl <= it->lvlmax)
		{
			props.push_back(it->prop);
		}
		else
		{
			props.push_back(0);
		}
    }
    
    int index = getRand(props.data(), props.size());
    return gridItems[index];
}





#pragma mark -
#pragma mark 轮盘抽奖

static void sendLotteryState(Role* role ,bool sendlatest = false)
{
    ack_lotterystate ack;
    ack.freelottery = role->getLotteryFree();
    ack.buylottery = role->getLotteryRmb();
    ack.incr_lottery_times = role->getIncrLotteryTimes();
    
    obj_lotteryhistory obj;
    
    
    IntMapJson::reverse_iterator iter = role->mLotteryHistory.rbegin();
    
    for (; iter != role->mLotteryHistory.rend(); ++iter)
    {
        obj.id = iter->first;
        const Json::Value& val = iter->second;
        
        obj.itemid = val["item"].asInt();
        obj.amount = val["amount"].asInt();
        obj.rmb = val["rmb"].asInt();
        obj.date = val["date"].asInt();
        
        ack.history.push_back(obj);
        
        if(sendlatest)
            break;
    }
    
    sendNetPacket(role->getSessionId(), &ack);
}

hander_msg(req_lotterystate, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    sendLotteryState(role);
}}

/*
 抽奖的配置文件比较复杂
 lottery.ini            概率总控制，根据日期使用不同子表
 lottery_path1.ini ...  概率子表
 buy.ini                抽奖购买价格
 consumable.ini         消耗品
 vip.ini                VIP等级不同抽奖次数不同
 */

extern int useConsumeItem(ItemCfgDef* cfg, Role* role, int count, string *award /*=NULL*/ );


struct LotteryConditionCheck    //抽奖条件检测
{
    bool  isFree;     //是免费的
    int money; // 要多少钱
    int  itemamount;  //得到道具数量
};

// 抽奖条件检测
enLotteryReuslt lotteryConditionPass(Role* role  , LotteryConditionCheck & check)
{
    int roleid = role->getInstID();
    
    //判断是否有免费使用次数
    if (role->getIncrLotteryTimes() > 0)
    {
        check.isFree = true;
        check.money = 0;
        check.itemamount = 1;
        return enLotteryReuslt_Success;
    }
    
    check.isFree = true;
    check.money = 0;
    check.itemamount= 0;
    
    int buylottery = role->getLotteryRmb();
    int freelottery = role->getLotteryFree();
    
    const VipPrivilegeCfgDef* vipcfg = VipPrivilegeCfg::getCfg(role->getVipLvl());
    if( ! vipcfg )
    {
        xyerr("lotteryConditionPass vipcfg %d", role->getVipLvl());
        return enLotteryReuslt_UnknowError;
    }

    const LotItemVec* Items = LotteryCfgMgr::getItem( role->getLvl() );
    if( Items == NULL || Items->empty() )
    {
        xyerr("lotteryConditionPass Items %d", role->getLvl());
        return enLotteryReuslt_UnknowError;
    }
    
    
    //全部次数用完了
    int dayMaxTime = vipcfg->freelotterytime + vipcfg->buylotterytime;
    if(freelottery + buylottery >=  dayMaxTime){
        return enLotteryReuslt_TimesEmtpy;
    }
    
    const bool isFree = freelottery <  vipcfg->freelotterytime;
    
    check.isFree = isFree;
    check.itemamount = 1;
    
    // double item
    if( ! isFree ){
        float doublerate = BuyCfg::getDouLottery(buylottery);
        float randnum = range_randf(0.0, 1.0);

        if(randnum < doublerate){
            check.itemamount ++;
        }
    }
    
    // bag full
    //wangzhigang 2014-11-19
//    int emptyslot = BackBag::EmptyGridAmount(roleid);
    int emptyslot = role->getBackBag()->EmptyGridAmount();
    if(emptyslot < check.itemamount ){
        return  enLotteryReuslt_BagFull;
    }
    
    // need more rmb
    if( ! isFree )
    {
        check.money = role->hasEnoughMoney(eLotteryBuy, buylottery);
        if( 0 == check.money )
            return enLotteryReuslt_NeedMoreRmb;
    }

    return enLotteryReuslt_Success;
}

// 随机出物品
static void randLotteryItem(bool isFree , int rolelvl, LotteryItem& out)
{
    const LotItemVec* Items = LotteryCfgMgr::getItem( rolelvl );
    assert(Items);
    
    vector<int> props;//权重
    
    //根据是否免费，挑出物品, 这里可以提前分捡一下，不要这里再捡了 KCDO
    for(int i = 0 ,ni  = Items->size(); i<ni; ++i)
    {
        if(isFree && eLotteryItemFree ==  (*Items)[i].freeType )
            props.push_back( (*Items)[i].prob );
        
        if(isFree && eLotteryItemFree !=  (*Items)[i].freeType)
            props.push_back( 0 );
        
        if( !isFree && eLotteryItemNotFree ==  (*Items)[i].freeType)
            props.push_back( (*Items)[i].prob );
        
        if( !isFree && eLotteryItemNotFree !=  (*Items)[i].freeType)
            props.push_back( 0 );
    }
    
    int index = getRand(props.data(), props.size());//假设中奖
    
    out = (*Items)[index];
}

// 发走马灯
static void broadcastLottery(const char* activity, Role* role , int itemid , int amount)
{
    assert(activity && role);
    
    ItemCfgDef* cfg = ItemCfg::getCfg( itemid );
    xyassert(cfg);
    
    int itemqua = cfg->ReadInt("qua", -1);
    string itemname = cfg->ReadStr("name");
    
    //恭喜xxxx在xx中获得 物品名(根据品质决定颜色) x2(如果没有x2就不用写)
    SystemBroadcast bc;
    /*
    bc.append(COLOR_WHITE, "恭喜");
    bc.append(COLOR_ROLE_NAME, role->getRolename().c_str());
    bc.append(COLOR_WHITE, "在%s中获得 ",activity);
    bc.append(itemqua, itemname.c_str());
    if( amount >=2 )
        bc.append(COLOR_ORANGE, " x%d",amount);
     */
    string str = SystemBroadcastCfg::getValue("win_content", "content");
    bc << str.c_str() << role->getRolename().c_str() << activity <<  itemqua << itemname.c_str() << amount;
    bc.send();
}

// 发物品
static void createLotteryItem(Role* role, int itemtype , int itemid , int amount)
{
    xyassert( amount>0 );
    ItemCfgDef* cfg = ItemCfg::getCfg( itemid );
    if (!cfg)
    {
        xyerr("抽奖发放物品出现错误！");
        return ;
    }
    
    if (cfg->ReadInt("type") == kItemConsumable)
    {
        useConsumeItem(cfg, role, amount);
    }
    else
    {
        GridArray effgrids;
        ItemArray items;
        items.push_back(ItemGroup(itemid, amount));
        
        
//        xyassertf( BackBag::PreAddItems(role->getInstID(), items, effgrids) , "%d * % d", itemid ,amount);
        xyassertf( role->preAddItems(items, effgrids) == CE_OK , "%d * % d", itemid ,amount);

        
//        role->updateBackBag(effgrids, items, true, "dice_lottery");
        role->playerAddItemsAndStore(effgrids, items, "dice_lottery", true);
        //BackBag::UpdateBackBag(role->getInstID(), effgrids);//加进背包
		//LogMod::addLogGetItem(role->getInstID(), role->getRolename().c_str(), role->getSessionId(), items, "dice_lottery");
    }
}



// 写日志
static void writeLotteryLog(Role* role ,const ack_lottery& ack)
{
    xyassert( ack.items.size() > 0 );
    
    ItemCfgDef* cfg = ItemCfg::getCfg(ack.items[0]);
    xyassertf(cfg,"%d", ack.items[0]);
    
    Xylog log(eLogName_Lottery,role->getInstID() );
    log<<ack.costrmb<<ack.items.size()<< cfg->getFullName();
}


//尝试抽一次
enLotteryReuslt lotteryOnceSucc(Role* role , ack_lottery& ack)
{
    assert(role);
    
    LotteryConditionCheck check;
    
    enLotteryReuslt ret  = lotteryConditionPass(role,check);
    if( enLotteryReuslt_Success != ret )
        return ret;
    
    if( check.isFree)
    {
        if (role->getIncrLotteryTimes() > 0)
        {
            role->setIncrLotteryTimes(role->getIncrLotteryTimes() - 1);
            role->saveNewProperty();
        }
        else
        {
            role->setLotteryFree( role->getLotteryFree() + 1);
        }
    }
    
    if( ! check.isFree )
    {
        role->consumeMoney(eLotteryBuy, role->getLotteryRmb(), "Lottery");
        role->setLotteryRmb( role->getLotteryRmb() + 1);
    }
    
    LotteryItem lotteryitem;
    randLotteryItem( check.isFree , role->getLvl() ,lotteryitem) ;
    
    ack.errorcode = enLotteryReuslt_Success;
    ack.costrmb = check.money;
    ack.costType = eRmbCost;
    ack.itemtype = lotteryitem.type;
    ack.items = vector<int>(check.itemamount, lotteryitem.id);
    
    role->pushLotteryHistory(lotteryitem.id, check.itemamount, check.money);
    
    createLotteryItem(role , lotteryitem.type,lotteryitem.id , check.itemamount);
    
    string activityname = GameTextCfg::getString("2004");
    
    //走马灯
    if(lotteryitem.broadcast)//"幸运抽奖"
        broadcastLottery(activityname.c_str(),role, lotteryitem.id, check.itemamount);
    
    writeLotteryLog(role,ack);

    return enLotteryReuslt_Success;
}

void onReqLottery(Role* role , int maxAmount)
{
    ack_n_lottery rotate;
    
    enLotteryReuslt errcode = enLotteryReuslt_UnknowError;
    
    vector<ack_lottery> succ;
    
    for (int i = 0; i< maxAmount; ++i)
    {
        ack_lottery ack;
        
        errcode = lotteryOnceSucc(role ,ack);
        
        if( enLotteryReuslt_Success != errcode )
            break;
        
        rotate.lastItemType = ack.itemtype;
        rotate.items.push_back( ack.items[0] );
        
        succ.push_back(ack);
    }
    
    role->saveNewProperty();
    
    if( succ.empty() )
    {
        rotate.err = errcode;
        return sendNetPacket(role->getSessionId(), & rotate);
    }
    
    rotate.err = enLotteryReuslt_Success;
    rotate.succTime = succ.size();
    sendNetPacket(role->getSessionId(), & rotate);
    
    sendLotteryState(role);
}

extern void lotteryAwardActivity(int roleid ,int beforeRmbTime , int afterRmbTime);

//抽奖
hander_msg(req_lottery, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)

    int beforeRmbTime = role->getLotteryRmb();
    
    onReqLottery(role,1);
    
    int afterRmbTime = role->getLotteryRmb();
    lotteryAwardActivity(roleid,beforeRmbTime,afterRmbTime);
}}

//一次请求n次抽奖，最大10
hander_msg(req_n_lottery, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    //uint64_t before = Timestamp::now_us ();
    
    int beforeRmbTime = role->getLotteryRmb();
    
    onReqLottery(role,10);
    
    int afterRmbTime = role->getLotteryRmb();
    lotteryAwardActivity(roleid,beforeRmbTime,afterRmbTime);
    
//    uint64_t after = Timestamp::now_us ();
//    
//    xyerr("抽奖用时 %d us", after- before);
}}


#pragma mark -
#pragma mark 激活码

// 激活码使用
hander_msg(req_activation, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
	
	// 背包空格子验证
	if (role->getBackBag()->EmptyGridAmount() < 2) {
		ack_activation ack;
		ack.errorcode = enActivationReuslt_BagFull;
		role->send(&ack);
		return;
	}
	
    // 请求验证页面
    std::ostringstream oss;
    oss<<"code="<<req.code;
    oss<<"&pf="<<SSessionDataMgr.getProp(sessionid, "platform");
	string os = SSessionDataMgr.getProp(sessionid, "os");
	oss<<"&os="<<(os.empty() ? "ios" : os);
    oss<<"&server="<<Process::env.getInt("server_id");
    oss<<"&roleId="<<roleid;

    create_cmd(http_request, http_request);
    http_request->fun_name = HTTP_ACTIVATION;
    http_request->url = UrlCfg::get_activation_url();//ACTIVATION_URL;
    http_request->post_param = oss.str();
    http_request->roleid = roleid;
	http_request->sessionid = sessionid;
    sendCmdMsg(HttpRequest::MQ, http_request);
}}

static int getKrLvAwardState(Role* role)
{
    bool noAward = GameFunctionCfg::getGameFunctionCfg().krLvlRewardType != eKrLvlRewardTyp;
    if(noAward)
        return eKrLvAwardResult_NoOpen;
    
    if( role->getLvl() < KoreaLevelAwardCfg::mLevel )
        return eKrLvAwardResult_NoLvl;
    
    if( role->getKoreaLevelAward()  )
        return eKrLvAwardResult_AlreadyGet;
    
    if( KoreaLevelAwardCfg::mAward.empty() )
        return eKrLvAwardResult_Error;
    
    vector<string> awards = StrSpilt(KoreaLevelAwardCfg::mAward.c_str(), ";");
//    int emptyslot = BackBag::EmptyGridAmount(role->getInstID());
    int emptyslot = role->getBackBag()->EmptyGridAmount();
    if( emptyslot < awards.size() )
        return eKrLvAwardResult_Error;
    
    return eKrLvAwardResult_Success;
}

// 韩国10级留言奖励
hander_msg(req_KrLvAward, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_KrLvAward ack;
    
    ack.errorcode = getKrLvAwardState(role);
    if( ack.errorcode != eKrLvAwardResult_Success)
    {
        role->send(&ack);
		return;
    }
    
    bool sendAwardSuccess = false;
    int packageid = Utils::safe_atoi(KoreaLevelAwardCfg::mAward.c_str(), 0);
    do {
        
        if (packageid <= 0)
        {
            break;
        }
        
        ItemArray items;
        GridArray effgrids;
        items.push_back(ItemGroup(packageid, 1));
        
        //wangzhigang 2014-11-19
//        if (!BackBag::PreAddItems(roleid, items, effgrids))
        if (role->preAddItems(items, effgrids))
        {
            break;
        }
//        role->updateBackBag(effgrids, items, true, "韩国留言奖励");
        role->playerAddItemsAndStore(effgrids, items, "韩国留言奖励", true);
        //BackBag::UpdateBackBag(roleid, effgrids);
        //LogMod::addLogGetItem(roleid, role->getRolename().c_str(), role->getSessionId(), items, "韩国留言奖励");
        sendAwardSuccess = true;
        
    } while (0);


    
    //vector<string> awards = StrSpilt(KoreaLevelAwardCfg::mAward.c_str(), ";");
    //ItemArray items;
    // =  role->addAwards(awards, items, "韩国留言奖励");
    send_if( !sendAwardSuccess , eKrLvAwardResult_Error); // 其它错误
    
    role->setKoreaLevelAward(1);
    role->saveNewProperty();
    
    send_if(true, eKrLvAwardResult_Success);
}}


hander_msg(req_KrLvAwardState, req)
{
    hand_Sid2Role_check(sessionid, roleid, role)
    
    ack_KrLvAwardState ack;
    
    if( getKrLvAwardState(role) ==  eKrLvAwardResult_Success )
    {
        ack.errorcode = 0;
    }else{
        ack.errorcode = 1;
    }
    
    role->send(&ack);
}}

hander_msg(req_daily_get_fat_status, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_daily_get_fat_status ack;
    ack.errorcode = eQueenBlessStatusType_Ok;
    ack.index_str = role->getQueenBless();
    ack.status = 0;
    time_t now = time(NULL);
    int mAward = QueenBlessCfg::getQueenBlessFat(now, role->getVipLvl());
    ack.award = mAward;
    
    QueenBlessTimeItem *item = QueenBlessCfg::isDuringTime(time(NULL));
    if (item == NULL)
    {
        role->send(&ack);
        return ;
    }
    string str = Utils::itoa(item->index);
    if (string::npos == ack.index_str.find(str))
    {
        ack.status = 1;
        role->send(&ack);
        return ;
    }
    role->send(&ack);
}}


hander_msg(req_daily_get_fat, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_daily_get_fat ack;
    
    time_t now = time(NULL);
    QueenBlessTimeItem *item = QueenBlessCfg::isDuringTime(now);
    //不在活动时间
    if (!item)
    {
        ack.errorcode = eQueenBlessGetType_NotDuringTime;
        role->send(&ack);
        return ;
    }
    
    //精力已经到达上限
    role->calcMaxFatLimit();
    if (role->getFat() >= role->getMaxFat())
    {
        ack.errorcode = eQueenBlessGetType_FatIsMax;
        role->send(&ack);
        return ;
    }
    
    int index = item->index;
    
    string record = role->getQueenBless();
    vector<string> records = StrSpilt(record, ";");
    vector<int> indexs;
    strVecToNumVec(records, indexs);
    if (count(indexs.begin(), indexs.end(), index))
    {
        ack.errorcode = eQueenBlessGetType_isAlreadyGet;
        ack.index = 0;
        role->send(&ack);
        return ;
    }
    
    int mAward = QueenBlessCfg::getQueenBlessFat(now, role->getVipLvl());
    ack.errorcode = eQueenBlessGetType_Ok;
    ack.index = item->index;
    indexs.push_back(item->index);
    string status = stringjoint<int>(indexs, ";");
    role->setQueenBless(status);
    role->saveNewProperty();
    role->addFat(mAward);

    role->send(&ack);

    Xylog log(eLogName_QueenBless, role->getInstID());
    string timeDuring = Utils::makeStr("%d:%d-%d:%d", item->starttime.tm_hour, item->starttime.tm_min,
                                 item->endtime.tm_hour, item->endtime.tm_min);
    log << item->index << timeDuring << mAward;
    
}}


#pragma mark ------ 角色奖励 ---------
hander_msg(req_role_award_state, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    role->getRoleLoginDaysAwardMgr()->ackRoleAwardState();
}}

hander_msg(req_role_award, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    role->getRoleLoginDaysAwardMgr()->ackRoleAwardGet(req.days);
}}










