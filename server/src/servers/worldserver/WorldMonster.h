//
//  WorldMonster.h
//  GameSrv
//
//  Created by xinyou on 14-7-15.
//
//

#ifndef __GameSrv__WorldMonster__
#define __GameSrv__WorldMonster__

#include "WorldCreature.h"

class WorldCreature;


struct PatrolData
{
    CCPoint mOriginPoint;
    CCPoint mLeftPoint;
    CCPoint mRightPoint;
    float mPatrolScale;
};
enum MonsterState
{
    eNonState,
    ePatrol,
    eAtck,
    eEscape,
    eTrack,
    eHangAround,
};



class WorldMonster : public WorldCreature
{
    READWRITE(PatrolData, mPatrolData, PatrolData)
    READWRITE(float, mAtkPeriod, AtkPeriod)
    READWRITE(float, mAtkProbality, AtkProbality)
    READWRITE(float, mPatrolLength, PatrolLength)
    READWRITE(float, mVision, Vision)
    READWRITE(int, mFollow, Follow)
    READWRITE(float, mChangRoundX, ChangRoundX)
    READWRITE(float, mChangRoundY, ChangRoundY)
    READWRITE(int, mCastSkillRol, CastSkillRol)
    READWRITE(int, mComCd, ComCd)
    READWRITE(int, mPerAnger, PerAnger) //怒气
    READWRITE(MonsterType, mMonsterType, MonsterType)
    READWRITE(CCRect, mSkillRect, SkillRect)
    
    READONLY(LivingObject*, mTarget, Target) //怪物跟踪或追逐的目标
    READWRITE(int, mStand, Stand)
    READWRITE(bool, mCanAtk, CanAtk)
    READWRITE(float, mActionPeriod, ActionPeriod)
    READWRITE(int, mFrequency, Frequency); //回血的频率
    
    READWRITE(int, mHangAround, HangAround)
    
private:
    MonsterState mMonsterState;
    int mEscape;
protected:
    virtual ~WorldMonster(void);
    //virtual void update(float dt);
    
#if 0
    void animationHandler(BoneAniEventType evtType, std::string aniName, std::string frameID, bool isLoop);
#endif
    bool IsSkillCoolingDown(int skillid);
    
    bool mIsStartedAI;
public:
    std::vector<MonsterSkills*> mMonsterStudyableSkills;
    std::map<int, int> mCoolDownTable;  //怪物技能冷却表
    WorldMonster();
    
    // added by jianghan for 为WorldPet增加这种构造方式
    WorldMonster(WorldObjectType);
    // end add
    
    bool InitPatrol();
    void FixAtkOrient(LivingObject* target);
    
    virtual void death(LivingObject* src);
    virtual void damage(int dmg, LivingObject* src);
    
    bool MonsterInit(int tplId);
    //初始化基本属性
    void InitBasicAttributes(MonsterCfgDef* monsterCfg);
    //初始其它高级属性
    void InitOtherAttributes(MonsterCfgDef* monsterCfg);
    
    // added by jianghan
    bool TargetInSight( WorldCreature * pTarget );
    // end add
    
    void DealMonsterAiState();
    void MonsterPatrol();
    void MonsterHangAround();
    int MonsterEscape(LivingObject* target);
    void MonsterLearnSKills(vector<MonsterSkills*> skills);
    void MonsterClearSkillTable();
    
    int MonsterCastSkill(LivingObject* target);
    int MonsterSetSkillCoolDown(int skillid);
    int MonsterTrack(LivingObject* target, MonsterType type = eShorRange);
    int MonsterDealWithCoolDown();
    int MonsterCanAtk(LivingObject* target);
    int MonsterFallow(LivingObject* target); //for pet in town
    int MonsterStand(float second);
    bool IsMonsterStanding();
    
#if 0
    void starIncrHP();
    void refreshMonsterHP(float dt);
    virtual void afterRefreshMonsterHPCallBack(int addHP = 0);
    void deleterefreshHpSchedule();
#endif
    
    //int GetNbeat(){return nbeat;}
    //    int ShortRangeMonster();
    //    int LongRangeMonster();
    //    int BossMonster();
    
    CCRect CalSkillRect(int width, int height);
    CCRect CalSkillRect();
    friend class LivingObject;
    
public:
    static WorldMonster* create(int tplid);
    static void destroy(WorldMonster* monster);
};

#endif /* defined(__GameSrv__WorldMonster__) */
