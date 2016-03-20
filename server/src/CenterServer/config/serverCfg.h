//
//  serverCfg.h
//  CenterServer
//
//  Created by 丁志坚 on 15/6/9.
//
//

#ifndef __CenterServer__serverCfg__
#define __CenterServer__serverCfg__

#include <stdio.h>
#include <map>
#include <vector>
#include <set>
#include <string>
using namespace std;

class ServerCfgDef
{
public:
    int mCfgId;
    int mRunId;
    string mName;
};

class ServerCfg
{
public:
    void loadFromHttp(const char* url);
    
    void load(const char* str);
    
    void clear()
    {
        mCfgIndex.clear();
        mRunIndex.clear();
    }
    
    void addServer(int cfgId, int runId, const char* name)
    {
        ServerCfgDef* record = new ServerCfgDef;
        record->mCfgId = cfgId;
        record->mRunId = runId;
        record->mName = name;
        
        if (mCfgIndex.find(record->mCfgId) != mCfgIndex.end()) {
            return;
        }
        
        mCfgIndex[record->mCfgId] = record;
        map<int, vector<ServerCfgDef*> >::iterator iter = mRunIndex.find(record->mRunId);
        if (iter == mRunIndex.end()) {
            iter = mRunIndex.insert(make_pair(record->mRunId, vector<ServerCfgDef*>())).first;
        }
        iter->second.push_back(record);
    }

    vector<int> getAllRunId();
    bool existRunId(int serverId);
    int  getRunId(int serverId);

    ServerCfgDef* getServerCfgDef(int serverId);

    void dump();
    
    map<int, ServerCfgDef*> mCfgIndex;
    map<int, vector<ServerCfgDef*> > mRunIndex;
};


extern ServerCfg* g_ServerCfg;





#endif /* defined(__CenterServer__serverCfg__) */
