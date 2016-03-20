//
//  Wardrobe.cpp
//  GameSrv
//
//  Created by xinyou on 14-5-14.
//
//

#include "Wardrobe.h"
#include "Role.h"
#include "redis_helper.h"
#include "DataCfg.h"
#include "EnumDef.h"
#include <errno.h>

Wardrobe::Wardrobe()
{
    mOwnerId = 0;
}

Wardrobe::~Wardrobe()
{
    
}


bool Wardrobe::load()
{
    if (mOwnerId == 0)
    {
        return false;
    }
    
    WardrobeData::load(mOwnerId);
    if (getLvl() == 0)
    {
        setLvl(1);
    }
    
    WardrobeFashionList* fashionLists[] = {&getBodyFashion(), &getWeaponFashion(),&getHomeFashion()};
    int fashionTypes[] = {eWholeBody, eWeapon,eCityBody};
    for (int i = 0; i < sizeof(fashionLists) / sizeof( WardrobeFashionList*); i++)
    {
        WardrobeFashionList* fashionList = fashionLists[i];
        int fashionType = fashionTypes[i];
        if (fashionList == NULL)
        {
            continue;
        }
        
        for (WardrobeFashionList::Iterator iter = fashionList->begin();
             iter != fashionList->end(); iter++)
        {
            WardrobeFashion* fashion = *iter;
            fashion->mType = fashionType;
            mAllFashions[fashion->mId] = fashion;
        }
    }

    //checkExpiredFashions(NULL);
    
    calProperty();
    
    return true;
}

Wardrobe* Wardrobe::create(Role* role)
{
    Wardrobe* wardrobe = new Wardrobe;
    wardrobe->mOwner = role;
    wardrobe->mOwnerId = role->getInstID();
    return wardrobe;
}

bool Wardrobe::hasFashion(int fashionId)
{
    map<int, WardrobeFashion*>::iterator iter = mAllFashions.find(fashionId);
    return iter != mAllFashions.end();
}

WardrobeFashion* Wardrobe::getFashion(int fashionId)
{
    map<int, WardrobeFashion*>::iterator iter = mAllFashions.find(fashionId);
    if (iter == mAllFashions.end())
    {
        return NULL;
    }
    
    return iter->second;
}

void Wardrobe::addExp(int exp)
{
    int curLvl = getLvl();
    int curExp = getExp();
    
    int maxLvl = WardrobeCfg::getMaxLvl();
    
    curExp += exp;
    while (curLvl < maxLvl) {
        WardrobeCfgDef* def = WardrobeCfg::getWardrobeCfg(curLvl);
        int needExp = def->getNeedexp();
        if (needExp > curExp)
        {
            break;
        }
        
        curLvl++;
        curExp -= needExp;
    }
    
    if (curLvl >= maxLvl)
    {
        curExp = 0;
    }
    
    if (curLvl > getLvl())
    {
        calProperty();
    }
    
    setExp(curExp);
    setLvl(curLvl);
    
    save();
}

int parseFashionType(const string& typeStr)
{
    if (typeStr == "weapon")
    {
        return eWeapon;
    }
    
    if (typeStr == "whole_body")
    {
        return eWholeBody;
    }
    if (typeStr == "city_body") {
        return eCityBody;
    }
    
    return eBodyPartNone;
}


bool Wardrobe::checkExpiredFashions(vector<int>* fashions)
{
    bool shouldSave = false;
    
    int nowTime = time(NULL);
    
    map<int, WardrobeFashion*>::iterator iter = mAllFashions.begin();
    
    for (;iter != mAllFashions.end(); iter++)
    {
        WardrobeFashion* fashion = iter->second;
        
        if (fashion->mExpired)
        {
            continue;
        }
        
        if (fashion->mExpireTime == -1)
        {
            continue;
        }
        
        if (fashion->mExpireTime < nowTime)
        {
            fashion->mExpired = true;
            if (fashions)
            {
                fashions->push_back(fashion->mId);
            }
            shouldSave = true;
            
            if (mOwner && fashion->mId == mOwner->getBodyFashion())
            {
                mOwner->setBodyFashion(0);
                shouldSave = true;
            }
            
            if (mOwner && fashion->mId == mOwner->getWeaponFashion())
            {
                mOwner->setWeaponFashion(0);
                shouldSave = true;
            }
            if (mOwner && fashion->mId == mOwner->getHomeFashion())
            {
                mOwner->setHomeFashion(0);
                shouldSave = true;
            }
            
            switch (fashion->mType) {
                case eWeapon:
                    getWeaponFashion().dirty(true);
                    break;
                case eWholeBody:
                    getBodyFashion().dirty(true);
                    break;
                case eCityBody:
                    getHomeFashion().dirty(true);
                    break;
                default:
                    break;
            }
        }
    }
    
    if (shouldSave)
    {
        mOwner->save();
        calProperty();
        save();
    }
    
    return shouldSave;
}

bool Wardrobe::addFashion(ItemCfgDef* fashionCfg)
{
    int exp = fashionCfg->ReadInt("offerexp");
    int lastTime = fashionCfg->ReadInt("duration");
    
    int fashionId = fashionCfg->ReadInt("template");
    ItemCfgDef* templateCfg = ItemCfg::getCfg(fashionId);
    string bodyPart = templateCfg->ReadStr("body_part");
    int type = parseFashionType(bodyPart);
    
    WardrobeFashion* fashion = getFashion(fashionId);
    WardrobeFashionList* fashionList = (type == eWholeBody) ?&getBodyFashion() : (type == eWeapon) ?&getWeaponFashion():&getHomeFashion();
    if (fashion == NULL)
    {
        int expireTime = lastTime == -1 ? -1 : lastTime + time(NULL);
        fashion = fashionList->addFashion(fashionId, expireTime);
        fashion->mType = type;
        mAllFashions[fashionId] = fashion;
    }
    else
    {
        int expireTime;
        if (lastTime == -1 || fashion->mExpireTime == -1)
        {
            expireTime = -1;
        }
        else
        {
            time_t nowTime = time(NULL);
            int startTime = nowTime > fashion->mExpireTime ? nowTime : fashion->mExpireTime;
            expireTime = startTime + lastTime;
        }
        fashionList->updateExpiration(fashion, expireTime);
    }
    
    addExp(exp);
    save();
    calProperty();
    return true;
}



void accuFashionProperty(WardrobeFashion* fashion, BaseProp& baseProp, BattleProp& batprop)
{
    ItemCfgDef* itemcfg = ItemCfg::getCfg(fashion->mId);
    if (itemcfg == NULL) {
        return;
    }
    
    batprop.setAtk(batprop.getAtk() + itemcfg->ReadInt("atk"));
    batprop.setDef(batprop.getDef() + itemcfg->ReadInt("def"));
    batprop.setDodge(batprop.getDodge() + itemcfg->ReadFloat("dodge"));
    batprop.setHit(batprop.getHit() + itemcfg->ReadFloat("hit"));
    batprop.setCri(batprop.getCri() + itemcfg->ReadFloat("cri"));
    batprop.setMaxHp(batprop.getMaxHp() + itemcfg->ReadInt("hp"));
}


void Wardrobe::calProperty()
{
    BaseProp baseProp;
    BattleProp battleProp;
    
    for (map<int, WardrobeFashion*>::iterator iter = mAllFashions.begin(); iter != mAllFashions.end(); iter++)
    {
        WardrobeFashion* fashion = iter->second;
        if (fashion->mExpired)
        {
            continue;
        }
        //时装属性
        accuFashionProperty(fashion, baseProp, battleProp);
    }
    
    //衣柜属性
    WardrobeCfgDef* def = WardrobeCfg::getWardrobeCfg(getLvl());
    if (def)
    {
        //baseProp.mCapa += def->getAccCapa();
        //baseProp.mInte += def->getAccInte();
        //baseProp.mPhys += def->getAccPhys();
        //baseProp.mStre += def->getAccStre();
        
        battleProp.mAtk += def->getAccAtk();
        battleProp.mDef += def->getAccDef();
        battleProp.mHit += def->getAccHit();
        battleProp.mDodge += def->getAccDodge();
        battleProp.mMaxHp += def->getAccHp();
        battleProp.mCri += def->getAccCri();
    }

    mBaseProp = baseProp;
    mBatProp = battleProp;
    
    mOwner->CalcPlayerProp();
}

void Wardrobe::accuProperty(BaseProp& baseProp, BattleProp& battleProp)
{
    baseProp += mBaseProp;
    battleProp += mBatProp;
}

vector<int> Wardrobe::getAllFashionId()
{
    map<int, WardrobeFashion*>::iterator iter = mAllFashions.begin();
    map<int, WardrobeFashion*>::iterator endIter = mAllFashions.end();
    
    vector<int> ret;
    ret.clear();
    
    for (; iter != endIter; iter++) {
        
        if (iter->second != NULL) {
            ret.push_back(iter->second->mId);
        }
    }
    
    return ret;
}
