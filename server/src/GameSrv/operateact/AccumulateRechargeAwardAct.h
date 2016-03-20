//
//  AccumulateRechargeAwardAct.h
//  GameSrv
//
//  Created by jin on 14-9-26.
//
//

#ifndef GameSrv_AccumulateRechargeAwardAct_h
#define GameSrv_AccumulateRechargeAwardAct_h

namespace AccumulateRechargeAwardAct
{
// 不同的channel有不同的返利比例, 玩家优先用channel比例，否则用默认的
#define ACCU_RMB "rmb"
    
    // 一场活动
    class Activity
    {
    public:
        struct AwardItem
        {
            int rmb;    // 累计充值
            string items;   // 奖励物品
        };
        
        void init(int index, int begTime , int endTime,const string& subchannel,const vector<AwardItem>& awards);
        
        int getBeginTime() { return mBeginTime;}
        int getEndTime() { return mEndTime; }
        
        // 玩家充值
        void recharge(int roleid, int rmb );
        
        // 玩家领取奖励
        void getAward(int roleid, int rmb );
        
        // 发送玩家状态
        void sendStatus(int roleid);
        
        string getSubChannel(){ return mSubChannel;}
        
        // 发送奖励
        bool sendAward(Role* role,  const string& award);
        
		string mTitle;
		string mContent;
		int mAwardType;
		string mChannel;
		string mServerId;
		int afterOpenServerDays;
		int mIndex;      // 活动唯一标记
        int mBeginTime;  // 开始时间
        int mEndTime;    // 结束时间
        string mSubChannel; // 具体平台，如果是"－1"表示默认
        vector<AwardItem>   mAwards; // 奖励内容
        
        typedef RedisHashJson<int>  AwardHistory;
        AwardHistory mAwardHistory; // 玩家领过奖历史
    };
	
	class ActivityMgr
    {
    public:
        ~ActivityMgr();
        void init();
        void recharge(int roleid, int rmb); // 充值
        void getAward(int roleid, int rmb); // 领奖
        void sendStatus(int roleid); // 发送当前状态
        
        Activity* getChannelAct(int roleid);  // 玩家优先用channel比例，否则用默认的
        
		vector<Activity*> & getActs() {return mActs;}
    protected:
        vector<Activity*> mActs;  // 不同时间段的此活动
    };
	
}

extern AccumulateRechargeAwardAct::ActivityMgr g_AccumulateRechargeAwardAct;



#endif
