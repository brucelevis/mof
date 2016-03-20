//
//  CustomGift.cpp
//  GameSrv
//
//  Created by jin on 13-11-7.
//
//

#include "CustomGift.h"
#include "json/json.h"


string CustomGift::getRandItem()
{
	vector<int> props;
	CustomGiftItemTd::iterator it;
	for (it = mGifts.begin(); it != mGifts.end(); it++) {
		props.push_back((*it).pro);
	}
	if (props.empty()) {
		return "";
	}
	int index = getRand(props.data(), props.size());
	return mGifts[index].items;
}

void CustomGift::loadJson(const string str)
{
	Json::Value main;
	Json::Reader reader;
	if (!reader.parse(str, main)) {
		return;
	}
	char key[5];
	Json::Value value = main["gift"];
	for (int num = 1; num <= value["num"].asInt(); num++) {
		CustomGiftItem cgi;
		sprintf(key, "g%d", num);
		cgi.items = value[key].asString();
		sprintf(key, "p%d", num);
		cgi.pro = value[key].asInt();
		mGifts.push_back(cgi);
	}
	mName = value["name"].asString();
	mDesc = value["desc"].asString();
}


string CustomGift::formatJson()
{
	Json::Value main;
	Json::Value value;
	char key[5];
	
	int num = 0;
	CustomGiftItemTd::iterator it;
	for (it = mGifts.begin(); it != mGifts.end(); it++) {
		num++;
		sprintf(key, "g%d", num);
		value[key] = (*it).items;
		sprintf(key, "p%d", num);
		value[key] = (*it).pro;
	}
	value["num"] = num;
	value["name"] = mName;
	value["desc"] = mDesc;
	
	main["gift"] = value;
	return xyJsonWrite(main);
}