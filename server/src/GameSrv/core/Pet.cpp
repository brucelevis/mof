#include "Pet.h"

#include "redis_helper.h"
#include "main.h"
#include "DBPetPropName.h"
#include "DBRolePropName.h"
#include "GameLog.h"
#include "Paihang.h"
#include "Role.h"
#include "Guild.h"
#include "NumericConvert.h"
#include "Totem.h"
#include "EnumDef.h"
#include "mail_imp.h"
#include "DataCfg.h"

INSTANTIATE_SINGLETON(PetMgr)


const char* Pet::sGenIdKey = "pet:incr_id";

Pet::Pet(int _petid) : Obj(ePet),
                       mSpiritSpeakId(0)
{
    petid = _petid;

    PetNewProperty::active(petid);

    isactive = 0;
}

bool Pet::load(redisContext* ctx)
{
    RedisResult result((redisReply*)redisCommand(ctx, "hmget pet:%d petmod lvl exp totalexp growth batk bdef bhp bdodge bhit skills owner isactive phys capa inte stre star starlvl", petid));

    petmod = result.readHash(0, 0);
    if (petmod == 0)
    {
        return false;
    }

    lvl = result.readHash(1, 0);
    
    exp = result.readHash(2, 0);
    check_min(exp, 0);
    
    totalexp = result.readHash(3, 0);
    check_min(totalexp, 0);
    int tmpgrowth = result.readHash(4, 0);

    batk = result.readHash(5, 0);
    bdef = result.readHash(6, 0);
    bhp = result.readHash(7, 0);
    bdodge = result.readHash(8, 0.0f);
    bhit = result.readHash(9, 0.0f);

    string skillstr = result.readHash(10);

    owner = result.readHash(11, 0);
    isactive = result.readHash(12, (int)kPetSleep);
    mPhys = result.readHash(13, 0);
    mCapa = result.readHash(14, 0);
    mInte = result.readHash(15, 0);
    mStre = result.readHash(16, 0);

    star  = result.readHash(17, 0);
    starlvl = result.readHash(18,0);

    PetNewProperty::load(petid);
    
    //简体宠物数值转换
    //doPetNumericConvert(this, petmod, batk, bdef, bhp, bhit, bdodge);
    
    convertStarToStage();
    
    calculateAttributes();
    
//    vector<int> myskills;
//    getSortedSkills(myskills);
    
    return true;
}

void Pet::save()
{
    doRedisCmd("hmset pet:%d  petmod %d lvl %d exp %d totalexp %d \
               batk %d bdef %d bhp %d bdodge %f bhit %f \
               owner %d isactive %d phys %d capa %d inte %d stre %d star %d starlvl %d ConvertMark 1",
               petid, petmod, lvl, exp, totalexp,
               batk, bdef, bhp, bdodge, bhit,
               owner, isactive, mPhys, mCapa, mInte, mStre,star,starlvl);

    saveNewProperty();
}


void Pet::saveBattle()
{
    doRedisCmd("hset pet:%d battle %d", petid, mBattle);
}

void Pet::saveExp()
{
    doRedisCmd("hmset pet:%d exp %d totalexp %d lvl %d", petid, exp, totalexp, lvl);
}

void Pet::saveProp()
{
    doRedisCmd("hmset pet:%d phys %d capa %d inte %d stre %d", petid, mPhys, mCapa, mInte, mStre);
}

void Pet::saveStar()
{
    doRedisCmd("hmset pet:%d %s %d %s %d",petid,
               g_GetPetPropName(ePetPropType_Star),star,
               g_GetPetPropName(ePetPropType_StarLevel),starlvl);
}

void Pet::saveNewProperty()
{
    PetNewProperty::save();
}



void Pet::gm_SetLvl(int i_lvl)
{
    PetCfgDef* cfg = PetCfg::getCfg(petmod);
    int i_NewExp = PetCfg::getLvlTotalExp(cfg->getQua(), i_lvl);
    lvl = 0;
    totalexp=0;
    exp=0;
    
    Role *role = SRoleMgr.GetRole(owner);
    role->addPetExp(this->getId(), i_NewExp, "gm_setlvl");
    
}
void Pet::addExp(int _exp, int ownerlvl)
{
    PetCfgDef* cfg = PetCfg::getCfg(petmod);
    if (cfg == NULL)
    {
        return;
    }
    

    //宠物合并
    //海外版等级差
    int extlvlAdd = this->getMergeExtlvlAdd();
    //国内幻兽等级上限增加
    int maxlvlAdd = this->getMergeMaxlvlAdd();
    
    int maxlvl = (cfg->getMaxlvl() + maxlvlAdd) > (ownerlvl + cfg->getExtlvl() + extlvlAdd) ? (cfg->getExtlvl() + ownerlvl + extlvlAdd) : (cfg->getMaxlvl() + maxlvlAdd);
    totalexp += _exp;
    exp += _exp;
    while (lvl < maxlvl)
    {
        int needexp = PetCfg::getLvlTotalExp(cfg->getQua(), lvl + 1);
        if (totalexp < needexp)
        {
            break;
        }
        lvl++;
    }

    int lvltotalexp = PetCfg::getLvlTotalExp(cfg->getQua(), lvl);
    if (lvl == maxlvl)
    {
        exp = 0;
    }
    else
    {
        exp = totalexp - lvltotalexp;
        check_min(exp, 0);
    }
    totalexp = exp + lvltotalexp;

    calculateAttributes();
    saveExp();
}

void Pet::calculateAttributes()
{
    PetCfgDef* petcfg = PetCfg::getCfg(petmod);
    if(petcfg == NULL)
        return;
    
    setLvl(PetSave::lvl);
    calculateBaseAttributes(petcfg);
    calculateIncrAttributes(petcfg);
	calculateTotemAttributes(petcfg);
    calculateBattle();
    
    //设置专门的战斗属性
    mRealBattleProp.setAtk(getAtk());
    mRealBattleProp.setDef(getDef());
    mRealBattleProp.setCri(getCri());
    mRealBattleProp.setDodge(getDodge());
    mRealBattleProp.setHit(getHit());
    
    mRealBattleProp.setMaxHp(getMaxHp());
    mRealBattleProp.setHpIncr(getHpIncr());
    mRealBattleProp.setMoveSpeed(getMoveSpeed());
    
    saveBattle();
    makePropVerifyCode();
}

//计算基础属性
void Pet::calculateBaseAttributes(const PetCfgDef* petcfg)
{
    assert(petcfg);

    MonsterCfgDef * def = MonsterCfg::getCfg(petcfg->getMonsterTpltID());
    PetGrowthVal growVal = PetCfg::getGrowthVal(def->getGrowthType(), lvl);
    float result = 0;
    float rate = getGrowth() * petcfg->getGrowthRate();
    

    int mergelvl = this->getMergeLvl();
    PetMergeLvlItem lvlItem = PetMergeCfg::getPetMergeLvlItem(petcfg->mQua, mergelvl);

    // 攻击 = 基础攻击 + 成功值 * 成长系数 + 力量转攻击系数 * 力量 * (升阶加成）  + 智力转攻击系数 * 智力 * (升阶加成）
    result = batk + growVal.atk * rate;
    result += petcfg->getStretoatk() * mStre * ( 1 + lvlItem.incr_stre );
    result += petcfg->getIntetoatk() * mInte * ( 1 + lvlItem.incr_inte );
    setAtk(result);

    // 防御 = 基础防 + 成长防 + 耐力转防御系数 * 耐力 * (升阶加成）
    result = bdef +  growVal.def * rate;
    result += petcfg->getCapatodef() * mCapa * ( 1 + lvlItem.incr_capa);
    setDef(result);

    // 命中 = 基础命中 + 成长命中
    result = bhit +  growVal.hit * rate;
    setHit(result);

    // 闪避 类似命中
    result = bdodge +  growVal.dodge * rate;
    setDodge(result);

    //生命 = 基础生命 + 成长生命 + 体力转生命系数 * 体力 * (升阶加成）
    result = bhp +  growVal.hp * rate;
    result += petcfg->getPhystohp() * mPhys * ( 1 + lvlItem.incr_phys );
    setMaxHp(result);

    // 暴击只有基础
    //result = petcfg->getCri();
    //暴击改成可成长的
    result = petcfg->getCri() + growVal.cri * rate;
    setCri(result);

    // cal speed
    MonsterCfgDef* monsterDef = MonsterCfg::getCfg(petcfg->getMonsterTpltID());
    if (monsterDef == NULL) {
        setMoveSpeed(0);
    } else {
        setMoveSpeed(monsterDef->getWalkSpeed());
    }
}

// 外部加成,分两类
// 1、出战宠   A = A * ( 升阶 + 技能乘法系数 ) + 技能加法值
// 2、休息宠   A = A * 升阶
void Pet::calculateIncrAttributes(const PetCfgDef* petcfg)
{
    assert(petcfg);
    float starRatio = 1.0f;

    if(PET_RATIO_STRATEGY == enPetRatioStrategy_Star)//老升星已废
    {
        starRatio = PetCfg::getPropertyRatio(petcfg->getQua()-1 , starlvl);
    }
    if(PET_RATIO_STRATEGY == enPetRatioStrategy_Stage)
    {
        starRatio = PetCfg::getStagePropertyRatio(petcfg->getQua()-1 , getStage() );
    }
	
    if( kPetSleep == isactive )
    {
        mAtk *= starRatio;
        mMaxHp *= starRatio;
        mDef *= starRatio;
        mDodge *= starRatio;
        mHit *= starRatio;
        mCri *= starRatio;
        hp = mMaxHp;
        return;
    }

    if( kPetActive == isactive )
    {
        SkillIncrBattle incr;
        calGuildSkillPropertyAdditionForPet(owner,incr);

        mAtk =  mAtk * ( starRatio + incr.battlePropFactor.getAtkFactor() ) + incr.battleProp.getAtk();
        mMaxHp = mMaxHp * (starRatio + incr.battlePropFactor.getMaxHpFactor()) + incr.battleProp.getMaxHp();
        mDef = mDef * ( starRatio + incr.battlePropFactor.getDefFactor() ) + incr.battleProp.getDef();
        mDodge = mDodge* ( starRatio + incr.battlePropFactor.getDodgeFactor() ) + incr.battleProp.getDodge();
        mHit = mHit * ( starRatio + incr.battlePropFactor.getHitFactor() ) + incr.battleProp.getHit();
        mCri = mCri * ( starRatio + incr.battlePropFactor.getCriFactor() ) + incr.battleProp.getCri();
        mMoveSpeed = mMoveSpeed + incr.battleProp.getMoveSpeed();
        mHpIncr = incr.battleProp.mHpIncr;
        hp = mMaxHp;

        return ;
    }
}

void Pet::calculateTotemAttributes(const PetCfgDef* petcfg)
{
	BattleProp& batProp = STotemMon.getAddition(owner);
	
	mAtk += batProp.getAtk();
	mDef += batProp.getDef();
	mMaxHp += batProp.getMaxHp();
	mHit += batProp.getHit();
	mDodge += batProp.getDodge();
	mCri += batProp.getCri();
}

//计算并保存战斗力
void Pet::calculateBattle()
{
    int battle = calcBattleForce(this);
    
//    mBattle = mAtk + mDef *2 + mMaxHp /10 + int ((mHit-90) *10) +int(mDodge * 10);
    
    mBattle = 0.6f * battle;//弱化显示，与人物平衡
}

//星级转升阶
void Pet::convertStarToStage()
{
    if( getStarConvertedStage() == 0)
    {
        setStarConvertedStage(1);
        if( getStage() != 0)
            return;
        setStage( starlvl*10 );
        saveNewProperty();
    }
}

void Pet::getBattleProp(obj_petBattleProp &info)
{
    info.petid = petid;
    info.petmod = petmod;
    info.lvl = PetSave::lvl;
    
    info.atk = mRealBattleProp.getAtk();
    info.def = mRealBattleProp.getDef();
    info.maxHp = mRealBattleProp.getMaxHp();
    info.hp = mRealBattleProp.getMaxHp();
    info.dodge = mRealBattleProp.getDodge();
    info.hit = mRealBattleProp.getHit();
    info.cri = mRealBattleProp.getCri();
    
    info.speed = mRealBattleProp.getMoveSpeed();
    info.skills = getSkill().toArray();
    info.incrHP = getHpIncr();
}


void Pet::getPetInfo(obj_petinfo &info)
{
    info.petid = this->petid;
    info.pettypeid = petmod;
    info.exp = exp;
    info.lvl = lvl;
    info.isactive = isactive;
    info.batk = batk;
    info.bdef = bdef;
    info.bhp = bhp;
    info.bdodge = bdodge;
    info.bhit = bhit;
    info.skills = getSkill().toArray();
    
    info.capa = mCapa;
    info.inte = mInte;
    info.phy = mPhys;
    info.stre = mStre;
    
    info.star = star;
    info.starlvl = starlvl;

    info.growth = getGrowth();
    info.stage = getStage();
    
    //升阶信息
    info.mergelvl = getMergeLvl();
    info.mergecurrentexp = getMergeCurrentExp();
    info.extlvladd = getMergeExtlvlAdd();
    info.maxlvladd = getMergeMaxlvlAdd();
    info.growthadd = getMergeGrowthAdd();
    
    info.verifyCode = verifyPropCode;
    
    
    if( getLockSkill().exist(0) && getLockSkill().size() == 1)
    {
        xyerr("pet lock skill ， 出现x10错误");
        getLockSkill().clear();
        save();
    }
    
    //PetCfgDef* petcfg = PetCfg::getCfg(this->petmod);
    //vector<int> sks = petcfg->skills;
    //assert(petcfg);
    
    
    // 宠物已经拥有的技能
    vector<int> petskill = getSkill().toArray();

    PetCfgDef* petcfg = PetCfg::getCfg(this->petmod);
    if (!petcfg)
    {
        return ;
    }
    
    // 配置表中得宠物技能
    set<int> cfgskill(petcfg->skills.begin(), petcfg->skills.end());
    
    for (size_t i = 0; i < petskill.size(); ++i)
    {
        int reqSkillCfgId = calSkillCfgIdAndLevel(petskill[i]).first;
        for (int i = 0; i< petcfg->skills.size(); ++i)
        {
            int cfgid = calSkillCfgIdAndLevel( petcfg->skills[i] ).first;
            if( reqSkillCfgId ==  cfgid)
            {
                cfgskill.erase(petcfg->skills[i]);
                break;
            }
        }
    }
    
    info.lockskill.assign(cfgskill.begin(), cfgskill.end());
    info.isnew = getNewFlag();
    info.swallowlock = getSwallowLock();
    //info.lockskill = getSkill().toArray();
}

void Pet::makePropVerifyCode()
{
    verifyPropCode = calVerifyCode(&(this->mRealBattleProp));
}

bool Pet::checkSkillStudied(int skillid)
{
    IntArrayObj::const_iterator it = getSkill().const_begin();
    
    for (; it != getSkill().const_end(); ++it)
    {
        int skillIndex = (*it) / 100;
        if(skillid == skillIndex)
        {
            if ((*it) >= skillid) {
                return true;
            }
            return false;
        }
    }
    
    return false;
}

string Pet::petInfoToString()
{
    stringstream str;
    str
    << petid << ","
    << petmod << ","
    << lvl <<","
    << getGrowth() <<","
    << totalexp <<","
    
    << getStage() <<","
    << getMergeLvl() <<","
    << getMergeCurrentExp() <<","
    << getMergeTotalExp() <<","
    
    << mStre <<","
    << mInte <<","
    << mPhys <<","
    << mCapa;
    
    vector<int> skills;
    getSortedSkills(skills);
    
    for (int i = 0; i < skills.size(); i++) {
        str<<",";
        str<<skills[i];
    }
    
    return str.str();
}

//int Pet::getSkillId(int index)
//{
//    if (index >= getSkill().size()) {
//        return 0;
//    }
//    
//    return getSkill().getData(index);
//}

void Pet::getSortedSkills(vector<int>& skills)
{
    skills.clear();
    
    PetCfgDef* petcfg = PetCfg::getCfg(petmod);
    if (petcfg == NULL) {
        return;
    }
    
    int skillSize = petcfg->skills.size();
    
    for (int i = 0; i < skillSize; i++) {

        int skillBefore = getPetSkillStudied(petcfg->skills[i], this);

        skills.push_back(skillBefore);
    }
}

void Pet::setSpiritSpeakId(int spiritspeakId)
{
    mSpiritSpeakId = spiritspeakId;
}

int Pet::getSpiritSpeakId()
{
    return mSpiritSpeakId;
}

void PetAssist::save()
{
    string str;
    for (int i = 0; i < mPets.size(); i++)
    {
        if (mPets[i] != NULL)
        {
            str.append(strFormat("%d;", mPets[i]->petid));
        }
        else
        {
            str.append("0;");
        }
    }

    mRole->saveProp(GetRolePropName(eRolePropAssistPetList), str.c_str());
}

void PetAssist::load()
{
    string assistpetstr = mRole->loadProp(GetRolePropName(eRolePropAssistPetList));

    StringTokenizer token(assistpetstr, ";");
    vector<Pet*> pets = mRole->mPetMgr->getPets();
    for (int i = 0; i < token.count(); i++)
    {
        Pet* pet = NULL;
        int petid = Utils::safe_atoi(token[i].c_str(), 0);
        if (petid == 0)
        {
            mPets.push_back(NULL);
            continue;
        }

        for (int j = 0; j < pets.size(); j++)
        {
            if (petid == pets[j]->petid)
            {
                pet = pets[j];
                break;
            }
        }

        mPets.push_back(pet);
    }
}

Pet* PetMgr::create(PetSave& info)
{
    int petid = genPetId();
    Pet* pet = new Pet(petid);
    pet->active(petid);

    pet->petmod = info.petmod;

    pet->lvl = info.lvl;
    pet->exp = info.exp;
    pet->totalexp = info.totalexp;
    pet->isactive = info.isactive;
    pet->owner = info.owner;
    pet->getSkill() = info.oldskills;
    pet->getLockSkill() =  info.oldlockSkills ;

    pet->batk = info.batk;
    pet->bdef = info.bdef;
    pet->bhp = info.bhp;
    pet->bdodge = info.bdodge;
    pet->bhit = info.bhit;

    pet->mStre = info.mStre;
    pet->mPhys = info.mPhys;
    pet->mInte = info.mInte;
    pet->mCapa = info.mCapa;


    pet->star  = info.star;
    pet->starlvl  = info.starlvl;

    //1.5版本后直接是已转的
    pet->setStage( 0 );
    pet->setStarConvertedStage(1);
    pet->setGrowth( info.oldGrowth );
    pet->setNewFlag(true);
    pet->setSwallowLock(false);
    
    pet->calculateAttributes();
    pet->save();

    return pet;
}

void PetAssist::onCalcProperty(BattleProp &battleprop, BaseProp &baseprop, int position)
{
    if (position <= 0 || position > mPets.size()) {
        return;
    }
    
    Pet* pet = mPets[position - 1];
    if (pet == NULL)
    {
        return;
    }
    
    PetCfgDef* petcfg = PetCfg::getCfg(pet->petmod);
    if (petcfg == NULL)
    {
        return;
    }
    
    int petqua = petcfg->getQua();
    float ratio = PetAssistCfg::getStagePropRatio(position, petqua, pet->getStage() );
    
    vector<PetAssistAddPropDef*> propNames = PetAssistCfg::getPetAssistAddPropDefVecByPos(position);
    
    for (int i = 0; i < propNames.size(); i++) {
        
        PetAssistAddPropDef* propDef = propNames[i];
        
        if (propDef == NULL || propDef->prop.empty()) {
            continue;
        }
        
        if (propDef->prop == "atk")
        {
            battleprop.setAtk(battleprop.getAtk() + pet->getAtk() * ratio);
        }
        else if (propDef->prop == "def")
        {
            battleprop.setDef(battleprop.getDef() + pet->getDef() * ratio);
        }
        else if (propDef->prop == "hit")
        {
            battleprop.setHit(battleprop.getHit() + pet->getHit() * ratio);
        }
        else if (propDef->prop == "dodge")
        {
            battleprop.setDodge(battleprop.getDodge() + pet->getDodge() * ratio);
        }
        else if (propDef->prop == "cri")
        {
            battleprop.setCri(battleprop.getCri() + pet->getCri() * ratio);
        }
        else if (propDef->prop == "hp")
        {
            battleprop.setMaxHp(battleprop.getMaxHp() + pet->getMaxHp() * ratio);
        }
        
    }
}

void PetAssist::calcProperty(BattleProp& battleprop, BaseProp& baseprop)
{
    for (int i = 0; i < mPets.size(); i++)
    {
        onCalcProperty(battleprop, baseprop, i + 1);
    }
}


int PetMgr::genPetId()
{
    RedisResult result(redisCmd("incr %s", Pet::sGenIdKey));
    return result.readInt();
}

Pet* PetMgr::loadPet(int petid)
{
    Pet* pet = new Pet(petid);
    if (!pet->load(get_DbContext()))
    {
        delete pet;
        pet = NULL;
    }

    return pet;
}

Pet*  PetMgr::getPet(int petid)
{
    for (int i = 0; i < mPets.size(); i++)
    {
        Pet* pet = mPets[i];
        if (pet && pet->petid == petid)
        {
            return mPets[i];
        }
    }

    return NULL;
}

void PetMgr::removePet(int petid)
{
    for (vector<Pet*>::iterator iter = mPets.begin(); iter != mPets.end(); iter++)
    {
        Pet* pet = *iter;
        if (pet && pet->petid == petid)
        {
            delete pet;
            mPets.erase(iter);
            break;
        }
    }
}


void PetMgr::addPet(Pet* pet)
{
    mPets.push_back(pet);
}


void PetMgr::unload()
{
    mActivePet = NULL;
    for (int i = 0; i < mPets.size(); i++)
    {
        delete mPets[i];
    }
    mPets.clear();
}

void PetMgr::load()
{
    unload();

    bool shouldSavePetList = false;

    string petlist = mRole->loadProp(GetRolePropName(eRolePropPetList));
    vector<string> petids = StrSpilt(petlist, ";,");
    for (int i = 0; i < petids.size(); i++)
    {
        int petid = atoi(petids[i].c_str());
        if (petid > 0)
        {
            Pet* pet = PetMgr::loadPet(petid);
            if (pet == NULL)
            {
                shouldSavePetList = true;
                continue;
            }
            mPets.push_back(pet);
            if (pet->isactive)
            {
                mActivePet = pet;
                mRole->setActivePetId(pet->petid);
                
                if (!mRole->mIsRobot) {
                    mRole->saveProp(GetRolePropName(eRoleActivePetId), mActivePet->petid);
                    //更新宠物排行榜
                    mRole->UploadPetPaihangData(eUpdate, pet->petid, pet->mBattle);
                }
            }
        }
    }
    if (shouldSavePetList)
    {
        save();
    }

}

void PetMgr::save()
{
    string petstr;
    for (int i = 0; i < mPets.size(); i++)
    {
        petstr.append(strFormat("%d,", mPets[i]->petid));
    }

    mRole->saveProp(GetRolePropName(eRolePropPetList), petstr.c_str());
}


void PetMgr::delPetFromDB(int petid){
    doRedisCmd("del pet:%d", petid);
}

void PetMgr::delPetsFromDB(const vector<int>& petids){
    RedisArgs args;
    RedisHelper::beginDelKeys(args);
    for (int i = 0; i < petids.size(); i++){
        RedisHelper::appendKeyname(args, strFormat("pet:%d", petids[i]).c_str());
    }
    RedisHelper::commitDelKeys(get_DbContext(), args);
}

bool createPetProp(int ownerId, int petMod, PetSave& info)
{
    PetCfgDef* cfg = PetCfg::getCfg(petMod);
    if (cfg == NULL)
    {
        return false;
    }
    
    info.owner = ownerId;
    info.isactive = 0;
    info.totalexp = 0;
    info.exp = 0;
    info.lvl = 1;
    info.petmod = petMod;
    info.oldGrowth = range_rand(cfg->growth[0], cfg->growth[1]);
    info.batk = range_rand(cfg->batk[0], cfg->batk[1]);
    info.bdef = range_rand(cfg->bdef[0], cfg->bdef[1]);
    info.bhp = range_rand(cfg->bhp[0], cfg->bhp[1]);
    info.bdodge = range_randf(cfg->bdodge[0], cfg->bdodge[1]);
    info.bhit = range_randf(cfg->bhit[0], cfg->bhit[1]);
    
    info.mCapa = cfg->getCapa();
    info.mInte = cfg->getInte();
    info.mPhys = cfg->getPhys();
    info.mStre = cfg->getStre();
    
    info.star = 0;
    info.starlvl = 0;
    
    if ( (rand()%100) < (cfg->getExtraProp() * 100)) {
        switch (rand()%4) {
            case 0:
                info.mCapa += cfg->getExtraPropPoint();
                break;
            case 1:
                info.mPhys += cfg->getExtraPropPoint();
                break;
            case 2:
                info.mInte += cfg->getExtraPropPoint();
                break;
            case 3:
                info.mStre += cfg->getExtraPropPoint();
                break;
            default:
                break;
        }
    }
    
    for (int i = 0; i < cfg->skills.size(); i++)
    {
        float randnum = range_randf(0.0, 1.0);
        if (randnum > cfg->skillprops[i])
            continue;
        
        bool isLock = range_randf(0.0, 1.0) < cfg->skilllockprops[i];
        
        if(isLock)
            info.oldlockSkills.push_back(cfg->skills[i]);
        else
            info.oldskills.push_back(cfg->skills[i]);
        
    }
    
    return true;
}

Pet* producePet(Role* role, int petmod)
{
    PetCfgDef* cfg = PetCfg::getCfg(petmod);
    if (cfg == NULL)
    {
        return NULL;
    }
    
    PetSave info;
    
    if (!createPetProp(role->getInstID(), petmod, info)) {
        return NULL;
    }
    
    Pet* pet = PetMgr::create(info);
    if (pet == NULL)
    {
        return NULL;
    }
    
    role->mPetMgr->addPet(pet);
    role->mPetMgr->save();
    
    return pet;
}

// 宠物是否已学某技能，学过的返回前一个技能id
int getPetSkillStudied(int skillid , Pet* pet)
{
    if( NULL == pet )
        return 0;
    
    int skillidcfg = calSkillCfgIdAndLevel( skillid ).first;
    
    IntArrayObj::const_iterator it = pet->getSkill().const_begin();
    
    for (; it != pet->getSkill().const_end(); ++it)
    {
        int studiedSkillId = calSkillCfgIdAndLevel( *it ).first;
        
        if(skillidcfg == studiedSkillId)
        {
            return *it;
        }
    }
    
    return 0;
}

//计算是否升星成功
bool PetCalStarUpgrade(Pet* mainPet, const vector<Pet*>& subPets, bool & bStarUpgradeSuccess, int& totalAbsorbStar)
{
    try{
        
        vector<int>::const_iterator iter,end;
        
        PetCfgDef* pMainPetCfg = PetCfg::getCfg(mainPet->petmod);
        
        //满级不能升了
        if( (mainPet->starlvl + 1) >= pMainPetCfg->starUpStep.size()){
            bStarUpgradeSuccess = false;
            return true;
        }
        
        //计算升星成功概率
        float successRate = 0;
        
        for (int i = 0; i < subPets.size(); i++) {
            
            Pet* subpet = subPets[i];
            PetCfgDef* petcfg = PetCfg::getCfg(subpet->petmod);
            int tmp = petcfg->getStarBorn() + subpet->star;
            float factor = PetCfg::getPercentConversion(petcfg->getQua() - 1, subpet->starlvl);
            totalAbsorbStar += factor * tmp;
        }
        
        int mainPetStar = mainPet->star + pMainPetCfg->getStarBorn() ;                  //主幻兽自身价值量
        int starUpgradeValue = pMainPetCfg->starUpStep[mainPet->starlvl];          //主幻兽本星级升星价值量
        int starUpgradeValueNext = pMainPetCfg->starUpStep[mainPet->starlvl + 1];  //主幻兽下一星级升星价值量
        float starUpSuccRatio = pMainPetCfg->starUpSuccRatio[mainPet->starlvl];    //升星基础成功率
        
        
        //升星成功率=（被吞噬幻兽总自身价值量+主幻兽自身价值量-主幻兽本星级升星价值量）/主幻兽下一星级升星价值量+主幻兽本星级升星基础成功率
        
        successRate = 1.0f * (totalAbsorbStar + mainPetStar - starUpgradeValue) / starUpgradeValueNext  + starUpSuccRatio;
        
        if( successRate < 0)
            bStarUpgradeSuccess = false;
        else{
            float randnum = range_randf(0.0, 1.0);
            
            if(randnum < successRate)
                bStarUpgradeSuccess = true;
            else
                bStarUpgradeSuccess = false;
        }
        
        return true;
        
    }catch(...){
        return false;
    }
}

bool PetStageUpgrade(Pet* mainPet, const vector<Pet*>& subPets)
{
    PetCfgDef* pMainCfg = PetCfg::getCfg(mainPet->petmod);
    
    int absorbStage = 0;
    for (int i =0; i< subPets.size(); ++i)
    {
        Pet* subpet = subPets[i];
        PetCfgDef* petcfg = PetCfg::getCfg(subpet->petmod);
        
        int stage = subpet->getStage();
        absorbStage += petcfg->getStageBorn() ;
        
        if( stage > 0 )//0阶没有继承的，下标为0的继承是指阶为1阶时的可继承部分
            absorbStage += petcfg->stageInherit[stage-1];
    }
    
    bool upgrade = false;
    while(true)
    {
        int stage = mainPet->getStage();
        if(stage>= pMainCfg->stageUpStep.size())//满级了不能升
            break;
        
        int step = pMainCfg->stageUpStep[stage];
        
        if(absorbStage >=  step)
        {
            absorbStage -= step;
            mainPet->setStage(++stage);
            upgrade = true;
            continue;
        }
        
        float rate = 1.0f * absorbStage / step;
        float randnum = range_randf(0.0, 1.0);
        if(randnum < rate)
        {
            mainPet->setStage(++stage);
            upgrade = true;
        }
        
        break;
    }
    
    return upgrade;
}

bool PetAbsorb(Pet* mainPet, Pet* subPet)
{
    PetCfgDef* cfg = PetCfg::getCfg(subPet->petmod);
    PetCfgDef* mainPetcfg = PetCfg::getCfg(mainPet->petmod);
    
    if (cfg == NULL || mainPetcfg == NULL) {
        return false;
    }
    
    float absorbedStreProp = 0;
    float absorbedInteProp = 0;
    float absorbedPhysProp = 0;
    float absorbedCapaProp = 0;
    
    absorbedStreProp += PetCfg::calcAbsorbedPropPointsNum(subPet->mStre, cfg->ownPoints);
    absorbedInteProp += PetCfg::calcAbsorbedPropPointsNum(subPet->mInte, cfg->ownPoints);
    absorbedPhysProp += PetCfg::calcAbsorbedPropPointsNum(subPet->mPhys, cfg->ownPoints);
    absorbedCapaProp += PetCfg::calcAbsorbedPropPointsNum(subPet->mCapa, cfg->ownPoints);
    
    //计算获得的属性点
    int absorbedStre = PetCfg::calcAbsorbedPropPoints(absorbedStreProp, mainPet->mStre, mainPetcfg->lvlPoints);
    int absorbedInte = PetCfg::calcAbsorbedPropPoints(absorbedInteProp, mainPet->mInte, mainPetcfg->lvlPoints);
    int absorbedPhys = PetCfg::calcAbsorbedPropPoints(absorbedPhysProp, mainPet->mPhys, mainPetcfg->lvlPoints);
    int absorbedCapa = PetCfg::calcAbsorbedPropPoints(absorbedCapaProp, mainPet->mCapa, mainPetcfg->lvlPoints);
    
    mainPet->mStre += absorbedStre;
    mainPet->mInte += absorbedInte;
    mainPet->mPhys += absorbedPhys;
    mainPet->mCapa += absorbedCapa;
    check_max(mainPet->mPhys, mainPetcfg->getMaxbaseprop());
    check_max(mainPet->mCapa, mainPetcfg->getMaxbaseprop());
    check_max(mainPet->mInte, mainPetcfg->getMaxbaseprop());
    check_max(mainPet->mStre, mainPetcfg->getMaxbaseprop());
    
    return true;
}

void notifyNewPet(Role* role, Pet* newPet)
{
    if (newPet == NULL) {
        return;
    }
    
    notify_get_pet notify;
    
    obj_petinfo info;
    newPet->getPetInfo(info);
    notify.petsinfo.push_back(info);
    
    sendNetPacket(role->getSessionId(), &notify);
}

int checkPetEvolutionCondition(Pet* pet, PetEvolutionMaterial* condition)
{
//    return 0;
    if (pet->getStage() < condition->starlvl) {
        return CE_PET_EVOLUTION_MATERIAL_STARLVL_TOO_LOW;
    }
    
    if (pet->lvl < condition->lvl) {
        return CE_PET_EVOLUTION_MATERIAL_LVL_TOO_LOW;
    }
    
    if (pet->getMergeLvl() < condition->stage) {
        return CE_PET_EVOLUTION_MATERIAL_STAGE_TOO_LOW;
    }
    
    int totalProp = pet->mStre + pet->mInte + pet->mPhys + pet->mCapa;
    if (totalProp < condition->prop) {
        return CE_PET_EVOLUTION_MATERIAL_PROP_TOO_LOW;
    }
    
    vector<int> myskills;
    myskills.clear();
    
    pet->getSortedSkills(myskills);
    
    //检查技能条件
    int conditionCount = condition->skills.size();
    int mySkillCount = myskills.size();
    
    for (int i = 0; i < conditionCount && i < mySkillCount; i++) {
        int skilllvl = condition->skills[i];

        int myskillvlv = calSkillCfgIdAndLevel(myskills[i]).second;
        
        if (myskillvlv < skilllvl) {
            return CE_PET_EVOLUTION_MATERIAL_SKILLLVL_TOO_LOW;
        }
    }

    return CE_OK;
}

int petModId2petEvolutionTypeid(int petMod)
{
    PetCfgDef* cfg = PetCfg::getCfg(petMod);
    
    if (cfg) {
        int evolutionTypeid = cfg->getMonsterTpltID() * 10 + cfg->getQua();
        return evolutionTypeid;
    }
    
    return 0;
}

int calSkillNeedPages(int skillid, int beginlvl)
{
    if (skillid <= 0) {
        return 0;
    }
    
    int skillLvl = skillid % 100;
    int needPages  = 0;
    
    if (skillLvl <= beginlvl) {
        return 0;
    }
    
    for (int i = 0; i < (skillLvl - beginlvl); i++) {
        
        SkillCfgDef* skillCfg = SkillCfg::getCfg(skillid - i);
        if (skillCfg) {
            needPages += skillCfg->needPages;
        }
    }
    
    return needPages;
}

int calSkillNeedGold(int skillid, int beginlvl)
{
    if (skillid <= 0) {
        return 0;
    }
    
    int skillLvl = skillid%100;
    int needGold = 0;
    
    if (skillLvl <= beginlvl) {
        return 0;
    }
    
    for (int i = 0; i < (skillLvl - beginlvl); i++) {
        
        SkillCfgDef* skillCfg = SkillCfg::getCfg(skillid - i);
        if (skillCfg) {
            needGold += skillCfg->needGold;
        }
    }
    
    return needGold;
}

int calMergeExpByMergeLvl(int quality, int mergelvl)
{
    int needMergeExp = 0;
    
    if (mergelvl <= 0) {
        return 0;
    }
    
    for (int i = 1; i <= mergelvl; i++) {
        needMergeExp += PetMergeCfg::getUplvlExp(quality, i);
    }
    
    return needMergeExp;
}

void calPetEvolutionRestitutionItem(Pet* myPet, PetEvolutionMaterial* condition, int& goldRestitution, ItemArray& items)
{
    if (!condition->restitution) {
        return;
    }
    
    ItemArray tempItems;
    
    //补偿经验幻兽蛋
    int mergeExpDiff = myPet->getMergeTotalExp() - calMergeExpByMergeLvl(condition->quality, condition->stage);
    
    check_min(mergeExpDiff, 0);
    
    ItemGroup itemEgg;
    PetEvolutionRestitutionItem* eggCfg = PetEvolutionRestitutionCfg::getPetEggCfg(condition->quality);
    
    if (eggCfg) {
        
        itemEgg.item = eggCfg->itemId;
        itemEgg.count = mergeExpDiff * PetEvolutionRestitutionCfg::getMergeExpPercent() / eggCfg->refValue;
        
    }
    
    if (itemEgg.count) {
        
        tempItems.push_back(itemEgg);
        
    }
    
    //计算补偿的金币及技能书
    int goldDiff = 0;
    
    vector<int> mySkills;
    mySkills.clear();
    
    myPet->getSortedSkills(mySkills);
    
    PetEvolutionRestitutionItem* skillBooksCfg = PetEvolutionRestitutionCfg::getSkillBookCfg(condition->quality);
    
    PetEvolutionRestitutionItem* common_skillBooksCfg = PetEvolutionRestitutionCfg::getSkillBookCfg(0);

    for (int i = 0; i < mySkills.size(); i++) {
        
        SkillCfgDef* skillcfg = SkillCfg::getCfg(mySkills[i]);
        
        if (skillcfg == NULL) {
            continue;
        }
        
        int beginLvl = 0;
        
        if (i < condition->skills.size()) {
            beginLvl = condition->skills[i];
        }
        
        check_min(beginLvl, 1);
        
        goldDiff += calSkillNeedGold(mySkills[i], beginLvl);
        
        int pagesDiff = calSkillNeedPages(mySkills[i], beginLvl);
        
        ItemGroup skillbook;
        
        if (skillcfg->getPetSkillType() && common_skillBooksCfg) {
            
            skillbook.item = common_skillBooksCfg->itemId;
            skillbook.count = pagesDiff * PetEvolutionRestitutionCfg::getPagesPercent() / common_skillBooksCfg->refValue;
        }
        else
        {
            if (skillBooksCfg) {
                skillbook.item = skillBooksCfg->itemId;
                skillbook.count = pagesDiff * PetEvolutionRestitutionCfg::getPagesPercent() / skillBooksCfg->refValue;
            }
        }
        
        if (skillbook.count) {
            tempItems.push_back(skillbook);
        }
    }
    
    goldRestitution += goldDiff * PetEvolutionRestitutionCfg::getGoldPercent();
    
    mergeItems(tempItems, items);
}

void calPetEvolutionResidueStage(int targetPetMod, vector<Pet*> subPets, ItemArray& items)
{
    PetCfgDef* targetCfg = PetCfg::getCfg(targetPetMod);
    
    if (targetCfg == NULL) {
        return;
    }
    
    int absorbStage = 0;
    for (int i = 0; i< subPets.size(); ++i)
    {
        Pet* subpet = subPets[i];
        PetCfgDef* petcfg = PetCfg::getCfg(subpet->petmod);
        if(petcfg == NULL)
        {
            continue;
        }
        int stage = subpet->getStage();
        absorbStage += petcfg->getStageBorn() ;
        
        if( stage > 0 )//0阶没有继承的，下标为0的继承是指阶为1阶时的可继承部分
            absorbStage += petcfg->stageInherit[stage-1];
    }
    
    if (absorbStage > targetCfg->totalStageUpStep) {
        
        ItemGroup itemEgg;
        ItemArray tempItems;
        PetEvolutionRestitutionItem* eggCfg = PetEvolutionRestitutionCfg::getStagePetEggCfg(targetCfg->mQua);
        
        if (eggCfg) {
            
            itemEgg.item = eggCfg->itemId;
            itemEgg.count = (absorbStage - targetCfg->totalStageUpStep) * PetEvolutionRestitutionCfg::getMergeExpPercent() / eggCfg->refValue;
            
            tempItems.push_back(itemEgg);
            
            mergeItems(tempItems, items);
        }
        
    }
}

void sendPetEvolutionRestitutionMail(Role* role, RewardStruct& reward, ItemArray& items)
{
    vector<string> attaches;
    vector<string> attachesDescription;
    
    string attachstr = "";
    string attachesDescriptionStr = "";
    
    reward2MailFormat(reward, items, &attachesDescription, &attaches);
    
    attachstr = StrJoin(attaches.begin(), attaches.end(), ";");
    attachesDescriptionStr = StrJoin(attachesDescription.begin(), attachesDescription.end(), ",");
    
    MailFormat *mailformat = MailCfg::getCfg("pet_evolution_restitution");
    
    string mail_content = "";
    string mail_tile = "";
    if (mailformat) {
        mail_tile = mailformat->title;
        mail_content = mailformat->content;
    }
    
    
    find_and_replace(mail_content, 1, attachesDescriptionStr.c_str());
    
    bool ret = sendMail(0,
                        mailformat->sendername.c_str(),
                        role->getRolename().c_str(),
                        mail_tile.c_str(),
                        mail_content.c_str(),
                        attachstr.c_str(),
                        Utils::itoa(role->getInstID()).c_str());
    
    if( false ==  ret )
    {
        StoreOfflineItem(role->getInstID(), attachstr.c_str());
    }
}

//---------------------------------------------------------
//幻兽大冒险,创建租用机器幻兽的函数

int generatePetAdventureRobotPetId(int sceneId, int petmod)
{
    int sceneType = (sceneId / 10000) % 10;
    int sceneIndex = sceneId % 100;
        
    stringstream szRobotPetId;
    if (sceneIndex < 10) {
        szRobotPetId << petmod << sceneType << 0 << sceneIndex;
    } else {
        szRobotPetId << petmod << sceneType << sceneIndex;

    }

    return Utils::safe_atoi(szRobotPetId.str().c_str());
}


bool initPetAdventureRobotPetProp(int sceneId, int petmod, Pet& pet)
{
    //幻兽大冒险机器幻兽配置属性
    PetAdventureRobotPetsForRentCfgDef* cfg = PetAdventureRobotPetsForRentCfg::getPetAdventurePetRobotCfgDef(sceneId);
    if (cfg == NULL)
    {
        return false;
    }
    
    //原幻兽模板配置属性
    PetCfgDef* petcfg = PetCfg::getCfg(petmod);
    if(petcfg == NULL) {
        return false;
    }
    
    MonsterCfgDef* monsterDef = MonsterCfg::getCfg(petcfg->getMonsterTpltID());
    if (monsterDef == NULL) {
        return false;
    }
    
    int robotPetId = generatePetAdventureRobotPetId(sceneId, petmod);
    pet.petid      = robotPetId;
    pet.owner      = 0;
    pet.isactive   = 0;
    pet.totalexp   = 0;
    pet.exp        = 0;
    pet.petmod     = petmod;
    
    pet.lvl        = cfg->batprop.getLvl();
    pet.batk       = cfg->batprop.getAtk();
    pet.bdef       = cfg->batprop.getDef();
    pet.bhp        = cfg->batprop.getMaxHp();
    pet.bdodge     = cfg->batprop.getDodge();
    pet.bhit       = cfg->batprop.getHit();
    
    pet.mCapa      = petcfg->getCapa();
    pet.mInte      = petcfg->getInte();
    pet.mPhys      = petcfg->getPhys();
    pet.mStre      = petcfg->getStre();
    
    
    pet.star       = 0;
    pet.starlvl    = 0;
    
    pet.setStage(0);
    pet.setStarConvertedStage(1);
    pet.setGrowth(cfg->growth);
    pet.setNewFlag(true);
    //pet.setSwallowLock(false);机器幻兽不需要吞噬锁
    vector<int> robotPetskills;
    for (int i = 0; i < petcfg->skills.size(); ++i)
    {
        
        SkillCfgDef* skillcfg = SkillCfg::getCfg(petcfg->skills[i]);
        if ( skillcfg == NULL && skillcfg->needPages <= 0 ) {
            continue;
        }
        
        int quaMaxLvl = 0;
        if( skillcfg->getPetSkillType() == ePetSkillType_Common)
        {
            quaMaxLvl = PetCfg::getMaxCommmonSkills(petcfg->getQua());
            
        }else{  // special skill
            quaMaxLvl = PetCfg::getMaxSpecialSkills(petcfg->getQua());
        }
        
        if (quaMaxLvl > skillcfg->getMaxLvl()) {
            quaMaxLvl = skillcfg->getMaxLvl();
        }
        
        std::pair<int,int> petCfgSkill = calSkillCfgIdAndLevel(petcfg->skills[i]);
        int petCfgSkillId = petCfgSkill.first;
        int petCfgSkillLevel = petCfgSkill.second;
        
        if ( (i+1) <= cfg->skillsLvl.size()) {
            petCfgSkillLevel = cfg->skillsLvl[i];
        }
        else {
            petCfgSkillLevel = 1;
        }
        
        if (petCfgSkillLevel > quaMaxLvl) {
            petCfgSkillLevel = quaMaxLvl;
        }
        
        int robotPetSkill = petCfgSkillId + petCfgSkillLevel;
        
        robotPetskills.push_back(robotPetSkill);
    }
    
    pet.getSkill() = robotPetskills;
    
    
    PetGrowthVal growVal = PetCfg::getGrowthVal(monsterDef->getGrowthType(), pet.lvl);
    float rate = pet.getGrowth() * petcfg->getGrowthRate();
    
    pet.setLvl(pet.lvl);
    pet.setAtk      (pet.batk   + growVal.atk   * rate);
    pet.setDef      (pet.bdef   + growVal.def   * rate);
    pet.setHit      (pet.bhit   + growVal.hit   * rate);
    pet.setDodge    (pet.bdodge + growVal.dodge * rate);
    pet.setMaxHp    (pet.bhp    + growVal.hp    * rate);
    pet.setCri      (petcfg->getCri() + growVal.cri * rate);
    pet.setMoveSpeed(monsterDef->getWalkSpeed());
    pet.setHpIncr   (0);
    
    pet.calculateBattle();
    
    //设置专门的战斗属性
    pet.mRealBattleProp.setAtk(pet.getAtk());
    pet.mRealBattleProp.setDef(pet.getDef());
    pet.mRealBattleProp.setHit(pet.getHit());
    pet.mRealBattleProp.setDodge(pet.getDodge());
    pet.mRealBattleProp.setMaxHp(pet.getMaxHp());
    pet.mRealBattleProp.setCri(pet.getCri());
    pet.mRealBattleProp.setMoveSpeed(pet.getMoveSpeed());
    pet.mRealBattleProp.setHpIncr(pet.getHpIncr());
    
    
    pet.makePropVerifyCode();
    return true;
    
}
//---------------------------------------------------------
