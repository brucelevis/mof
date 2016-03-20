//
//  MailApp.cpp
//  GameSrv
//
//  Created by jin on 14-3-26.
//
//

#include "CustomMailApp.h"
#include "main.h"
#include "Utils.h"
#include "GameLog.h"
#include "hander_include.h"
#include "DynamicConfig.h"
#include "HttpRequestApp.h"
#include "GuildRole.h"
#include "GuildMgr.h"
#include "mail_imp.h"

#define CUSTOM_MAIL_INFO_INTERVAL 300	// 更新定制邮件的时间(秒)
#define CUSTOM_MAIL_SEND_NUM 50	// 单次执行发送的人数

#define CUSTOM_MAIL_FIL_LVL "lvl"
#define CUSTOM_MAIL_FIL_VIPLVL "viplvl"
#define CUSTOM_MAIL_FIL_GUILD "guild"
#define CUSTOM_MAIL_FIL_GUILDLVL "guildlvl"
#define CUSTOM_MAIL_FIL_CHANNEL "channel"
#define CUSTOM_MAIL_FIL_GUILDMASTER "guildmaster"
#define CUSTOM_MAIL_FIL_JOB "job"
#define CUSTOM_MAIL_FIL_SEX "sex"
#define CUSTOM_MAIL_FIL_LASTLOGINTM "lastlogintm"
#define CUSTOM_MAIL_FIL_NOWLOGINTM "nowlogintm"

void CustomMailApp::load(const char* str)
{

}

void CustomMailApp::load(Json::Value &val)
{
	try {
		mId = xyJsonAsInt(val["id"]);
		mSender = val["sender"].asString();
		mTitle = val["title"].asString();
		mContent = val["content"].asString();
		mAttack = val["attack"].asString();
		
		mFiltration = val["filtration"].asString();
		mContinueValid = xyJsonAsInt(val["continueValid"]);
	} catch (...) {
		return;
	}
}


string CustomMailApp::str() const
{
	Json::Value val;
	
    return xyJsonWrite(val);
}

bool CustomMailApp::sendCustomMail(Role *role)
{
	return sendMail(0, mSender.c_str(), role->getRolename().c_str(), mTitle.c_str(), mContent.c_str(), mAttack.c_str(), "");
}


#pragma mark - 定制邮件的过滤条件函数
bool FiltrationLvl(Role *role, const string &fil)
{
	int min = 0;
	int max = 0;
	if (sscanf(fil.c_str(), "%d-%d", &min, &max) == 2) {
		if (role->getLvl() >= min && role->getLvl() <= max)
			return true;
	}
	return false;
}

bool FiltrationVipLvl(Role *role, const string &fil)
{
	int min = 0;
	int max = 0;
	if (sscanf(fil.c_str(), "%d-%d", &min, &max) == 2) {
		if (role->getVipLvl() >= min && role->getVipLvl() <= max)
			return true;
	}
	return false;
}

bool FiltrationGuild(Role *role, const string &fil)
{
	int guild = 0;
	if (sscanf(fil.c_str(), "%d", &guild) == 1) {
		if (SRoleGuild(role->getInstID()).getGuild() == guild)
			return true;
	}
	return false;
}

bool FiltrationGuildLvl(Role *role, const string &fil)
{
	int min = 0;
	int max = 0;
	// 公会不存在
	int gid = SRoleGuild(role->getInstID()).getGuild();
	if (gid == 0) {
		return false;
	}
	if (sscanf(fil.c_str(), "%d-%d", &min, &max) == 2) {
		int lvl = SGuild(gid).getLevel();
		if (lvl >= min && lvl <= max)
			return true;
	}
	return false;
}

bool FiltrationChannel(Role *role, const string &fil)
{
	string channel = SSessionDataMgr.getProp(role->getSessionId(), "platform");
	StringTokenizer token(fil, ",");
	for (StringTokenizer::Iterator it = token.begin(); it != token.end(); it++) {
		if (strcmp((*it).c_str(), channel.c_str()) == 0) {
			return true;
		}
	}
	return false;
}

bool FiltrationJob(Role *role, const string &fil)
{
	int job = -1;
	if (sscanf(fil.c_str(), "%d", &job) == 1) {
		if (role->getJob() == job) {
			return true;
		}
	}
	return false;
}

bool FiltrationSex(Role *role, const string &fil)
{
	int sex = -1;
	if (sscanf(fil.c_str(), "%d", &sex) == 1) {
		if (role->getSex() == sex) {
			return true;
		}
	}
	return false;
}

bool FiltrationGuildMaster(Role *role, const string &fil)
{
	RoleGuildProperty& prop = SRoleGuild(role->getInstID());
	if (prop.getGuild() == 0) {
		return false;
	}
	if (prop.getPosition() != kGuildMaster) {
		return false;
	}
	int guild = -1;
	if (sscanf(fil.c_str(), "%d", &guild) == 1) {
		if (guild == 0 || prop.getGuild() == guild) {
			return true;
		}
	}
	return false;
}

bool FiltrationLastLogin(Role* role, const string &fil)
{
    int lastlogin = role->getLastLogintm();
    int mintm = 0;
    int maxtm = 0;
    if(sscanf(fil.c_str(), "%d-%d", &mintm, &maxtm))
    {
        if(lastlogin < mintm || lastlogin > maxtm)
            return true;
    }
    return false;
}

bool FiltraionNowLogin(Role* role, const string &fil)
{
    int nowlogin = role->mLoginMoment;
    int mintm = 0;
    int maxtm = 0;
    if(sscanf(fil.c_str(), "%d-%d", &mintm, &maxtm))
    {
        if(nowlogin>= mintm && nowlogin <= maxtm)
            return true;
    }
    return false;
}

bool CustomMailAppMou::checkFiltration(Role *role, const string &filtration)
{
	if (filtration.empty()) {
		return true;
	}
	
	StringTokenizer token(filtration, ";");
	for (StringTokenizer::Iterator it = token.begin(); it != token.end(); it++) {
		StringTokenizer fils(*it, " ");
		if (fils.count() != 2) {
			continue;
		}
		if (handleFil(role, fils[0], fils[1]) == false) {
			return false;
		}
	}
	return true;
}

CustomMailAppMou g_CustomMailAppMou;
void CustomMailAppMou::init()
{
	CustomMailAppMou::onRefresh(NULL);
	addTimer(onRefresh, NULL, CUSTOM_MAIL_INFO_INTERVAL, FOREVER_EXECUTE);
	
	// 注册过滤条件
	addFilCb(CUSTOM_MAIL_FIL_LVL, FiltrationLvl);
	addFilCb(CUSTOM_MAIL_FIL_VIPLVL, FiltrationVipLvl);
	addFilCb(CUSTOM_MAIL_FIL_CHANNEL, FiltrationChannel);
	addFilCb(CUSTOM_MAIL_FIL_GUILD, FiltrationGuild);
	addFilCb(CUSTOM_MAIL_FIL_GUILDLVL, FiltrationGuildLvl);
	addFilCb(CUSTOM_MAIL_FIL_GUILDMASTER, FiltrationGuildMaster);
	addFilCb(CUSTOM_MAIL_FIL_JOB, FiltrationJob);
	addFilCb(CUSTOM_MAIL_FIL_SEX, FiltrationSex);
    addFilCb(CUSTOM_MAIL_FIL_LASTLOGINTM, FiltrationLastLogin);
    addFilCb(CUSTOM_MAIL_FIL_NOWLOGINTM, FiltraionNowLogin);
}

void CustomMailAppMou::addFilCb(const string &key, FiltrationCb cb)
{
	if (mFil.find(key) != mFil.end()) {
		return;
	}
	mFil.insert(make_pair(key, cb));
}

bool CustomMailAppMou::handleFil(Role *role, const string &key, const string &fil)
{
	CustomMailFilTd::iterator it = mFil.find(key);
	if (it == mFil.end()) {
		return true;
	}
	return it->second(role, fil);
}

void CustomMailAppMou::load(const char *ret)
{
	Json::Value val;
	if (!xyJsonSafeParse(ret, val)) {
		xyerr("定制邮件 : http ret err，no JSON.");
		return;
	}
	if (val["ret"].isNull() || xyJsonAsInt(val["ret"]) != 0) {
		return;
	}
	mIndex = xyJsonAsInt(val["index"]);
	mRefreshTm = time(NULL);
	mMail.clear();
	// 加载
	Json::Value data = val["data"];
	for (int i = 0; i < data.size(); i++) {
		CustomMailApp app;
		app.load(data[i]);
		if (app.isErr()) {
			continue;
		}
		mMail.insert(make_pair(app.getId(), app));
	}
	// 加载所有玩家到队列
	this->loadRoleQueue();
	
	// 日志记录
	Xylog log(eLogName_CustomMailLoad, 0);
	log<<ret
	<<mMail.size()
	<<mQueue.size();
	
	this->sendProcCmd();
}

void CustomMailAppMou::loadRoleQueue()
{
	RoleMgr::Iterator it;
	for (it = SRoleMgr.begin(); it != SRoleMgr.end(); it++) {
		mQueue.push(it->first);
	}
}

void CustomMailAppMou::roleOnlineCheck(int roleId)
{
	this->loadRoleQueue(roleId);
	this->sendProcCmd();
}

void CustomMailAppMou::loadRoleQueue(int roleId)
{
	mQueue.push(roleId);
}

void CustomMailAppMou::sendCustomMail()
{
	for (int i = 0; i < CUSTOM_MAIL_SEND_NUM; i++) {
		if (mQueue.empty()) {
			return;
		}
		int roleid = mQueue.front();
		Role *role = SRoleMgr.GetRole(roleid);
		if (NULL != role) {
			role->getCustomMailMgr()->send();
		}
		mQueue.pop();
	}
	// 未处理完,　继续发送指令
	this->sendProcCmd();
}

void CustomMailAppMou::sendProcCmd()
{
	create_cmd(send_custom_mail, send_custom_mail);
	sendCmdMsg(Game::MQ, send_custom_mail);
}

void CustomMailAppMou::onRefresh(void *param)
{
	create_cmd(http_request, http_request);
    http_request->fun_name = HTTP_CUSTOM_MAIL_INFO;
    http_request->url = UrlCfg::getUrl(HTTP_CUSTOM_MAIL_INFO);
    http_request->post_param = xystr("serverId=%d", Process::env.getInt("server_id"));
    http_request->is_https = false;
    sendCmdMsg(HttpRequest::MQ, http_request);
}



#pragma mark - 定制邮件人物属性

void CustomMail::load(const char *str)
{
	Json::Value val;
	if (!xyJsonSafeParse(str, val)) {
		return;
	}
	mId = xyJsonAsInt(val["id"]);
	mIsGet = xyJsonAsInt(val["isget"]);
}

string CustomMail::str() const
{
	Json::Value val;
	val["id"] = mId;
	val["isget"] = mIsGet;
    return xyJsonWrite(val);
}

std::ostream& operator<<(std::ostream& out, const CustomMail& obj)
{
	return out << obj.str();
}

std::istream& operator>>(std::istream& in, CustomMail& obj)
{
	string tmp;
	in>>tmp;
	obj.load(tmp.c_str());
	return in;
}

CustomMailMgr::CustomMailMgr()
{
	mRole = NULL;
}

void CustomMailMgr::load(Role *role)
{
	mRole = role;
	int roleid = mRole->getInstID();
	
	mCustomMailMap.load(roleid);
}

void CustomMailMgr::send()
{
	CustomMailAppMou::Iterator begin = g_CustomMailAppMou.begin();
	CustomMailAppMou::Iterator end = g_CustomMailAppMou.end();
	RoleCustomMailTd::iterator roleBegin = mCustomMailMap.begin();
	RoleCustomMailTd::iterator roleEnd = mCustomMailMap.end();
	
	//　获取玩家没有领取过的邮件
	list<CustomMailApp*> get;
	xySetDifferenceMap(begin, end, roleBegin, roleEnd, get);
	
	//　检查过滤,　发送邮件
	for (list<CustomMailApp*>::iterator it = get.begin(); it != get.end(); it++) {
		CustomMailApp *tmp = *it;
		bool bet = g_CustomMailAppMou.checkFiltration(mRole, tmp->getFiltration());
		if (bet) {
			// 发送邮件, 发送失败时, 下次继续检查发送
			if (!tmp->sendCustomMail(mRole)) {
				continue;
			}
		} else if (tmp->getContinueValid() != 0){
			// 条件不满足, 若是持续有效, 不打标记
			continue;
		}
		CustomMail cm(tmp->getId(), bet);
		mCustomMailMap.set(cm.getId(), cm);
		
		// 日志
		Xylog log(eLogName_CustomMailSend, mRole->getInstID());
		log<<tmp->getId()
		<<tmp->getTitle()
		<<tmp->getAttack()
		<<tmp->getFiltration()
		<<bet;
	}
	// 移除不存在的邮件
	removeExpiredMail(g_CustomMailAppMou.getIndex());
}

void CustomMailMgr::removeExpiredMail(int index)
{
	RoleCustomMailTd::iterator it;
	for (it = mCustomMailMap.begin(); it != mCustomMailMap.end(); ) {
		if (it->first < index) {
			mCustomMailMap.remove((it++)->first);
		} else {
			break;
		}
	}
}


