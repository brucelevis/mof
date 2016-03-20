 //
//  RedisHashObj.cpp
//  GameSrv
//
//  Created by Huang Kunchao on 13-8-21.
//
//

#include "RedisHashObj.h"
#include "GameLog.h"
#include "Utils.h"
#include "json/json.h"
#include "DataCfg.h"
#include "EnumDef.h"
#include <algorithm>


void IntArrayObj::load(const char* str)
{
    mData.clear();

    StringTokenizer token(str,";");
    StringTokenizer::Iterator iter;
    for (iter = token.begin(); iter != token.end(); ++iter)
    {
        mData.push_back( atoi(iter->c_str()) );
    }
}

string IntArrayObj::str()
{
    return StrJoin(mData.begin(), mData.end(),";");
}

void IntArrayObj::push_back(int val)
{
    dirty(true);
    mData.push_back(val);
}

void IntArrayObj::clear()
{
    dirty(true);
    mData.clear();
}

void IntArrayObj::erase(int val)
{
    dirty(true);
    iterator iter = std::find(mData.begin(), mData.end(), val);
    if( iter != mData.end())
        mData.erase(iter);
}

int IntArrayObj::size() const
{
    return mData.size();
}

bool IntArrayObj::exist(int val) const 
{
    return std::find(mData.begin(), mData.end(), val) != mData.end();
}

IntArrayObj& IntArrayObj::operator = (const vector<int>& rhs)
{
    dirty(true);
    mData = rhs;
    return *this;
}

void PetUpSkillProb::load(const char* str)
{
    xyassert(str);
    mUpSkillProbDataList.clear();
    
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(str, value)) {
        return;
    }
    
    try {
        for (int i = 0; i < value.size(); ++i) {
            PetUpSkillProbData pusb;
            pusb.skillid = value[i]["skillid"].asInt();
            pusb.upSkillFailTimes = value[i]["upskillfailtimes"].asInt();
            pusb.upFailTotalProb = value[i]["upfailtotalprob"].asDouble();
            pusb.upSkillProb = value[i]["upskillprob"].asDouble();
            mUpSkillProbDataList.insert(make_pair(pusb.skillid, pusb));
        }
    } catch (...) {
        
    }
}

string PetUpSkillProb::str()
{
    Iterator it;
    Json::Value value;
    for (it = mUpSkillProbDataList.begin(); it != mUpSkillProbDataList.end(); ++it) {
        Json::Value tmp;
        tmp["skillid"] = (it->second).skillid;
        tmp["upskillfailtimes"] = (it->second).upSkillFailTimes;
        tmp["upfailtotalprob"] = (it->second).upFailTotalProb;
        tmp["upskillprob"] = (it->second).upSkillProb;
        value.append(tmp);
    }
    return Json::FastWriter().write(value);
}

void PetUpSkillProb::clear()
{
    dirty(true);
    mUpSkillProbDataList.clear();
}

//角色公会技能
void RoleGuildSkill::load(const char* str)
{
    xyassert(str);
    mSkill.clear();

    StringTokenizer token(str,";");
    StringTokenizer::Iterator iter;
    for (iter = token.begin(); iter != token.end(); ++iter) {
        int skill = 0, level = 0;
        sscanf(iter->c_str(), "%d-%d",&skill,&level);
        if(0 == skill)
        {
            mSkill.clear();
            xyassert(skill);
        }
        mSkill.insert(make_pair(skill,level));
    }
}

string RoleGuildSkill::str()
{
    SkillMap::iterator iter;
    vector<string> tmp;
    for (iter = mSkill.begin(); iter != mSkill.end(); ++iter) {
        tmp.push_back( Utils::makeStr("%d-%d",iter->first,iter->second));
    }
    return StrJoin(tmp.begin(), tmp.end(),";");
}

void RoleGuildSkill::set(int id , int lvl)
{
    dirty(true);
    mSkill[id] = lvl;
}

int RoleGuildSkill::get(int id)
{
    SkillMap::iterator iter = mSkill.find(id);
    if(iter == mSkill.end())
        return 0;
    return iter->second;
}

// 修正非法技能的逻辑处理　04-23 by jin
void RoleGuildSkill::correctIllegalSkill()
{
	dirty(true);
    for (SkillMap::iterator iter = mSkill.begin(); iter != mSkill.end();) {
		if (iter->first % 100 > 0) {
			mSkill.erase(iter++);
		}
		else
		{
			iter++;
		}
    }
}

void RoleHonorAction::load(const char* str)
{
	xyassert(str);
    mActionVec.clear();

	Json::Reader reader;
    Json::Value value;

    if (!reader.parse(str, value)) {
        return;
    }

	if (!value.isArray()) {
		return;
	}

	for (int i = 0; i < value.size(); i++) {
		RoleHonorActionDef ha;
		ha.key = value[i]["key"].asInt();
		ha.cond = value[i]["cond"].asString();
		ha.param = value[i]["param"].asString();
		ha.time = xyJsonAsInt(value[i]["param"]);
		mActionVec.push_back(ha);
	}
}

string RoleHonorAction::str()
{
	Iterator it;
	Json::Value value;
    for (it = mActionVec.begin(); it != mActionVec.end(); it++) {
		Json::Value tmp;
		tmp["key"] = it->key;
		tmp["cond"] = it->cond;
		tmp["param"] = it->cond;
		tmp["time"] = it->time;

		value.append(tmp);
    }
    return Json::FastWriter().write(value);
}



void RoleHonorAction::add(int key, string cond, string param)
{
	dirty(true);
	RoleHonorActionDef ha;
	ha.key = key;
	ha.cond = cond;
	ha.param = param;
	ha.time = time(NULL);
	mActionVec.push_back(ha);
}


void RoleHonorAction::clear()
{
	dirty(true);
	mActionVec.clear();
}

void RoleMidastouchAction::load(const char* str)
{
	xyassert(str);
    mActionVec.clear();
	
	Json::Reader reader;
    Json::Value value;
	
    if (!reader.parse(str, value)) {
        return;
    }
		
	for (int i = 0; i < value.size(); i++) {
		mActionVec.push_back(xyJsonAsInt(value[i]));
	}
}

string RoleMidastouchAction::str()
{
	Iterator it;
	Json::Value value;
    for (it = mActionVec.begin(); it != mActionVec.end(); it++) {
		value.append(*it);
    }
    return xyJsonWrite(value);
}

void RoleMidastouchAction::add(int rmb)
{
	dirty(true);
	mActionVec.push_back(rmb);
}


void RoleMidastouchAction::clear()
{
	dirty(true);
	mActionVec.clear();
}

void RoleConstellFail::load(const char* str)
{
	xyassert(str);
    mConstellFail.clear();
	
	Json::Reader reader;
    Json::Value value;
	
    if (!reader.parse(str, value)) {
        return;
    }
	
	for (int i = 0; i < value.size(); i++) {
		RoleConstellFailDef def;
		def.id = xyJsonAsInt(value[i]["id"]);
		def.times = xyJsonAsInt(value[i]["times"]);
		def.accuRate = value[i]["accuRate"].asDouble();
		mConstellFail[def.id] = def;
	}
}

string RoleConstellFail::str()
{
	Iterator it;
	Json::Value value;
    for (it = mConstellFail.begin(); it != mConstellFail.end(); it++) {
		Json::Value tmp;
		tmp["id"] = it->first;
		tmp["times"] = it->second.times;
		tmp["accuRate"] = it->second.accuRate;
		value.append(tmp);
    }
    return xyJsonWrite(value);
}

void RoleConstellFail::add(int key)
{
	dirty(true);
	mConstellFail[key].times++;
}

void RoleConstellFail::clear(int key)
{
	dirty(true);
	mConstellFail[key].clear();
}

void RoleConstellFail::setAccuRate(int key, float val)
{
	dirty(true);
	mConstellFail[key].accuRate = val;
}

void RoleTwistEgg::load(const char* str)
{
	xyassert(str);
    mData.clear();
	
	Json::Reader reader;
    Json::Value value;
	
    if (!reader.parse(str, value)) {
        return;
    }
	
	if (!value.isArray()) {
		return;
	}
	
	for (int i = 0; i < value.size(); i++) {
		RoleTwistEggDef def;
		def.id = xyJsonAsInt(value[i]["id"]);
		def.first = xyJsonAsInt(value[i]["first"]);
		def.free = xyJsonAsInt(value[i]["free"]);
		def.freeTime = xyJsonAsInt(value[i]["freeTime"]);
		mData[def.id] = def;
	}
}

string RoleTwistEgg::str()
{
	Json::Value value;
    for (Iterator it = mData.begin(); it != mData.end(); it++) {
		Json::Value tmp;
		tmp["id"] = it->second.id;
		tmp["first"] = it->second.first;
		tmp["free"] = it->second.free;
		tmp["freeTime"] = it->second.freeTime;
		value.append(tmp);
    }
    return xyJsonWrite(value);
}

RoleTwistEggDef& RoleTwistEgg::get(int key)
{
	Iterator it = mData.find(key);
	if (it != mData.end()) {
		return it->second;
	}
	RoleTwistEggDef& def = mData[key];
	def.id = key;
	return def;
}

void RoleTwistEgg::refresh()
{
	for (Iterator it = mData.begin(); it != mData.end(); it++) {
		it->second.refresh();
	}
	this->save();
}

void RoleTwistEgg::save()
{
	dirty(true);
}

void RoleDungeonState::load(const char* str)
{
    xyassert(str);
    mStates.clear();
    
	Json::Reader reader;
    Json::Value value;
    
    if (!reader.parse(str, value)) {
        return;
    }
    
	try
	{
		for (int i = 0; i < value.size(); i++) {
            Json::Value &stateValue = value[i];
			RoleDungeonStateDef state;
			state.mIndex = stateValue["index"].asInt();
			state.mResetTimes = stateValue["reset_times"].asInt();
			state.mDungeonLevel = stateValue["dungeon_level"].asInt();
			state.mSweepLevel = stateValue["sweep_level"].asInt();
            mStates[state.mIndex] = state;
		}
	}
	catch(...)
	{
	}

}

string RoleDungeonState::str()
{
    Iterator it;
	Json::Value value;
    for (it = mStates.begin(); it != mStates.end(); it++) {
		Json::Value tmp;
		tmp["index"] = it->second.mIndex;
		tmp["reset_times"] = it->second.mResetTimes;
		tmp["dungeon_level"] = it->second.mDungeonLevel;
		tmp["sweep_level"] = it->second.mSweepLevel;
        
		value.append(tmp);
    }
    return Json::FastWriter().write(value);
}

RoleDungeonState::Iterator RoleDungeonState::setSweepLevel(int index, int level)
{
    Iterator iter = findOrInsert(index);
    iter->second.mSweepLevel = level;
    dirty(true);
    return iter;
}

RoleDungeonState::Iterator RoleDungeonState::setDungeonLevel(int index, int level)
{
    Iterator iter = findOrInsert(index);
    iter->second.mDungeonLevel = level;
    dirty(true);
    return iter;
}

RoleDungeonState::Iterator RoleDungeonState::setResetTimes(int index, int times)
{
    Iterator iter = findOrInsert(index);
    iter->second.mResetTimes = times;
    dirty(true);
    return iter;
}

RoleDungeonState::Iterator RoleDungeonState::addResetTimes(int index, int times)
{
    Iterator iter = findOrInsert(index);
    iter->second.mResetTimes += times;
    dirty(true);
    return iter;
}

int RoleDungeonState::getSweepLevel(int index)
{
    Iterator iter = mStates.find(index);
    if (iter != mStates.end()) {
        return iter->second.mSweepLevel;
    }
    
    return 0;
}

int RoleDungeonState::getDungeonLevel(int index)
{
    Iterator iter = mStates.find(index);
    if (iter != mStates.end()) {
        return iter->second.mDungeonLevel;
    }
    
    return 0;
}

int RoleDungeonState::getResetTimes(int index)
{
    
    Iterator iter = mStates.find(index);
    if (iter != mStates.end()) {
        return iter->second.mResetTimes;
    }
    
    return 0;
}

void RoleDungeonState::refresh()
{
    for (Iterator iter = begin(); iter != end(); iter++) {
        iter->second.mDungeonLevel = 0;
        iter->second.mResetTimes = 0;
    }
    dirty(true);
}

void RoleDungeonState::clear()
{
    mStates.clear();
    dirty(true);
}
	


const char* const WardrobeFashionList::IdKey = "id";
const char* const WardrobeFashionList::ExpireTimeKey = "expiretime";
const char* const WardrobeFashionList::IsExpiredKey = "expired";


WardrobeFashionList::~WardrobeFashionList()
{
    for (int i = 0; i < mFashionList.size(); i++) {
        delete mFashionList[i];
    }
}

void WardrobeFashionList::load(const char* str)
{
	Json::Reader reader;
    Json::Value value;
    
    if (!reader.parse(str, value) || !value.isArray())
    {
        return;
    }
    
	try
	{
		for (int i = 0; i < value.size(); i++)
        {
			WardrobeFashion* wf = new WardrobeFashion;
			wf->mId = value[i][IdKey].asInt();
			wf->mExpireTime = value[i][ExpireTimeKey].asInt();
            wf->mExpired = value[i][IsExpiredKey].asInt();
            mFashionList.push_back(wf);
		}
	}
	catch(...)
	{
	}
}

string WardrobeFashionList::str()
{
	Json::Value value;
    for (vector<WardrobeFashion*>::iterator iter = mFashionList.begin();
         iter != mFashionList.end(); iter++)
    {
		Json::Value tmp;
        WardrobeFashion* fashion = *iter;
		tmp[IdKey] = fashion->mId;
		tmp[ExpireTimeKey] = fashion->mExpireTime;
        tmp[IsExpiredKey] = (int)fashion->mExpired;
		value.append(tmp);
    }
    return Json::FastWriter().write(value);
}


WardrobeFashionList::Iterator WardrobeFashionList::begin()
{
    return mFashionList.begin();
}

WardrobeFashionList::Iterator WardrobeFashionList::end()
{
    return mFashionList.end();
}

WardrobeFashion* WardrobeFashionList::addFashion(int fashionId, int expireTime)
{
    WardrobeFashion* fashion = new WardrobeFashion;
    fashion->mExpired = false;
    fashion->mId = fashionId;
    fashion->mExpireTime = expireTime;
    mFashionList.insert(mFashionList.end(), fashion);
	dirty(true);
    return fashion;
}

void WardrobeFashionList::updateExpiration(WardrobeFashion* fashion, int expireTime)
{
    fashion->mExpireTime = expireTime;
    fashion->mExpired = false;
	dirty(true);
}

DailyScheduleList::~DailyScheduleList()
{
    for (Iterator iter = begin(); iter != end(); iter++) {
        DailySchedule* schedule = *iter;
        delete schedule;
    }
}

void DailyScheduleList::clear()
{
    for (Iterator iter = begin(); iter != end(); iter++) {
        DailySchedule* schedule = *iter;
        if (schedule) {
            schedule->clear();
        }
    }
    dirty(true);
}

void DailyScheduleList::load(const char* str)
{
    mScheduleList.resize(getDailyScheduleTypeCount(), NULL);
    
    Json::Reader reader;
    Json::Value value;
    
    if (!reader.parse(str, value) || !value.isArray()) {
        return;
    }
    
    try {
        for (int i = 0; i < value.size(); i++) {
            string type = value[i]["type"].asString();
            int progress = value[i]["progress"].asInt();
            bool isCompleted = (bool)value[i]["iscompleted"].asInt();
            
            int scheduleType = getDailyScheduleTypeByName(type.c_str());
            DailyScheduleCfgDef* cfgDef = DailyScheduleCfg::getCfgByType(scheduleType);
            if (cfgDef == NULL) {
                continue;
            }
            DailySchedule* schedule = DailySchedule::create(cfgDef);
            if (schedule) {
                schedule->setProgress(isCompleted, progress);
                mScheduleList[schedule->mType] = schedule;
            }
        }
    } catch(...) {
        
    }

}

string DailyScheduleList::str()
{
    Json::Value value;
    for (Iterator iter = begin(); iter != end(); iter++) {
        DailySchedule* schedule = *iter;
        if (schedule == NULL) {
            continue;
        }
        const char* typeStr = getNameByDailyScheduleType(schedule->mType);
        Json::Value tmp;
        tmp["type"] = typeStr;//schedule->mId;
        tmp["progress"] = schedule->mProgress;
        tmp["iscompleted"] = (int)schedule->mIsCompleted;
        value.append(tmp);
    }
    return Json::FastWriter().write(value);
}

DailyScheduleList::Iterator DailyScheduleList::begin()
{
    return mScheduleList.begin();
}

DailyScheduleList::Iterator DailyScheduleList::end()
{
    return mScheduleList.end();
}

DailySchedule* DailyScheduleList::check(int type, int target, int progress)
{
    DailySchedule* schedule = NULL;
    Iterator iter;
    for (iter = begin(); iter != end(); iter++) {
        schedule = *iter;
        if (schedule == NULL) {
            continue;
        }
        
        if (schedule->mType == type) {
            schedule = *iter;
            break;
        }
    }
    
    if (iter == end()) {
        DailyScheduleCfgDef* cfgDef = DailyScheduleCfg::getCfgByType(type);
        if (cfgDef != NULL) {
            schedule = DailySchedule::create(cfgDef);
            if (schedule) {
                schedule->setProgress(false, 0);
                mScheduleList[schedule->mType] = schedule;
                dirty(true);
            }
        }
    }
    
    if (schedule == NULL || schedule->mIsCompleted) {
        return NULL;
    }
    
    if (schedule->check(target, progress)) {
        dirty(true);
    }

    return schedule;
}

/*
 award1 award2 award3 award4
 index1 index2 index3 index4
 
 根据等级award可能会变化，但是每个index只能领取一次
 */
bool DailyScheduleAwardState::getHaveGotAward(int awardIndex)
{
    if (mGettedAwardIndices.find(awardIndex) != mGettedAwardIndices.end()) {
        return true;
    }
    
    return false;
}

void DailyScheduleAwardState::setHaveGotAward(int awardId, int awardIndex)
{
    mGettedAward.insert(awardId);
    mGettedAwardIndices.insert(awardIndex);
    dirty(true);
}

string DailyScheduleAwardState::str()
{
    string ret;
    for (std::set<int>::iterator iter = mGettedAward.begin();
         iter != mGettedAward.end(); iter++) {
        string awardStr = strFormat("%d;", *iter);
        ret.append(awardStr);
    }
    return ret;
}

void DailyScheduleAwardState::load(const char* str)
{
    vector<string> awardIds = StrSpilt(str, ";");
    for (int i = 0; i < awardIds.size(); i++) {
        int awardId = Utils::safe_atoi(awardIds[i].c_str());
        mGettedAward.insert(awardId);
    
        DailyScheduleAwardCfgDef* cfgDef = DailyScheduleCfg::getAwardCfg(awardId);
        mGettedAwardIndices.insert(cfgDef->mIndex);
    }
}

void DailyScheduleAwardState::clear()
{
    mGettedAward.clear();
    mGettedAwardIndices.clear();
    dirty(true);
}

void WeaponEnchantDataList::load(const char* str)
{
    xyassert(str);
    mEnchantDataList.clear();
    
    Json::Reader reader;
    Json::Value value;
    if (!reader.parse(str, value)) {
        return;
    }
    
    try {
        for (int i = 0; i < value.size(); ++i) {
            WeaponEnchantData wed;
            wed.mEnchantId = value[i]["enchantid"].asInt();
            wed.mEnchantLvl = value[i]["enchantlvl"].asInt();
            wed.mEnchantExp = value[i]["enchantexp"].asInt();
            mEnchantDataList.insert((make_pair(wed.mEnchantId, wed)));
        }
    } catch (...) {
        
    }
}

string WeaponEnchantDataList::str()
{
    Iterator iter;
    Json::Value value;
    for (iter = mEnchantDataList.begin(); iter != mEnchantDataList.end(); ++iter) {
        Json::Value tmp;
        tmp["enchantid"] = (iter->second).mEnchantId;
        tmp["enchantlvl"] = (iter->second).mEnchantLvl;
        tmp["enchantexp"] = (iter->second).mEnchantExp;
        value.append(tmp);
    }
    return Json::FastWriter().write(value);
}

void WeaponEnchantDataList::clear()
{
    dirty(true);
    mEnchantDataList.clear();
}

void RoleAccumulateRecord::load(const char* str)
{
    xyassert(str);
    mRecord.clear();
    
	Json::Reader reader;
    Json::Value value;
    
    if (!reader.parse(str, value)) {
        return;
    }
    
	try
	{
		for (int i = 0; i < value.size(); i++) {
            mRecord.insert(make_pair(value[i]["Time"].asInt(), value[i][getName()].asInt()));
        }
	}
	catch(...)
	{
	}
}

string RoleAccumulateRecord::str()
{
    Iterator it;
	Json::Value value;
    for (it = mRecord.begin(); it != mRecord.end(); it++) {
		Json::Value tmp;
		tmp["Time"] = it->first;
		tmp[getName()] = it->second;
        
		value.append(tmp);
    }
    
    Json::Value tmp;
    tmp["Sum"] = getRecord();
    value.append(tmp);
    
    return xyJsonWrite(value);
}

void RoleAccumulateRecord::insertRecord(int rmb)
{
    string dbname = getName();
    if (dbname.empty()) {
        return;
    }
    
    ServerGroupCfgDef* def = NULL;
    if (dbname == "rmbconsumedrecord") {
        def= g_ConsumeRankListGroupCfg.getServerGroupCfgDefByServerid(Process::env.getInt("server_id"));
    } else if (dbname == "rechargerecord") {
        def = g_RechargeRankListGroupCfg.getServerGroupCfgDefByServerid(Process::env.getInt("server_id"));
    }

    if (def == NULL) {
        return;
    }
    
    int currentTime = time(NULL);
    int beginTime = def->mBeginTime;
    int endTime   = def->mEndTime;
    if (currentTime < beginTime || endTime < currentTime) {
        return;
    }
    
    mRecord.insert(make_pair(currentTime, rmb));
    dirty(true);
}

int RoleAccumulateRecord::getRecord()
{
    int nRecord = 0;
    Iterator it;
    for (it = mRecord.begin(); it != mRecord.end(); it ++) {
        nRecord += it->second;
    }
    return nRecord;
}

//暂时记录七天内的消费 以后 再改一下 支持配置
void RoleAccumulateRecord::refresh()
{
    if (mRecord.empty()) {
        return;
    }
    
    string dbname = getName();
    if (dbname.empty()) {
        return;
    }
    
    ServerGroupCfgDef* def = NULL;
    if (dbname == "rmbconsumedrecord") {
        def= g_RechargeRankListGroupCfg.getServerGroupCfgDefByServerid(Process::env.getInt("server_id"));
    } else if (dbname == "rechargerecord") {
        def = g_RechargeRankListGroupCfg.getServerGroupCfgDefByServerid(Process::env.getInt("server_id"));
    }
    
    if (def == NULL) {
        return;
    }
    
    int beginTime = def->mBeginTime;
    int endTime   = def->mEndTime;
    
    Iterator it;
    map tmp;
    for (it = mRecord.begin(); it != mRecord.end(); it ++) {
        if (beginTime <= it->first && it->first <= endTime) {
            tmp.insert(make_pair(it->first, it->second));
        }
    }

    mRecord = tmp;
    
    dirty(true);
}

void
RoleCsPvpStateData::refresh() {
    mUsedFreeTimes = 0;
    mRmbTimes = 0;
    dirty(true);
}


void
RoleCsPvpStateData::load(const char* str) {
    Json::Value value;
    Json::Reader reader;
    reader.parse(str, value);
    
    if (!value.isObject()) {
        mRmbTimes = 0;
        mUsedFreeTimes = 0;
        mRmbBuyTimes = 0;
        mBattleColddown = 0;
    } else {
        mUsedFreeTimes = value["used_free_times"].asInt();
        mRmbTimes = value["rmb_times"].asInt();
        mRmbBuyTimes = value["rmb_buy_times"].asInt();
        mBattleColddown = value["battle_colddown"].asInt();
    }
}

string
RoleCsPvpStateData::str() {
    Json::Value value;
    value["used_free_times"] = mUsedFreeTimes;
    value["rmb_times"] = mRmbTimes;
    value["rmb_buy_times"] = mRmbBuyTimes;
    value["battle_colddown"] = mBattleColddown;
    return Json::FastWriter().write(value);
}

void
RoleCsPvpStateData::consumeTime() {
    if (mRmbTimes > 0) {
        mRmbTimes--;
    } else {
        mUsedFreeTimes++;
    }
    dirty(true);
}

void
RoleCsPvpStateData::buyTime(int times) {
    mRmbBuyTimes += times;
    mRmbTimes += times;
    dirty(true);
}

void
RoleCsPvpStateData::setBattleColddown(time_t colddown) {
    mBattleColddown = colddown;
    dirty(true);
}

void
RoleWorshipData::load(const char* str) {
    Json::Value value;
    Json::Reader reader;
    reader.parse(str, value);
    
    if (!value.isObject()) {
        mUsedCommonTimes = 0;
        mUsedSpecialTimes = 0;
    } else {
        mUsedCommonTimes = value["used_common_times"].asInt();
        mUsedSpecialTimes = value["used_special_times"].asInt();
    }
}

string
RoleWorshipData::str() {
    Json::Value value;
    value["used_special_times"] = mUsedSpecialTimes;
    value["used_common_times"] = mUsedCommonTimes;
    return Json::FastWriter().write(value);
}

void
RoleWorshipData::refresh() {
    mUsedCommonTimes = 0;
    mUsedSpecialTimes = 0;
    dirty(true);
}

void
RoleWorshipData::addUsedCommonTimes() {
    mUsedCommonTimes++;
    dirty(true);
}

void
RoleWorshipData::addUsedSpecialTimes() {
    mUsedSpecialTimes++;
    dirty(true);
}
