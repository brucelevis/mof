//
//  Constellation.cpp
//  GameSrv
//
//  Created by 志刚 on 13-3-4.
//  Copyright (c) 2013年 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "Constellation.h"
#include "DBRolePropName.h"
#include "main.h"
#include "Role.h"
#include "msg.h"
#include "DataCfg.h"
#include <stdlib.h>
#include "Paihang.h"
#include "GameLog.h"
ConstellationMgr::ConstellationMgr(Role* role)
{
    mRole = role;
}

void ConstellationMgr::LoadDataFromDB()
{
	this->checkCompatible();
	
    constellpro.clear();
	RedisResult result(redisCmd("hgetall constellpro:%d", mRole->getInstID()));
    int elementnum = result.getHashElements();
    for (int i = 0; i < elementnum; i += 2)
    {
		ConstellData constelldata;
        constelldata.consid = Utils::safe_atoi(result.readHash(i, "").c_str(), 0);
        constelldata.step = Utils::safe_atoi(result.readHash(i + 1, "").c_str(), 0);
		constellpro.insert(make_pair(constelldata.consid, constelldata));
    }
}

/*
	兼容旧星座数据,　因为内容多,　不再适合用单键存储
	清空role下面的constellpro键
	新组hash键 constellpro主键
*/
void ConstellationMgr::checkCompatible()
{
	constellpro.clear();
	// 旧键role下面的constellpro键
	RedisResult result(redisCmd("hmget role:%d constellpro", mRole->getInstID()));
	string constellStr = result.readHash(0, "");
	if (constellStr.empty()) {
		return;
	}
	StringTokenizer token(constellStr, ";");
	for (StringTokenizer::Iterator iter = token.begin(); iter != token.end(); ++iter)
	{
		ConstellData constelldata;
		sscanf(iter->c_str(), "%d:%d", &constelldata.consid, &constelldata.step);
		constellpro.insert(make_pair(constelldata.consid, constelldata));
	}
	// 改写旧键, 不删除, 考虑到怕有人重用该键
	doRedisCmd("hmset role:%d constellpro %s", mRole->getInstID(), "");
	// 插入新键
	std::map<int, ConstellData>::iterator it;
	for (it = constellpro.begin(); it != constellpro.end(); it++) {
		doRedisCmd("hmset constellpro:%d %d %d", mRole->getInstID(), it->second.consid, it->second.step);
	}
	return;
}

void ConstellationMgr::SaveConstellationData(int constellid)
{
	ConstellData *data = getConstell(constellid);
	if (NULL == data) {
		return;
	}
	doRedisCmd("hmset constellpro:%d %d %d", mRole->getInstID(), data->consid, data->step);
}


eConstellupgResult ConstellationMgr::UpgConstell(int constellid, int& step)
{
    ConstellCfgDef *cfg = ConstellationCfg::getCfg(constellid);
    if (cfg == NULL) {
        return eConstellupgResult_NoConfig;
    }
	
	ConstellData *data = getConstell(constellid);
	if (NULL == data) {
        ConstellData constell;
        constell.consid = constellid;
        constellpro.insert(make_pair(constell.consid, constell));
		data = getConstell(constellid);
    }
	
	if (data->step >= cfg->steps.size()) {
		return eConstellupgResult_IsFull;
	}
	
	ConstellStepDef &def = cfg->steps[data->step];
    
	
    if (def.getCons() > mRole->getConstellVal()) {
        return eConstellupgResult_NoCoin;
    }
	
	mRole->addConstellVal(-def.getCons(), "星座升级");
    
	int prestep = data->step;
	
	
	RoleConstellFailDef &failDef = mRole->getConstellFailArr().get(cfg->index);
	
    // 检查机率是否升级
    float maxFail = def.getConProMaxFail();
    bool success = false;
    if( maxFail > 0 )
    {
        float accuRate = failDef.accuRate + (failDef.times * def.getConProAdd() + def.getConPro());
        if( accuRate > maxFail )
        {
            // 人品累积完成
            success = true;
        }
        mRole->getConstellFailArr().setAccuRate(cfg->index);
    }

    if( success == false )
    {
        int randSerial = -1;
        if( randomHit(def.getConPro() + failDef.times * def.getConProAdd(),randSerial)  )
        {
            // 符合（补偿）概率命中
            success = true;
        }
    }
    
    if( success )
    {
        data->addStep(1);
		mRole->getConstellFailArr().clear(cfg->index);
    }
    else
    {
		mRole->getConstellFailArr().add(cfg->index);
    }

    
    mRole->saveNewProperty();
	step = data->step;
	
	this->SaveConstellationData(constellid);

	LogMod::addLogConstellationUpgrade(mRole->getInstID(),
									   mRole->getRolename().c_str(),
									   mRole->getSessionId(),
									   data->consid,
									   prestep,
									   data->step,
									   def.getCons(),
									   mRole->getConstellVal());

    return eConstellupgResult_Ok;
}


void ConstellationMgr::CalcProperty(BattleProp& batprop, BaseProp& baseprop)
{
	std::map<int, ConstellData>::iterator it;
	for (it = constellpro.begin(); it != constellpro.end(); it++) {
		ConstellCfgDef*	cfg = ConstellationCfg::getCfg(it->second.consid);
		int step = it->second.step;
		if (NULL == cfg || step <= 0 || step > cfg->step) {
			xyerr("ConstellationMgr::CalcProperty %d %d", it->second.consid, it->second.step);
            continue;
		}
		batprop.mAtk += cfg->steps[step - 1].mAccAtk;
        batprop.mDef += cfg->steps[step - 1].mAccDef;
        batprop.mDodge += cfg->steps[step - 1].mAccDodge;
        batprop.mHit += cfg->steps[step - 1].mAccHit;
        batprop.mCri += cfg->steps[step - 1].mAccCri;
        batprop.mMaxHp += cfg->steps[step - 1].mAccHp;
		
		if (step == cfg->step) {
			batprop.mAtk += cfg->mPageAtk;
			batprop.mDef += cfg->mPageDef;
			batprop.mDodge += cfg->mPageDodge;
			batprop.mHit += cfg->mPageHit;
			batprop.mCri += cfg->mPageCri;
			batprop.mMaxHp += cfg->mPageHp;
		}
	}
}

void ConstellationMgr::getFailtimes(vector<obj_constellTimes> &vec)
{
	RoleConstellFail &times = mRole->getConstellFailArr();
	RoleConstellFail::Iterator it;
	for (it = times.begind(); it != times.end(); it++) {
		obj_constellTimes obj;
		obj.index = it->first;
		obj.failtimes = it->second.times;
		vec.push_back(obj);
	}
}


void ConstellationMgr::sendConstellState()
{
    ack_constellstate ack;
    ack.constellval = mRole->getConstellVal();
	map<int, ConstellData>::iterator it;
	for (it = constellpro.begin(); it != constellpro.end(); it++) {
		obj_constelldata constelldata;
        constelldata.consid = it->second.consid;
        constelldata.step = it->second.step;
        ack.constellpro.push_back(constelldata);
	}
	
    this->getFailtimes(ack.constellTimes);
	mRole->send(&ack);
}

ConstellData * ConstellationMgr::getConstell(int constellid)
{
	map<int, ConstellData>::iterator it = constellpro.find(constellid);
	if (it != constellpro.end()) {
		return &it->second;
	}
	return NULL;
}
