//
//  WebStateMgr.h
//  GameSrv
//
//  Created by jin on 14-10-21.
//
//

#ifndef __GameSrv__WebStateMgr__
#define __GameSrv__WebStateMgr__

#include <iostream>
#include <string>
#include <vector>

using namespace std;

class WebStateMou
{
public:
	WebStateMou() {
		mState = 0;
		mAppCmdState = 0;
	}
	
	~WebStateMou(){}
	
	void init();
	void load(const char* ret);
	void loadIpList(const char* ret);
	
	static void onRefresh(void *);
	static void onRefreshIpList(void *);
	static void onRefreshUpServer(void *);
	
	bool isState() {return mState == 0 ? false : true;}
	bool isSafeIp(const char *ip);
	bool isAppCmd() {return mAppCmdState == 0 ? false : true;}
	string getTips() {return mTips;}
private:
	int mState;
	string mTips;
	int mAppCmdState;
	
	vector<string> mIpList;
};

extern WebStateMou g_WebStateMou;

#endif /* defined(__GameSrv__WebStateMgr__) */
