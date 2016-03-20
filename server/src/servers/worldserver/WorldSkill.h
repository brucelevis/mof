//
//  Skill.h
//  GoldOL
//
//  Created by Mike Chang on 12-10-23.
//  Copyright (c) 2012年 __MyCompanyName__. All rights reserved.
//

#ifndef GAMESRV_WORLD_Skill_h
#define GAMESRV_WORLD_Skill_h
#include "Defines.h"
#include "DataCfg.h"
#include "WorldCreature.h"

#define MAX_CRIT 1000
#define LivingObejct WorldCreature

enum SkillCastErrorCode
{
    eCastSKillOk = 0,
    eSkillInCdTime,
    eOutOfSkillRange,
    eSkillOtherError,
};

namespace world
{

class Skill
{
public:
    int skillid;
    SkillCfgDef* skillData;   
public:
    Skill(int skill_id);
};

struct EffectValue
{
    int effectval;
    float effectfloat;
    int lastTime;
};
typedef void (*EFFECTFUNC)(WorldCreature* owner, WorldCreature* target, int skillId, SkillEffectVal *effect );   //效果函数生命格式
    
    
}
//void KnockBack(LivingObject* mOwner, LivingObject* target, int effectType, struct SkillEffectVal &effect);
void InsertEffectTable(map < int, world::EFFECTFUNC>& effectTable);    //用于初始化技能效果映射表的小函数

//技能效果管理类
class SkillEffectMgr
{
public:
    SkillEffectMgr(){}
    ~SkillEffectMgr(){}
public:
    static bool InitEffectTable();
    static world::EFFECTFUNC GetEffectFunc(SkillAffect effectType);
private:
    static map<int, world::EFFECTFUNC> m_effectFunc;
};

bool IsHited(WorldCreature* mOwner, WorldCreature* target);
HitType GetSkillDamageValue(WorldCreature* mOwner, WorldCreature* target, SkillEffectVal *effect, int& damage);

//计算某技能的合理施法范围，目前只为战斗AI服务
CCRect CalSkillCastRect(WorldCreature* attackObj, SkillCfgDef* skilldata, ObjOrientation atkOrient);
    
#endif
