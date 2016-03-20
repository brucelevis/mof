//
//  CustomGift.h
//  GameSrv
//
//  Created by jin on 13-11-7.
//
//

#ifndef __GameSrv__CustomGift__
#define __GameSrv__CustomGift__

#include <iostream>
#include <string>
#include "Defines.h"
#include "Singleton.h"
#include "../global/Activity.h"


// 自定义内容
struct CustomGiftItem
{
	string items;
	int pro;
};

// 随机自定义内容包
class CustomGift
{
public:
	CustomGift() {}
	CustomGift(const string &str) {
		loadJson(str);
	}
	
	void setName(const string &name) {
		mName = name;
	}
	
	void setDesc(const string &desc) {
		mDesc = desc;
	}
	
	// 添加自定义内容
	void addCustomGiftItem(const string &items, int pro=1)
	{
		CustomGiftItem cgi;
		cgi.items = items;
		cgi.pro = pro;
		mGifts.push_back(cgi);
	}
	
	// 根据权值,　获取随机物品
	string getRandItem();
	
	// 格式json串
	void loadJson(const string str);
	
	// 输出json串
	string formatJson();
private:
	typedef vector<CustomGiftItem> CustomGiftItemTd;
	CustomGiftItemTd mGifts;
	// 名称
	string mName;
	// 描述说明
	string mDesc;
};

#endif /* defined(__GameSrv__CustomGift__) */
