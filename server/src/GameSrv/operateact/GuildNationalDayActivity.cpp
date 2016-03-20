//
//  GuildNationalDayActivity.cpp
//  GameSrv
//  国庆公会活动，超过2级的公会对成员加个人贡献
//  Created by Huang Kunchao on 13-9-29.
//
//

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

class GuildNationalDayActivity
{
public:
    GuildNationalDayActivity()
    {
        masteraward = 0;
        memberaward = 0;
    }
    ~GuildNationalDayActivity(){}
    
    void init();
    void open();
    
protected:
    void award(int guildid);
    
    int opentime;
    int masteraward;
    int memberaward;
};
static GuildNationalDayActivity activity;


void AfterServerStart(void * param)
{
    activity.init();
}
addTimerDef(AfterServerStart, NULL, 5, 1);


void OnGuildNationalDayActivityOpen(void * param)
{
    activity.open();
}

void GuildNationalDayActivity::init()
{
//    string fullpath = Cfgable::getFullFilePath("GuildNationalDayActivity.ini");
    GameInifile inifile("GuildNationalDayActivity.ini");
    std::list<std::string> sections;
    inifile.getSections(std::back_inserter(sections));
    
    bool found = false;
    
    for (std::list<std::string>::iterator iter = sections.begin(); iter != sections.end(); iter++ )
    {
        OperateActNotForThisServer_ThenContinue();
        
        found = true;
        
        opentime = Utils::parseDate( inifile.getValue(*iter, "Date").c_str() );
        masteraward = inifile.getValueT(*iter, "MasterAward", 0);
        memberaward = inifile.getValueT(*iter, "MemberAward", 0);
        break;
    }
    
    if(! found )
        return;
    
    if(-1 == opentime )
        return;
    
    int leftsecond =  opentime - time(0);
    
    if(leftsecond > 0)
    {
        addTimer(OnGuildNationalDayActivityOpen, NULL, leftsecond, 1);
    }

    Xylog log(eLogName_GuildNationalDayActivity,0);
    string str = GameTextCfg::getFormatString("1038", leftsecond);
    //log<<"公会大乱斗活动启动,倒计时"<<leftsecond;
    log << str;
    log_info("公会大乱斗活动启动,倒计时"<<leftsecond<<"秒");
}

void GuildNationalDayActivity::open()
{
    
    const vector<int> & vec = SGuildMgr.GetAllGuild();
    for (int i = 0 , ni = vec.size() ; i<ni; ++i)
    {
        int guildid = vec[i];
        Guild& guild = SGuild( guildid );
        
        if(guild.getLevel() < 2)
            continue;
        
        Xylog log(eLogName_GuildNationalDayActivity,0);
        log<<guild.getId()<<guild.getGuildName()<<guild.getLevel()<<guild.getMembers().size();
        log.save();
        
        award(guildid);
    }
}

static void mail(const char* rolename , int val)
{
    MailFormat *f = MailCfg::getCfg("guild_vs");
    if (f)
    {
        string mail_content = f->content;
        string val_str = Utils::itoa(val);
        //恭喜您所在公会赢得公会大乱斗活动奖励！您已获得%d个人贡献值，请于公会页面查看。祝您的公会蒸蒸日上！
        if (1 != find_and_replace(mail_content, 1, val_str.c_str()))
        {
            log_warn("mail content format error.[guild_vs]");
        }
        sendMail(0, f->sendername.c_str(), rolename, f->title.c_str(), mail_content.c_str(), "", "");
    }
}

void GuildNationalDayActivity::award(int guildid)
{
    Guild& guild = SGuild( guildid );
    
    for (Guild::GuildMember::iterator iter = guild.getMembers().begin();
         iter != guild.getMembers().end(); ++iter)
    {
        RoleGuildProperty& prop = SRoleGuild(iter->first);
        
        int val = 0;
        
        if(prop.getPosition() == kGuildMaster)
        {
            val = masteraward;
        }else{
            val = memberaward;
        }
        
        string str =  GameTextCfg::getString("1039");
        prop.addConstrib( val , "公会大乱斗奖励");
        
        mail(prop.getRoleName().c_str(), val);
        
        Xylog log(eLogName_GuildNationalDayActivity,prop.getId());
        log<<val;
    }
    
}



