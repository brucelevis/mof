/******************************************************************************
 Redish Hash 自定义类型要求
    1、继承 RedisHashObject 并实现其纯虚函数
    2、修改值的函数要调用下面函数以回写数据库
        dirty(true);
 ******************************************************************************/
#pragma once
#include <map>
#include <string>
#include <vector>
#include <set>
#include "RedisHashBaseObj.h"
#include "daily_schedule/daily_schedule.h"
using namespace std;

class RedisHashObject : public RedisHashObjectBase<string>
{
public:
    RedisHashObject(){ mDirty = false;}
    virtual ~RedisHashObject(){}
    virtual void load(const char* str) = 0;
    virtual string str()  = 0;
};

class IntArrayObj : public RedisHashObject
{
public:
    typedef std::vector<int> Type;
    typedef Type::iterator iterator;
    typedef Type::const_iterator const_iterator;
    
    virtual void load(const char* str);
    virtual string str() ;
    void push_back(int val);
    void erase(int val);
    void clear();
    bool exist(int val) const;
    int size() const ;
    
    const_iterator const_begin(){ return mData.begin();}
    const_iterator const_end()  { return mData.end();}
        
    const Type& toArray() const { return mData; };
    IntArrayObj& operator = (const std::vector<int>& rhs);
    
protected:
    Type mData;
};

struct PetUpSkillProbData
{
    PetUpSkillProbData() {
        skillid = 0;
        upSkillFailTimes = 0;
        upFailTotalProb = 0.0;
        upSkillProb = 0.0;
    }
    
    int skillid;
    int upSkillFailTimes;
    float upFailTotalProb;
    float upSkillProb;
    
};

class PetUpSkillProb : public RedisHashObject
{
public:
    virtual void load(const char* str);
    virtual string str();
    
    typedef std::map<int, PetUpSkillProbData> UpSkillProbDataList;
    typedef UpSkillProbDataList::iterator Iterator;
    
    void add(int skillid, PetUpSkillProbData &data)
    {
        mUpSkillProbDataList[skillid] = data;
        dirty(true);
    }
    
    PetUpSkillProbData * find(int skillid)
    {
        Iterator it = mUpSkillProbDataList.find(skillid);
        if (it == mUpSkillProbDataList.end()) {
            return NULL;
        }
        return &(it->second);
    }
    
    void erase(int skillid)
    {
        mUpSkillProbDataList.erase(skillid);
        dirty(true);
    }
    
    UpSkillProbDataList copy() {return mUpSkillProbDataList;}
    
    Iterator begin() { return mUpSkillProbDataList.begin(); }
    Iterator end() { return mUpSkillProbDataList.end(); }
    
    void clear();
protected:
    UpSkillProbDataList mUpSkillProbDataList;
    
};

class RoleGuildSkill : public RedisHashObject   //角色公会技能
{
public:
    typedef map<int,int> SkillMap;
    typedef SkillMap::iterator Iterator;
    
    virtual void load(const char* str);
    virtual string str() ;
    void set(int id , int lvl);
    int get(int id);
    
    Iterator begin(){ return mSkill.begin();}
    Iterator end()  { return mSkill.end();}
	
	// 修正非法技能的逻辑处理　04-23 by jin
	void correctIllegalSkill();
protected:
    SkillMap    mSkill;
};

struct RoleHonorActionDef
{
	int key;
	string cond;
	string param;
	int time;
};

// 称号动作
class RoleHonorAction : public RedisHashObject
{
public:
	typedef std::vector<RoleHonorActionDef> ActionVec;
	typedef ActionVec::iterator Iterator;
	
	virtual void load(const char* str);
    virtual string str();
	
	void add(int key, string cond, string param);
	void clear();
	
	Iterator begind() {return mActionVec.begin();}
	Iterator end() {return mActionVec.end();}
	
protected:
	ActionVec mActionVec;
};

class RoleMidastouchAction : public RedisHashObject
{
public:
	typedef std::vector<int> ActionVec;
	typedef ActionVec::iterator Iterator;
	
	virtual void load(const char* str);
    virtual string str();
	
	Iterator begind() {return mActionVec.begin();}
	Iterator end() {return mActionVec.end();}

	void add(int rmb);
	void clear();
	
protected:
	ActionVec mActionVec;
};

class RoleConstellFailDef
{
public:
	int id;
	int times;
	float accuRate;
	
	RoleConstellFailDef() : id(0), times(0), accuRate(0.0)
	{
	}
	
	void clear()
	{
		times = 0;
		accuRate = 0.0;
	}
};

class RoleConstellFail : public RedisHashObject
{
public:
	typedef std::map<int, RoleConstellFailDef> ConstellFail;
	typedef ConstellFail::iterator Iterator;
	
	virtual void load(const char* str);
    virtual string str();
	
	RoleConstellFailDef & get(int key) {return mConstellFail[key];}
	
	Iterator begind() {return mConstellFail.begin();}
	Iterator end() {return mConstellFail.end();}
	
	void add(int key);
	void clear(int key);
	
	void setAccuRate(int key, float val = 0.0);

protected:
	ConstellFail mConstellFail;
};

struct RoleTwistEggDef
{
	int id;
	int free;
	int freeTime;
	int first;
	
	RoleTwistEggDef()
	{
		id = 0;
		free = 0;
		first = 0;
		freeTime = 0;
	}
	
	void addFree(int value) {free += value;}
	void setFirst() {first = 1;}
	void refresh() { free = 0;}
	bool isFirst() {return first == 0 ? true : false;}
	void setFreeTime(int tm) {freeTime = tm;}
};

class RoleTwistEgg : public RedisHashObject
{
public:
	typedef map<int, RoleTwistEggDef>::iterator Iterator;

	virtual void load(const char* str);
    virtual string str();
	
	RoleTwistEggDef& get(int key);
	
	void refresh();
	void save();
protected:
	map<int, RoleTwistEggDef> mData;
};

class RoleDungeonStateDef
{
public:
    int mIndex;
    int mSweepLevel;
    int mDungeonLevel;
    int mResetTimes;
    RoleDungeonStateDef()
    {
        mIndex = 0;
        mSweepLevel = 0;
        mDungeonLevel = 0;
        mResetTimes = 0;
    }
};

class RoleDungeonState : public RedisHashObject
{
public:
    
	typedef std::map<int, RoleDungeonStateDef> RoleDungeonStateDefMap;
	typedef RoleDungeonStateDefMap::iterator Iterator;
	
	virtual void load(const char* str);
    virtual string str();
    
    int getSweepLevel(int index);
    int getDungeonLevel(int index);
    int getResetTimes(int index);
    
	Iterator setSweepLevel(int index, int level);
	Iterator setDungeonLevel(int index, int level);
	Iterator setResetTimes(int index, int times);
	Iterator addResetTimes(int index, int times);

    void refresh();
    
	void clear();
	
	Iterator begin() {return mStates.begin();}
	Iterator end() {return mStates.end();}
    Iterator findOrInsert(int index)
    {
        Iterator iter = mStates.find(index);
        if (iter == mStates.end()) {
            RoleDungeonStateDef state;
            state.mIndex = index;
            iter = mStates.insert(make_pair(index, state)).first;
        }
        return iter;
    }
	
protected:
	RoleDungeonStateDefMap mStates;
};

struct WardrobeFashion
{
    int mId;
    int mType;
    int mExpireTime;
    bool mExpired;
};

class WardrobeFashionList : public RedisHashObject
{
    static const char* const IdKey;
    static const char* const ExpireTimeKey;
    static const char* const IsExpiredKey;
    
public:
    WardrobeFashionList() {}
    ~WardrobeFashionList();
    
	virtual void load(const char* str);
    virtual string str();
    
    
	typedef std::vector<WardrobeFashion*> FashionList;
	typedef FashionList::iterator Iterator;
    
    Iterator begin();
    Iterator end();
    WardrobeFashion* addFashion(int fashionId, int expireTime);
    void updateExpiration(WardrobeFashion* fashion, int expireTime);
    
protected:
    FashionList mFashionList;
};

class DailyScheduleList : public RedisHashObject
{
public:
    typedef std::vector<DailySchedule*> ScheduleList;
    typedef std::vector<DailySchedule*>::iterator Iterator;
public:
    DailyScheduleList() {}
    ~DailyScheduleList();
    
    virtual void load(const char* str);
    virtual string str();
    void clear();
    
    DailySchedule* check(int type, int target, int progress);
    
    Iterator begin();
    Iterator end();
    
protected:
    ScheduleList mScheduleList;
};

class DailyScheduleAwardState : public RedisHashObject
{
public:
    typedef std::set<int>::iterator Iterator;
    
public:
    bool getHaveGotAward(int awardIndex);
    void setHaveGotAward(int awardId, int awardIndex);
    virtual void load(const char* str);
    virtual string str();
    void clear();
    
    Iterator begin() {return mGettedAward.begin();}
    Iterator end() {return mGettedAward.end();}
    
protected:
    std::set<int> mGettedAward;
    std::set<int> mGettedAwardIndices;
};

struct WeaponEnchantData
{
	WeaponEnchantData(){
		mEnchantId = 0;
		mEnchantLvl = 0;
		mEnchantExp = 0;
	}
	
    int mEnchantId;
    int mEnchantLvl;
    int mEnchantExp;
};

class WeaponEnchantDataList : public RedisHashObject
{
public:

    virtual void load(const char* str);
    virtual string str();
    
    typedef std::map<int, WeaponEnchantData> EnchantDataList;
    typedef EnchantDataList::iterator Iterator;
    
    void add(int enchantid, WeaponEnchantData &data)
    {
        mEnchantDataList[enchantid] = data;
        dirty(true);
    }
    
    WeaponEnchantData * find(int _id)
    {
		Iterator it = mEnchantDataList.find(_id);
		if (it == mEnchantDataList.end()) {
			return NULL;
		}
		return &(it->second);
    }
	
	EnchantDataList copy() {return mEnchantDataList;}
    
    Iterator begin() {return mEnchantDataList.begin();}
    Iterator end() {return mEnchantDataList.end();}
    void clear();
    
protected:
    EnchantDataList mEnchantDataList;
};

class RoleAccumulateRecord : public RedisHashObject
{
public:
    
    typedef std::map<int, int>              map;
    typedef std::map<int, int>::iterator    Iterator;
    
	virtual void load(const char* str);
    virtual string str();
    
    void insertRecord(int rmb);
    int  getRecord();
    
    void refresh();
	
private:
    map mRecord;
    int mRecordType;
};

class RoleRmbConsumedRecord : public RoleAccumulateRecord
{};

class RoleRechargeRecord : public RoleAccumulateRecord
{};


class RoleCsPvpStateData : public RedisHashObject
{
public:
    virtual void load(const char* str);
    virtual string str();
    
    void refresh();
    
    void consumeTime();
    void buyTime(int times);
    void setBattleColddown(time_t colddown);
    
    int getUsedFreeTimes() {return mUsedFreeTimes;}
    int getRmbBuyTimes() {return mRmbBuyTimes;}
    int getRmbTimes() {return mRmbTimes;}
    int getBattleColddown() {return mBattleColddown;}
    
    int mUsedFreeTimes;
    int mRmbBuyTimes;
    int mRmbTimes;
    int mBattleColddown;
};

class RoleWorshipData : public RedisHashObject
{
public:
    
    virtual void load(const char* str);
    virtual string str();
    
    void refresh();
    
    void addUsedCommonTimes();
    void addUsedSpecialTimes();
    
    int getUsedCommonTimes() {return mUsedCommonTimes;}
    int getUsedSpecialTimes() {return mUsedSpecialTimes;}
    
    int mUsedCommonTimes;
    int mUsedSpecialTimes;
};
