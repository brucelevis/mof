//
//  Worker.h
//  MergeDB
//
//  Created by pireszhi on 14-1-22.
//  Copyright (c) 2014年 pireszhi. All rights reserved.
//

#ifndef __MergeDB__Worker__
#define __MergeDB__Worker__

#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>
#include <map>
#include <set>

#include "dbconnector.h"
using namespace std;

typedef vector<int> RoleList;

struct KeysInfo
{
    KeysInfo()
    {
        ownerId = 0;
        mEffectKeysIndexs.clear();
    }
    int ownerId;
    vector<int> mEffectKeysIndexs;
};

class Worker
{
public:
    Worker(DBConnector& conn):_conn(conn),
                              _Index(0)
    {
        mCheckRolesId.clear();
        mRoleids.clear();
    }
    
    bool onMerge();
    
    
    bool mergePets(int roleid);
    
    //公会
    bool loadAllGuildid();
    bool onLoadAllGuildId(vector<int>& tempId);
    bool mergeGuild();
    bool dumpGuildKeys(int guildid);
    bool restoreGuild(vector<int>& guilds);
    bool onRestoreGuild(int guild, vector<KeysInfo>& guildinfo);
    
    //存档迁移
    bool mergeRoles();
    bool dumpRoles(int roleid);
    bool reStoreRoles(vector<int>& roles);
    bool onRestoreRoles(int roleid, vector<KeysInfo>& roleinfo);
    
    //角色重命名
    bool roleRename();
    bool onRoleRename(vector<int>& roles);
    
    //公会id重置
    bool resetRoleGuildId();
    bool onResetRoleGuildId(vector<int>& roles);
    
    // 删除一下属性
    bool removeRoleProps();
    bool onRemoveRolePeops(vector<int>& roles);
    
    //合并宠物
    bool loadPetsId();
    bool onLoadPetsId(vector<int>& roles);
    string makeNewPetlist(string petlist);
    bool mergePets();
    bool onMergePets(vector<int>& pets);
    
    void loadPlayerList();
    
    bool updateGeneralIds();
    
    //加载活动的东西
    bool mergeActivityData();
    
    void loadActivityKeys(string configkey, vector<string>& keys);
    
    bool onMergeActivityData(string keyName);
    
    bool onMergeAcivityKey(string realKey);
    
    //合并一些全局的值
    bool mergeGlobalMaxVal();
    
    bool mergeGlobalMinVal();
    
public:
    vector<int> mRoleids;
    vector<int> mPetsId;
    vector<int> mGuildAllId;
    set<int> mCheckRolesId;
    set<int> mCheckPetsId;
    
    int _Index;
    DBConnector& _conn;
    vector<string> mPlayers;
    
private:
    bool getPlayerRoles(vector<string>& player);
    bool rolesIdFliter();
    bool onRolesIdFliter(vector<int>& testRoles, time_t now);
    bool petIdFliter();
};

class WorkerMgr
{
public:
    WorkerMgr():gen_roleid(0),
                gen_petid(0),
                gen_guildid(0),
                mValidDays(0),
                mValidMinLvl(0)
    {
        mPlayers.clear();
        mDelRoleIds.clear();
    }
    void doMerge();
    bool init();
    bool loadConfig();
    void setGeneralRoleid(int genid)
    {
        gen_roleid = ( gen_roleid < genid ? genid : gen_roleid );
    }
    
    void setGeneralPetid(int genid)
    {
        gen_petid += genid;
    }
    
    void setGeneralGuildid(int genid)
    {
        gen_guildid += genid;
    }
    
    int getGeneralRoleid()
    {
        return gen_roleid;
    }
    int getGeneralGuildId()
    {
        return gen_guildid;
    }
    int getGeneralPetId()
    {
        return gen_petid;
    }
    
    int getValidDays()
    {
        return mValidDays;
    }
    
    int getValidMinLvl()
    {
        return mValidMinLvl;
    }
    bool addPlayerRoles(string playerName, int roleid);
    
    bool savePlayerRoleList();
    
    bool onSavePlayerRoleList(int& playerCount);
    
    void saveGeneralIds();
    
    void updateGlobalMaxVal(string keyName, int val);
    
    void updateGlobalMinVal(string keyName, int val);
    
    void saveMaxVal();
    
    void saveMinVal();
public:
    map<string, RoleList> mPlayers;
    set<int> mDelRoleIds;
    vector<string> mRoleKeysName;
    vector<string> mGuildKeysName;
    vector<string> mActivityKeys;
    
    vector<string> mMergeMaxKey;
    vector<string> mMergeMinKey;
private:
    int gen_roleid;
    int gen_guildid;
    int gen_petid;
    int mValidDays;
    int mValidMinLvl;
    
    map<string, int> mMaxVal;
    map<string, int> mMinVal;
};

extern WorkerMgr _WORKERMGR;
#endif /* defined(__MergeDB__Worker__) */
