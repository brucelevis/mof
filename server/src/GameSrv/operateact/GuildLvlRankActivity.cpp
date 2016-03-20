//
//  GuildLvlRankActivity.cpp
//  GameSrv
//
//  Created by jin on 13-11-25.
//
//	开服定时任务， 公会等级排行榜定时奖励

#include "DynamicConfig.h"
#include "cstdio"
#include <vector>
#include "GuildMgr.h"
#include "GuildRole.h"
#include "mail_imp.h"
#include "GameLog.h"
#include "OperateActUtils.h"
#include "DataCfg.h"

using namespace std;

class GuildLvlRankActivity
{
public:
    GuildLvlRankActivity()
    {
		mMasterRawards.clear();
		mMembeRawards.clear();
    }
    ~GuildLvlRankActivity(){}
    
    void init();
    void open();
    
protected:
    void award(int guildid, int index);
    int opentime;
	int mRankNum;
	vector<string> mMasterRawards;
	vector<string> mMembeRawards;
};
static GuildLvlRankActivity activity;

void GuildLvlRankActivityStart(void * param)
{
    activity.init();
}
addTimerDef(GuildLvlRankActivityStart, NULL, 5, 1);


void OnGuildLvlRankActivityOpen(void * param)
{
    activity.open();
}

void GuildLvlRankActivity::init()
{
//    string fullpath = Cfgable::getFullFilePath("GuildlvlActivity.ini");
    GameInifile inifile("GuildlvlActivity.ini");
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
			sprintf(awardName, "Num%dMasterAward", i);
			mMasterRawards.push_back(inifile.getValue(*iter, awardName));
			sprintf(awardName, "Num%dNumAward", i);
			mMembeRawards.push_back(inifile.getValue(*iter, awardName));
		}
		
		// 数据表异常检查
		if (mRankNum <= 0
			|| mMasterRawards.size() != mRankNum
			|| mMembeRawards.size() != mRankNum)
		{
			log_info("《公会进击》公会等级排行榜奖励启动, GuildlvlActivity.ini 数据表配置错误");
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
        addTimer(OnGuildLvlRankActivityOpen, NULL, leftsecond, 1);
    }
	
    Xylog log(eLogName_GuildlvlRankActivity, 0);
    string str = GameTextCfg::getFormatString("1036", leftsecond);
    //log<<"《公会进击》公会等级排行榜奖励启动,倒计时"<<leftsecond<<"秒";
    log << str;
    log_info("《公会进击》公会等级排行榜奖励启动,倒计时"<<leftsecond<<"秒");
}

void GuildLvlRankActivity::open()
{
	// 排名前十
    const vector<int> & vec = SGuildMgr.GetAllGuild();
	//log_info("《公会进击》大小："<<vec.size());
	int size = min((int)vec.size(), mRankNum);
    for (int i = 0; i < size; i++)
    {
        int guildid = vec[i];
        Guild& guild = SGuild(guildid);
        
        Xylog log(eLogName_GuildlvlRankActivity, 0);
        log<<guild.getId()
		<<guild.getGuildName()
		<<guild.getLevel()
		<<guild.getMembers().size()
		<< "排名:"
		<<i + 1;
        
        award(guildid, i);
    }
}

static void mail(const char* rolename, string att)
{    
    MailFormat *f = MailCfg::getCfg("offensive_guild");
    if (f)
    {
        //恭喜您所在公会赢得《进击的公会》活动奖励！
        sendMail(0, f->sendername.c_str(), rolename, f->title.c_str(), f->content.c_str(), att.c_str(), "");
    }
}

void GuildLvlRankActivity::award(int guildid, int index)
{
    Guild& guild = SGuild( guildid );
    
    for (Guild::GuildMember::iterator iter = guild.getMembers().begin();
         iter != guild.getMembers().end(); ++iter)
    {
        RoleGuildProperty& prop = SRoleGuild(iter->first);
        
        string att;
        if(prop.getPosition() == kGuildMaster)
        {
            att = mMasterRawards[index];
        }else{
            att = mMembeRawards[index];
        }
        
        mail(prop.getRoleName().c_str(), att);
        
        Xylog log(eLogName_GuildlvlRankActivity, prop.getId());
        log<<att;
    }
    
}



