//
//  MailApp.h
//  GameSrv
//
//  Created by jin on 14-3-26.
//
//

#ifndef __GameSrv__CustomMailApp__
#define __GameSrv__CustomMailApp__

#include "RedisDhash.h"
#include "std_includes.h"

class Role;

#pragma mark - 定制邮件管理类
class CustomMailApp
{
public:
	CustomMailApp() : mId(0), mContinueValid(0) {};
	~CustomMailApp(){}

	void load(const char* str);
	void load(Json::Value &val);
    string str() const;
	
	int getId() {return mId;}
	string & getFiltration() {return mFiltration;}
	bool isErr() {return mId == 0 ? true : false;}
	int getContinueValid() {return mContinueValid;}
	string & getTitle() {return mTitle;}
	string & getAttack() {return mAttack;}
	
	bool sendCustomMail(Role *role);
	
private:
	// ID
	int mId;
	
	string mSender;
	string mTitle;
	string mContent;
	string mAttack;
	
	// 过滤条件
	string mFiltration;
	// 持续有效
	int mContinueValid;
};


class CustomMailAppMou
{
public:
	typedef map<int, CustomMailApp> CustomMailAppTd;
	typedef CustomMailAppTd::iterator Iterator;
	
	typedef bool (*FiltrationCb)(Role *role, const string &fil);
	typedef map<string, FiltrationCb> CustomMailFilTd;
public:
	CustomMailAppMou() : mIndex(0), mRefreshTm(0) {}
	void init();
	//　加载从http返回的定制邮件信息
	void load(const char *ret);
	
	CustomMailAppTd::iterator begin() {return mMail.begin();}
	CustomMailAppTd::iterator end() {return mMail.end();}
	
	static void onRefresh(void *param);
	bool checkFiltration(Role *role, const string &filtration);
	
	// 发送邮件
	void sendCustomMail();
	int getIndex() {return mIndex;}
	
	void roleOnlineCheck(int roleId);
private:
	void loadRoleQueue();
	void loadRoleQueue(int roleId);
	void sendProcCmd();
	
	CustomMailAppTd mMail;
	
	// 过滤条件的执行函数
	CustomMailFilTd mFil;
	void addFilCb(const string &key, FiltrationCb cb);
	bool handleFil(Role *role, const string &key, const string &fil);
	
	// 邮件定位,　用于移除不存在邮件
	int mIndex;
	int mRefreshTm;
	queue<int> mQueue;
};
extern CustomMailAppMou g_CustomMailAppMou;


#pragma mark - 定制邮件人物属性
class CustomMail
{
public:
	CustomMail(){}
	CustomMail(int id, int isGet) : mId(id), mIsGet(isGet) {}
	~CustomMail(){}
	
	void load(const char* str);
	string str() const;
	
	int getId() {return mId;}
	int getIsGet() {return mIsGet;}
private:
	int mId;	// 邮件id
	int mIsGet;	// 是否获得
};

typedef RedisDhash<int, CustomMail, rnMailCustom> RoleCustomMailTd;
class CustomMailMgr
{
public:
	CustomMailMgr();
	void load(Role *role);
	
	void send();
private:
	void removeExpiredMail(int index);
	
	Role *mRole;
	RoleCustomMailTd mCustomMailMap;
};

#endif /* defined(__GameSrv__CustomMailApp__) */
