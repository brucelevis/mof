//
//  PetLvlRankActivity.cpp
//  GameSrv
//
//  Created by jin on 13-11-26.
//
//  开服定时任务， 幻兽等级排行榜定时奖励

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
using namespace std;

class PetLvlRankActivity
{
public:
    PetLvlRankActivity()
    {
		mRawards.clear();
    }
    ~PetLvlRankActivity(){}
    
    void init();
    void open();
    
	int getRankNum(){return mRankNum;}
protected:
    int opentime;
	int mRankNum;
	vector<string> mRawards;
};
static PetLvlRankActivity activity;

// 活动开启注册
void PetLvlRankActivityStart(void * param)
{
	if(SSortMgr.isRunning())
	{
		activity.init();
		return;
	}
	addTimer(PetLvlRankActivityStart, NULL, 5, 1);
}
addTimerDef(PetLvlRankActivityStart, NULL, 5, 1);

// 幻兽排行数据
class PaihangPetData
{
public:
    vector<SortPetInfo> data;
};
static PaihangPetData _PaihangPetData;

void finishPaihangPetData(int rid, int type, vector<SortPetInfo> data)
{
	//log_info("《谁是最神兽》大小："<<data.size());
    vector<SortPetInfo>& v =  _PaihangPetData.data;
    v.clear();
    v.insert(v.end(), data.begin(), data.end());
	activity.open();
}

void OnPetLvlRankActivityOpen(void * param)
{
	// 注册获取幻兽排行榜回调
	getSortPetData(0, 1, activity.getRankNum(), finishPaihangPetData);
}

// 活动初始
void PetLvlRankActivity::init()
{
//    string fullpath = Cfgable::getFullFilePath("petfightpointActivity.ini");
    GameInifile inifile("petfightpointActivity.ini");
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
			log_info("《谁是最神兽》幻兽等级排行榜奖励启动, rolelvlActivity.ini 数据表配置错误");
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
        addTimer(OnPetLvlRankActivityOpen, NULL, leftsecond, 1);
    }
	
    Xylog log(eLogName_PetlvlRankActivity, 0);
    string logstr = GameTextCfg::getFormatString("1040", leftsecond);
    //log<<"《谁是最神兽》幻兽等级排行榜奖励启动,倒计时"<<leftsecond<<"秒";
    log << logstr;
    log_info("《谁是最神兽》幻兽等级排行榜奖励启动,倒计时"<<leftsecond<<"秒");
}

static void mail(const char* rolename, const string &att)
{
    MailFormat *f = MailCfg::getCfg("pet_vs_best");
    if (f)
    {
        //恭喜您获得《谁是最神兽》活动奖励！
        sendMail(0, f->sendername.c_str(), rolename, f->title.c_str(), f->content.c_str(), att.c_str(), "");
    }
}

void PetLvlRankActivity::open()
{
    for (int i = 0; i < _PaihangPetData.data.size(); i++)
    {
        SortPetInfo &spi = _PaihangPetData.data[i];
		
		string att = mRawards[i];
		mail(spi.mastername.c_str(), att);
		
        string petidstr = GameTextCfg::getString("1041");
        string rankstr = GameTextCfg::getString("1037");
        Xylog log(eLogName_PetlvlRankActivity, spi.masterid);
		log<<att
		<< petidstr//"幻兽id"
		<<spi.petid
		<< rankstr//"排名:"
		<<i + 1;
    }
}


