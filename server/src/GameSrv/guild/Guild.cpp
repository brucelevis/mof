#include "Guild.h"
#include "GuildShare.h"
#include "GuildRole.h"
#include "GameLog.h"
#include "Utils.h"
#include "Role.h"
#include <vector>
#include <algorithm>
using namespace std;

#pragma mark -
#pragma mark Guild Implememt


struct MemSortNode
{
    int roleid;
    int exploit;
};

bool MemCmpFunc(const MemSortNode& left , const MemSortNode& right)
{
    return left.exploit > right.exploit ? true : false;
}

// 清掉公会里面被删的角色
void Guild::cleanNotExistMember()
{
    vector<int> clr;
    
    // 先清了不存在的会员
    GuildMember::iterator itMember;
    for (itMember = mMembers.begin(); itMember != mMembers.end(); ++itMember)
    {
        int roleid = itMember->first;
        
        if( ! isRoleExistInDbForMainThread(roleid) )
            clr.push_back(roleid);
    }
    for (vector<int>::iterator it = clr.begin(); it != clr.end(); ++it)
    {
        mMembers.remove( *it );
    }
    
    // 公会一个人都没了
    if( mMembers.size() == 0 )
    {
        xystr("%s公会一个人都没了",getGuildName().c_str());
        return;
    }
    
    // 如果会长没了，从member随便挑一个
    if( ! isRoleExistInDbForMainThread( getMaster() ) )
    {
        xystr("%s会长已被删，随机出会长",getGuildName().c_str());
        setMaster( mMembers.begin()->first );
    }
    
    update();//立马回写
}

//重算排名和职位时机: 初始化，人员进出，转让会长。其它时间，一小时定时刷
void Guild::calRankAndPosition()
{
    if(mMembers.size() == 0)
        return;
    
    GuildMember::iterator it;

    MemSortNode node;
    vector<MemSortNode> vec;

    for (it = mMembers.begin(); it != mMembers.end(); ++it)
    {
        if(it->first == getMaster() )
            continue;

        node.roleid = it->first;
        node.exploit = SRoleGuild(it->first).getExploit();
        vec.push_back(node);
    }
    sort(vec.begin(), vec.end() , MemCmpFunc);

    mRank.clear();
    mRank.push_back(getMaster());
    for (int i = 0 ,ni = vec.size(); i< ni; ++i) {
        mRank.push_back(vec[i].roleid);
    }
	
	/*
	 公会职位不再按照排行榜,　改由会长任职
	 数据没有记录会长的职位时,　刷一遍数据 (05-20 by jin)
	 */
	if (SRoleGuild(getMaster()).getPosition() == 0) {
		for (it = mMembers.begin(); it != mMembers.end(); ++it)
		{
			int roleId = it->first;
			GuildMemberPosition ePos = kGuildMember;
			
			if (getMaster() == roleId) {
				roleId = getMaster();
				ePos = kGuildMaster;
			} 
			
			RoleGuildProperty& property = SRoleGuild(roleId);
			property.setPosition(ePos);
			property.save();
		}
	}
	
	// 需同步发送排名与职位
	for (int i = 0; i < mRank.size(); ++i)
    {
        int roleid = mRank[i];
		
		RoleGuildProp &prop = SRoleGuild(roleid);
		
		// 异常处理,　因为会长弹劾没有设置职位,　造成职位错误,　在这加逻辑修正　2014-07-29 by jin
		if (roleid == getMaster())
		{
			if(prop.getPosition() != kGuildMaster) {
				prop.setPosition(kGuildMaster);
				prop.save();
			}
		}
		else
		{
			if (prop.getPosition() == kGuildMaster) {
				prop.setPosition(kGuildMember);
				prop.save();
			}
		}
		
        Role* role = SRoleMgr.GetRole(roleid);
        if(NULL == role)
            continue;
		
        syn_player_guild_position syn;
        syn.position = prop.getPosition();
        role->send(&syn);
		
		syn_player_guild_rank synRank;
		synRank.rank = getRank(roleid);
        role->send(&synRank);
    }
}


int Guild::getMembersPositionCount(GuildMemberPosition ePosition)
{
	int count = 0;
	for (GuildMember::iterator it = mMembers.begin(); it != mMembers.end(); ++it) {
        RoleGuildProperty& property = SRoleGuild(it->first);
		if (property.getPosition() == ePosition) {
			count++;
		}
    }
	return count;
}

bool Guild::checkPositionIsFull(GuildMemberPosition ePosition)
{
	bool bet = false;
	GuildLvlDef* pGuildLvlDef = GuildCfgMgr::getGuildLvlDef(getLevel());
	if (NULL == pGuildLvlDef) {
		return bet;
	}
	
	int count = this->getMembersPositionCount(ePosition);
	switch (ePosition) {
		case kGuildViceMaster :
			bet = count < pGuildLvlDef->deputyChairmanNum ? false : true;
			break;
		case kGuildElite :
			bet = count < pGuildLvlDef->eliteNum ? false : true;
			break;
		case kGuildMember :
			bet = false;
			break;
		default:
			bet = true;
			break;
	}
	return bet;
}

int Guild::getRank(int roleId)
{
	vector<int>::iterator it = find(mRank.begin(), mRank.end(), roleId);
	if (it == mRank.end()) {
		return 0;
	}
	return distance(mRank.begin(), it) + 1;
}

void Guild::sendGuildInfo(int roleid)
{
    notify_guild_info ack;
    ack.guildid = getId();
    ack.guildname = getGuildName();
    ack.createdate = getCreateDate();
    if( getDissmissDate() != 0)
        ack.dissmissdate = getDissmissDate() - time(0);//返回给客户端的是余下时间
    ack.membernum = getMembers().size();
    ack.master = getMaster();
    ack.mastername = SRoleGuild(ack.master).getRoleName();
    ack.notice = getNotice();
    ack.fortune = getFortune();
    ack.construction = getConstruction();
    ack.donatermbnum = getDonateRmbNum();
    ack.inspired = getInspired();
	if (getTransferMasterDate() != 0) {
		ack.transferMasterDate = getTransferMasterDate() - time(0);
	}
	
    sendRolePacket(roleid,&ack);
}

void Guild::sendGuildInfoAllMember()
{
    for (GuildMember::iterator itMember = mMembers.begin();
         itMember != mMembers.end(); ++itMember) {
        sendGuildInfo(itMember->first);
    }
}

void Guild::sendAllMemberPacket(INetPacket* packet)
{
    xyassert(packet);

    for (GuildMember::iterator itMember = mMembers.begin();
         itMember != mMembers.end(); ++itMember) {
        sendRolePacket(itMember->first,packet);
    }
}



void Guild::sendMemberList(int roleid, req_guild_member_list& req) const
{
    ack_guild_member_list ack;

    std::pair<int,int> range = checkPageRange(mRank.size(), req.start, req.num);
    ack.start = range.first;

    for (int i = range.first; i < range.second; ++i)
    {
        int id = mRank[i];

        RoleGuildProperty& prop = SRoleGuild(id);
        obj_guild_member mb;
        mb.roleid = id;
        mb.rolename = prop.getRoleName();
        mb.rank = i;
        mb.viplvl = prop.getViplvl();
        mb.job = prop.getJob();
        mb.lvl = prop.getLevel();
        mb.position = prop.getPosition();
        mb.exploit = prop.getExploit();
        mb.lastActive = prop.getLastActive();
        mb.battleForce = prop.getBattleForce();
        mb.roletype = prop.getRoleType();

        ack.members.push_back(mb);
    }

    sendRolePacket(roleid,&ack);
}


int Guild::getLevel()
{
    if(isNull())
        return 0;
    return calGuildLevel(getFortune(), getConstruction());
}

int Guild::getBossLevel()
{
    if( isNull() )
        return 0;
    return calGuildBossLevel( getBossExp() );
}

int Guild::getMaxMemberSize()
{
    return calGuildMaxMember( getLevel() );
}

void Guild::addFortuneConstruction(int fort,int cons, const char* rolename , const char* des)
{
    assert(rolename && des);
    
    if(0 == fort && 0 == cons)
        return;

    stringstream ss;
    string str1 = GameTextCfg::getFormatString("1033", rolename, des);
    
    //ss<<rolename<<"通过"<<des<<"增加了";
    ss << str1;
    if(fort > 0)
    {
        string str = GameTextCfg::getFormatString("1034", fort);
        //ss<<fort<<"公会财富";
        ss << str;
    }
    if(cons > 0)
    {
        string str = GameTextCfg::getFormatString("1035", cons);
        //ss<<cons<<"公会建设度";
        if(fort > 0)
        {
            ss << ",";
        }
        ss << str;
    }
    appendLog(ss.str());
    
    setFortune( getFortune() + fort);
    setConstruction( getConstruction() + cons);
    sendGuildInfoAllMember();//有可能公会等级会变，所以全部重发
    
    Xylog log(eLogName_GuildFortCons, 0);
    log
	<< getId()
	<< getGuildName()
	<< rolename
	<< getFortune()
	<< getConstruction()
	<< fort
	<< cons
	<< des;
}

void Guild::addBossExp(int exp)
{
    setBossExp( exp + getBossExp());
    nty_guild_boss_exp nty_exp;
    nty_exp.exp = getBossExp();
    sendAllMemberPacket(&nty_exp);
}

void Guild::appendLog(const string& log,int level)
{
    mGuildLog.append(log.c_str(),level);
}

void Guild::sendLog(int roleid,int start ,int num)
{
    mGuildLog.send(roleid,start,num);
}

void Guild::appendChatMsg(GuildChatMsg& msg)
{
    mGuildChat.append(msg);
}
void Guild::sendChatMsg(int roleid,int start,int num)
{
    mGuildChat.send(roleid, start, num);
}

bool Guild::guildRename(string newname)
{
    setGuildName(newname);
    
    save();
    
    return true;
}

VoiceInfoItem Guild::getVoiceInfo(int64_t voiceId)
{
    return mGuildChat.findVoiceInfo(voiceId);
}
