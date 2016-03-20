//
//  FashionCollect.cpp
//  GameSrv
//
//  Created by pireszhi on 14-11-5.
//
//

#include "FashionCollect.h"
#include "GameLog.h"
#include "mail_imp.h"
#include "RoleAwake.h"
#include "EnumDef.h"
#include "Role.h"

const char* getFashionCollectDBName()
{
    return "fashioncollect";
}

const char* getFashionCollectHistoryDBName()
{
    return "fashioncollect_history";
}

void calMaterialAddProp(int materialid, int count, BattleProp& batprop)
{
    ItemCfgDef* cfg = ItemCfg::getCfg(materialid);
    
    if ( materialid <= 0 || count <= 0) {
        return;
    }
    
    if(cfg)
    {
        batprop.setAtk( batprop.getAtk() + cfg->ReadInt("atk") * count);
        batprop.setMaxHp( batprop.getMaxHp() + cfg->ReadInt("hp") * count);
        batprop.setDef(batprop.getDef() + cfg->ReadInt("def") * count);
        batprop.setDodge(batprop.getDodge() + cfg->ReadFloat("dodge") * count);
        batprop.setHit(batprop.getHit() + cfg->ReadFloat("hit") * count);
        batprop.setCri(batprop.getCri() + cfg->ReadFloat("cri") * count);
    }
}

int progressid2typeid(int progressid)
{
    return progressid / 100;
}

/*
 * FashionCollectMaterialRecycle
 */

void FashionCollectMaterialRecycle::load(int roleid)
{
    RedisResult result(redisCmd("hget role:%d %s", roleid, getKeyName()));
    string valStr = result.readStr();
    StringTokenizer token(valStr.c_str(), ";");
    
    if(token.count() >= 4)
    {
        mVersion = Utils::safe_atoi(token[0].c_str());
        mLvl = Utils::safe_atoi(token[1].c_str());
        mCurrentExp = Utils::safe_atoi(token[2].c_str());
        mTotalExp = Utils::safe_atoi(token[3].c_str());
    }
    else
    {
        mVersion = 0;
        mLvl = 0;
        mCurrentExp = 0;
        mTotalExp = 0;
    }
    
    FashionMaterialRecycleCfgDef* Cfg = FashionMaterialRecycleCfg::getCfg(mLvl);
    if (Cfg)
    {
        mPropParam = Cfg->parameter;
    }
    
}

int FashionCollectMaterialRecycle::addExp(int exp)
{
    FashionMaterialRecycleCfgDef* nextLvlCfg = FashionMaterialRecycleCfg::getCfg(mLvl + 1);
    
    if (nextLvlCfg == NULL) {
        return CE_FASHIONCOLLECT_MATERIAL_RECYCLE_SYSTEM_LVL_FULL;
    }
    
    mCurrentExp += exp;
    return lvlUp();
}

int FashionCollectMaterialRecycle::lvlUp()
{
    FashionMaterialRecycleCfgDef* nextLvlCfg = FashionMaterialRecycleCfg::getCfg(mLvl + 1);
    
    if (nextLvlCfg == NULL) {
        FashionMaterialRecycleCfgDef* currentLvlCfg = FashionMaterialRecycleCfg::getCfg(mLvl);
        check_min(mCurrentExp, currentLvlCfg->exp);
        return CE_FASHIONCOLLECT_MATERIAL_RECYCLE_SYSTEM_LVL_FULL;
    }
    
    if (mCurrentExp < nextLvlCfg->exp) {
        return CE_OK;
    }
    
    mCurrentExp -= nextLvlCfg->exp;
    mLvl++;
    mPropParam = nextLvlCfg->parameter;
    
    mTotalExp += nextLvlCfg->exp;
    
    return lvlUp();
}



/********************
 FashionCollectObject
 *******************/

void FashionCollectObject::setMaterial(string str)
{
    StringTokenizer token(str, ";");
    for(int i = 0; i < token.count(); i++)
    {
        StringTokenizer material(token[i], "*");
        if(material.count() < 2)
        {
            continue;
        }
        
        if(i < mMaterials.size())
        {
            int materialid = Utils::safe_atoi(material[0].c_str());
            int materialCount = Utils::safe_atoi(material[1].c_str());
            
            if (materialid <= 0) {
                continue;
            }
            
            //每次启动都检查配置中材料的位置跟存储的是否相同
            TargetOfFashionCollectMaterial* materialInfo = FashionCollectCfg::getMaterialInfowithTarget(materialid, mFashionObjectId);
            
            if (materialInfo == NULL || materialInfo->grid < 0 || materialInfo->grid >= mMaterials.size()) {
                
                log_error("fashioncollection materal can find grid position. targetid:"<<mFashionObjectId<<", materialid:"<<materialid);
                return;
            }
            
            if (materialInfo->count > materialCount) {
                
                int residual = materialCount - materialInfo->count;
                materialCount = materialInfo->count;
            }
            
            mMaterials[materialInfo->grid].item = materialid;
            mMaterials[materialInfo->grid].count = materialCount;
            
            calMaterialAddProp(materialid, materialCount, mPropAdd);
        }
    }
}

void FashionCollectObject::addNewMaterial(int grid, int materialid, int& count)
{
    if(grid < 0)
    {
        return;
    }
    
    FashionCollectCfgDef* cfg = FashionCollectCfg::getCfg(mFashionObjectId);
    if(cfg == NULL)
    {
        return;
    }
    
    if(cfg->materialIds.size() <= grid)
    {
        return;
    }
    
    if(cfg->materialIds[grid].itemid != materialid)
    {
        return;
    }
    
    int requireCount = cfg->materialIds[grid].count;
    
    mMaterials[grid].item = materialid;
    
    int remainCount = requireCount - mMaterials[grid].count;
    
    check_min(remainCount, 0);
    
    check_max(remainCount, count);
    
    if(remainCount == 0)
    {
        return;
    }
    
    count -= remainCount;
    
    calMaterialAddProp(materialid, remainCount, mPropAdd);
    
    while(remainCount)
    {
        mMaterials[grid].count++;
        remainCount--;
    }
    
    return;

}

string FashionCollectObject::tostring()
{
    stringstream str;
//    str<<mFashionObjectId<<":";
    for(int i = 0; i < mMaterials.size(); i++)
    {
        str<<mMaterials[i].item<<"*"<<mMaterials[i].count<<";";
    }
    return str.str();
}

void FashionCollectObject::save(int roleid)
{
    RedisArgs args;
    string cmdHead = Utils::makeStr("%s:%d", getFashionCollectDBName(), roleid);
    RedisHelper::beginHmset(args, cmdHead.c_str());
    
    RedisHelper::appendHmset(args, mFashionObjectId, tostring().c_str());
    RedisHelper::commitHmset(get_DbContext(), args);
    
    return;
}

/**************
 FashionCollect
 *************/

FashionCollect* FashionCollect::Create(Role* role)
{
    FashionCollect* collection = new FashionCollect;
    
    collection->mMaster = role;
    collection->mMasterId = role->getInstID();
    
    return collection;
}

void FashionCollect::loadData()
{
    RedisResult result(redisCmd("hgetall %s:%d", getFashionCollectDBName(), mMasterId));
    for (int i = 0; i < result.getHashElements(); i += 2) {
        
        int progressId = result.readHash(i, 0);
        string material = result.readHash(i + 1, "");
        
        addNewProgressAndSave(progressId, material);
        
        if (checkAdvanceCondition(progressId) == CE_OK) {
            mPreFinishId.push_back(progressId);
        }
    }

    //加载已完成的进阶记录
    loadPreCollection();
    
    mMaterialRecycle->load(mMasterId);
    
    checkSystemAddNewCollection();
    
    //计算各种属性加成
//    calTotalProperty();
    
    //加载材料盒子
    mMaterialBox->Load(mMasterId);
    
    mMaster->CalcPlayerProp();
}

bool FashionCollect::finishProgressFailLastTime()
{
    for (int i = 0; i < mPreFinishId.size(); i++) {
        
        int progressid = mPreFinishId[i];
        
        ItemArray items;
        
        if (finishProgress(progressid) == CE_OK) {
            items = getFashionCollectProduct(progressid, mMaster->getRoleType());
        }
        else
        {
            continue;
        }
        
        GridArray effectGrid;
        if (mMaster->preAddItems(items, effectGrid) == CE_OK) {
            mMaster->playerAddItemsAndStore(effectGrid, items, "fashioncollect", true);
        }
        else
        {
            RewardStruct rewards;
            vector<string> attachs;
            reward2MailFormat(rewards, items, NULL, &attachs);
            string attach = StrJoin(attachs.begin(), attachs.end(), ";");
            
            sendMailWithName(0, "", mMaster->getRolename().c_str(), "bag_full", attach.c_str());
        }
    }
    
    mPreFinishId.clear();
    
    return true;
}

void FashionCollect::loadPreCollection()
{
    RedisResult result(redisCmd("hgetall %s:%d", getFashionCollectHistoryDBName(), mMasterId));
    for (int i = 0; i < result.getHashElements(); i += 2) {
        
        int progressTypeId = result.readHash(i, 0);
        string history = result.readHash(i + 1, "");
        
        if (progressTypeId <= 0) {
            continue;
        }
        
        PreCollection preCollection;
        preCollection.clear();
        
        int maxlvlProgress = 0;
        StringTokenizer token(history, ";");
        for (int i = 0; i < token.count(); i++) {
            int progress = Utils::safe_atoi(token[i].c_str());
            
            if (progress <= 0) {
                continue;
            }
            
            FashionCollectCfgDef* cfg = FashionCollectCfg::getCfg(progress);
            
            if (cfg == NULL) {
                continue;
            }
            
            //为了检查在原有最高阶再增加阶数的情况
            if (maxlvlProgress < progress) {
                maxlvlProgress = progress;
            }
            
            preCollection.push_back(progress);
        }
        
        if (preCollection.size() > 0) {
            mPreCollection.insert(make_pair(progressTypeId, preCollection));
        }
        
        if (preCollection.size() <= 0) {
            doRedisCmd("hdel %s:%d %d", getFashionCollectHistoryDBName(), mMasterId, progressTypeId);
        }
        else if (preCollection.size() != token.count()) {
            savePreCollection(progressTypeId, preCollection);
        }
        
        if (maxlvlProgress > 0) {
            
            int nextprogress = maxlvlProgress + 1;
            map<int, FashionCollectObject>::iterator iter = mCollection.find(nextprogress);
            
            //如果没有下一阶的数据再判断配置表有没有下一阶
            if (iter == mCollection.end()) {
                
                FashionCollectCfgDef* cfg = FashionCollectCfg::getCfg(nextprogress);
                if (cfg != NULL) {
                
                    FashionCollectObject collectObj(nextprogress);
                    
                    collectObj.type = cfg->fashionType;
                    collectObj.stage = cfg->stage;
                    mCollection.insert(make_pair(nextprogress, collectObj));
                    
                    collectObj.save(mMasterId);
                }
            }
        }
    }
}

//本来在自动放置的版本才调用，现在不用
void FashionCollect::addPlayerMaterial(int materialId, int count)
{
    FashionCollectMaterialInfo targets = FashionCollectCfg::getMaterialTargets(materialId);
    for (int i = 0; i < targets.size(); i++) {
        
        TargetOfFashionCollectMaterial* target = targets[i];
        
        if (target != NULL) {
            
            addProgressMaterial(target->targetid, target->grid, materialId, count);
            
            if (count <= 0) {
                break;
            }
        }
    }
    
    calCurrentProgressProp();
    
    //有剩余放到材料盒子里
    if (count) {
        ItemGroup material;
        material.item = materialId;
        material.count = count;
        
        ItemArray items;
        items.push_back(material);
        
        GridArray effgrids;
        bool canAdd = mMaterialBox->PreAddItems(items, effgrids);
        if (canAdd) {
            mMaterialBox->UpdateBackBag(effgrids);
            mMaterialBox->Store(mMasterId, effgrids);
        }
    }
    
}

int FashionCollect::putMaterialIntoGrid(int materialId, int count, int targetProgress, int grid)
{
    int myCount = mMaterialBox->GetItemNum(materialId);
    
    if (myCount < count) {
        return CE_FASHIONCOLLECT_MATERIAL_NOT_ENOUGH;
    }
    
    FashionCollectCfgDef* progressCfg = FashionCollectCfg::getCfg(targetProgress);
    if (progressCfg == NULL) {
        return CE_FASHIONCOLLECT_READ_CFG_ERROR;
    }
    
    if (grid >= progressCfg->materialIds.size()) {
        return CE_FASHIONCOLLECT_MATERIAL_GRID_ERROR;
    }
    
    if (progressCfg->materialIds[grid].itemid != materialId) {
        return CE_FASHIONCOLLECT_MATERIAL_ERROR_FOR_THIS_GRID;
    }
    
    int remaincount = count;
    
    if (!addProgressMaterial(targetProgress, grid, materialId, remaincount)) {
        return CE_FASHIONCOLLECT_NOT_FIND_THIS_PROGRESS;
    }
    
    ItemArray material;
    material.push_back(ItemGroup(materialId, count - remaincount));
    
    GridArray effeGrid;
    
    if (!mMaterialBox->PreDelItems(material, effeGrid)) {
        return CE_FASHIONCOLLECT_MATERIAL_NOT_ENOUGH;
    }
    
    mMaterialBox->UpdateBackBag(effeGrid);
    mMaterialBox->Store(mMasterId, effeGrid);
    
    sendMaterialBoxNotify(effeGrid);
    sendPropAddNotify();
    
    return CE_OK;
}

bool FashionCollect::addProgressMaterial(int targetProgress, int grid, int materialId, int& count)
{
    map<int, FashionCollectObject>::iterator iter = mCollection.find(targetProgress);
    if (iter != mCollection.end()) {
        int addcount = count;
        
        iter->second.addNewMaterial(grid, materialId, count);
        iter->second.save(mMasterId);
        
        LogMod::addLogFashionCollectPutOnMaterial(mMaster, targetProgress, materialId, grid, addcount - count, iter->second.tostring());
        
        return true;
    }
    
    return false;
}

void FashionCollect::addPropByRatio(float ratio)
{
    mTotalBattlePropAdded *= (1 + ratio);
}

void FashionCollect::calTotalProperty()
{
    mTotalBattlePropAdded = BattleProp();
    
    //计算收集历史所带来的额外属性
    calHistoryCollectProperty();
    
    //下面计算当前的属性
    calCurrentProgressProp();
    
    mTotalBattlePropAdded += mHistoryAddBattleProp;
    mTotalBattlePropAdded += mCurrentProgressAddBattleProp;
    
    FashionMaterialRecycleCfgDef* recycleCfg = FashionMaterialRecycleCfg::getCfg(mMaterialRecycle->mLvl);
    
    float param = 0.0f;
    if (recycleCfg) {
        param = recycleCfg->parameter;
    }
    
    mTotalBattlePropAdded *= (1 + param);
    
    //觉醒附加的属性
    if (mMaster->getRoleAwake()) {
        addPropByRatio(mMaster->getRoleAwake()->getFashionPropAddRatio());
    }
}

void FashionCollect::calHistoryCollectProperty()
{
    mHistoryAddBattleProp = BattleProp();
    map<int, PreCollection>::iterator iter = mPreCollection.begin();
    map<int, PreCollection>::iterator endIter = mPreCollection.end();
    
    //计算收集历史所带来的额外属性
    for (; iter != endIter; iter++) {
        onCalHistoryCollectProperty(iter->second, mHistoryAddBattleProp);
    }
}

void FashionCollect::onCalHistoryCollectProperty(PreCollection &precollect, BattleProp &batProp)
{
    for (int i = 0; i < precollect.size(); i++) {
        
        FashionCollectCfgDef* cfg = FashionCollectCfg::getCfg(precollect[i]);
        
        if (cfg == NULL) {
            continue;
        }
        
        batProp += cfg->mMaterialPropAdded;
        
        batProp += cfg->mPropAward;
    }
}

void FashionCollect::calCurrentProgressProp()
{
    mCurrentProgressAddBattleProp = BattleProp();
    
    map<int, FashionCollectObject>::iterator iter = mCollection.begin();
    map<int, FashionCollectObject>::iterator endIter = mCollection.end();
    
    for (; iter != endIter; iter++) {
        mCurrentProgressAddBattleProp += iter->second.mPropAdd;
    }
}

void FashionCollect::addPlayerTotalProperty(BattleProp &batprop)
{
    calTotalProperty();
    batprop += mTotalBattlePropAdded;
}

const ItemArray& FashionCollect::getMaterialBoxInfo()
{
    vector<int> effgrids;
    mMaterialBox->Sort(effgrids);
    mMaterialBox->Store(mMasterId, effgrids);
    return mMaterialBox->GetItems();
}

int FashionCollect::finishProgress(int progressId)
{
    if (checkAdvanceCondition(progressId)) {
        return 1;
    }
    
    updatePreCollection(progressId);
    
    int newProgressId = progressId + 1;
    
    addNewProgressAndSave(newProgressId);
    
    LogMod::addLogFashionCollectFinishProgress(mMaster, progressId, 0, 0);
    
    //重新算属性加成
    sendPropAddNotify();
    
    return CE_OK;
}

void FashionCollect::addNewProgressAndSave(int newProgressId)
{
    FashionCollectCfgDef* newCfg = FashionCollectCfg::getCfg(newProgressId);
    
    if (newCfg == NULL)
    {
        return;
    }
    
    FashionCollectObject newObject(newProgressId);
    newObject.stage = newCfg->stage;;
    newObject.type = newCfg->fashionType;
    
//    //检查背包是否有所需的材料
//    GridArray effeGrids;
//    
//    for (int i = 0; i < newCfg->materialIds.size(); i++) {
//        
//        int materialId =newCfg->materialIds[i].itemid;
//        int materialCount = mMaterialBox->GetItemNum(materialId);
//        
//        if (materialCount <= 0) {
//            continue;
//        }
//        
//        int requireCount = newCfg->materialIds[i].count;
//        check_max(requireCount, materialCount);
//        
//        int delCount = materialCount;
//        
//        check_max(delCount, requireCount);
//        
//        check_min(delCount, 0);
//        
//        ItemGroup item;
//        item.item = materialId;
//        item.count = delCount;
//        
//        ItemArray items;
//        
//        items.push_back(item);
//        
//        if (mMaterialBox->PreDelItems(items, effeGrids)) {
//            
//            mMaterialBox->UpdateBackBag(effeGrids);
//            
//            newObject.mMaterials[i] = item;
//            
//            //增加材料所带来的属性
//            calMaterialAddProp(materialId, delCount, newObject.mPropAdd);
//        }
//    }
    
    mCollection.insert(make_pair(newProgressId, newObject));
    
//    mMaterialBox->Store(mMasterId, effeGrids);
    newObject.save(mMasterId);
    
    //有通知前端增加新的收集过程
    notify_fashioncollect_add_newprogress notify;
    
    if(newCfg->stage == 1)
    {
        notify.preProgressid = 0;
    }
    else{
        notify.preProgressid = newProgressId - 1;
    }
    
    onClientGetCollectionInfo(newObject, notify.newCollectInfo);
    
    sendNetPacket(mMaster->getSessionId(), &notify);
}

void FashionCollect::addNewProgressAndSave(int newProgressId, string materialStr)
{
    //如果现有的配置表没找到，就不加载了，但保留在数据库
    FashionCollectCfgDef* cfg = FashionCollectCfg::getCfg(newProgressId);
    if (cfg == NULL) {
        return;
    }
    
    FashionCollectObject collectObj(newProgressId);
    
    collectObj.type = cfg->fashionType;
    collectObj.stage = cfg->stage;
    
    StringTokenizer token(materialStr, ";");
    bool reSave = false;
    
    for(int i = 0; i < token.count(); i++)
    {
        StringTokenizer material(token[i], "*");
        if(material.count() < 2)
        {
            continue;
        }
        if(i < collectObj.mMaterials.size())
        {
            int materialid = Utils::safe_atoi(material[0].c_str());
            int materialCount = Utils::safe_atoi(material[1].c_str());
            
            if (materialid <= 0) {
                continue;
            }
            
            //每次启动都检查配置中材料的位置跟存储的是否相同
            TargetOfFashionCollectMaterial* materialInfo = FashionCollectCfg::getMaterialInfowithTarget(materialid, newProgressId);
            
            if (materialInfo == NULL || materialInfo->grid < 0 || materialInfo->grid >= collectObj.mMaterials.size()) {
                
                log_error("fashioncollection materal can find grid position. targetid:"<<newProgressId<<", materialid:"<<materialid);
                return;
            }
            
            int residual = materialCount - materialInfo->count;
            
            if (residual > 0) {
                
                materialCount = materialInfo->count;
                
                ItemGroup residualMaterials(materialid, residual);
                ItemArray items;
                items.push_back(residualMaterials);
                
                GridArray effeGrid;
                mMaterialBox->PreAddItems(items, effeGrid);
                mMaterialBox->UpdateBackBag(effeGrid);
                mMaterialBox->Store(mMasterId, effeGrid);
                
                reSave = true;
                
            }
            
            collectObj.mMaterials[materialInfo->grid].item = materialid;
            collectObj.mMaterials[materialInfo->grid].count = materialCount;
            
            calMaterialAddProp(materialid, materialCount, collectObj.mPropAdd);
        }
    }
    
    if (reSave) {
        collectObj.save(mMasterId);
    }
    mCollection.insert(make_pair(newProgressId, collectObj));
}

void FashionCollect::delProgress(int progressid)
{
    map<int, FashionCollectObject>::iterator iter = mCollection.find(progressid);
    
    if (iter != mCollection.end()) {
        mCollection.erase(iter);
    }
    
    //在数据库删除
    RedisArgs args;
    string cmdHead = Utils::makeStr("%s:%d", getFashionCollectDBName(), mMasterId);
    RedisHelper::beginHdel(args, cmdHead.c_str());
    RedisHelper::appendHdel(args, progressid);
    RedisHelper::commitHdel(get_DbContext(), args);
    
    //
    notify_fashioncollect_delete_progress notify;
    notify.progressid = progressid;
    sendNetPacket(mMaster->getSessionId(), &notify);
}

void FashionCollect::updatePreCollection(int lastestCollectId)
{
    FashionCollectCfgDef* cfg = FashionCollectCfg::getCfg(lastestCollectId);
    
    if (cfg != NULL) {
        mHistoryAddBattleProp += cfg->mPropAward;
    }
    
    //在进度中删除已完成的
    delProgress(lastestCollectId);
    
    int index = lastestCollectId / 100;
    
    map<int, PreCollection>::iterator iter = mPreCollection.find(index);
    
    if (iter == mPreCollection.end()) {
        PreCollection newCollection;
        newCollection.push_back(lastestCollectId);
        mPreCollection.insert(make_pair(index, newCollection));
        
        savePreCollection(index, newCollection);
    }
    else
    {
        iter->second.push_back(lastestCollectId);
        
        savePreCollection(index, iter->second);
    }
}

void FashionCollect::savePreCollection(int index, PreCollection& collect)
{
    RedisArgs args;
    string key = Utils::makeStr("%s:%d", getFashionCollectHistoryDBName(), mMasterId);
    RedisHelper::beginHmset(args, key.c_str());
    stringstream ostr;
    for (int i = 0; i < collect.size(); i++) {
        ostr<<collect[i]<<";";
    }
    RedisHelper::appendHmset(args, index, ostr.str().c_str());
    RedisHelper::commitHmset(get_DbContext(), args);
}

int FashionCollect::checkAdvanceCondition(int progressid)
{
    map<int, FashionCollectObject>::iterator iter = mCollection.find(progressid);
    
    if (iter == mCollection.end()) {
        return CE_FASHIONCOLLECT_NOT_FIND_THIS_PROGRESS;
    }
    
    FashionCollectObject& currentProgress = iter->second;
    
    FashionCollectCfgDef* cfg = FashionCollectCfg::getCfg(progressid);
    
    if (cfg == NULL) {
        return CE_FASHIONCOLLECT_READ_CFG_ERROR;
    }
    
    for (int i = 0; i < cfg->materialIds.size(); i++) {
        
        if (currentProgress.mMaterials[i].item != cfg->materialIds[i].itemid) {
            return CE_FASHIONCOLLECT_MATERIAL_ERROR_FOR_THIS_GRID;
        }
        
        if (currentProgress.mMaterials[i].count < cfg->materialIds[i].count) {
            return CE_FASHIONCOLLECT_MATERIAL_FOR_THIS_PROGRES_WITHOUT_COMLETION;
        }
    }
    
    for (int i = 0; i < cfg->shareMaterialIds.size(); i++) {
        int materialId = cfg->shareMaterialIds[i].itemid;
        int itemCount = mMaterialBox->GetItemNum(materialId);
        
        if (itemCount < cfg->shareMaterialIds[i].count) {
            return CE_FASHIONCOLLECT_MATERIAL_FOR_THIS_PROGRES_WITHOUT_COMLETION;
        }
    }
    
    return CE_OK;
}

void FashionCollect::checkSystemAddNewCollection()
{
    vector<FashionCollectCfgDef*>& firstStages = FashionCollectCfg::getFirstStageIds();
    
    RedisArgs args;
    string cmdHead = Utils::makeStr("%s:%d", getFashionCollectDBName(), mMasterId);
    RedisHelper::beginHmset(args, cmdHead.c_str());
    bool hasNew = false;
    
    for (int i = 0; i < firstStages.size(); i++) {
        FashionCollectCfgDef* firstDef = firstStages[i];
        
        if (firstDef == NULL) {
            continue;
        }
        
        int progIndex = firstDef->fashionObjectId / 100;
        
        //历史记录有这类型的跳过
        map<int, PreCollection>::iterator preIter = mPreCollection.find(progIndex);
        if (preIter != mPreCollection.end()) {
            continue;
        }
        
        //历史记录没有这类型，但当前有的跳过
        map<int, FashionCollectObject>::iterator currentIter = mCollection.find(firstDef->fashionObjectId);
        if (currentIter != mCollection.end()) {
            continue;
        }
        
        //系统新加的一种收集路线
        FashionCollectObject newObject(firstDef->fashionObjectId);
        newObject.type = firstDef->fashionType;
        newObject.stage = firstDef->stage;
        mCollection.insert(make_pair(firstDef->fashionObjectId, newObject));
        
        RedisHelper::appendHmset(args, firstDef->fashionObjectId, newObject.tostring().c_str());
        hasNew = true;
    }
    
    if (hasNew) {
        
        RedisHelper::commitHmset(get_DbContext(), args);
        
    }
}

bool FashionCollect::finishProgressUnconditional(int targetProgressid, vector<int>& progressIdFinshed)
{
    int index = targetProgressid / 100;
    map<int, PreCollection>::iterator iter = mPreCollection.find(index);
    
    int totalStage = targetProgressid % 100;
    
    if (iter == mPreCollection.end()) {
        
        for (int i = 1; i <= totalStage; i++) {
            
            int finishProgress = index * 100 + i;
            
            updatePreCollection(finishProgress);
        }
        
        int newProgressId = targetProgressid + 1;
        
        addNewProgressAndSave(newProgressId);
    }
    else
    {
        int historyCount = iter->second.size();
        if (historyCount <= 0) {
            return false;
        }

        int latestProgress = iter->second[historyCount - 1];
        int latestMaxStage = latestProgress % 100;
        
        int remainStages = totalStage - latestMaxStage;
        
        if (latestMaxStage <= 0 || remainStages <= 0) {
            return true;
        }
        
        for (int i = 1; i < remainStages; i++) {
            
            int finishProgress = latestProgress + i;
            
            updatePreCollection(finishProgress);
            
            progressIdFinshed.push_back(finishProgress);
        }
        
        int nextProgresssId = targetProgressid + 1;
        addNewProgressAndSave(nextProgresssId);
    }
    return true;
}

void FashionCollect::playerGetNewFashionid(int newFashionid)
{
    ItemCfgDef* itemCfg = ItemCfg::getCfg(newFashionid);
    if (itemCfg == NULL) {
        return;
    }
    
    int progressid = itemCfg->ReadInt("fashioncollect_id");
    
    vector<int> progressFinished;
    progressFinished.clear();
    
    if(progressid >= 0)
    {
        finishProgressUnconditional(progressid, progressFinished);
        sendPropAddNotify();
    }
    
    for (int i = 0; i < progressFinished.size(); i++) {
        LogMod::addLogFashionCollectFinishProgress(mMaster, progressFinished[i], 1, newFashionid);
    }
}

void FashionCollect::saveAllCollection()
{
    map<int, FashionCollectObject>::iterator iter = mCollection.begin();
    map<int, FashionCollectObject>::iterator endIter = mCollection.end();
    
    RedisArgs args;
    string cmdHead = Utils::makeStr("%s:%d", getFashionCollectDBName(), mMasterId);
    RedisHelper::beginHmset(args, cmdHead.c_str());
    
    for (; iter != endIter; iter++) {
         RedisHelper::appendHmset(args, iter->first, iter->second.tostring().c_str());
    }
    
    if (mCollection.size() > 0) {
        RedisHelper::commitHmset(get_DbContext(), args);
    }
    
}

void FashionCollect::clientGetCollectionList(vector<obj_fashionCollect_info>& info)
{
    map<int, FashionCollectObject>::iterator iter = mCollection.begin();
    map<int, FashionCollectObject>::iterator endIter = mCollection.end();
    
    for (; iter != endIter; iter++)
    {
        obj_fashionCollect_info obj;
        onClientGetCollectionInfo(iter->second, obj);
        info.push_back(obj);
    }
}

void FashionCollect::clientGetLatestCollectionId(vector<int>& preProgress)
{
    map<int, PreCollection>::iterator iter = mPreCollection.begin();
    map<int, PreCollection>::iterator endIter = mPreCollection.end();
    
    for (; iter != endIter; iter++)
    {
        int vsize = iter->second.size();
        
        if (vsize) {
            preProgress.push_back(iter->second[vsize - 1]);
        }
    }
}

void FashionCollect::clientGetCollectionInfo(int progressid, obj_fashionCollect_info &info)
{
    map<int, FashionCollectObject>::iterator iter = mCollection.find(progressid);
    
    if (iter != mCollection.end()) {

        onClientGetCollectionInfo(iter->second, info);
    }
}

void FashionCollect::onClientGetCollectionInfo(const FashionCollectObject& object, obj_fashionCollect_info& info)
{
    info.ObjectId = object.mFashionObjectId;
    info.type = object.type;
    info.stage = object.stage;
    
    for (int i = 0; i < object.mMaterials.size(); i++) {
        
        const ItemGroup& material = object.mMaterials[i];
        info.materials.push_back(material.item);
        info.count.push_back(material.count);
    }
}

void FashionCollect::sendMaterialBoxNotify(const GridArray& effeGrids)
{
    if (mMaster == NULL) {
        return;
    }
    
    if (effeGrids.size() <= 0) {
        return;
    }
    
    notify_fashioncollect_materialbox notify;
    
    for (int i = 0; i < effeGrids.size(); i++)
    {
        obj_addordelItem item;
        item.deloradd = (effeGrids[i].item.item > 0 ? 1 : 0);
        item.index = effeGrids[i].index;
        item.itemid = effeGrids[i].item.item;
        item.bind = effeGrids[i].item.bind;
        item.count = effeGrids[i].item.count;
        item.json = effeGrids[i].item.json;
        
        notify.items.push_back(item);
    }
    
    sendNetPacket(mMaster->getSessionId(), &notify);
}

void FashionCollect::sendPropAddNotify()
{
    ack_fashionCollect_prop_added ack;
    
    calTotalProperty();
    
    clientGetRecycleInfoAndPropAdded(ack);
    
    sendNetPacket(mMaster->getSessionId(), &ack);
}

int FashionCollect::materialReclaim(vector<int>& grids)
{
    int oldLvl = mMaterialRecycle->mLvl;
    
    int vsize = grids.size();
    GridArray effeGrids;

    int errorcode = CE_OK;
    
    if (vsize <= 0) {
        return CE_FASHIONCOLLECT_MATERIAL_NOT_EXIST;
    }
    
    for (int i = 0; i < vsize; i++) {
        
        ItemGroup items = mMaterialBox->GetItem(grids[i]);
        
        int itemId = items.item;
        int recycleCount = items.count;
        
        if(items.item <= 0)
        {
            continue;
        }
        
        errorcode = onMaterialReclaim(items.item, items.count);
        
        recycleCount -= items.count;
        
        BagGrid newGrid;
        
        if (items.count <= 0) {
            items = ItemGroup(0, 0);
        }
        
        newGrid.item = items;
        newGrid.index = grids[i];
        effeGrids.push_back(newGrid);
        
        if (recycleCount) {
            LogMod::addLogFashionCollectMaterialRecycle(mMaster, itemId, recycleCount, mMaterialRecycle->mLvl, mMaterialRecycle->mCurrentExp);
        }
        
        if (errorcode) {
            break;
        }
    }
    
    mMaterialBox->UpdateBackBag(effeGrids);
    mMaterialBox->Store(mMasterId, effeGrids);
    sendMaterialBoxNotify(effeGrids);

    mMaterialRecycle->save(mMasterId);
    
    if (oldLvl != mMaterialRecycle->mLvl) {
        
        sendPropAddNotify();
        mMaster->CalcPlayerProp();
    }
    
    return errorcode;
}

int FashionCollect::materialReclaim(int gridIndex, int count)
{
    int oldLvl = mMaterialRecycle->mLvl;
    
    ItemGroup items = mMaterialBox->GetItem(gridIndex);

    if (items.item <= 0) {
        return CE_FASHIONCOLLECT_MATERIAL_NOT_EXIST;
    }
    
    if (items.count < count) {
        return CE_FASHIONCOLLECT_MATERIAL_NOT_ENOUGH;
    }
    
    GridArray effeGrids;
    int requestCount = count;
    
    //先扣掉材料
    items.count -= requestCount;
    int errorcode = onMaterialReclaim(items.item, requestCount);

    //如果没扣完，把没扣的加回去
    items.count += requestCount;
    
    if (requestCount < count) {
        LogMod::addLogFashionCollectMaterialRecycle(mMaster, items.item, count - requestCount, mMaterialRecycle->mLvl, mMaterialRecycle->mCurrentExp);
    }
    
    BagGrid newgrid;
    
    if (items.count <= 0) {
        items = ItemGroup(0, 0);
    }
    
    newgrid.item = items;
    newgrid.index = gridIndex;
    effeGrids.push_back(newgrid);
    
    mMaterialBox->UpdateBackBag(effeGrids);
    mMaterialBox->Store(mMasterId, effeGrids);
    
    sendMaterialBoxNotify(effeGrids);
    
    mMaterialRecycle->save(mMasterId);
    
    if (oldLvl != mMaterialRecycle->mLvl) {
        
        sendPropAddNotify();
        mMaster->CalcPlayerProp();
    }
    
    return errorcode;
}

int FashionCollect::onMaterialReclaim(int materialId, int& count)
{
    int errorcode = CE_OK;

    ItemCfgDef* cfg = ItemCfg::getCfg(materialId);
    if (cfg == NULL) {
        return CE_FASHIONCOLLECT_READ_CFG_ERROR;
    }
    
    int expAdd = cfg->ReadInt("reclaimExp");
    
    while (count) {
        
        errorcode = mMaterialRecycle->addExp(expAdd);
        
        if (errorcode) {
            break;
        }else{
            count--;
        }
        
    }
    
    return errorcode;
}

void FashionCollect::clientGetRecycleInfoAndPropAdded(ack_fashionCollect_prop_added& ack)
{
    ack.atk = mTotalBattlePropAdded.getAtk();
    ack.def = mTotalBattlePropAdded.getDef();
    ack.maxHp = mTotalBattlePropAdded.getMaxHp();
    ack.hit = mTotalBattlePropAdded.getHit();
    ack.dodge = mTotalBattlePropAdded.getDodge();
    ack.cri = mTotalBattlePropAdded.getCri();
    
    ack.recycleLvl = mMaterialRecycle->mLvl;
    ack.recycleExp = mMaterialRecycle->mCurrentExp;
    ack.recyclePropParam = mMaterialRecycle->mPropParam;
    
}

//static
ItemArray FashionCollect::getFashionCollectProduct(int progressId, int roleType)
{
    FashionCollectCfgDef* cfg = FashionCollectCfg::getCfg(progressId);
    
    ItemArray items;
    
    if (cfg == NULL) {
        return items;
    }
    
    ObjJob roleJob = RoleType2Job(roleType);
    ObjSex roleSex = RoleType2Sex(roleType);
    
    for (int i =0; i<cfg->fashionProductId.size(); i++)
    {
        ItemCell& product = cfg->fashionProductId[i];
        
        ItemCfgDef* itemcfg = ItemCfg::getCfg(product.itemid);
        
        ObjJob job = (ObjJob)itemcfg->ReadInt("job", eAllJob);
        ObjSex sex = (ObjSex) itemcfg->ReadInt("sex",eAllSex);
        
        if((job == roleJob || job == eAllJob) && (sex == roleSex || sex == eAllSex))
        {
            ItemGroup item(product.itemid, product.count);
            items.push_back(item);
        }
    }
    
    for (int i = 0; i < cfg->awardGift.size(); i++) {
        
        ItemCell gift = cfg->awardGift[i];
        
        ItemGroup giftItem(gift.itemid, gift.count);
        items.push_back(giftItem);
    }
    
    return items;
}
