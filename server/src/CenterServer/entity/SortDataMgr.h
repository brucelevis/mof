//
//  RoleMgr.h
//  CenterServer
//
//  Created by Cheung Zirpon on 15-6-23.
//
//

#ifndef __CenterServer__RoleMgr__
#define __CenterServer__RoleMgr__

#include <iostream>
#include <vector>
#include "paihangSortInfo.h"
#include "dbMgr.h"

class CPaihangInfoMgr
{
public:
    CPaihangInfoMgr(){}
    void init(int serverid, string key)
    {
        mServerId = serverid;
        mKey      = key;
    }
    ~CPaihangInfoMgr()
    {
        clear();
    }
    
    //获取GameServer: bat,consume, recharge, pet排行榜上 全部 角色数据 和 幻兽 数据
    void  loadPaihangData(vector<PaihangInfo*> &data);
    void  clear();
    
    PaihangInfo*    getInfo(int objId);

    typedef map<int, PaihangInfo*>::iterator    IterObj;

private:
    
    int                         mServerId;
    string                      mKey;
    map<int, PaihangInfo*>      mObjMap;

};

#endif /* defined(__CenterServer__RoleMgr__) */
