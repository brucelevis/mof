//
//  RoleFightPointActivity.cpp
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
using namespace std;

class RoleFightPointActivity
{
public:
    RoleFightPointActivity()
    {
		mRawards.clear();
    }
    ~RoleFightPointActivity(){}
    
    void init();
    void open();
    
	int getRankNum(){return mRankNum;}
protected:
    //void award(int guildid, int rank);
    int opentime;
	int mRankNum;
	vector<string> mRawards;
};
static RoleFightPointActivity activity;

// 活动开启注册
void RoleFightPointActivityStart(void * param)
{
	if(SSortMgr.isRunning())
	{
		activity.init();
		return;
	}
	addTimer(RoleFightPointActivityStart, NULL, 5, 1);
}
addTimerDef(RoleFightPointActivityStart, NULL, 5, 1);

// 角色排行数据
class PaihangRoleData
{
public:
    vector<SortInfo> data;
};
static PaihangRoleData _PaihangRoleData;

static void finishLoadPaihangRoleData(int rid, int type, vector<SortInfo> data)
{
    vector<SortInfo>& v =  _PaihangRoleData.data;
    v.clear();
    v.insert(v.end(), data.begin(), data.end());
	activity.open();
}

static void OnRoleLvlRankActivityOpen(void * param)
{
	// 注册获取角色排行榜回调
	getSortData(0, eSortBat, 1, activity.getRankNum(), finishLoadPaihangRoleData);
}

// 活动初始
void RoleFightPointActivity::init()
{
//    string fullpath = Cfgable::getFullFilePath("rolefightpointActivity.ini");
    GameInifile inifile("rolefightpointActivity.ini");
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
			log_info("角色战斗力排行榜奖励启动, rolefightpointActivity.ini 数据表配置错误");
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
	
    Xylog log(eLogName_RoleFightPointActivity, 0);
    log << "角色战斗力排行榜奖励启动,倒计时" << leftsecond << "秒";
    log_info("角色战斗力排行榜奖励启动,倒计时" << leftsecond << "秒");
}

static void mail(const char* rolename, const string &att)
{
    MailFormat *f = MailCfg::getCfg("rolefightpointActivity");
    if (f)
    {
        sendMail(0, f->sendername.c_str(), rolename, f->title.c_str(), f->content.c_str(), att.c_str(), "");
    }
}

void RoleFightPointActivity::open()
{
    for (int i = 0; i < _PaihangRoleData.data.size(); i++)
    {
        SortInfo &si = _PaihangRoleData.data[i];
        
		string att = mRawards[i];
        
        //发送邮件
		mail(si.rolename.c_str(), att);
		
        Xylog log(eLogName_RoleFightPointActivity, si.roleid);
		log<< att << "战斗力" << si.battle <<"排名:" <<i + 1;
    }
}



