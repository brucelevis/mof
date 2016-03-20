//
//  Skill.h
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//

#ifndef __GameSrv__Skill__
#define __GameSrv__Skill__

#include "Defines.h"
#include "DataCfg.h"

class Role;

class Obj;

//typedef void (*EFFECTFUNC)(Obj* owner, Obj* target, void* skill);   //效果函数生命格式
//void InsertEffectTable(map < int, EFFECTFUNC>& effectTable);    //用于初始化技能效果映射表的小函数

/*
 ----------------------------------------------
 */
//技能效果管理类
//class SkillEffect
//{
//public:
//    SkillEffect(){}
//    ~SkillEffect(){}
//public:
//    static bool InitEffectTable();
//    static EFFECTFUNC GetEffectFunc(SkillAffect effectType);
//private:
//    static map<int, EFFECTFUNC> m_effectFunc;
//};

/*
 -----------------------------------------------
 */
class Skill : public SkillCfg {
    READWRITE(Obj*, mOwner, Owner)
    READWRITE(int, mID, ID)
    READWRITE(float, mCDPos, CDPos)
    READWRITE(int, mSkillType, SkillType)
    
public:
    Skill(){mCDPos = 0;}
    Skill(Obj* obj,int _id);
    virtual ~ Skill(){}
    
    void config();
    bool CheckSkillCD();    //检查技能CD
    void SetSkillCD(float cd);
    int JudeStatBeforCast();    //施放技能前对象的状态判断
    int cast();
    int perform();
    
    SkillCfgDef* getCfg(bool& exist);
};
/*
 ------------------------------------------------
 */
class SkillMod {
    READWRITE(Obj*, mOwner, Owner)
    READWRITE(int, mCommCDPos, mCommCDPos)
    vector<Skill*> mStudySkills;
    vector<Skill*> mEquipedSkills;
    Skill* mUniqueSkill;
public:
    
    SkillMod(Obj* obj);
    
    virtual ~SkillMod();
    
    void loadSkillsFromDB(int roleid);
    
    //学习或升级技能
    bool learn(int _id);
    bool gm_learn(int _id);
    
    //新建一个技能
    Skill* createSkill(Obj* owner, int skillid);
    
    Skill* getSkillInSkillTable(int skillid);
    
    Skill* getEquipSkill(int position);
    
    //检查公共CD
    bool CheckComCD();  //检查公共Cd
    
    //设置公共cd
    void SetComCD(int cd);
    
    //释放技能
    int cast(int idx);
    
    //
    int perform(int idx){return 1;}
    
    //获取已经学习了的技能表
    vector<int> getStudySkillList();
    
    vector<int> getEquipSkillList();
    
    //判断该技能是否已学
    bool IsSkillLearn(int skillid);
    
    //计算所有技能能够给角色添加的属性，以前是用于一些验证客户端战斗的
    void skillsAddBattleProp(BattleProp &prop);
    
    //装备被动，计算任务属性时用到
    void equipPassiveSkill(BaseProp& baseProp, BattleProp& battleProp);
    
    //装备主动技能
    bool equipInitiativeSkill(int skillid, int position);
    
    //卸载主动技能
    bool unEquipInitiativeSkill(int position);
    
    bool unEquipInitiativeSkillBySkillId(int skillId);
    
    //判断是否能自动装备
    bool checkAutoEquip(int skillid);
    
    //
    bool chkUp();
    
    // +宠物skill
    int calcDmg(int skillID,int skillEffectIndex,bool isCriHurt,Obj*, BattleProp* firer);
    
    void saveEquipSkillList(int roleid);
    
    void saveStudySkillList();
    
    int getSkillLevel(int index);
    
    bool skillReplace(int preSkill, int newSkill);

private:
    bool deleteSkill(int skillId);
};

//获取玩家的
int getStudySkill(Role* role, int index);

//
bool SaveSkillTable(int roleid, vector<int>& skilllist);

//
void onSaveEquipSkillList(int roleid, vector<int>& skilllist);

//
bool checkSkillOwner(SkillOwner skillOwner, ObjJob playerJob);

int CalcSkillLvl(int skillId);
int getSkillIndex(int skillId);

#endif /* defined(__GameSrv__Skill__) */

