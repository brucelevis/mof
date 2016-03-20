//
//  BroadcastNotify.cpp
//  GameSrv
//
//  Created by prcv on 13-7-4.
//
//

#include <stdio.h>
#include <string>
#include "Defines.h"
#include "msg.h"
#include "Utils.h"
#include "Game.h"
#include "main.h"
#include "BroadcastNotify.h"
using namespace std;

/*
kBroadcastSystem = 0,
kBroadcastGetPet = 1,
kBroadcastPromoteEquip = 2,
kBroadcastDungeon = 3,
kBroadcastPvpRank = 4,
kBroadcastPetStar = 5,
kBroadcastGetStone = 6,
kBroadcastVipLvl = 7,

 0.系统                        content=color&content1=color1&content2=color2
 1.获得紫色\粉色幻兽            rolename=abcd&petmod=310001
 2.获得(提品)了紫色\粉色装备    rolename=abcd&equipid=310001
 3.通关地下城20层             rolename=abcd&dungeonid=310000&floor=12
 4.竞技场排名提升到第一        rolename=abcd&rank=310001
 5.幻兽提升到8\9\10星级       rolename=abcd&petmod=310001&petstar=8
 6.合成8级或以上的石头         rolename=abcd&stoneid=310001
 7.vip等级提升                rolename=abcd&lvl=7
 */

void broadcastSystem(string content, int noViewSystem)
{
    notify_broadcast notify;
    notify.createDate = Game::tick;
    notify.type = kBroadcastSystem;
    notify.content = content;
	notify.noViewSystem = noViewSystem;
    broadcastPacket(WORLD_GROUP, &notify);
}


static void appendKeyVal(string& src,  const char* key, const char* val)
{
    src.append(key);
    src.append(Utils::sContentColorSplitStr);
    src.append(val);
    src.append(Utils::sLabelSplitStr);
}

static void appendKeyVal(string& src,  const char* key, int val)
{
    src.append(key);
    src.append(Utils::sContentColorSplitStr);
    src.append(Utils::itoa(val));
    src.append(Utils::sLabelSplitStr);
}

void broadcastGetPet(string rolename, int petmod)
{
    notify_broadcast notify;
    notify.createDate = Game::tick;
    notify.type = kBroadcastGetPet;
    appendKeyVal(notify.content, "rolename", rolename.c_str());
    appendKeyVal(notify.content, "petmod", petmod);
    broadcastPacket(WORLD_GROUP, &notify);
}

void broadcastPromoteEquip(string rolename, int equipid)
{
    notify_broadcast notify;
    notify.createDate = Game::tick;
    notify.type = kBroadcastPromoteEquip;
    appendKeyVal(notify.content, "rolename", rolename.c_str());
    appendKeyVal(notify.content, "equipid", equipid);
    broadcastPacket(WORLD_GROUP, &notify);
}


void broadcastDungeon(string rolename, int dungeonid, int floor)
{
    notify_broadcast notify;
    notify.createDate = Game::tick;
    notify.type = kBroadcastDungeon;
    appendKeyVal(notify.content, "rolename", rolename.c_str());
    appendKeyVal(notify.content, "dungeonid", dungeonid);
    appendKeyVal(notify.content, "floor", floor);
    broadcastPacket(WORLD_GROUP, &notify);
}


void broadcastPvpRank(string rolename, int rank)
{
    notify_broadcast notify;
    notify.createDate = Game::tick;
    notify.type = kBroadcastPvpRank;
    appendKeyVal(notify.content, "rolename", rolename.c_str());
    appendKeyVal(notify.content, "rank", rank);
    broadcastPacket(WORLD_GROUP, &notify);
}


void broadcastPetStar(string rolename, int petmod, int petstar)
{
    notify_broadcast notify;
    notify.createDate = Game::tick;
    notify.type = kBroadcastPetStar;
    appendKeyVal(notify.content, "rolename", rolename.c_str());
    appendKeyVal(notify.content, "petmod", petmod);
    appendKeyVal(notify.content, "petstar", petstar);
    broadcastPacket(WORLD_GROUP, &notify);
}


void broadcastGetStone(string rolename, int stoneid)
{
    notify_broadcast notify;
    notify.createDate = Game::tick;
    notify.type = kBroadcastGetStone;
    appendKeyVal(notify.content, "rolename", rolename.c_str());
    appendKeyVal(notify.content, "stoneid", stoneid);
    broadcastPacket(WORLD_GROUP, &notify);
}


void broadcastVipLvl(string rolename, int lvl)
{
    notify_broadcast notify;
    notify.createDate = Game::tick;
    notify.type = kBroadcastVipLvl;
    appendKeyVal(notify.content, "rolename", rolename.c_str());
    appendKeyVal(notify.content, "lvl", lvl);
    broadcastPacket(WORLD_GROUP, &notify);
}



void broadcastPetMatchResult(int type ,const bcMatchRetParam& param)
{
    notify_broadcast notify;
    notify.createDate = Game::tick;
    notify.type = type;
    appendKeyVal(notify.content, "round", param.round);
    appendKeyVal(notify.content, "winers", param.winers);
    appendKeyVal(notify.content, "wowner", param.wowner.c_str());
    appendKeyVal(notify.content, "wpetmod", param.wpetmod);
    appendKeyVal(notify.content, "losers", param.losers);
    appendKeyVal(notify.content, "lowner", param.lowner.c_str());
    appendKeyVal(notify.content, "lpetmod", param.lpetmod);
    broadcastPacket(WORLD_GROUP, &notify);
}

void broadcastPetMatchDiscerningEyes(const bcMatchPetDiscernEyeParam& param)
{
    notify_broadcast notify;
    notify.createDate = Game::tick;
    notify.type = kBroadcastPetMatchDiscerningEyes;
    appendKeyVal(notify.content, "name1", param.name1.c_str());
    appendKeyVal(notify.content, "name2", param.name2.c_str());
    appendKeyVal(notify.content, "name3", param.name3.c_str());
    appendKeyVal(notify.content, "wowner", param.wowner.c_str());
    appendKeyVal(notify.content, "wpetmod", param.wpetmod);
    broadcastPacket(WORLD_GROUP, &notify);
}


/*void SystemBroadcast::append(int color,const char* str,...)
{
    MAKE_VA_STR_DEF(str);
    
    string colorstr;
    map<int, string>::iterator itColor = mColor.find(color);
    if(itColor == mColor.end())
        colorstr = mColor[COLOR_WHITE];
    else
        colorstr = itColor->second;
    
    ss<<Utils::sLabelSplitStr<<strstr<<Utils::sContentColorSplitStr<<colorstr<<Utils::sLabelSplitStr;
}*/

SystemBroadcast& SystemBroadcast::operator<<(const char* str)
{
    if (!mReady) {
        mVarStr.compile(str);
        mReady = true;
    } else {
        mVarStr << str;
    }
    
    return *this;
}


SystemBroadcast& SystemBroadcast::operator<<(int value)
{
    if (mReady) {
        string str = Utils::itoa(value);
        mVarStr << str.c_str();
    }
    
    return *this;
}

/*
void SystemBroadcast::append(const char *str, ...)
{
    MAKE_VA_STR_DEF(str);
    vector<string> strarr = StrSpilt(strstr, "|");
    if (strarr.size() % 2)
    {
        return;
    }
    for (int i = 0; i < strarr.size(); i = i + 2)
    {
        string colorstr;
        do
        {
            int color = atoi(strarr[i].c_str());
            if (0 == color)
            {
                colorstr = strarr[i];
                break;
            }
            
            map<int, string>::iterator itColor = mColor.find(color);
            if(itColor == mColor.end())
            {
                colorstr = mColor[COLOR_WHITE];
            }
            else
            {
                colorstr = itColor->second;
            }
        } while (0);
        
        ss << Utils::sLabelSplitStr << strarr[i+1] << Utils::sContentColorSplitStr
        << colorstr << Utils::sLabelSplitStr;
    }
}
*/

void SystemBroadcast::clear()
{
    ss.clear();
    ss.str("");
}

void SystemBroadcast::send(int noViewSystem)
{
    string strstr = mVarStr.getStr();
    vector<string> strarr = StrSpilt(strstr, "|");
    if (strarr.size() % 2)
    {
        return;
    }
    for (int i = 0; i < strarr.size(); i = i + 2)
    {
        int colorTag = atoi(strarr[i].c_str());
        string colorstr = ColorCfg::getColor(colorTag);
        
        ss << Utils::sLabelSplitStr << strarr[i+1] << Utils::sContentColorSplitStr
        << colorstr << Utils::sLabelSplitStr;
    }
    
    broadcastSystem(ss.str(), noViewSystem);
}

