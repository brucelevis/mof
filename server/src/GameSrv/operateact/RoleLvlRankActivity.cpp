//
//  RoleLvlRankActivity.cpp
//  GameSrv
//
//  Created by jin on 13-11-26.
//
//  开服定时任务， 角色等级排行榜定时奖励

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

class RoleLvlRankActivity
{
public:
    RoleLvlRankActivity()
    {
		mRawards.clear();
    }
    ~RoleLvlRankActivity(){}
    
    void init();
    void open();
    
	int getRankNum(){return mRankNum;}
protected:
    //void award(int guildid, int rank);
    int opentime;
	int mRankNum;
	vector<string> mRawards;
};
static RoleLvlRankActivity activity;

// 活动开启注册
void RoleLvlRankActivityStart(void * param)
{
	if(SSortMgr.isRunning())
	{
		activity.init();
		return;
	}
	addTimer(RoleLvlRankActivityStart, NULL, 5, 1);
}
addTimerDef(RoleLvlRankActivityStart, NULL, 5, 1);

// 角色排行数据
class PaihangRoleData
{
public:
    vector<SortInfo> data;
};
static PaihangRoleData _PaihangRoleData;

void finishLoadPaihangRoleData(int rid, int type, vector<SortInfo> data)
{
	//log_info("《稀有幻兽送斗皇》大小："<<data.size());
    vector<SortInfo>& v =  _PaihangRoleData.data;
    v.clear();
    v.insert(v.end(), data.begin(), data.end());
	activity.open();
}

void OnRoleLvlRankActivityOpen(void * param)
{
	// 注册获取角色排行榜回调
	getSortData(0, eSortLvl, 1, activity.getRankNum(), finishLoadPaihangRoleData);
}

// 活动初始
void RoleLvlRankActivity::init()
{
//    string fullpath = Cfgable::getFullFilePath("rolelvlActivity.ini");
    GameInifile inifile("rolelvlActivity.ini");
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
			log_info("《稀有幻兽送斗皇》角色等级排行榜奖励启动, rolelvlActivity.ini 数据表配置错误");
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
        addTimer(OnRoleLvlRankActivityOpen, NULL, leftsecond, 1);
    }
	
    Xylog log(eLogName_RolelvlRankActivity, 0);
    string str = GameTextCfg::getFormatString("1042", leftsecond);
    //log<<"《稀有幻兽送斗皇》角色等级排行榜奖励启动,倒计时"<<leftsecond<<"秒";
    log << str;
    log_info("《稀有幻兽送斗皇》角色等级排行榜奖励启动,倒计时"<<leftsecond<<"秒");
}

static void mail(const char* rolename, const string &att)
{
    MailFormat *f = MailCfg::getCfg("rare_pets_gift");
    if (f)
    {
        //恭喜您获得《稀有幻兽送斗皇》活动奖励！
        sendMail(0, f->sendername.c_str(), rolename, f->title.c_str(), f->content.c_str(), att.c_str(), "");
    }
}

void RoleLvlRankActivity::open()
{
    for (int i = 0; i < _PaihangRoleData.data.size(); i++)
    {
        SortInfo &si = _PaihangRoleData.data[i];
        // log_info(si.lvl<<" "<<si.rolename);
		string att = mRawards[i];
		mail(si.rolename.c_str(), att);
		
        string lvlstr = GameTextCfg::getString("1043");
        string rankstr= GameTextCfg::getString("1037");
        Xylog log(eLogName_RolelvlRankActivity, si.roleid);
		log<<att
		<<lvlstr//"等级"
		<<si.lvl
		<<rankstr//"排名:"
		<<i + 1;
    }
}


