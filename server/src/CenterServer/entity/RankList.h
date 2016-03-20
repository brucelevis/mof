//
//  RankList.h
//  CenterServer
//
//  Created by Cheung Zirpon on 15-6-23.
//
//

#ifndef __CenterServer__RankList__
#define __CenterServer__RankList__

#include <iostream>
#include <vector>
#include <set>

#include "paihangSortInfo.h"
#include "dbMgr.h"
#include "DataCfg.h"
#include "sendMessage.h"
#include "EnumDef.h"

extern const char* getKeyByEnum(int type);

typedef bool (*CmpFun)(PaihangInfo* first, PaihangInfo* second);

class CSort
{
public:
    CSort(){}
    CSort(vector<CmpFun> funs) : mCmpFunc(funs) {}
    ~CSort(){}
    
    bool operator()(PaihangInfo* first, PaihangInfo* second) const
    {
        for (int i = 0; i < mCmpFunc.size(); i++) {
            if (mCmpFunc[i](first, second)) {
                return true;
            }
        }
        
        return false;
    }
    
    static bool cmpBat(PaihangInfo* first, PaihangInfo* second)
    {
        if (first->getSortValueByType(eSortBat) == second->getSortValueByType(eSortBat))
        {
            return long(first) < long(second);
        }
        return second->getSortValueByType(eSortBat) < first->getSortValueByType(eSortBat);
    }
    
    static bool cmpConsume(PaihangInfo* first, PaihangInfo* second)
    {
        if (first->getSortValueByType(eSortConsume) == second->getSortValueByType(eSortConsume))
        {
            return long(first) < long(second);
        }
        return second->getSortValueByType(eSortConsume) < first->getSortValueByType(eSortConsume);
    }
    
    static bool cmpRecharge(PaihangInfo* first, PaihangInfo* second)
    {
        if (first->getSortValueByType(eSortRecharge) == second->getSortValueByType(eSortRecharge))
        {
            return long(first) < long(second);
        }
        return second->getSortValueByType(eSortRecharge) < first->getSortValueByType(eSortRecharge);
    }
    static bool cmpPetBattle(PaihangInfo* first, PaihangInfo* second)
    {
        if (first->getSortValueByType(eSortPet) == second->getSortValueByType(eSortPet))
        {
            return long(first) < long(second);
        }
        return second->getSortValueByType(eSortPet) < first->getSortValueByType(eSortPet);
    }

    static bool cmpAddress(SortInfo* first, SortInfo* second)
    {
        return long(first) < long(second);
    }
    
    vector<CmpFun> mCmpFunc;
    
};

class CPaihangList
{
public:
    
    CPaihangList(){}
    ~CPaihangList(){}
    
    typedef map<int, PaihangInfo*>::iterator Iterator;
    
    void init(int serverid, SortType type, vector<CmpFun> funtor);
    
    bool loadcheck();
    
    //由于GameSERVER是10个/次发送的需要一个记录总数的,这样写 法 感觉很不好维护  以后有时间会优化
    void onRecvData(int total, vector<PaihangInfo*> &data);
    void freshDB(int total, vector<PaihangInfo*> &data);
    void loadDB();
    void sort();
    void clear();
    void onHeartBeat();
    
    void setNextFreshTime();
    
    void             getList(vector<PaihangInfo*>& data);
    PaihangInfo*        getSortDataById(int objId);
    PaihangInfo*        getSortDataByRank(int rank);
    
    int                     mServerId;
    map<int , PaihangInfo*>    mRecvDataCache;
    map<int , PaihangInfo*>    mData;
    set<PaihangInfo*, CSort>   mList;
    int                     mNextFreshTime;
    string                  mKey;
    SortType                mSortType;
};


#endif /* defined(__CenterServer__RankList__) */
