//
//  pet_handler.c
//  GameSrv
//
//  Created by prcv on 13-5-6.
//
//

#include "hander_include.h"
#include "Pet.h"
#include "Utils.h"
#include "Paihang.h"
#include "PetPvp.h"
#include "GameLog.h"
#include "daily_schedule_system.h"
#include "DBRolePropName.h"
#include "MysteriousMgr.h"
#include "Honor.h"
#include "centerClient.h"

extern void sendSceneDisplayedPets(Role* role);
extern void broadcastSceneDisplayedPets(Scene* scene, int roleId = 0);

//get pet list
handler_msg(req_list_pet, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_list_pet ack;
    ack.errorcode = 0;
    
    const vector<Pet*>& pets = role->mPetMgr->getPets();
    for (int i = 0; i < pets.size() && i < 100; i++)
    {
        Pet* pet = pets[i];
        if (pet)
        {
            obj_petinfo info;
            pet->getPetInfo(info);            
            ack.petinfos.push_back(info);
        }
    }
    
    sendNetPacket(sessionid, &ack);
    
    role->sendPetPropAndVerifycode(role->mPetMgr->getActivePet());
}}

//recall active
handler_msg(req_recall_pet, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_recall_pet ack;  
    ack.errorcode = 1;
    ack.petid = req.petid;
    
    bool shouldSendPetList = false;
    int displayedNum = GlobalCfg::sCityPetNum;
    
    Pet* activePet = role->mPetMgr->getActivePet();
    if (activePet && activePet->petid == req.petid)
    {
        Scene* scene = role->getScene();
        if (scene){
            int rank = scene->removePet(activePet);
            if (rank > 0 && rank <= displayedNum) {
                shouldSendPetList = true;
            }
        }
        
        activePet->isactive = 0;
        ack.errorcode = 0;
        role->mActivePetId = 0;
        role->saveProp(GetRolePropName(eRoleActivePetId), role->getActivePetId());
        activePet->save();
        role->mPetMgr->setActivePet(NULL);
        
        //在宠物排行榜中删除
        role->UploadPetPaihangData(eRemove, activePet->petid, activePet->mBattle);
        
        activePet->calculateAttributes();//不出战降低战斗力
        
        role->sendPetPropAndVerifycode(activePet);
     
        if (shouldSendPetList) {
            broadcastSceneDisplayedPets(role->getScene());
        }
    }
    
    sendNetPacket(sessionid, &ack);
}}

//use pet
handler_msg(req_use_pet, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_use_pet ack;
    ack.errorcode = 1;
    ack.petid = req.petid;
    
    if (req.petid == 0)
    {
        return;
    }
    
    if (role->mAssistPet->getPetAssistPos(req.petid) > 0)
    {
        return;
    }
    
    Pet* activePet = role->mPetMgr->getActivePet();
    if (activePet && activePet->petid == req.petid)
    {
        return;
    }
    
    Pet* toActive = role->mPetMgr->getPet(req.petid);
    if (toActive == NULL)
    {
        return;
    }
    
    bool shouldSendPetList = false;
    int displayedNum = GlobalCfg::sCityPetNum;
    
    if (activePet)
    {
        Scene* scene = role->getScene();
        if (scene){
            int remRank = scene->removePet(activePet);
            if (remRank > 0 && remRank <= displayedNum) {
                shouldSendPetList = true;
            }
        }
        
        activePet->isactive = 0;
        activePet->save();
        role->UploadPetPaihangData(eRemove, activePet->petid, activePet->mBattle);
        activePet->calculateAttributes();//不出战降低战斗力，没有了主人公会技能有加成
        role->sendPetPropAndVerifycode(activePet);
    }
    //在宠物排行榜中删除
    
    Scene* scene = role->getScene();
    if (scene){
        int addRank = scene->addPet(toActive);
        if (addRank > 0 && addRank <= displayedNum) {
            shouldSendPetList = true;
        }
    }
    toActive->isactive = 1;
    role->mActivePetId = req.petid;
    role->saveProp(GetRolePropName(eRoleActivePetId), role->mActivePetId);
    toActive->save();
    
    //出战增强战斗力
    toActive->calculateAttributes();
    
    role->UploadPetPaihangData(eUpdate, req.petid, toActive->mBattle);
    role->mPetMgr->setActivePet(toActive);
    
    ack.errorcode = 0;
    sendNetPacket(sessionid, &ack);
    
    if (shouldSendPetList) {
        broadcastSceneDisplayedPets(role->getScene());
    }
    
    //更新出战宠信息
    role->sendPetPropAndVerifycode(toActive);
}}


#pragma mark -
#pragma mark 吞宠

//发送吞宠结果
static void SendAckAbsorbPet(int sid ,ack_absorb_pet& ack, int errorcode)
{
    ack.errorcode = errorcode;    
    sendNetPacket(sid, &ack);
}

//检查吞宠条件
static bool _CheckAbsorbPet(Role* role, const req_absorb_pet& req, ack_absorb_pet& ack , int sessionid ,int& fee)
{
    //被吃的数量校验
    if( ! PetCfg::checkAbsorbAmount(req.absorbedid.size()) ){
        SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_PetAbsorAmountError);
        return false;
    }
    
    vector<int>::const_iterator iter,end;
    //首先校验宠物ID合法性，下面就不校验了
    const vector<int>& vecid = req.absorbedid;
    for (iter = vecid.begin(),end=vecid.end(); iter != end ;++iter) {
        int absorbid = *iter;
        
        //宠物不能吞自己
        if (absorbid == req.petid)
        {
            SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_CanotAbsorbedYourSelf);
            return false;
        }
        
        //自己没这个宠
        Pet* pet = role->mPetMgr->getPet(absorbid);
        if( pet == NULL){
            SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_PetIdError);
            return false;
        }else{
            
            //宠物吞噬锁开启,不能吞噬
            if (true == pet->getSwallowLock()) {
                notify_disconnect_tip notify;
                notify.tip = "请解锁幻兽再执行操作";
                role->send(&notify);
                return false;
            }
            
            //出战宠不能被吃
            if (pet == role->mPetMgr->getActivePet())
            {
                SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_ActivePetCanotBeAbsorbed);
                return false;
            }
            
            //模板文件有问题
            PetCfgDef* petcfg = PetCfg::getCfg(pet->petmod);
            if(petcfg == NULL){
                log_error("can't find pet template , petid"<<req.petid<<" petmod ="<<pet->petmod);
                SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_UnknownError);
                return false;
            }            
        }
    }
    
    
    //检查主宠数据
    Pet* pMainPet = role->mPetMgr->getPet(req.petid);
    if (pMainPet == NULL)
    {
        SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_UnknownError);
        return false;
    }
    PetCfgDef* pMainPetCfg = PetCfg::getCfg(pMainPet->petmod);
    if (pMainPetCfg == NULL)
    {
        SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_UnknownError);
        return false;
    }
    
    
    if(PET_RATIO_STRATEGY == enPetRatioStrategy_Star)
    {
        //升星消耗金币 = 主幻兽本等级升星消耗金币系数*（被吞兽个数）
        int consumefactor = pMainPetCfg->starUpConsumeFactor[pMainPet->starlvl];
        fee = consumefactor * req.absorbedid.size();
        if( fee< 0 || role->getGold() < fee)
        {
            SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_NeedMoreGold);
            return false;
        }
    }
    
     if(PET_RATIO_STRATEGY == enPetRatioStrategy_Stage)
     {
         //升阶消耗金币 = 主幻兽本等级升阶消耗金币系数*（被吞兽个数）
         fee = PetCfg::getStageUpConsumeFactor(pMainPetCfg->getQua() - 1, pMainPet->getStage());
         fee *= req.absorbedid.size();

         if( role->getGold() < fee)
         {
             SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_NeedMoreGold);
             return false;
         }
     }
    
    return true;
}


//计算是否升星成功
//static bool _CalStarUpgrade(Role* role, const req_absorb_pet& req , bool & bStarUpgradeSuccess, int& totalAbsorbStar)
//{
//    try{
//        
//        vector<int>::const_iterator iter,end;
//        
//        Pet* pMainPet = role->mPetMgr->getPet(req.petid);
//        PetCfgDef* pMainPetCfg = PetCfg::getCfg(pMainPet->petmod);
//        
//        //计算升星成功概率
//        float successRate = 0;
//
//        for (iter = req.absorbedid.begin(),end = req.absorbedid.end(); iter != end; ++iter) {
//            Pet* pet = role->mPetMgr->getPet(*iter);
//            PetCfgDef* petcfg = PetCfg::getCfg(pet->petmod);
//            int tmp = petcfg->getStarBorn() + pet->star;
//            float factor = PetCfg::getPercentConversion(petcfg->getQua() - 1, pet->starlvl);
//            totalAbsorbStar += factor * tmp;
//        }
//        
//        //满级不能升了
//        if(pMainPet->starlvl + 1 >= pMainPetCfg->starUpStep.size()){
//            bStarUpgradeSuccess = false;
//            return true;
//        }
//        
//        int mainPetStar = pMainPet->star + pMainPetCfg->getStarBorn() ;                  //主幻兽自身价值量
//        int starUpgradeValue = pMainPetCfg->starUpStep[pMainPet->starlvl];          //主幻兽本星级升星价值量
//        int starUpgradeValueNext = pMainPetCfg->starUpStep[pMainPet->starlvl + 1];  //主幻兽下一星级升星价值量
//        float starUpSuccRatio = pMainPetCfg->starUpSuccRatio[pMainPet->starlvl];    //升星基础成功率       
//        
//        
//        //升星成功率=（被吞噬幻兽总自身价值量+主幻兽自身价值量-主幻兽本星级升星价值量）/主幻兽下一星级升星价值量+主幻兽本星级升星基础成功率
//        
//        successRate = 1.0f * (totalAbsorbStar + mainPetStar - starUpgradeValue) / starUpgradeValueNext  + starUpSuccRatio;
//        
//        
//        if( successRate < 0)
//            bStarUpgradeSuccess = false;
//        else{
//            float randnum = range_randf(0.0, 1.0);
//
//            if(randnum < successRate)
//                bStarUpgradeSuccess = true;
//            else
//                bStarUpgradeSuccess = false;
//        }
//        
//        return true;
//        
//    }catch(...){
//        return false;
//    }
//}
//
////升阶，外部保证req参数正确
//static bool _StageUpgrade(Role* role, const req_absorb_pet& req )
//{
//    Pet* pMainPet = role->mPetMgr->getPet(req.petid);
//    PetCfgDef* pMainCfg = PetCfg::getCfg(pMainPet->petmod);
//    
//    int absorbStage = 0;
//    for (int i =0; i< req.absorbedid.size(); ++i)
//    {
//        int petid = req.absorbedid[i];
//        Pet* pet = role->mPetMgr->getPet(petid);
//        PetCfgDef* petcfg = PetCfg::getCfg(pet->petmod);
//        
//        int stage = pet->getStage();        
//        absorbStage += petcfg->getStageBorn() ;
//        
//        if( stage > 0 )//0阶没有继承的，下标为0的继承是指阶为1阶时的可继承部分
//        absorbStage += petcfg->stageInherit[stage-1];
//    }
//    
//    bool upgrade = false;
//    while(true)
//    {
//        int stage = pMainPet->getStage();
//        if(stage>= pMainCfg->stageUpStep.size())//满级了不能升
//            break;
//        
//        int step = pMainCfg->stageUpStep[stage];
//
//        if(absorbStage >=  step)
//        {
//            absorbStage -= step;
//            pMainPet->setStage(++stage);
//            upgrade = true;
//            continue;
//        }
//        
//        float rate = 1.0f * absorbStage / step;
//        float randnum = range_randf(0.0, 1.0);
//        if(randnum < rate)
//        {
//            pMainPet->setStage(++stage);
//            upgrade = true;
//        }
//        
//        break;
//    }
//    
//    return upgrade;
//}

//幻兽相关的日志会用到
string makePetInfo4PetLog(Pet* pet)
{
    PetCfgDef* cfg = PetCfg::getCfg(pet->petmod);
    
    stringstream str;

    string petName = "";
    
    if (cfg != NULL) {
        petName = cfg->name;
    }
    
    str<< petName << "," << pet->petInfoToString();
    
    return str.str();
}

handler_msg(req_absorb_pet, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    Xylog log(eLogName_PetAbsorb,roleid);
	Xylog logNew(eLogName_PetAbsorbNew, roleid);
	
    ack_absorb_pet ack;
    ack.errorcode = enPetAbsorbRetsult_UnknownError;
    ack.petid = req.petid;
    ack.absorbedid = req.absorbedid;
    
    Pet* mainPet = role->mPetMgr->getPet(req.petid);
    if (mainPet == NULL) {
        return;
    }
    PetCfgDef* mainPetcfg = PetCfg::getCfg(mainPet->petmod);
    if (mainPetcfg == NULL) {
        return;
    }
    
    log
	<< "吞前属性"
	<< makePetInfo4PetLog(mainPet);
    
    vector<Pet*> subPets;
    subPets.clear();
    
    for (int i = 0; i < req.absorbedid.size(); i++)
    {
        if (role->mAssistPet->getPetAssistPos(req.absorbedid[i]) > 0)
        {
            return;
        }
        
        Pet* subpet = role->mPetMgr->getPet(req.absorbedid[i]);
        
        if (subpet) {
            subPets.push_back(subpet);
        }
    }
    
    int fee = 0;
    if( ! _CheckAbsorbPet(role, req,ack,sessionid,fee) ){//校验参数
        return;
    }
    xyassert(fee>=0);
    
    int totalAbsorbStar = 0;
    bool bStarUpgradeSuccess = false;
    
    //星数处理
    if( ! PetCalStarUpgrade(mainPet, subPets, bStarUpgradeSuccess,totalAbsorbStar))
    {
        log_error("pet absorb unknown error !!! ");
        SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_UnknownError);
        return;
    }
    
    //阶数的处理
    bool stageUpgrade = PetStageUpgrade(mainPet,subPets);

    //扣费
    role->addGold(-fee, "pet_absorb");
    log << fee;
    
    //杀宠、加属性、加星级
    vector<int>::const_iterator iter,end;
    int addExp = 0 ;
    
	stringstream logNewPets;
	stringstream logPets;
    vector<int> realAbsorbedIds;
    for (int i = 0; i < subPets.size(); i++) {
        
        Pet* absorbed = subPets[i];
        if (absorbed == NULL)
            continue;
        
        PetCfgDef* cfg = PetCfg::getCfg(absorbed->petmod);
        
        //吞噬成功
        if (PetAbsorb(mainPet, absorbed)) {
            
			logPets
			<< makePetInfo4PetLog(absorbed)
            << cfg->getExp() << ";";
            
            //累计经验
            addExp += (absorbed->totalexp + cfg->getExp());
            
            realAbsorbedIds.push_back(absorbed->petid);
            
            string petInfo = makePetInfo4PetLog(absorbed);
            LogMod::addLogDelPetInDB(role, absorbed->petid, absorbed->petmod,petInfo.c_str(),  "pet_absorbed");
            
            logNewPets << absorbed->petid << "," << absorbed->petmod << "," << cfg->name << ";";
            role->mPetMgr->removePet(absorbed->petid);
            
        }

    }
    
    //在幻兽竞技场中删除
    SPetPvpMgr.safe_removePetsInFormation(roleid, req.absorbedid);
    
	logNew << req.petid << mainPet->petmod << mainPetcfg->name << logNewPets.str();
    
    mainPet->star += totalAbsorbStar;
    
    if(bStarUpgradeSuccess)
        mainPet->starlvl ++;
    
    //经验要放到最后，因为这个函数会重新拿starlvl算战斗力
    role->addPetExp(req.petid, addExp, "absorb_pet");
    
    log
	<< "吞后属性"
	<< makePetInfo4PetLog(mainPet)
    << logPets.str();
	
    //客户端同步
    notify_sync_pet_prop notify;
    notify.petid =mainPet->petid;
    notify.mPhys = mainPet->mPhys;
    notify.mCapa = mainPet->mCapa;
    notify.mInte = mainPet->mInte;
    notify.mStre = mainPet->mStre;
    sendNetPacket(role->getSessionId(), &notify);
    
    notify_sync_pet_star notifystar;
    notifystar.petid = mainPet->petid;
    notifystar.star = mainPet->star;
    notifystar.starlvl = mainPet->starlvl;
    sendNetPacket(role->getSessionId(), &notifystar);
    
    if(stageUpgrade)
    {
        notify_sync_pet_stage nty;
        nty.petid = mainPet->petid;
        nty.stage = mainPet->getStage();
        sendNetPacket(role->getSessionId(),&nty);
    }
    
    //写DB
    mainPet->saveProp();
    mainPet->saveExp();
    mainPet->saveStar();
    mainPet->saveNewProperty();
    
    role->mPetMgr->save();
    PetMgr::delPetsFromDB(realAbsorbedIds);
    
    
    //更新宠物排行榜中的战斗力
    if (mainPet->petid == role->getActivePetId()) {
        role->UploadPetPaihangData(eUpdate, mainPet->petid, mainPet->mBattle);
    }
    
    
    if(PET_RATIO_STRATEGY == enPetRatioStrategy_Star)
    {
        if(bStarUpgradeSuccess)
            SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_SuccessAndStarlvlUpgrade);
        else
            SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_SuccussButStarlvlUnchange);
    }
    
    if(PET_RATIO_STRATEGY == enPetRatioStrategy_Stage)
    {
        if(stageUpgrade)
            SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_SuccussAndStageUpgrade);
        else
            SendAckAbsorbPet(sessionid,ack,enPetAbsorbRetsult_SuccussButStageUnchange);
    }
    
	mainPet->calculateAttributes();
	role->onSendPetPropAndVerifycode(mainPet);
	
	role->onCalPlayerProp();
	role->sendRolePropInfoAndVerifyCode();
}}

//检查宠物合并的条件， 返回错误代码 ePetMergeRetsult
static bool checkMergeCondition(Role *role, req_pet_merge &req, ack_pet_merge &ack, int &needgold)
{
    //判断数量有没错误,现在是2只合并
    if (req.merge_petid.size() != 1)
    {
        ack.errorcode = ePetMergeRetsult_MergeNumError;
        return false;
    }
    
    //角色上没找到主合并的宠物
    if (!role->mPetMgr->getPet(req.petid))
    {
        ack.errorcode = ePetMergeRetsult_PetIdError;
        return false;
    }
    
    //主合并宠物 模板文件有问题
    Pet *mainPet = role->mPetMgr->getPet(req.petid);
    PetCfgDef *mainPetDef =  PetCfg::getCfg(mainPet->petmod);
    if (!mainPetDef)
    {
        ack.errorcode = ePetMergeRetsult_PetIdError;
        return false;
    }
    

    if (GameFunctionCfg::getGameFunctionCfg().petCanMergeType == ePetMergeList) //简体版
    {
        if (mainPetDef->petMergeList.empty())
        {
            ack.errorcode = ePetMergeRetsult_UnknowError;
            return false;
        }
    }
    else
    {
        if (mainPetDef->mTypeForPetMerger == 0)
        {
            ack.errorcode = ePetMergeRetsult_UnknowError;
            return false;
        }
    }
    
    //主宠物已经是最大等级
    if (mainPet->getMergeLvl() >= PetMergeCfg::getMaxStage())
    {
        ack.errorcode = ePetMergeRetsult_MainPetIsMaxStageLvl;
        return false;
    }
    
    for (vector<int>::iterator iter = req.merge_petid.begin(); iter != req.merge_petid.end(); ++iter)
    {
        //助阵宠物不能被合并
        if (role->mAssistPet->getPetAssistPos(*iter) > 0)
        {
            ack.errorcode = ePetMergeRetsult_PetAssistCanotBeMerged;
            return false;
        }
        
        //自己不能吞自己
        if (req.petid == *iter)
        {
            ack.errorcode = ePetMergeRetsult_CanotMergeYourself;
            return false;
        }
        
        //在角色身上没找到次合并宠物
        if (!role->mPetMgr->getPet(*iter))
        {
            ack.errorcode = ePetMergeRetsult_PetIdError;
            return false;
        }
        
        //宠物吞噬锁开启,不能merge 该宠
        bool swalloLock = role->mPetMgr->getPet(*iter)->getSwallowLock();
        if (swalloLock == true) {
            notify_disconnect_tip notify;
            notify.tip = "请解锁幻兽再执行操作";
            role->send(&notify);
            return false;
        }

        //次合并宠物是出战宠，不准被吞，只准吞别的宠物
        if (role->getActivePetId() == *iter)
        {
            ack.errorcode = ePetMergeRetsult_ActivePetCanotBeMerged;
            return false;
        }
        
        //模板文件有问题
        Pet *pet = role->mPetMgr->getPet(*iter);
        if (!PetCfg::getCfg(pet->petmod))
        {
            ack.errorcode = ePetMergeRetsult_PetIdError;
            return false;
        }
        
        // 不是相同品质
        PetCfgDef *petcfg = PetCfg::getCfg(pet->petmod);
		if (mainPetDef->getQua() != petcfg->getQua()) {
			ack.errorcode = ePetMergeRetsult_QuaError;
			return false;
		}
        
        if (GameFunctionCfg::getGameFunctionCfg().petCanMergeType == ePetMergeList) //简体版
        {
            // 没有找到
            if (0 == count(mainPetDef->petMergeList.begin(), mainPetDef->petMergeList.end(), pet->petmod))
            {
                ack.errorcode = ePetMergeRetsult_PetIdError;
                return false;
            }
        }
        else
        {
            if (petcfg->mTypeForPetMerger == 0)
            {
                ack.errorcode = ePetMergeRetsult_UnknowError;
                return false;
            }
            
            if (mainPetDef->mTypeForPetMerger != petcfg->mTypeForPetMerger)
            {
                ack.errorcode = ePetMergeRetsult_PetsNotSameTypeOrNotSameQua;
                return false;
            }
        }
    }
    
    //检查够不够钱
    int petQua = mainPetDef->getQua();
    PetMergeItem *item = PetMergeCfg::getPetMergeItem(petQua);
    if (!item)
    {
        ack.errorcode = ePetMergeRetsult_UnknowError;
        return false;
    }
    if (role->getGold() < item->cost_gold)
    {
        ack.errorcode = ePetMergeRetsult_YouNeedMoreGold;
        return false;
    }
    needgold = item->cost_gold;

    
    ack.errorcode = ePetMergeRetsult_Ok;
    return true;
}

static int __getMergeCntByTotalexp(int totalexp, const int &selfexp)
{
    int cnt = 0;
    while (totalexp > 0)
    {
        ++cnt;
        totalexp -= selfexp;
    }
    return cnt;
}

//开始合并幻兽
static bool doPetMerge(Role *role, req_pet_merge &req, ack_pet_merge &ack, const int &needgold)
{
    //扣钱
    role->addGold(-needgold, "宠物合并（升阶）");
    
    Xylog log2(eLogName_PetMerge, role->getInstID());
    Xylog log1(eLogName_PetMerge, role->getInstID());
    log2 << "新";
    log1 << "旧";
    
    
    Pet *pet = role->mPetMgr->getPet(req.petid);
    PetCfgDef *mainPetDef = PetCfg::getCfg(pet->petmod);
    
    int mergelvl = pet->getMergeLvl();
    int mergeCurrentExp = pet->getMergeCurrentExp();
    int mergeTotalExp = pet->getMergeTotalExp();
    
    //【旧】宠物ID 阶等级 当前经验 总经验 等级差加成 最大等级加成 成长值加成 删除宠物ID
    log1 << makePetInfo4PetLog(pet) << pet->getMergeExtlvlAdd() << pet->getMergeMaxlvlAdd() << pet->getMergeGrowthAdd();
    log1 << "del:";

    //可以增加的经验
    int addExp = 0 ;
    int addExtlvl = 0;
    int addMaxlvl = 0;
    int addGrowth = 0;
    int addPetexp = 0;
    
    vector<Pet*> mergePets;
    vector<int> removeids; //要删除的宠物ID
    for (vector<int>::iterator iter = req.merge_petid.begin(); iter != req.merge_petid.end(); ++iter)
    {
        int mergepetid = *iter;
        Pet* mergepet = role->mPetMgr->getPet(mergepetid);
        mergePets.push_back(mergepet);
    }
    
    //添加吞噬效果
    PetStageUpgrade(pet, mergePets);
    
    for (int i = 0; i < mergePets.size(); i++) {
        
        Pet* mergepet = mergePets[i];
        PetCfgDef* cfg = PetCfg::getCfg(mergepet->petmod);
        if (cfg == NULL) {
            return false;
        }
        
        //宠物总经验 + 宠物自己的经验
        addExp += mergepet->getMergeTotalExp() + cfg->getMergeExp();
        //等级差
        addExtlvl += mergepet->getMergeExtlvlAdd() + cfg->getExtlvlAdd();
        //等级上限
        addMaxlvl += mergepet->getMergeMaxlvlAdd() + cfg->getMaxlvlAdd();
        //成长值上限
        addGrowth += mergepet->getMergeGrowthAdd() + cfg->getGrowthAdd();
        
        //加上吞噬效果
        PetAbsorb(pet, mergepet);
        
        //累计经验
        addPetexp += (mergepet->totalexp + cfg->getExp());;
        
        removeids.push_back(mergepet->petid);
        
        string petInfo = makePetInfo4PetLog(mergepet);
        LogMod::addLogDelPetInDB(role, mergepet->petid, mergepet->petmod, petInfo.c_str(),  "pet_merge");
        
        stringstream ss;
        ss << mergepet->petid << mergepet->petmod << ":" <<  cfg->getMergeExp() + mergepet->getMergeCurrentExp() << "," << mergepet->getMergeTotalExp() << ","
        << mergepet->getMergeLvl() << "," << mergepet->getMergeExtlvlAdd() << ","
        << mergepet->getMergeMaxlvlAdd() << "," << mergepet->getMergeGrowthAdd() << " ";
        log1 << ss.str();
        
        role->mPetMgr->removePet(mergepet->petid);
    }
    
    //从幻兽竞技场中删除被吞的宠物
    SPetPvpMgr.safe_removePetsInFormation(role->getInstID(), req.merge_petid);
    
    //增加总经验
    mergeTotalExp += addExp;
    
    int maxstageTotalExp = PetMergeCfg::getUplvlTotalExp(mainPetDef->mQua, PetMergeCfg::getMaxStage());
    if (mergeTotalExp > maxstageTotalExp)
    {
        mergeTotalExp = maxstageTotalExp;
    }
    
    addExp += mergeCurrentExp;
    int remain_exp = 0;
    int oldMergelvl = mergelvl;
    
    //根据现等级，增加的经验，判断会不会升阶
    PetMergeCfg::getUplvlByExp(mainPetDef->mQua, mergelvl, addExp, remain_exp);
    
    
    //获取升级到X阶需要吞噬Y只
    int mergedcount = __getMergeCntByTotalexp(mergeTotalExp, mainPetDef->getMergeExp());
    int extlvladd = mergedcount * mainPetDef->getExtlvlAdd();
    int maxlvladd = mergedcount * mainPetDef->getMaxlvlAdd();
    int growthadd = mergedcount * mainPetDef->getGrowthAdd();
    
    //保存宠物升阶数据
    pet->setMergeLvl(mergelvl);
    pet->setMergeTotalExp(mergeTotalExp);
    pet->setMergeCurrentExp(remain_exp);
    pet->setMergeExtlvlAdd(extlvladd);
    pet->setMergeMaxlvlAdd(maxlvladd);
    pet->setMergeGrowthAdd(growthadd);
    pet->saveNewProperty();
    pet->save();
    
    role->addPetExp(pet->petid, addPetexp, "merge_pet");
    
    //【新】宠物ID 阶等级 当前经验 总经验 等级差加成 最大等级加成 成长值加成 
    log2 << makePetInfo4PetLog(pet)
    << pet->getMergeExtlvlAdd() << pet->getMergeMaxlvlAdd() << pet->getMergeGrowthAdd() << pet->petid;
    
    //删除宠物
    role->mPetMgr->save();
    PetMgr::delPetsFromDB(removeids);
    
    //客户端同步
    notify_sync_pet_prop notifyprop;
    notifyprop.petid =pet->petid;
    notifyprop.mPhys = pet->mPhys;
    notifyprop.mCapa = pet->mCapa;
    notifyprop.mInte = pet->mInte;
    notifyprop.mStre = pet->mStre;
    sendNetPacket(role->getSessionId(), &notifyprop);
    
    //旧的星级，不知道有没有用
    notify_sync_pet_star notifystar;
    notifystar.petid = pet->petid;
    notifystar.star = pet->star;
    notifystar.starlvl = pet->starlvl;
    sendNetPacket(role->getSessionId(), &notifystar);
    
    //现在的星级
    notify_sync_pet_stage nty;
    nty.petid = pet->petid;
    nty.stage = pet->getStage();
    sendNetPacket(role->getSessionId(),&nty);
    
    //发送同步宠物升阶以及经验变化
    notify_sync_pet_mergelvl notify;
    notify.petid = pet->petid;
    notify.merge_lvl = mergelvl;
    notify.current_exp = remain_exp;
    notify.extlvladd = pet->getMergeExtlvlAdd();
    notify.maxlvladd = pet->getMergeMaxlvlAdd();
    notify.growthadd = pet->getMergeGrowthAdd();
    role->send(&notify);
    
    ack.petid = req.petid;
    ack.merge_petid = req.merge_petid;
    //role->send(&ack);
    

    //通过此动作来更新宠物战斗力
    //role->addPetExp(req.petid, 0);
    pet->calculateAttributes();
    role->sendPetPropAndVerifycode(pet);
    
    //更新宠物排行榜中的战斗力
    if (pet->petid == role->getActivePetId())
    {
        role->UploadPetPaihangData(eUpdate, pet->petid, pet->mBattle);
    }        

    return true;
}

#pragma mark -
#pragma mark 宠物合并（升阶）

handler_msg(req_pet_merge, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_pet_merge ack;
    ack.errorcode = ePetMergeRetsult_UnknowError;
    ack.petid = 0;
    
    int needgold = 0;

    //检查合并条件
    if (!checkMergeCondition(role, req, ack, needgold))
    {
        role->send(&ack);
        return ;
    }
    
    //宠物合并
    doPetMerge(role, req, ack, needgold);
    
    role->send(&ack);
}}


#pragma mark -
#pragma mark 助阵

handler_msg(req_assist_pet_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_assist_pet_list ack;
    ack.errorcode = 0;
    
    const vector<Pet*>& pets = role->mAssistPet->getPets();
    for (vector<Pet*>::const_iterator iter = pets.begin(); iter != pets.end(); iter++)
    {
        Pet* pet = *iter;
        if (pet != NULL)
        {
            ack.assistPets.push_back(pet->petid);
        }
        else
        {
            ack.assistPets.push_back(0);
        }
    }
    
    sendNetPacket(sessionid, &ack);
    //for (int i = 0; i < ro)
}}

handler_msg(req_set_assist_pet, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    ack_set_assist_pet ack;
    ack.errorcode = 1;
    ack.petid = req.petid;
    ack.pos = req.pos;
    
    int petid = req.petid;
    int pos = req.pos;
    do
    {
        
        if ( !PetAssistCfg::isPositionOpen(role->getLvl(), role->getRoleAwakeLvl(), req.pos) )
        {
            break;
        }
        
        if (petid > 0 && petid == role->getActivePetId())
        {
            break;
        }
        
        if (role->mAssistPet->getPetAssistPos(petid) > 0)
        {
            break;
        }
        
        if (petid == 0)
        {
            role->mAssistPet->unsetAssistPet(pos);
        }
        else
        {
            Pet* pet = role->mPetMgr->getPet(petid);
            if (!pet)
            {
                break;
            }
            role->mAssistPet->setAssistPet(pet, pos);
        }
        
        role->mAssistPet->save();
        role->CalcPlayerProp();
        
        ack.errorcode = 0;
    }
    while (0);
    
    sendNetPacket(sessionid, &ack);
}}


#pragma mark -
#pragma mark 图鉴

// 图鉴列表
handler_msg(req_illustrations_list, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_illustrations_list ack;
    do
    {
        // 获取列表
        IllustrationsMgr *illMgr = role->getIllustrationsMgr();
        IllustrationsTd::const_iterator begin = illMgr->begin();
        IllustrationsTd::const_iterator end = illMgr->end();
        for (; begin != end; begin++) {
            if (begin->second.getType() != req.type) {
                continue;
            }
            obj_illustrationsInfo info;
            info.id = begin->second.getIllId();
            info.green = begin->second.getQuaNum(eIllQua_Green);
            info.blue = begin->second.getQuaNum(eIllQua_Blue);
            info.purple = begin->second.getQuaNum(eIllQua_Purple);
            info.greenReward = begin->second.getRewardState(eIllQua_Green);
            info.blueReward = begin->second.getRewardState(eIllQua_Blue);
            info.purpleReward = begin->second.getRewardState(eIllQua_Purple);
            info.allReward = begin->second.getRewardState(eIllQua_Count);
            ack.info_arr.push_back(info);
        }
    } while (false);
    sendNetPacket(role->getSessionId(), &ack);
}}

// 获取图鉴奖励
handler_msg(req_illustrations_reward, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_illustrations_reward ack;
    ack.errorcode = enIllustrationsRewardReuslt_UnknowError;
    ack.qua = req.qua;
    ack.illId = req.illId;
    do
    {
        IllustrationsQuality qua = (IllustrationsQuality)req.qua;
        
        IllustrationsMgr *illMgr = role->getIllustrationsMgr();
        Illustrations *ill = illMgr->getIllustrations(req.illId);
        if (NULL == ill) {
            ack.errorcode = enIllustrationsRewardReuslt_NoFound;
            break;
        }
        if (eIllState_NotReceive != ill->getRewardState(qua)) {
            ack.errorcode = enIllustrationsRewardReuslt_NotReceive;
            break;
        }
        
        string rewardStr = IllustrationsCfg::getIllQuaReward(req.illId, qua);
        vector<string> awards = StrSpilt(rewardStr, ";");
        ItemArray items;
        if (!role->addAwards(awards, items, "Illustrations Reward"))
        {
            ack.errorcode = enIllustrationsRewardReuslt_AddBagErr;
            break;
        }
        // 修改领取状态
        illMgr->getReward(ill, qua);
		
		Xylog log(eLogName_IllustrationsAward, roleid);
		log << ill->getIllId() << qua << ill->getRewardState(qua) << rewardStr;

        ack.errorcode = enIllustrationsRewardReuslt_Success;
    } while (false);
    sendNetPacket(role->getSessionId(), &ack);
}}

handler_msg(req_get_pet_data, req)
{
    if (req.serverid != Process::env.getInt("server_id") && req.serverid > 0) {
        
        req_get_csPetData_through_cs csReq;
        csReq.doReqServerid = Process::env.getInt("server_id");
        
        csReq.petid         = req.petid;
        csReq.onReqServerid = req.serverid;
        csReq.clientSession = sessionid;
        
        sendNetPacket(CenterClient::instance()->getSession(), &csReq);
    } else {
    
        ack_get_pet_data ack;
        ack.errorcode = 1;
        
        Pet pet(req.petid);
        if (pet.load(get_DbContext()))
        {
            ack.owner = pet.owner;
            ack.petinfo.petid = pet.petid;
            ack.petinfo.petmod = pet.petmod;
            ack.petinfo.growth = pet.getGrowth();
            ack.petinfo.lvl = pet.lvl;
            ack.petinfo.atk = pet.getAtk();
            ack.petinfo.def = pet.getDef();
            ack.petinfo.hp = pet.getMaxHp();
            ack.petinfo.maxHp = pet.getMaxHp();
            ack.petinfo.dodge = pet.getDodge();
            ack.petinfo.hit = pet.getHit();
            ack.petinfo.skills = pet.getSkill().toArray();
            ack.petinfo.capa = pet.mCapa;
            ack.petinfo.inte = pet.mInte;
            ack.petinfo.phy = pet.mPhys;
            ack.petinfo.stre = pet.mStre;
            ack.petinfo.stage = pet.getStage();
            ack.petinfo.cri = pet.getCri();
            ack.petinfo.mergelvl = pet.getMergeLvl();
            ack.petinfo.maxlvladd = pet.getMergeMaxlvlAdd();
        }
        
        sendNetPacket(sessionid, &ack);
    }
}}


#pragma mark -
#pragma mark 洗练成长值

#define return_sendAckPetRefineGrowth( err ) \
do{\
    ack.errcode = err; \
    ack.growth = pet->getGrowth();\
    sendNetPacket(sessionid, &ack);\
    return ;\
}while(0)

const int GROWTH_PILL_ID = 410001;

handler_msg(req_pet_refine_growth,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_pet_refine_growth ack;
    ack.petid = req.petid;
    
    Pet* pet = role->mPetMgr->getPet(req.petid);
    xycheck(pet);
    
    PetCfgDef* petcfg = PetCfg::getCfg(pet->petmod);
    xyassertf(petcfg,"petmod = %d", pet->petmod);
    
    //幻兽升阶后会影响幻兽成长值上限，则会影响它们洗练值
    int addgrowth = pet->getMergeGrowthAdd();
    
    
    if( pet->getGrowth() >=  petcfg->getMaxGrowthRefine() + addgrowth)
    {
        return_sendAckPetRefineGrowth( ePetRefineGrowth_GrowthMax );
    }
    
    xyassertf(petcfg->getNeedGrowthPill() > 0 , "petmod = %d", pet->petmod);
    
    vector<int> pills(petcfg->getNeedGrowthPill(), GROWTH_PILL_ID);
    
    ItemGroup pill(GROWTH_PILL_ID, petcfg->getNeedGrowthPill());
    
    //wangzhigang 2014-11-19
    ItemArray items;
    items.push_back(pill);
    GridArray effeGrid;
    
    if (!role->getBackBag()->PreDelItems(items, effeGrid))
    {
        return_sendAckPetRefineGrowth( ePetRefineGrowth_NeedMorePill );
    }
    
    role->updateItemsChange(effeGrid);
    
    {
        LogMod::addUseItemLog(role, items, "req_pet_refine_growth");
    }
    
    Xylog log(eLogName_PetGrowthRefine,roleid);
    log<<"宠:"<<req.petid<<"名:"<<petcfg->name<<"成长丸数量:"<<pills.size();
    
    // 1-（（幻兽当前成长 - 最小成长值）/（（幻兽最大成长 + 成长值增加数 - 最小成长）*成功率系数））^0.5
    xyassert(petcfg->getMinGrowthRefine() * petcfg->getGrowthSuccFactor() > 0);
	int growth = petcfg->getMaxGrowthRefine() + addgrowth - petcfg->getMinGrowthRefine();
	
	bool isSucc = false;
	if (growth == 0) {
		if (pet->getGrowth() >= petcfg->getMinGrowthRefine())
		{
			return_sendAckPetRefineGrowth( ePetRefineGrowth_Fail );
		}
		else
		{
			isSucc = true;
		}
	}
	else
	{
		float failRate = (pet->getGrowth() - petcfg->getMinGrowthRefine()) / (growth * petcfg->getGrowthSuccFactor()) ;
		float succRate = 1 - sqrtf(failRate);
		
		float randnum = range_randf(0.0, 1.0);
		isSucc =  randnum <  succRate ;
	
		log<<"成功率:"<<succRate<<"随机数:"<<randnum<<"成功:"<<isSucc<<"成长变化值:";
	}
	
	// 算点数
	int point = range_randf(0.0, 1.0) < 0.5 ? 1 : 2;
	point = isSucc ? point : -point;
	
	log<<point<<"洗练前:"<<pet->getGrowth();
    
    if( isSucc )
    {
        int val = min ( (pet->getGrowth() + point) , petcfg->getMaxGrowthRefine()  + addgrowth);
        pet->setGrowth( val );
        pet->saveNewProperty();
        log<<"洗练后:"<<pet->getGrowth();
        xyerr("战斗力前: %d",pet->mBattle);
        
        pet->calculateAttributes();
        
        role->sendPetPropAndVerifycode(pet);
        
        xyerr("战斗力后: %d",pet->mBattle);
        return_sendAckPetRefineGrowth( ePetRefineGrowth_Succ );
    }
    
    
    int val = max ( (pet->getGrowth() + point)  , petcfg->getMinGrowthRefine() );
    pet->setGrowth( val );
    pet->saveNewProperty();
    log<<"洗练后:"<<pet->getGrowth();
    xyerr("战斗力前: %d",pet->mBattle);
    pet->calculateAttributes();
    
    //更新宠物验证属性
    role->sendPetPropAndVerifycode(pet);

    
    xyerr("战斗力后: %d",pet->mBattle);
    return_sendAckPetRefineGrowth( ePetRefineGrowth_Fail );
}}

// 已经参加过幻兽试炼场的幻兽列表
handler_msg(req_petelite_fightpetlist,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_petelite_fightpetlist ack;
    ack.petlist = role->getPetEliteFightedPet().toArray();
    role->send(&ack);
}}

// 选择某只幻兽来参加幻兽试炼场
handler_msg(req_enter_petelite_copy,req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    if (!checkCurrSceneType(req.copyid, stPetEliteCopy)) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    //防止客户端重复发多条消息过来
    if (0 < role->getPreEnterSceneid()) {
        if (req.oncemore == 0) {
            return;
        } else if (role->getCurrSceneId() != req.copyid) {
            return;
        } else if (role->getPreEnterSceneid() == req.copyid) {
            return;
        }
    }
    
    //判断可否进入，防止同时进入多个副本
    if (!checkCanEnterCopy(role, req.copyid)) {
        return;
    }
    
    ack_enter_petelite_copy ack;
    
    ack.copyid = req.copyid;
    
    send_if( role->getPetEliteCopyLeftTime() <= 0, CE_YOUR_ENTER_TIMES_USEUP );
    
    int count = req.list_petid.size();
    for (int i=0; i<count; i++) {
        send_if( role->mPetMgr->getPet( req.list_petid[i] ) == NULL, CE_PET_ELITE_PET_ID_ERROR );
    }
    
    //所记住的出战队列中为不同的幻兽，不能为同一只幻兽
    set<int> temp;
    for (int i=0; i<req.list_petid.size(); i++)
    {
        temp.insert(req.list_petid[i]);
    }

    string listPetidStr = StrJoin(temp.begin(), temp.end(), ";");
    role->setListPetidOrder(listPetidStr);   //记住出战队列

    role->setPreEnterSceneid(req.copyid);
    for (int i=0; i<count; i++) {
        role->mCurrentPetElitePets.push_back(req.list_petid[i]);
    }
    
    int nowtime = time(NULL);
    role->setLastEnterDungeonTime(nowtime);
    role->saveNewProperty();
    
    if (role->getDailyScheduleSystem()) {
        role->getDailyScheduleSystem()->onEvent(kDailySchedulePetElite, 0, 1);
    }
    
    send_if( true , CE_OK);
}}

// 获取幻兽出战列表
hander_msg(req_get_petidlist_copy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_get_petidlist_copy ack;
    
    StringTokenizer segListPetidStr(role->getListPetidOrder(), ";");
    for (StringTokenizer::Iterator it = segListPetidStr.begin(); it != segListPetidStr.end(); it++) {
        int pid = Utils::safe_atoi((*it).c_str());
        if (NULL == role->mPetMgr->getPet(pid)) {
            continue;
        }
         ack.list_petid.push_back(pid);
    }
    
    role->send(&ack);
}}

hander_msg(req_finish_petelite_copy, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_finish_petelite_copy ack;
    ack.copyid = req.copyid;
    
    if (role->getCurrSceneId() != req.copyid ) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    for (int i=0; i<role->mCurrentPetElitePets.size(); i++) {
        if (role->mCurrentPetElitePets[i] == 0) {
            kickAwayRoleWithTips(role, "");
            return;
        }
    }
    
    if (!checkCurrSceneType(req.copyid, stPetEliteCopy)) {
        kickAwayRoleWithTips(role, "");
        return;
    }
    
    role->backToCity();
    
    SceneCfgDef* _sceneCfg = SceneCfg::getCfg(req.copyid);
    if (_sceneCfg == NULL) {
        return;
    }
    
    // 记下参加副本的宠
    
    //可能是暂时去掉已使用的幻兽的记录 by wangzhigang
    //role->getPetEliteFightedPet().push_back( role->getCurrentPetElitePet() );
    
   // role->setCurrentPetElitePet(0);
    role->mCurrentPetElitePets.clear();
    
    int fcindex = 0;
    std::vector<int> dropindices;
    
    if (req.ispass) {
        // 要是昨天进的副本，次数不减。
        int nowtime = time(NULL);
        int lasttime = role->getLastEnterDungeonTime();
        if (isSameDay(nowtime, lasttime))
        {
            // 扣副本次数
            role->setPetEliteTimes( role->getPetEliteTimes() + 1 );
        }
        
        //扣精力
        role->addFat(-_sceneCfg->energy_consumption);
        
        notify_syn_petelitetimes nty;
        nty.times = role->getPetEliteCopyLeftTime();
        role->send(&nty);
        
        if (canInorgeBattleCheck() == false && req.deadMonsterNum != SceneCfg::getMonsterCount(_sceneCfg)) {

            kickAwayRoleWithTips(role, "怪物数目不对");
            return;
        }
        
        //玩家战斗力的验证
        int battleTime = 0;
        if (checkPlayerCompatPowerForCheat(role, req.copyid, battleTime)) {
            return;
        }
        
        UpdateQuestProgress("finishpetelitecopy", req.copyid, req.star, role, true);
        RewardStruct notifyReward;
		string awardItems;
        ack.errorcode = sendNormalCopyAward(role, _sceneCfg, req.copyid, 0,
											fcindex, dropindices, notifyReward, awardItems);
        
		// 称号检查
		SHonorMou.procHonor(eHonorKey_Copy, role, xystr("%d",req.copyid));
        
        LogMod::addLogFinishPetElite(role, req.copyid);
        
        ack.award.flopid= req.copyid;
        ack.award.findex = fcindex;
        ack.award.dropindices = dropindices;
        
        ack.award.gold = notifyReward.reward_gold;
        ack.award.battlepoint = notifyReward.reward_batpoint;
        ack.award.exp = notifyReward.reward_exp;
        ack.award.constell = notifyReward.reward_consval;
        ack.award.petexp = notifyReward.reward_petexp;
		ack.award.awardItems = awardItems;
        ack.award.enchantdust = notifyReward.reward_enchantdust;
        ack.award.enchantgold = notifyReward.reward_enchantgold;
        
        sendNetPacket(sessionid, &ack);
    }
}}

hander_msg(req_petevolution_begin_evolution, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_petevolution_begin_evolution ack;
    
    Pet* firstPet = role->mPetMgr->getPet(req.firstpetid);
    Pet* secondPet = role->mPetMgr->getPet(req.secondpetid);
    
    string leftPetLogInfo = "";
    string rightPetLogInfo = "";
    string newPetLogInfo = "";
    
    do {
        
        if (firstPet == NULL || secondPet == NULL) {
            ack.errorcode = 1;
            break;
        }
        
        if (firstPet->getSwallowLock() == true || secondPet->getSwallowLock() == true) {
            notify_disconnect_tip notify;
            notify.tip = "请解锁幻兽再执行操作";
            role->send(&notify);
            ack.errorcode = 1;
            break;
        }
        
        int firstTypeid = petModId2petEvolutionTypeid(firstPet->petmod);
        int secondTypeid = petModId2petEvolutionTypeid(secondPet->petmod);
        
        PetEvolutionNode::MaterialPair material = PetEvolutionCfgMgr::getMaterials(req.targetpetmod, firstTypeid, secondTypeid);
        
        if (material.first == NULL || material.second == NULL) {
            ack.errorcode = CE_PET_EVOLUTION_MATERIAL_ERROR;
            break;
        }
        
        ack.errorcode = checkPetEvolutionCondition(firstPet, material.first);
        if (ack.errorcode) {
            break;
        }
        
        ack.errorcode = checkPetEvolutionCondition(secondPet, material.second);
        if (ack.errorcode){
            break;
        }
        
        PetSave info;
        if (!createPetProp(role->getInstID(), req.targetpetmod, info))
        {
            ack.errorcode = CE_PET_EVOLUTION_CREATE_TARGET_PET_ERROR;
            break;
        }
        
        Pet* newPet = PetMgr::create(info);
        if (newPet == NULL) {
            ack.errorcode = CE_PET_EVOLUTION_CREATE_TARGET_PET_ERROR;
            break;
        }
        
        ItemArray evolutionRestitution;
        RewardStruct reward;
        
        vector<int> realAbsorbedIds;
        vector<Pet*> subPets;
        subPets.clear();
        
        subPets.push_back(firstPet);
        subPets.push_back(secondPet);
        
        realAbsorbedIds.clear();
        
        //补偿技能书，经验哥布林
        calPetEvolutionRestitutionItem(firstPet, material.first, reward.reward_gold, evolutionRestitution);
        
        calPetEvolutionRestitutionItem(secondPet, material.second, reward.reward_gold, evolutionRestitution);
        
        //补偿星级哥布林
        calPetEvolutionResidueStage(newPet->petmod, subPets, evolutionRestitution);
        
        if (evolutionRestitution.size() || reward.reward_gold)
        {
            sendPetEvolutionRestitutionMail(role, reward, evolutionRestitution);
        }
        
        leftPetLogInfo = makePetInfo4PetLog(firstPet);
        rightPetLogInfo = makePetInfo4PetLog(secondPet);
        
        int totalAbsorbStar = 0;
        bool bStarUpgradeSuccess = false;
        
        //升星
        PetCalStarUpgrade(newPet, subPets, bStarUpgradeSuccess,totalAbsorbStar);
        
        //升阶
        PetStageUpgrade(newPet,subPets);
        
        int addPetexp = 0;
        
        //吞噬
        for (int i = 0; i < subPets.size(); i++) {
            
            Pet* absorbed = subPets[i];
            if (absorbed == NULL)
                continue;
            
            PetCfgDef* cfg = PetCfg::getCfg(absorbed->petmod);
            
            //吞噬成功
            if (PetAbsorb(newPet, absorbed)) {
                
                //累计经验
                addPetexp += (absorbed->totalexp + cfg->getExp());
                
                //后续还有什么助阵什么的要删掉
                
                //被吞的助阵宠解除掉
                int assistPosition = role->mAssistPet->getPetAssistPos(absorbed->petid);
                if (assistPosition) {
                    
                    role->mAssistPet->unsetAssistPet(assistPosition);
                }
            }
            
            //把出战宠设置成空
            if (absorbed->isactive) {
                
                role->mActivePetId = 0;
                role->saveProp(GetRolePropName(eRoleActivePetId), role->getActivePetId());
                role->mPetMgr->setActivePet(NULL);
                
                //在宠物排行榜中删除
                role->UploadPetPaihangData(eRemove, absorbed->petid, absorbed->mBattle);
            }
            
            //delete pet*
            realAbsorbedIds.push_back(absorbed->petid);
            
            string petInfo = makePetInfo4PetLog(absorbed);
            LogMod::addLogDelPetInDB(role, absorbed->petid, absorbed->petmod, petInfo.c_str(), "pet_evolution");
            
            role->mPetMgr->removePet(absorbed->petid);
        }
        
        //玩家的属性又要重新算
        role->CalcPlayerProp();
        
        //更新战斗力排行
        role->UploadPaihangData(eUpdate);
        
        //在幻兽竞技场中删除
        SPetPvpMgr.safe_removePetsInFormation(roleid, realAbsorbedIds);
        
        //通知前端删掉宠物
        notify_delete_pet delpet;
        delpet.pets = realAbsorbedIds;
        sendNetPacket(sessionid, &delpet);
        
        if(bStarUpgradeSuccess)
            newPet->starlvl ++;
        
        role->mPetMgr->addPet(newPet);
        newPet->addExp(addPetexp, role->getLvl());
        newPet->calculateAttributes();
        
        notifyNewPet(role, newPet);
		
		role->getIllustrationsMgr()->addIllustrations(newPet->petmod);
        
        newPet->saveProp();
        newPet->saveExp();
        newPet->saveStar();
        newPet->saveNewProperty();
        
        newPetLogInfo = makePetInfo4PetLog(newPet);
        LogMod::addLogPetEvolutionInfo(role, leftPetLogInfo, rightPetLogInfo, newPetLogInfo);
        
        LogMod::addLogGetPet(roleid, role->getRolename().c_str(), role->getSessionId(), newPet->petmod, newPet->petid, "pet_evolution");
        
        UpdateQuestProgress("petevolution", -1, 1, role, true);
        
        role->mPetMgr->save();
        PetMgr::delPetsFromDB(realAbsorbedIds);
        ack.petid = newPet->petid;
        
    } while (false);
    
    sendNetPacket(sessionid, &ack);
}}

hander_msg(req_set_pet_unnew, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    
    Pet* pet = role->mPetMgr->getPet(req.petid);
    if (pet) {
        pet->setNewFlag(false);
        pet->saveNewProperty();
    }
}}

//幻兽吞噬锁
handler_msg(req_set_petswallowlock, req)
{
    hand_Sid2Role_check(sessionid, roleid, role);
    ack_set_petswallowlock ack;
    ack.petid = req.petid;
    do
    {
        if (req.petid == 0)
        {
            ack.errorcode = CE_PET_NOT_EXIST;
            break;
        }
        Pet* pet = role->mPetMgr->getPet(req.petid);
        if (pet == NULL)
        {
            ack.errorcode = CE_PET_NOT_EXIST;
            break;
        }
        
        vector<int> swallowLockStatusChange;
        
        swallowLockStatusChange.push_back(pet->getSwallowLock());
        
        pet->setSwallowLock(!pet->getSwallowLock());
        pet->PetNewProperty::save();
        
        ack.isLocked = pet->getSwallowLock();
        ack.errorcode = CE_OK;
        
        swallowLockStatusChange.push_back(pet->getSwallowLock());
        
        LogMod::addLogPetSwallowLockOperate(role, req.petid, swallowLockStatusChange);
    }while (false);

    sendNetPacket(sessionid, &ack);
    
}}
