//
//  BroadcastNotify.h
//  GameSrv
//
//  Created by prcv on 13-7-4.
//
//

#ifndef GameSrv_BroadcastNotify_h
#define GameSrv_BroadcastNotify_h


#include <string>
#include <sstream>
#include <iostream>
#include <map>
using namespace std;

void broadcastSystem(string content, int noViewSystem = 0);
void broadcastGetPet(string rolename, int petmod);
void broadcastPromoteEquip(string rolename, int equipid);
void broadcastDungeon(string rolename, int dungeonid, int floor);
void broadcastPvpRank(string rolename, int rank);
void broadcastPetStar(string rolename, int petmod, int petstar);
void broadcastGetStone(string rolename, int stoneid);
void broadcastVipLvl(string rolename, int lvl);


struct bcMatchRetParam
{
    int     round;      //1:1/4决赛 2:半决赛 3:决赛
    int     winers;     //支持者人数
    string  wowner;     //主人
    int     wpetmod;    //幻兽模版ID
    int     losers;     //支持者人数
    string  lowner;     //主人
    int     lpetmod;    //幻兽模版ID
};

struct bcMatchPetDiscernEyeParam
{
    string  name1;      //支持者1
    string  name2;      //支持者2
    string  name3;      //支持者3
    string  wowner;     //主人名
    int     wpetmod;    //幻兽模版ID
};
void broadcastPetMatchResult(int type, const bcMatchRetParam& param);
void broadcastPetMatchDiscerningEyes(const bcMatchPetDiscernEyeParam& param);


#include "VarStr.h"

class SystemBroadcast
{
public:
    SystemBroadcast(){mReady = false;}
    ~SystemBroadcast(){}
    //void append(int color,const char* str,...);
    //void append(const char *str, ...);
    void clear();
    void send(int noViewSystem = 0);
    
    SystemBroadcast& operator<<(const char* str);
    SystemBroadcast& operator<<(int value);
    
    
protected:
    stringstream ss;
    
    bool mReady;
    VarStr mVarStr;
};


#endif
