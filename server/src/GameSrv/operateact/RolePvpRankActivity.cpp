//
//  RoleLvlRankActivity.cpp
//  GameSrv
//
//  Created by jin on 13-11-26.
//
//  开服角色竞技场排行榜定时奖励,PS:配置表server_id一定不能相同。

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

class RolePvpRankActivity
{
public:
    RolePvpRankActivity()
    {
		mRawards.clear();
    }
    ~RolePvpRankActivity(){}
    
    void init();
    void open();
    
	int getRankNum(){return mRankNum;}
protected:
    int opentime;
	int mRankNum;
    //等级区间，奖励
    map<int, string> mRawards;
    
};
static RolePvpRankActivity activity;

// 活动开启注册
void RolePvpRankActivityStart(void * param)
{
	if(SSortMgr.isRunning())
	{
		activity.init();
		return;
	}
	addTimer(RolePvpRankActivityStart, NULL, 5, 1);
}
addTimerDef(RolePvpRankActivityStart, NULL, 5, 1);

// 角色排行数据
class PaihangRoleData
{
public:
    vector<SortInfo> data;
};
static PaihangRoleData _PaihangRoleData;

void finishLoadRolePvpData(int rid, int type, vector<SortInfo> data)
{
    vector<SortInfo>& v =  _PaihangRoleData.data;
    v.clear();
    v.insert(v.end(), data.begin(), data.end());
	activity.open();
}

void OnRolePvpRankActivityOpen(void * param)
{
	// 注册获取角色排行榜回调
	getSortData(0, eSortPvp, 1, activity.getRankNum(), finishLoadRolePvpData);
}

// 活动初始
void RolePvpRankActivity::init()
{
//    string fullpath = Cfgable::getFullFilePath("RolePvpRankActivity.ini");
    GameInifile inifile("RolePvpRankActivity.ini");
    
    int count = inifile.getValueT("common", "server_count", 0);
    if (count  == 0)
    {
        return ;
    }
    
    bool found = false;
    for (int i = 1; i <= count; ++i)
    {
        string tmp = Utils::itoa(i);
        //如果不是本server ID则放弃
        int serverid = inifile.getValueT(tmp, "server_id", 0);
        int thisServerId = Process::env.getInt("server_id");
        if( ALL_SERVER_ID != serverid && serverid != thisServerId)
        {
            continue;
        }
        
        //开始时间
        opentime = Utils::parseDate(inifile.getValue(tmp, "date").c_str());
        
        char nodename[60];
        snprintf(nodename, sizeof(nodename), "award%d", i);
        vector<string> keys;
        inifile.getKeys(nodename, back_inserter(keys));
        for (int i = 0; i < keys.size(); ++i)
        {
            vector<string> tokens = StrSpilt(keys[i], "-");
            if (tokens.size() != 2)
            {
                return ;
            }
            int val = Utils::safe_atoi(tokens[1].c_str());
            string award = inifile.getValue(nodename, keys[i]);
            mRawards.insert(make_pair(val, award));
        }
        
        found = true;
    }
    
    if (mRawards.empty())
    {
        return ;
    }
    mRankNum = (mRawards.rbegin())->first;
    
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
        addTimer(OnRolePvpRankActivityOpen, NULL, leftsecond, 1);
    }
	
    Xylog log(eLogName_RolelvlRankActivity, 0);
    string str = GameTextCfg::getFormatString("1042", leftsecond);
    log << str;
    log_info("角色PVP等级排行榜奖励启动,倒计时"<<leftsecond<<"秒");
}

static bool mail(const char* rolename, const string &att)
{
    MailFormat *f = MailCfg::getCfg("RolePvpRankActivity");
    if (f)
    {
        int ret = sendMail(0, f->sendername.c_str(), rolename, f->title.c_str(), f->content.c_str(), att.c_str(), "");
        return ret;
    }
    return false;
}

void RolePvpRankActivity::open()
{
    log_info("开始发送角色PVP等级排行榜奖励");
    map<int, string>::iterator iter = mRawards.begin();
    int lvl = iter->first;
    string att = iter->second;
    
    for (int i = 0; i < _PaihangRoleData.data.size(); i++)
    {
        SortInfo &si = _PaihangRoleData.data[i];
        if (i < lvl)
        {
            if (mail(si.rolename.c_str(), att))
            {
                //todo: 邮件发送成功，记录邮件发送成功日志
                Xylog log(eLogName_RolePvpRankActivity_Mail, si.roleid);
                log << si.pvp << si.pvpRank << att;
            }
            
            //记录排名所有角色
            Xylog log(eLogName_RolePvpRankActivity, si.roleid);
            log << si.pvp << si.pvpRank << att;
        }
        else
        {
            if (iter != mRawards.end())
            {
                ++iter;
                lvl = iter->first;
                att = iter->second;
                --i;
            }
        }
    }
}


