//
//  AccumulateConsumeAwardAct.h
//  GameSrv
//
//  Created by jin on 14-9-26.
//
//

#ifndef GameSrv_AccumulateConsumeAwardAct_h
#define GameSrv_AccumulateConsumeAwardAct_h

namespace AccumulateConsumeAward
{
#define ACCU_RMB "rmb"
	
	// 一场活动
    class Activity
    {
    public:
        struct AwardItem
        {
            int rmb;    // 累计
            string items;   // 奖励物品
        };
        
        void init(int index, int begTime , int endTime,const vector<AwardItem>& awards);
        
        int getBeginTime() { return mBeginTime;}
        int getEndTime() { return mEndTime; }
        
        // 玩家消费累计
        void consume(int roleid, int rmb );
        
        // 玩家领取奖励
        void getAward(int roleid, int rmb );
        
        // 发送玩家状态
        void sendStatus(int roleid);
        
        
        // 发送奖励
        bool sendAward(Role* role,  const string& award);
        
		
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
	
	class ActivityMgr
    {
    public:
        ~ActivityMgr();
        void init();
        void consume(int roleid, int rmb); // 消费
        void getAward(int roleid, int rmb); // 领奖
        void sendStatus(int roleid); // 发送当前状态
        
		vector<Activity*> & getActs() {return mActs;}
    protected:
        vector<Activity*> mActs;  // 不同时间段的此活动
    };
}

extern AccumulateConsumeAward::ActivityMgr g_AccumulateConsumeAward;

#endif
