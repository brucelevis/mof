//
//  OnceRechargeAwardAct.h
//  GameSrv
//
//  Created by jin on 14-9-26.
//
//

#ifndef GameSrv_OnceRechargeAwardAct_h
#define GameSrv_OnceRechargeAwardAct_h


// 一场活动
class OnceRechargeAct
{
public:
    struct AwardItem
    {
        int rmb;    // 至少要充值
        string items;   // 奖励物品
    };
    
    void init(int index, int begTime , int endTime,const vector<AwardItem>& awards);
    
    int getBeginTime() { return mBeginTime;}
    int getEndTime() { return mEndTime; }
    
    // 尝试奖励
    void award(int roleid,const char* rolename , int rmb );
    
	string mTitle;
	string mContent;
	int mAwardType;
	string mChannel;
	string mServerId;
    
    int mIndex;      // 活动唯一标记
    int mBeginTime;  // 开始时间
    int mEndTime;    // 结束时间
    int afterOpenServerDays;
    vector<AwardItem>   mAwards; // 奖励内容
    
    typedef RedisHashJson<int>  AwardHistory;
    AwardHistory mAwardHistory; // 玩家领过奖历史
};

class OnceRechargeAwardActivity
{
public:
    ~OnceRechargeAwardActivity();
    void init();    // 读配置
    void award(int roleid, const char* rolename, int rmb); // 发奖
    void getAward(int roleid, int time); // 领奖
    void sendStatus(int roleid); // 发送当前状态
    
    typedef RedisHashJson<int>  AwardHistory;
    AwardHistory mAllRoldAwardHistory; // 玩家领过奖历史
    
	vector<OnceRechargeAct*> & getActs() {return mActs;}
protected:
    vector<OnceRechargeAct*> mActs;  // 该服不同时间段的活动
    bool sendAward(Role* role ,  const string& award);
};

extern OnceRechargeAwardActivity g_OnceRechargeAwardActivity;

#endif
