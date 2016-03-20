//
//  PetNumericConvert.h
//  GameSrv
//
//  Created by nothing on 14-3-15.
//
//

#ifndef __GameSrv__PetNumericConvert__
#define __GameSrv__PetNumericConvert__

#include <iostream>
#include <string>

#include "ItemBag.h"
using namespace std;

class Pet;
class Role;

//宠物属性转换，重新生成宠物成长，攻击力，防御力，血量，命中，回避
//只用于国内版本
void doPetNumericConvert(Pet *pet,
                         const int &pet_mod,
                         int &pet_atk,          // 攻击力
                         int &pet_def,          //防御力
                         int &pet_hp,           //血量
                         float &pet_hit,        //命中
                         float &pet_dodge);      //闪避

void doBagItemToGold(Role *role, ItemGroup &itemgroup, bool *isNeedStore = NULL);

void RoleEquipNumericConvert(Role *role);

void convertEquip(Role *role, ItemArray &items);

void itemToGold(Role *role, ItemArray &items);

void saveRoleEquipAndBackBag(Role *role);


#endif /* defined(__GameSrv__PetNumericConvert__) */
