//
//  Retinue.h
//  GameSrv
//
//  Created by pireszhi on 15-6-19.
//
//
#pragma once

#include "redis_helper.h"
#include "RedisHash.h"
#include "Defines.h"
#include "cyclemsg.h"

class Role;
class RetinueBag;

/**
 * 侍魂技能
 **/

struct RetinueSkillInfo
{
    RetinueSkillInfo(int skillId, int lvl):retinueSkillId(skillId),
                                           lvl(lvl)
    {
        
    }
    int retinueSkillId;
    int lvl;
};

class RetinueSkillMgr
{
public:
    static RetinueSkillMgr* Create(Role* role);
public:
    RetinueSkillMgr();
    ~RetinueSkillMgr();
    void load();
    
    void saveSkills();
    void saveEquipedSkills();
    
    bool upgSkill(int skillId, int incrLvl);
    bool equipSkill(int skillId, int index);
    bool unEquipSkill(int index);
    vector<int> getEquipedSkills();
    RetinueSkillInfo* getSkillInfo(int skillId);
    
    void clientGetSkillsInfo(vector<obj_retinue_skillInfo>& data);
    void clientGetEquipedSkills(vector<obj_retinue_skillInfo>& data);
    
    RetinueSkillInfo* addNewSkill(int skillId);
private:
    
private:
    vector<RetinueSkillInfo*> mRetinueSkills;         //已拥有技能
    vector<RetinueSkillInfo*> mRetinueEquipedSkills;  //已装备技能
    Role* mMaster;
};

/**
 * 侍魂通灵
 **/
struct SpiritSpeak
{
    SpiritSpeak(int gribCount):spiritId(0)
    {
        if(gribCount)
        {
            retinues.resize(gribCount, 0);
        }
    }
    
    int spiritId;
    int petid;
    
    string retinueList2String()
    {
        ostringstream list;
        for(int i = 0; i < retinues.size(); i++)
        {
            list << retinues[i] << ";";
        }
        
        return list.str();
    }
    vector<int> retinues;
    
    BattleProp mBattleProp;
};

class SpiritSpeakMgr
{
public:
    static SpiritSpeakMgr* Create(Role* role);
public:
    SpiritSpeakMgr():mMaster(NULL)
    {
        mSpiritSpeak.clear();
    }
    void load();
    void save();
    bool setSpiritSpeak(int spiritId, int position, int petId, int retinueId);
    void removeSpiritSpeakPet(int spiritId);
    
    void calcProp(BattleProp& battleProp);
    
    void clientGetSpiritSpeakInfo(vector<obj_spiritspeak_info>& data);
    
    void clientGetSingleSpiritSpeakInfo(int spiritspeakId, obj_spiritspeak_info& obj);
private:
    void calcSingleSpiritProp(SpiritSpeak& spirit);
    
    void onClientGetSingleSpiritSpeakInfo(SpiritSpeak& spirit, obj_spiritspeak_info& obj);
private:
    Role* mMaster;
    vector<SpiritSpeak> mSpiritSpeak;
};

/**
 * 侍魂本体
 **/
class Retinue : public RetinueData
{
public:
    
    Retinue(int retinueId);
    
    bool load(redisContext* conn);
    
    void calcProp();
    
    int getId();
    
    void getBattleProp(BattleProp& battleProp);
    
    int getPropertyIntension() { return propertyIntension; }
    
    void addExp(int exp);
    
    bool starUp();
    
    int getNextLvlExp(){ return mNextLvlupExp;}
    
    int getNextStarupExp(){return mNextStarUpExp;}
    
    void getRetinueInfo(obj_retinue_info& data);
    
    void getBattleProp(obj_retinueBattleProp &battleProp);
    
    int getSpiritSpeakId(){return mSpiritSpeakId;}
    
    void setSpiritSpeakId(int spiritspeakId){mSpiritSpeakId = spiritspeakId;}
    
    string retinueInfo2String();
    
private:
    
    bool lvlUp();
    
private:
    int mRetinueId;
    
    int propertyIntension; //侍魂属性强度

    int mNextLvlupExp;
    
    int mNextStarUpExp;
    
    int mSpiritSpeakId;
    
    int mPropStrength;
    
    BattleProp mBattleProp;
};

/**
 * 侍魂管理器，包括技能的管理，通灵的管理
 **/
class RetinueMgr
{
public:
    static const char* sGenIdKey;
    
    static int genRetinueId();
    
    static RetinueMgr* create(Role* role);
    
    static Retinue* Create(int modId);
    
    static Retinue* loadRetinue(int retinueId);
    
    void clientGetRetinueList(vector<obj_retinue_info>& data);
    
    READWRITE(Retinue*, mActiveRetinue, ActiveRetinue)
   
public:
    
    RetinueMgr();
    
    ~RetinueMgr();
    
    void addRetinue(Retinue* retinue);
    
    void save();
    
    Retinue* getRetinue(int retinueId);
    
    void loadAllRetinue();
    
public:
    RetinueBag* mRetinueBag;
    RetinueSkillMgr* mSkillMgr;
    SpiritSpeakMgr* mSpiritSpeakMgr;
private:
    vector<Retinue*> mRetinues;
    Role* mMaster;
   
    
};
class RetinueModDef;
extern void getRetinueInfoFromLua(RetinueModDef* cfg, int lvl, int star, int& nextLvlExp, int& nextStarExp, BattleProp& battleProp);

extern void sendRetinueNotify(Role* role, Retinue* retinue);

