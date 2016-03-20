//
//  RoleMgr.cpp
//  CenterServer
//
//  Created by Cheung Zirpon on 15-6-23.
//
//

#include "SortDataMgr.h"

void
CPaihangInfoMgr::loadPaihangData(vector<PaihangInfo*> &data)
{
    if (0 == data.size()) {
        return;
    }
    for (int i = 0; i < data.size(); i++) {
        
        int    objId    = data[i]->getSortValueByType(eSortIndex);
        string sInfo = data[i]->str();
        if (sInfo.empty()) {
            return;
        }
        
        doRedisCmd("hset GameServer:%d:%s %d %s", mServerId, mKey.c_str(), objId, sInfo.c_str());
        
        if (mObjMap.end() != mObjMap.find(objId)) {
            PaihangInfo* info = mObjMap[objId];
            delete info;
            info = NULL;
            mObjMap.erase(objId);
        }
        mObjMap.insert(make_pair(objId, data[i]));
    }
}

void
CPaihangInfoMgr::clear()
{
    for (IterObj iter = mObjMap.begin(); iter != mObjMap.end(); iter++) {
        PaihangInfo* tmp = iter->second;
        delete tmp;
        tmp = NULL;
    }
    mObjMap.clear();
    
}

PaihangInfo*
CPaihangInfoMgr::getInfo(int objId)
{
    IterObj iter = mObjMap.find(objId);
    if (mObjMap.end() != iter) {
        return iter->second;
    }
    
    return NULL;
}



