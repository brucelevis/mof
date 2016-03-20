#include "WorldMonsterAI.h"
#include "WorldMonster.h"
#include "WorldGeometry.h"
#include "msg.h"

enum MonsterAIState
{
    kMonsterAINone,
    kMonsterAIPatrol,
    kMonsterAIBattle,
};


WorldMonsterAI::WorldMonsterAI()
{
    mState = kMonsterAINone;
}

void WorldMonsterAI::update(uint64_t ms)
{
    mUpdateCounter++;
    
    WorldMonster* monster = dynamic_cast<WorldMonster*>(mCreature);
    if (monster->isDead()) {
        return;
    }
 
    // added by jianghan
    monster->DealMonsterAiState();
    // end add
    
    //百分之20的概率什么都不做
    int rolNothing = rand() % 100;
    if (rolNothing < monster->getHangAround()) {
        monster->MonsterHangAround();
        return;
    }
    
    WorldScene* scene = monster->getScene();
    
    vector<WorldCreature*> objs;
    objs.clear();
    objs = scene->getAreaCanAttackObjs(monster, monster->getType());
    
    WorldCreature* target = monster->getNearestObj(objs);
    if(target == NULL) {
        monster->MonsterPatrol();
        return;
    }
    
    //寻找攻击目标
    //判断是否在视野范围，如果是，再判断是否追击
    CCPoint targetPos = target->getPosition();
    if(ccpDistance(monster->getPosition(), targetPos) >= monster->getVision()) {
        monster->MonsterPatrol();
        return;
    }
    
    //释放技能
    if (monster->MonsterCastSkill(target)) {
        return;
    }
    
    if (monster->MonsterCanAtk(target)) {
        
        bool canAtk = false;
        
        int rol = rand() % 100;
        int atkPeriod = (int)(monster->getAtkPeriod() / monster->getActionPeriod()) + 1;
        if (mUpdateCounter % atkPeriod == 0) {
            if (rol < monster->getAtkProbality()) {
                canAtk = true;
            }
        }
        
        if (canAtk) {
            monster->FixAtkOrient(target);
            monster->attack(ms, 1);
            return;
        }
    }
    
    //有技能的话 就跑到技能能能攻击的范围
    if (monster->MonsterTrack(target, monster->getMonsterType())) {
        return;
    }
    
    if (monster->MonsterEscape(target)) {
        return;
    }
    
    if ((rand() % 100) < 50) {
        monster->MonsterHangAround();
    }
}
