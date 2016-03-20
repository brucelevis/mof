//
//  Defines.cpp
//  GoldOL
//
//  Created by Forest Lin on 13-9-22.
//
//

#include "Defines.h"



ObjSex RoleType2Sex(int roletype)
{
    return (ObjSex)((roletype - 1) / 3);
}

ObjJob RoleType2Job(int roletype)
{
    if ( roletype <= 0 || roletype > 6) {
        return eAllJob;
    }
    return (ObjJob)((roletype - 1) % 3);
}

int calcBattleForce(BattleProp * battleProp)
{
    int battle = battleProp->getAtk() + battleProp->getDef() *2 + battleProp->getMaxHp()/10 + int ((battleProp->getHit()-90) *10) +int(battleProp->getDodge() * 10) + int(battleProp->getCri() * 20);
    return battle;
}

int calcPetBattleForce(BattleProp* battleProp)
{
    int battle = battleProp->getAtk() + battleProp->getDef() *2 + battleProp->getMaxHp()/10 + int ((battleProp->getHit()-90) *10) +int(battleProp->getDodge() * 10) + int(battleProp->getCri() * 20);
    
    return battle * 0.6;
}

int calVerifyCode(BattleProp * battleProp)
{
    float fcode = .0f;
    fcode += 0.17f * battleProp->BattleProp::getAtk();
    fcode += 0.85f * battleProp->BattleProp::getMaxHp();
    fcode += 0.76f * battleProp->BattleProp::getCri();
    fcode += 0.38f * battleProp->BattleProp::getDef();
    fcode += 0.71f * battleProp->BattleProp::getDodge();
    fcode += 0.43f * battleProp->BattleProp::getHit();
    
    return fcode;
}

std::string getSceneNameBySceneType(eSceneType type)
{
    return
    (type == stDungeon) ? "Normal_Dungeons" :
    (type == stFriendDunge) ? "Firend_dungeons" :
    (type == stDefendStatue) ? "God_Dungeons" :
    (type == stChallenge)? "Guild_Dungeons":
    "";
}

PetAdventureType  getPetAdventureType(std::string type)
{
    return
    (type == "normal") ?  ePetAdventure_normal :
    (type == "permanent_activity") ?  ePetAdventure_permanent_activity  :
    (type == "temporary_activity" ) ?  ePetAdventure_temporary_activity :eUnKnowType
    ;
}

