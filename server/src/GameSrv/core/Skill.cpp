//
//  Skill.cpp
//  GameSrv
//
//  Created by 麦_Mike on 12-12-28.
//
//
#include <sys/time.h>
#include <time.h>
#include "Skill.h"
#include "Obj.h"
#include "Scene.h"
#include "Role.h"

int getStudySkill(Role* role, int index)
{
    int job = role->getJob();
    //int lvl = role->getLvl();
    int skillid = 400000 + job*10000 + 100*index + 0;
    return skillid;
}

bool SaveSkillTable(int roleid, vector<int>& skilllist)
{
    redisReply* rreply;
    string skillTable = "";
    char temp[10] = {0};
    for (int i = 0; i < skilllist.size(); i++) {
        sprintf(temp, "%d", skilllist[i]);
        skillTable += temp;
        skillTable += ":";
    }
    
    rreply=redisCmd("hmset role:%d skilltable %s",roleid, skillTable.c_str());
    freeReplyObject(rreply);
    
    return true;
}

void onSaveEquipSkillList(int roleid, vector<int>& skilllist)
{
    string skilltable = "";
    for (int i = 0; i < skilllist.size(); i++) {
        string skillStr = Utils::makeStr("%d;", skilllist[i]);
        skilltable.append(skillStr);
    }
    
    doRedisCmd("hset role:%d equipskills %s", roleid, skilltable.c_str());
}

bool checkSkillOwner(SkillOwner skillOwner, ObjJob playerJob)
{
    switch (skillOwner) {
        case eOwnerAssassin:
        {
            if (playerJob == eAssassin) {
                return true;
            }
            break;
        }
        case eOwnerMage:
        {
            if (playerJob == eMage) {
                return true;
            }
            break;
        }
        case eOwnerWarrior:
        {
            if (playerJob == eWarrior) {
                return true;
            }
            break;
        }
        default:
            break;
    }
    return false;
}

int CalcSkillLvl(int skillId)
{
    return skillId%100;
}

int getSkillIndex(int skillId)
{
    return (skillId/100) * 100;
}


Skill::Skill(Obj* obj,int _id)
:mOwner(obj)
,mID(_id)
{
    config();
    mCDPos = 0;
}

SkillCfgDef* Skill::getCfg(bool& exist){
    return SkillCfg::getCfg(mID,exist);
}

void Skill::config(){
   // bool exist;
   // SkillCfgDef& def = SkillCfg::getCfg(mID,exist);
    //setAffectVal(def.affectValue);
}

int Skill::cast(){
    //chk cd
    bool isSucc;
    SkillCfgDef* skillData = SkillCfg::getCfg(this->getID(), isSucc);
    bool isCD = this->CheckSkillCD();
    
    if( !isCD )
    {
        perform();
        this->SetSkillCD(skillData->skillCD);
    }
    return 1;
}

bool Skill::CheckSkillCD()
{
    int lastTime = this->getCDPos();
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int timeNow = (tv.tv_sec%1000000)*1000 + tv.tv_usec/1000;
    cout<<"timeCheck:  "<<lastTime<<endl;;
    if (timeNow >= lastTime) {
        /*
         
         
         */
        return false;
    }
    return true;
}
void Skill::SetSkillCD(float cd)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int timeNow = (tv.tv_sec%1000000)*1000 + tv.tv_usec/1000;
    //int timeNow = tv.tv_sec*1000 + tv.tv_usec/1000;
    this->setCDPos(timeNow + cd);
    cout<<"casttime:  "<<timeNow<<endl;;
}
int Skill::JudeStatBeforCast()
{
    Obj* owner = this->getOwner();
    
    ObjType objType = owner->getType();
    int state = owner->getState();
    
    if (objType == eMonster) 
    {
        /*
         
         */
        return 0;
    }
    if (state) {
        /*
         
         
         */
        return 1;
    }
    return -1;
}
int Skill::perform(){
//    vector<Obj*> objs = mOwner->getScene()->areaObjs(mOwner, 100);
//    vector<Obj*>::iterator it = objs.begin();
//    for (; it!=objs.end(); it++) {
//        Obj* obj = (Obj*)(*it);
//        obj->beAtk(mOwner,this);
//    }
//    bool isSucc;
//    SkillCfgDef skillData = SkillCfg::getCfg(this->getID(), isSucc);
//    SkillAffect effectType = skillData.affectType;
//    EFFECTFUNC  func = SkillEffect::GetEffectFunc(effectType);
//    if (func==NULL) 
//    {
//        return -1; 
//    }
//    else
//    {
//        func(mOwner, NULL, (void*)this);
//    }

    return 1;
}

/*
 -----------------------------------------------------
 */
SkillMod::SkillMod(Obj* obj)
:mOwner(obj){
    mStudySkills.resize(MAX_SKILL_COUNT, NULL);
    mEquipedSkills.resize(SKILL_EQUIT_NUM, NULL);
}

SkillMod::~SkillMod()
{
    for (int i = 0 ; i < mStudySkills.size(); i++) {
        if (mStudySkills[i] != NULL) {
            Skill* skill = mStudySkills[i];
            delete skill;
        }
    }
    
    mEquipedSkills.clear();
}

//计算一般技能的伤害值，包含暴击的计算
HitType GetSkillDamageValue(BattleProp* owner, Obj* target, SkillEffectVal *effect, int& damage)
{
    //bool isCrit;
    float rol1 = rand()%100;
    int dmg = owner->getAtk() * ( 0.95f + 0.1 *(rol1/100));
    int skillAttack = dmg * ( 1 + effect->getAffectFloat()) + effect->getAffectValue();
    
    float DefPro = target->BattleProp::getDef() / (float)(target->BattleProp::getDef() + target->getLvl()*60 + 500);
    
    damage = skillAttack*(1.0f - DefPro);
    damage = damage > 0 ? damage : 1;
    
    int rol = rand()%1000;
    if (owner->getCri()*10 > rol) {
        damage = damage * 1.5f;
        return eCri;
    }
    return eHit;
}

//计算减血buff，如中毒，的伤害，不计算暴击
int CalDecrHpBuffDamage(BattleProp* owner, Obj* target, SkillEffectVal *effect)
{
    int skillAttack = owner->getAtk() * ( 1 + effect->getAffectFloat()) + effect->getAffectValue();
    float DefPro = target->BattleProp::getDef() / (float)(target->BattleProp::getDef() + target->getLvl()*60 + 500);
    int damage;
    damage = skillAttack*(1.0f - DefPro);
    damage = damage > 0 ? damage : 1;
    
    return damage;
}

int SkillMod::calcDmg(int skillID,int skillEffectIndex,bool isCriHurt,Obj* target, BattleProp* firer)
{    
    bool exist; int dmg;
    SkillCfgDef* def = SkillCfg::getCfg(skillID,exist);
	if (skillEffectIndex >= def->effectList.size() || skillEffectIndex < 0) {
		return 0;
	}
    SkillEffectVal* effval = def->effectList[skillEffectIndex];
    bool noCri = (effval->mSkillEffectId == eHurtPoisoned);
    if(noCri)
        dmg = CalDecrHpBuffDamage(firer,target,effval);
    else
        GetSkillDamageValue(firer,target,effval,dmg);
    
    return dmg;
}


int SkillMod::cast(int idx){

    if (idx < mEquipedSkills.size() && NULL != mEquipedSkills[idx]) {
        mEquipedSkills[idx]->cast();
        return 1;
    }
    
    return 0;
}

Skill* SkillMod::createSkill(Obj *owner, int skillid)
{
    SkillCfgDef* def = SkillCfg::getCfg(skillid);
    if (def == NULL) {
        return NULL;
    }

    Skill* newSkill = new Skill(mOwner, skillid);
    newSkill->mSkillType = def->skillType;
    
    if (newSkill->mSkillType == eAnger) {
        mUniqueSkill = newSkill;
    }
    
    return newSkill;
}

bool SkillMod::learn(int _id){

    SkillCfgDef* def = SkillCfg::getCfg(_id);
    if (def == NULL) {
        return false;
    }
    
    for(int i=0;i < mStudySkills.size();i++) {
        
        if( mStudySkills[i] != NULL ){
            
            int lastskill = mStudySkills[i]->getID();
            
            //对学过的技能升级
            if (getSkillIndex(lastskill) == getSkillIndex(_id) ) {
                mStudySkills[i]->mID = _id;
                return true;
            }
        }
        else    //没有学过的技能
        {
            mStudySkills[i] = createSkill(mOwner, _id);
            return true;
        }
    }
    return false;
}

bool SkillMod::gm_learn(int _id){
    
    SkillCfgDef* def = SkillCfg::getCfg(_id);
    if (def == NULL) {
        return false;
    }
    
    for(int i=0;i < mStudySkills.size();i++) {
        
        if( mStudySkills[i] != NULL ){
            
            int lastskill = mStudySkills[i]->getID();
            
            //对学过的技能升级
            if (getSkillIndex(lastskill) == getSkillIndex(_id)) {
                
//                //新技能号比旧技能号等级必须大于一
//                if ( (_id - lastskill ) != 1) {
//                    return false;
//                }
                
                mStudySkills[i]->mID = _id;
                return true;
            }
        }
        else    //没有学过的技能
        {
            mStudySkills[i] = createSkill(mOwner, _id);
            return true;
        }
    }
    return false;
}

void SkillMod::loadSkillsFromDB(int roleid)
{
    //get skilltable
    RedisResult result(redisCmd("hmget role:%d skilltable equipskills",roleid));
    
    string skilltable;
    string equipskills;
    
    skilltable = result.readHash(0, "");
    equipskills = result.readHash(1, "");
    
    StringTokenizer token(skilltable,":");
    for (StringTokenizer::Iterator iter = token.begin(); iter != token.end(); ++iter)
    {
        int skillid = Utils::safe_atoi((*iter).c_str());
        if( !learn(skillid))
        {
            string errorInfo = Utils::makeStr("player:%d load skilltable from DB error", roleid);
            log_error(errorInfo.c_str());
        }
    }
    
    //load equipSkills
    if (equipskills.empty()) {
        for (int i = 0; i < mStudySkills.size(); i++) {
            if (mStudySkills[i] != NULL) {
                checkAutoEquip(mStudySkills[i]->getID());
            }
        }
        saveEquipSkillList(roleid);
    }
    else
    {
        bool isChange = false;
        
        StringTokenizer equipToken(equipskills, ";");
        for (int i = 0; i < equipToken.count() && i < mEquipedSkills.size(); i++) {
            
            int skillid = Utils::safe_atoi(equipToken[i].c_str());
            Skill* skill = getSkillInSkillTable(skillid);
            if (!skill) {
				continue;
			}
            if (skill != NULL && skill->getSkillType() == eInitiative) {
                mEquipedSkills[i] = skill;
            }
            else{
                isChange = true;
                continue;
            }
        }
        
        if (isChange) {
            saveEquipSkillList(roleid);
        }
    }
}

Skill* SkillMod::getSkillInSkillTable(int skillid)
{
    for (int i = 0; i < mStudySkills.size(); i++) {
        if (mStudySkills[i] != NULL && mStudySkills[i]->getID() == skillid) {
            return mStudySkills[i];
        }
    }
    return NULL;
}

Skill* SkillMod::getEquipSkill(int position)
{
    if (position < mEquipedSkills.size()) {
        return mEquipedSkills[position];
    }
    return NULL;
}

bool SkillMod::chkUp(){
    SkillOwner type = mOwner->toSkillOwner();
    vector<int> ret = Skill::roleTotal(type);
    vector<int>::iterator it=ret.begin();
    for (; it!=ret.end(); it++) {
        learn(*it);
    }
    
    return false;
}
bool SkillMod::CheckComCD()
{
    //int ComCd = this->getmCommCDPos();
    return true;
}
void SkillMod::SetComCD(int cd)
{
    
}

vector<int> SkillMod::getStudySkillList()
{
    vector<int> skillList;
    skillList.clear();
    for (int i = 0; i < mStudySkills.size(); i++) {
        if (mStudySkills[i] == NULL) {
            continue;
        }
        if( mStudySkills[i]->getID() )
        {
            skillList.push_back(mStudySkills[i]->getID());
        }
    }
    return skillList;
}

vector<int> SkillMod::getEquipSkillList()
{
    vector<int> skills;
    for (int i = 0; i < mEquipedSkills.size(); i++) {
        if (mEquipedSkills[i] != NULL) {
            skills.push_back(mEquipedSkills[i]->getID());
        }
        else{
            skills.push_back(0);
        }
    }
    
    return skills;
}

bool SkillMod::IsSkillLearn(int skillid)
{
    for (int i = 0; i < mStudySkills.size(); i++) {
        if (mStudySkills[i]) {
            if (skillid == mStudySkills[i]->getID()) {
                return true;
            }
        }
    }
    
    // 添加宠物判断
    Pet* pet = ((Role*)mOwner)->mPetMgr->getActivePet();
    return (pet && pet->hasSkill(skillid));
    //return false;
}

int SkillMod::getSkillLevel(int index)
{
    for (int i = 0; i < mStudySkills.size(); i++) {
        if (mStudySkills[i]) {
            
            int skillid = mStudySkills[i]->getID();
            
            int tmpIndex = getSkillIndex(skillid);
            
            if (tmpIndex == index) {
                return CalcSkillLvl(skillid);
            }
        }
    }
    
    return 0;
}

//void ModifyCurHp(Obj* mOwner, Obj* target, void* skill)
//{
//    vector<Obj*> objs = mOwner->getScene()->areaObjs(mOwner, 100);
//    vector<Obj*>::iterator it = objs.begin();
//    for (; it!=objs.end(); it++) {
        //Obj* obj = (Obj*)(*it);
        //obj->beAtk(mOwner,(Skill*) skill); 做在客户端
//    }
    //return true;
//}

void SkillMod::skillsAddBattleProp(BattleProp &prop)
{
    for (int i = 0; i < mStudySkills.size(); i++) {
        
        if (mStudySkills[i] == NULL) {
            continue;
        }
        
        if ( (mStudySkills[i]->getID()/100)%100 <= 0 ) {
            continue;
        }
        
        SkillCfgDef* skill = SkillCfg::getCfg(mStudySkills[i]->getID());
        if (skill == NULL) {
            continue;
        }
        
        for (int j = 0; j < skill->effectList.size(); j++) {
            
            SkillEffectVal* effect = skill->effectList[j];
            
            if ( effect->mTargetType !=  eForSelf) {
                continue;
            }
            switch (effect->affectType) {
                case eBuffChangeAttack:
                    prop.mAtk *= (1 + effect->affectfloat);
                    prop.mAtk += effect->affectValue;
                    break;
                case eBuffChangeDef:
                    prop.mDef *= (1 + effect->affectfloat);
                    prop.mDef += effect->affectValue;
                    break;
                case eBuffChangeDogge:
                    prop.mDodge *= (1 + effect->affectfloat);
                    prop.mDodge += effect->affectValue;
                    break;
                case eBuffChangeCri:
                    prop.mCri *= (1 + effect->affectfloat);
                    prop.mCri += effect->affectValue;
                    break;
                case eBuffChangeHit:
                    prop.mHit *= (1 + effect->affectfloat);
                    prop.mHit += effect->affectValue;
                    break;
                default:
                    break;
            }
        }
    }
}

void SkillMod::equipPassiveSkill(BaseProp& baseProp, BattleProp& battleProp)
{
    SkillIncrBattle tmp;
    for (int i = 0; i < mStudySkills.size(); i++)
    {
        if ( mStudySkills[i] == NULL )
        {
            continue;
        }
        
        if ( mStudySkills[i]->getSkillType() == ePassive)
        {
            SkillCfg::calcBattlePropFromSkill(mStudySkills[i]->getID(), eForSelf, tmp);
        }
    }
    
    baseProp += tmp.baseProp;
    battleProp += tmp.battleProp;
}

bool SkillMod::equipInitiativeSkill(int skillid, int position)
{
    if (position >= mEquipedSkills.size()) {
        return false;
    }
    
    for (int i = 0; i < mEquipedSkills.size(); i++) {
        
        //判断是否已经装备了该技能
        if (mEquipedSkills[i] != NULL && skillid == mEquipedSkills[i]->getID()) {
            
            //如果已在目标位置返回true
            if (i == position) {
                return true;
            }
            else    //否则把原来的位置置空
            {            
                mEquipedSkills[i] = NULL;
                break;
            }
        }
    }
    
    Skill* skill = getSkillInSkillTable(skillid);
    mEquipedSkills[position] = skill;
    return true;
}

bool SkillMod::unEquipInitiativeSkill(int position)
{
    if (position >= mEquipedSkills.size()) {
        return false;
    }
    
    mEquipedSkills[position] = NULL;
    return true;
}

bool SkillMod::unEquipInitiativeSkillBySkillId(int skillId)
{
    for (int i = 0; i < mEquipedSkills.size(); i++) {
        if (mEquipedSkills[i] != NULL && mEquipedSkills[i]->getID() == skillId) {
            mEquipedSkills[i] = NULL;
            return true;
        }
    }
    
    return false;
}

bool SkillMod::checkAutoEquip(int skillid)
{
    SkillCfgDef* def = SkillCfg::getCfg(skillid);
    if (def == NULL || def->skillType != eInitiative) {
        return false;
    }
    
    int noSkillIndex = -1;
    for (int i = 0; i < mEquipedSkills.size(); i++) {
        //如果有空位，记录第一个空位
        if (mEquipedSkills[i] == NULL) {
            if (noSkillIndex >= 0) {
                continue;
            }
            noSkillIndex = i;
        }
        else if(mEquipedSkills[i]->getID() == skillid)
        {
            return false;
        }
    }
    
    if (noSkillIndex >= 0) {
        if(equipInitiativeSkill(skillid, noSkillIndex))
        {
            return true;
        }
    }
    
    return false;
}

void SkillMod::saveEquipSkillList(int roleid)
{
    vector<int> equipSkills = getEquipSkillList();
    onSaveEquipSkillList(roleid, equipSkills);
}

void SkillMod::saveStudySkillList()
{
    vector<int> skillList;
    skillList = this->getStudySkillList();
    SaveSkillTable(mOwner->getInstID(), skillList);
}

bool SkillMod::skillReplace(int preSkill, int newSkill)
{
    int skillLvl = getSkillLevel(preSkill);
    
    unEquipInitiativeSkillBySkillId(getSkillIndex(preSkill) + skillLvl);
    
    deleteSkill(preSkill);
    
    int newSkillId = getSkillIndex(newSkill) + skillLvl;
    
    if (learn(newSkillId))
    {
        checkAutoEquip(newSkillId);
        
        //保存新的技能列表
//        vector<int> skillList;
//        skillList = getStudySkillList();
//        SaveSkillTable(mOwner->getInstID(), skillList);
//        
//        saveEquipSkillList(mOwner->getInstID());
        
        return true;
    }
    
    return false;
}

bool SkillMod::deleteSkill(int skillId)
{
    for(int i=0;i < mStudySkills.size();i++) {
        
        if( mStudySkills[i] != NULL ){
            
            int lastskill = mStudySkills[i]->getID();
            
            //对学过的技能升级
            if (getSkillIndex(lastskill) == getSkillIndex(skillId)) {
                
                Skill* oldSkill = mStudySkills[i];
                delete oldSkill;
                
                mStudySkills[i] = NULL;
                return true;
            }
        }
    }
    return false;
}