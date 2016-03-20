/****************************************************************************
 活动期间
 每日金钻抽奖满5次，即送二级攻击石*3
 每日金钻抽奖满10次，即送三级攻击石*3
 每日金钻抽奖满20次，即送四级攻击石*3
 ****************************************************************************/

#include "DynamicConfig.h"
#include "mail_imp.h"
#include "GameLog.h"
#include "process.h"
#include "Role.h"
#include "OperateActUtils.h"
#include <map>
#include <string>

struct LotteryAwardItem
{
    int rmblottery; // 金钻抽奖
    string items;   // 奖励物品
};

struct LotteryAct  // 一场活动
{
    int beginTime;  // 开始时间
    int endTime;    // 结束时间
    std::map<int,LotteryAwardItem> awards; // 奖励内容
};


class OperateActivity
{
public:
    void init();    // 读配置
    void award(int roleid,int beforeRmbTime , int afterRmbTime); // 发奖
protected:
    vector<LotteryAct> mActs;  // 不同时间段的活动
};

static OperateActivity g_act;



void OperateActivity::init()
{
//    string fullpath = Cfgable::getFullFilePath("OperateActLotteryAward.ini");
    GameInifile inifile("OperateActLotteryAward.ini");
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
    {
        OperateActNotForThisServer_ThenContinue();
        
        LotteryAct act;
        act.beginTime = Utils::parseDate( inifile.getValue(*iter, "startdate").c_str() );
        act.endTime = Utils::parseDate( inifile.getValue(*iter, "overdate").c_str() );
        const int awardnum = inifile.getValueT(*iter, "awardnum", 0);
        
        for (int i = 0; i < awardnum; ++i)
        {
            LotteryAwardItem award;
            award.rmblottery = inifile.getValueT(*iter, xystr("needlottery%d",i+1), -1);
            award.items = inifile.getValue(*iter, xystr("itemawards%d",i+1));
            assert( award.rmblottery >=0 );
            act.awards.insert( make_pair(award.rmblottery,award) );
        }
        
        mActs.push_back(act);
    }
}

extern std::string parseItemMailContent(const char* str);

// 通过邮件附件发礼包
static void mail(int roleid , const char* rolename , const LotteryAwardItem& award)
{
    stringstream attach;
    attach << award.items ;
    
    MailFormat *f = MailCfg::getCfg("lottery_feedback");
    if (f)
    {

        string mail_content = f->content;
        string rmblottery = Utils::itoa(award.rmblottery);
        string award_str = parseItemMailContent(award.items.c_str());
        
        //您今日已经累计进行了%d次转盘抽奖，获得了%s.
        if (2 != find_and_replace(mail_content, 2, rmblottery.c_str(), award_str.c_str()))
        {
            log_warn("mail content format error. [lottery_feedback]");
        }
        
        bool ret = sendMail(0,
                            f->sendername.c_str(),
                            rolename,
                            f->title.c_str(),
                            mail_content.c_str(),
                            attach.str().c_str(), "");
        
        if( false ==  ret )
        {
            StoreOfflineItem(roleid,attach.str().c_str());
        }
        
        Xylog log(eLogName_OperateActLotteryAward, roleid);
        log<<award.rmblottery<<award.items;
    }
    else
    {
        log_warn("read mail.ini <lottery_feedback> error.");
    }
}



void OperateActivity::award(int roleid,int beforeRmbTime , int afterRmbTime)
{
    assert( beforeRmbTime <= afterRmbTime);
    if(beforeRmbTime == afterRmbTime)
        return;
    
    int currentTime = time(0);
    
    Role* role = SRoleMgr.GetRole(roleid);
    xyassertf(role,"%d - %d - %d", roleid, beforeRmbTime , afterRmbTime);

    
    for(vector<LotteryAct>::iterator it = mActs.begin();it != mActs.end(); ++it)
    {
        const LotteryAct& act = *it;
        
        if( currentTime < act.beginTime || act.endTime < currentTime)
            continue;
        
        for (int i = beforeRmbTime+1; i<= afterRmbTime; ++i)
        {
            std::map<int,LotteryAwardItem>::const_iterator itAward = act.awards.find(i);
            if( itAward != act.awards.end())
                mail(roleid,role->getRolename().c_str() , itAward->second);
        }
    }
}



#pragma mark -


void lotteryAwardActivity(int roleid ,int beforeRmbTime , int afterRmbTime)
{
    g_act.award(roleid,beforeRmbTime,afterRmbTime);
}

static void AfterServerStartInitLotteryAward(void * param)
{
    g_act.init();
}
addTimerDef(AfterServerStartInitLotteryAward, NULL, 5, 1);

