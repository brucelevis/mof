//
//  Paihang.h
//  GameSrv
//
//  Created by pireszhi on 13-7-1.
//
//

#ifndef __GameSrv__Paihang__
#define __GameSrv__Paihang__
#include <string>
#include <stdio.h>
#include <vector>
#include <map>
#include "Defines.h"
#include "Utils.h"
#include "Singleton.h"
#include "paihangSortInfo.h"

#define PAIHANG_LOAD_COUNT 500
#define PAIHANG_COUNT 200

using namespace std;

enum UploadOptionType
{
    eRemove,
    eUpdate,
};
struct PetPaihangUploadData
{
    int petid;
    int battle;
};

struct PaihangMsgData
{
	PaihangMsgData()
	{
		
	}
    int64_t roleid;
    int sessionid;
    int paihangType;
    int job;
    int beginRank;
    int endRank;
};

typedef void (*GETPAIHANGPETDATAFUNC)(int roleid, int sortType, vector<SortPetInfo> data);
typedef void (*GETPAIHANGDATAFUNC)(int roleid, int sortType, vector<SortInfo> data);

class SortManager
{
public:
    SortManager();
    ~SortManager(){};
    void init();
    void updateDB(int roleid, int lvl, int bat, int consume, int recharge, int optionType);
    void updatePetDB(int petid, int battle, int optionType);
    void updatePaihang(int roleid, int pvp);
    bool isRunning()
    {
        return mRunning;
    }

    void safe_delRoleInPaihang(int roleid);
    
    void safe_delPetInPaihang(int petid);

public:
    void getSortData(int type, int job, int beginRank, int endRank, vector<SortInfo*> &outdata);
    void getPetSortData(int beginRank, int endRank, vector<SortPetInfo*> &outdata);
    
    string getPaihangDbKey(int type);
    
    // 取数据库数据
    void getPaihangRoleDbDataByType(int type, vector<SortInfo*> &output);
    void getPaihangPetDbData(vector<SortPetInfo*> &output);

    void loadSortData();
    void getMyRank(int64_t roleid, int type, int &pvp, int &rank);
    void getMyPetRank(int roleid, int &rank);
    void clearSort();
    void onHeartBeat(float dt);
    void setNextFreshTime();
    
protected:
    SortInfo*       getRoleInfoFromDB(int64_t roleid);
    
    //直接取数据库数据
    SortPetInfo*    loadPetInfoFromDB(int petid);
    SortInfo*       loadRoleInfoFromDB(int roleid);
    
    SortInfo* getRobotInfo(int64_t robotid);
private:
    vector<SortInfo*> m_sort_bat;
    vector<SortInfo*> m_sort_lvl;
    vector<SortInfo*> m_sort_pvp;
    
    vector<SortInfo*> m_sort_consume;
    vector<SortInfo*> m_sort_recharge;

    vector<SortPetInfo*> m_sort_pet;
    std::map<int, int> m_role_pet;
    
    map<int64_t, SortInfo*> m_allroledata;
    int mNextFreshTime;
    bool mRunning;
};

void getSortPetData(int roleid, int beginRank, int endRank, GETPAIHANGPETDATAFUNC cbfunc);
void getSortData(int roleid, int type, int beginRank, int endRank, GETPAIHANGDATAFUNC cbfunc, int rolejob = eAllJob);
void geteSortBatData(int roleid, int beginRank, int endRank, GETPAIHANGDATAFUNC cbfunc);



void reqPaihangDbData(int sessionid, int type);
void sendPaihangRoleDbData2CenterServer(int sessionid, int type, vector<SortInfo*> &data);
void sendPaihangPetDbData2CenterServer(int sessionid, int type, vector<SortPetInfo*> &data);


//#define SSortMgr Singleton<SortManager>::Instance()
extern SortManager SSortMgr;

#endif /* defined(__GameSrv__Paihang__) */
