//
//  DBPetPropName.h
//  GameSrv
//
//  Created by Huang Kunchao on 13-7-8.
//  宠物数据库字段名字,具体含义请看宠物属性
//

#ifndef __GameSrv__DBPetPropName__
#define __GameSrv__DBPetPropName__

#ifndef PetPropTypeToValueBegin
#define PetPropTypeToValueBegin() enum DBPetPropType {
#endif

#ifndef PetPropTypeToValue
#define PetPropTypeToValue(name, value)  name,
#endif

#ifndef PetPropTypeToValueEnd
#define PetPropTypeToValueEnd()  ePetPropCount, };
#endif


PetPropTypeToValueBegin()

PetPropTypeToValue(ePetPropType_PetMod, petmod)
PetPropTypeToValue(ePetPropType_Level, lvl)
PetPropTypeToValue(ePetPropType_Exp, exp)
PetPropTypeToValue(ePetPropType_TotalExp, totalexp)

PetPropTypeToValue(ePetPropType_Growth, growth)
PetPropTypeToValue(ePetPropType_Attack, batk)
PetPropTypeToValue(ePetPropType_Defense, bdef)
PetPropTypeToValue(ePetPropType_HitPoints,bhp)

PetPropTypeToValue(ePetPropType_Dodge,bdodge)
PetPropTypeToValue(ePetPropType_Hit,bhit)
PetPropTypeToValue(ePetPropType_Skills,skills)
PetPropTypeToValue(ePetPropType_Owner,owner)

PetPropTypeToValue(ePetPropType_IsActive,isactive)
PetPropTypeToValue(ePetPropType_Physical,phys)
PetPropTypeToValue(ePetPropType_Capa,capa)
PetPropTypeToValue(ePetPropType_Intelligence,inte)
PetPropTypeToValue(ePetPropType_Strength,stre)

PetPropTypeToValue(ePetPropType_Star,star)
PetPropTypeToValue(ePetPropType_StarLevel,starlvl)

PetPropTypeToValueEnd()

extern const char* g_GetPetPropName(int type);

static const char* szPetProChangeType_Absorb        = "Absorb";            //吞宠
static const char* szPetProChangeType_BeAbsorbed    = "BeAbsorbed";        //被吞


#endif /* defined(__GameSrv__DBPetPropName__) */
