//
//  Wardrobe.h
//  GameSrv
//
//  Created by xinyou on 14-5-14.
//
//

#ifndef __GameSrv__Wardrobe__
#define __GameSrv__Wardrobe__

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <errno.h>
#include "RedisHash.h"
#include "basedef.h"
#include "Defines.h"

using namespace std;

class Role;
struct ItemCfgDef;

class Wardrobe : public WardrobeData
{
public:
    static Wardrobe* create(Role* role);
    
public:
    Wardrobe();
    ~Wardrobe();
    
    bool load();
    
    WardrobeFashionList& getFashionList(int type);
    
    void addExp(int exp);
    bool hasFashion(int fashionId);
    WardrobeFashion* getFashion(int fashionId);
    
    bool checkExpiredFashions(vector<int>* fashions);
    
    bool addFashion(ItemCfgDef* fashionCfg);
    
    void calProperty();
    void accuProperty(BaseProp& baseProp, BattleProp& battleProp);
    Role* getOwner() {return mOwner;}
    
    void traverseFashions(TraverseCallback cb, void* param);
    
    vector<int> getAllFashionId();
    
private:
    
    Role* mOwner;
    int   mOwnerId;
    map<int, WardrobeFashion*> mAllFashions;
    
    BaseProp mBaseProp;
    BattleProp mBatProp;
};

#endif /* defined(__GameSrv__Wardrobe__) */
