//
//  FamousDungeActivity.cpp
//  GameSrv
//
//  Created by nothing on 14-3-11.
//
//

#include "DynamicConfig.h"
#include "cstdio"
#include <vector>
#include "mail_imp.h"
#include "GameLog.h"
#include "GuildMgr.h"
#include "GuildRole.h"
#include "Paihang.h"
#include "ActivityMod.h"
#include "EnumDef.h"
#include "OperateActUtils.h"
#include "FamesHall.h"
#include "inifile.h"

using namespace std;

class FamousDungeActivity
{
public:
    FamousDungeActivity()
    {
		mRawards.clear();
    }
    ~FamousDungeActivity(){}
    
    void init();
    void open();
    
	int getRankNum(){return mRankNum;}
protected:
    int opentime;
	int mRankNum;
	vector<string> mRawards;
};
static FamousDungeActivity activity;

// 活动开启注册
void FamousDungeActivityStart(void * param)
{
	//if(SSortMgr.isRunning())
	//{
		activity.init();
    //return;
	//}
	//addTimer(FamousDungeActivityStart, NULL, 5, 1);
}
addTimerDef(FamousDungeActivityStart, NULL, 5, 1);


class FamousDungeData
{
public:
    map<int, FamesList> data;
};
static FamousDungeData _RankData;

static void finishGetFramesHallData(const map<int, FamesList>& data)
{
    /*
     eWarrior=0  // 剑士
     eMage=1    // 法师
     eAssassin=2// 刺客
    */
    map<int, FamesList> &v =  _RankData.data;
    v.clear();
    v.insert(data.begin(), data.end());
    
	activity.open();
}

static void OnFamousDungeActivityOpen(void * param)
{
    getFamesHallData(finishGetFramesHallData);
}

// 活动初始
void FamousDungeActivity::init()
{
//    string fullpath = Cfgable::getFullFilePath("famousdungeActivity.ini");
    GameInifile inifile("famousdungeActivity.ini");
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    bool found = false;
    char awardName[32];
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++)
    {
        OperateActNotForThisServer_ThenContinue();
		
        opentime = Utils::parseDate(inifile.getValue(*iter, "Date").c_str());
		mRankNum = inifile.getValueT(*iter, "maxnum", 0);
		for (int i = 1; i <= mRankNum; i++)
		{
			sprintf(awardName, "Num%dAward", i);
			mRawards.push_back(inifile.getValue(*iter, awardName));
		}
		// 数据表异常检查
		if (mRankNum <= 0 || mRawards.size() != mRankNum) {
			log_info("famousdungeActivity.ini 数据表配置错误");
			return;
		}
		
		found = true;
        break;
    }
    
    if(!found)
	{
        return;
	}
    
    if(-1 == opentime)
	{
        return;
	}
    
    int leftsecond = opentime - time(0);
    
    if(leftsecond > 0)
    {
        addTimer(OnFamousDungeActivityOpen, NULL, leftsecond, 1);
    }
	
    Xylog log(eLogName_FamousDungeActivity, 0);
    log << "名人堂排行榜奖励启动,倒计时" << leftsecond << "秒";
    log_info("名人堂排行榜奖励启动,倒计时"<<leftsecond<<"秒");
}

static void mail(const char* rolename, const string &att)
{
    MailFormat *f = MailCfg::getCfg("famousdungeActivity");
    if (f)
    {
        sendMail(0, f->sendername.c_str(), rolename, f->title.c_str(), f->content.c_str(), att.c_str(), "");
    }
}

void FamousDungeActivity::open()
{
    for (int i = eWarrior; i <= eAssassin; ++i)
    {
        for (int j = 0; j < activity.getRankNum(); ++j)
        {
            string att = mRawards[j];
            mail(_RankData.data[i][j].name.c_str(), att);
            
            FamesInfo &info = _RankData.data[i][j];
            Xylog log(eLogName_FamousDungeActivity, info.personId);
            log << att << "职业" << i << "排名" << j;
        }
    }
}
