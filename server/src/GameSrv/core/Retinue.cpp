//
//  Retinue.cpp
//  GameSrv
//
//  Created by pireszhi on 15-6-19.
//
//

#include <stdio.h>
#include "std_includes.h"
#include "Retinue.h"
#include "DataCfg.h"
#include "main.h"
#include "ItemBag.h"
#include "Role.h"
#include "GameScript.h"

const char* RetinueMgr::sGenIdKey = "retinue:incr_id";

void getRetinueInfoFromLua(RetinueModDef* cfg, int lvl, int star, int& nextLvlExp, int& nextStarExp, BattleProp& battleProp)
{
    if (cfg == NULL) {
        return;
    }
    
    try
    {
        CMsgTyped msg;
        msg.SetInt(lvl);
        msg.SetInt(star);
        
        msg.SetInt(cfg->bExp);
        msg.SetInt(cfg->bStarExp);
        
        msg.SetInt(cfg->bAtk);
        msg.SetInt(cfg->bDef);
        msg.SetInt(cfg->bHp);
        msg.SetFloat(cfg->bHit);
        msg.SetFloat(cfg->bDodge);
        msg.SetFloat(cfg->bCri);
        
        if (GameScript::instance()->call("retinue_getRetinueInfo", msg) == 8) {
            
            battleProp.setAtk((int)msg.FloatVal());
            battleProp.setDef((int)msg.FloatVal());
            battleProp.setHp((int)msg.FloatVal());
            battleProp.setMaxHp(battleProp.getHp());
            battleProp.setHit(msg.FloatVal());
            battleProp.setDodge(msg.FloatVal());
            battleProp.setCri(msg.FloatVal());
            
            nextLvlExp = msg.FloatVal();
            nextStarExp = msg.FloatVal();
        }
    }
    catch(...)
    {
        
    }
    
    return;
}

int getRetinueLvlExp(RetinueModDef* cfg, int lvl)
{
    int exp = 0;
    
    if (cfg == NULL) {
        return exp;
    }
    
    try
    {
        CMsgTyped msg;
        msg.SetInt(lvl);
        msg.SetInt(cfg->bExp);

        if (GameScript::instance()->call("retinue_getLvlUpExp", msg) == 1) {
            
            exp = msg.FloatVal();
        }
    }
    catch(...)
    {
        
    }
    
    return exp;
}

int getRetinueStarLvlExp(RetinueModDef* cfg, int star)
{
    int exp = 0;
    
    if (cfg == NULL) {
        return exp;
    }
    
    try
    {
        CMsgTyped msg;
        msg.SetInt(star);
        msg.SetInt(cfg->bStarExp);
        
        if (GameScript::instance()->call("retinue_getStarUpExp", msg) == 1) {
            
            exp = msg.FloatVal();
            
        }
    }
    catch(...)
    {
        
    }
    
    return exp;
}

void sendRetinueNotify(Role* role, Retinue* retinue)
{
    notify_retinue_info notify;
    
    retinue->getRetinueInfo(notify.retinue);
    
    sendNetPacket(role->getSessionId(), &notify);
}

/*
 *retineuSkillMgr
 */
RetinueSkillMgr::RetinueSkillMgr():mMaster(NULL)
{
    mRetinueEquipedSkills.resize(3, NULL);
}

RetinueSkillMgr::~RetinueSkillMgr()
{
    int skillCount = mRetinueSkills.size();
    
    for (int i = 0; i < skillCount; i++) {
        RetinueSkillInfo* skill = mRetinueSkills[i];
        if (skill) {
            delete skill;
            mRetinueSkills[i] = NULL;
        }
    }
    
    int equipedCount = mRetinueEquipedSkills.size();
    for (int i = 0; i < equipedCount; i++) {
        RetinueSkillInfo* skill = mRetinueEquipedSkills[i];
        if (skill) {
            delete skill;
            mRetinueEquipedSkills[i] = NULL;
        }
    }
}

RetinueSkillMgr* RetinueSkillMgr::Create(Role *role)
{
    if (role == NULL) {
        return NULL;
    }
    
    RetinueSkillMgr* mgr = new RetinueSkillMgr;
    mgr->mMaster = role;
    mgr->load();
    
    return mgr;
}

void RetinueSkillMgr::load()
{
    if (mMaster == NULL) {
        return;
    }
    StringTokenizer skillList(mMaster->getRetinueSkills(), ";");
    for (int i = 0; i < skillList.count(); i++) {
        StringTokenizer skill(skillList[i], "-");
        if (skill.count() < 2) {
            continue;
        }
        
        int skillId = Utils::safe_atoi(skill[0].c_str());
        int lvl = Utils::safe_atoi(skill[1].c_str());
        
        if (skillId <= 0 || lvl <= 0) {
            continue;
        }
        
        if(getSkillInfo(skillId))
        {
            continue;
        }
        
        RetinueSkillInfo* info = new RetinueSkillInfo(skillId, lvl);
        mRetinueSkills.push_back(info);
    }
    
    StringTokenizer equipedSkills(mMaster->getRetinueEquipedSkills(), ";");
    for (int i = 0; i < equipedSkills.count() && i < mRetinueEquipedSkills.size(); i++) {
        
        int skillId = Utils::safe_atoi(equipedSkills[i].c_str());
        
        if (skillId <= 0) {
            continue;
        }
        
        RetinueSkillInfo* info = getSkillInfo(skillId);
        
        mRetinueEquipedSkills[i] = info;
    }
}

void RetinueSkillMgr::saveSkills()
{
    ostringstream list;
    
    int skillCount = mRetinueSkills.size();
    for (int i = 0; i < skillCount; i++) {
        RetinueSkillInfo* skill = mRetinueSkills[i];
        if (skill == NULL) {
            continue;
        }
        
        list<<skill->retinueSkillId<<"-"<<skill->lvl;
        list<<";";
    }
    
    mMaster->setRetinueSkills(list.str());
    mMaster->NewRoleProperty::save();
}

void RetinueSkillMgr::saveEquipedSkills()
{
    ostringstream equipedSkills;
    
    for (int i = 0; i < mRetinueEquipedSkills.size(); i++) {
        RetinueSkillInfo* skill = mRetinueEquipedSkills[i];
        
        if (skill == 0) {
            equipedSkills<<0;
        }
        else
        {
            equipedSkills<<skill->retinueSkillId;
        }
        equipedSkills<<";";
    }
    
    mMaster->setRetinueEquipedSkills(equipedSkills.str());
    mMaster->NewRoleProperty::save();
}

bool RetinueSkillMgr::upgSkill(int skillId, int incrLvl)
{
    RetinueSkillInfo* skill = getSkillInfo(skillId);
    
    if (NULL == skill) {
        
        skill = addNewSkill(skillId);
        
        if (skill == NULL) {
            return false;
        }
    }
    
    skill->lvl += incrLvl;
    saveSkills();
    
    return true;
}

bool RetinueSkillMgr::equipSkill(int skillId, int index)
{
    RetinueSkillInfo* skill = getSkillInfo(skillId);
    
    if (NULL == skill) {
        return false;
    }
    
    if (index >= mRetinueEquipedSkills.size() || index < 0) {
        return false;
    }
    
    mRetinueEquipedSkills[index] = skill;
    
    saveEquipedSkills();
    
    return true;
}

bool RetinueSkillMgr::unEquipSkill(int index)
{
    if (index >= mRetinueEquipedSkills.size() || index < 0) {
        return false;
    }
    
    mRetinueEquipedSkills[index] = NULL;
    saveEquipedSkills();
    
    return true;
}

vector<int> RetinueSkillMgr::getEquipedSkills()
{
    vector<int> skills;
    vector<RetinueSkillInfo*>::iterator it = mRetinueEquipedSkills.begin();
    for(; it != mRetinueEquipedSkills.end(); ++it){
        if((*it) != NULL && (*it) -> retinueSkillId != 0){
            skills.push_back((*it)->retinueSkillId);
        }
    }
    return skills;
}

RetinueSkillInfo* RetinueSkillMgr::addNewSkill(int skillId)
{
    RetinueSkillInfo* newSkill = new RetinueSkillInfo(skillId, 0);
    
    mRetinueSkills.push_back(newSkill);
    
    return newSkill;
}

RetinueSkillInfo* RetinueSkillMgr::getSkillInfo(int skillId)
{
    for (int i = 0; i < mRetinueSkills.size(); i++) {
        RetinueSkillInfo* skill = mRetinueSkills[i];
        if (NULL == skill) {
            continue;
        }
        
        if (skill->retinueSkillId == skillId) {
            return skill;
        }
    }
    
    return NULL;
}

void RetinueSkillMgr::clientGetSkillsInfo(vector<obj_retinue_skillInfo> &data)
{
    for (int i = 0; i < mRetinueSkills.size(); i++) {
        RetinueSkillInfo* skill = mRetinueSkills[i];
        
        if (skill == NULL) {
            continue;
        }
        
        obj_retinue_skillInfo obj;
        obj.retinueSkillId = skill->retinueSkillId;
        obj.skilllvl = skill->lvl;
        
        data.push_back(obj);
    }
}

void RetinueSkillMgr::clientGetEquipedSkills(vector<obj_retinue_skillInfo>& data)
{
    for (int i = 0; i < mRetinueEquipedSkills.size(); i++) {
        
        RetinueSkillInfo* skill = mRetinueEquipedSkills[i];
        
        obj_retinue_skillInfo obj;
        
        if (skill) {
            
            obj.retinueSkillId = skill->retinueSkillId;
            
            obj.skilllvl = skill->lvl;
            
        }
        
        data.push_back(obj);
    }
}

SpiritSpeakMgr* SpiritSpeakMgr::Create(Role* role)
{
    SpiritSpeakMgr* mgr = new SpiritSpeakMgr;
    if (mgr == NULL) {
        return NULL;
    }
    
    mgr->mMaster = role;
    return mgr;
}

void SpiritSpeakMgr::load()
{
    if (mMaster == NULL) {
        return;
    }
    
    string listJson = mMaster->getSpiritSpeak();
    
    Json::Value jsonVal;
    Json::Reader reader;
    
    reader.parse(listJson, jsonVal);
    if (jsonVal.isArray() == false) {
        return;
    }
    
    for (int i = 0; i < jsonVal.size(); i++) {
        Json::Value& jsonSpirit = jsonVal[i];
        
        int petid = jsonSpirit["petId"].asInt();
        int spiritId = jsonSpirit["spiritId"].asInt();
        
        SpiritSpeakDef* sCfg = RetinueCfg::getSpiritSpeakCfg(spiritId);
        if (sCfg == NULL) {
            continue;
        }
        
        SpiritSpeak speak(sCfg->gribCount);
        speak.petid = petid;
        speak.spiritId = spiritId;
        speak.petid = jsonSpirit["petId"].asInt();
        speak.spiritId = jsonSpirit["spiritId"].asInt();
        speak.retinues.resize(sCfg->gribCount, 0);
        
        string retinueIds = jsonSpirit["retinues"].asString();
        StringTokenizer retinues(retinueIds, ";");
        for (int j = 0; j < retinues.count() && j < speak.retinues.size(); j++) {
            
            int retinueId = Utils::safe_atoi(retinues[j].c_str());
            
            Retinue* retinue = mMaster->getRetinueMgr()->getRetinue(retinueId);
            if (retinue) {
                retinue->setSpiritSpeakId(speak.spiritId);
            }
            
            speak.retinues[j] = retinueId;
        }
        
        calcSingleSpiritProp(speak);
        mSpiritSpeak.push_back(speak);
    }
}

void SpiritSpeakMgr::save()
{
    ostringstream retinueList;
    Json::Value speakList;
    
    for (int i = 0; i < mSpiritSpeak.size(); i++) {
        Json::Value speakVal;
        speakVal["spiritId"] = mSpiritSpeak[i].spiritId;
        speakVal["petId"] = mSpiritSpeak[i].petid;
        speakVal["retinues"] = mSpiritSpeak[i].retinueList2String();
        
        speakList.append(speakVal);
    }
    
    Json::FastWriter writer;
    string speakListStr = writer.write(speakList);
    
    mMaster->setSpiritSpeak(speakListStr);
    mMaster->NewRoleProperty::save();
}

void SpiritSpeakMgr::removeSpiritSpeakPet(int spiritId)
{
    for (int i = 0; i < mSpiritSpeak.size(); i++) {
        
        SpiritSpeak& spirit = mSpiritSpeak[i];
        if (spirit.spiritId == spiritId)
        {
            spirit.petid = 0;
            calcSingleSpiritProp(spirit);
        }
    }
}

bool SpiritSpeakMgr::setSpiritSpeak(int spiritId, int position, int petId, int retinueId)
{
    if (spiritId <= 0)
    {
        return false;
    }
    
    SpiritSpeakDef* sCfg = RetinueCfg::getSpiritSpeakCfg(spiritId);
    if (sCfg == NULL) {
        return false;
    }
    
    Retinue* retinue = mMaster->getRetinueMgr()->getRetinue(retinueId);

    if (retinue) {
        retinue->setSpiritSpeakId(spiritId);
    }
    
    for (int i = 0; i < mSpiritSpeak.size(); i++) {
        
        SpiritSpeak& spirit = mSpiritSpeak[i];
        if (spirit.spiritId == spiritId) {
            
            //判断是否只换宠物
            if (position >= 0 && retinueId >= 0)
            {
                if (spirit.retinues.size() <= position) {
                    spirit.retinues.resize(position + 1, 0);
                }
                
                int oldRetinueId = spirit.retinues[position];
                Retinue* oldRetinue = mMaster->getRetinueMgr()->getRetinue(oldRetinueId);
                if (oldRetinue) {
                    oldRetinue->setSpiritSpeakId(0);
                }
                
                spirit.retinues[position] = retinueId;
            }
            
            Pet* pet = mMaster->mPetMgr->getPet(petId);
            if (pet) {
                spirit.petid = petId;
            }
            else{
                spirit.petid = 0;
            }
            
//            calcSingleSpiritProp(spirit);
            save();
            return true;
        }
    }
    
    //只换宠物
    SpiritSpeak spirit(sCfg->gribCount);
    spirit.spiritId = spiritId;
    spirit.retinues.resize(sCfg->gribCount, 0);
    
    if (position >= 0 && retinueId > 0) {
        
        spirit.retinues[position] = retinueId;
        
    }
    
    Pet* pet = mMaster->mPetMgr->getPet(petId);
    if (pet) {
        spirit.petid = petId;
        
        int oldSpiritSpeakId = pet->getSpiritSpeakId();
        if (oldSpiritSpeakId)
        {
            mMaster->getRetinueMgr()->mSpiritSpeakMgr->removeSpiritSpeakPet(oldSpiritSpeakId);
        }
        
        pet->setSpiritSpeakId(spiritId);
        
    }
    else{
        
        pet = mMaster->mPetMgr->getPet(spirit.petid);
        
        if (pet) {
            pet->setSpiritSpeakId(0);
        }
        
        spirit.petid = 0;
    }
    
//    calcSingleSpiritProp(spirit);
    mSpiritSpeak.push_back(spirit);
    
    save();
    return true;
}

void SpiritSpeakMgr::calcProp(BattleProp& battleProp)
{
    for (int i = 0; i < mSpiritSpeak.size(); i++) {
        
        if(mSpiritSpeak[i].spiritId == 0)
        {
            continue;
        }
        calcSingleSpiritProp(mSpiritSpeak[i]);
        
        battleProp += mSpiritSpeak[i].mBattleProp;
    }
}

void SpiritSpeakMgr::calcSingleSpiritProp(SpiritSpeak& spirit)
{
    if ( spirit.spiritId <= 0) {
        return;
    }
    
    BattleProp spiritProp;
    
    Pet* pet = mMaster->mPetMgr->getPet(spirit.petid);
    
    do
    {
        if (pet == NULL) {
            break;
        }
        
        PetCfgDef* petCfg = PetCfg::getCfg(pet->petmod);
        
        if (petCfg == NULL) {
            break;
        }
        
        float spiritcoef = 0.0f;

        spiritcoef = petCfg->getSpiritSpeakCoef();   //petCfg-> 通灵加成系数
        
        for (int i = 0; i < spirit.retinues.size(); i++) {
            
            int retinueId = spirit.retinues[i];
            
            if ( retinueId <= 0) {
                continue;
            }
            
            Retinue* retinue = mMaster->getRetinueMgr()->getRetinue(retinueId);
            
            if (retinue == NULL) {
                //log something
                continue;
            }
            
            BattleProp retinueProp;
            retinue->getBattleProp(retinueProp);
            
            spiritProp += retinueProp;
        }
        
        spiritProp *= spiritcoef;
        
    }while(false);

    
    spirit.mBattleProp = spiritProp;
}

void SpiritSpeakMgr::clientGetSpiritSpeakInfo(vector<obj_spiritspeak_info> &data)
{
    for (int i = 0; i < mSpiritSpeak.size(); i++) {
        
        SpiritSpeak& spirit = mSpiritSpeak[i];
        obj_spiritspeak_info obj;
        onClientGetSingleSpiritSpeakInfo(spirit, obj);
        data.push_back(obj);
    }
}

void SpiritSpeakMgr::onClientGetSingleSpiritSpeakInfo(SpiritSpeak& spirit, obj_spiritspeak_info& obj)
{
    obj.petId = spirit.petid;
    obj.spiritspeakId = spirit.spiritId;
    obj.retinueIds = spirit.retinues;
    
    obj.atk = spirit.mBattleProp.getAtk();
    obj.def = spirit.mBattleProp.getDef();
    obj.hp = spirit.mBattleProp.getMaxHp();
    obj.hit = spirit.mBattleProp.getHit();
    obj.dodge = spirit.mBattleProp.getDodge();
    obj.cri = spirit.mBattleProp.getCri();
}

void SpiritSpeakMgr::clientGetSingleSpiritSpeakInfo(int spiritspeakId, obj_spiritspeak_info& obj)
{
    for (int i = 0; i < mSpiritSpeak.size(); i++) {
        
        SpiritSpeak& spirit = mSpiritSpeak[i];
        if(spirit.spiritId == spiritspeakId)
        {
            onClientGetSingleSpiritSpeakInfo(spirit, obj);
            return;
        }
    }
}

/*
 * Retinue
 */

Retinue::Retinue(int retinueId):mRetinueId(retinueId),
                                mNextLvlupExp(0),
                                mNextStarUpExp(0),
                                propertyIntension(0),
                                mSpiritSpeakId(0),
                                mPropStrength(0)
{
    RetinueData::active(mRetinueId);
}

bool Retinue::load(redisContext *conn)
{
    RetinueData::load(mRetinueId);
    
    return true;
}

//struct TestRetinue
//{
//    TestRetinue()
//    {
//        int nextExp = 0;
//        int nextStarExp = 0;
//        
//        BattleProp battleProp;
//        
//        RetinueModDef* cfg = RetinueCfg::getRetinueModCfg(101002);
//        getRetinueInfoFromLua(cfg, 1, 1, nextExp, nextStarExp, battleProp);
//        
//        printf("exp:%d --- starExp:%d\n", nextExp, nextStarExp);
//        printf("atk:%d, def:%d, hp:%d, hit:%f, dodge:%f, cri:%f\n", battleProp.getAtk(), battleProp.getDef(), battleProp.getHp(), battleProp.getHit(), battleProp.getDodge(), battleProp.getCri());
//        
//    }
//}mTestRetinue;

void Retinue::calcProp()
{
    BattleProp battleProp;
    
    RetinueModDef* cfg = RetinueCfg::getRetinueModCfg(getModId());
    
    if (cfg == NULL) {
        return;
    }
    
    int nextExp = 0;
    int nextStarExp = 0;
    
    //以后根据等级, 星级用lua计算;
    getRetinueInfoFromLua(cfg, getLvl(), getStar(), nextExp, nextStarExp, battleProp);
    
    mBattleProp = battleProp;
    
    if (getLvl() >= cfg->maxLvl) {
        nextExp = 0;
    }
    
    if (getStar() >= cfg->maxStar) {
        nextStarExp = 0;
    }
    
    //计算下一等级所需经验
    mNextLvlupExp = nextExp;
    
    //计算下一星级所需经验
    mNextStarUpExp = nextStarExp;
    
    mPropStrength = calcBattleForce(&mBattleProp);
}

int Retinue::getId()
{
    return mRetinueId;
}

void Retinue::getRetinueInfo(obj_retinue_info& data)
{
//    obj_retinue_info obj;
    data.retinueId = this->getId();
    data.retinueMod = this->getModId();
    data.isFighting = this->getActive();
    data.lvl = this->getLvl();
    data.star = this->getStar();
    data.isFighting = this->getActive();
    data.nextLvlExp = this->getNextLvlExp();
    data.nextStarupExp = this->getNextStarupExp();
    data.exp = this->getExp();
    
    data.propStrength = this->mPropStrength;
    
    BattleProp batprop;
    this->getBattleProp(batprop);
    
    data.atk = batprop.getAtk();
    data.def = batprop.getDef();
    data.hp = batprop.getMaxHp();
    data.hit = batprop.getHit();
    data.dodge = batprop.getDodge();
    data.cri = batprop.getCri();
}

void Retinue::getBattleProp(BattleProp& battleProp)
{
    battleProp = mBattleProp;
}

void Retinue::getBattleProp(obj_retinueBattleProp &battleProp)
{
    battleProp.retinueid = mRetinueId;
    battleProp.lvl = this -> getLvl();
    battleProp.retinuemod = this -> getModId();
    battleProp.atk = mBattleProp.getAtk();
    battleProp.def = mBattleProp.getDef();
    battleProp.dodge = mBattleProp.getDodge();
    battleProp.hit = mBattleProp.getHit();
    battleProp.cri = mBattleProp.getCri();
}

void Retinue::addExp(int exp)
{
    setExp(getExp() + exp);
    
    lvlUp();
    
    calcProp();
    
    RetinueData::save();
}

bool Retinue::lvlUp()
{
    RetinueModDef* cfg = RetinueCfg::getRetinueModCfg(getModId());
    
    if (cfg == NULL) {
        return false;
    }
    
    if (getLvl() >= cfg->maxLvl) {
        setExp(0);
        return false;
    }
    
    int nextLvlExp = getRetinueLvlExp(cfg, getLvl() + 1);
    
    while(getExp() >= nextLvlExp) {
        
        setLvl(getLvl() + 1);
        setExp(getExp() - nextLvlExp);
        
        if (getLvl() >= cfg->maxLvl) {
            setExp(0);
            return true;
        }
        
        nextLvlExp = getRetinueLvlExp(cfg, getLvl() + 1);
    }
    
    return false;
}

bool Retinue::starUp()
{
    setStar(getStar() + 1);
    
    RetinueData::save();
    
    return true;
}

string Retinue::retinueInfo2String()
{
    ostringstream ostring;
    
    ostring
    << this->getActive()
    << ","
    
    << this->getId()
    << ","
    
    << this->getModId()
    << ","
    
    << this->getExp()
    << ","
    
    << this->getLvl()
    << ","
    
    << this->getStar()
    << ","
    
    << this->getNextLvlExp()
    << ","
    
    << this->getNextStarupExp()
    << ","
    
    << this->mBattleProp.getAtk()
    << ","
    
    << this->mBattleProp.getDef()
    << ","
    
    << this->mBattleProp.getHp()
    << ","
    
    << this->mBattleProp.getHit()
    << ","
    
    << this->mBattleProp.getDodge()
    << ","
    
    << this->mBattleProp.getCri();
    
    return ostring.str();
}
/*
 * Mgr
 */
RetinueMgr::RetinueMgr(): mRetinueBag(NULL),
                          mSkillMgr(NULL),
                          mSpiritSpeakMgr(NULL),
                          mActiveRetinue(NULL)
{
    
}

RetinueMgr::~RetinueMgr()
{
    int retinueCount = mRetinues.size();
    for (int i = 0; i < retinueCount; i++) {
        Retinue* retinue = mRetinues[i];
        
        if (retinue) {
            mRetinues[i] = NULL;
            delete retinue;
        }
    }
    
    if (mRetinueBag) {
        delete mRetinueBag;
        mRetinueBag = NULL;
    }
    
    if (mSkillMgr) {
        delete mSkillMgr;
        mSkillMgr = NULL;
    }
    
    if (mSpiritSpeakMgr) {
        delete mSpiritSpeakMgr;
        mSpiritSpeakMgr = NULL;
    }
}

int RetinueMgr::genRetinueId()
{
    RedisResult result(redisCmd("incr %s", RetinueMgr::sGenIdKey));
    return result.readInt();
}

RetinueMgr* RetinueMgr::create(Role* role)
{
    RetinueMgr* mgr = new RetinueMgr;
    mgr->mMaster = role;
    mgr->mRetinueBag = RetinueBag::Create(RETINUEBAG_MAX_SIZE);
    mgr->mSkillMgr = RetinueSkillMgr::Create(mgr->mMaster);
    mgr->mSpiritSpeakMgr = SpiritSpeakMgr::Create(mgr->mMaster);
    
    return mgr;
}

Retinue* RetinueMgr::Create(int modId)
{
    RetinueModDef* cfg = RetinueCfg::getRetinueModCfg(modId);
    
    if ( cfg == NULL) {
        return NULL;
    }
    
    int newid = genRetinueId();
    Retinue* retinue = new Retinue(newid);
    
    if (retinue) {
        retinue->setLvl(1);
        retinue->setModId(modId);
        retinue->setStar(0);
        retinue->setActive(false);
        retinue->setTolalExp(0);
        retinue->setExp(0);
        
        //读lua获取属性
        retinue->calcProp();
    }
    
    return retinue;
}

void RetinueMgr::addRetinue(Retinue* retinue)
{
    retinue->setOwner(mMaster->getInstID());
    retinue->RetinueData::save();
    mRetinues.push_back(retinue);
}

void RetinueMgr::save()
{
    int retinueSize = mRetinues.size();
    
    ostringstream strstream;
    
    for (int i = 0; i < retinueSize; i++) {
        
        if (mRetinues[i] == NULL) {
            continue;
        }
        
        strstream<<mRetinues[i]->getId();
        strstream<<";";
    }
    
    //保存侍魂列表
    mMaster->setRetinueList(strstream.str());
    mMaster->NewRoleProperty::save();
}

Retinue* RetinueMgr::getRetinue(int retinueId)
{
    int retinueSize = mRetinues.size();
    
    for (int i = 0; i < retinueSize; i++) {
        
        Retinue* retinue = mRetinues[i];
        if ( retinue == NULL) {
            continue;
        }
        
        if (retinue->getId() == retinueId) {
            return retinue;
        }
    }
    
    return NULL;
}

void RetinueMgr::loadAllRetinue()
{
    //获取侍魂列表
    string retinueList = mMaster->getRetinueList();
    
    StringTokenizer list(retinueList, ";");
    
    for (int i = 0; i < list.count(); i++) {
        
        int retinueId = Utils::safe_atoi(list[i].c_str());
        
        Retinue* retinue = RetinueMgr::loadRetinue(retinueId);
        
        if (retinue == NULL) {
            continue;
        }
        
        if(retinue -> getActive()){
            mActiveRetinue = retinue;
        }
        
        retinue->calcProp();
        mRetinues.push_back(retinue);
    }
    
    //
    mRetinueBag->Load(mMaster->getInstID());
    
    vector<int> effgrids;
    mRetinueBag->Sort(effgrids);
    mRetinueBag->Store(mMaster->getInstID(), effgrids);
    
    mSkillMgr->load();
    mSpiritSpeakMgr->load();
}

Retinue* RetinueMgr::loadRetinue(int retinueId)
{
    Retinue* retinue = new Retinue(retinueId);
    
    if (NULL == retinue) {
        return NULL;
    }
    
    if (retinue->load(get_DbContext()) == false) {
        delete retinue;
        return NULL;
    }
    
    return retinue;
}

void RetinueMgr::clientGetRetinueList(vector<obj_retinue_info>& data)
{
    for (int i = 0; i < mRetinues.size(); i++) {
        
        Retinue* retinue = mRetinues[i];
        if (retinue == NULL) {
            continue;
        }
        
        obj_retinue_info obj;
        
        retinue->getRetinueInfo(obj);
        
        data.push_back(obj);
    }
}
