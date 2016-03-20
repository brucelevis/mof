//
//  Pet.h
//  GameSrv
//
//  Created by prcv on 13-5-6.
//
//  宠物

#ifndef __GameSrv__Pet__
#define __GameSrv__Pet__

#include "Obj.h"
#include "redis_helper.h"
#include "main.h"
#include "Singleton.h"
#include "reference.h"
#include "RedisHash.h"
#include "msg.h"
#include "ItemBag.h"

#include <iostream>
#include <vector>
#include <map>
#define MAX_PETS_COUNT 50
using namespace std;

class Role;
class PetCfgDef;
class PetEvolutionMaterial;

enum PetState
{
    kPetSleep,          //休息
    kPetActive,         //出战
};

struct PetSave
{
    PetSave():  petmod(0),
                isactive(0),
                totalexp(0),
                exp(0),
                lvl(0),
                batk(0),
                bdef(0),
                bhp(0),
                bdodge(0.0f),
                bhit(0.0f),
                mStre(0),
                mInte(0),
                mPhys(0),
                mCapa(0),
                star(0),
                starlvl(0),
                oldGrowth(0),
                hp(0),
                mBattle(0),
                owner(0),
                verifyPropCode(0.0f)
    {}
    int petmod;         //宠物模板id,决定品质、暴击
    int isactive;       //休息或出战

    int totalexp;       //总经验 = 1 +2 +3级 + ... + 当前级
    int exp;            //当前级经验
    int lvl;            //等级

    int batk;           //攻击
    int bdef;           //防御
    int bhp;            //生命
    float bdodge;       //闪避
    float bhit;         //命中
    
    int mStre;          //力量
    int mInte;          //智力
    int mPhys;          //体力
    int mCapa;          //耐力
    
    int star;           //后天星量。自身价值量 = 先天 + 后天
    int starlvl;        //星级
    
    int oldGrowth;      //老成长值，已废，改用getGrowth()
        
    int hp;
    int mBattle;
    vector<int> oldskills; //技能
    vector<int> oldlockSkills;//被锁技能

    int owner;          //主人
    
    float verifyPropCode; //属性校验码
};

class Pet : public PetSave, public Obj ,public PetNewProperty
{
public:
    static const char* sGenIdKey;
    
    int petid;

    Pet(int _petid);

    bool load(redisContext* ctx);

    void save();

    void saveExp();

    void gm_SetLvl(int i_lvl);
    
    void addExp(int exp, int ownerlvl);
    
    void saveProp();

    void saveStar();
    
    void saveBattle();
    
    void saveNewProperty();
    
    void calculateAttributes();    //计算战斗力
    
    void convertStarToStage();
    
    bool hasSkill(int ID) { return getSkill().exist(ID); }
    
    void getBattleProp(obj_petBattleProp &info);
    
    void getPetInfo(obj_petinfo &info);
    
    void makePropVerifyCode();
    
    void calculateBattle();//计算战斗力
    
    //BattleProp mRealBattleProp;
    
//  幻兽大冒险加 Buf 前备份
    vector<int> upgrade_skill;
    bool isBattleSkill(int skillId)
    {
        for (vector<int>::iterator iter = upgrade_skill.begin(); iter != upgrade_skill.end(); iter++) {
            if (*iter == skillId) {
                return true;
            }
        }
        
        return false;
    }

    
    bool checkSkillStudied(int skillid);
    
    void getSortedSkills(vector<int>& skills);
    
    string petInfoToString();
    
//    int getSkillId(int index);    //通过顺序来获取技能id
//    
//    int setSkillId(int index);     //设置技能id，
    void setSpiritSpeakId(int spiritspeakId);
    int getSpiritSpeakId();
protected:
    void calculateBaseAttributes(const PetCfgDef* petcfg); //计算基础属性
    void calculateIncrAttributes(const PetCfgDef* petcfg); //外部加成
	void calculateTotemAttributes(const PetCfgDef* petcfg); // 图腾加成
private:
    int mSpiritSpeakId;
};

class PetAssist
{
public:
    static PetAssist* create(Role* role)
    {
        PetAssist* petassist = new PetAssist;
        petassist->mRole = role;
        return petassist;
    }
    
    void unsetAssistPet(int pos)
    {
        if (pos > 0 && pos <= mPets.size())
        {
            mPets[pos - 1] = NULL;
        }
    }
    void setAssistPet(Pet* pet, int pos)
    {
        for (int i = mPets.size(); i < pos; i++)
        {
            mPets.push_back(0);
        }
        
        if (pos > 0)
        {
            mPets[pos - 1] = pet;
        }
    }
    int  getPetAssistPos(int petid)
    {
        for (int i = 0; i < mPets.size(); i++)
        {
            if (mPets[i] && mPets[i]->petid == petid)
            {
                return i + 1;
            }
        }
        
        return 0;
    }
    
    const vector<Pet*>& getPets()
    {
        return mPets;
    }
    
    
    
    void save();
    
    void load();
    
    void calcProperty(BattleProp& battleprop, BaseProp& baseprop);
    
private:
    
    void onCalcProperty(BattleProp& battleprop, BaseProp& baseprop,int position);
    
private:
    vector<Pet*> mPets;
    Role* mRole;
};

class PetMgr
{
public:
    //生成宠物id
    static int genPetId();
    
    //创建新的宠物
    static Pet* create(PetSave& info);
    
    //加载已有的宠物
    static Pet* loadPet(int petid);
    
    static PetMgr* create(Role* role)
    {
        PetMgr* petmgr = new PetMgr;
        petmgr->mRole = role;
        return petmgr;
    }
    
    static void delPetFromDB(int petid);
    
    static void delPetsFromDB(const vector<int>& petids);
    
    const vector<Pet*>& getPets()
    {
        return mPets;
    }
    
    Pet* getActivePet()
    {
        return mActivePet;
    }
    void setActivePet(Pet* pet)
    {
        mActivePet = pet;
    }
    
    Pet* getPet(int petid);
    void removePet(int petid);
    void addPet(Pet* pet);
    
    void load();
    void save();
    void unload();
    
    int getPetsCount()
    {
        return mPets.size();
    }
    
private:
    vector<Pet*> mPets;
    Pet* mActivePet;
    Role* mRole;
};

//幻兽大冒险机器幻兽的创建
int  generatePetAdventureRobotPetId(int sceneId, int petmod);
bool initPetAdventureRobotPetProp(int sceneId, int petmod, Pet& pet);

enum enPetRatioStrategy                 //加成策略
{
    enPetRatioStrategy_Star,            //升星加成,升级带坑
    enPetRatioStrategy_Stage            //升阶加成,升级纯概率
};

const int PET_RATIO_STRATEGY = enPetRatioStrategy_Stage;//2013.9.13后使用

/*******
 幻兽创建
 ******/
extern bool createPetProp(int ownerId, int petMod, PetSave& info);
extern Pet* producePet(Role* role, int petmod);
extern void notifyNewPet(Role* role, Pet* newPet);
extern int getPetSkillStudied(int skillid , Pet* pet);

/*******
 幻兽吞噬
 ******/

//目测这个已经没用了
bool PetCalStarUpgrade(Pet* mainPet, const vector<Pet*>& subPets, bool & bStarUpgradeSuccess, int& totalAbsorbStar);

bool PetStageUpgrade(Pet* mainPet, const vector<Pet*>& subPets);


/**
 *	@brief	宠物的吞噬，其中的宠物经验由于涉及到owner的等级，所以在其他地方处理
 *
 *	@param 	mainPet 	主宠
 *	@param 	subPet 	被吞噬的副宠， 不用vector形式是因为方便以前的日志
 *
 *	@return	true=成功， false = 失败
 */
bool PetAbsorb(Pet* mainPet, Pet* subPet);

/*******
 幻兽进化
 ******/

int checkPetEvolutionCondition(Pet* pet, PetEvolutionMaterial* condition);
int petModId2petEvolutionTypeid(int petMod);
void calPetEvolutionRestitutionItem(Pet* myPet, PetEvolutionMaterial* condition, int& goldRestitution, ItemArray& items);
void sendPetEvolutionRestitutionMail(Role* role, RewardStruct& reward, ItemArray& items);

void calPetEvolutionResidueStage(int targetPetMod, vector<Pet*> subPets, ItemArray& items);

#endif /* defined(__GameSrv__Pet__) */
