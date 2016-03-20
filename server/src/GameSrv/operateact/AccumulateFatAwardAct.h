//
//  AccumulateFatAwardAct.h
//  GameSrv
//
//  Created by haifeng on 14-12-29.
//
//

#ifndef __GameSrv__AccumulateFatAwardAct__
#define __GameSrv__AccumulateFatAwardAct__

#include <string>
#include <vector>
#include "Role.h"
#include "RedisHashJson.h"

using namespace std;

namespace AccumulateFatAwardAct
{
#define ACCU_FAT "fat"
    
    class FatActivity
    {
    public:
        struct AwardItem
        {
            int fat;        // 累计精力
            string items;   // 奖励物品
        };
        
        void init(int index, int beginTime, int endTime, const vector<AwardItem>& awards);
        int getBeginTime () {return mBeginTime;}
        int getEndTime() {return mEndTime;}
        
        // 玩家累计消耗精力
        void consume(int roleid, int fat);
        // 玩家领取奖励
        void getAward(int roleid, int fat);
        // 发送玩家状态
        void sendStatus(int roleid);
        // 发送奖励
        bool sendAward(Role* role, const string& award);
        
        string mTitle;
        string mContent;
        int mAwardType;
        string mChannel;
        string mServerId;
        int afterOpenServerDays;
        int mIndex;        // 活动唯一标记
        int mBeginTime;    // 活动开始时间
        int mEndTime;      // 活动结束时间
        vector<AwardItem>  mAwards;   // 奖励内容
        
        typedef RedisHashJson<int> FatAwardHistory;
        FatAwardHistory mFatAwardHistory;   // 玩家领过奖的历史
        
    };
    
    class ActivityMgr
    {
    public:
        ~ActivityMgr();
        void init();
        void consume(int roleid, int fat);      // 消费
        void getAward(int roleid, int fat);     // 领奖
        void sendStatus(int roleid);            // 发送当前状态
        
        vector<FatActivity*>& getActs() {return mActs;}
        
    protected:
        vector<FatActivity*> mActs;    // 不同时间段的此活动
        
    };
    
}

extern AccumulateFatAwardAct::ActivityMgr g_AccumulateConsumeFatAward;

#endif /* defined(__GameSrv__AccumulateFatAwardAct__) */
