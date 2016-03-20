
#include "GuildApply.h"
#include "Role.h"
#include "GameLog.h"
#include "GuildMgr.h"
#include "GuildRole.h"
#include "json/json.h"
#include "RedisResultHelper.h"
#include "RedisKeyName.h"
using namespace std;


static const int AUTO_CLEAN_APPLY_DAY = 3;      //自动删除未处理申请天数


GuildApplicant::GuildApplicant():
roleid(0),date(0),viplvl(0),job(0),level(0),battle(0)
{
}

string GuildApplicant::str() const
{
    Json::Value value;
    value["roleid"] = roleid;
    value["date"] = date;
    value["viplvl"]= viplvl;
    value["job"] = job;
    value["level"] = level;
    value["battle"]= battle;
    value["rolename"]= rolename;
    return Json::FastWriter().write(value);
}

void GuildApplicant::load(const char* str)
{
    if(NULL == str) return;
    
    Json::Reader reader;
    Json::Value retval;
    
    if ( ! reader.parse(str, retval)){
        xyerr("GuildApply json parse error , %s ",str);
        return;
    }
    
    roleid = retval["roleid"].asInt();
    date = retval["date"].asInt();
    viplvl = retval["viplvl"].asInt();
    job = retval["job"].asInt();
    level = retval["level"].asInt();
    battle = retval["battle"].asInt();
    rolename = retval["rolename"].asString();
}

std::ostream& operator<<(std::ostream& out, const GuildApplicant& obj)
{
    return out << obj.str();
}

std::istream& operator>>(std::istream& in,GuildApplicant& obj)
{
    string tmp;
    in>>tmp;
    obj.load(tmp.c_str());
    return in;
}

#pragma mark -
#pragma mark class GuildApplyMgr implement

struct GuildApplyNode
{
    GuildApplyNode(int gid,int rid)
    {
        this->gid = gid;
        this->rid = rid;
    }
    int gid;
    int rid;
};


GuildApplyMgr::Apply& GuildApplyMgr::getApply(int roleid)
{
    UserMap::iterator iter = mUserMap.find(roleid);
    if(iter != mUserMap.end())
        return iter->second;
    
    if( ! isRoleExistInDbForMainThread(roleid) )
        return mDummyApply;
    
    Apply gs;
    gs.load(roleid);
    mUserMap.insert(make_pair(roleid, gs));
    
    return mUserMap[roleid];
}

GuildApplyMgr::Applicant& GuildApplyMgr::getApplicant(int guildid)
{
    Guild& g = SGuild(guildid);
    if( g.getId() == 0){
        return mDummyApplicant;
    }
    
    GuildMap::iterator iter = mGuildMap.find(guildid);
    if(iter != mGuildMap.end())
        return iter->second;
    
    Applicant am;
    am.load(guildid);
    mGuildMap.insert(make_pair(guildid, am));
    
    return mGuildMap[guildid];
}

//增加申请
eApplyGuild GuildApplyMgr::insert(int roleid , int guildid )
{
    Role* role = SRoleMgr.GetRole(roleid);      // 不在线直接返回
    if(NULL == role){
        xyerr("GuildApplyMgr::add() role = NULL ERROR");
        return eApplyGuild_UnknownErr;
    }
    
    const bool isGuildIdValid = SGuild(guildid).getId() == 0 ;
    const bool isRoleHasGuild =  SRoleGuild(roleid).getGuild() != 0 ;
    
    if( isGuildIdValid || isRoleHasGuild )      // 玩家已有公会，或者这个公会不存在
        return eApplyGuild_UnknownErr;
    
    if( getApply(roleid).exist(guildid) )       // 已经申请过了
        return eApplyGuild_AlreadyApplied;
	
	if (getApply(roleid).size() >= 5)			// 申请达到５个
		return eApplyGuild_RoleApplyFull;
    
    if( getApplicant(guildid).size() >= GuildCfgMgr::getMaxApplicant() )   // 公会申请者已满
        return eApplyGuild_GuildApplicantFull;
    
//    if( role->getGuildLeftTime() + GUILD_WAIT_FOR_APPLY_SECOND > time(0))
//        return eApplyGuild_NeedToWait;          // 离开公会后一定时间（配置）内无法再申请或创建公会
    
    if( role->getGuildLeftTime() +  GuildCfgMgr::getWaitApplySecond() > time(0))
        return eApplyGuild_NeedToWait;   // 离开公会后，再次申请或创建需要等待的时间
    
    GuildApplicant ga;
    ga.roleid = roleid;
    ga.date = time(0);
    ga.viplvl = role->getVipLvl();
    ga.job = role->getJob();
    ga.level = role->getLvl();
    ga.battle = role->getBattleForce();
    ga.rolename = role->getRolename();
    
    getApply(roleid).set(guildid, guildid); 
    getApplicant(guildid).set(roleid, ga);
    
    //申请者同步，管理员不用同步，等着来取
    //on_req_role_apply_guild_list(role);
    
    
    return eApplyGuild_Success;
}

//删除申请列表
bool GuildApplyMgr::remove(int roleid , int guildid)
{
    bool bRet = true;
    
    if( ! getApply(roleid).exist(guildid)  )
        bRet = false;
    
    if( ! getApplicant(guildid).exist(roleid) )
        bRet = false;
    
    getApply(roleid).remove(guildid);           
    getApplicant(guildid).remove(roleid);
    
    if( bRet )
    {
        notify_delete_apply nty;
        nty.guildid = guildid;
        nty.roleid = roleid;
        
        //客户端在线则同步列表
        Role* role = SRoleMgr.GetRole(roleid);
        if(role){
            sendRolePacket(roleid,&nty);
        }
        
        //在线的管理员同步列表
        Guild& guild = SGuild(guildid);
        Guild::GuildMember::iterator itMember = guild.getMembers().begin();
        for (;itMember != guild.getMembers().end(); ++itMember)
        {
            if ( ! SGuildMgr.roleHasRight(itMember->first,kGuildAcceptApplicant) )
                continue;
            Role* mgr = SRoleMgr.GetRole(itMember->first);
            if(NULL == mgr)
                continue;
            sendRolePacket(itMember->first,&nty);
        }
    }
    
    return bRet;
}


void GuildApplyMgr::clearApply(int roleid)
{
    Apply& gs = getApply(roleid);
    vector<GuildApplyNode> tmp;
    for (Apply::iterator it = gs.begin(); it != gs.end(); ++it) {
        tmp.push_back(GuildApplyNode(it->first,roleid));
    }
    
    for (vector<GuildApplyNode>::iterator iter = tmp.begin();
         iter != tmp.end(); ++iter)
    {
        remove(iter->rid,iter->gid);
    }
    
    gs.deleteFromDatabase();
    mUserMap.erase(roleid);
}

void GuildApplyMgr::clearApplicant(int guildid)
{
    Applicant& ap = getApplicant(guildid);
    vector<GuildApplyNode> tmp;
    for (Applicant::iterator it = ap.begin(); it!= ap.end(); ++it) {
        tmp.push_back(GuildApplyNode(guildid,it->first));
    }
    
    for (vector<GuildApplyNode>::iterator iter = tmp.begin();
         iter != tmp.end(); ++iter)
    {
        remove(iter->rid,iter->gid);
    }
        
    ap.deleteFromDatabase();
    mGuildMap.erase(guildid);
}

//自动删除3天前的申请列表
void GuildApplyMgr::clearExpiredApplication()
{
    int remDate = time(0) - GUILD_APPLY_EXPIRED_SECOND;
    
    vector<GuildApplyNode> tmp;
    
    GuildMap::iterator itGuild;
    Applicant::iterator itApplicant;
    for (itGuild = mGuildMap.begin(); itGuild != mGuildMap.end(); ++itGuild)//全部公会
    {
        for (itApplicant = itGuild->second.begin();
             itApplicant != itGuild->second.end(); ++ itApplicant)//某个公会下全部申请
        {
            if(itApplicant->second.date <= remDate)
                tmp.push_back(GuildApplyNode(itGuild->first,itApplicant->first));
        }
    }
    
    for (vector<GuildApplyNode>::iterator iter = tmp.begin();
         iter != tmp.end(); ++iter)
    {
        remove(iter->rid,iter->gid);
    }
}

//角色申请的列表
void GuildApplyMgr::on_req_role_apply_guild_list(Role* role)
{
    xyassert(role);
    ack_role_apply_guild_list ack;
    Apply& gs = getApply(role->getInstID());
    for (Apply::iterator it = gs.begin(); it != gs.end(); ++it) {
        ack.guilds.push_back(it->first);
    }
    return sendRolePacket(role->getInstID(), &ack);
}

typedef GuildApplicant* ApplicantPointer;
bool ApplicantLess(const ApplicantPointer& left, const ApplicantPointer& right)
{
    return left->date < right->date;
}

void ApplicantSort(GuildApplyMgr::Applicant& in, vector<ApplicantPointer>& out)
{
    out.clear();
    for (GuildApplyMgr::Applicant::iterator it = in.begin(); it!= in.end(); ++it)
    {
        ApplicantPointer p = &(it->second);
        if(p == NULL)
            continue;
        out.push_back(p);
    }
    sort(out.begin(), out.end(),ApplicantLess);
}


//公会被申请列表
void GuildApplyMgr::on_req_guild_applicant_list(Role* role,req_guild_applicant_list& req)
{
    xyassert(role);
    int roleid = role->getInstID();
    
    if( ! SGuildMgr.roleHasRight(roleid, kGuildAcceptApplicant) ) //权限
        return ;
 
    int guildid = SRoleGuild(roleid).getGuild();
    Applicant& am = getApplicant(guildid);

    vector<ApplicantPointer> appVec;
    ApplicantSort(am,appVec);
    
    std::pair<int, int> range = checkPageRange(appVec.size(), req.start, req.num);
    
    ack_guild_applicant_list ack;
    ack.start = range.first;
    obj_guild_applicant obj;
    
    for ( int i = range.first; i<range.second; ++i)
    {
        ApplicantPointer p = appVec[i];
        if(p == NULL)
            continue;
        
        obj.roleid = p->roleid;
        obj.rolename = p->rolename;
        obj.applytime = p->date;
        obj.job = p->job;
        obj.lvl = p->level;
        obj.batpoint = p->battle;
        obj.vip = p->viplvl;
        ack.applicants.push_back(obj);
    }
    return sendRolePacket(roleid, &ack);
}

static void notifyGuildMgrNewApplicant(int guildid)
{
    // 通知
    Guild& guild = SGuildMgr.getGuild(guildid);
    assert( ! guild.isNull() );
    for (Guild::GuildMember::iterator iter = guild.getMembers().begin();
         iter != guild.getMembers().end(); ++iter)
    {
        RoleGuildProperty& prop = SRoleGuild(iter->first);
        if(prop.getPosition() == kGuildMaster || prop.getPosition() == kGuildViceMaster)
        {
            notify_new_applicant notify;
            sendRolePacket(prop.getId(), &notify);
        }
    }
}

//申请公会
void GuildApplyMgr::on_req_apply_guild(Role* role,req_apply_guild& req)
{
    xyassert(role);
    ack_apply_guild ack;
    
    ack.guildid = req.guildid;
    ack.errorcode = insert(role->getInstID() , req.guildid);
    
    if( ack.errorcode ==  eApplyGuild_Success )
    {
        notifyGuildMgrNewApplicant(req.guildid);
    }
    return sendNetPacket(role->getSessionId(), &ack);
}

// 取消申请公会
void GuildApplyMgr::on_req_ancel_apply_guild(Role* role, req_cancel_apply_guild &req)
{
	xyassert(role);
	ack_cancel_apply_guild ack;
	ack.guildid = req.guildid;
	
	this->remove(role->getInstID(), req.guildid);
	
	role->send(&ack);
}

GuildApplyMgr SGuildApplyMgr;
