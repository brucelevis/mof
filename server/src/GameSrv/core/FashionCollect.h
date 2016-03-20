//
//  FashionCollect.h
//  GameSrv
//
//  Created by pireszhi on 14-11-5.
//
//

#ifndef __GameSrv__FashionCollect__
#define __GameSrv__FashionCollect__

#include <iostream>
#include "ItemBag.h"
#include "redis_helper.h"
#include "main.h"
#include "Defines.h"
#include "msg.h"
class Role;

static const char* getFashionCollectDBName();
static const char* getFashionCollectHistoryDBName();

void calMaterialAddProp(int materialid, int count, BattleProp& batprop);

//时装收集的对象或者说是目标
struct FashionCollectObject
{
    int mFashionObjectId;
    FashionCollectType type;
    int stage;
    ItemArray mMaterials;
    BattleProp mPropAdd;
    
    FashionCollectObject(int fashionid):mFashionObjectId(fashionid)
    {
        ItemGroup item;
        mMaterials.resize(FASHIONCOLLECT_MAX_GRID_FOR_MATERIAL, item);
    }
    
    FashionCollectObject(const FashionCollectObject& object)
    {
        mFashionObjectId = object.mFashionObjectId;
        type = object.type;
        stage = object.stage;
        mMaterials = object.mMaterials;
        mPropAdd = object.mPropAdd;
    }
    
    FashionCollectObject operator=(const FashionCollectObject& src)
    {
        if (this != &src)
        {
            mFashionObjectId = src.mFashionObjectId;
            mMaterials = src.mMaterials;
            type = src.type;
            stage = src.stage;
            mPropAdd = src.mPropAdd;
        }
        
        return *this;
    }
    
    void setMaterial(string str);
    
    void addNewMaterial(int grid, int materialid, int& count);
    
    void save(int roleid);
    
    string tostring();
};

struct FashionCollectMaterialRecycle
{
    FashionCollectMaterialRecycle():mVersion(0),
                                    mLvl(0),
                                    mCurrentExp(0),
                                    mTotalExp(0),
                                    mPropParam(0.0f)
    {
        
    }
    
    void load(int roleid);
    
    void save(int roleid)
    {
        doRedisCmd("hset role:%d %s %s", roleid, getKeyName(), tostring().c_str());
    }
    
    int addExp(int exp);
    
    int lvlUp();
    
    const char* getKeyName(){return "fashionmaterialrecycle";}

    string tostring()
    {
        stringstream ostr;
        ostr<<mVersion<<";";
        ostr<<mLvl<<";";
        ostr<<mCurrentExp<<";";
        ostr<<mTotalExp<<";";
        return ostr.str();
    }
    
    int mVersion;
    int mLvl;
    int mCurrentExp;
    int mTotalExp;
    
    float mPropParam;
};

class FashionCollect
{
public:
    
    typedef vector<int> PreCollection;

    FashionCollect():mMasterId(0), mMaster(NULL)
    {
        mMaterialBox = MaterialBoxForFashion::Create(FASHIONCOLLECT_LIMIT_SIZE_OF_MATERIALBOX);
        mMaterialRecycle = new FashionCollectMaterialRecycle;
    }
    
    ~FashionCollect()
    {
        delete mMaterialBox;
        delete mMaterialRecycle;
    }
    
public:
    
    static FashionCollect* Create(Role* role);
    
    void loadData();
    
    void playerGetNewFashionid(int newFashionid);
    
    void addPlayerTotalProperty(BattleProp& batprop);
    
    void clientGetCollectionList(vector<obj_fashionCollect_info>& info);
    
    void clientGetCollectionInfo(int progressid, obj_fashionCollect_info& info);
    
    void clientGetLatestCollectionId(vector<int>& preProgress);
    
    void sendMaterialBoxNotify(const GridArray& effeGrids);
    
    void sendPropAddNotify();
    
    int materialReclaim(vector<int>& grids);
    
    int materialReclaim(int gridIndex, int count);
    
    void clientGetRecycleInfoAndPropAdded(ack_fashionCollect_prop_added& ack);
    
    int putMaterialIntoGrid(int materialId, int count, int targetProgress, int grid);
    
    int finishProgress(int progressId);
    
    const ItemArray& getMaterialBoxInfo();
    
    MaterialBoxForFashion* getMaterialBox(){return mMaterialBox;}
    
    const int getMaterialSize(){return mMaterialBox->GetCapacity();};
    
    static ItemArray getFashionCollectProduct(int progressId, int roleType);
    
    bool finishProgressFailLastTime();
    
    void addPropByRatio(float ratio);
    
private:

    int checkAdvanceCondition(int progressid);
    
    int onMaterialReclaim(int materialId, int& count);
    
    bool finishProgressUnconditional(int targetProgressid, vector<int>& progressIdFinshed);
    
    bool addProgressMaterial(int targetProgress, int grid, int materialId, int& count);
    
    void loadPreCollection();
    
    void checkSystemAddNewCollection();
    
    void updatePreCollection(int lastestCollectId);
    
    void calTotalProperty();
    
    void calCurrentProgressProp();
    
    void delProgress(int progressid);

    void calHistoryCollectProperty();
    
    void onCalHistoryCollectProperty(PreCollection& precollect, BattleProp& batProp);
    
    void addNewProgressAndSave(int newProgressId);
    
    void addNewProgressAndSave(int newProgressId, string materialStr);
    
    void saveAllCollection();
    
    void savePreCollection(int index, PreCollection& collect);
    
    //这是自动放置材料的， 现有版本不用这个
    void addPlayerMaterial(int materialId, int count);
    
    void onClientGetCollectionInfo(const FashionCollectObject& object, obj_fashionCollect_info& info);
    
private:
    
    map<int, FashionCollectObject> mCollection;
    map<int, PreCollection> mPreCollection;
    
    BattleProp mHistoryAddBattleProp;
    BattleProp mCurrentProgressAddBattleProp;
    
    BattleProp mTotalBattlePropAdded;
    
    int mMasterId;
    Role* mMaster;
    
    MaterialBoxForFashion* mMaterialBox;
    FashionCollectMaterialRecycle* mMaterialRecycle;
    vector<int> mPreFinishId;
};

#endif /* defined(__GameSrv__FashionCollect__) */
