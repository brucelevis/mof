//
//  Totem.cpp
//  GameSrv
//
//  Created by jin on 14-5-19.
//
//

#include "Totem.h"
#include "Utils.h"
#include "Role.h"
#include "GameLog.h"

INSTANTIATE_SINGLETON(TotemMon)

eTotemImmolation TotemMon::petCheck(Role *role, vector<int> &pets)
{
	for (vector<int>::const_iterator it = pets.begin(); it != pets.end(); it++) {

		Pet* pet = role->mPetMgr->getPet(*it);
		if (NULL == pet)
		{
			return eTotemImmolation_PetErr;
		}
        //宠物吞噬锁开启,不能吞噬
        if (true == pet->getSwallowLock()) {
            notify_disconnect_tip notify;
            notify.tip = "请解锁幻兽再执行操作";
            role->send(&notify);
            return eTotemImmolation_PetErr;
        }
		// 出战宠
		if (pet == role->mPetMgr->getActivePet())
		{
			return eTotemImmolation_PetIsActive;
		}
		// 助阵宠
		if (role->mAssistPet->getPetAssistPos(*it) > 0)
		{
			return eTotemImmolation_PetIsAssist;
		}
	}
	return eTotemImmolation_Success;
}

eTotemImmolation TotemMon::totemCheck(const Totem &totem, Role *role, vector<int> &pets)
{
	SysTotemLvl *infoLvl = SysTotemCfgMgr::sCfg.getInfoLvl(totem.getId(), totem.getLvl() + 1);
	if (NULL == infoLvl)
	{
		return eTotemImmolation_SysLvlErr;
	}
	//　超数量
	if (pets.size() > infoLvl->needUpNum - totem.getNum())
	{
		return eTotemImmolation_PetIsFull;
	}
	
	for (vector<int>::const_iterator it = pets.begin(); it != pets.end(); it++) {
		
		Pet* pet = role->mPetMgr->getPet(*it);
		if (NULL == pet)
		{
			return eTotemImmolation_PetErr;
		}
		if (pet->getStage() < infoLvl->immolationStage)
		{
			return eTotemImmolation_PetNoStage;
		}
		if (pet->getLvl() < infoLvl->immolationLvl)
		{
			return eTotemImmolation_PetNoLvl;
		}
		
		PetCfgDef* petcfg = PetCfg::getCfg(pet->petmod);
		if (NULL == petcfg)
		{
			return eTotemImmolation_PetNoMonsterCfg;
		}
		// 需要检查对应的宠物模板id
		if (!infoLvl->ignoreMonster
			&& petcfg->getMonsterTpltID() != totem.getId())
		{
			return eTotemImmolation_PetNoMonster;
		}
		if (petcfg->getQua() < infoLvl->immolationQua)
		{
			return eTotemImmolation_PetNoQua;
		}
	}
	return eTotemImmolation_Success;
}


void TotemMon::totemCalculate(int roleId, BattleProp &battleProp)
{
	Role *role = SRoleMgr.GetRole(roleId);
	if (NULL != role) {
		role->getTotemMgr()->calculateProperty(battleProp);
	} else {
		TotemMgr totemMgr;
		totemMgr.init(roleId);
		totemMgr.load();
		
		totemMgr.calculateProperty(battleProp);
	}
}

void TotemMon::updateAddition(int roleId)
{
	BattleProp batProp;
	this->totemCalculate(roleId, batProp);
	mBatAddition[roleId] = batProp;
}

BattleProp & TotemMon::getAddition(int roleId)
{
	map<int, BattleProp>::iterator it = mBatAddition.find(roleId);
	if (it != mBatAddition.end()) {
		return it->second;
	}
	BattleProp batProp;
	this->totemCalculate(roleId, batProp);
	pair<map<int, BattleProp>::iterator, bool> ret;
	ret = mBatAddition.insert(make_pair(roleId, batProp));
	return ret.first->second;
}

void TotemMon::sendAllPet(Role *role)
{
	vector<Pet*> pets = role->mPetMgr->getPets();
	for (vector<Pet*>::iterator it = pets.begin(); it != pets.end(); it++) {
		(*it)->calculateAttributes();
		role->onSendPetPropAndVerifycode(*it);
	}
}

void TotemMon::refresh()
{
	mBatAddition.clear();
}

bool Totem::load(const char *str)
{
	Json::Value val;
	if (!xyJsonSafeParse(str, val)) {
		return false;
	}
	mId = xyJsonAsInt(val["id"]);
	mLvl = xyJsonAsInt(val["lvl"]);
	mNum = xyJsonAsInt(val["num"]);
	return true;
}

string Totem::format() const
{
	Json::Value val;
	val["id"] = mId;
	val["lvl"] = mLvl;
	val["num"] = mNum;
	return xyJsonWrite(val);
}

bool Totem::updateUp()
{
	// 检查升级
	bool bet = this->totemLvlUp();
	if (bet) {
		mNum = 0;
	}
	return bet;
}

bool Totem::addTotemNum(int num)
{
	check_min(num, 0);
	
	mNum += num;
	
	// 检查升级
	return this->totemLvlUp();
}

bool Totem::totemLvlUp()
{
	SysTotemInfo *info = SysTotemCfgMgr::sCfg.getInfo(mId);
	if (NULL == info) {
		return false;
	}
	// 满级了
	if (mLvl >= info->getLvlCount()) {
		mNum = 0;
		return false;
	}
	SysTotemLvl *infoLvl = SysTotemCfgMgr::sCfg.getInfoLvl(mId, mLvl + 1);
	if (NULL == infoLvl) {
		return false;
	}
	// 未满足升级值
	if (mNum < infoLvl->needUpNum) {
		return false;
	}
	
	mLvl += 1;
	mNum -= infoLvl->needUpNum;
	return true;
}

void TotemMgr::load()
{
	RedisResult result(redisCmd("hvals totem:%d", mRoleId));
    int elementnum = result.getHashElements();
    for (int i = 0; i < elementnum; i++) {
		Totem totem;
		if (!totem.load(result.readHash(i).c_str())) {
            continue;
        }
		mTotem.insert(make_pair(totem.getId(), totem));
	}
	
	this->updateOldData();
}

void TotemMgr::save(const Totem& totem)
{
	doRedisCmd("hmset totem:%d %d %s", mRoleId, totem.getId(), totem.format().c_str());
}

bool TotemMgr::addTotem(int totemId, int num)
{
	Xylog log(eLogName_TotemNum, mRoleId);
	Totem &totem = this->getTotem(totemId);
	// 操作前数据
	log << totem.getId() << num << totem.getLvl() << totem.getNum();
	
	bool isUp = totem.addTotemNum(num);
	
	// 操作后数据
	log << totem.getLvl() << totem.getNum() << isUp;

	this->save(totem);
	return isUp;
}

void TotemMgr::updateOldData()
{
	for (map<int, Totem>::iterator it = mTotem.begin(); it != mTotem.end(); it++) {
		if (it->second.updateUp()) {
			this->save(it->second);
		}
	}
}

void TotemMgr::calculateProperty(BattleProp& batprop)
{
	map<int, int> info;
	for (Iterator it = mTotem.begin(); it != mTotem.end(); it++) {
		info.insert(pair<int, int>(it->first, it->second.getLvl()));
	}
	SysTotemCfgMgr::sCfg.calculate(batprop, info);
}

const Totem & TotemMgr::getTotemConst(int totemId)
{
	return this->getTotem(totemId);
}


Totem & TotemMgr::getTotem(int totemId)
{
	map<int, Totem>::iterator it = mTotem.find(totemId);
	if (it != mTotem.end()) {
		return it->second;
	}
	Totem totem(totemId);
	pair<map<int, Totem>::iterator, bool> ret;
	ret = mTotem.insert(make_pair(totemId, totem));
	return ret.first->second;
}

