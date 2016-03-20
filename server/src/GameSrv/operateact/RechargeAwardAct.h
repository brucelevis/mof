//
//  RechargeAwardAct.h
//  GameSrv
//
//  Created by jin on 14-9-25.
//
//

#ifndef GameSrv_RechargeAwardAct_h
#define GameSrv_RechargeAwardAct_h

struct RechardAwardItem
{
    int rmbNeed;    // 需要充值
    float rmbWard;  // 奖励金钻比例
    string items;   // 奖励物品
};

struct RechargeAct  // 一场活动
{
    int beginTime;  // 开始时间
    int endTime;    // 结束时间
    int afterOpenServerDays; //开服多少天
    int mIndex;     //唯一标识
    vector<RechardAwardItem>   awards; // 奖励内容
	
	string mTitle;
	string mContent;
	int mAwardType;
	string mChannel;
	string mServerId;
};


typedef vector<RechardAwardItem>::reverse_iterator AwardItemRiter;
typedef vector<RechargeAct>::iterator RechargeActIter;

class RechargeAwardActivity
{
public:
    void init();    // 读配置
    void award(int roleid,const char* rolename, int rmb , bool isOnline); // 发奖
    void sendStatus(int roleid); // 发送当前状态
    void getAward(int roleid, int time); // 领奖
	
	vector<RechargeAct> & getActs() { return mActs; }
protected:
    vector<RechargeAct> mActs;  // 不同时间段的活动
    bool sendAward(Role* role ,  const string& award);
    
    typedef RedisHashJson<int>  AwardHistory;
    AwardHistory mAllRoldAwardHistory; // 玩家领过奖历史
};

extern RechargeAwardActivity g_act;

#endif
