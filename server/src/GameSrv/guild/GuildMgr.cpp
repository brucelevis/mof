
//
//  GuildMgr.cpp
//  GameSrv
//
//  Created by prcv on 13-4-22.
//
//

#include "GuildMgr.h"
#include "hiredis.h"
#include "main.h"
#include "Role.h"
#include "Utils.h"
#include "GuildRole.h"
#include "GuildShare.h"
#include "GameLog.h"
#include "GuildLog.h"
#include "GuildApply.h"
#include "EnumDef.h"
#include "DynamicConfig.h"
#include "Utils.h"
#include "GuildStore.h"
#include "hander_include.h"
#include "DataCfgActivity.h"
#include "BroadcastNotify.h"
#include "mail_imp.h"
#include "BossController.h"
#include "Global.h"
#include "TreasureFight.h"
#include "daily_schedule/daily_schedule_system.h"
#include "ChatRoom.h"

#include "Game.h"

#pragma mark -
#pragma mark GuildMgr implement

//可以考虑 检查数据一致性 role 下的 guild 与 guild 下的 member 是否一致 ，
//有可能断电什么的造成不一致，写数据写到一半，如果不一致，定时修复 auto_safe_sync
//要检查GuildAllId 下的 是否存在

//心跳
void updateGuild(void* param)
{
    SGuildMgr.update();
}
addTimerDef(updateGuild,NULL, GUILD_UPDATE_SECOND , FOREVER_EXECUTE);

void updateAllGuildMemberPosition(void* param)
{
    SGuildMgr.updateGuildMemberPosition();
}
addTimerDef(updateAllGuildMemberPosition,NULL, GUILD_POSITION_UPDATE_SECOND , FOREVER_EXECUTE);


static int allocGuildId()
{
    RedisResult result(redisCmd("incr gen_guildid"));
    return result.readInt();
}

static void sendRoleGuildAndGuildShareData(int roleid)
{
    SRoleGuildMgr.getRoleGuild(roleid).sendPlayerGuildDetail();
    SGuildMgr.getRoleGuild(roleid).sendGuildInfo(roleid);
}

GuildMgr SGuildMgr;

GuildMgr::GuildMgr()
{
    mDummyGuild = new Guild();
}
GuildMgr::~GuildMgr()
{
    SafeDelete(mDummyGuild);
}

void GuildMgr::init()
{
    mGuildAllId.load(0);
    
    vector<int> err;
    
    GuildAllId::iterator itId = mGuildAllId.begin();
    for (;itId != mGuildAllId.end(); ++itId)
    {
        int id = itId->first;
        Guild guild(id);
        if(guild.isNull())
        {
            //xyerr("公会id = %d 出错，自动删除",id);
            xyerr(GameTextCfg::getString("1021").c_str(),id);
            err.push_back(id);
            continue;
        }
        
        if( guild.getMembers().size() == 0)
        {
            xyerr("公会id=%d 没人了，删了公会",id);
            err.push_back(id);
            continue;
        }
        
        mGuilds.insert(make_pair(id, guild));
    }
    
    for (int i = 0 ; i< err.size() ; ++i) {
        //xyerr("从全部公会列表中删除公会%d",err[i]);
        xyerr(GameTextCfg::getString("1022").c_str(),err[i]);
        mGuildAllId.remove( err[i] );
        destroyGuild( err[i] );
    }
   
    update();
}

void GuildMgr::release()
{
    //mGuildAllId.clear(); // 2013.11.27手贱，导致全部列表被删，苹果更新1.6版本拖了3个小时
    SafeDelete(mDummyGuild);
}

Guild& GuildMgr::getGuild(const char* gname)
{
    GuildNameMap::iterator iter = mNameIndex.find(gname);
    if(iter == mNameIndex.end())
        return *mDummyGuild;
    
    return getGuild(iter->second);
}

Guild& GuildMgr::getGuild(int guildid)
{
    GuildMap::iterator iter = mGuilds.find(guildid);
    if(iter != mGuilds.end())
        return iter->second;
        
    GuildProp tmp;
    tmp.active(guildid);
    if( ! isRedisKeyExist(tmp.dbkey().c_str()) )
        return *mDummyGuild;
    
    mGuilds.insert(make_pair(guildid, Guild(guildid)));
    return mGuilds[guildid];
}

Guild& GuildMgr::getRoleGuild(int roleid)
{
    RoleGuildProperty& prop = SRoleGuild(roleid);
    if(prop.isNull())
        return *mDummyGuild;
    return getGuild( prop.getGuild() );
}



bool GuildMgr::roleHasRight(int roleid, int right)
{
    RoleGuildProperty& prop = SRoleGuild(roleid);
    if ( prop.noGuild())
        return false;
    
    return isGuildPosHasManagerRight(prop.getPosition(),right);
}

void GuildMgr::buildGuildNameIndex()
{
    mNameIndex.clear();
    for (GuildMap::iterator itGuild = mGuilds.begin();
         itGuild != mGuilds.end(); ++itGuild) {
        mNameIndex[itGuild->second.getGuildName()] = itGuild->first;
    }
}

bool CmpGuildFunc(const GuildPointer & left , const GuildPointer & right)
{
    //等级高的排前
    if ( left->getLevel() > right->getLevel())
        return true;
    if ( left->getLevel() < right->getLevel() )
        return false;
    
    //财富的排前
    if ( left->getFortune() > right->getFortune())
        return true;
    if ( left->getFortune() < right->getFortune())
        return false;
    
    //老的排前
    if ( left->getCreateDate() < right->getCreateDate() )
        return true;
    
    return false;
}

void GuildMgr::buildGuildRank()
{
    vector<GuildPointer> vec;
    for (GuildMap::iterator itGuild = mGuilds.begin();
         itGuild != mGuilds.end(); ++itGuild) {
        vec.push_back( & (itGuild->second) );
    }
    sort(vec.begin(), vec.end(), CmpGuildFunc);
    mGuildRank.clear();
    for (int i = 0 ,ni = vec.size(); i < ni ; ++i) {
        mGuildRank.push_back( vec[i]->getId() );
    }
}

static void sendDissmissGuildMail(const char* guildname , const char* rolename)
{
    MailFormat *f =  MailCfg::getCfg("guild_dismiss");
    if (f)
    {
        string mail_content = f->content;
        if (1 != find_and_replace(mail_content, 1, guildname))
        {
            log_warn("mail content format error. [guild_dissmiss]");
        }

        sendMail(0, f->sendername.c_str(), rolename, f->title.c_str(), mail_content.c_str(), "", "");
    }
}

void GuildMgr::kickAllMember(int guildid)
{
    vector<int> members;
    Guild& guild = getGuild(guildid);
    for (Guild::GuildMember::iterator itMember = guild.getMembers().begin();
         itMember != guild.getMembers().end(); ++itMember)
    {
        members.push_back(itMember->first);
    }
    
    
    for (int i = 0; i< members.size(); ++i)
    {
        RoleGuildProperty& prop = SRoleGuild(members[i]);
        sendDissmissGuildMail(guild.getGuildName().c_str(), prop.getRoleName().c_str());
        roleLeaveGuild(guildid,members[i]);
    }
}

void GuildMgr::destroyGuild(int guildid)
{    
    kickAllMember(guildid);
    
    getGuild(guildid).destroy();

    mGuilds.erase(guildid);
    mGuildAllId.remove(guildid);
    
    SGuildApplyMgr.clearApplicant(guildid);
}

void GuildMgr::saveAllGuildProp()
{
    for (GuildMap::iterator itGuild = mGuilds.begin();
         itGuild != mGuilds.end(); ++itGuild)
    {
        itGuild->second.update();
    }
}

void GuildMgr::dissmissExpiredGuild()
{
    int curTime = time(0);
    vector<int> glist;
    for (GuildMap::iterator itGuild = mGuilds.begin();
         itGuild != mGuilds.end(); ++itGuild)
    {
        if(itGuild->second.getDissmissDate() == 0)
            continue;
        if(itGuild->second.getDissmissDate() < curTime)
            glist.push_back(itGuild->first);
    }
    for (int i = 0; i< glist.size(); ++i) {
        destroyGuild(glist[i]);
    }
}

void GuildMgr::transferMaster(int guildId)
{
	Guild& guild = getGuild(guildId);
	
	int memberId = guild.getTransferMasterId();
	if(!guild.getMembers().exist(memberId)) {
		return;
	}
	
	int masterId = guild.getMaster();
	RoleGuildProperty& prop = SRoleGuild(masterId);
	RoleGuildProperty& targetProp = SRoleGuild(memberId);
	
	targetProp.setPosition(kGuildMaster);
	targetProp.save();
	
	prop.setPosition(kGuildMember);
	prop.save();
	
	guild.setTransferMasterDate(0);
	guild.setTransferMasterId(0);
	guild.setMaster(memberId);
	guild.calRankAndPosition();
	guild.save();
	
	guild.sendGuildInfoAllMember();
	
	vector<int> tmp;
	tmp.push_back(masterId);
	tmp.push_back(memberId);
	for (int i = 0; i< tmp.size(); ++i)
	{
		Role* role = SRoleMgr.GetRole(tmp[i]);
		if(role)
		{
			role->CalcPlayerProp();
			sendRoleGuildAndGuildShareData(tmp[i]);
		}
	}

	prop.sendPlayerGuildDetail();
	targetProp.sendPlayerGuildDetail();
	
	notify_transfer_guild_master nty;
    nty.oldmaster = masterId;
    nty.newmaster = memberId;
    guild.sendAllMemberPacket(&nty);
	
	Xylog log(eLogName_GuildCommon, masterId);
    log << "转让会长" << memberId << guild.getGuildName() << guild.getId();
}

void GuildMgr::dissmissExpiredTransferMaster()
{
	int curTime = time(0);
    for (GuildMap::iterator itGuild = mGuilds.begin();
         itGuild != mGuilds.end(); ++itGuild)
    {
		if(itGuild->second.getTransferMasterDate() == 0)
            continue;
		if(itGuild->second.getTransferMasterDate() < curTime)
		{
			transferMaster(itGuild->first);
		}
	}
}

void GuildMgr::update()
{
    dissmissExpiredGuild();
	
	dissmissExpiredTransferMaster();

    buildGuildNameIndex();
    
    buildGuildRank();
    
    saveAllGuildProp();
    
    SRoleGuildMgr.saveAllRoleGuildProperty();
    
    SGuildApplyMgr.clearExpiredApplication();
    
}

void GuildMgr::updateGuildMemberPosition()
{
    for (GuildMap::iterator itGuild = mGuilds.begin();
         itGuild != mGuilds.end(); ++itGuild)
    {
        itGuild->second.calRankAndPosition();
    }
}

void GuildMgr::on_req_guild_list(Role* role , req_guild_list& req)
{
    xyassert(role);
    ack_guild_list ack;
    
    std::pair<int, int> range = checkPageRange(mGuildRank.size(), req.beginrank, req.num);
    ack.beginrank = range.first;
    
    for (int i = range.first; i < range.second; ++i)
    {
        int guildid = mGuildRank[i];
        Guild& guild = getGuild(guildid);
        obj_guild_info obj;
        obj.rank = i;
        obj.guildid = guild.getId();
        obj.guildname = guild.getGuildName();
        obj.level = guild.getLevel();
        obj.membernum = guild.getMembers().size();
        int masterid  = guild.getMaster();
        obj.mastername = SRoleGuild(masterid).getRoleName();
        obj.guildfortune = guild.getFortune();
        
        ack.guilds.push_back(obj);
    }
    sendNetPacket(role->getSessionId(),&ack);
}

bool GuildMgr::checkCreateGuildConditionOk(Role* role,const string& guildname)
{
    ack_create_guild ack;
    ack.errorcode = kCreateGuildResult_Success;
    RoleGuildProperty& prop = SRoleGuild( role->getInstID() );
    
    do{
        GuildCreateDef* pGuildCreateDef = GuildCfgMgr::getCreateDef();
        if(NULL == pGuildCreateDef)
        {
            xyerr("GuildCreateDef cfg ERROR");
            ack.errorcode = kCreateGuildResult_Fail;
            break;
        }
        
        if (prop.getGuild() != 0)
        {
            ack.errorcode = kCreateGuildResult_Fail;
            break;
        }
        
        if( mNameIndex.count(guildname) )
        {
            ack.errorcode = kCreateGuildResult_NameExist;
            break;
        }
        
        if (role->getLvl() < pGuildCreateDef->needLvl)
        {
            ack.errorcode = kCreateGuildResult_LevelLow;
            break;
        }
        
        if( role->getGuildLeftTime() + GuildCfgMgr::getWaitApplySecond() > time(0))
        {
            ack.errorcode = kCreateGuildResult_NeedToWait;
            break;
        }
        
        if ( role->CheckMoneyEnough(pGuildCreateDef->needGlod, eGoldCost,"GuildCreate") != CE_OK)
        {
            ack.errorcode = kCreateGuildResult_NeedMoreGold;
            break;
        }
        

        
    }while(0);
    
    sendNetPacket(role->getSessionId(),&ack);
    
    return kCreateGuildResult_Success == ack.errorcode ;
}

int GuildMgr::createNewGuild(int master , const string& guildname)
{
    int newGuildId = allocGuildId();
    mGuildAllId.set(newGuildId, newGuildId);
    
    Guild newGuild(newGuildId);
    
    //newGuild.active(newGuildId);
    newGuild.setGuildName(guildname);
    newGuild.setCreateDate( time(0) );
    newGuild.setDissmissDate(0);
    newGuild.setMaster(master);
    newGuild.setNotice("");
    newGuild.setFortune(0.0f);
    newGuild.setConstruction(0.0f);
    newGuild.setDonateRmbNum(0);
    newGuild.save();
    
    newGuild.getMembers().set(master, master);
    
    newGuild.calRankAndPosition();
    
    mGuilds.insert(make_pair(newGuildId, newGuild));
    
    newGuild.appendLog(GameTextCfg::getString("1001").c_str());
    return newGuildId;
}

void initMasterDataAfterCreateGuild(int master, int guildid)
{
    RoleGuildProperty& prop = SRoleGuild(master);

    prop.setGuild(guildid);
    prop.setPosition(kGuildMaster);
    prop.setJoinDate( time(0) );
    prop.clearExploit("创建公会");
    prop.save();
    
    SGuildApplyMgr.clearApply(master);
    
    Role* role = SRoleMgr.GetRole(master);
    if(role)
    {
        role->CalcPlayerProp();
        role->setGuildid(guildid);
    }
    sendRoleGuildAndGuildShareData(master);
}

void broadcastCreateGuild(const string& rolename,const string& guildname)
{
    SystemBroadcast bc;
    /*
    bc.append(COLOR_ROLE_NAME, rolename.c_str());
    bc.append(COLOR_WHITE, "创建了");
    bc.append(COLOR_GREEN, guildname.c_str());
    bc.append(COLOR_WHITE, "公会，想要加入的朋友赶紧点击公会按钮搜索吧！");
     */
    string str = SystemBroadcastCfg::getValue("create_guild", "content");
    //assert(str.empty() != true);
    bc << str.c_str() << rolename.c_str() << guildname.c_str();
    bc.send();
}

//创建公会
void GuildMgr::on_req_create_guild(Role* role, req_create_guild& req)
{
    xyassert(role);
    
    if( ! checkCreateGuildConditionOk(role,req.guildname) )
        return;
        
    int newGuildId = createNewGuild(role->getInstID(), req.guildname);
    
    initMasterDataAfterCreateGuild(role->getInstID(), newGuildId);
    
    broadcastCreateGuild(role->getRolename(),req.guildname);
    
    //增加进全部公会列表
    update();
    
    Xylog log(eLogName_GuildCreateAndDissmiss, role->getInstID());
    log << "Create" << req.guildname << newGuildId;
}

//解散公会
void GuildMgr::on_req_dismiss_guild(Role* role,req_dismiss_guild& req)
{
    xyassert(role);
    int roleid = role->getInstID();
    ack_dismiss_guild ack;
    
    if ( ! roleHasRight(roleid,kGuildDismissGuild) ) {
		ack.errorcode = eDismissGuildResult_NoPermission;
        sendNetPacket(role->getSessionId(),&ack);
		return;
	}
    
    Guild& guild = getRoleGuild(roleid);
	
	if (guild.getTransferMasterDate() != 0) {
		ack.errorcode = eDismissGuildResult_TransferMaster;
        sendNetPacket(role->getSessionId(),&ack);
		return;
	}
	
	if (guild.getDissmissDate() != 0){
		ack.errorcode = eDismissGuildResult_DismissGuild;
        sendNetPacket(role->getSessionId(),&ack);
		return;
	}

	int date = time(0) + GUILD_DISSMISS_SECOND;
	ack.dissmissdate = GUILD_DISSMISS_SECOND;
	guild.setDissmissDate(date);
	
	guild.appendLog(xystr(GameTextCfg::getFormatString("1003", GUILD_DISSMISS_SECOND*1.0/3600).c_str()), eGuildLogLvl_C);
	
	guild.sendGuildInfoAllMember();
	
	Xylog log(eLogName_GuildCreateAndDissmiss,role->getInstID());
	log<< "SetDissmiss" << guild.getGuildName() << guild.getId();

	ack.errorcode = eDismissGuildResult_Success;
	sendNetPacket(role->getSessionId(),&ack);
}

//取消解散
void GuildMgr::on_req_cancle_dismiss_guild(Role* role, req_cancle_dismiss_guild& req)
{
    xyassert(role);
    int roleid = role->getInstID();
    ack_cancle_dismiss_guild ack;
    
    if ( ! roleHasRight(roleid,kGuildDismissGuild) )
        return;
    
    Guild& guild = getRoleGuild(roleid);
    
    if (guild.getDissmissDate() != 0)
    {
        guild.setDissmissDate(0);
        ack.errorcode = 0;
        sendNetPacket(role->getSessionId(),&ack);
        guild.sendGuildInfoAllMember();
        
        // guild.appendLog("取消解散公会");
        guild.appendLog(GameTextCfg::getString("1004"));

        
        Xylog log(eLogName_GuildCreateAndDissmiss,role->getInstID());
        log << "CancleDissmiss" << guild.getGuildName() << guild.getId();
    }
}

// 取消转让
void GuildMgr::on_req_cancle_transfer_master(Role* role, req_cancle_transfer_master& req)
{
	xyassert(role);
    int roleid = role->getInstID();
    ack_cancle_transfer_master ack;
	
	if ( ! roleHasRight(roleid, kGuildTransferMaster) )
	{
		ack.errorcode = eTransferMasterResult_NoPermission;
		sendNetPacket(role->getSessionId(), &ack);
		return;
	}
	
	Guild& guild = getRoleGuild(roleid);
    
    if (guild.getTransferMasterDate() != 0)
    {
		guild.setTransferMasterDate(0);
		guild.setTransferMasterId(0);
		guild.save();
        guild.sendGuildInfoAllMember();
		
		guild.appendLog(GameTextCfg::getString("1052"));
		
		Xylog log(eLogName_GuildCommon, role->getInstID());
		log << "取消转让会长" << guild.getGuildName() << guild.getId();
	}
	ack.errorcode = eTransferMasterResult_Success;
	sendNetPacket(role->getSessionId(), &ack);
}

//搜索公会
void GuildMgr::on_req_search_guild(Role* role, req_search_guild& req)
{
    xyassert(role);
    ack_search_guild ack;
    
    Guild& guild = getGuild(req.guildname.c_str());
    if( guild.isNull())
    {
        ack.errorcode = 1;
        return sendNetPacket(role->getSessionId(),&ack);
    }
    
    int rank = 0;
    for (; rank < mGuildRank.size(); ++rank) {
        if(mGuildRank[rank] == guild.getId())
            break;
    }
    
    
    ack.errorcode = 0;
    ack.guild.rank = rank;
    ack.guild.guildid = guild.getId();
    ack.guild.guildname = guild.getGuildName();
	ack.guild.guildfortune = guild.getFortune();
    ack.guild.level = guild.getLevel();
    ack.guild.membernum = guild.getMembers().size();
    ack.guild.mastername = SRoleGuild( guild.getMaster() ).getRoleName();

    sendNetPacket(role->getSessionId(),&ack);
}

//查看本人公会
void GuildMgr::on_req_guild_info(Role* role , req_guild_info& req)
{
    xyassert(role);
    
    Guild& guild = getRoleGuild(role->getInstID());
    
    if( guild.isNull() )
    {
        notify_guild_info ack;
        ack.guildid  = 0;
        return sendNetPacket(role->getSessionId(),&ack);
    }
    guild.sendGuildInfo(role->getInstID());
}

//查看本公会成员
void GuildMgr::on_req_guild_member_list(Role* role, req_guild_member_list& req)
{
    xyassert(role);
    Guild& guild = getRoleGuild(role->getInstID());
    if( ! guild.isNull())
        guild.sendMemberList(role->getInstID(), req);
}

//查看本人公会属性
void GuildMgr::on_req_player_guild_detail(Role* role)
{
    xyassert(role);
    RoleGuildProperty& prop = SRoleGuild( role->getInstID() );
    prop.sendPlayerGuildDetail();
}

// 检查申请者是否符合条件
bool checkApplicantOk(int mgrRoleId,ack_accept_apply_guild& ack)
{
    RoleGuildProperty& applicant = SRoleGuild( ack.applicantid );
    
    if( applicant.isNull() )
    {
        ack.errorcode = eGuildMgrAcceptApply_RoleDelete ;
        sendRolePacket(mgrRoleId,&ack);
        return false;
    }
    
    if( ! applicant.noGuild() )
    {
        ack.errorcode = eGuildMgrAcceptApply_RoleHasGuild ;
        sendRolePacket(mgrRoleId,&ack);
        return false;
    }
    return true;
}

bool checkGuildMemberNotFull(int mgrRoleId, ack_accept_apply_guild& ack)
{
    Guild& guild =   SGuildMgr.getRoleGuild(mgrRoleId);

    if(guild.getMembers().size() >= guild.getMaxMemberSize())
    {
        ack.errorcode = eGuildMgrAcceptApply_MemNumLimit;
        sendRolePacket(mgrRoleId,&ack);
        return false;
    }
    return true;
}

bool checkAndRemoveApplicatList(int mgrRoleId, ack_accept_apply_guild& ack)
{
    Guild& guild =   SGuildMgr.getRoleGuild(mgrRoleId);
    
    bool bRet = SGuildApplyMgr.remove(ack.applicantid,guild.getId());
    if( ! bRet)
    {
        ack.errorcode = eGuildMgrAcceptApply_NotInList;
        sendRolePacket(mgrRoleId,&ack);
        return false;
    }
    return true;
}

void initRoleGuildDataAfterJoinGuild(int roleid, int guildid)
{
    RoleGuildProperty& prop = SRoleGuild( roleid );

    prop.setGuild(guildid);
	prop.setPosition(kGuildMember);
    prop.setJoinDate(time(0));
    prop.clearExploit("加入公会");
    prop.save();
    
    SGuildApplyMgr.clearApply(roleid);
    
    Role* role = SRoleMgr.GetRole(roleid);
    if(role)
    {
        role->CalcPlayerProp();
        role->setGuildid(guildid);
    }
};

static void notifyNewMember(int memberid, const string& memberName,int guildid)
{
    Guild& guild = SGuildMgr.getGuild(guildid);
    
    Guild::GuildMember::iterator itMember = guild.getMembers().begin();
    for ( ; itMember != guild.getMembers().end(); ++itMember)
    {
        Role* role = SRoleMgr.GetRole( itMember->first );
        if(NULL == role)
            continue;
        notify_guild_new_member nb;
        nb.guildid = guild.getId();
        nb.guildname = guild.getGuildName();
        nb.memberid = memberid;
        nb.membername = memberName;
        sendNetPacket(role->getSessionId(),&nb);
    }
}

static void writeNewMemberLog(int guidid,const string& name)
{
    // SGuild(guidid).appendLog( xystr("%s加入公会",name.c_str()) );
    
    SGuild(guidid).appendLog( xystr(GameTextCfg::getFormatString("1005",name.c_str()).c_str()));
}

static void addNewMemberForGuild(int memberid, const string& memberName , int guildid)
{
    Guild& guild = SGuildMgr.getGuild(guildid);
    
    guild.getMembers().set(memberid,memberid);

    guild.calRankAndPosition();
    
    guild.sendGuildInfoAllMember();
    
    notifyNewMember(memberid,memberName,guildid);
    
    writeNewMemberLog(guildid,memberName);
}

//接受加入公会
void GuildMgr::on_req_accept_apply_guild(Role* role,req_accept_apply_guild& req)
{
    xyassert(role);
    int roleid = role->getInstID();
    ack_accept_apply_guild ack;
    ack.applicantid = req.applicantid;
    ack.applicantname = req.applicantname;
    
    if ( ! roleHasRight(roleid,kGuildAcceptApplicant) )
        return;
    
    if ( ! checkApplicantOk(roleid,ack) )
        return;
    
    if ( ! checkGuildMemberNotFull(roleid, ack) )
        return;
    
    if ( ! checkAndRemoveApplicatList(roleid,ack) )
        return;
        
    ack.errorcode = eGuildMgrAcceptApply_Success;
    sendRolePacket(roleid,&ack);
    
    Guild& guild =   SGuildMgr.getRoleGuild(roleid);
    
    addNewMemberForGuild(req.applicantid,req.applicantname, guild.getId());
    
    initRoleGuildDataAfterJoinGuild(req.applicantid, guild.getId() );
    
    Xylog log(eLogName_GuildJoinAndLeave, req.applicantid);
    log << "接受加入" << guild.getGuildName() << "管理员" << role->getRolename() << guild.getId();
}

bool checkAndRemoveApplicatListForReject(int mgrRoleId, ack_reject_apply_guild& ack)
{
    Guild& guild =   SGuildMgr.getRoleGuild(mgrRoleId);
    
    bool bRet = SGuildApplyMgr.remove(ack.applicantid,guild.getId());
    if( ! bRet)
    {
        ack.errorcode = eGuildMgrRejectApply_NotInList;
        sendRolePacket(mgrRoleId,&ack);
        return false;
    }
    return true;
}

void notifyApplicantWasRejected(int applicantid, int guildid)
{
    notify_reject_apply_for_applicant rej;
    Role* role = SRoleMgr.GetRole(applicantid);
    if(role)
    {
        rej.guildid = guildid;
        rej.guildname = SGuild( guildid ).getGuildName();
        sendNetPacket(role->getSessionId(),&rej);
    }
}

//拒绝申请
void GuildMgr::on_req_reject_apply_guild(Role* role,req_reject_apply_guild& req)
{
    xyassert(role);
    int roleid = role->getInstID();
    ack_reject_apply_guild ack;
    ack.applicantid = req.applicantid;
    ack.applicantname = req.applicantname;
    
    if ( ! roleHasRight(roleid,kGuildAcceptApplicant) )
        return;
    
    if( ! checkAndRemoveApplicatListForReject(roleid,ack) )
        return;
    
    ack.errorcode = eGuildMgrRejectApply_Success;
    sendNetPacket(role->getSessionId(),&ack);
    
    notifyApplicantWasRejected(req.applicantid, SRoleGuild(roleid).getGuild() );
}

void notifyMemberLeaveGuild( int guildid , int roleid )
{
    Guild& guild = SGuild(guildid);
    RoleGuildProperty& prop = SRoleGuild(roleid);
    
    Guild::GuildMember::iterator itMember = guild.getMembers().begin();
    for ( ; itMember != guild.getMembers().end(); ++itMember)
    {
        Role* role = SRoleMgr.GetRole( itMember->first );
        if(NULL == role)
            continue;
        
        notify_guild_leave nb;
        nb.guildid = guild.getId();
        nb.guildname = guild.getGuildName();
        nb.roleid = roleid;
        nb.rolename = prop.getRoleName();
        sendNetPacket(role->getSessionId(),&nb);
    }
}

void writeMemberLeaveLog(int guildid,const string& memberName)
{
    // SGuild(guildid).appendLog( xystr("%s离开公会",memberName.c_str()) );
    SGuild(guildid).appendLog( xystr(GameTextCfg::getFormatString("1006",memberName.c_str()).c_str()));
}

void modifyGuildDataWhenMemberLeave(Guild& guild,int memberid,const string& memberName)
{   
    guild.getMembers().remove( memberid );
    
    guild.calRankAndPosition();
    
    guild.sendGuildInfoAllMember();
    
    writeMemberLeaveLog(guild.getId(),memberName);
}

void modifyRoleDataWhenLeaveGuild(RoleGuildProperty& prop)
{
    int guildid = prop.getGuild();
    
    prop.setGuild(0);
    string leavestr = GameTextCfg::getString("1006");
    prop.addConstrib( -1 * prop.getConstrib() * 0.5f , "离开公会");
    prop.setJoinDate( 0 );
    prop.clearExploit("离开公会");
	prop.setPosition(kGuildPositionNotset);
    prop.save();
    
    Role* role = SRoleMgr.GetRole(prop.getId());
    if(role){
		SGuildMgr.on_req_player_guild_detail(role);
        role->CalcPlayerProp();
        role->setGuildLeftTime( time(0) );
        
        //踢出工会章， 删除公会战的东西
        g_GuildTreasureFightMgr.playerLeaveGuild(role);
        role->setGuildid(0);
	}
	else
	{
        //删除公会战的东西
        g_GuildTreasureFightMgr.clearPlayerDataInDB(prop.getId(), guildid);
        
		NewRoleProperty newrole;
		newrole.load(prop.getId());
        newrole.setGuildLeftTime( time(0) );
		newrole.save();
	}

}

bool GuildMgr::roleLeaveGuild(int guildid , int roleid)
{
    RoleGuildProperty& member = SRoleGuild( roleid );
    
    const bool isGuildNotMatch =  member.getGuild() != guildid ;
    
    if( member.noGuild() || isGuildNotMatch )
        return false;
    
    Guild& guild =   getGuild( guildid );
    

    
    notifyMemberLeaveGuild(guildid,roleid);
    
    modifyGuildDataWhenMemberLeave(guild,member.getId(),member.getRoleName());

    modifyRoleDataWhenLeaveGuild(member);
    
    return true;
}



//退出公会
void GuildMgr::on_req_guild_leave(int roleid)
{   
    ack_guild_leave ack;
    
    RoleGuildProperty& prop = SRoleGuild( roleid );
    if(prop.getGuild() == 0)
        return;
    
    //会长退会
    Guild& guild = getGuild(prop.getGuild());
    if(guild.getMaster() == roleid)
    {
		// 公会还有多人,　会长不能退会
		if (guild.getMembers().size() > 1) {
			ack.errorcode = 1;
			sendRolePacket(roleid, &ack);
			return;
		}
			
		// 只剩下会长一个人，解散公会需要48小时。
		if (guild.getDissmissDate() == 0)
		{
			int date = time(0) + GUILD_DISSMISS_SECOND;
			guild.setDissmissDate(date);
			
			guild.appendLog(xystr(GameTextCfg::getFormatString("1003", GUILD_DISSMISS_SECOND/3600.0).c_str()), eGuildLogLvl_C);
			
			Xylog log(eLogName_GuildJoinAndLeave,roleid);
			log<<"GuildMasterLeave,Guild dismiss"<<guild.getGuildName()<<guild.getId();
		
			Xylog log2(eLogName_GuildCreateAndDissmiss, roleid);
            log2<< "SetDissmiss" << guild.getGuildName() << guild.getId();
		}
		
        ack.errorcode = 0;
        sendRolePacket(roleid, &ack);
        return;
    }
    
    // 一般人退会
    if( roleLeaveGuild(prop.getGuild(),roleid) )
    {
        ack.errorcode = 0;
        
        Xylog log(eLogName_GuildJoinAndLeave,roleid);
        log << "主动退出" << guild.getGuildName() << "" << "" << guild.getId();
        
		//　申请了弹劾
        resetImpeachApplicantIf(guild, roleid, "guild_impeach_three");
		
		//　是等待会长生效的会员
		if (roleid == guild.getTransferMasterId()) {
			guild.setTransferMasterDate(0);
			guild.setTransferMasterId(0);
			guild.save();
			guild.sendGuildInfoAllMember();
		}
        
    }else{
        ack.errorcode = 1;
    }
    
    sendRolePacket(roleid,&ack);
}

static void sendKickGuildMail(const char* guildname , const char* rolename , int constrib)
{    
    MailFormat *f = MailCfg::getCfg("kicked_guild");
    if (f)
    {
        //你被请出了%s公会，公会个人贡献剩余%d.
        string mail_content = f->content;
        string constrib_str = Utils::itoa(constrib);
        if (2 != find_and_replace(mail_content, 2, guildname, constrib_str.c_str()))
        {
            log_warn("mail content format error. [kicked_guild]");
        }
        
        sendMail(0, f->sendername.c_str(), rolename, f->title.c_str(), mail_content.c_str(), "", "");
    }
    
}

//踢人
void GuildMgr::on_req_guild_kick(Role* role,req_guild_kick& req)
{
    xyassert(role);
    int mgrRoleId = role->getInstID();
    ack_guild_kick ack;
    ack.memberid = req.memberid;
    
    if ( ! roleHasRight(mgrRoleId,kGuildKickMember) )
        return;
    
    Guild& guild = getRoleGuild(mgrRoleId);
    
    if( mgrRoleId == req.memberid )
    {
        ack.errorcode = eGuildKickMember_DenyKickYourself;
        return sendNetPacket(role->getSessionId(),&ack);
    }
    
    if ( guild.getMaster() == req.memberid)
    {
        ack.errorcode = eGuildKickMember_DenyKickMaster;
        return sendNetPacket(role->getSessionId(),&ack);
    }
    
    RoleGuildProperty& kickProp = SRoleGuild( req.memberid );
    RoleGuildProperty& prop = SRoleGuild( mgrRoleId );
    
    bool isViceMaster = prop.getPosition() == kGuildViceMaster;
    bool isKickViceMaster = kickProp.getPosition() == kGuildViceMaster;
    if( isViceMaster && isKickViceMaster) return;
    
    
    if( roleLeaveGuild(prop.getGuild(),req.memberid) )
    {
        ack.errorcode = eGuildKickMember_Success;
        RoleGuildProperty & kickprop = SRoleGuild(req.memberid);
        
        sendKickGuildMail(guild.getGuildName().c_str(), kickprop.getRoleName().c_str(),
                          kickprop.getConstrib());
        
        //若正好是申请人
        resetImpeachApplicantIf(getGuild(prop.getGuild()),req.memberid, "guild_impeach_four");
        
        stringstream log;
        string kickstr = GameTextCfg::getFormatString("1007", role->getRolename().c_str(), kickprop.getRoleName().c_str());
        //log<<role->getRolename()<<"将"<<kickprop.getRoleName()<<"踢出了公会";
        log << kickstr;
        guild.appendLog( log.str() );
        
        Xylog xlog(eLogName_GuildJoinAndLeave, kickprop.getId());
        xlog << "被踢" << guild.getGuildName() << "管理员" <<role->getRolename() << guild.getId();
        
		
		if (req.memberid == guild.getTransferMasterId()) {
			guild.setTransferMasterDate(0);
			guild.setTransferMasterId(0);
			guild.save();
			guild.sendGuildInfoAllMember();
		}
		
    }else{
        ack.errorcode = eGuildKickMember_NotInGuild;
    }
    return sendNetPacket(role->getSessionId(),&ack);
}

//修改公告
void GuildMgr::on_req_modify_guild_notice(Role* role, req_modify_guild_notice& req)
{
    xyassert(role);
    int mgrRoleId = role->getInstID();
    
    if ( ! roleHasRight(mgrRoleId,kGuidModifyNotice) )
        return;
    
    ack_modify_guild_notice ack;
    ack.errorcode = 0;
    sendNetPacket(role->getSessionId(),&ack);
    
    RoleGuildProperty& prop = SRoleGuild( mgrRoleId );
    Guild& guild = getGuild(prop.getGuild());
    guild.setNotice(req.notice);
    
    guild.sendGuildInfoAllMember();
    
    Xylog log(eLogName_GuildCommon,role->getInstID());
    //string str = GameTextCfg::getString("1025");
    log<< "修改公告"<< req.notice;
}

//捐献  KCDO 要重构此函数
void GuildMgr::on_req_guild_donate(Role* role,req_guild_donate& req)
{
    xyassert(role);
    int roleid = role->getInstID();
    
    RoleGuildProperty& prop = SRoleGuild( roleid );
    if(prop.getGuild() == 0)
        return;
    
    if( req.type <= GuildDonateType_Begin ||
        req.type >= GuildDonateType_End)
        return;
    
    Guild& guild = getGuild( prop.getGuild() );
    ack_guild_donate ack;
    ack.type = req.type;
    
    int addRoleContrib = 0;
    int addRoleExploit = 0;
    int addGuildFortune = 0;
    int addGuildConstruction = 0;
    int donateval = 0;
    string donatename;
    
    GuildLvlDef* pGuildLvlDef = GuildCfgMgr::getGuildLvlDef(guild.getLevel());
    if(NULL == pGuildLvlDef)
    {
        xyerr(" GuildMgr::on_req_guild_donate pGuildLvlDef error");
        ack.errorcode = eGuildDonateResult_UnknowError;
        return sendNetPacket(role->getSessionId(),&ack);
    }
    
    GuildPresentRmbDef& pGuildPresentRmbDef = GuildCfgMgr::getGuildPresentRmbDef();
    
    //公会金钻
    if(GuildDonateType_Rmb == req.type)
    {
        if(guild.getDonateRmbNum() >= pGuildLvlDef->donateRmbNum)
        {
            ack.errorcode = eGuildDonateResult_RmbNumLimit;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        
        if (role->getVipLvl() < pGuildLvlDef->donateRmbVipLvl) {
            ack.errorcode = eGuildDonateResult_NeedHigherVipLvl;
            return sendNetPacket(role->getSessionId(), &ack);
        }
        
        addRoleContrib = pGuildPresentRmbDef.rmbaddcontribute;
        addRoleExploit = pGuildPresentRmbDef.rmbaddfeats;
        addGuildFortune = pGuildPresentRmbDef.rmbaddwealth;
        addGuildConstruction = pGuildPresentRmbDef.rmbaddoverbuild;
        
        donateval = pGuildPresentRmbDef.donateRmb;
        //donatename = MoneyMLCfg::getValue("10003", "desc");//"金钻";
        donatename = MoneyCfg::getName("rmb");
        
        if ( role->CheckMoneyEnough(donateval, eRmbCost, "GuildDonate") != CE_OK)
        {
            ack.errorcode = eGuildDonateResult_NeedMoreRmb;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        
        guild.setDonateRmbNum( guild.getDonateRmbNum() + 1);
    }
    
    //个人金钻
    if(GuildDonateType_SelfRmb == req.type)
    {
        if( role->getGuildDnoateNum() >= pGuildLvlDef->donateGoldBatNum)
        {
            ack.errorcode = eGuildDonateResult_DnoateGoldBatNumLimit;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        
        if (role->getVipLvl() < pGuildLvlDef->donateRmbVipLvl) {
            ack.errorcode = eGuildDonateResult_NeedHigherVipLvl;
            return sendNetPacket(role->getSessionId(), &ack);
        }
        
        GuildPresentDef* pGuildPresentDef = GuildCfgMgr::getGuildSelfPresentDef( role->getGuildDnoateNum() );
        if(NULL == pGuildPresentDef)
        {
            xyerr(" GuildMgr::on_req_guild_donate pGuildPresentDef error");
            ack.errorcode = eGuildDonateResult_UnknowError;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        
        addRoleContrib = pGuildPresentDef->addcontribute;
        addRoleExploit = pGuildPresentDef->addfeats;
        addGuildFortune = pGuildPresentDef->addwealth;
        addGuildConstruction = pGuildPresentDef->addoverbuild;
        
        donateval = pGuildPresentDef->donateRmbSelf;
        //donatename = MoneyMLCfg::getValue("10003", "desc");//"金钻";
        donatename = MoneyCfg::getName("rmb");
        
        if ( role->CheckMoneyEnough(donateval, eRmbCost,"GuildDonate") != CE_OK)
        {
            ack.errorcode = eGuildDonateResult_NeedMoreRmb;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        
        role->setGuildDnoateNum( role->getGuildDnoateNum() + 1);
    }
    
    //个人金币
    if(GuildDonateType_Gold == req.type)
    {
        if( role->getGuildDnoateNum() >= pGuildLvlDef->donateGoldBatNum)
        {
            ack.errorcode = eGuildDonateResult_DnoateGoldBatNumLimit;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        
        GuildPresentDef* pGuildPresentDef = GuildCfgMgr::getGuildSelfPresentDef( role->getGuildDnoateNum() );
        if(NULL == pGuildPresentDef)
        {
            xyerr(" GuildMgr::on_req_guild_donate pGuildPresentDef error");
            ack.errorcode = eGuildDonateResult_UnknowError;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        
        addRoleContrib = pGuildPresentDef->goldaddcontribute;
        addRoleExploit = pGuildPresentDef->goldaddfeats;
        
        donateval = pGuildPresentDef->donateGold;
        //donatename = MoneyMLCfg::getValue("10001", "desc");//"金币";
        donatename = MoneyCfg::getName("gold");
        
        if ( role->CheckMoneyEnough(donateval, eGoldCost,"GuildDonate") != CE_OK)
        {
            ack.errorcode = eGuildDonateResult_NeedMoreGold;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        
        role->setGuildDnoateNum( role->getGuildDnoateNum() + 1);
    }
    
    //战功
    if(GuildDonateType_BattlePoint == req.type)
    {
        return;//直接返回
        
        if( role->getGuildDnoateNum() >= pGuildLvlDef->donateGoldBatNum)
        {
            ack.errorcode = eGuildDonateResult_DnoateGoldBatNumLimit;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        
        GuildPresentDef* pGuildPresentDef = GuildCfgMgr::getGuildPresentDef( role->getGuildDnoateNum() );
        if(NULL == pGuildPresentDef)
        {
            xyerr(" GuildMgr::on_req_guild_donate pGuildPresentDef error");
            ack.errorcode = eGuildDonateResult_UnknowError;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        
        
        addRoleContrib = pGuildPresentDef->addcontribute;
        addRoleExploit = pGuildPresentDef->addfeats;
        addGuildFortune = pGuildPresentDef->addwealth;
        addGuildConstruction = pGuildPresentDef->addoverbuild;
        
        donateval = pGuildPresentDef->donateBat;
        //donatename = MoneyMLCfg::getValue("10002", "desc");//"战功";
        donatename = MoneyCfg::getName("batpoint");
        
        if( role->getBattlePoint() < donateval)
        {
            ack.errorcode = eGuildDonateResult_NeedMoreBattlePoint;
            return sendNetPacket(role->getSessionId(),&ack);
        }
        role->addBattlePoint(- donateval , "GuildDonate");
        
        role->setGuildDnoateNum( role->getGuildDnoateNum() + 1);
    }
    
    int oldlevel = guild.getLevel();
    
    string str = GameTextCfg::getString("1009");
    prop.addConstrib( addRoleContrib , "公会捐献");
    prop.addExploit( addRoleExploit , "公会捐献");
    
    prop.sendPlayerGuildDetail();
    
    string donatestr = GameTextCfg::getString("1026");
    guild.addFortuneConstruction(addGuildFortune, addGuildConstruction,
                                 role->getRolename().c_str(),donatestr.c_str());//"捐献");
    

    
    ack.errorcode = eGuildDonateResult_Success;
    sendNetPacket(role->getSessionId(),&ack);
    
    if(0 != donateval)
    {
        stringstream ss;
        string donatestr = GameTextCfg::getFormatString("1010", role->getRolename().c_str(), donateval, donatename.c_str());
        //ss<<role->getRolename()<<"捐献了"<<donateval<<donatename;
        ss << donatestr;
        guild.appendLog(ss.str());
    }
    
    if( guild.getLevel() != oldlevel)
    {
        SystemBroadcast bc;
        // bc.append(COLOR_GREEN, guild.getGuildName().c_str());
        //bc.append(COLOR_WHITE, "公会升到了%d级，能容纳更多的成员了，想要加入的朋友点击公会按钮申请吧！", guild.getLevel());
        string str = SystemBroadcastCfg::getValue("guild_levelup", "content");
        bc << str.c_str() << guild.getGuildName().c_str() << guild.getLevel();
        bc.send();
        
        
        guild.appendLog( xystr(GameTextCfg::getFormatString("1011", guild.getLevel()).c_str()));//"公会升到了%d级",guild.getLevel()) );
        guild.calRankAndPosition();
    }
    
    Xylog log(eLogName_GuildDonate,role->getInstID());
    log << donateval << donatename << guild.getId() << guild.getGuildName();
    
    prop.save();
    guild.update();
    role->saveNewProperty();
    
    if (role->getDailyScheduleSystem()) {
        role->getDailyScheduleSystem()->onEvent(kDailyScheduleGuildDonate, 0, 1);
    }
}

//转让会长
void GuildMgr::on_req_transfer_guild_master(Role* role, req_transfer_guild_master& req)
{
    xyassert(role);
	ack_transfer_guild_master ack;
    if ( ! roleHasRight(role->getInstID(),kGuildTransferMaster)) {
		ack.errorcode = eTransferMasterResult_NoPermission;
		sendNetPacket(role->getSessionId(), &ack);
		return;
	}
    
    RoleGuildProperty& prop = SRoleGuild( role->getInstID() );
    Guild& guild = getGuild( prop.getGuild());
    xyassert(guild.getId());
    
    if( ! guild.getMembers().exist(req.memberid) ) {
		ack.errorcode = eTransferMasterResult_NoMember;
		sendNetPacket(role->getSessionId(), &ack);
        return;
	}
	
	if (guild.getTransferMasterDate() != 0){
		ack.errorcode = eTransferMasterResult_TransferMaster;
		sendNetPacket(role->getSessionId(), &ack);
        return;
	}
		
	if (guild.getDissmissDate() != 0) {
		ack.errorcode = eTransferMasterResult_DismissGuild;
		sendNetPacket(role->getSessionId(), &ack);
		return;
	}
	
	guild.setTransferMasterDate(Game::tick + GUILD_APPLY_EXPIRED_SECOND);
	guild.setTransferMasterId(req.memberid);
	guild.save();
	
	ack.errorcode = eTransferMasterResult_Success;
    sendNetPacket(role->getSessionId(), &ack);
	
	guild.sendGuildInfoAllMember();
	
	stringstream ss;
	string str = GameTextCfg::getFormatString("1013",
											  GUILD_APPLY_EXPIRED_SECOND * 1.0 / 3600,
											  SRoleGuild(role->getInstID()).getRoleName().c_str(),
											  SRoleGuild(req.memberid).getRoleName().c_str());
	
	ss << str;
	guild.appendLog(ss.str(), eGuildLogLvl_C);
    
    Xylog log(eLogName_GuildCommon,role->getInstID());
    log << "转让会长" << req.memberid << guild.getGuildName() << guild.getId();
}


// 公会商店
void GuildMgr::on_req_guild_store(Role* role, req_guild_store& req)
{
	xyassert(role);
	ack_guild_store ack;
	ack.errorcode = StoreListReuslt_Success;
    ack.nextTm = 0;
    ack.updateNum = 0;
    do
    {
		GuildStoreMgr *gsMgr = role->getGuildStoreMgr();
		if (!gsMgr->checkIsOpen()) {
			ack.errorcode = StoreListReuslt_NoOpen;
            break;
        }
		// 更新列表
		gsMgr->checkUpdtaeList();
        // 获取列表
        StoreGoodsTd::const_iterator begin = gsMgr->begin();
        StoreGoodsTd::const_iterator end = gsMgr->end();
        for (; begin != end; begin++) {
            if (begin->second.isBuy) {
                continue;
            }
            obj_store_goods_info info;
            info.index = begin->second.index;
            info.goodsId = begin->second.goodsId;
            info.goodsNum = begin->second.goodsNum;
            info.type = begin->second.type;
            info.price = begin->second.price;
            info.isBuy = begin->second.isBuy;
            ack.info_arr.push_back(info);
        }
		ack.updateNum = gsMgr->getUpdateNum();
        ack.nextTm = gsMgr->getProcUpdateTime();
		ack.errorcode = StoreListReuslt_Success;
	} while (false);
	sendNetPacket(role->getSessionId(), &ack);
}

void GuildMgr::on_req_guild_storeUpdate(Role* role, req_guild_storeUpdate& req)
{
	xyassert(role);
    int mgrRoleId = role->getInstID();
	ack_guild_storeUpdate ack;
	ack.errorcode = StoreManuallyListReuslt_Success;
    ack.costrmb = 0;
    ack.updateNum = 0;
	do
    {
		GuildStoreMgr *gsMgr = role->getGuildStoreMgr();
		if (!gsMgr->checkIsOpen()) {
			ack.errorcode = StoreManuallyListReuslt_NoOpen;
            break;
        }
        
        if (gsMgr->isTimesUseUp(role->getVipLvl()))
        {
            ack.errorcode = StoreManuallyListReuslt_RefreshTimesIsUseUp;
            break;
        }
        
		// 用完免费次数, 扣费
        if (!gsMgr->isFreeUpdate()) {
            int cost = gsMgr->getUpdateNeedRmb();
            string str = GameTextCfg::getString("1014");
            if (role->CheckMoneyEnough(cost, eRmbCost, str.c_str() /*"手动刷新公会商店"*/) != CE_OK)
            {
                ack.errorcode = StoreManuallyListReuslt_NoRmb;
                break;
            }
            ack.costrmb = cost;
        }
		// 刷新列表
		gsMgr->manuallyUpdateList();
		ack.updateNum = gsMgr->getUpdateNum();
		ack.errorcode = StoreManuallyListReuslt_Success;
	}while (false);

	sendNetPacket(role->getSessionId(), &ack);
}

void GuildMgr::on_req_guild_storeBuy(Role* role, req_guild_storeBuy& req)
{
	xyassert(role);
    int roleid = role->getInstID();
	ack_guild_storeBuy ack;
	ack.errorcode = StoreBuyReusltReuslt_UnknowError;
	ack.indexId = req.indexId;
	ack.cost = 0;
    ack.costType = 0;
	do
    {
		GuildStoreMgr *gsMgr = role->getGuildStoreMgr();
		if (!gsMgr->checkIsOpen()) {
            ack.errorcode = StoreBuyReusltReuslt_NoOpen;
            break;
        }
		if (gsMgr->checkIsTimeout()) {
            ack.errorcode = StoreBuyReusltReuslt_Timeout;
            break;
        }
		StoreGoods *sg = gsMgr->getStoreGoods(req.indexId);
		if (sg == NULL) {
            xyerr("Guild StoreGoods : role %d buy err index %d", roleid, req.indexId);
            ack.errorcode = StoreBuyReusltReuslt_ErrIndexId;
            break;
        }
		if (sg->isBuy) {
            ack.errorcode = StoreBuyReusltReuslt_Buy;
            break;
        }
		ItemCfgDef* cfg = ItemCfg::getCfg(sg->goodsId);
        if (NULL == cfg) {
            break;
        }
		// 检查背包
		ItemArray items;
		GridArray effgrids;
		items.push_back(ItemGroup(sg->goodsId, sg->goodsNum));

        if(role->preAddItems(items, effgrids) != CE_OK){
			ack.errorcode = StoreBuyReusltReuslt_BagFull;
			break;
		}
        // 扣费
		if (sg->type == enStoreGoodsType_ConsumeCost) {
			RoleGuildProperty& prop = SRoleGuild(roleid);
            string str = GameTextCfg::getString("1015");
			if (!prop.addConstrib(-sg->price , "公会商店购物")) {
				ack.errorcode = StoreBuyReusltReuslt_NoCost;
				break;
			}
			
		} else {
            string str = GameTextCfg::getString("1016");
			if ( role->CheckMoneyEnough(sg->price, sg->type, str.c_str()/*"公会商店物品"*/) != CE_OK) {
				ack.errorcode = StoreBuyReusltReuslt_NoCost;
				break;
			}
		}
        ack.cost = sg->price;
        ack.costType = sg->type;
        // 是直接使用的物品
        bool isOpen = false;
        if (cfg->ReadInt("type") == kItemConsumable) {
            string affect_type1 = cfg->ReadStr("affect_type1");
            if (strcmp(affect_type1.c_str(), "incr_gold") == 0 ||
                strcmp(affect_type1.c_str(), "incr_exp") == 0 ||
                strcmp(affect_type1.c_str(), "incr_batpoint") == 0 ||
                strcmp(affect_type1.c_str(), "incr_constell") == 0)
            {
                isOpen = true;
            }
        }
        // 添加物品到玩家
        if (isOpen)
        {
            if (useConsumeItem(cfg, role, sg->goodsNum)) {
                xyerr("Guild StoreGoods : role %d use item err %d", roleid, sg->goodsId);
                break;
            }
        }
        else
        {
            //BackBag::UpdateBackBag(roleid, effgrids);
//            role->updateBackBag(effgrids, items, true, "buyGuildStoreGoods");
            role->playerAddItemsAndStore(effgrids, items, "buyGuildStoreGoods", true);
        }
		
		// 记录购买标记
		gsMgr->setStoreGoodsBuyIndex(sg);
		ack.errorcode = StoreBuyReusltReuslt_Success;
	}while (false);
	
	sendNetPacket(role->getSessionId(), &ack);
}

//升级公会技能
void GuildMgr::on_req_upgrade_guild_skill(Role* role, req_upgrade_guild_skill& req)
{
    xyassert(role);
    
    Guild& guild = getRoleGuild(role->getInstID());
    if(guild.isNull())
        return;
    
    ack_upgrade_guild_skill ack;
    ack.skillid = req.skillid;
	
	//　技能id非法检查 04-23 jin
	if (req.skillid <= 0 || req.skillid % 100 > 0) {
		Xylog log(eLogName_GuildSkillIllegal, role->getInstID());
		log << req.skillid;
		return;
	}
    
    RoleGuildProperty& prop = SRoleGuild( role->getInstID() );
    int currentLevel = prop.getSkill().get(req.skillid);
    
    GuildSkillDef * pGuildSkillDef = GuildCfgMgr::getSkillDef(req.skillid, currentLevel+1);
    
    do{
        
        if(NULL == pGuildSkillDef)
            break;
        
        if(guild.getLevel() < pGuildSkillDef->needGuildLvl)
            break;
        
        if(currentLevel >= pGuildSkillDef->maxLvl)
            break;
                
        if( role->getBattlePoint() < pGuildSkillDef->needBattlePoint)
            break;
        
        if( prop.getConstrib() < pGuildSkillDef->needContribute)
            break;
        
        role->addBattlePoint(- pGuildSkillDef->needBattlePoint , "GuildSkill");
        string upstr = GameTextCfg::getString("1017");
        prop.addConstrib( - pGuildSkillDef->needContribute, "升级公会技能");
        prop.getSkill().set(req.skillid, currentLevel+1);
        
        ack.errcode = 0;
        ack.newlevel = prop.getSkill().get(req.skillid);
        sendNetPacket(role->getSessionId(), &ack);
        
        //更新角色战斗力
        role->CalcPlayerProp();
        
        //更新出战宠战斗力
        Pet* pet = role->mPetMgr->getActivePet();
        if (pet)
        {
            pet->calculateAttributes();
            role->sendPetPropAndVerifycode(pet);
        }
        
        Xylog log(eLogName_GuildUpgradeSkill,role->getInstID());
        log
		<< guild.getId()
		<< guild.getGuildName()
		<< ack.skillid
		<< ack.newlevel
		<< pGuildSkillDef->needBattlePoint
		<< pGuildSkillDef->needContribute;
        
        return;
        
    }while(0);
     
    ack.errcode = 1;
    sendNetPacket(role->getSessionId(), &ack);
}

//公会聊天
void GuildMgr::on_req_guild_chat(Role* role,req_guild_chat& req)
{
    if(req.msg.empty())
        return;
    
    if (req.messageType != kChatContentTypeText &&
        req.messageType != kChatContentTypeVoice) {
        return;
    }
    
    xyassert(role);
    Guild& guild = getRoleGuild(role->getInstID());
    if(guild.isNull())
        return;
    
    notify_guild_chat nty;
    nty.msg = req.msg;
    
    nty.senderRoleid = role->getInstID();
    nty.senderRoleName = role->getRolename();
    nty.senderRoleType = role->getRoleType();
    nty.senderRoleLvl = role->getLvl();
    
    nty.messageType = req.messageType;
    nty.createDate = Game::tick;
    
    GuildChatMsg record;
    record.roleid = role->getInstID();
    record.rolename = role->getRolename();
    record.roleType = role->getRoleType();
    record.roleLvl = role->getLvl();
    
    record.messageType = req.messageType;
    record.position = SRoleGuild( role->getInstID() ).getPosition();
    record.msg = req.msg;
    record.createDate = Game::tick;
    
    if (req.messageType == kChatContentTypeVoice) {
        
        VoiceDataStruct* voicedata = g_VoiceChatMgr.createVoiceData(eGameChatGuildChat,req.voiceData, Game::tick, true);
        
        if( voicedata != NULL )
        {
            record.voiceId = voicedata->mVoiceId;
            record.voiceDuration = req.voiceDuration;
            record.translated = req.translated;
            
            nty.voiceid = voicedata->mVoiceId;
            nty.voiceDuration = req.voiceDuration;
            nty.translated = req.translated;
        }
    }
    
    Guild::GuildMember::iterator itMember = guild.getMembers().begin();
    for (; itMember != guild.getMembers().end(); ++itMember)
    {
        Role* role = SRoleMgr.GetRole(itMember->first);
        if(NULL == role)
            continue;
        sendNetPacket(role->getSessionId(), &nty);
    }
    
    guild.appendChatMsg(record);
}

//旧的公会聊天消息
void GuildMgr::on_req_past_guild_chat(Role* role, req_past_guild_chat& req)
{
    xyassert(role);
    Guild& guild = getRoleGuild( role->getInstID());
    if(guild.isNull())
        return;
    guild.sendChatMsg(role->getInstID(), req.start, req.num);
}


static GuildGadDef* getRoleHeraldry(int roleid)
{
    Guild& guild = SGuildMgr.getRoleGuild(roleid);
    if(guild.isNull())
        return NULL;
    
    GuildGadDef* def = GuildCfgMgr::getGadDef(guild.getLevel(), guild.getRank(roleid));
    if(NULL == def)
    {
        xyerr("纹章对战斗力的加成失败");
        return NULL;
    }
    return def;
}

//计算纹章对战斗力的加成
void calHeraldryPropertyAddition(int roleid , BaseProp& base , BattleProp& bat )
{
    GuildGadDef* def =  getRoleHeraldry(roleid);
    if(def)
    {
        bat.mAtk += def->getAtk();
        bat.mDef += def->getDef();
        bat.mMaxHp += def->getAddMaxHp();
        bat.mHit +=def->getHit();
        bat.mDodge += def->getDodge();
    }
}

//纹章对精力上限的提高
int calHeraldryFatAddition(int roleid)
{
    GuildGadDef* def =  getRoleHeraldry(roleid);
    if(def)
    {
        return def->getAddMaxFat();
    }
    return 0;
}

//公会被动技能对角色战斗力的加成
void calGuildSkillPropertyAdditionForSelf(int roleid , BaseProp& base , BattleProp& bat)
{
    RoleGuildProperty& prop = SRoleGuild(roleid);
    vector<int> skills;
    prop.getGuildSkill(skills);
    
    SkillIncrBattle tmp;
    for (int i = 0; i< skills.size(); ++i)
    {
        SkillCfg::calcBattlePropFromSkill(skills[i],eForSelf,tmp);
    }
    
    base += tmp.baseProp;
    bat += tmp.battleProp;
}

// 公会被动技能对宠物战斗力的加成
void calGuildSkillPropertyAdditionForPet(int roleid , SkillIncrBattle& out)
{
    RoleGuildProperty& prop = SRoleGuild(roleid);
    
    for (RoleGuildSkill::Iterator itSkill = prop.getSkill().begin();
         itSkill != prop.getSkill().end(); ++itSkill)
    {
        int skillFullId = itSkill->first + itSkill->second;//400800 + 1 -> 400801才是id
        SkillCfg::calcBattlePropFromSkill(skillFullId,eForPet,out);
    }
}

void GuildMgr::freshGuild(Guild& guild)
{
    doImpeachMaster(guild);
    guild.setDonateRmbNum(0);
    guild.setInspired(0);
    guild.sendGuildInfoAllMember();
    
    Guild::GuildMember::iterator itMember = guild.getMembers().begin();
    for (; itMember != guild.getMembers().end(); ++itMember)
    {
        Role* role = SRoleMgr.GetRole(itMember->first);
        if(NULL == role)
            continue;
        on_req_player_guild_detail(role);
    }
}

//跨天刷新
void GuildMgr::spanDayRefresh()
{    
    GuildMap::iterator itGuild = mGuilds.begin();
    for (; itGuild != mGuilds.end(); ++itGuild)
    {
        Guild& guild = itGuild->second;
        freshGuild(guild);
    }
}

// 取公会boss经验
void GuildMgr::on_req_guild_boss_exp(Role* role)
{
    xyassert(role);
    Guild& guild = getRoleGuild(role->getInstID());
    if(guild.isNull())
        return;
    nty_guild_boss_exp nty;
    nty.exp = guild.getBossExp();
    sendNetPacket(role->getSessionId(),&nty);
}


void send_ack_train_guild_boss(int roleid , int err, int exp)
{
    ack_train_guild_boss ack;
    ack.err = err;
    ack.train_exp = exp;
    sendRolePacket(roleid,&ack);
}

// 这里传的等级是, 该级公会可培养boss的最大等级
static int maxGuildBossExp(int guildlvl)
{
    GuildBossLvlDef* def =  GuildCfgMgr::getGuildBossLvlExp( guildlvl );
    if(NULL == def )
        return 0;
    return def->exp;
}

// 培养公会boss
void GuildMgr::on_req_train_guild_boss(Role* role,req_train_guild_boss& req)
{
    xyassert(role);
    
    RoleGuildProperty& property = SRoleGuild(role->getInstID());
    if(property.noGuild())
        return;
    
    Guild& guild = getGuild( property.getGuild() );
    
    //正在杀boss不能培养，此期间新成立的公会，这里是可以培养的
    if( 0 != g_BossActivityController.getGuildBossId(guild.getId()) )
        return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_ActivityOpen, 0);
    
	GuildLvlDef *guildLvlDef = GuildCfgMgr::getGuildLvlDef(guild.getLevel());
	if(NULL == guildLvlDef)
    {
        xyerr("GuildMgr::on_req_train_guild_boss guildLvlDef == NULL");
        return;
    }
	
    // boss最大等级配置错误
    if (guildLvlDef->maxbosslvl <= 0) {
        return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_BossMaxLvlErr, 0);
    }
    
    GuildBossLvlDef* def =  GuildCfgMgr::getGuildBossLvlDef( guild.getBossExp());
    if(NULL == def)
    {
        xyerr("GuildMgr::on_req_train_guild_boss def == NULL");
        return;
    }
    
    //次数限制是与公会级别，这里放在一齐有点不对，后面再改了 KCDO
    GuildBossLvlDef* timeLimitDef = GuildCfgMgr::getGuildBossLvlExp( guild.getLevel());
    xyassertf(timeLimitDef,"%d", guild.getLevel());
    
    // 道具培养
    if( eTrainGuildBossType_ItemTrain == req.train_type)
    {
        if( role->getGuildItemTrainBossTime() >= timeLimitDef->itemTrainBossTimePerDay )
            return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_ItemTrainTimeLimit, 0);
        
        stringstream ss;
        
        int exp = 0;
        for (int i = 0; i< req.items.size(); ++i)
        {
            ItemCfgDef* itemcfg = ItemCfg::getCfg(req.items[i]);
            if(NULL == itemcfg)
                return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_ItemIdError, 0);
            
            int tmp = itemcfg->ReadInt("add_bossexp");
            if( 0 == tmp)
                return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_ItemIdError, 0);
            
            exp += tmp;
            
            ss<<itemcfg->getFullName()<<"\t";
        }

        ItemArray materail,merged;
        GridArray effgrids;
        
        for (int i = 0; i< req.items.size(); ++i)
        {
            ItemGroup itemgroup;
            itemgroup.item = req.items[i];
            itemgroup.count = 1;
            materail.push_back(itemgroup);
        }
        
        //因为PreDelItems 不能有重复的
        mergeItems(materail, merged);
        
        //wangzhigang 2014-11-19
//        if (!BackBag::PreDelItems(role->getInstID(), merged, effgrids))
        if(!role->getBackBag()->PreDelItems(merged, effgrids))
        {
            return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_ItemIdError, 0);
        }

        role->playerDeleteItemsAndStore(effgrids, merged, "train_guild_boss", true);
        
        role->setGuildItemTrainBossTime( 1 + role->getGuildItemTrainBossTime() );

        // 加个人贡献和BOSS经验
        // 当boss达到最高等级，只加个人贡献
        string bossstr = GameTextCfg::getString("1019");
        SRoleGuild( role->getInstID()).addConstrib(exp, "公会BOSS培养");
//        ack.train_exp = exp;
        
        if (guild.getBossLevel() < guildLvlDef->maxbosslvl) {
            int maxAdd = maxGuildBossExp(guildLvlDef->maxbosslvl) - guild.getBossExp();
            exp = min(maxAdd, exp);
            guild.addBossExp( exp );
            
            Xylog log(eLogName_GuildBossTrain,role->getInstID());
            log
            << guild.getId()
            << guild.getGuildName()
            << guild.getBossExp() - exp
            << guild.getBossExp()
            << exp
            << req.train_type
            << ss.str();
            
            return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_Success, exp);
        }
        
        Xylog log(eLogName_GuildBossTrain,role->getInstID());
        log
		<< guild.getId()
		<< guild.getGuildName()
		<< guild.getBossExp() - exp
		<< guild.getBossExp()
		<< exp
		<< req.train_type
		<< ss.str();
    
        return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_BossExpFull, exp);
        
    }
    
    //金钻
    if( eTrainGuildBossType_RmbTrain == req.train_type )
    {
        if( role->getGuildRmbTrainBossTime() >= timeLimitDef->rmbTrainBossTimePerDay )
            return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_RmbTrainTimeLimit, 0);
    
        int needRmb = def->rmbTrainNeedRmb( role->getGuildRmbTrainBossTime() );
        int exp = def->rmbTrainAddBossExp( role->getGuildRmbTrainBossTime() );
        
        string bossstr = GameTextCfg::getString("1019");
        if ( role->CheckMoneyEnough(needRmb, eRmbCost, "公会Boss培养") != CE_OK)
            return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_NeedMoreRmb, 0);
        
        
        role->setGuildRmbTrainBossTime( 1 + role->getGuildRmbTrainBossTime() );
        SRoleGuild( role->getInstID()).addConstrib(exp, "公会BOSS培养");
//        ack.train_exp = exp;
        
        // boss达到最高等级时，不加boss经验
        if (guild.getBossLevel() < guildLvlDef->maxbosslvl) {
            int maxAdd = maxGuildBossExp(guildLvlDef->maxbosslvl) - guild.getBossExp();
            exp = min(maxAdd, exp);
            guild.addBossExp( exp );
            
            Xylog log(eLogName_GuildBossTrain,role->getInstID());
            log
            << guild.getId()
            << guild.getGuildName()
            << guild.getBossExp() - exp
            << guild.getBossExp()
            << exp
            << req.train_type
            << needRmb;
            
            return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_Success, exp);
        }
      
        Xylog log(eLogName_GuildBossTrain,role->getInstID());
        log
		<< guild.getId()
		<< guild.getGuildName()
		<< guild.getBossExp() - exp
		<< guild.getBossExp()
		<< exp
        << req.train_type
		<< needRmb;
        
        return send_ack_train_guild_boss(role->getInstID(), eTrainGuildBoss_BossExpFull, exp);
        
    }

    role->saveNewProperty();
}

// 取本公会排名
void GuildMgr::on_req_my_guild_rank(Role* role)
{
    xyassert(role);
    Guild& guild = getRoleGuild( role->getInstID() );
    if( guild.isNull() )
        return;
    
    ack_my_guild_rank ack;
    ack.rank = distance(mGuildRank.begin(), find(mGuildRank.begin(),mGuildRank.end() , guild.getId()) );
    sendNetPacket(role->getSessionId(),&ack);
}




// 祝福
void GuildMgr::on_req_guild_bless(Role* role, req_guild_bless& req)
{
    // 是否是成员
    assert(role);
    Guild& guild = getRoleGuild(role->getInstID());
    xycheck( ! guild.isNull() );
    xycheck( guild.getMembers().exist(req.target) );
    
    ack_guild_bless ack;
    ack.errorcode = 0;
    ack.target = req.target;
    
    // 必须在线
    Role* target = SRoleMgr.GetRole(req.target);
    send_if(NULL == target , eGuildBless_TargetOffline );

    // 已被你祝福过了
    send_if(role->getGuildBlessMate().exist( target->getInstID() ), eGuildBless_AlreadyBless);
    
    // 被祝福已满
    const ChallengeGuildCfg& cfg = ChallengeCfg::getGuildDef();
    send_if( target->getGuildBlessed() >= cfg.blessedNum , eGuildBless_TargetBlessFull );

    // 钱够不够
    int usermb = 0;
    if( role->getGuildBlessMate().size() >= cfg.blessingNum)
    {
        send_if(role->getRmb() < cfg.blessingRmb , eGuildBless_NeedMoreRmb);
        string str = GameTextCfg::getString("1029");
        assert( role->CheckMoneyEnough(cfg.blessingRmb, eRmbCost,/*"公会祝福"*/str.c_str()) == CE_OK );
        usermb = cfg.blessingRmb;
    }
    
    role->getGuildBlessMate().push_back( target->getInstID() );
    target->setGuildBlessed( target->getGuildBlessed() + 1);
    
    role->saveNewProperty();
    target->saveNewProperty();
    
    //祝福成功可得奖励,2.1先不做，2014-6-17现在给增加上
    // TODO
    const ChallengeGuildCfg& ChallengGuildcfg = ChallengeCfg::getGuildDef();
    vector<string> awards = StrSpilt(ChallengGuildcfg.blesser_get_award.c_str(), ";");
    ItemArray items;
    role->addAwards(awards, items, "祝福成功可得奖励");
    
    // 日志
    Xylog log(eLogName_GuildBless, role->getInstID());
    log << target->getRolename() << usermb << guild.getId() << guild.getGuildName();
    
    // 公会消息
    stringstream ss;
    string str = GameTextCfg::getFormatString("1030", role->getRolename().c_str(), target->getRolename().c_str());
    //ss<<role->getRolename()<<"祝福了"<<target->getRolename();
    ss << str;
    guild.appendLog(ss.str());
    
    ack.errorcode = eGuildBless_Success;
    target->send(&ack);
    send_if(true,eGuildBless_Success);
}

// 鼓舞
void GuildMgr::on_req_guild_inspire(Role* role, req_guild_inspire& req)
{
    assert(role);
	
    RoleGuildProperty& prop = SRoleGuild( role->getInstID() );
    if(prop.getGuild() == 0)
        return;
	
	Guild& guild = getGuild( prop.getGuild() );
    xycheck( ! guild.isNull() );
    
    ack_guild_inspire ack;
    
    const ChallengeGuildCfg& cfg = ChallengeCfg::getGuildDef();
    
    send_if( role->getGuildInspire() >= cfg.inspireNum, eGuildInspire_YourTimeUsedUp);
    send_if( guild.getInspired() >= cfg.inspiredNum , eGuildInspire_GuildMaxTime);
    send_if( role->getRmb() < cfg.inspiredRmb , eGuildInspire_NeedMoreRmb);
    
    
    string str = GameTextCfg::getString("1031");
    assert( role->CheckMoneyEnough(cfg.inspiredRmb, eRmbCost,/*"公会鼓舞"*/str.c_str()) == CE_OK );
    role->setGuildInspire( role->getGuildInspire() + 1);
    guild.setInspired( guild.getInspired() + 1);
    role->saveNewProperty();
    guild.update();
    
    Xylog log(eLogName_GuildInspire,role->getInstID());
    log<<cfg.inspiredRmb << prop.getGuild() << guild.getGuildName();
    
    stringstream ss;
    string oncestr = GameTextCfg::getFormatString("1032", role->getRolename().c_str());
    //ss<<role->getRolename()<<"鼓舞公会1次";
    ss << oncestr;
    guild.appendLog(ss.str());
    
    send_if( true , eGuildInspire_Success );
}

static bool isMasterOfflineLessThanNDay(const int &last_active_time)
{
    if (last_active_time == 0)
    {
        return true;
    }
    int nowtime = time(NULL);
    int day_interval = (nowtime - last_active_time) / 60;
    if (day_interval < GuildCfgMgr::getImpeachTime())
    {
        return true;
    }
    return false;
}

//发送因弹劾失败退还金钻邮件。
void sendImpeachKickbackMail(const char *receiver, const int &rmb, const char *mailtype)
{
    if (!mailtype)
    {
        return ;
    }
    MailFormat *f = MailCfg::getCfg(mailtype);
    if (!f)
    {
        return ;
    }
    string attach = Utils::makeStr("rmb %d*1", rmb);
    sendMail(0, f->sendername.c_str(), receiver, f->title.c_str(), f->content.c_str(), attach.c_str(), "");
}

//当会长上线了，重置弹劾信息。
void resetImpeachInfoWhenMasterOnLine(int roleid, const char *mail_type)
{
    RoleGuildProperty &prop  = SRoleGuild(roleid);
    if (prop.noGuild())
    {
        return ;
    }
    Guild& guild = SGuild(prop.getGuild());
    if (guild.getMaster() != roleid)
    {
        return ;
    }
    int applicant = guild.getImpeachApplicant();
    if (applicant == 0)
    {
        return ;
    }
    guild.setImpeachApplicant(0);
    guild.save();
    RoleGuildProperty &applicant_prop  = SRoleGuild(applicant);
    sendImpeachKickbackMail(applicant_prop.getRoleName().c_str(), GuildCfgMgr::getImpeachCost(), mail_type);
}

void resetImpeachApplicantIf(Guild &guild, int roleid, const char *mail_type)
{
    //退出公会的时候刚好自己又申请了弹劾
    if (guild.getImpeachApplicant() == roleid )
    {
        guild.setImpeachApplicant(0);
        guild.save();
        RoleGuildProperty &prop  = SRoleGuild(roleid);
        sendImpeachKickbackMail(prop.getRoleName().c_str(), GuildCfgMgr::getImpeachCost(), mail_type);
    }
}

int GuildMgr::guildRename(int roleid, string newname)
{
    
    Guild& guild = SGuildMgr.getRoleGuild(roleid);
    
    if (guild.isNull()) {
        return CE_GUILD_NOT_EXIST;
    }
    
    if( mNameIndex.count(newname) )
    {
        return CE_GUILD_NAME_EXIST;
    }
    
    std::map<string, int>::iterator iter = mNameIndex.find(guild.getGuildName());
    
    if (iter != mNameIndex.end()) {
        mNameIndex.erase(iter);
    }
    
    mNameIndex.insert(make_pair(newname, guild.getId()));
    
	Xylog log(eLogName_ReGuildName, roleid);
	log<<guild.getGuildName();
	
    guild.guildRename(newname);
    
	log<<newname;
	
    return CE_OK;
}

void GuildMgr::doImpeachMaster(Guild& guild)
{
    int applicant_id = guild.getImpeachApplicant();
    if (applicant_id == 0)
    {
        return ;
    }
    
    int master_id = guild.getMaster();
    
    RoleGuildProperty &master_prop = SRoleGuild(master_id);
    if (master_prop.noGuild())
    {
        return ;
    }
    
    int master_lastactive = master_prop.getLastActive();
    
    if (isMasterOfflineLessThanNDay(master_lastactive))
    {
        return ;
    }
    
    guild.setMaster(applicant_id);
    guild.calRankAndPosition();
    
    guild.setImpeachApplicant(0);
    guild.save();
    
    Xylog log(eLogName_GuildImpeachLog, applicant_id);
    log << "每日刷新，取代了会长";

    //公会日志: %s弹劾成功，%s现在是会长了
    RoleGuildProperty &applicant_prop = SRoleGuild(applicant_id);
    string str = GameTextCfg::getFormatString("1049", applicant_prop.getRoleName().c_str(), applicant_prop.getRoleName().c_str());
    guild.appendLog(str, eGuildLogLvl_C);
    guild.sendGuildInfoAllMember();
    
	// 重设职位
	master_prop.setPosition(kGuildMember);
	master_prop.save();
	applicant_prop.setPosition(kGuildMaster);
	applicant_prop.save();

    //重算两个角色战斗力
    vector<int> tmp;
    tmp.push_back(master_id);
    tmp.push_back(applicant_id);
    for (int i = 0; i< tmp.size(); ++i)
    {
        Role* role = SRoleMgr.GetRole( tmp[i] );
        if(role)
        {
            role->CalcPlayerProp();
            sendRoleGuildAndGuildShareData(tmp[i]);
        }
    }
    
    /*
    notify_transfer_guild_master nty;
    nty.oldmaster = role->getInstID();
    nty.newmaster = req.memberid;
    guild.sendAllMemberPacket(&nty);
    
    Xylog log(eLogName_GuildCommon,role->getInstID());
    string logstr = GameTextCfg::getString("1012");
    //log<<"转让会长"<<nty.newmaster;
    log<<logstr<<nty.newmaster;
     */
    log_info("会长长期没有上线，已被取代。");
}

//获取弹劾信息
void GuildMgr::on_req_guild_impeach_info(Role *role)
{
    assert(role);
    
    RoleGuildProperty& prop = SRoleGuild(role->getInstID());
    if (prop.noGuild())
    {
        return ;
    }
    
    Guild& guild = getGuild( prop.getGuild());
    if( ! guild.getMembers().exist(role->getInstID()) )
    {
        return;
    }
    
    int master_id = guild.getMaster();
    
    //获取会长属性。
    RoleGuildProperty& master_prop = SRoleGuild(master_id);
    if (master_prop.noGuild())
    {
        return ;
    }
    
    //获取会长最后活动时间
    int master_lastactive = master_prop.getLastActive();
     
    ack_guild_impeach_info ack;
    
    int impeach_applicant_id = guild.getImpeachApplicant();

    //没有超过指定时间，发送错误信息
    if (isMasterOfflineLessThanNDay(master_lastactive))
    {
        ack.errorcode = eGuildImpeachInfoError_MasterOfflineLessThanNDay;
        ack.applicant_contrib = 0;
        sendNetPacket(role->getSessionId(), & ack);
        return ;
    }
    
    //会长离线时间超过指定时间了。

    //暂时没有申请者，或者当前申请者已经不在公会了，发送没有申请者信息
    if (impeach_applicant_id == 0 || !guild.getMembers().exist(impeach_applicant_id)) //应该不会出现此人不在公会的情况。
    {
        guild.setImpeachApplicant(0);
        guild.save();
        ack.errorcode = eGuildImpeachInfoError_NoApplicant;
        ack.applicant_contrib = 0;
        sendNetPacket(role->getSessionId(), &ack);
        return ;
    }
    
    
    //有申请者,发送申请者名称以及贡献值
    RoleGuildProperty& applicant_prop = SRoleGuild(impeach_applicant_id);
    ack.errorcode = eGuildImpeachInfoError_HaveApplicant;
    ack.applicant_name = applicant_prop.getRoleName();
    ack.applicant_contrib = applicant_prop.getExploit();
    sendNetPacket(role->getSessionId(), &ack);
    return ;
}
//


//会长弹劾
void GuildMgr::on_req_guild_impeach(Role *role)
{
    assert(role);
    
    RoleGuildProperty& prop = SRoleGuild(role->getInstID());
    if (prop.noGuild())
    {
        return ;
    }
    
    Guild& guild = getGuild( prop.getGuild());
    //角色不存在这个公会
    if(!guild.getMembers().exist(role->getInstID()))
    {
        return;
    }
    
    //获取申请者ID
    int applicant_id = guild.getImpeachApplicant();
    
    ack_guild_impeach ack;
    
    //此时申请者已经是自己了，能不能客户端自己判断？
    if (applicant_id == role->getInstID())
    {
        ack.errorcode = eGuildImpeachError_ImpeachAppliantIsYouNow;
        sendNetPacket(role->getSessionId(), &ack);
        return ;
    }
    
    //金钻不够
    if (role->getRmb() < GuildCfgMgr::getImpeachCost())
    {
        ack.errorcode = eGuildImpeachError_LessRmb;
        sendNetPacket(role->getSessionId(), &ack);
        return ;
    }
    
    bool isHaveApplicant = false;
    if (applicant_id != 0)
    {
        isHaveApplicant = true;
    }
    
    if (isHaveApplicant)
    {
        RoleGuildProperty& applicant_prop = SRoleGuild(applicant_id);
        //公会功勋低于申请者
        if (applicant_prop.getExploit() >= prop.getExploit())
        {
            ack.errorcode = eGuildImpeachError_ContribLessThanApplicant;
            sendNetPacket(role->getSessionId(), &ack);
            return ;
        }
    }
    
    //金钻够了则设置申请者
    role->CheckMoneyEnough(GuildCfgMgr::getImpeachCost(), eRmbCost, "申请弹劾会长");
    guild.setImpeachApplicant(role->getInstID());
    guild.save();
    
    Xylog log(eLogName_GuildImpeachLog, role->getInstID());
    log << "申请了弹劾会长";
    
    ack.errorcode = eGuildImpeachError_Ok;
    sendNetPacket(role->getSessionId(), &ack);
    
    if (isHaveApplicant)
    {
        RoleGuildProperty& applicant_prop = SRoleGuild(applicant_id);
        sendImpeachKickbackMail(applicant_prop.getRoleName().c_str(), GuildCfgMgr::getImpeachCost(), "guild_impeach");
        
        //公会日志: %s申请了弹劾，导致%s的申请失败了
        string str = GameTextCfg::getFormatString("1048", role->getRolename().c_str(), applicant_prop.getRoleName().c_str());
        guild.appendLog(str, eGuildLogLvl_C);
    }
    else
    {
        //公会日志: %s申请了弹劾
        string str = GameTextCfg::getFormatString("1047", role->getRolename().c_str());
        guild.appendLog(str, eGuildLogLvl_C);
    }
    guild.sendGuildInfoAllMember();
}

// 指派职位
void GuildMgr::on_req_guild_appoint_position(Role *role, req_guild_appoint_position& req)
{
	xyassert(role);
	ack_guild_appoint_position ack;
	ack.errorcode = eGuildAppointPosition_UnknowError;
	do {
		RoleGuildProperty& prop = SRoleGuild(role->getInstID());
		if (prop.noGuild()) {
			break;
		}
		Guild& guild = getGuild(prop.getGuild());
		
		if(!guild.getMembers().exist(req.roleId))
		{
			ack.errorcode = eGuildAppointPosition_NoGuild;
			break;
		}
		RoleGuildProperty& targetProp = SRoleGuild(req.roleId);
		if (targetProp.noGuild()) {
			ack.errorcode = eGuildAppointPosition_NoGuild;
			break;
		}

		//　１:检查权限
		if (!roleHasRight(role->getInstID(), kGuildAppointPosition)) {
			ack.errorcode = eGuildAppointPosition_NoPosition;
			break;
		}
		//　２:是否对自己指派
		if (role->getInstID() == req.roleId) {
			ack.errorcode = eGuildAppointPosition_IsSelf;
			break;
		}
		//	3:与会长公会不一至
		if (prop.getGuild() != targetProp.getGuild()) {
			ack.errorcode = eGuildAppointPosition_NoGuildLike;
			break;
		}
		//　4:只能指派副会/精英/会员
		if (req.pos != kGuildViceMaster
			&& req.pos != kGuildElite
			&& req.pos != kGuildMember)
		{
			ack.errorcode = eGuildAppointPosition_NoPosRange;
			break;
		}
		//　5:检查指派职位人数是否已满
		if (guild.checkPositionIsFull((GuildMemberPosition)req.pos)) {
			ack.errorcode = eGuildAppointPosition_PositionIsFull;
			break;
		}
		// 6:修改职位
		targetProp.setPosition(req.pos);
		targetProp.save();
		
		Role* tarRole = SRoleMgr.GetRole(req.roleId);
        if(NULL != tarRole)
        {
            tarRole->CalcPlayerProp();
        }
		
		// 7:同步职位消息
		syn_player_guild_position syn;
        syn.position = targetProp.getPosition();
        sendRolePacket(req.roleId, &syn);
		
		// 8:日志
		Xylog log(eLogName_GuildAppointPosition, role->getInstID());
		log
		<< prop.getGuild()
		<< guild.getGuildName()
		<< req.roleId
		<< req.pos;
		
		ack.errorcode = eGuildAppointPosition_Success;
	} while (false);
	role->send(&ack);
}

void GuildMgr::gmSetLvl(int roleID, int i_Lvl)
{
    Role *role = SRoleMgr.GetRole(roleID);
    RoleGuildProperty& prop = SRoleGuild(role->getInstID());
    if (prop.noGuild())
    {
        return ;
    }
    
    Guild& guild = getGuild( prop.getGuild());
    //角色不存在这个公会
    if(!guild.getMembers().exist(role->getInstID()))
    {
        return;
    }
    
    //getFortune(), getConstruction()
    //guild.setFortune(i_Lvl);
    GuildLvlDef * GuildInfo = GuildCfgMgr::getGuildLvlDef(i_Lvl);
    if (!GuildInfo) {
        return;
    }
    guild.setFortune(GuildInfo->needWealth);
    guild.setConstruction(GuildInfo->needOverbuild);
    guild.save();
    guild.sendGuildInfoAllMember();//有可能公会等级会变，所以全部重发
}

void GuildMgr::gmSetForturn(int roleID, int i_Forturn)
{
    Role *role = SRoleMgr.GetRole(roleID);
    RoleGuildProperty& prop = SRoleGuild(role->getInstID());
    if (prop.noGuild())
    {
        return ;
    }
    
    Guild& guild = getGuild( prop.getGuild());
    //角色不存在这个公会
    if(!guild.getMembers().exist(role->getInstID()))
    {
        return;
    }
    
    //getFortune(), getConstruction()
    guild.setFortune(i_Forturn);
    guild.save();
    guild.sendGuildInfoAllMember();//有可能公会等级会变，所以全部重发
}

void GuildMgr::gmSetDcons(int roleID, int i_Dcons)
{
    Role *role = SRoleMgr.GetRole(roleID);
    RoleGuildProperty& prop = SRoleGuild(role->getInstID());
    if (prop.noGuild())
    {
        return ;
    }
    
    Guild& guild = getGuild( prop.getGuild());
    //角色不存在这个公会
    if(!guild.getMembers().exist(role->getInstID()))
    {
        return;
    }
    
    //getFortune(), getConstruction()
    
    guild.setConstruction(i_Dcons);
    guild.save();
    guild.sendGuildInfoAllMember();//有可能公会等级会变，所以全部重发
}

void GuildMgr::safe_gmSetLvl(int roleID,int i_Lvl)
{
    create_global(GmSetGuildLvl, msg);
    msg->num = i_Lvl;
    msg->roleid = roleID;
    sendGlobalMsg(Global::MQ, msg);

}

void GuildMgr::safe_gmSetFortune(int roleID,int i_Forturn)
{
    create_global(GmSetGuildFortune, msg);
    msg->num = i_Forturn;
    msg->roleid = roleID;
    sendGlobalMsg(Global::MQ, msg);
    
}
void GuildMgr::safe_gmSetDcons(int roleID,int i_Dcons)
{
    create_global(GmSetGuildDcons, msg);
    msg->num = i_Dcons;
    msg->roleid = roleID;
    sendGlobalMsg(Global::MQ, msg);
}

