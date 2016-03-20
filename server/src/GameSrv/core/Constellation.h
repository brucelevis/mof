//
//  Constellation.h
//  GameSrv
//
//  Created by 志刚 on 13-3-4.
//  Copyright (c) 2013年 __MyCompanyName__. All rights reserved.
//

//  星座管理

#ifndef GameSrv_Constellation_h
#define GameSrv_Constellation_h
#include "DataCfg.h"
class Role;

class ConstellData
{
public:
    int consid;
    int step;
	
	void addStep(int value) {
		step += value;
	}
    
    ConstellData() : consid(0), step(0) {
    }
};

class ConstellationMgr
{
public:
    ConstellationMgr(Role* role);
    ~ConstellationMgr() {}
    static ConstellationMgr* create(Role* role)
    {
        return new ConstellationMgr(role);
    }
    
    
    void LoadDataFromDB();
    void SaveConstellationData(int constellid);
    void CalcProperty(BattleProp& batprop, BaseProp& baseprop);
	
    eConstellupgResult UpgConstell(int constellid, int& step);
	
    void sendConstellState();
	void getFailtimes(vector<obj_constellTimes> &obj);
    
	// 取星座最高
    ConstellData getConstellData()
    {
        ConstellData constell;
        constell.step = 0;
        constell.consid = 0;
		if (!constellpro.empty()) {
			constell = constellpro.rbegin()->second;
		}
        return constell;
    }
    
	// 根据id获取星座
	ConstellData * getConstell(int constellid);
private:
	// 兼容旧键
	void checkCompatible();
	
    map<int, ConstellData> constellpro;
    
	Role *mRole;
};


#endif
