//
//  PlayerEquip.h
//  GameSrv
//
//  Created by pireszhi on 15-6-12.
//
//
#ifndef __GameSrv__PlayerEquip__
#define __GameSrv__PlayerEquip__

#include "std_includes.h"
#include "ItemHelper.h"

class Role;
using namespace std;

struct BaptizeData
{
    string propName;
    float value;
};

struct Baptize
{
    int qua;
    vector<BaptizeData> props;
};

class EquipBaptize
{
public:
    
    static bool equipBaptize(int roleid, int equipIndex, ItemGroup& equip, int qua, int index, string& propName, string& value);
    
    static bool calcBaptizeProp(ItemGroup& equip, int job, BattleProp& batProp);
    
    static bool calcBaptizeProp(string& baptizeData, int equipQua, int job, BattleProp& batProp);
    
    static void checkPlayerEquipBaptize2TimeOut(Role* role);
    
    static bool recoverBaptizeProp(int roleid, int equipIndex, ItemGroup& equip, int qua, int index);
    
private:
    
    static string setQuaBatize(string& data, int index, const char* propName, const char* value);
    
    static string setBaptizeProp(string& baptizeData, int qua, int index, const char* propName, const char* value);
    
    static void deleteBackupBaptizeProp(ItemGroup& equip);
    
    static bool calcBaptizeQuaProp(string& data, int job, BattleProp& batProp);
    
};

#endif

